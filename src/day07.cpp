#include "advent2018.h"

void day07(input_t input) {
	// Per-node egress vector
	std::array<__m256i, 26> out = { };
	auto out8 = (int8_t *) &out[0];

	for (auto s = input.s, e = s + input.len; s < e; s += 49) {
		uint8_t a = s[ 5] - 'A';
		uint8_t b = s[36] - 'A';
		if ((a >= 26) | (b >= 26)) {
			fprintf(stderr, "bad input\n");
			abort();
		}
		out8[32 * a + b] = 1;
	}

	// Indegree vector with nodes 27-31 masked off
	__m256i in = _mm256_setr_epi64x(0, 0, 0, -0x10000);
	for (auto&& v : out) {
		in = _mm256_add_epi8(in, v);
	}

	auto _in = in;
	char part1[27], *p = part1;
	for (uint32_t todo = 0x3ffffff;;) {
		// Find all nodes with indegree zero
		auto cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(
					_in, _mm256_setzero_si256()));
		cmp &= todo;
		if (!cmp) {
			break;
		}

		// Get the first such node in alphabetical order
		auto m = _tzcnt_u32(cmp);
		auto bit = _blsi_u32(cmp);
		todo ^= bit;

		*p++ = 'A' + m;

		// Subtract this node's sucessors from the indegree vector
		_in = _mm256_sub_epi8(_in, out[m]);
	}
	*p = 0;

	// Part 2
	std::vector<std::pair<int,int>> Q;
	int part2 = 0;
	for (uint32_t todo = 0x3ffffff;;) {
		// Same thing as before, but with 5 workers
		auto cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(
					in, _mm256_setzero_si256()));
		cmp &= todo;
		while (cmp && Q.size() < 5) {
			auto m = _tzcnt_u32(cmp);
			auto bit = _blsi_u32(cmp);
			todo ^= bit;
			cmp ^= bit;
			Q.emplace_back(part2 + 61 + m, m);
		}
		if (Q.empty()) {
			break;
		}

		// Move forward in time until the next step(s) finish
		std::sort(Q.rbegin(), Q.rend());
		part2 = Q.back().first;
		do {
			// Unblock successor nodes
			in = _mm256_sub_epi8(in, out[Q.back().second]);
			Q.pop_back();
		} while (!Q.empty() && Q.back().first == part2);
	}

	printf("Day 07 Part 1: %s\nDay 07 Part 2: %d\n", part1, part2);
}
