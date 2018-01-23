#pragma once

#include <ostream>
#include <sstream>
#include "constants.hpp"
#include "util.hpp"

namespace hlt {
    struct Location {
        double pos_x, pos_y;
		
		Location() { }
		
		Location(double x, double y) {
			pos_x = x;
			pos_y = y;
		}
		
		Location normalize() const {
			double norm = std::sqrt(pos_x * pos_x + pos_y * pos_y);
			return norm ? *this / norm : Location(0, 0);
		}
		
        double get_distance_to(const Location& target) const {
            const double dx = pos_x - target.pos_x;
            const double dy = pos_y - target.pos_y;
            return std::sqrt(dx*dx + dy*dy);
        }

        double dist(const Location& target) const {
            const double dx = pos_x - target.pos_x;
            const double dy = pos_y - target.pos_y;
            return std::sqrt(dx*dx + dy*dy);
        }

        double dist_sq(const Location& target) const {
            const double dx = pos_x - target.pos_x;
            const double dy = pos_y - target.pos_y;
            return dx*dx + dy*dy;
        }

        int orient_towards_in_deg(const Location& target) const {
            return util::angle_rad_to_deg_clipped(orient_towards_in_rad(target));
        }

        double orient_towards_in_rad(const Location& target) const {
            const double dx = target.pos_x - pos_x;
            const double dy = target.pos_y - pos_y;

            return std::atan2(dy, dx) + 2 * M_PI;
        }
		
		double get_angle(const Location& target) const {
			return orient_towards_in_rad(target);
		}

        Location get_closest_point(const Location& target, const double target_radius) const {
            const double radius = target_radius + constants::MIN_DISTANCE_FOR_CLOSEST_POINT;
            const double angle_rad = target.orient_towards_in_rad(*this);

            const double x = target.pos_x + radius * std::cos(angle_rad);
            const double y = target.pos_y + radius * std::sin(angle_rad);

            return { x, y };
        }
		
		const Location operator/(const double d) const {
			Location nloc(pos_x / d, pos_y / d);
			return nloc;
		}

		const Location operator*(const double d) const {
			Location nloc(pos_x * d, pos_y * d);
			return nloc;
		}

		const Location operator+(const Location loc) const {
			Location nloc(pos_x + loc.pos_x, pos_y + loc.pos_y);
			return nloc;
		}

		const Location operator-(const Location loc) const {
			Location nloc(pos_x - loc.pos_x, pos_y - loc.pos_y);
			return nloc;
		}

		Location& operator/=(const double d) {
			pos_x /= d;
			pos_y /= d;
			return *this;
		}

		Location& operator*=(const double d) {
			pos_x *= d;
			pos_y *= d;
			return *this;
		}

		Location& operator+=(const Location loc) {
			pos_x += loc.pos_x;
			pos_y += loc.pos_y;
			return *this;
		}

		Location& operator-=(const Location loc) {
			pos_x -= loc.pos_x;
			pos_y -= loc.pos_y;
			return *this;
		}
		
		bool operator==(const Location& loc) {
			return pos_x == loc.pos_x && pos_y == loc.pos_y;
		}
		
		bool operator!=(const Location& loc) {
			return pos_x != loc.pos_x || pos_y != loc.pos_y;
		}
		
		std::string tostring() const {
			std::ostringstream oss;
			oss << "{" << pos_x << "," << pos_y << "}";
			return oss.str();
		}

        friend std::ostream& operator<<(std::ostream& out, const Location& location);
    };

    static bool operator==(const Location& l1, const Location& l2) {
        return l1.pos_x == l2.pos_x && l1.pos_y == l2.pos_y;
    }
	
}
