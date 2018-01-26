#! /usr/bin/env python3


# TODO: mode_remove: "rm: invalid option -- 'q'"
# TODO: investigate crashes (happen only when giving new commands)
# TODO: save logs from workers
# TODO: cache previous backups?
# TODO: add support for more machines?
# TODO: check if redis IP can be non-local for VM?
# TODO: change to controller + workers
# TODO: when removing bot, data in games should be replaced?
# TODO: ability to deactivate bot from playing game? (helps with bot distribution)
# TODO: add rename (alias?) command (probably need a pause or has to be during stop)?
# TODO: add mode for executing script / installing everything (mode_init)
# TODO: verify successful compilation when adding a new bot

import copy
import time
import random
import datetime
import re
import argparse
import json
import os.path
import subprocess
import trueskill as ts
import redis

DEFAULT_TAU = 0.002
DEFAULT_WEIGHT_EXP = 2
DEFAULT_PROB_FFA = 0.5
DEFAULT_MIN_MATCHES = 200
DEFAULT_MM_COEFF = 2.5
DEFAULT_WORKERS_NO = 2

CONFIG_FILE = 'config.json'
HALITE_PATH = './halite'

pars = None
redis_connection = None


# Various functions

def connect():
    global redis_connection
    if redis_connection is None:
        cfg = load_config()
        try:
            redis_connection = redis.Redis(host=cfg['server_ip'], decode_responses=True)
        except redis.RedisError:
            redis_connection = None
    return redis_connection


def log(*args):
    r = connect()
    if r is None:
        return

    dt = datetime.datetime.utcnow().replace(microsecond=0)
    r.rpush('logs', '[' + str(dt) + '] ' + ' '.join(map(str, args)))


def load_config():
    if not os.path.exists(CONFIG_FILE):
        return json.loads('{}')
    return json.load(open(CONFIG_FILE, 'r'))


def save_config(config):
    json.dump(config, open(CONFIG_FILE, 'w'), indent=4)


# Worker functions

def play_game(bots):
    cmd = HALITE_PATH + " -r -t"
    for bot in bots:
        cmd += " ./bots/" + bot
    task = subprocess.run(cmd.split(), stdout=subprocess.PIPE)

    rv = [-1] * len(bots)
    for line in task.stdout.decode('UTF-8').split('\n'):
        search = re.search('^Player #(.), .+ in rank #(.) and was .+', line)
        if search:
            rv[int(search.group(1))] = int(search.group(2))

    return rv


def reset_ranking(ranking, bot):
    ranking[bot]['mu'] = 25.0
    ranking[bot]['mu_1v1'] = 25.0
    ranking[bot]['mu_ffa'] = 25.0
    ranking[bot]['sigma'] = 8.333
    ranking[bot]['sigma_1v1'] = 8.333
    ranking[bot]['sigma_ffa'] = 8.333
    ranking[bot]['games'] = 0


def calc_new_ranking(ranking, bots, result, tau):
    ts.setup(tau=tau, draw_probability=0.0001)
    ratings = [[ts.Rating(mu=ranking[bot]['mu'], sigma=ranking[bot]['sigma'])] for bot in bots]
    ratings_ffa = [[ts.Rating(mu=ranking[bot]['mu_ffa'], sigma=ranking[bot]['sigma_ffa'])] for bot in bots]
    ratings_1v1 = [[ts.Rating(mu=ranking[bot]['mu_1v1'], sigma=ranking[bot]['sigma_1v1'])] for bot in bots]
    ratings = ts.rate(ratings, ranks=result)
    if len(bots) == 2:
        ratings_1v1 = ts.rate(ratings_1v1, ranks=result)
    else:
        ratings_ffa = ts.rate(ratings_ffa, ranks=result)
    for i, bot in enumerate(bots):
        ranking[bot]['mu'] = ratings[i][0].mu
        ranking[bot]['sigma'] = ratings[i][0].sigma
        ranking[bot]['mu_ffa'] = ratings_ffa[i][0].mu
        ranking[bot]['sigma_ffa'] = ratings_ffa[i][0].sigma
        ranking[bot]['mu_1v1'] = ratings_1v1[i][0].mu
        ranking[bot]['sigma_1v1'] = ratings_1v1[i][0].sigma
        ranking[bot]['games'] += 1


