#include "advent2018.h"

void day08(input_t input) {
	std::vector<uint8_t> v;
	v.reserve(input.len);

	uint8_t last = 0xff;
	for (; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			if (last < 10) {
				v.back() = 10 + c;
			} else {
				v.emplace_back(c);
			}
		}
		last = c;
	}

	int part1 = 0;

	auto it = v.begin();
	std::vector<uint32_t> results;

	std::function<uint32_t()> recur = [&]() {
		auto frame = results.size();

		if (it + 2 > v.end()) abort();
		auto n_child = *it++;
		auto n_meta  = *it++;

		// Recursively handle children
		for (auto k = n_child; k--; ) {
			results.push_back(recur());
		}

		if (it + n_meta > v.end()) abort();

		uint32_t val = 0;
		if (n_child) {
			// Recursive case: sum of children
			while (n_meta--) {
				auto idx = *it++;
				part1 += idx--;
				if (idx < n_child) {
					val += results[frame + idx];
				}
			}
		} else {
			// Base case: sum of metadata
			while (n_meta--) {
				val += *it++;
			}
			part1 += val;
		}

		results.resize(frame);
		return val;
	};

	auto part2 = recur();

	printf("Day 08 Part 1: %d\nDay 08 Part 2: %d\n", part1, part2);
}
