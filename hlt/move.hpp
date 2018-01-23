#pragma once

#include <string>
#include "types.hpp"
#include "util.hpp"

namespace hlt {
    enum class MoveType {
        Noop = 0,
        Thrust,
        Dock,
        Undock,
    };
	
	enum class Evasion {
		None = 0,
		Small = 1,
		Medium = 2,
		Full = 3,
	};

    struct Move {
        MoveType type;
        EntityId ship_id;
        double move_thrust;
        double move_angle_deg;
        EntityId dock_to;
		
		double value = 0;
		bool bait = false;
		int attackers = 0;
		
		Evasion evasion = Evasion::None;
		bool evade = false;
		
		
		Location velocity;
		bool velocity_cached = false;
		
		Move() { }
		
		Move(MoveType _type, EntityId _ship_id, double _move_thrust, double _move_angle_deg, EntityId _dock_to) {
			type = _type;
			ship_id = _ship_id;
			move_thrust = _move_thrust;
			move_angle_deg = _move_angle_deg;
			dock_to = _dock_to;
		}
		
		static Location get_velocity(double ang, double thrust) {
			return thrust ? Location(thrust * cos(ang), thrust * sin(ang)) : Location(0, 0);
		}
		
		static Location get_velocity_deg(double ang, double thrust) {
			return Move::get_velocity(ang / 180.0 * M_PI, thrust);
		}
		
		Location get_velocity() {
			if (velocity_cached) return velocity;
			velocity = Move::get_velocity(move_angle_deg / 180.0 * M_PI, move_thrust);
			velocity_cached = true;
			return velocity;
		}
		
		std::string tostring() {
			return "P: " + get_velocity().tostring() + " V: " + std::to_string(value);
		}
		
        static Move noop() {
			return Move(MoveType::Noop, 0, 0, 0, 0);
        }

        static Move dock(const EntityId ship_id, const EntityId dock_to) {
            return Move(MoveType::Dock, ship_id, 0, 0, dock_to);
        }

        static Move undock(const EntityId ship_id) {
            return Move(MoveType::Undock, ship_id, 0, 0, 0);
        }

        static Move stop(const EntityId ship_id) {
            return Move(MoveType::Thrust, ship_id, 0, 0, 0);
        }

        static Move thrust(const EntityId ship_id, const double thrust, const double angle_deg) {
            return Move(MoveType::Thrust, ship_id, thrust, angle_deg, 0);
        }

        static Move thrust_rad(const EntityId ship_id, const double thrust, const double angle_rad) {
            return Move(MoveType::Thrust, ship_id, thrust, util::angle_rad_to_deg_clipped(angle_rad), 0);
        }
    };
}