def update_ranking(bots, result):
    r = connect()
    tau = float(r.get('cfg_tau') or DEFAULT_TAU)

    with r.pipeline() as pipe:
        while True:
            try:
                pipe.watch('bots')
                ranking = json.loads(r.get('bots'))

                if not all([bot in ranking for bot in bots]):
                    return

                calc_new_ranking(ranking, bots, result, tau)

                ranking_str = json.dumps(ranking)
                pipe.multi()
                r.set('bots', ranking_str)
                pipe.execute()
                break
            except redis.WatchError:
                r.incr('bots_updates_fail')
                continue

    # TODO: add more stats for games? (seed, replay path, turns, ship produced?)
    r.rpush('games', json.dumps({'bots': bots, 'result': result}))
    log('New Game Played', bots, result)


def choose_match():
    r = connect()
    weight_exp = float(r.get('cfg_weight_exp') or DEFAULT_WEIGHT_EXP)
    min_matches = int(r.get('cfg_min_matches') or DEFAULT_MIN_MATCHES)
    mm_coeff = float(r.get('cfg_mm_coeff') or DEFAULT_MM_COEFF)

    ranking = json.loads(r.get('bots'))
    if len(ranking) < 2:
        return None
    bots = [{'name': k, 'mu': v['mu'], 'sigma': v['sigma'], 'games': v['games']} for k, v in ranking.items()]
    bots = sorted(bots, key=lambda b: b['mu'], reverse=True)

    while True:
        prob_ffa = 0
        do_min_matches = any([bot['games'] < min_matches for bot in bots])
        if do_min_matches and random.random() < 3/4:
            prob_ffa = 2/3
            selected_bot = bots[min([(bot['games'], i) for i, bot in enumerate(bots)])[1]]
        else:
            prob_ffa = 0 if do_min_matches else 1/2
            weights = [pow((len(bots) - i) * 1.0 / len(bots), weight_exp) / pow(bot['games'], 0.3) for i, bot in enumerate(bots)]
            # selected_bot = bots[random.choices(range(len(bots)), weights)]
            v = random.random() * sum(weights)
            selected_bot = bots[len(bots) - 1]
            for i, w in enumerate(weights):
                v -= w
                if v <= 0:
                    selected_bot = bots[i]
                    break

        players_no = 4 if random.random() < prob_ffa else 2

        mu_rank_limit = mm_coeff / (0.01 + random.random()) ** 0.65

        opponents = [bot['name'] for bot in bots if bot['name'] != selected_bot['name'] and abs(bot['mu'] - selected_bot['mu']) < mu_rank_limit]
        if len(opponents) + 1 < players_no:
            continue

        rv = [selected_bot['name']] + random.sample(opponents, players_no - 1)
        random.shuffle(rv)
        return rv


def mode_worker():
    log('[Worker] +1')
    r = connect()
    r.incr('msg_workers_no')
    try:
        while True:
            if not r.exists('msg_active'):
                break

            bots = choose_match()
            if bots is None:
                time.sleep(0.25)
                continue

            results = play_game(bots)
            update_ranking(bots, results)
    finally:
        log('[Worker] -1')
        r.decr('msg_workers_no')


def mode_stop():
    log('[Action] Stop')
    # TODO: check number of running processes remotely?

    r = connect()
    r.delete('msg_active')
    while True:
        workers_no = r.get('msg_workers_no') or 0
        print("Current Worker Count:", workers_no)
        if int(workers_no) == 0:
            break
        time.sleep(1)


def mode_start():
    r = connect()
    if r.exists('msg_active'):
        print('[Error] Already Started')
        return
    log('[Action] Start')
    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    r.set('msg_active', 1)
    workers_no = int(r.get('cfg_workers_no') or DEFAULT_WORKERS_NO)
    for i in range(workers_no):
        print('\rStarting Worker %d / %d    ' % (i+1, workers_no), end='')
        subprocess.run('plink %s "screen -d -m ./cli.py worker"' % address)
    print()


def mode_restart():
    log('[Action] Restart')
    mode_stop()
    mode_start()


# Entry points for different modes

def mode_status():
    log('[Action] Status')
    r = connect()
    if not r:
        print('[Error] Can\'t connect to redis')
        return

    print('Active:', r.exists('msg_active'))
    print('Max Workers:', r.get('cfg_workers_no'))
    print('Current Workers:', r.get('msg_workers_no'))
    print('Games Total:', r.llen('games'))
    print('Failed Updates:', r.get('bots_updates_fail'))
    print('Log Total:', r.llen('logs'))
    if pars.logs_no > 0:
        print('Last %d Logs:' % pars.logs_no)
        logs = r.lrange('logs', -pars.logs_no, -1)
        for line in logs:
            print('> ' + line)


