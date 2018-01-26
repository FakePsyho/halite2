Source code for [mine](https://twitter.com/fakepsyho) bot that most probably is going to place 2nd on [Halite 2](https://halite.io/) programming contest.

Before you check the source code, I highly recommend going through this write-up.


# Intro / First Submissions

Originally, I intended to put only few hours into the contest. I failed, hence you can enjoy this write-up :)

I opted for the starting kit (since I like to have something working as soon as possible) and one by one I tried fix major leaks in my bot with as small changes as possible. If you're looking for a relatively simple code that achieves decent results you can check my early submissions (#25 and above). My first submission that broke into top 10 (#35) was around 250 lines (disregarding starter kit and my template code).

Summary of the most important changes (more or less in the order of importance):
* Use global greedy strategy with evaluation function for selecting destinations for ships (more on that below).
* Avoid collisions. I still used default navigation, but I avoided collisions in a most naive way: I reduced speed of a ship until it didn't collide. Since default navigation avoids stationary ships this is guaranteed to work since stationary ships shouldn't collide with anything. (this was obviously removed later).
* Send correct number of ships to each planet (count ships already on the way)
* Limit amount of ships that can follow a single enemy ship.
* Primitive way of retreating when being outnumbered (called evasion in my code)
* In 4-player games, bump priority for planets furthest away from center, as well as drastically reduce priority for planets in the middle to avoid 3+ player battles.
* Hide in the corner for 4-player games.

My final submission is not much that different from the early versions. From high-level point of view, it's actually much simpler. The biggest reason for ~4x increase in code size is the amount of dead code (leftovers from prototyping / debugging) and bloated pre-processing part where I compute various features with non-descriptive variable names. 


# Understanding The Problem

Halite 2 is quite simple game with a lot of depth. It's easy to get consumed with all potential cases that may arise during the gameplay. I won't focus on the details, since there are just too many of them. Each time I was able implement something from my TODO list, three new positions were added there.

The golden rule is to just focus on the things that have the biggest influence on your solution while keeping everything simple. The usual workflow is to watch few replays, see where your bot sucks, add some code that possibly fixes that and submit new version. Hard parts are the ability of finding our weakest point and be able to give up when we're not able to find any simple way of fixing it. Implementing things in a simple way is only hard at first - sooner or later you'll get there so don't worry.

To give you an example, it was clear right away that fixing navigation system is one of the most important things. Unfortunately, rewriting it is a rather big task. I opted for pushing this away as long as I could in order to figure out what's the easiest way of including it in my solution. As a temporary measure I added self-collision avoidance as a post-process step since I knew I was able to quickly implement this. It may look like an insignificant decision, but thanks to that I avoided potentially huge refactor of my code in the future while I was able to keep a steady flow of improvements.

My general mindset was to avoid any metagame-specific problems (situations that occur exclusively due to currently dominating style of playing) and instead focus only on issues that after fixing them the outcome would be positive no matter how my opponents are behaving. I also focused almost exclusively on 4-player games, because they had much higher impact on the final ranking (everyone will have twice as many 4-player games as 2-player games). All of my 2-player or 4-player specific strategies are just small modifications to evaluation function (more on that below).


# Core Concepts

Before I dive to explaining my solution, let's explain two core concepts that might not be familiar to everyone.

### Evaluation Function

Evaluation function is a similar concept to fitness function. Basically, it's a function that given some state/decision/situation will evaluate it to (usually) a single number. Allowing us to order states/decisions/situations from best to worst.

Designing solution around evaluation function means that you call your evaluation function with all potential choices and you select one that gives the best result. This means that if you want to give some type of decisions higher priority than to others, you need to bump up their values.

If you have some machine learning experience, you can think of evaluation function as a model with high-level custom features and hand-crafted weights.

Evaluation function is generally used as a replacement for decision trees (random forest vs neural network analogy) or analytic solution. Good example of analytic vs evaluation is movement. You can try to calculate optimal move for ship directly (bad idea), or you can call your evaluation function for all possible angle/thrust values and select the best one. For example, the most naive pathfinding would be to evaluate all moves, for those that collide return `-infinity` and for those that don't collide return `-distance_to_target`. And that's already far superior to default navigation.

As for decision trees (i.e. nested if-else clauses), there are several advantages of evaluation functions over trees:
* Keeps all decision-making logic in one place, which helps with quick iteration process
* Doubles as a design pattern and keeps your code well organized
* Allows for easy way of combining several high-level features
* Allows for (semi-)automated weight adjustment
* Discourages you from using decision-tree logic (decision-trees are not necessarily bad, but people tend to overuse them)

