#include "advent2018.h"

namespace {

struct Day21 {
	uint32_t input, value;
	__m128i Cv;

	Day21(uint32_t input) : input(input), value() {
		Cv = _mm_setr_epi32(0, 0, 0, input);
	}

	operator uint32_t () {
		return value;
	}

	Day21& operator () () {
		Cv = _mm_or_si128(Cv, _mm_setr_epi32(1, 0, 0, 0));
		Cv = _mm_mullo_epi32(Cv, _mm_setr_epi32(65899, 14156473, 318547, 318547));
		Cv = _mm_hadd_epi32(Cv, Cv);
		Cv = _mm_hadd_epi32(Cv, Cv);
		value = _mm_extract_epi32(Cv, 0) & 0xffffff;
		Cv = _mm_insert_epi32(Cv, input, 3);
		Cv = _mm_srlv_epi32(Cv, _mm_setr_epi32(16, 8, 0, 0));
		Cv = _mm_and_si128(Cv, _mm_setr_epi32(255, 255, 255, -1));
		return *this;
	}
};

}

void day21(input_t input) {
	uint32_t in = 0;

	if (input.len < 97) {
		fprintf(stderr, "Day 21 input too short\n");
		abort();
	}

	for (input.s += 97, input.len -= 97; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c >= 10) {
			break;
		}
		in = 10 * in + c;
	}

	std::vector<uint32_t, aligned_allocator<uint32_t, alignof(__m256i)>> V;
	V.reserve(32768);

	Day21 H(in);
	V.push_back(H);
	V.push_back(H());

	// Brent's cycle detection algorithm
	// Can also be done in O(1) space, but that was 30% slower
	uint32_t period = 1, h = 1, T = V[0];
	for (uint32_t power = 1; T != V[h]; period++, h++, V.push_back(H())) {
		if (power == period) {
			T = V[h];
			power <<= 1;
			period = 0;
		}
	}

	uint32_t part1 = V[1], part2 = 0;
	for (auto tp = (__m256i *) &V[0], hp = (__m256i *) &V[period]; ; hp++, tp++) {
		__m256i cmp = _mm256_cmpeq_epi32(
				_mm256_loadu_si256(hp), _mm256_load_si256(tp));
		uint32_t mask = _mm256_movemask_epi8(cmp);
		if (mask) {
			int32_t offset = _tzcnt_u32(mask) >> 2;
			part2 = ((uint32_t *) hp)[offset - 1];
			break;
		}
	}

	printf("Day 21 Part 1: %u\nDay 21 Part 2: %u\n", part1, part2);
}
