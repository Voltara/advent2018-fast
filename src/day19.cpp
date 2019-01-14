#include "advent2018.h"

using Quad = std::array<int8_t, 4>;

static uint32_t divisor_sum(uint32_t n) {
	uint32_t divisor_sum = 1;

	auto try_factor = [&](uint32_t f) {
		if (n % f == 0) {
			uint32_t mult = 1, fk = 1;
			do {
				n /= f;
				fk *= f;
				mult += fk;
			} while (n % f == 0);
			divisor_sum *= mult;
		}
	};

	// Wheel factorization
	for (uint32_t f : { 2, 3, 5 }) {
		try_factor(f);
	}

	for (uint32_t f = 7, step = 0x62642424; f * f <= n; ) {
		try_factor(f);
		f += step & 15;
		step = (step << 28) | (step >> 4);
	}

	if (n > 1) {
		divisor_sum *= n + 1;
	}

	return divisor_sum;
}

void day19(input_t input) {
	int8_t ipreg = -1;

	while (input.len--) {
		uint8_t c = *input.s++ - '0';
		if (c < 6) {
			ipreg = c;
			break;
		}
	}
	if (ipreg == -1) {
		abort();
	}

	std::vector<Quad> V;
	Quad tmp;
	for (int have = 0, n = 0, i = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 100) {
			have = 1;
			n = 10 * n + c;
		} else if (have) {
			tmp[i] = n;
			n = have = 0;
			if (++i == 4) {
				i = 0;
				V.push_back(tmp);
			}
		}
	}

	std::array<uint32_t, 2> part = { 0, 1 };
	for (auto&& P : part) {
		uint32_t R[8] = { P };
		for (auto &ip = R[ipreg]; ip < V.size(); ip++) {
			auto &O = V[ip];
			auto Ia = O[1],    Ib = O[2],     Ic = O[3];
			auto Ra = R[Ia&7], Rb = R[Ib&7], &Rc = R[Ic&7];
			switch (O[0]) {
			    case  -7: Rc = Ra + Ib;    break;     // addi
			    case   2: Rc = Ra + Rb;    break;     // addr
			    case  66: P  = Rb;         goto done; // eqrr
			    case -51: Rc = Ra * Ib;    break;     // muli
			    case -42: Rc = Ra * Rb;    break;     // mulr
			    case  77: Rc = Ia;         break;     // seti
			    case  86: Rc = Ra;         break;     // setr
			    default: abort();
			}
		}
done:
		P = divisor_sum(P);
	}

	printf("Day 19 Part 1: %u\nDay 19 Part 2: %u\n", part[0], part[1]);
}