def mode_add_bot():
    log('[Action] Add Bot', pars.name)
    r = connect()
    ranking = json.loads(r.get('bots'))
    if pars.name in ranking:
        print('[Error] Bot %s already exists' % pars.name)
        return

    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    subprocess.run('zip bot.zip -q hlt/* MyBot.cpp')
    subprocess.run('pscp bot.zip %s:bot.zip' % address)
    subprocess.run('plink %s "unzip -o bot.zip"' % address)
    subprocess.run('plink %s "g++ -std=gnu++14 -O2 MyBot.cpp -o bot hlt/hlt_in.cpp hlt/location.cpp hlt/map.cpp"' % address)
    subprocess.run('plink %s "zip -q backup/%s.zip hlt/* MyBot.cpp bot"' % (address, pars.name))
    subprocess.run('plink %s "cp bot bots/%s"' % (address, pars.name))
    subprocess.run('plink %s "rm -rf bot.zip hlt MyBot.cpp bot"' % address)

    with r.pipeline() as pipe:
        while True:
            try:
                pipe.watch('bots')
                ranking = json.loads(r.get('bots'))

                ranking[pars.name] = {'name': pars.name,
                                      'desc': pars.description,
                                      'date': str(datetime.datetime.utcnow().replace(microsecond=0))}
                reset_ranking(ranking, pars.name)

                ranking_str = json.dumps(ranking)
                pipe.multi()
                pipe.set('bots', ranking_str)
                pipe.execute()
                break
            except redis.WatchError:
                continue


def mode_remove_bot():
    log('[Action] Remove', pars.name)
    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    r = connect()
    subprocess.run('plink %s "rm -q backup/%s.zip bots/%s"' % (address, pars.name, pars.name))
    with r.pipeline() as pipe:
        while True:
            try:
                pipe.watch('bots')
                ranking = json.loads(r.get('bots'))
                if pars.name not in ranking:
                    print('[Error] Bot %s doesn\'t exist' % pars.name)
                    break

                del ranking[pars.name]

                ranking_str = json.dumps(ranking)
                pipe.multi()
                pipe.set('bots', ranking_str)
                pipe.execute()
                break
            except redis.WatchError:
                continue


def mode_update():
    log('[Action] Update')
    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    r = connect()
    if not r.exists('bots'):
        r.set('bots', json.dumps({}))

    for k, v in cfg.items():
        r.set('cfg_' + k, v)
        log('Updating', k, 'to', v)

    subprocess.run('pscp cli.py %s:cli.py' % address)
    cfg['server_ip'] = '127.0.0.1'
    json.dump(cfg, open('tmp.json', 'w'), indent=4)
    subprocess.run('pscp tmp.json %s:config.json' % address)
    subprocess.run('rm tmp.json')


def mode_backup():
    if not pars.file.endswith('.zip'):
        pars.file += '.zip'
    log('[Action] Backup')
    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    subprocess.run('plink %s "redis-cli save"' % address)
    subprocess.run('plink %s "cp /var/lib/redis/dump.rdb backup.rdb"' % address)
    subprocess.run('plink %s "zip -q data.zip backup/* bots/* backup.rdb' % address)
    subprocess.run('pscp %s:data.zip %s' % (address, pars.file))
    subprocess.run('plink %s "rm -rf backup.rdb data.zip"' % address)


def mode_restore():
    if not pars.file.endswith('.zip'):
        pars.file += '.zip'
    # log('[Action] Restore')
    cfg = load_config()
    address = 'ubuntu@%s' % cfg['server_ip']
    subprocess.run('plink %s "redis-cli shutdown"' % address)
    subprocess.run('plink %s "rm -rf *.zip backup/* bots/*"' % address)
    subprocess.run('pscp %s %s:data.zip' % (pars.file, address))
    subprocess.run('plink %s "unzip -o data.zip"' % address)
    subprocess.run('plink %s "sudo mv backup.rdb /var/lib/redis/dump.rdb"' % address)
    subprocess.run('plink %s "rm -rf *.zip"' % address)
    subprocess.run('plink %s "sudo redis-server /etc/redis/redis.conf --daemonize yes"' % address)
    r = connect()
    r.delete('msg_active')
    r.delete('msg_workers_no')
    log('[Action] Successfully Restored')


