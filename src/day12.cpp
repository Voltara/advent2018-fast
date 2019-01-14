#include "advent2018.h"

static int64_t score(const std::vector<bool> &state, int64_t base) {
	int64_t r = 0;
	for (auto b : state) {
		r += base++ & -int64_t(b);
	}
	return r;
}

void day12(input_t input) {
	std::vector<bool> state = { };
	for (; input.len && *input.s != '\n'; input.s++, input.len--) {
		switch (*input.s) {
		    case '.': state.push_back(false); break;
		    case '#': state.push_back(true);  break;
		}
	}

	uint32_t rules = 0;
	for (uint32_t n = 0; input.len--; input.s++) {
		switch (*input.s) {
		    case '.':   n <<= 1;         break;
		    case '#':   n <<= 1; n |= 1; break;
		    case '\n':  rules |= (n & 1) << (n >> 1);
		}
	}

	int64_t base = 0, part1, part2;

	std::vector<bool> new_state;
	for (int64_t gen = 0; ; ) {
		int n = 0, offset;

		// Skip over prefix of dead cells
		state.push_back(true);
		for (offset = 0; !state[offset]; offset++) {
		}
		state.pop_back();

		// Apply rules to every 5-bit subsequence
		auto new_base = base + offset - 2;
		for (; offset < state.size(); offset++) {
			auto b = state[offset];
			n = ((n << 1) | b) & 31;
			new_state.push_back((rules >> n) & 1);
		}
		// Continue applying rules until all 1-bits are gone
		while ((n = (n << 1) & 31)) {
			new_state.push_back((rules >> n) & 1);
		}

		// Trim dead cells from the end
		while (!new_state.empty() && !new_state.back()) {
			new_state.pop_back();
		}

		// Keep going until both parts are solved
		if (++gen >= 20) {
			if (gen == 20) {
				part1 = score(new_state, new_base);
			} else if (state == new_state) {
				part2 = score(new_state, new_base);
				auto delta = part2 - score(state, base);
				part2 += delta * (50000000000LL - gen);
				break;
			}
		}

		state.swap(new_state);
		new_state.clear();
		base = new_base;
	}

	printf("Day 12 Part 1: %ld\nDay 12 Part 2: %ld\n", part1, part2);
}
