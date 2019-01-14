#include "advent2018.h"

namespace {

struct State {
	uint16_t cell = 0x8080;
	uint16_t dist = 0;
};

/* Short-term memory, up to 16 recent coordinates visited
 * Only capable of recalling +1 or -1 change in distance
 * Purpose: To handle small amounts of backtracking
 */
struct Goldfish {
	__m128i xcoord = _mm_setzero_si128();
	__m128i ycoord = _mm_setzero_si128();
	uint32_t value = 0;

	bool remember(State &S) {
		__m128i xv = _mm_set1_epi8(S.cell);
		__m128i yv = _mm_set1_epi8(S.cell >> 8);

		__m128i cmp = _mm_and_si128(_mm_cmpeq_epi8(xcoord, xv), _mm_cmpeq_epi8(ycoord, yv));
		uint32_t idx = _mm_movemask_epi8(cmp);
		if (idx) {
			idx = _mm_tzcnt_32(idx) << 1;
			uint16_t delta = (S.dist - (value >> idx)) & 2;
			S.dist += delta - 1;
			return true;
		} else {
			xcoord = _mm_alignr_epi8(xcoord, xv, 15);
			ycoord = _mm_alignr_epi8(ycoord, yv, 15);
			value = (value << 2) | (S.dist & 3);
			return false;
		}
	}
};

}

void day20(input_t input) {
	uint16_t part1 = 0, part2 = 0;

	Goldfish goldfish;
	std::vector<State> stack;
	stack.reserve(300);

	for (State S; input.len--; input.s++) {
		switch (*input.s) {
		    case 'N': S.cell += 0x0200;
		    case 'S': S.cell += 0xfeff;
		    case 'E': S.cell += 0x0002;
		    case 'W': S.cell += 0xffff;
			      if (!goldfish.remember(S)) {
				      part1 = std::max(part1, ++S.dist);
				      part2 += (S.dist >= 1000);
			      }
			      break;
		    case '(': stack.push_back(S);
			      break;
		    case '|': S = stack.back();
			      break;
		    case ')': S = stack.back();
			      stack.pop_back();
			      break;
		}
	}

	printf("Day 20 Part 1: %u\nDay 20 Part 2: %u\n", part1, part2);
}
