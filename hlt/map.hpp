#pragma once

#include "map.hpp"
#include "types.hpp"
#include "ship.hpp"
#include "planet.hpp"

namespace hlt {
    class Map {
    public:
        int map_width, map_height;
		
		int my_id;

        std::unordered_map<PlayerId, std::vector<Ship>> ships;
        std::unordered_map<PlayerId, entity_map<unsigned int>> ship_map;
		
        std::vector<Planet> planets;
        entity_map<unsigned int> planet_map;

		Map() { };
		
        Map(int width, int height);

        const Ship& get_ship(const PlayerId player_id, const EntityId ship_id) const {
            return ships.at(player_id).at(ship_map.at(player_id).at(ship_id));
        }

        Ship& get_my_ship(EntityId ship_id) {
            return ships[my_id][ship_map[my_id][ship_id]];
        }

        const Planet& get_planet(const EntityId planet_id) const {
            return planets.at(planet_map.at(planet_id));
        }
		
		bool in_bounds(const Location& loc) {
			return loc.pos_x > 0 && loc.pos_x < map_width && loc.pos_y > 0 && loc.pos_y < map_height;
		}
		
		bool in_ex_bounds(const Location& loc, double d) {
			return loc.pos_x > d && loc.pos_x < map_width - d && loc.pos_y > d && loc.pos_y < map_height - d;
		}
    };
}
