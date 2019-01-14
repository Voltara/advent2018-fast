#include "advent2018.h"

static constexpr int SAFE = 10000;

namespace {

struct Point {
	int x, y;
	Point() : x(), y() { }
	Point(int x, int y) : x(x), y(y) { }
	bool operator < (const Point &o) const {
		return (y != o.y) ? (y < o.y) : (x < o.x);
	}
	Point operator + (const Point &o) const {
		return { x + o.x, y + o.y };
	}
	Point operator - (const Point &o) const {
		return { x - o.x, y - o.y };
	}
	Point min(const Point &o) const {
		return { std::min(x, o.x), std::min(y, o.y) };
	}
	Point max(const Point &o) const {
		return { std::max(x, o.x), std::max(y, o.y) };
	}
};

}

void day06(input_t input) {
	std::vector<Point> P;

	for (int n = 0, have = 0, which = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			have = 1;
		} else if (have) {
			if ((which = !which)) {
				P.emplace_back(n, 0);
			} else {
				P.back().y = n;
			}
			n = have = 0;
		}
	}

	// Get the bounding box for the points
	Point bbox_min{INT_MAX,INT_MAX}, bbox_max{INT_MIN,INT_MIN};
	for (auto&& p : P) {
		bbox_min = bbox_min.min(p);
		bbox_max = bbox_max.max(p);
	}
	bbox_max = bbox_max + Point{1,1}; // Right-open interval

	// Translate the coordinates relative to the bounding box
	// and compute the vector sum of all points
	Point sum;
	for (auto& p : P) {
		p = p - bbox_min;
		sum = sum + p;
	}
	bbox_max = bbox_max - bbox_min;
	bbox_min = { 0, 0 };

	/* Part 1 */
	struct Cell {
		int16_t id;
		int16_t distance;
	};
	std::vector<Cell> G(bbox_max.x * bbox_max.y);
	std::vector<Point> frontier = P, next;
	for (int i = 0; i < P.size(); i++) {
		auto&& p = P[i];
		G[p.y * bbox_max.x + p.x].id = i + 1;
	}

	auto expand = [&next](const Cell &cur, Cell &neighbor, int x, int y, int d) {
		if (neighbor.id) {
			if (neighbor.id != cur.id) {
				if (neighbor.distance == d) {
					neighbor.id = -1;
				}
			}
		} else {
			neighbor.id = cur.id;
			neighbor.distance = d;
			next.emplace_back(x, y);
		}
	};

	for (int dist = 1; !frontier.empty(); dist++) {
		for (auto &&p : frontier) {
			auto idx = p.y * bbox_max.x + p.x;
			auto &cur = G[idx];
			if (p.y > 0)              expand(cur, G[idx - bbox_max.x], p.x, p.y - 1, dist);
			if (p.x > 0)              expand(cur, G[idx - 1],          p.x - 1, p.y, dist);
			if (p.x < bbox_max.x - 1) expand(cur, G[idx + 1],          p.x + 1, p.y, dist);
			if (p.y < bbox_max.y - 1) expand(cur, G[idx + bbox_max.x], p.x, p.y + 1, dist);
		}
		frontier.swap(next);
		next.clear();
	}

	std::vector<int> T(P.size() + 2);
	int idx = 0;
	for (int x = 0; x < bbox_max.x; x++) {
		T[G[idx++].id + 1] = INT_MIN;
	}
	for (int y = 2; y < bbox_max.y; y++) {
		T[G[idx++].id + 1] = INT_MIN;
		for (int x = 2; x < bbox_max.x; x++) {
			T[G[idx++].id + 1]++;
		}
		T[G[idx++].id + 1] = INT_MIN;
	}
	for (int x = 0; x < bbox_max.x; x++) {
		T[G[idx++].id + 1] = INT_MIN;
	}

	int part1 = 0;
	for (auto&& t : T) {
		part1 = std::max(part1, t);
	}

	/* Part 2 */

	const auto compute_distance_sums =
		[](std::vector<int> &v, int n_points, int sum)
	{
		int distance_sum = sum; // Current sum of distances
		int delta = n_points;   // Number of points >= this coord
		for (auto&& k : v) {
			/* There is a lot going on here:
			 * - Read the number of points at the current coord
			 * - Update the distance delta (those points have
			 *   transitioned from "moving toward" to "moving away")
			 * - Write the current distance_sum the vector
			 * - Decrease the distance_sum by the new delta
			 */
			delta -= std::exchange(k, distance_sum) * 2;
			distance_sum -= delta;
		}
		// Invariant: delta == -n_points

		// Pad the vector with values up to the SAFE distance
		for (int k = v.back() + n_points; k < SAFE; k += n_points) {
			v.push_back(k);
		}
		for (int k = v.front() + n_points; k < SAFE; k += n_points) {
			v.push_back(k);
		}

		// Sort by increasing distance sum
		std::sort(v.begin(), v.end());
	};

	// Count how many points are in each row/column
	std::vector<int> x_dist(bbox_max.x), y_dist(bbox_max.y);
	for (auto&& p : P) {
		x_dist[p.x]++;
		y_dist[p.y]++;
	}

	// Compute distance sums separately for the x and y coordinate
	compute_distance_sums(x_dist, P.size(), sum.x);
	compute_distance_sums(y_dist, P.size(), sum.y);

	/* For each x-coordinate, find how many y-coordinates are
	 * within the SAFE distance
	 */
	int part2 = 0;
	size_t y_idx = y_dist.size() - 1;
	for (auto x : x_dist) {
		while (y_dist[y_idx] + x >= SAFE) {
			if (y_idx-- == 0) {
				goto done_part2;
			}
		}
		part2 += y_idx + 1;
	}
done_part2:

	printf("Day 06 Part 1: %d\nDay 06 Part 2: %d\n", part1, part2);
}
