#pragma once

#include <string>
#include "location.hpp"
#include "types.hpp"
#include "move.hpp"

namespace hlt {
    struct Entity {
        EntityId entity_id;
        PlayerId owner_id;
        Location location;
		Move move = Move::noop();
        int health;
        double radius;

        bool is_alive() const {
            return health > 0;
        }
		
		Location destination() {
			return location + move.get_velocity();
		}
		
		std::string tostring() {
			return "[ENT] " + std::to_string((entity_id)) + " " + location.tostring() + "," + std::to_string(radius) + "," + move.tostring();
		}
    };
}
