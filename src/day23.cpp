#include "advent2018.h"

namespace {

// https://en.wikipedia.org/wiki/Q*bert#/media/File:Qbert.png
struct QBot {
	std::array<int64_t, 4> v;
	int64_t r;
	QBot() : v(), r() { }
	QBot(int64_t x, int64_t y, int64_t z, int64_t r) :
		v({ -x+y+z, x-y+z, x+y-z, x+y+z }), r(r) { }
	int32_t in_range(QBot &q) const {
		return labs(q.v[0] - v[0]) <= r &
		       labs(q.v[1] - v[1]) <= r &
		       labs(q.v[2] - v[2]) <= r &
		       labs(q.v[3] - v[3]) <= r;
	}
};

using Range = std::pair<int64_t, int64_t>;

struct Region {
	// Inclusive ranges
	std::array<Range, 4> R;

	Region(
			const std::array<std::vector<int64_t>, 4>& splits,
			std::array<Range, 4> &idx)
		: R()
	{
		for (int i = 0; i < 4; i++) {
			R[i] = {
				splits[i][idx[i].first],
				splits[i][idx[i].second] - 1
			};
		}
	}
	int64_t distance_to_origin() const {
		/* The coordinates of the closest point are
		 * either all odd or all even.  Solve each case
		 * separately, and choose the better outcome.
		 */
		Region Ra0 = *this, Ra1 = *this;

		/* Adjust the ranges so that Ra0 is all even, and Ra1
		 * is all odd.  Mark a case bad if this is not possible.
		 */
		bool bad0 = false, bad1 = false;
		for (size_t i = 0; i < 4; i++) {
			auto&& r = R[i];

			if (r.first & 1) {
				bad0 |= (r.first == r.second);
				Ra0.R[i].first++;
			} else {
				bad1 |= (r.first == r.second);
				Ra1.R[i].first++;
			}

			if (r.second & 1) {
				Ra0.R[i].second--;
			} else {
				Ra1.R[i].second--;
			}
		}

		auto best_dist = INT64_MAX;
		if (!bad0) best_dist = std::min(best_dist, Ra0.solve_distance());
		if (!bad1) best_dist = std::min(best_dist, Ra1.solve_distance());
		return best_dist;
	}

    private:
	/* Find a solution for (N0 + N1 + N2 == N3) that minimizes the
	 * distance to origin, i.e. highest absolute value of N0..N3
	 *
	 * Boundary values for all ranges must be uniformly even or odd.
	 */
	int64_t solve_distance() const {
		auto H = R;

		// Put ranges R[0..2] in absolute value order
		for (int i = 0; i < 3; i++) {
			auto& r = H[i];
			if (labs(r.second) < labs(r.first)) {
				std::swap(r.first, r.second);
			}
		}

		// Get initial value for the sum (N0 + N1 + N2)
		auto sum = H[0].first + H[1].first + H[2].first;
		int64_t delta = 0;
		if (sum < H[3].first) {
			delta = H[3].first - sum;  // Too low
		} else if (sum > H[3].second) {
			delta = H[3].second - sum; // Too high
			H[3].first = H[3].second;
		} else {
			H[3].first = sum;          // Just right
		}

		// Get initial distance to origin
		int64_t dist = 0;
		for (auto&& r : H) {
			dist = std::max(dist, labs(r.first));
		}

		while (delta) {
			int64_t candidates = 0;
			for (int i = 0; i < 3; i++) {
				// Adjust H[0..2] as far as possible without affecting
				// distance to origin
				int64_t step = 0;
				if (delta > 0) {
					if (H[i].second <= 0) continue; // wrong direction
					auto slack = std::min( dist, H[i].second) - H[i].first;
					step = std::min(slack, delta);
				} else {
					if (H[i].second >= 0) continue; // wrong direction
					auto slack = std::max(-dist, H[i].second) - H[i].first;
					step = std::max(slack, delta);
				}
				H[i].first += step;
				delta -= step;
				// Keep track of how many variables still have adjustment room
				if (H[i].first != H[i].second) {
					candidates++;
				}
			}
			if (delta) {
				if (!candidates) {
					// No solution
					return INT64_MAX;
				}
				/* Increase distance-to-origin by the minimum amount which
				 * would satisfy the remaining delta, assuming all candidates
				 * have sufficient slack.  (This may underestimate, in which
				 * case it will loop again with a smaller candidate count.)
				 * Distance must be raised in even increments.
				 */
				dist += (((labs(delta) / 2) + (candidates - 1)) / candidates) * 2;
			}
		}

		return dist;
	}
};

}

