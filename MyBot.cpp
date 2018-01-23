// Author: Psyho
// Blog: http://psyho.gg/
// Twitter: https://twitter.com/fakepsyho

#include "hlt/hlt.hpp"
#include "hlt/collision.hpp"
#include <bits/stdc++.h>
#include <sys/time.h>

using namespace std;
using namespace hlt;
using namespace hlt::collision;
using namespace hlt::constants;
using namespace hlt::util;

const bool LOG_MOVES = false;
const bool GIVE_UP_LOSING = false;	

#define INLINE   inline __attribute__ ((always_inline))
#define NOINLINE __attribute__ ((noinline))

#define ALIGNED __attribute__ ((aligned(16)))

#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define SSELOAD(a)     _mm_load_si128((__m128i*)&a)
#define SSESTORE(a, b) _mm_store_si128((__m128i*)&a, b)

#define FOR(i,a,b)  for(int i=(a);i<(b);++i)
#define REP(i,a)    FOR(i,0,a)
#define ZERO(m)     memset(m,0,sizeof(m))
#define ALL(x)      x.begin(),x.end()
#define PB          push_back
#define S           size()
#define byte        unsigned char
#define LL          long long
#define ULL         unsigned long long
#define LD          long double
#define MP          make_pair
#define X           first
#define Y           second
#define VC          vector
#define PII         pair<int, int>
#define PDD         pair<double, double>
#define VI          VC<int>
#define VVI         VC<VI>
#define VVVI        VC<VVI>
#define VPII        VC<PII>
#define VVPII       VC<VPII>
#define VVVPII      VC<VVPII>
#define VD          VC<double>
#define VVD         VC<VD>
#define VVVD        VC<VVD>
#define VPDD        VC<PDD>
#define VVPDD       VC<VPDD>
#define VVVPDD      VC<VVPDD>
#define VS          VC<string>
#define VVS         VC<VS>
#define VVVS        VC<VVS>
#define DB(a)       cerr << #a << ": " << (a) << endl;

#define INF         1e9

template<class A, class B> ostream& operator<<(ostream &os, pair<A,B> &p) {os << "(" << p.X << "," << p.Y << ")"; return os;}
template<class A, class B, class C> ostream& operator<<(ostream &os, tuple<A,B,C> &p) {os << "(" << get<0>(p) << "," << get<1>(p) << "," << get<2>(p) << ")"; return os;}
template<class T> ostream& operator<<(ostream &os, VC<T> &v) {os << "{"; REP(i, v.S) {if (i) os << ", "; os << v[i];} os << "}"; return os;}
template<class T> ostream& operator<<(ostream &os, set<T> &s) {VC<T> vs(ALL(s)); return os << vs;}
template<class A, class B> ostream& operator<<(ostream &os, map<A, B> &m) {VC<pair<A,B>> vs; for (auto &x : m) vs.PB(x); return os << vs;}
template<class T> string i2s(T x) {ostringstream o; if (floor(x) == x) o << (int)x; else o << x; return o.str();}
VS splt(string s, char c = ' ') {VS all; int p = 0, np; while (np = s.find(c, p), np >= 0) {all.PB(s.substr(p, np - p)); p = np + 1;} all.PB(s.substr(p)); return all;}

void log(string s) {Log::log(s);}

double getTime() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec * 1e-6;
}

struct Timer {
	double total = 0;
	double startTime;
	bool running = false;
	string name = "";
	
	Timer(string _name = "") {
		name = _name;
	}
	
	void start() {
		startTime = getTime();
		running = true;
	}
	
	void stop() {
		total += getTime() - startTime;
		running = false;
	}
	
	double elapsed() {
		return total + (running ? getTime() - startTime : 0);
	}
	
	string tostring() {
		return "Timer" + (name.S ? string(" ") + name : string("")) + ": " + i2s(elapsed());
	}
};

VC<Timer*> timerPool;
#define ADD_TIMER(name) Timer timer##name(#name); timerPool.PB(&timer##name);
Timer timerX;
Timer timerY;
Timer timerZ;

struct RNG {
    unsigned int MT[624];
    int index;
	
	RNG(int seed = 1) {
		init(seed);
	}
    
    void init(int seed = 1) {
        MT[0] = seed;
        FOR(i, 1, 624) MT[i] = (1812433253UL * (MT[i-1] ^ (MT[i-1] >> 30)) + i);
        index = 0;
    }
    
    void generate() {
        const unsigned int MULT[] = {0, 2567483615UL};
        REP(i, 227) {
            unsigned int y = (MT[i] & 0x8000000UL) + (MT[i+1] & 0x7FFFFFFFUL);
            MT[i] = MT[i+397] ^ (y >> 1);
            MT[i] ^= MULT[y&1];
        }
        FOR(i, 227, 623) {
            unsigned int y = (MT[i] & 0x8000000UL) + (MT[i+1] & 0x7FFFFFFFUL);
            MT[i] = MT[i-227] ^ (y >> 1);
            MT[i] ^= MULT[y&1];
        }
        unsigned int y = (MT[623] & 0x8000000UL) + (MT[0] & 0x7FFFFFFFUL);
        MT[623] = MT[623-227] ^ (y >> 1);
        MT[623] ^= MULT[y&1];
    }
    
    unsigned int rand() {
        if (index == 0) {
            generate();
        }
        
        unsigned int y = MT[index];
        y ^= y >> 11;
        y ^= y << 7  & 2636928640UL;
        y ^= y << 15 & 4022730752UL;
        y ^= y >> 18;
        index = index == 623 ? 0 : index + 1;
        return y;
    }
    
    INLINE int next() {
        return rand();
    }
    
    INLINE int next(int x) {
        return rand() % x;
    }
    
    INLINE int next(int a, int b) {
        return a + (rand() % (b - a));
    }
    
    INLINE double nextDouble() {
        return (rand() + 0.5) * (1.0 / 4294967296.0);
    }
	
    INLINE double nextDouble(double a, double b) {
        return nextDouble() * (b - a) + a;
    }
};

RNG rng(1);

PlayerId player_id;
PlayerId opp_player_id;
bool leftSide = false;
Map m;

map<EntityId, Location> shipTarget;
map<EntityId, double> damageReceived;
map<EntityId, bool> attacked;
map<EntityId, int> planetShipTurn;
map<EntityId, map<EntityId, double>> pathBonus;
map<EntityId, double> closestEnemyShip;
map<EntityId, int> shipSafety;
map<EntityId, int> maxFollowers;
map<EntityId, int> enemyClusters;
map<EntityId, int> enemyFollowers;
map<EntityId, int> planetMyShips;
map<EntityId, int> planetEnemyShips;
map<EntityId, double> closestEnemy;
map<int, EntityId> aggressiveShips;
map<EntityId, double> planetValue;
map<EntityId, double> shipValue;
bool holdCorners[2][2];
map<EntityId, bool> shipUsed;
map<int, int> minerDefenders;
map<int, Ship*> enemyShips;
VC<pair<Location, PlayerId>> spawnedShips;
int turn = 0;
int players_no = 2;

const int MASK_PLANETS = 1;
const int MASK_ALLIES = 2;
const int MASK_ALLIES_DOCKED = 4;
const int MASK_ENEMIES = 8;
const int MASK_ENEMIES_DOCKED = 16;
const int MASK_ALL = 31;