So, to summarize. It may look like a small thing, it may feel awkward at first, but it's a way to go when designing behavior of AI-based systems. Especially when the environment is more on the simple side as its the case with Halite. Just don't treat it as a remedy for all of your future problems.


### Global Greedy Assignment

There's a common problem in AI-battle tasks where you have N units to your disposal and each of them can perform some set of actions. There's also a standard way of dealing with it. If the evaluation of (unit, action) doesn't depend on results of other units, you can just iterate over all units and select the best action for each one. If it depends, things get slightly more complicated. When your evaluation function is comparable between different units, you can use global greedy assignment (not an official name of algorithm, but it's quite descriptive).

Works as follows:
```
while (len(units_without_actions) > 0) {
  best_ship = null
  best_action = null
  best_eval = -infinity
  for unit in units_without_actions {
    for action in possible_actions {
      eval = evaluate(unit, action)
      if (eval > best_eval) {
        best_eval = eval
        best_unit = unit
        best_action = action
      }
    }
  }
  update(best_unit, best_action)  
}
```

For most problems this is going to work really well and will give result close to perfect matching. The only downside is that the algorithm now takes `O(ships^2*actions)` instead of `O(ships*actions)`. But well, just learn to write fast code ;) Since this loop was expensive for me, I cached best action for each ship and recalculated it only when evaluation value for that action changed. This works, because my evaluation function could not improve after `update()` equivalent. Which means, if action gave the same value, it's guaranteed that it's still the best choice. Small trick but slashed down execution times by 50-100x in worst cases for whole phase.


# Phases

Now that we have our basics explained, let's see how each of my turns look like:

### 1) Pre-Process

This calculates various global/ship/planet features that are going to be used later.

There isn't much to talk about here. The reason why I'm calculating all of the features beforehand is to have as short decision code as possible. Also, very ugly, please ignore. 

### 2) Calculate Orders

This assigns high-level orders to each ship by using already-mentioned greedy assignment.

I have only two types of actions: go to planet and colonize it & attack enemy ship. In my code there are few other types, but those are just leftovers from experimenting. So, for each of my ships I iterate over all empty planets & all enemy ships. For each of them I calculate evaluation function and I take the action that returns the best score.

The core of this selection can be mostly formulated as: 
If there's enemy ship within X distance, prioritize enemy ship. Otherwise go to the closest empty planet. Both ships & planet values are augmented by few other features but that's the core concept. 

### 3) Calculate Moves

Based on high-level orders, calculate move (i.e. angle/thrust pair) for each ship.

By now each ship should have an assigned action (order) to it. Either it's going to try colonize planets or attack enemy ships. In rare cases (usually at the end of the game when I'm winning) no orders are assigned, but it's fine since the game is finished anyway.

Moves are calculated using another evaluation function. I iterate over all (angle, thrust) pairs, discard all moves that end up colliding and select the one that maximizes evaluation function. Depending on the type of move, evaluation function is called with different set of parameters that enable/disable various components of evaluation (distance to target, penalty from being close to enemies, bonus for being close to allies, etc.).

For collision checking I'm using currently calculated moves for each of my ship. This way, the set of current paths is always collision-free. 

### 4) Evasion/Clustering Post-Process

Based on calculated moves, find all my ships that might fight inefficiently and recalculate moves for them.

Fighting efficiency can be defined as ratio of `my_expected_attacks / total_expected_attackss`. For estimation I use a rather naive heuristic where I assume that each enemy ship is flying directly towards me, while my ships follow exactly already calculated path. If that ratio is 0.5 or lower (in other words, number of my attacking ships is lower than those of opponents) I mark that this ship should retreat. 

All marked ships have their moves recalculated with included components for clustering and avoiding enemies.

### 5) Baiting Post-Process

Based on calculated moves, try to find set of moves that may turn inefficient fight into efficient one.

Even if you're outnumbered, it's possible to move in a way that only some of the enemy ships will be able to attack you. If you're able to position your ships that are outside of 13 (ship attack + max move speed) range, they won't be able to attack you no matter how they are going to move. I try to reposition my retreating ships so that they are outside of that range for all except one ship. This way, if any attack occurs, it's going to be at least even fight for me.

