#include "advent2018.h"

// Recipes 0..22 precomputed
static constexpr std::array<uint8_t, 23> BASE = {
	3,7,1,0,1,0,1,2,4,5,1,5,8,9,1,6,7,7,9,2,5,1,0 };

/* Precomputed ingredient sequences for starting offsets 0..8,
 * up to (but not including) offset 23 where they all converge.
 *   Example: 0xfff94113 is the sequence [ 3, 1, 1, 4, 9 ].
 * Upper nibbles are filled with 0xf, so that when all values
 * have been shifted right, the value becomes -1 (0xffffffff)
 */
static constexpr std::array<uint32_t, 9> PREMIX = {
	0xfff94113, 0xffff0657, 0xfff94111, 0xfff94110, 0xffff9411,
	0xffff9410, 0xfffff941, 0xffff1812, 0xffffff94 };

/* NOTE: To keep the pattern matching code from being unnecessarily
 * complex, this supports only inputs of exactly length 6.
 */
static constexpr size_t INPUT_LEN = 6;

/* Preallocated arrays for mixers and recipes.  Should be enough
 * to handle any of the inputs given.  Mixers can be increased
 * without penalty; recipes needs to be prefilled with ones, so
 * there is a speed penalty for making it too large.
 */
static constexpr size_t MIXERS_MEM  =  8 << 20;
static constexpr size_t RECIPES_MEM = 22 << 20;

/* The loop termination condition is checked at intervals within
 * the vectorized middle loop.  Therefore, there needs to be enough
 * 'recipes' memory to hold the new values appended between checks.
 */
static constexpr size_t CHECK_INTERVAL = 1 << 8;
static constexpr size_t GENERATE_LIMIT = RECIPES_MEM - (64 * CHECK_INTERVAL);

