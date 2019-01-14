#include "advent2018.h"

void day05(input_t input) {
	std::vector<char> P(1, -1), Q;

	P.reserve(input.len);
	Q.reserve(input.len);

	while (input.len-- && input.s[input.len] < 'A') { }
	for (input.len++; input.len--; input.s++) {
		if ((*input.s ^ P.back()) == 0x20) {
			P.pop_back();
		} else {
			P.push_back(*input.s);
		}
	}

	size_t part1 = P.size(), part2 = part1;

	for (int ignore = 1; ignore <= 26; ignore++) {
		Q.clear();
		for (auto s : P) {
			if ((s & 31) == ignore) {
				// ignore
			} else if ((s ^ Q.back()) == 0x20) {
				Q.pop_back();
			} else {
				Q.push_back(s);
			}
		}
		part2 = std::min(part2, Q.size());
	}

	printf("Day 05 Part 1: %ld\nDay 05 Part 2: %ld\n", --part1, --part2);
}
