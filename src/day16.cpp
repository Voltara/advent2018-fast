#include "advent2018.h"

using Quad = std::array<int8_t, 4>;

void day16(input_t input) {
	std::vector<Quad> V;
	Quad tmp;
	size_t prog_idx = -1;
	for (int8_t i = 0, n = 0, have = 0; input.len--; input.s++) {
		if (*input.s == ']') {
			prog_idx = V.size();
		}
		uint8_t c = *input.s - '0';
		if (c < 10) {
			have = 1;
			n = 10 * n + c;
		} else if (have) {
			have = 0;
			tmp[i] = n;
			n = 0;
			if (++i == 4) {
				i = 0;
				V.push_back(tmp);
			}
		}
	}
	prog_idx++;

	std::array<uint16_t, 16> M;
	M.fill(0xffff);

	int part1 = 0;
	for (auto ti = V.begin(), end = ti + prog_idx - 2; ti < end; ) {
		auto &R = *ti++;  // registers (before)
		auto &O = *ti++;  // opcode
		auto &A = *ti++;  // registers (after)

		O[0] &= 15; O[1] &= 3; O[2] &= 3; O[3] &= 3;

		auto Ia = O[1],  Ib = O[2];
		auto Ra = R[Ia], Rb = R[Ib];

		uint16_t ok = 0;
		auto E = A[O[3]]; // expected result
		ok |= (E == (Ra +  Rb)) <<  0;
		ok |= (E == (Ra +  Ib)) <<  1;
		ok |= (E == (Ra *  Rb)) <<  2;
		ok |= (E == (Ra *  Ib)) <<  3;
		ok |= (E == (Ra &  Rb)) <<  4;
		ok |= (E == (Ra &  Ib)) <<  5;
		ok |= (E == (Ra |  Rb)) <<  6;
		ok |= (E == (Ra |  Ib)) <<  7;
		ok |= (E == (Ra      )) <<  8;
		ok |= (E == (Ia      )) <<  9;
		ok |= (E == (Ia >  Rb)) << 10;
		ok |= (E == (Ra >  Ib)) << 11;
		ok |= (E == (Ra >  Rb)) << 12;
		ok |= (E == (Ia == Rb)) << 13;
		ok |= (E == (Ra == Ib)) << 14;
		ok |= (E == (Ra == Rb)) << 15;
		part1 += _popcnt32(ok) >= 3;
		M[O[0]] &= ok;
	}

	for (uint32_t ops = 0xffff, todo = 0xffff; todo; ) {
		for (uint32_t i = todo; i; i = _blsr_u32(i)) {
			auto &m = M[_tzcnt_u32(i)];
			if (!_blsr_u32(m &= ops)) {
				ops ^= m;
				todo ^= _blsi_u32(i);
			}
		}
	}

	std::array<uint16_t, 4> R = { 0, 0, 0, 0 };
	for (auto ip = V.begin() + prog_idx; ip != V.end(); ip++) {
		auto &O = *ip;
		O[0] &= 15; O[1] &= 3; O[2] &= 3; O[3] &= 3;
		auto instr = _tzcnt_u32(M[O[0]]);
		auto Ia = O[1],  Ib = O[2],   Ic = O[3];
		auto Ra = R[Ia], Rb = R[Ib], &Rc = R[Ic];
		switch (instr) {
		    case  0: Rc = Ra +  Rb; break;
		    case  1: Rc = Ra +  Ib; break;
		    case  2: Rc = Ra *  Rb; break;
		    case  3: Rc = Ra *  Ib; break;
		    case  4: Rc = Ra &  Rb; break;
		    case  5: Rc = Ra &  Ib; break;
		    case  6: Rc = Ra |  Rb; break;
		    case  7: Rc = Ra |  Ib; break;
		    case  8: Rc = Ra      ; break;
		    case  9: Rc = Ia      ; break;
		    case 10: Rc = Ia >  Rb; break;
		    case 11: Rc = Ra >  Ib; break;
		    case 12: Rc = Ra >  Rb; break;
		    case 13: Rc = Ia == Rb; break;
		    case 14: Rc = Ra == Ib; break;
		    case 15: Rc = Ra == Rb; break;
		}
	}

	printf("Day 16 Part 1: %d\nDay 16 Part 2: %d\n", part1, R[0]);
}