void day14(input_t in) {
	int input_n = 0;

	alignas(alignof(uint32_t)) std::array<uint8_t, std::max(size_t(4), INPUT_LEN)> input;
	for (size_t i = 0; i < INPUT_LEN; i++) {
		input[i] = in.s[i] - '0';
		input_n = 10 * input_n + input[i];
	}

	// Only those recipes used as ingredients, excluding the PREMIX recipes
	auto mixers = reinterpret_cast<uint8_t *>(aligned_alloc(32, MIXERS_MEM));
	size_t mixers_size = 0;

	/* Full array of recipes, initialized with the base recipes.  The rest
	 * of the array is prepopulated with ones, which makes dealing with the
	 * tens digits significantly more efficient
	 */
	auto recipes = reinterpret_cast<uint8_t *>(aligned_alloc(32, RECIPES_MEM));
	std::fill(recipes, recipes + RECIPES_MEM, 1);
	std::copy(BASE.begin(), BASE.end(), recipes);
	size_t recipes_size = BASE.size();

	/* Premix queues for the two elves; using signed integers for
	 * arithmetic right-shift
	 */
	int32_t premix[] = { int32_t(PREMIX[0]), int32_t(PREMIX[8]) };

	// Next index to be added to mixers
	size_t next_idx = BASE.size(), last_idx = BASE.size();

	// Current index into mixers for both elves
	size_t idx[2] = { 0, 0 };

	// Part 2 "needle" vector (first 4 digits of the input) for SIMD search
	auto needle = _mm256_set1_epi32(*reinterpret_cast<uint32_t *>(&input[0]));
	size_t search_offset = 0, found_offset = -1;

	// Appends a single recipe to the 'recipes' and (maybe) 'mixers' lists
	const auto append_recipe = [&](uint8_t recipe) {
		// Tens digit
		if (recipe >= 10) {
			recipe -= 10;
			if (recipes_size == next_idx) {
				mixers[mixers_size++] = 1;
				last_idx = next_idx;
				next_idx += 2;
			}
			// 'recipe' is prepopulated with ones, so just increment size
			recipes_size++;
		}

		// Ones digit
		if (recipes_size == next_idx) {
			mixers[mixers_size++] = recipe;
			last_idx = next_idx;
			next_idx += recipe + 1;
		}
		recipes[recipes_size++] = recipe;

		// Handle wraparound
		for (int i = 0; i < 2; i++) {
			if (idx[i] == mixers_size) {
				premix[i] = PREMIX[last_idx + mixers[mixers_size - 1] + 1 - recipes_size];
				idx[i] = 0;
			}
		}
	};

	for (;;) {
		// One or both of the elves is working on premix
		do {
			uint8_t recipe = 0;
			for (int i = 0; i < 2; i++) {
				if (~premix[i]) {
					recipe += premix[i] & 0xf;
					premix[i] >>= 4;
				} else {
					recipe += mixers[idx[i]++];
				}
			}
			append_recipe(recipe);
		} while ((premix[0] & premix[1]) != -1);

		/* Both elves are working on the converged mixers; these are
		 * processed in groups of 32 through the magic of SIMD.
		 * This outer loop will run multiple times (with successively
		 * fewer iterations), because each block of iterations will add
		 * more mixers to the list.
		 */
		if (idx[0] != 0) {
			/* Invariant: One of the indices is always zero, so we
			 * can use aligned load instructions for that one.
			 */
			std::swap(idx[0], idx[1]);
		}
		for (;;) {
			size_t iterations = (mixers_size - std::max(idx[0], idx[1])) / 32;
			if (iterations == 0) {
				break;
			}

			for (size_t i = 0; i < iterations; i++) {
				auto v0 = _mm256_load_si256( reinterpret_cast<__m256i *>(&mixers[idx[0]]));
				auto v1 = _mm256_loadu_si256(reinterpret_cast<__m256i *>(&mixers[idx[1]]));
				idx[0] += 32, idx[1] += 32;

				// vsum = v0 + v1
				auto vsum = _mm256_add_epi8(v0, v1);

				// vtens = (vsum < 10) ? 0 : -1
				auto vtens = _mm256_cmpgt_epi8(vsum, _mm256_set1_epi8(9));

				// vsum %= 10
				vsum = _mm256_sub_epi8(vsum, _mm256_and_si256(vtens, _mm256_set1_epi8(10)));

				// Calculate the running sum of vtens (separate running sum per 16-byte lane)
				vtens = _mm256_add_epi8(vtens, _mm256_bslli_epi128(vtens, 8));
				vtens = _mm256_add_epi8(vtens, _mm256_bslli_epi128(vtens, 4));
				vtens = _mm256_add_epi8(vtens, _mm256_bslli_epi128(vtens, 2));
				vtens = _mm256_add_epi8(vtens, _mm256_bslli_epi128(vtens, 1));

				/* Use the summed vtens vector to compute adjusted offsets into 'recipes'
				 * These offsets are where the "vsum % 10" digits will be stored.
				 */
				auto voffset = _mm256_sub_epi8(
						_mm256_setr_epi8(
							0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
							0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15), vtens);

				/* Scatter the digits into the correct locations.  We do this
				 * separately for each 16-byte lane.  This is faster than using
				 * additional cross-lane operations to compute a 32-element
				 * running sum for the 'vtens' vector above.
				 */
				auto digit = reinterpret_cast<uint8_t *>(&vsum);
				auto offset = reinterpret_cast<uint8_t *>(&voffset);

				// Low lane
				for (int i = 0; i < 16; i++) {
					recipes[recipes_size + offset[i]] = digit[i];
				}
				recipes_size += offset[15] + 1;

				// High lane
				for (int i = 16; i < 32; i++) {
					recipes[recipes_size + offset[i]] = digit[i];
				}
				recipes_size += offset[31] + 1;

				// Check for loop termination
				if ((i % CHECK_INTERVAL) == 0) {
					static_assert(INPUT_LEN == 6, "search is implemented only for INPUT_LEN == 6");

					// Search for the part2 solution, examining 16 starting offsets at a time
					for (; search_offset < recipes_size - 32; search_offset += 16) {
						auto v = _mm256_loadu_si256(reinterpret_cast<__m256i *>(&recipes[search_offset]));
						v = _mm256_permutevar8x32_epi32(v, _mm256_setr_epi32(0, 1, 2, -1, 2, 3, 4, -1));

						auto cmp = _mm256_mpsadbw_epu8(v, needle, 0);
						cmp = _mm256_cmpeq_epi16(cmp, _mm256_setzero_si256());

						/* Iterate over all 4-byte matches, and check whether the
						 * final 2 bytes of the input also match
						 */
						auto mask = _mm256_movemask_epi8(cmp) & 0x55555555;
						while (mask) {
							auto m = _tzcnt_u32(mask) >> 1;
							mask = _blsr_u32(mask);

							found_offset = search_offset + m;
							if (recipes[found_offset + 4] == input[4] &&
							    recipes[found_offset + 5] == input[5])
							{
								goto done;
							}
						}
					}

					// Or, just give up
					if (recipes_size > GENERATE_LIMIT) {
						found_offset = -1;
						goto done;
					}
				}
			}

			/* Propagate new values from 'recipes' into 'mixers'
			 * This loop will always iterate at least once, because
			 * the maximum interval between mixers is 10, and the
			 * vectorized loop produces at least 32 new recipes.
			 */
			do {
				last_idx = next_idx;
				mixers[mixers_size++] = recipes[next_idx];
				next_idx += recipes[next_idx] + 1;
			} while (next_idx < recipes_size);
			// Invariant: (idx[0] < mixers_size) && (idx[1] < mixers_size)
		}

		// Handle the remaining <32 recipes before wrapping back to premix
		do {
			append_recipe(mixers[idx[0]++] + mixers[idx[1]++]);
		} while ((premix[0] & premix[1]) == -1);
	}

done:
	std::string part1(10, 0);
	std::copy(recipes + input_n, recipes + input_n + 10, part1.begin());
	for (auto &c : part1) {
		c += '0';
	}

	printf("Day 14 Part 1: %s\nDay 14 Part 2: %ld\n",
			part1.c_str(), found_offset);
}
