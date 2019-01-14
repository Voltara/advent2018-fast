#include "advent2018.h"

namespace {

struct answer_t {
	int32_t x, y, sz;
};

}

static constexpr size_t dim = 300;
static constexpr size_t pad = (dim + 2 + 7) & ~7LL;

static int32_t g[pad][pad] = { };

static answer_t solve(int32_t sz0, int32_t sz1) {
        answer_t A = { };
        int32_t max = INT_MIN;
	auto vmax = _mm256_set1_epi32(max);
	for (int32_t sz = sz0 - 1; sz < sz1; sz++) {
		for (int32_t y = 1; y <= dim - sz; y++) {
			auto g0a = (__m256i *)  g[y -  1];
			auto g0b = (__m256i *) (g[y -  1] + sz + 1);
			auto g1a = (__m256i *)  g[y + sz];
			auto g1b = (__m256i *) (g[y + sz] + sz + 1);

			for (int32_t x = 1; x <= dim - sz; x += 8) {
				auto v0a = _mm256_loadu_si256(g0a);
				auto v0b = _mm256_loadu_si256(g0b);
				auto v1a = _mm256_loadu_si256(g1a);
				auto v1b = _mm256_loadu_si256(g1b);
				auto sum = _mm256_sub_epi32(
						_mm256_add_epi32(v0a, v1b),
						_mm256_add_epi32(v1a, v0b));
				g0a++, g0b++, g1a++, g1b++;
				auto cmp = _mm256_cmpgt_epi32(sum, vmax);
				if (_mm256_movemask_epi8(cmp)) {
					auto arr = (int32_t *) &sum;
					for (int32_t i = 0; i < 8; i++) {
						if (x + i > dim - sz) {
							break;
						}
						if (arr[i] > max) {
							max = arr[i];
							vmax = _mm256_set1_epi32(max);
							A = { x + i, y, sz + 1 };
						}
					}
				}
			}
		}
	}
	return A;
}

static int sigma(double S) {
	return ceil(sqrt(33) * S);
}

void day11(input_t input) {
	int32_t in = 0;
	for (; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			in = 10 * in + c;
		}
	}

	// Difference between rows, before div/mod
	int32_t row_add[pad];
	for (int32_t x = 0; x < pad; x++) {
		int32_t p = x + 10;
		row_add[x] = p * p;
	}
	row_add[0] = 0;

	// Build the array in a way that the compiler can vectorize
	for (int32_t x = 0; x < pad; x++) {
		// First row
		int32_t p = x + 10;
		g[1][x] = (in + p) * p;
	}
	g[1][0] = 0;
	for (int32_t y = 1; y <= dim; y++) {
		// Subsequent rows
		for (int32_t x = 0; x < pad; x++) {
			g[y + 1][x] = g[y][x] + row_add[x];
		}
		for (int32_t x = 0; x < pad; x++) {
			g[y][x] /= 100;
			g[y][x] %= 10;
			g[y][x] -= 5;
		}
		g[y][0] = 0;
	}

	// Make an integral image out of the array
	for (int32_t y = 1; y < dim; y++) {
		for (int32_t x = 0; x < pad; x++) {
			g[y + 1][x] += g[y][x];
		}
	}
	for (int32_t y = 1; y <= dim; y++) {
		for (int32_t x = 2; x <= dim; x++) {
			g[y][x] += g[y][x - 1];
		}
	}

	auto P1 = solve(3, 3);
	auto P2 = solve(4, sigma(6.0));

	printf("Day 11 Part 1: %d,%d\nDay 11 Part 2: %d,%d,%d\n",
			P1.x, P1.y,
			P2.x, P2.y, P2.sz);
}