VC<Entity> findCloseEntities(Location loc, double radius, int mask = MASK_ALL, EntityId ship_id = -1) {
	VC<Entity> rv;
	if (mask & MASK_PLANETS) for (Planet& p : m.planets) if (loc.dist(p.location) < radius + p.radius) rv.PB(p);
	if (mask & MASK_ALLIES) for (Ship& s : m.ships[player_id]) if (s.entity_id != ship_id && loc.dist(s.location) < radius && !s.docked()) rv.PB(s);
	if (mask & MASK_ALLIES_DOCKED) for (Ship& s : m.ships[player_id]) if (s.entity_id != ship_id && loc.dist(s.location) < radius && s.docked()) rv.PB(s);
	if (mask & MASK_ENEMIES) for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& s : ps.Y) if (!s.docked() && loc.dist(s.location) < radius) rv.PB(s);
	if (mask & MASK_ENEMIES_DOCKED) for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& s : ps.Y) if (s.docked() && loc.dist(s.location) < radius) rv.PB(s);
	return rv;
}

int detectCollisions(const Location& start, const Location& end, VC<Entity>& entities, double radius = 0.5001, bool earlyExit = true) {
	int rv = m.in_bounds(end) ? 0 : 1;
	if (rv && earlyExit) return 1;
	for (auto& e : entities) 
		if (segment_circle_intersect(start, end, e, radius)) {
			if (earlyExit) return 1;
			rv++;
		}
	return rv;
}

int detectCollisions(const Location& start, double ang, double thrust, VC<Entity>& entities, double radius = 0.5001, bool earlyExit = true) {
	Location end = start + Move::get_velocity(ang, thrust);
	return detectCollisions(start, end, entities, radius, earlyExit);
}

VI getSpawningTimes(Planet &p, int max_turns = 20) {
	VI docked;
	for (EntityId& eid : p.docked_ships) {
		const Ship& ship = m.get_ship(p.owner_id, eid);
		if (ship.docking_status == ShipDockingStatus::Docked) {
			docked.PB(0);
		} else if (ship.docking_status == ShipDockingStatus::Docking) {
			docked.PB(ship.docking_progress - 1);
		}
	}
	
	if (docked.empty()) return VI();
		
	VI rv;
	int v = p.current_production;
	FOR(t, 1, max_turns+1) {
		REP(i, docked.S) {
			if (docked[i]) {
				docked[i]--;
			} else {
				v += BASE_PRODUCTIVITY;
			}
		}
		if (v >= 72) {
			rv.PB(t);
			v -= 72;
		}
	}
	return rv;
}

Location getSpawningPoint(Planet& p) {
	Location bloc(-1e9, -1e9);
	Location origin(m.map_width / 2.0, m.map_height / 2.0);
	VC<Entity> entities = findCloseEntities(p.location, p.radius + 10, MASK_ALL);
	FOR(dx, -SPAWN_RADIUS, +SPAWN_RADIUS + 1) FOR(dy, -SPAWN_RADIUS, +SPAWN_RADIUS + 1) {
		double offset_angle = atan2(dy, dx);
		double offset_x = dx + p.radius * cos(offset_angle);
		double offset_y = dy + p.radius * sin(offset_angle);
		auto loc = p.location + Location(offset_x, offset_y);
		
		bool collision = false;
		for (Entity& e : entities) if (loc.dist(e.location) < SHIP_RADIUS * 4) {
			collision = true;
			break;
		}
		if (collision) continue;
		if (origin.dist_sq(loc) < origin.dist_sq(bloc)) bloc = loc;
	}
	return bloc;
}



double round_angle(double rad) {
	return angle_rad_to_deg_clipped(rad) / 180.0 * M_PI;
}

Location velocity[360][8];

void initVelocity() {
	REP(deg, 360) REP(thrust, 8) {
		velocity[deg][thrust] = Move::get_velocity(deg * M_PI / 180.0, thrust);
	}
}

