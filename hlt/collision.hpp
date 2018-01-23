#pragma once

#include <algorithm>

#include "entity.hpp"
#include "location.hpp"

const bool DEBUG_COLLISION = false;

namespace hlt {
    namespace collision {
        static double square(const double num) {
            return num * num;
        }

        static bool segment_circle_intersect(const Location& start, const Location& end, const Location& center, const double radius) {
            const double start_x = start.pos_x;
            const double start_y = start.pos_y;
            const double end_x = end.pos_x;
            const double end_y = end.pos_y;
            const double center_x = center.pos_x;
            const double center_y = center.pos_y;
            const double dx = end_x - start_x;
            const double dy = end_y - start_y;

            const double a = square(dx) + square(dy);

            if (a == 0.0) return start.dist_sq(center) <= square(radius);
			
            const double b =
                    -2 * (square(start_x) - (start_x * end_x)
                    - (start_x * center_x) + (end_x * center_x)
                    + square(start_y) - (start_y * end_y)
                    - (start_y * center_y) + (end_y * center_y));


            const double t = std::max(0.0, std::min(-b / (2 * a), 1.0));

            const double closest_x = start_x + dx * t;
            const double closest_y = start_y + dy * t;
            return Location(closest_x, closest_y).dist_sq(center) <= square(radius);
        }
		
        static double segment_circle_dist(const Location& start, const Location& end, const Location& center) {
            const double start_x = start.pos_x;
            const double start_y = start.pos_y;
            const double end_x = end.pos_x;
            const double end_y = end.pos_y;
            const double center_x = center.pos_x;
            const double center_y = center.pos_y;
            const double dx = end_x - start_x;
            const double dy = end_y - start_y;

            const double a = square(dx) + square(dy);

            if (a == 0.0) return start.dist(center);
			
            const double b =
                    -2 * (square(start_x) - (start_x * end_x)
                    - (start_x * center_x) + (end_x * center_x)
                    + square(start_y) - (start_y * end_y)
                    - (start_y * center_y) + (end_y * center_y));

            const double t = std::max(0.0, std::min(-b / (2 * a), 1.0));

            const double closest_x = start_x + dx * t;
            const double closest_y = start_y + dy * t;
            return Location(closest_x, closest_y).dist(center);
        }
		
        static bool segment_circle_intersect(const Location& start, const Location& end, Entity& entity, const double fudge) {
			return segment_circle_intersect(start, end - entity.move.get_velocity(), entity.location, entity.radius + fudge);
		}
		
        static double segment_circle_dist(const Location& start, const Location& end, Entity& entity) {
			return segment_circle_dist(start, end - entity.move.get_velocity(), entity.location);
		}
    }
}
