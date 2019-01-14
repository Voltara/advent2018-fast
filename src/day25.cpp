#include "advent2018.h"

namespace {

struct DisjointSet {
	struct Set {
		uint16_t parent;
		uint16_t rank = 0;
		Set(uint16_t id) : parent(id) {
		}
	};

	std::vector<Set> S;
	uint16_t trees;

	DisjointSet(uint16_t n) {
		S.reserve(trees = n);
		for (uint16_t i = 0; i < n; i++) {
			S.emplace_back(i);
		}
	}

	uint16_t find(uint16_t x) {
		while (S[x].parent != x) {
			x = std::exchange(S[x].parent, S[S[x].parent].parent);
		}
		return x;
	}

	void join(uint16_t x, uint16_t y) {
		if ((x = find(x)) != (y = find(y))) {
			trees--;
			if (S[x].rank < S[y].rank) {
				S[x].parent = y;
			} else {
				S[y].parent = x;
				if (S[x].rank == S[y].rank) {
					S[x].rank++;
				}
			}
		}
	}
};

}

void day25(input_t input) {
	std::vector<int8_t, aligned_allocator<int8_t, alignof(__m256i)>> V;
	V.reserve(8192);

	for (uint8_t n = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			V.push_back(n ? -c : c);
			n = 0;
		} else if (*input.s == '-') {
			n = 1;
		}
	}

	uint32_t n_points = V.size() / 4;

	// Pad to nearest multiple of AVX2 vector size
	V.resize((V.size() + 31ULL) & ~31ULL);
	uint32_t n_vectors = V.size() / 32;

	DisjointSet S(n_points);

	auto p32  = (uint32_t *) &V[0];
	auto p256 = (__m256i  *) &V[0];

	// Pairwise measure distance between points
	for (uint32_t i = 1; i < n_points; i++) {
		__m256i v = _mm256_set1_epi32(p32[i]);

		// Compare this point against 8 points at a time
		for (uint32_t j = 0; ; j++) {
			// Absolute difference per coordinate
			__m256i d = _mm256_abs_epi8(_mm256_sub_epi8(v, p256[j]));
			// Horizontal sum of differences per point
			d = _mm256_add_epi32(d, _mm256_slli_epi32(d, 16));
			d = _mm256_add_epi32(d, _mm256_slli_epi32(d,  8));
			// Less than 4?
			d = _mm256_cmpgt_epi32(_mm256_set1_epi32(0x04000000), d);

			uint32_t m = _mm256_movemask_epi8(d);
			if (m) {
				m &= 0x11111111;
				uint32_t base = j * 8;
				do {
					uint32_t idx = base + _tzcnt_u32(m) / 4;
					if (idx == i) {
						goto next_point;
					}
					S.join(i, idx);
					m = _blsr_u32(m);
				} while (m);
			}
		}
next_point:;
	}
	printf("Day 25 Part 1: %u\n", S.trees);
}