bool earlyPhase = true;
bool late = false;
Move move_to(const Ship& ship, const Location& target, double maxSpeed = MAX_SPEED, double radius = 0, bool avoidEnemies = false, bool cluster = false, bool avoidEdges = false, int baitTarget = -1, bool reversePlayers = false) {
	radius *= radius;
	VC<Entity> planets = findCloseEntities(ship.location, 10.0, MASK_PLANETS);
	VC<Entity> docked = findCloseEntities(ship.location, 10.0, MASK_ALLIES_DOCKED);
	VC<Entity> entities = findCloseEntities(ship.location, 16.0, MASK_PLANETS | MASK_ALLIES | MASK_ALLIES_DOCKED | MASK_ENEMIES_DOCKED, ship.entity_id);
	VC<Entity> enemies = avoidEnemies ? findCloseEntities(ship.location, 25.0, MASK_ENEMIES) : VC<Entity>();
	VC<Entity> miners = findCloseEntities(ship.location, 13.0, MASK_ENEMIES_DOCKED);
	// VC<Entity> allies = cluster || reversePlayers ? findCloseEntities(ship.location, 30.0, MASK_ALLIES | MASK_ALLIES_DOCKED, ship.entity_id) : VC<Entity>();
	VC<Entity> allies = cluster ? findCloseEntities(ship.location, 30.0, MASK_ALLIES | MASK_ALLIES_DOCKED) : VC<Entity>();
	VC<Location> spawnedEnemies;
	Entity baitEnemy;
	if (reversePlayers) {
		allies = VC<Entity>();
		docked = VC<Entity>();
		miners = findCloseEntities(ship.location, 13.0, MASK_ALLIES_DOCKED);
		enemies = findCloseEntities(ship.location, 25.0, MASK_ALLIES);
		for (auto& e : entities) e.move.move_thrust = 0;
	}
	if (avoidEnemies) {
		for (Entity& enemy : enemies) {
			enemy.move.move_thrust = MAX_SPEED;
			enemy.move.move_angle_deg = util::angle_rad_to_deg_clipped(enemy.location.orient_towards_in_rad(ship.location));
		}
		if (!reversePlayers) {
			for (auto& spawn : spawnedShips) {
				if (spawn.Y != player_id && ship.location.dist(spawn.X) < 25.0) spawnedEnemies.PB(spawn.X);
			}
		}
		
		if (baitTarget != -1) {
			for (auto& e : enemies) if (e.entity_id == baitTarget) baitEnemy = e;
			if (baitEnemy.entity_id != baitTarget) baitTarget = -1;
		}
	}
	
	double ang = ship.location.get_angle(target);
	
	int TOTAL_ANG_STEPS = 360;
	
	double bv = 1e9;
	double bang = 0;
	double bd2 = 0;
	double bthrust = -1;
	int battackers = 0;
	bool bbait = false;
	
	// if (radius == 0 && !avoidEnemies && !detectCollisions(ship.location, round_angle(ang), (int)maxSpeed, entities)) return Move::thrust_rad(ship.entity_id, (int)maxSpeed, round_angle(ang));
	
	const double DANGER_DIST = baitTarget == -1 ? 6.25 : 6.25;
	int angoff = angle_rad_to_deg_clipped(ang);
	
	for (int xang = 0; xang < 360; xang += 360 / TOTAL_ANG_STEPS) {
		int nang = (xang + angoff) % 360;
		REP(nthrust, maxSpeed + 1e-9) {
			if (nthrust == 0 && xang) continue;
			Location end = ship.location + velocity[nang][nthrust];
			double d2 = target.dist_sq(end);
			double av = d2;
			if (radius) av = av < radius ? radius - av : 10000 + av;
			
			if (enemies.S == 0 && allies.S == 0 && av >= bv) continue;
			
			if (detectCollisions(ship.location, end, entities)) continue;
			
			bool closeToPlanet = false;
			for (auto& p : planets) {
				if (end.dist_sq(p.location) < (p.radius + 1) * (p.radius + 1)) {
					closeToPlanet = true;
					break;
				}
			}
			if (closeToPlanet) continue;
			
			if (!avoidEnemies && !m.in_ex_bounds(end, 4)) continue;
			
			bool abait = false;
			int aattackers = 0;
			
			if (avoidEnemies) {
				double minDist = 1e9;
				int closeEnemies = 0;
				if (true || baitTarget == -1) {
					if (enemies.S) {
						for (Entity& e : enemies) {
							if (e.entity_id == baitTarget) continue;
							double d = segment_circle_dist(ship.location, end, e);
							minDist = min(minDist, d);
							closeEnemies += d < DANGER_DIST;
						}
						for (auto& loc : spawnedEnemies) {
							minDist = min(minDist, end.dist(loc));
							closeEnemies += end.dist(loc) < DANGER_DIST;
						}
					}
				} else {
					for (Entity& e : enemies) {
						if (e.entity_id == baitTarget) continue;
						double d = end.dist(e.location) - MAX_SPEED;
						minDist = min(minDist, d);
						closeEnemies += d < DANGER_DIST;
					}
					for (auto& loc : spawnedEnemies) {
						minDist = min(minDist, end.dist(loc));
						closeEnemies += end.dist(loc) < DANGER_DIST;
					}
				}
				aattackers = closeEnemies;
				
				if (minDist < 1e5) {
					av = 10000;
					if (d2 >= radius) {
						av += 10000;
						av += sqrt(d2);
					} else {
						// if (minDist < 6.25) av += 10000;
						av += sqrt(radius) - sqrt(d2);
					}
					if (minDist < DANGER_DIST) {
						double dockDist = 1e9;
						for (auto& e : docked) dockDist = min(dockDist, end.dist_sq(e.location));
						if (dockDist > 4 * 4)
							av += closeEnemies * 20000;
					}
					double maxDist = 9.0 + (4 - ship.health / 64.0) * 1.0;
					av -= min(maxDist, minDist) * 4;
				}
				
				if (true || avoidEdges) {
					double edgeXDist = min(end.pos_x, m.map_width - end.pos_x);
					double edgeYDist = min(end.pos_y, m.map_height - end.pos_y);
					av += max(0.0, 12 - edgeXDist);
					av += max(0.0, 12 - edgeYDist);
				}
				
				if (baitTarget != -1 && segment_circle_dist(ship.location, end, baitEnemy) < 5.5) {
					av -= 1000;
					abait = true;
				}
				
				if (allies.S) {
					av -= sqrt(d2);
					for (Entity& e : allies) {
						av += 17.5 * sqrt(max(1.0, end.dist(e.destination()) + 0.0));
					}
				}
			}
			
			if (radius || avoidEnemies) {
				for (Entity& e : miners)
					if (end.dist_sq(e.location) < 6 * 6) {
						av -= 2;
						// break;
					}
			}
			
			if (avoidEnemies && earlyPhase && players_no == 4) {
				av += (leftSide ? max(0.0, end.pos_x - 50) : max(0.0, m.map_width - end.pos_x - 50)) * 0.025;
			}
			
			if (av < bv) {
				bd2 = d2;
				bbait = abait;
				battackers = aattackers;
				
				bv = av;
				bang = nang;
				bthrust = nthrust;
			}
		}
	}
	
	// if (reversePlayers) {
		// log("Ang: " + i2s(bang) + " Thrust: " + i2s(bthrust));
		// Location end = ship.location + Move::get_velocity_deg(bang, bthrust);
		// log("Vel: " + Move::get_velocity_deg(bang, bthrust).tostring());
		// log("End: " + end.tostring());
		// log("BV: " + i2s(bv) + " BD2: " + i2s(bd2));
		// for (Entity& entity : entities) {
			// log("Rad: " + i2s(entity.radius) + " Pos: " + entity.location.tostring() + " DistS: " + i2s(entity.location.dist(ship.location)) + " DistE: " + i2s(entity.location.dist(end)) + " Close: " + i2s(segment_circle_dist(ship.location, end, entity.location)));
		// }
		// for (auto& e : enemies) {
			// log("Enemy: " + i2s((int)e.entity_id) + " Pos: " + e.location.tostring() + " DistSO: " + i2s(ship.location.dist(e.location)) + " Close: " + i2s(segment_circle_dist(ship.location, end, e)));
		// }
	// }
	
	Move mv = Move::thrust(ship.entity_id, bthrust, bang);
	mv.value = bv;
	mv.bait = bbait;
	mv.attackers = battackers;
	return mv;
}

enum OrderType {
	Unknown = 0,
	Invalid = 1,
	Colonize = 2,
	AttackShip = 3,
	AttackMiner = 4,
	DefendMiner = 5,
	HitAndRun = 6,
	Hide = 7,
	Undock = 8,
};

struct Order {
	OrderType type;
	double value;
	Move move;
	EntityId ship;
	EntityId planet;
	EntityId target;
	int cornerx;
	int cornery;
	
	Order() {
		type = OrderType::Unknown;
		value = -INF;
		move = Move::noop();
		ship = -1;
		target = -1;
		planet = -1;
		cornerx = -1;
		cornery = -1;
	}
	
	string tostring() {
		string s = "[Order] Ship: " + i2s((int)ship) + " Value: " + i2s(value) + " Type: ";
		if (type == OrderType::Unknown) {
			s += "Unknown";
		} else if (type == OrderType::Invalid) {
			s += "Invalid";
		} else if (type == OrderType::Colonize) {
			s += "Colonize Planet: " + i2s((int)planet);
		} else if (type == OrderType::AttackShip) {
			s += "AttackShip";
		} else if (type == OrderType::AttackMiner) {
			s += "AttackMiner";
		} else if (type == OrderType::DefendMiner) {
			s += "DefendMiner";
		} else if (type == OrderType::HitAndRun) {
			s += "HitAndRun";
		} else if (type == OrderType::Hide) {
			s += "Hide At: " + i2s(cornerx) + "," + i2s(cornery);
		} else if (type == OrderType::Undock) {
			s += "Undock";
		}
		return s;
	}
};

Order colonizeOrder(const Ship& ship, const Planet& planet, bool delay = false) {
	Order o;
	o.type = OrderType::Colonize;
	o.ship = ship.entity_id;
	o.planet = planet.entity_id;
	
	if (planetEnemyShips[planet.entity_id] > 0) return o;
	int spots_left = planet.docking_spots - planetMyShips[planet.entity_id];
	if (planetShipTurn[planet.entity_id] < (ship.location.dist(planet.location) - 4 - planet.radius) / 7.0) spots_left--;
	if (spots_left <= 0) return o;
	
	double v = 0;
	if (ship.can_dock(planet)) {
		v = 2500 + closestEnemy[ship.entity_id] + planetMyShips[planet.entity_id];
		// if (shipSafety[ship.entity_id] >= 2) v += 2000 + 100 * shipSafety[ship.entity_id];
		if (shipSafety[ship.entity_id] >= (players_no == 2 ? 2 : 1)) v += 2000;
		if (delay) v = -10000;
	} else {
		v = 2000 - ship.location.dist(planet.location) + planet.radius + planetValue[planet.entity_id] + (turn < 20 ? planetMyShips[planet.entity_id] * 2 : 0);
	}
	o.value = v;
	
	return o;
}

int totalAttackMiners = 0;
int totalHitAndRunners = 0;

