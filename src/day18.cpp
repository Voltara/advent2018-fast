#include "advent2018.h"

namespace {

struct Map {
	__m256i pad0{};
	std::array<__m256i, 100> V;
	__m256i pad1{};

	Map() {
	}

	Map(const char *s) {
		// 0x00 = ".", 0x10 = "|", 0x01 = "#"
		for (size_t i = 0; i < 100; s += 51) {
			auto v0 = _mm256_loadu_si256((__m256i *) s);
			auto v1 = _mm256_loadu_si256((__m256i *) (s + 32));
			V[i++] = _mm256_and_si256(v0, _mm256_set1_epi8(0x11));
			V[i++] = _mm256_and_si256(v1, _mm256_set_epi64x(
						0, 0x1111, -1ULL/15, -1ULL/15));
		}
	}

	Map advance() const {
		struct {
			std::array<__m256i, 100> sum;
			std::array<__m256i, 2> zero;
		} s;
		s.zero.fill(_mm256_setzero_si256());

		auto &sum = s.sum;

		// left + center + right
		auto leftp  = (__m256i *) ((uint8_t *) &V[0] - 1);
		auto rightp = (__m256i *) ((uint8_t *) &V[0] + 1);
		for (size_t i = 0; i < 100; i++) {
			sum[i] = _mm256_add_epi8(V[i],
					_mm256_add_epi8(
						_mm256_loadu_si256(leftp++),
						_mm256_loadu_si256(rightp++)));
		}

		// up + center + down
		__m256i total0{sum[0]}, total1{sum[1]}, up0{}, up1{};
		for (size_t i = 0; i < 100; i += 2) {
			auto cur0 = sum[i], cur1 = sum[i + 1];

			// Add the row below
			total0 = _mm256_add_epi8(total0, sum[i + 2]);
			total1 = _mm256_add_epi8(total1, sum[i + 3]);

			// Complete the neighbor sum
			sum[i    ] = _mm256_sub_epi8(total0, V[i    ]);
			sum[i + 1] = _mm256_sub_epi8(total1, V[i + 1]);

			// Subtract the row above
			total0 = _mm256_sub_epi8(total0, up0);
			total1 = _mm256_sub_epi8(total1, up1);

			up0 = cur0, up1 = cur1;
		}

		// logic
		Map A;
		for (size_t i = 0; i < 100; i++) {
			auto &nv = sum[i], v = V[i];
			// TODO any better way to avoid signed compare issue with 0x80 (8 neighboring trees)?
			auto tree = _mm256_and_si256(_mm256_srli_epi64(nv, 4), _mm256_set1_epi8(0x0f));
			auto yard = _mm256_and_si256(nv, _mm256_set1_epi8(0x0f));
			auto tree3 = _mm256_cmpgt_epi8(tree, _mm256_set1_epi8(0x02));
			auto yard3 = _mm256_cmpgt_epi8(yard, _mm256_set1_epi8(0x02));
			auto miss1 = _mm256_or_si256(
				_mm256_cmpeq_epi8(tree, _mm256_setzero_si256()),
				_mm256_cmpeq_epi8(yard, _mm256_setzero_si256()));
			auto v_open = _mm256_cmpeq_epi8(v, _mm256_set1_epi8(0x00));
			auto v_tree = _mm256_cmpeq_epi8(v, _mm256_set1_epi8(0x10));
			auto v_yard = _mm256_cmpeq_epi8(v, _mm256_set1_epi8(0x01));
			v = _mm256_xor_si256(v, _mm256_and_si256(v_open, _mm256_and_si256(tree3, _mm256_set1_epi8(0x10))));
			v = _mm256_xor_si256(v, _mm256_and_si256(v_tree, _mm256_and_si256(yard3, _mm256_set1_epi8(0x11))));
			v = _mm256_xor_si256(v, _mm256_and_si256(v_yard, _mm256_and_si256(miss1, _mm256_set1_epi8(0x01))));
			if (i & 1) {
				A.V[i] = _mm256_and_si256(v, _mm256_set_epi64x(0, 0x1111, -1ULL/15, -1ULL/15));
			} else {
				A.V[i] = v;
			}
		}
		return A;
	}

	bool operator < (const Map& o) const {
		for (size_t i = 0; i < 100; i++) {
			auto a = V[i], b = o.V[i];
			uint32_t eq = _mm256_movemask_epi8(_mm256_cmpeq_epi8(b, a));
			uint32_t lt = _mm256_movemask_epi8(_mm256_cmpgt_epi8(b, a));
			if (~eq) {
				eq += lt << 1;
				return eq < lt;
			}
		}
		return false;
	}

	uint32_t value() const {
		uint32_t tree = 0, yard = 0;
		for (auto&& v : V) {
			tree += _popcnt32(_mm256_movemask_epi8(_mm256_slli_epi64(v, 3)));
			yard += _popcnt32(_mm256_movemask_epi8(_mm256_slli_epi64(v, 7)));
		}
		return tree * yard;
	}
};

struct History {
	static std::vector<Map> V;
	bool operator()(uint16_t a, uint16_t b) const {
		return V[a] < V[b];
	}
};
decltype(History::V) History::V;

}

void day18(input_t input) {
	std::set<uint16_t, History> seen;
	History::V.reserve(1000);
	History::V.emplace_back(input.s);
	seen.insert(0);

	uint32_t idx = 0;
	for (;;) {
		History::V.push_back(History::V.back().advance());
		auto result = seen.insert(seen.size());
		if (!result.second) {
			uint32_t base = *result.first, loop = seen.size() - base;
			idx = base + (1000000000 - base) % loop;
			break;
		}
	}

	printf("Day 18 Part 1: %u\nDay 18 Part 2: %u\n", History::V[10].value(), History::V[idx].value());
}
