#include "advent2018.h"

namespace {

struct val_t {
	int a, b, c;
	val_t() { }
	val_t(int a, int b, int c) : a(a), b(b), c(c) { }
	bool operator < (const val_t &o) {
		return (a == o.a) ? (b < o.b) : (a < o.a);
	}
};

}

void day01(input_t input) {
	std::vector<val_t> v;
	v.reserve(1000);

	// Parse input, accumulate frequencies
	for (int n = 0, total = 0, neg = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			continue;
		}
		neg |= (*input.s == '-');
		if (n) {
			total += n - neg ^ -neg;
			v.emplace_back(total, 0, v.size());
			n = neg = 0;
		}
	}

	// Part 1 solution is the final sum
	auto part1 = v.back().a;

	// Split up into div/mod
	for (auto &n : v) {
		n.b = n.a / part1;
		if ((n.a %= part1) < 0) {
			n.a += part1;
			n.b--;
		}
	}

	// Group by mod, sort by ascending div
	std::sort(v.begin(), v.end());

	// Get div deltas per mod
	val_t last{-1, 0, 0}, tmp;
	for (auto &n : v) {
		if (n.a == last.a) {
			tmp = { n.b - last.b, last.c, n.a + n.b * part1 };
		} else {
			tmp = { INT_MAX, INT_MAX, INT_MAX };
		}
		last = n;
		n = tmp;
	}

	// Order ascending by div, then index
	std::sort(v.begin(), v.end());

	printf("Day 01 Part 1: %d\nDay 01 Part 2: %d\n", part1, v[0].c);
}