Order attackOrder(const Ship& ship, const Ship& eship, EntityId eid) {
	Order o;
	// if (enemyFollowers[eid] >= maxFollowers[enemyClusters[eid]]) return o;
	if (enemyFollowers[eid] >= maxFollowers[eid]) return o;
	
	double dist = ship.location.dist(eship.location);
	// double dist = ship.location.dist(shipTarget[eid]);
	double v = 1000 - dist;
	if (dist < 25 || dist < 45 && (m.in_ex_bounds(eship.location, 10.0) || eship.docked())) v += 2000;
	if (aggressiveShips.count(eid)) v += 15;
	// if (eship.owner_id == opp_player_id && turn < 60) v += 15;
	if (eship.docked()) {
		v += 40;
		v += pathBonus[ship.entity_id][eship.entity_id];
	}
	v += shipValue[eid];
	// if (m.ships[player_id].S >= 10 && totalHitAndRunners == 0 && eship.docking_status != ShipDockingStatus::Undocked) v += 10000;
	
	if (false && v > 10000) {
		o.type = OrderType::HitAndRun;
	} else if (false && aggressiveShips.count(eid) && minerDefenders[aggressiveShips[eid]] <= 1) {
		o.type = OrderType::DefendMiner;
	} else if (eship.docking_status == ShipDockingStatus::Undocked) {
		o.type = OrderType::AttackShip;
	} else {
		// o.type = totalHitAndRunners == 0 ? OrderType::HitAndRun : OrderType::AttackMiner;
		// o.type = OrderType::HitAndRun;
		o.type = OrderType::AttackMiner;
	}
	o.ship = ship.entity_id;
	o.target = eid;
	o.value = v;
	return o;
}

Order hideOrder(const Ship& ship, int cx, int cy) {
	Location loc(cx == 0 ? 1 : m.map_width - 1, cy == 0 ? 1 : m.map_height - 1);
	double dist = ship.location.get_distance_to(loc);
	
	Order o;
	o.ship = ship.entity_id;
	o.cornerx = cx;
	o.cornery = cy;
	o.type = OrderType::Hide;
	o.value = 5000 - dist;
	return o;
}

void recalcOrder(Order& o) {
	const Ship& ship = m.get_ship(player_id, o.ship);
	if (o.type == OrderType::Colonize) {
		o = colonizeOrder(ship, m.planets[o.planet]);
	} else if (o.type == OrderType::AttackShip || o.type == OrderType::AttackMiner || o.type == OrderType::DefendMiner || o.type == OrderType::HitAndRun) {
		Ship eship = *enemyShips[o.target];
		o = attackOrder(ship, eship, o.target);
	} else if (o.type == OrderType::Hide) {
		o = hideOrder(ship, o.cornerx, o.cornery);
	} else {
		assert(false);
	}
}

Move execOrder(Order &o) {
	const Ship& ship = m.get_ship(player_id, o.ship);
	
	if (o.type == OrderType::Colonize) {
		Planet& planet = m.planets[o.planet];
		if (ship.can_dock(planet)) {
			Move mv = Move::dock(ship.entity_id, planet.entity_id);
			mv.evasion = Evasion::None;
			return mv;
		} else {
			Move mv = move_to(ship, planet.location);
			mv.evasion = Evasion::None;
			return mv;
		}
		
	} else if (o.type == OrderType::AttackShip) {
		Ship eship = *enemyShips[o.target];
		double dist = ship.location.get_distance_to(eship.location);
		Location loc;
		loc = ship.location + (eship.location - ship.location).normalize() * max(MAX_SPEED, eship.location.dist(ship.location));
		if (!m.in_ex_bounds(loc, 4)) loc = ship.location + (eship.location - ship.location).normalize() * 1000;
		if (shipTarget[eship.entity_id] != eship.location)
			loc = shipTarget[eship.entity_id];
		double aces = 1e9;
		for (auto &e : findCloseEntities(ship.location, 10.0, MASK_ALLIES, ship.entity_id)) aces = min(aces, closestEnemyShip[e.entity_id]);
		double ces = closestEnemyShip[ship.entity_id];
		double maxSpeed = MAX_SPEED;
		if (maxFollowers[eship.entity_id] > 2 && ces < 20.0 && aces < 25.0 && ces < aces) maxSpeed = max(3.0, min(MAX_SPEED, 7.0 - (aces - ces) / 3));
		Move mv = move_to(ship, loc, maxSpeed);
		// Move mv = move_to(ship, eship.location, MAX_SPEED);
		mv.evasion = Evasion::Small;
		return mv;
		
	} else if (o.type == OrderType::AttackMiner) {
		Ship eship = *enemyShips[o.target];
		double dist = ship.location.get_distance_to(eship.location);
		
		double aces = 1e9;
		for (auto &e : findCloseEntities(ship.location, 10.0, MASK_ALLIES, ship.entity_id)) aces = min(aces, closestEnemyShip[e.entity_id]);
		double ces = closestEnemyShip[ship.entity_id];
		double maxSpeed = MAX_SPEED;
		if (maxFollowers[eship.entity_id] > 2 && ces < 20.0 && aces < 25.0 && ces < aces) maxSpeed = max(3.0, min(MAX_SPEED, 7.0 - (aces - ces) / 3));
		
		Move mv = move_to(ship, eship.location, maxSpeed, 5.05);
		mv.evasion = Evasion::Medium;
		return mv;
		
	} else if (o.type == OrderType::HitAndRun) {
		Ship eship = *enemyShips[o.target];
		double dist = ship.location.get_distance_to(eship.location);
		Move mv = move_to(ship, eship.location, MAX_SPEED, 5.05, true);
		mv.evasion = Evasion::None;
		return mv;
		
	// } else if (o.type == OrderType::DefendMiner) {
		// Ship aship = m.get_ship(player_id, aggressiveShips[o.target]);
		// double dist = ship.location.get_distance_to(aship.location);
		// Move mv = Move::stop(ship.entity_id);
		// REP(tries, 10) {
			// auto nav = move_to(ship, aship.location + Location(rng.nextDouble(-2, 2), rng.nextDouble(-2, 2)), max(0.0, min(dist + 1.25, MAX_SPEED)));
			// if (nav.Y) {
				// mv = nav.X;
				// break;
			// }
		// }
		// mv.evasion = Evasion::None;
		// return mv;		
		
	} else if (o.type == OrderType::Hide) {
		Location loc = Location(o.cornerx == 0 ? 1 : m.map_width - 1, o.cornery == 0 ? 1 : m.map_height - 1);
		double dist = ship.location.get_distance_to(loc);
		Move mv = move_to(ship, loc, min(dist, MAX_SPEED), 0.0, true);
		mv.evasion = Evasion::None;
		return mv;
		
	} else if (o.type == OrderType::Undock) {
		Move mv = Move::undock(ship.entity_id);
		return mv;
		
	} else {
		assert(false);
	}
}