This works on the assumption that enemy is chasing you with full speed when you're outnumbered. Unfortunately, the trade off is that you're reducing the distance to the enemy which may backfire. I'm certain that this tactic was very effective as early rush defence (you're going to have nearly 100% winrate vs players who just chase you), but I'm not sure how effective it was during standard game.


# Summary

So, this is it. If you think that doesn't seem like much, you're probably right. On the other hand, the devil lies in the details. I have omitted pretty much all of the details for my evaluation functions. And well, there's no other way around than reading through the code (which I highly recommend considering you already have a high-level overview of what is happening there).

For example, this part:
```
if (avoidEnemies && earlyPhase && players_no == 4) {
  av += (leftSide ? max(0.0, end.pos_x - 50) : max(0.0, m.map_width - end.pos_x - 50)) * 0.025;
}
```
gives penalty for 4-player games when during rush defence, my ships will randomly walk on the second half of the map. I added it after noticing that sometimes my ships wander through enemy territory. 

This part is responsible for whole "clustering" behavior of my ships:
```
for (Entity& e : allies) {
  av += 17.5 * sqrt(max(1.0, end.dist(e.destination()) + 0.0));
}
```
both `max` and `+ 0.0` are just leftovers from experimenting with over values, `17.5` is just some magic constant so that it works well along my other magic constants; also yes, literally this thing alone is responsible for any kind of clustering behavior I have in my code, the power of evaluation functions and using them effectively

Unfortunately, I can't tell what parts were the most important ones. After getting all the basics right (see first section), your gains are usually statistically insignificant in such problems. Most of my biggest improvements came from looking at 10 replays where I lost and I trying to correct behavior of my AI by adding few lines of code to some evaluation function.

# Random Comments