static int64_t solve_part2(const std::array<std::vector<int64_t>, 4>& splits,
		const std::vector<QBot>& Q)
{
	/* Coordinate ranges under consideration */
	std::array<Range, 4> idx = {
		Range{0, splits[0].size() - 1},
		Range{0, splits[1].size() - 1},
		Range{0, splits[2].size() - 1},
		Range{0, splits[3].size() - 1}
	};

	/* Bit mask that keeps track of which QBots are still in range
	 * at each depth.  Each level recursion consumes one bit, so this
	 * limits the supported input size (15,000 bots should be safe.)
	 */
	std::vector<uint64_t> mask(Q.size(), 1);

	size_t best_intersection = 0;
	int64_t best_distance = INT64_MAX;

	std::function<void(int,size_t,uint64_t,uint64_t)> recur =
		[&](int axis, size_t valid, uint64_t bit, uint64_t next_bit) {
			bool ok = false;

			// Choose the next axis to split
			for (int i = 0; i < 4; i++, axis = (axis + 1) % 4) {
				ok = (idx[axis].first + 1 < idx[axis].second);
				if (ok) {
					break;
				}
			}

			// Leaf node?
			if (!ok) {
				// Find distance to origin (INT64_MAX means
				// there are no valid points in the region)
				Region region(splits, idx);
				auto dist = region.distance_to_origin();
				if (dist != INT64_MAX) {
					if (best_intersection < valid) {
						best_intersection = valid;
						best_distance = dist;
					} else {
						best_distance = std::min(best_distance, dist);
					}
				}
				return;
			}

			// Split the axis
			auto range = idx[axis];
			auto mid = range.first + (range.second - range.first) / 2;
			auto threshold = splits[axis][mid];

			// Find which QBots are still valid on each side of the split
			auto valid_low = 0, valid_high = 0;
			for (size_t i = 0; i < Q.size(); i++) {
				auto& q = Q[i];
				auto& m = mask[i];

				auto is_valid = m & bit;
				m &= ~(bit | next_bit);
				if (is_valid) {
					// Does it intersect the low side?
					if (q.v[axis] - q.r < threshold) {
						m |= bit;
						valid_low++;
					}
					// Does it intersect the high side?
					if (q.v[axis] + q.r >= threshold) {
						m |= next_bit;
						valid_high++;
					}
				}
			}

			// Search the more promising side first
			if (valid_low > valid_high) {
				// Low first
				if (valid_low >= best_intersection) {
					idx[axis].second = mid;
					recur((axis + 1) % 4, valid_low,  bit,      next_bit << 1);
					idx[axis].second = range.second;
				}
				if (valid_high >= best_intersection) {
					idx[axis].first = mid;
					recur((axis + 1) % 4, valid_high, next_bit, next_bit << 1);
					idx[axis].first = range.first;
				}
			} else {
				// High first
				if (valid_high >= best_intersection) {
					idx[axis].first = mid;
					recur((axis + 1) % 4, valid_high, next_bit, next_bit << 1);
					idx[axis].first = range.first;
				}
				if (valid_low >= best_intersection) {
					idx[axis].second = mid;
					recur((axis + 1) % 4, valid_low,  bit,      next_bit << 1);
					idx[axis].second = range.second;
				}
			}
		};

	recur(0, Q.size(), 1, 2);

	return best_distance;
}

void day23(input_t input) {
	std::vector<QBot> Q;

	std::array<int64_t, 4> A;
	for (int64_t n = 0, have = 0, neg = 0, i = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			have = 1;
			continue;
		}
		neg |= (*input.s == '-');
		if (have) {
			A[i++] = n - neg ^ -neg;
			have = neg = n = 0;
			if (i == 4) {
				Q.emplace_back(A[0], A[1], A[2], A[3]);
				i = 0;
			}
		}
	}

	// Part 1
	QBot bigboy;
	for (auto&& q : Q) {
		if (q.r > bigboy.r) {
			bigboy = q;
		}
	}
	int32_t part1 = 0;
	for (auto&& q : Q) {
		part1 += bigboy.in_range(q);
	}

	// Part 2
	std::array<std::vector<int64_t>, 4> splits;
	splits.fill({0}); // Avoid zero-crossing leaf nodes
	for (auto&& q : Q) {
		for (int i = 0; i < 4; i++) {
			splits[i].push_back(q.v[i] - q.r);
			splits[i].push_back(q.v[i] + q.r + 1);
		}
	}

	// Sort the splits for each axis and remove duplicates
	for (auto&& s : splits) {
		std::sort(s.begin(), s.end());
		s.erase(std::unique(s.begin(), s.end()), s.end());
	}

	int64_t part2 = solve_part2(splits, Q);

	printf("Day 23 Part 1: %d\nDay 23 Part 2: %ld\n", part1, part2);
}
