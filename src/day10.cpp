#include "advent2018.h"

namespace {

struct Point {
	int32_t x, y, dx, dy;
	Point(std::array<int32_t, 4> val) :
		x(val[0]), y(val[1]), dx(val[2]), dy(val[3])
	{
	}
};

}

void day10(input_t input) {
	// Character recognition
	std::unordered_map<uint64_t, char> glyphs = {
		{ 0x861861fe186148c, 'A' },
		{ 0x7e186185f86185f, 'B' },
		{ 0x7a104104104185e, 'C' },
		{ 0xfc104105f04107f, 'E' },
		{ 0x04104105f04107f, 'F' },
		{ 0xbb1861e4104185e, 'G' },
		{ 0x86186187f861861, 'H' },
		{ 0x391450410410438, 'J' },
		{ 0x8512450c3149461, 'K' },
		{ 0xfc1041041041041, 'L' },
		{ 0x871c69a659638e1, 'N' },
		{ 0x04104105f86185f, 'P' },
		{ 0x86145125f86185f, 'R' },
		{ 0x86149230c492861, 'X' },
		{ 0xfc104210842083f, 'Z' },
	};

	std::vector<Point> V;

	std::array<int32_t, 4> val;
	for (int32_t n = 0, neg = 0, have = 0, i = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			have = 1;
			continue;
		}
		neg |= (*input.s == '-');
		if (have) {
			val[i++] = n - neg ^ -neg;
			n = neg = have = 0;
			if (i == 4) {
				V.emplace_back(val);
				i = 0;
			}
		}
	}

	// Find points that belong to the top/bottom rows of the text
	int32_t dy_down = INT_MIN, dy_up = INT_MAX;
	int32_t  y_down = INT_MIN,  y_up = INT_MAX;
	for (auto&& p : V) {
		if (p.dy < dy_up) {
			dy_up = p.dy;
			y_up = p.y;
		} else if (p.dy == dy_up) {
			y_up = std::min(y_up, p.y);
		}
		if (p.dy > dy_down) {
			dy_down = p.dy;
			y_down = p.y;
		} else if (p.dy == dy_down) {
			y_down = std::max(y_down, p.y);
		}
	}

	// Find when those points are likely to be in the correct position
	auto part2 = 1 + (y_up - y_down) / (dy_down - dy_up);

	int32_t bb_up = INT_MAX, bb_left = INT_MAX;
	for (auto&& p : V) {
		p.x += part2 * p.dx;
		p.y += part2 * p.dy;
		bb_left = std::min(bb_left, p.x);
		bb_up   = std::min(bb_up,   p.y);
	}

	// Construct a bitmap of the points in their final positions
	std::array<uint64_t, 8> text = { };
	for (auto&& p : V) {
		p.x -= bb_left;
		p.y -= bb_up;
		text[(p.x / 8) % 8] |= uint64_t(1) << (6 * p.y + p.x % 8);
	}

	// Recognize the letters
	std::string part1;
	for (auto&& n : text) {
		part1 += glyphs[n];
	}

	printf("Day 10 Part 1: %s\nDay 10 Part 2: %u\n", part1.c_str(), part2);
}