int main() {
	ADD_TIMER(Parse);
	ADD_TIMER(PreProcess);
	ADD_TIMER(Orders);
	ADD_TIMER(Moves);
	ADD_TIMER(Evasion);
	ADD_TIMER(Bait);
	ADD_TIMER(FindEntities);
	ADD_TIMER(MoveTo);
	
	initVelocity();
	
	ios_base::sync_with_stdio(false);
    const Metadata metadata = initialize("Psyho");
    player_id = metadata.player_id;

    const Map& initial_map = metadata.initial_map;
	
	VC<Planet> oplanets = initial_map.planets;

    ostringstream initial_map_intelligence;
    initial_map_intelligence
            << "width: " << initial_map.map_width
            << "; height: " << initial_map.map_height
            << "; players: " << initial_map.ship_map.size()
            << "; my ships: " << initial_map.ship_map.at(player_id).size()
            << "; planets: " << initial_map.planets.size();
    log(initial_map_intelligence.str());

	turn = 0;
	while (true) {
		turn++;
		
		timerParse.start();
        m = in::get_map();
		m.my_id = player_id;
		timerParse.stop();
		Log::clear();
		
		double startTime = getTime();
		
		players_no = m.ships.S;
		if (turn == 1) {
			opp_player_id = player_id;
			for (auto& ps : m.ships) if (ps.X != player_id && ps.Y.S && (ps.Y[0].location.pos_x < m.map_width / 2) == (m.ships[player_id][0].location.pos_x < m.map_width / 2))
				opp_player_id = ps.X;
			if (m.ships[player_id].S)
				leftSide = m.ships[player_id][0].location.pos_x < m.map_width / 2;
		}
		
		timerPreProcess.start();
		log("[Phase Start] Preprocess");
		
		//Calc attacked && damageReceived
		attacked.clear();
		damageReceived.clear();
		for (auto& ps : m.ships) for (Ship& s : ps.Y) {
			//Doesn't work when enemy ship is docked
			if (s.docked()) continue;
			VC<Entity> vs = findCloseEntities(s.location, 6.0, MASK_ALLIES | MASK_ALLIES_DOCKED | MASK_ENEMIES | MASK_ENEMIES_DOCKED);
			VI targets;
			for (Entity& e : vs) if (e.owner_id != s.owner_id) targets.PB(e.entity_id);
			
			if (targets.S == 0) continue;
			attacked[s.entity_id] = true;
			double dmg = 64.0 / targets.S;
			for (int eid : targets) damageReceived[eid] += dmg;
		}
		
		//Calc earlyPhase
		if (earlyPhase) {
			for (Ship& s : m.ships[player_id]) {
				if (s.docked()) {
					earlyPhase = false;
					break;
				}
			}
		}
		
		//Calc planetShipSoon
		planetShipTurn.clear();
		for (Planet& p : m.planets) {
			planetShipTurn[p.entity_id] = 1e9;
			if (!p.owned) continue;
			if (findCloseEntities(p.location, 60.0, MASK_ENEMIES).S) continue;
			VI turns = getSpawningTimes(p);
			if (turns.S == 0) continue;
			planetShipTurn[p.entity_id] = turns[0];
		}
		
		//Calc closestEnemyShip
		closestEnemyShip.clear();
		for (Ship& s : m.ships[player_id]) {
			closestEnemyShip[s.entity_id] = 1e9;
			for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& es : ps.Y)
				closestEnemyShip[s.entity_id] = min(closestEnemyShip[s.entity_id], s.location.dist(es.location));
		}

		//calc shipSafety
		shipSafety.clear();
		map<EntityId, VI> pSpawningTimes;
		map<EntityId, Location> pSpawningPoints;
		for (Planet& p : m.planets) {
			pSpawningTimes[p.entity_id] = getSpawningTimes(p, 10);
			pSpawningPoints[p.entity_id] = getSpawningPoint(p);
		}
		for (Ship& s : m.ships[player_id]) {
			VC<pair<double, bool>> ships;
			for (auto& ps : m.ships) for (Ship& os : ps.Y) {
				if (s.entity_id == os.entity_id) continue;
				if (os.docked()) continue;
				double dist = s.location.dist(os.location);
				if (dist < 70)
					ships.PB(MP(dist + (ps.X == player_id ? 10 : 0), ps.X == player_id));
			}
			for (Planet& p : m.planets) {
				double d2sp = s.location.dist(pSpawningPoints[p.entity_id]);
				for (int t : pSpawningTimes[p.entity_id]) {
					double d = d2sp + (t + 1) * MAX_SPEED;
					if (d < 70) ships.PB(MP(d + (p.owned ? 10 : 0), p.owned));
				}
			}
			sort(ALL(ships));
			int safety = 100;
			int cur = 0;
			for (auto& p : ships) {
				if (p.Y) {
					cur++;
				} else {
					cur--;
					safety = min(cur, safety);
				}
			}
			if (ships.S == 0 && findCloseEntities(s.location, 45.0, MASK_ENEMIES_DOCKED).S > 0) safety = 0;
			shipSafety[s.entity_id] = safety;
		}
		
		//Calc spawnedShips
		spawnedShips.clear();
		for (Planet& p : m.planets) {
			int docked = 0;
			if (p.owner_id != -1) {
				for (EntityId& eid : p.docked_ships) {
					const Ship& ship = m.get_ship(p.owner_id, eid);
					docked += ship.docking_status == ShipDockingStatus::Docked || ship.docking_status == ShipDockingStatus::Docking && ship.docking_progress == 1;
				}
			}
			if (docked * BASE_PRODUCTIVITY + p.current_production >= 72)
				spawnedShips.PB(MP(getSpawningPoint(p), p.owner_id));
		}		
			
		//calc closestEnemy
		closestEnemy.clear();
		for (Ship& s : m.ships[player_id]) {
			closestEnemy[s.entity_id] = 1e9;
			for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& es : ps.Y)
				closestEnemy[s.entity_id] = min(closestEnemy[s.entity_id], s.location.dist(es.location));
		}				
		
		//Calc maxFollowers & enemyClusters
		maxFollowers.clear();
		enemyClusters.clear();
		for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& s : ps.Y) {
			int v = turn < 60 ? 3 : players_no == 4 ? 5 : 7;
			// if (s.docked()) v = 5;
			bool alone = true;
			for (Ship& s2 : ps.Y) if (s.entity_id != s2.entity_id && s.location.dist_sq(s2.location) < 25 * 25) alone = false;
			if (alone && !s.docked()) v = 2;
			maxFollowers[s.entity_id] = v;
			enemyClusters[s.entity_id] = alone ? 1 : 2;
		}
		
		//Calc shipValue
		shipValue.clear();
		for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& s : ps.Y) {
			double v = 0;
			for (auto& ps : m.ships) if (ps.X != player_id && ps.X != s.owner_id) for (Ship& s2 : ps.Y) {
				double d = s.location.dist(s2.location);
				if (d < 50) v -= 1.5 + 2 * (50 - d) / 50;
			}
			int totalDefenders = 0;
			for (Ship& s2 : m.ships[s.owner_id]) {
				if (s2.entity_id == s.entity_id) continue;
				if (s2.docked()) continue;
				totalDefenders += s.location.dist_sq(s2.location) < 10 * 10;
			}
			// if (s.docked()) v -= min(5, totalDefenders) * 2;
			v -= min(5, totalDefenders) * 2;
			// if (s.docked() && getSpawningPoint(m.planets[s.docked_planet]).dist(s.location) < 6) v -= 4;
			v += Location(m.map_width / 2.0, m.map_height / 2.0).dist(s.location) * 0.2;
			shipValue[s.entity_id] = v;
		}		
		
		//Create enemyShips
		enemyShips.clear();
		for (auto& ps : m.ships) for (Ship& ship : ps.Y) enemyShips[ship.entity_id] = &ship;
		
		//Calc aggressiveShips
		aggressiveShips.clear();
		for (Ship& ship : m.ships[player_id]) if (ship.docking_status != ShipDockingStatus::Undocked) {
			for (auto ps : m.ships) if (ps.X != player_id) for (Ship& eship : ps.Y) if (eship.docking_status == ShipDockingStatus::Undocked) {
				double dist = ship.location.get_distance_to(eship.location);
				if (dist < 25) {
					int eid = eship.entity_id;
					if (aggressiveShips.count(eid) == 0 || dist < m.get_ship(player_id, aggressiveShips[eid]).location.get_distance_to(eship.location))
						aggressiveShips[eid] = ship.entity_id;
				}
			}
		}
		
		//Calc shipTarget
		shipTarget.clear();
		for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& ship : ps.Y) {
			shipTarget[ship.entity_id] = ship.location;
			if (ship.docked()) continue;
			if (enemyClusters[ship.entity_id] > 1) continue;
			VC<Entity> docked = findCloseEntities(ship.location, 40.0, MASK_ALLIES_DOCKED);
			if (docked.S == 0) continue;
			Location target(-10000, -10000);
			for (auto& s : docked) {
				double d = ship.location.dist_sq(s.location);
				if (d < ship.location.dist_sq(target)) target = s.location;
			}
			Move emv = move_to(ship, target, MAX_SPEED, 0, true, false, false, -1, true);
			if (emv.move_thrust < 0) continue;
			shipTarget[ship.entity_id] = ship.location + emv.get_velocity();
		}
		
		//Calc planetMyShips & planetEnemyShips
		planetMyShips.clear();
		planetEnemyShips.clear();
		for (auto& ps : m.ships) for (Ship& ship : ps.Y) {
			if (ship.docking_status == ShipDockingStatus::Undocked) continue;
			EntityId planetId = ship.docked_planet;
			if (ps.X == player_id) {
				planetMyShips[planetId]++;
			} else {
				planetEnemyShips[planetId]++;
			}
		}
		
		//Calc planetValue
		planetValue.clear();
		Location centerMass;
		int planetsOwned = 0;
		for (Planet& p : m.planets) if (p.owned) {
			centerMass += p.location;
			planetsOwned++;
		}
		if (planetsOwned) centerMass /= planetsOwned;
		
		for (Planet& p : m.planets) {
			double v = 0;
			
			double dist_from_origin = Location(m.map_width / 2, m.map_height / 2).dist(p.location);
			// v += dist_from_origin * (players_id == 4 ? 0.1 : -0.25);
			v += dist_from_origin * (players_no == 4 ? 0.4 : 0);
			
			// double dist_to_closest_planet = 1e9;
			// for (Planet& p2 : m.planets) if (p.entity_id != p2.entity_id) dist_to_closest_planet = min(dist_to_closest_planet, p.location.dist(p2.location));
			// v += dist_to_closest_planet * (players_no == 4 ? 0 : -0.2);
			
			double dist_from_mass_center = planetsOwned ? p.location.dist(centerMass) : 0.0;
			if (players_no == 4) v -= dist_from_mass_center * 0.1;
			
			if (players_no == 4 && dist_from_origin < 30) v -= 25;
			
			v += p.docking_spots * 3;
			
			// double dist_to_enemy = 1e9;
			// for (Planet& p2 : m.planets) if (planetEnemyShips[p2.entity_id] > 0)
				// dist_to_enemy = min(dist_to_enemy, p.location.dist(p2.location));
			// if (dist_to_enemy > 1e6) dist_to_enemy = 0;
			// v += dist_to_enemy * 0.15;
			
			planetValue[p.entity_id] = v;
		}
		
		//Calc goingDown
		bool goingDown = false;
		ZERO(holdCorners);
		int totalShips = 0;
		for (auto& ps : m.ships) totalShips += ps.Y.S;
		if (players_no > 2 && turn > 40 && m.ships[player_id].S < totalShips * 0.1) goingDown = true;
		
		//Calc pathBonus
		pathBonus.clear();
		for (Ship& ship : m.ships[player_id]) for (auto& ps : m.ships) if (ps.X != player_id) for (Ship& eship : ps.Y) {
			break;
			if (ship.docked()) continue;
			if (!eship.docked()) continue;
			Location loc = ship.location + (eship.location - ship.location).normalize() * min(7.5, eship.location.dist(ship.location) / 2);
			Location eloc = eship.location + (ship.location - eship.location).normalize() * min(7.5, eship.location.dist(ship.location) / 2);
			double v = 10;
			for (auto& ps2 : m.ships) if (ps.X != player_id) for (Ship& s : ps.Y) {
				if (s.docked()) continue;
				// double d = segment_circle_dist(ship.location, eship.location, s.location);
				double d = segment_circle_dist(loc, eloc, s.location);
				if (d < 15) v -= 8 / (2 + d);
			}
			pathBonus[ship.entity_id][eship.entity_id] = v;
		}
		timerPreProcess.stop();		
		
		//Calc noColonize
		static bool noColonize = false;
		bool delayColonize = false;
		static int noColonizeTurn = 0;
		if (turn < 20) {
			double maxShipDist = 0;
			bool allUndocked = true;
			REP(i, m.ships[player_id].S) {
				REP(j, i) maxShipDist = max(maxShipDist, m.ships[player_id][i].location.dist(m.ships[player_id][j].location));
				allUndocked &= m.ships[player_id][i].docking_status == ShipDockingStatus::Undocked;
			}
			for (auto& ps : m.ships) if (ps.X != player_id) {
				double maxEnemyDist = 0;
				double minEnemyDist = 1e9;
				for (Ship& eship : ps.Y) {
					double d = 0;
					for (Ship& ship : m.ships[player_id]) {
						maxEnemyDist = max(maxEnemyDist, ship.location.dist(eship.location));
						d = max(d, ship.location.dist(eship.location));
					}
					minEnemyDist = min(minEnemyDist, d);
				}
				int docking_progress = 6;
				bool enemyUndocked = true;
				for (Ship& eship : ps.Y) if (eship.docked()) {
					docking_progress = min(docking_progress, eship.docking_progress);
					enemyUndocked = false;
				}
				// if (allUndocked && maxShipDist < 10 && turn < 20 && m.ships[player_id].S == 3 && maxEnemyDist < 35 + docking_progress * 8)
				// if (allUndocked && maxShipDist < 10 && turn < 20 && m.ships[player_id].S == 3 && maxEnemyDist < 43 + docking_progress * 7)
				if (allUndocked && turn < 20 && m.ships[player_id].S == 3 && maxEnemyDist < 43 + docking_progress * 7 + (players_no == 2 ? 10 : 0))
					if (!noColonize) {
						log("No Colonize!!!");
						noColonize = true;
						noColonizeTurn = turn;
					}
				if (allUndocked && turn < 20 && enemyUndocked && m.ships[player_id].S == 3 && ps.Y.S == 3 && minEnemyDist < 80) {
					delayColonize = true;
				}
			}
		}
		
		//Generate Orders
		timerOrders.start();
		shipUsed.clear();
		enemyFollowers.clear();
		minerDefenders.clear();
		VC<Order> orders;
		VC<Order> so(m.ships[player_id].S);
		log("[Phase Start] Orders");
		
		totalAttackMiners = 0;
		totalHitAndRunners = 0;
		int ordersSaved = 0;
		int ordersCalced = 0;
		while (true) {
			bool late = getTime() - startTime > 1.0;
			
			Order bo;
			bo.type = OrderType::Invalid;
			bo.value = 0;
			
			REP(i, m.ships[player_id].S) {
				Ship& ship = m.ships[player_id][i];
				if (shipUsed[ship.entity_id]) continue;
				if (ship.docked()) continue;
				// if (damageReceived[ship.entity_id] >= ship.health) continue;
				
				if (!late && so[i].value > 0) {
					double prevValue = so[i].value;
					recalcOrder(so[i]);
					if (prevValue == so[i].value) {
						ordersSaved++;
						if (so[i].value > bo.value)
							bo = so[i];
						continue;
					} 
				}
				ordersCalced++;
				so[i] = Order();
				
				for (Planet& planet : m.planets) {
					if (noColonize && turn < noColonizeTurn + 10) continue;
					Order o = colonizeOrder(ship, planet, delayColonize);
					if (o.value > so[i].value)
						so[i] = o;
				}
				
				for (auto ps : m.ships) if (ps.X != player_id) for (Ship& eship : ps.Y) {
					if (damageReceived[eship.entity_id] >= eship.health) continue;
					Order o = attackOrder(ship, eship, eship.entity_id);
					if (o.value > so[i].value)
						so[i] = o;
				}
				
				if (goingDown && closestEnemy[ship.entity_id] < 100) REP(cx, 2) REP(cy, 2) if (!holdCorners[cx][cy]) {
					Order o = hideOrder(ship, cx, cy);
					if (o.value > so[i].value)
						so[i] = o;
				}
				
				if (late && so[i].value > 0) {
					bo = so[i];
					orders.PB(bo);
					shipUsed[bo.ship] = true;
					if (bo.planet >= 0) planetMyShips[bo.planet]++;
					if (bo.target >= 0) enemyFollowers[bo.target]++;
					if (bo.cornerx >= 0) holdCorners[bo.cornerx][bo.cornery] = true;
					if (bo.type == OrderType::DefendMiner) minerDefenders[aggressiveShips[bo.target]]++;
					if (bo.type == OrderType::AttackMiner) totalAttackMiners++;
					if (bo.type == OrderType::HitAndRun) totalHitAndRunners++;
					if (bo.type == OrderType::Colonize && m.get_my_ship(bo.ship).can_dock(m.planets[bo.planet])) {
						for (Ship& s : m.ships[player_id]) shipSafety[s.entity_id]--;
					}
				}
				
				if (so[i].value > bo.value)
					bo = so[i];
			}
			
			if (late || bo.type == OrderType::Invalid)
				break;
			
			
			orders.PB(bo);
			shipUsed[bo.ship] = true;
			if (bo.planet >= 0) planetMyShips[bo.planet]++;
			if (bo.target >= 0) enemyFollowers[bo.target]++;
			if (bo.cornerx >= 0) holdCorners[bo.cornerx][bo.cornery] = true;
			if (bo.type == OrderType::DefendMiner) minerDefenders[aggressiveShips[bo.target]]++;
			if (bo.type == OrderType::AttackMiner) totalAttackMiners++;
			if (bo.type == OrderType::HitAndRun) totalHitAndRunners++;
			if (bo.type == OrderType::Colonize && m.get_my_ship(bo.ship).can_dock(m.planets[bo.planet])) {
				for (Ship& s : m.ships[player_id]) shipSafety[s.entity_id]--;
			}
		}
		timerOrders.stop();
		
		log("Orders Saved: " + i2s(ordersSaved));
		log("Orders Calced: " + i2s(ordersCalced));
		
		// Convert to Hit And Run
		for (Order &o : orders) if (o.type == OrderType::AttackMiner) {
			bool alone = true;
			Location loc = m.get_my_ship(o.ship).location;
			for (Ship &s : m.ships[player_id]) if (s.entity_id != o.ship) {
				alone &= loc.dist_sq(s.location) > 30 * 30;
			}
			if (alone) o.type = OrderType::HitAndRun;
		}
		
		//Undocking
		log("[Phase Start] Undocking");
		for (Ship& s : m.ships[player_id]) {
			if (!(s.docking_status == ShipDockingStatus::Docked || s.docking_status == ShipDockingStatus::Docking && s.docking_progress == 1)) continue;
			if (turn > 20) break;
			bool undock = false;
			for (auto& ps : m.ships) if (ps.X != player_id) {
				bool allUndocked = true;
				double minDist = 1e9;
				double maxDist = 0;
				for (Ship& es : ps.Y) {
					allUndocked &= !es.docked();
					minDist = min(minDist, s.location.dist(es.location));
					maxDist = max(maxDist, s.location.dist(es.location));
				}
				if (allUndocked && maxDist < 59 && minDist < 52) undock = true;
			}
			
			Planet& p = m.planets[s.docked_planet];
			int prod = p.current_production;
			for (EntityId& eid : p.docked_ships) {
				const Ship& ship = m.get_ship(p.owner_id, eid);
				if (ship.docking_status == ShipDockingStatus::Docked || ship.docking_status == ShipDockingStatus::Docking && ship.docking_progress == 1)
					prod += BASE_PRODUCTIVITY;
			}
			
			if (prod >= 72 && prod < 72 + BASE_PRODUCTIVITY) undock = false;
			
			if (undock) {
				m.planets[s.docked_planet].docked_ships.erase(find(ALL(m.planets[s.docked_planet].docked_ships), s.entity_id));
				Order o;
				o.value = 0;
				o.type = OrderType::Undock;
				o.ship = s.entity_id;
				orders.PB(o);
			}
		}
		
		timerMoves.start();
		//Create Moves
		log("[Phase Start] Create Moves");
		VC<Move> moves(orders.S);
		REP(pass, 3) {
			REP(i, orders.S) {
				Order& o = orders[i];
				Move mv = execOrder(o);
				mv.move_thrust = max(0.0, mv.move_thrust);
				moves[i] = mv;
				m.get_my_ship(mv.ship_id).move = mv;
			}
		}
		timerMoves.stop();
		
		//Evasion
		timerEvasion.start();
		log("[Phase Start] Evasion");
		VC<bool> changed(moves.S, true);
		map<EntityId, int> shipToPos;
		REP(i, moves.S)	shipToPos[moves[i].ship_id] = i;
		
		// VI perm(moves.S); REP(i, moves.S) perm[i] = i;
		REP(pass, 6) {
			if (getTime() - startTime > 1.6) break;
			VC<bool> newChanged(moves.S, false);
			// random_shuffle(ALL(perm));
			for (int i = (int)moves.S - 1; i >= 0; i--) {
				// int i = perm[k];
				if (moves[i].type != MoveType::Thrust) continue;
				if (moves[i].evasion == Evasion::None) continue;
				
				Ship& ship = m.get_my_ship(moves[i].ship_id);
				
				Location loc = ship.location;
				Location tloc = ship.destination();
				
				int totalAllies = 0;
				int totalEnemies = 0;
				Location enemiesPos(0, 0);
				
				VC<Entity> allies = findCloseEntities(ship.location, 30.0, MASK_ALLIES, ship.entity_id);
				VC<Entity> dockedAllies = findCloseEntities(ship.location, 25.0, MASK_ALLIES_DOCKED);
				VC<Entity> enemies = findCloseEntities(ship.location, 25.0, MASK_ENEMIES);
				
				bool skip = !changed[i];
				for (Entity& e : allies) if (shipToPos.count(e.entity_id) && changed[shipToPos[e.entity_id]]) {
					skip = false;
					break;
				}
				if (skip) continue;
				
				VC<Entity> expectedEnemies;
				
				for (auto& e : enemies) {
					e.move.move_thrust = min(ship.location.dist(e.location), MAX_SPEED);
					// e.move.move_thrust = MAX_SPEED;
					e.move.move_angle_deg = util::angle_rad_to_deg_clipped(e.location.orient_towards_in_rad(ship.location));
					if (segment_circle_intersect(ship.location, ship.destination(), e, 5.50) || segment_circle_intersect(ship.location, ship.location, e, 5.50)) {
						totalEnemies++;
						enemiesPos += e.location;
						expectedEnemies.PB(e);
					}
				}
				
				for (auto& e : allies) {
					for (auto& es : expectedEnemies) {
						if (segment_circle_intersect(e.location, e.destination(), es, 5.75)) {
							totalAllies++;
							break;
						}
					}
				}
				
				for (auto spawn : spawnedShips) if (spawn.Y == player_id && tloc.dist(spawn.X) < 6.0) totalAllies++;
				
				for (auto& e : dockedAllies) {
					bool good = false;
					good |= ship.destination().dist(e.destination()) < 9;
					for (auto& es : expectedEnemies) {
						good |= segment_circle_intersect(es.location, es.destination(), e, 5.75);
					}
					if (good) totalAllies++;
				}
					
				
				for (auto spawn : spawnedShips) if (spawn.Y != player_id && tloc.dist(spawn.X) < 6.0) {
					totalEnemies++;
					enemiesPos += spawn.X;
				}
				
				if (moves[i].evade || totalEnemies > totalAllies || totalEnemies > 0 && moves[i].evasion == Evasion::Full) {
					
					if (m.ships[player_id].S == 1 && enemies.S == 1 && m.ships[enemies[0].owner_id].S == 1 && (ship.health + 63) / 64 > (enemies[0].health + 63) / 64) 
						continue;
					
					timerMoveTo.start();
					enemiesPos = totalEnemies ? enemiesPos / totalEnemies : ship.location + Location(1, 0);
					Location tloc = ship.location + (ship.location - enemiesPos).normalize() * 1000;
					Move mv = move_to(ship, tloc, MAX_SPEED, 0.0, true, true, m.ships[player_id].S <= 3);
					// log("[Evade] Pass: " + i2s(pass) + " ID: " + i2s(moves[i].ship_id) + " Allies: " + i2s(totalAllies) + " Enemies: " + i2s(totalEnemies) + " Move: " + mv.tostring());
					mv.evade = true;
					mv.evasion = moves[i].evasion;
					if (mv.move_thrust >= 0) {
						newChanged[i] = mv.move_thrust != moves[i].move_thrust || mv.move_angle_deg != moves[i].move_angle_deg;
						moves[i] = mv;
						m.get_my_ship(moves[i].ship_id).move = moves[i];
					}
					timerMoveTo.stop();
				}
			}
			bool diff = false;
			REP(i, newChanged.S) diff |= newChanged[i];
			if (!diff) break;
			changed = newChanged;
		}
		timerEvasion.stop();
		
		//Baiting
		timerBait.start();
		log("[Phase Start] Bait");
		set<EntityId> movesProcessed;
		REP(i, moves.S) {
			if (getTime() - startTime > 1.6) break;
			if (movesProcessed.count(moves[i].ship_id)) continue;
			
			//find group
			const double GROUP_TRANS_DIST = 2.0;
			const double GROUP_MAX_DIST = 5.0;
			VC<Entity> potGroup = findCloseEntities(m.get_my_ship(moves[i].ship_id).location, GROUP_TRANS_DIST + GROUP_MAX_DIST + 1, MASK_ALLIES);
			
			VC<Ship> group;
			set<EntityId> groupVS;
			VC<EntityId> groupQueue;
			groupQueue.PB(moves[i].ship_id);
			while (!groupQueue.empty()) {
				int id = groupQueue.back();
				groupQueue.pop_back();
				if (groupVS.count(id)) continue;
				group.PB(m.get_my_ship(id));
				groupVS.insert(id);
				for (auto& e : potGroup) if (group.back().location.dist_sq(e.location) < GROUP_TRANS_DIST * GROUP_TRANS_DIST && e.entity_id != id)
					groupQueue.PB(e.entity_id);
			}
			
			//check group properties
			bool allEvade = true;
			double maxDist = 0;
			Location groupCenter(0, 0);
			for (auto& e : group) {
				movesProcessed.insert(e.entity_id);
				allEvade &= e.move.evade;
				groupCenter += e.location;
				for (auto &e2 : group) maxDist = max(maxDist, e.location.dist_sq(e2.location));
			}
			maxDist = sqrt(maxDist);
			groupCenter /= group.S;
			
			VC<Entity> enemies = findCloseEntities(groupCenter, 20.0, MASK_ENEMIES);
			
			if (group.S == 1 || !allEvade || maxDist > GROUP_MAX_DIST || enemies.S > 5) continue;
			
			//find bait target
			map<EntityId, Move> om;
			for (auto& ship : group) om[ship.entity_id] = ship.move;
			
			double bv = 0;
			map<EntityId, Move> bm;
			for (auto& e : enemies) {
				map<EntityId, Move> am;
				double av = 0;
				for (auto& ship : group) {
					Move mv = move_to(ship, ship.location, MAX_SPEED, 0.0, true, true, m.ships[player_id].S <= 3, e.entity_id);
					// av += mv.bait * 1100 - mv.attackers * 1e9 - mv.value / 1e6;
					av += mv.bait * 1000 - mv.attackers * 10000;
					am[ship.entity_id] = mv;
					m.get_my_ship(ship.entity_id).move = mv;
					if (mv.move_thrust < 0) av -= 1e9;
				}
				
				if (av > bv) {
					bv = av;
					bm = am;
				}
				
				for (auto& ship : group) ship.move = om[ship.entity_id];
			}
			
			if (bv >= 2000) {
				ostringstream o; o << "[Bait] Ships: " << groupVS << " Value: " << bv;
				log(o.str());
				REP(i, moves.S) if (bm.count(moves[i].ship_id)) {
					moves[i] = bm[moves[i].ship_id];
					m.get_my_ship(moves[i].ship_id).move = moves[i];
				}
			}
		}
		timerBait.stop();
		
		//GiveUp?
		bool giveUp = false;
		giveUp |= GIVE_UP_LOSING && m.ships[player_id].S < totalShips * 0.3;
		if (giveUp)	REP(i, 2) moves.PB(Move::stop(m.ships[player_id][0].entity_id));
		
		//Print Timers
		static bool prevGameEnd = false;
		bool gameEnd = false;
		for (auto& ps : m.ships) if (ps.Y.S >= totalShips * 0.9) gameEnd = true;
		VI ownedPlanets(players_no);
		for (Planet& p : m.planets) if (p.owner_id != -1) ownedPlanets[p.owner_id]++;
		REP(i, players_no) if (ownedPlanets[i] >= m.planets.S - 2) gameEnd = true;
		if (!prevGameEnd && gameEnd) {
			prevGameEnd = true;
			timerPool.PB(&timerX);
			timerPool.PB(&timerY);
			timerPool.PB(&timerZ);
			for (Timer* t : timerPool)
				log(t->tostring());
		}
		
		log("Total Moves: " + i2s(moves.S));
		
		if (LOG_MOVES) {
			REP(i, moves.S) {
				Move& mv = moves[i];
				Order& o = orders[i];
				Location loc = m.get_ship(player_id, mv.ship_id).location;
				log("[Order] " + o.tostring() +  " [Move] Type: " + i2s((int)mv.type) + " Evasion: " + i2s((int)mv.evasion) + " Thrust: " + i2s(mv.move_thrust) + " Angle: " + i2s(mv.move_angle_deg) + " Pos: " + loc.tostring() + " NPos: " + (loc + mv.get_velocity()).tostring());
			}
		}
		log("[Done]");
        if (!out::send_moves(moves)) {
            log("send_moves failed; exiting");
            break;
        }
    }
}