def mode_init():
    pass


def mode_show_ranking():
    log('[Action] Ranking')
    r = connect()
    ranking = json.loads(r.get('bots'))

    if pars.true:

        games = [json.loads(game) for game in r.lrange('games', 0, -1)]
        games = [game for game in games if all([bot in ranking for bot in game['bots']])]
        new_ranking = copy.deepcopy(ranking)
        for bot in new_ranking.keys():
            reset_ranking(new_ranking, bot)
        for i, game in enumerate(games):
            if i % 100 == 0:
                print('\rRecalculating Ratings: %d / %d       ' % (i, len(games)), end='')
            for bot in game['bots']:
                tmp_ranking = copy.deepcopy(ranking)
                tmp_ranking[bot] = new_ranking[bot].copy()
                calc_new_ranking(tmp_ranking, game['bots'], game['result'], 0)
                new_ranking[bot] = tmp_ranking[bot]
        ranking = new_ranking

    ranking = sorted(ranking.values(), key=lambda b: b['mu'] - 3 * b['sigma'], reverse=True)
    for i, b in enumerate(ranking[0:pars.limit]):
        print('{:>3} {:>12} {:>5.2f} {:>5} {:>5.2f} {:>5.3f} {:>5.2f} {:>5.3f} {:>5.2f} {:>5.3f} {:>34}'.format(
            i+1,
            b['name'],
            b['mu'] - 3 * b['sigma'],
            b['games'],
            b['mu'],
            b['sigma'],
            b['mu_1v1'],
            b['sigma_1v1'],
            b['mu_ffa'],
            b['sigma_ffa'],
            b['desc'],
        ))


def mode_bot_stats():
    log('[Action] Stats')
    r = connect()
    ranking = json.loads(r.get('bots'))
    if pars.name not in ranking:
        print('[Error] Bot %s doesn\'t exist' % pars.name)
        return
    sorted_ranking = sorted(ranking.values(), key=lambda b: b['mu'] - 3 * b['sigma'], reverse=True)

    rank = next(i for i, b in enumerate(sorted_ranking) if b['name'] == pars.name) + 1

    games = [json.loads(game) for game in r.lrange('games', 0, -1)]
    games = [game for game in games if pars.name in game['bots']]
    games_1v1 = 0
    games_ffa = 0
    places_1v1 = [0] * 2
    places_ffa = [0] * 4
    for game in games:
        pos = game['result'][game['bots'].index(pars.name)] - 1
        if len(game['bots']) == 2:
            games_1v1 += 1
            places_1v1[pos] += 1
        else:
            games_ffa += 1
            places_ffa[pos] += 1

    places_1v1 = [v / sum(places_1v1) * 100 for v in places_1v1]
    places_ffa = [v / sum(places_ffa) * 100 for v in places_ffa]

    bot = ranking[pars.name]
    print('Name:', bot['name'])
    print('Description:', bot['desc'])
    print('Rank:', rank)
    print('Date:', bot['date'])
    print('Rating:', bot['mu'] - 3 * bot['sigma'])
    print('All: {:>5.2f} {:>5.3f} {:>5}'.format(bot['mu'], bot['sigma'], bot['games']))
    print('1v1: {:>5.2f} {:>5.3f} {:>5} {:>5.2f} {:>5.2f}'.format(bot['mu_1v1'], bot['sigma_1v1'], games_1v1, places_1v1[0], places_1v1[1]))
    print('FFA: {:>5.2f} {:>5.3f} {:>5} {:>5.2f} {:>5.2f} {:>5.2f} {:>5.2f}'.format(bot['mu_ffa'], bot['sigma_ffa'], games_ffa, places_ffa[0], places_ffa[1], places_ffa[2], places_ffa[3]))

    sorted_ranking.insert(0, {'name': '---'})
    true_mu = 0
    true_mu_1v1 = 0
    true_mu_ffa = 0
    for rank, bot in enumerate(sorted_ranking):
        if bot['name'] == pars.name:
            continue
        new_ranking = copy.deepcopy(ranking)
        reset_ranking(new_ranking, pars.name)

        games_1v1 = 0
        games_ffa = 0
        wins_1v1 = 0
        wins_ffa = 0
        for game in games:
            if any([name not in ranking for name in game['bots']]):
                continue

            if bot['name'] in game['bots']:
                my_pos = game['result'][game['bots'].index(pars.name)] - 1
                opp_pos = game['result'][game['bots'].index(bot['name'])] - 1
                if len(game['bots']) == 2:
                    games_1v1 += 1
                    wins_1v1 += my_pos < opp_pos
                else:
                    games_ffa += 1
                    wins_ffa += my_pos < opp_pos
            else:
                calc_new_ranking(new_ranking, game['bots'], game['result'], 0)
                new_ranking = {name: bot.copy() if bot['name'] != pars.name else new_ranking[pars.name] for name, bot in ranking.items()}

        if rank == 0:
            true_mu = new_ranking[pars.name]['mu']
            true_mu_1v1 = new_ranking[pars.name]['mu_1v1']
            true_mu_ffa = new_ranking[pars.name]['mu_ffa']
            print('True Mu: {:>5.2f}'.format(true_mu))
            print('Opponents:')
            continue

        print('{:>3} {:>12}  Total: {:>5.2f}   1v1: {:>5} {:>5.2f} {:>5.2f}   FFA: {:>5} {:>5.2f} {:>5.2f}'.format(
            rank,
            bot['name'],
            true_mu - new_ranking[pars.name]['mu'],
            games_1v1,
            min(99.99, 0 if games_1v1 == 0 else wins_1v1 / games_1v1 * 100),
            true_mu_1v1 - new_ranking[pars.name]['mu_1v1'],
            games_ffa,
            min(99.99, 0 if games_ffa == 0 else wins_ffa / games_ffa * 100),
            true_mu_ffa - new_ranking[pars.name]['mu_ffa'],
        ))