There are dozens of small tricks within the code. I'll give a list of some of the more interesting ones that I still remember:
* It wasn't mentioned anywhere, but my solution is completely stateless (other than knowing which turn it is). It drastically simplifies the solution and reduces the amount of possible bugs you can make. In general, unless you absolutely know what you're doing, keep your solution stateless.
* When calculating distance to targets, I'm ignoring planets. Not the ideal solution, but it has very little impact. There's a chance that ship can get stuck because there's a big planet directly in front if it and every non-colliding move it can generate, moves away from the target. The quick fix for that was to disallow moves that end up very close to the planet (there's no point in doing it anyway).
* You may notice in my code that both 3rd and 4th phases are performed several times. 3rd phase has several passes because there's a chance that ship may want to go to a place which is currently occupied by another ship which may change after all ships are processed. Similarly 4th phase is repeated because it doesn't always happen that all ships will get marked to retreat in one pass. And, each time ship's move is recalculated, it affects the evaluation function values for neighboring ships.
* There's no point in moving very close to the border of the map, unless you're retreating and you have absolutely no other way to go (since it limits your maneuverability). For ships that aren't retreating, I mark all moves that end up within X distance of the border as invalid. This slightly helps with chasing ships.
* I use expected spawning positions of new ships during 4th phase and when evaluating movement. I just reimplemented the same function from Halite's source.
* Calculating if ship is for colonizing is a little bit tricky but I found a nice heuristic for that. Let us define "safe distance" as first distance where enemy ships outnumber our ships. This simulates the risk of getting outnumbered (and thus the miner dying defenceless).
* This was mentioned before, but limiting how many ships can follow each enemy ship is a really cheap way of forcing your units to spread among different goals. In fact, it works so well that I never got rid off this behavior.
* Normally when I'm targeting enemy ships, I'm using their previous position as my target (possibly adjusted by max speed so that I won't slow down when they were very close turn before). However, as a countermeasure for harassment. For single enemy units, I try to predict where the enemy would go by running my own move function on that unit and use the predicted movement as a new target.
* There's a small trick in movement evaluation function that tries to move ships with lower health further away from the enemies. While this is implemented very badly and doesn't give a lot, it's another example of showing what evaluation functions are capable of while requiring only minimal effort.
* Handling of rushes is just done by not docking/undocking when enemy ships are within certain distance. There are few small additional features, but mostly there's no unique logic for handling rushes in my code.
* I implemented an (almost) distributed tester with built-in matchmaking system similar to what Halite uses (although I fixed the issue of unbalanced number of 2p/4p matches that was driving me crazy). This was the first time when I implemented something like this, so definitely don't look at it as a model way of implementing it. It took me around 10-12h to get it, which most probably wasn't worth it, but I treated as my learning project (I had no prior knowledge of redis, etc). "Almost" is in parenthesis because there're still few small features missing in order to make it fully distributed. I didn't finish it, because 48-core machine (EC2 m5.24xlarge) was able to simulate 25K games per hour for me, which was enough. Worth noting is that may way of implementing remote commands in `cli.py` is, let's just say, less than optimal. I'm talking about all of those `plink` calls - a putty-equivalent to `ssh` on linux.

There's also a ton of things that I haven't implemented:
* I don't do any kind of defending at all. This is the biggest weakness of my solution. There's nothing worse in early game than having 10 ships randomly chasing 2 harassing ships while your base dies. The reason I gave this low priority is that attacking is far more important (you don't have to defend when your opponent is dead). Plus, defending doesn't make sense if you're not able to make it more effective than chasing enemies. And this ain't easy.
* I thought about solving 3v3 rushes by using reinforced learning. While the game is not a good target for ML/RL in general, since it essentially has two different logical layers (high-level order logic and low-level move logic). Those small-scale battles are ideal situation for them.
* Another random idea involving machine learning was to download all replays from top bots and train lstm-based supervised model for enemy ships as a predictive model. Then, use those predictions as expected moves and essentially modify our problem into optimization problem where each turn you try to maximize battle efficiency of our ships (damage dealt vs damage received). In the simplest form, I could try to use it for 3v3 battles only.
* There's somewhat easy way for implementing effective algorithm for selecting starting planets. It's very cheap to perform forward simulations of the game if you assume that ships are just going to the closest planet and all fights end up with both ships dead. This way you can perform hundreds of random simulations and take the one that gives highest expected ratio of `your_ships / all_ships`. Main reason why I haven't done this is that for 2-player games that would result in higher percentage of games ending up in rushes (my AI wasn't moving to the center as often as it should) so I wasn't sure if the net gain would be positive.

# Source Code

My source consists of a single file: `MyBot.cpp` and (slightly modified) starter kit. Basically, whenever I felt I'm tired of writing the same code over and over again, I added new method somewhere within the starter kit. I tried to keep it mostly backwards compatible (since I archived only `MyBot.cpp` files), but I'm not certain if that's the actual case. 

`old.zip` contains source codes for all submissions I made. If you have more time, you could see how my solution evolved over time. Unfortunately I didn't had a script for saving those for the first few submissions. 

The top part of my `MyBot.cpp` is my template code that I use in pretty much every programming contest. The core part are the defines that allow for faster prototyping. I use it so often that writing `for (int i = 0; i < n; i++)` feels very unfamiliar for me now. Most C++ programmers use something similar in competitive programming community and most competitive programmers use C++. Since we all use similar set of macros, reading code with those `#define`s is actually easier than reading such code without it. For us, that is.

The rest of the source code follows described phases and should be more or less clear. The most important part is `move_to()` function that evaluates and selects best move based on supplied set of parameters. It's a little messy, but it handles all movement within my solution (i.e. going to planets, rushes, clustering, harassment, normal attacks, chasing enemy).

`cli.py` doubles as both distributed tester and command-line interface to it. The way it works is you create a remote machine, do all of the necessary setup (`cli_setup.sh` is designed for Amazon's EC2 Ubuntu machine, DO NOT RUN THIS FILE LOCALLY). You set the IP in `config.json` and then you just use `cli.py` locally to execute commands. You can add/remove bots, produce rankings and few other things. 

# Closing Words

If you liked this competition and you're searching for something similar and you're not willing to wait for Halite 3. You should check out [CodinGame](https://www.codingame.com/home). From my point of view, Halite was done much better than anything I saw on CodinGame. Plus, in the past they showed some (truly) remarkable incompetence. That being said, it's the only site that regularly offers contests with a similar structure. Also, ReCurse said that they improved, so blame him if anything goes wrong ;)

If this got you more interested in competitive programming I highly recommend reading an Overview of Programming Contests (http://psyho.gg/overview-of-programming-contests/) that I wrote few years ago. Even though it's slightly outdated, it's still the best way of learning what kind of programming contests you can find out there and what each of them teaches. The blog also contains few in-depth write-ups about other contests I participated in. Just a small disclaimer, I'm no longer updating the blog due to limited available time. 

# Questions?

If you have any questions feel free to ask me on Halite's discord or ask directly through [twitter](https://twitter.com/fakepsyho). 
