#include "advent2018.h"

void day09(input_t input) {
	uint32_t n_players = 0, n_points = 0;
	for (uint32_t n = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (n && *input.s == ' ') {
			(n_players ? n_points : n_players) = n;
			n = 0;
		}
	}

	// State after first round, from puzzle description
	constexpr std::array<int32_t, 22> initial = {
		 2, 20, 10, 21,  5, 22, 11,  1, 12,  6, 13,
		 3, 14,  7, 15,  0, 16,  8, 17,  4, 18, 19,
	};

	// Next marbles up for play, interleaved
	auto new_marbles = _mm256_setr_epi32(28, 24, 29, 25, 30, 26, 31, 27);

	// Initialize the playing field
	auto marbles = (int32_t *) aligned_alloc(32, n_points * 384);
	std::copy(initial.begin(), initial.end(), marbles);
	auto Mnext = marbles, Mend = marbles + 22;

	// Initialize score sheet
	if (n_players % 23 == 0) {
		n_players /= 23;
	}
	size_t cur_player = 0;
	std::vector<uint64_t> score(n_players);
	score[cur_player] = 32;

	const auto append = [&](size_t step, const __m256i& v) {
		_mm256_storeu_si256((__m256i *) Mend, v);
		Mend += step;
	};

	const auto load_next = [&]() {
		auto m = _mm256_load_si256((__m256i *) Mnext);
		Mnext += 8;
		return m;
	};

	// Prime the AVX2 register with the first group of 8 marbles
	auto M = _mm256_permutevar8x32_epi32(load_next(), _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));

	uint32_t turn = 46;

	auto simulate = [&](size_t count) {
		for (; count--; turn += 23) {
			// First 8 turns
			append(8, _mm256_blend_epi32(M, new_marbles, 0xaa));
			append(8, _mm256_shuffle_epi32(_mm256_blend_epi32(M, new_marbles, 0x55), 0xb1));

			new_marbles = _mm256_add_epi32(new_marbles, _mm256_set1_epi32(8));

			// Next 8 turns
			M = _mm256_permutevar8x32_epi32(load_next(), _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
			append(8, _mm256_blend_epi32(M, new_marbles, 0xaa));
			append(8, _mm256_shuffle_epi32(_mm256_blend_epi32(M, new_marbles, 0x55), 0xb1));

			new_marbles = _mm256_add_epi32(new_marbles, _mm256_setr_epi32(0, 8, 6, 8, 4, 0, 5, 0));

			// Final 7 turns
			M = _mm256_permutevar8x32_epi32(load_next(), _mm256_setr_epi32(0, 3, 1, 5, 2, 4, 7, 6));
			append(5, _mm256_blend_epi32(M, new_marbles, 0xfa));

			cur_player++;
			cur_player &= -(cur_player < n_players);
			score[cur_player] += turn + _mm256_extract_epi32(M, 4);

			// Prepare the first 8 marbles of the next round
			M = _mm256_blend_epi32(_mm256_shuffle_epi32(M, 0x8d), new_marbles, 0x4c);
			M = _mm256_add_epi32(M, _mm256_setr_epi32(0, 0, 0, 4, 0, 0, 0, 0));

			new_marbles = _mm256_add_epi32(new_marbles, _mm256_setr_epi32(15, 7, 9, 7, 11, 15, 10, 15));
		}
	};

	// Part 1
	auto n_rounds1 = n_points / 23;
	simulate(n_rounds1 - 1);
	uint64_t part1 = 0;

	for (auto&& s : score) {
		part1 = std::max(part1, s);
	}

	// Part 2: 100x the number of points
	auto n_rounds2 = 100 * n_points / 23;

	// Simulate only enough to generate the scoreballs
	auto n_simulate2 = (n_rounds2 + 3) * 16 / 37;
	simulate(n_simulate2 - n_rounds1);
	n_rounds2 -= n_simulate2;

	// Tally up the remaining scoreballs
	for (Mnext += 10; n_rounds2--; Mnext += 16, turn += 23) {
		cur_player++;
		cur_player &= -(cur_player < n_players);
		score[cur_player] += turn + *Mnext;
	}

	uint64_t part2 = 0;
	for (auto&& s : score) {
		part2 = std::max(part2, s);
	}

	printf("Day 09 Part 1: %lu\nDay 09 Part 2: %lu\n", part1, part2);

	free(marbles);
}