def main():
    parser = argparse.ArgumentParser(description='description goes here')
    subparsers = parser.add_subparsers(title='commands')

    parser_status = subparsers.add_parser('status', help='checks the status of server')
    parser_status.set_defaults(func=mode_status)
    parser_status.add_argument('logs_no', help='number of lines of log to print', type=int, nargs='?', default=50)

    parser_bot_stats = subparsers.add_parser('bot', help='print statistics about specific bot')
    parser_bot_stats.set_defaults(func=mode_bot_stats)
    parser_bot_stats.add_argument('name', help='name of the bot')

    parser_add_bot = subparsers.add_parser('add', help='submits current codebase as a new bot to the server')
    parser_add_bot.set_defaults(func=mode_add_bot)
    parser_add_bot.add_argument('name', help='name of the bot')
    parser_add_bot.add_argument('description', help='description of the bot', nargs='?', default='n/a')

    parser_remove_bot = subparsers.add_parser('remove', help='removes bot from the server')
    parser_remove_bot.set_defaults(func=mode_remove_bot)
    parser_remove_bot.add_argument('name', help='name of the bot')

    parser_backup = subparsers.add_parser('backup', help='backs up the data (redis and bots)')
    parser_backup.set_defaults(func=mode_backup)
    parser_backup.add_argument('file', help='file name for backup zip file', nargs='?', default='backup')

    parser_restore = subparsers.add_parser('restore', help='restores backed up data')
    parser_restore.set_defaults(func=mode_restore)
    parser_restore.add_argument('file', help='zip file generated through backup command', nargs='?', default='backup')

    parser_update = subparsers.add_parser('update', help='updates this script on server')
    parser_update.set_defaults(func=mode_update)

    parser_show_ranking = subparsers.add_parser('ranking', help='shows the current TS ranking for all bots')
    parser_show_ranking.set_defaults(func=mode_show_ranking)
    parser_show_ranking.add_argument('limit', help='limits ranking to top X bots', nargs='?', type=int, default=20)
    parser_show_ranking.add_argument('-t', '--true', help='shows true rating (assuming tau=0)', action='store_true')

    parser_start = subparsers.add_parser('start', help='start remote workers')
    parser_start.set_defaults(func=mode_start)

    parser_stop = subparsers.add_parser('stop', help='stop remote workers')
    parser_stop.set_defaults(func=mode_stop)

    parser_restart = subparsers.add_parser('restart', help='restart remote workers')
    parser_restart.set_defaults(func=mode_restart)

    parser_init = subparsers.add_parser('init', help='initializes server')
    parser_init.set_defaults(func=mode_init)

    parser_worker = subparsers.add_parser('worker', help='starts worker on current machine (should be executed only on VM)')
    parser_worker.set_defaults(func=mode_worker)

    global pars
    pars = parser.parse_args()

    if pars.func:
        pars.func()


if __name__ == '__main__':
    main()
