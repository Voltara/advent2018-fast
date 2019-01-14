#include "advent2018.h"

namespace {

/* The 1000x1000 fabric is represented as a bitmap 1024 units wide
 * (16x 64-bit integers.)  Although claims are at most 29 units wide,
 * they can straddle two 64-bit integers.
 */
struct claim_t {
	uint64_t mask0, mask1;
	uint16_t id, y, h;
	uint8_t idx;
	bool part2;

	claim_t(std::array<uint16_t, 5> &a) :
		id(a[0]), y(a[2]), h(a[4]), part2(true)
	{
		auto x = a[1], w = a[3];
		idx = x / 64;
		int shift = x % 64;
		uint64_t mask = (uint64_t(1) << w) - 1;
		mask0 = mask << shift;
		mask1 = shift ? (mask >> (64 - shift)) : 0;
	}

	bool operator < (const claim_t &o) const {
		return y > o.y;
	}
};

}

void day03(input_t input) {
	std::vector<claim_t> v;
	v.reserve(1500);

	std::array<uint16_t, 5> arr;
	for (int n = 0, have = 0, i = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			have = 1;
		} else if (have) {
			arr[i++] = n;
			if (i == 5) {
				v.emplace_back(arr);
				i = 0;
			}
			n = have = 0;
		}
	}

	// Sort by starting y-index
	std::sort(v.begin(), v.end());

	int part1 = 0, part2 = 0;

	/* Sweep vertically down the fabric, one row at a time,
	 * checking for collisions between claims.
	 */
	std::array<uint64_t, 17> row, collide;
	while (!v.empty()) {
		row.fill(0);
		collide.fill(0);

		auto y = v.back().y;
		for (auto vi = v.rbegin(); vi != v.rend() && vi->y == y; vi++) {
			collide[vi->idx + 0] |= row[vi->idx + 0] & vi->mask0;
			row[vi->idx + 0] |= vi->mask0;
			collide[vi->idx + 1] |= row[vi->idx + 1] & vi->mask1;
			row[vi->idx + 1] |= vi->mask1;
		}

		for (auto vi = v.rbegin(); vi != v.rend() && vi->y == y; vi++) {
			if (vi->part2) {
				if ((collide[vi->idx + 0] & vi->mask0) ||
				    (collide[vi->idx + 1] & vi->mask1))
				{
					vi->part2 = false;
				}
			}

			vi->y++;
			if (--vi->h == 0) {
				/* Reached the bottom of this claim; check for
				 * part2 solution and remove the claim
				 */
				if (vi->part2) {
					part2 = vi->id;
				}
				std::swap(*vi, v.back());
				v.pop_back();
			}
		}

		// Just tally up the 1-bits for part1
		for (auto c : collide) {
			part1 += _mm_popcnt_u64(c);
		}
	}

	printf("Day 03 Part 1: %d\nDay 03 Part 2: %d\n", part1, part2);
}
