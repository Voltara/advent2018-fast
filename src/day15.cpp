#include "advent2018.h"

namespace {

struct Map {
	__m256i zero0; // padding
	__m256i v[4];
	__m256i zero1; // padding

	Map() : zero0(), v(), zero1() {
	}

	Map(int pos) : zero0(), v(), zero1()  {
		(*this)[pos / 32] = 1 << (pos % 32);
	}

	uint32_t& operator[] (size_t idx) {
		return ((uint32_t *) &v[0])[idx];
	}

	const uint32_t& operator[] (size_t idx) const {
		return ((uint32_t *) &v[0])[idx];
	}

	Map& operator &= (const Map& o) {
		for (size_t i = 0; i < 4; i++) {
			v[i] = _mm256_and_si256(v[i], o.v[i]);
		}
		return *this;
	}

	Map operator & (const Map& o) const {
		return Map{*this} &= o;
	}

	bool operator == (const Map& o) const {
		uint32_t cmp = 0xffffffff;
		for (int i = 0; i < 4; i++) {
			cmp &= _mm256_movemask_epi8(_mm256_cmpeq_epi8(v[i], o.v[i]));
		}
		return cmp == 0xffffffff;
	}

	bool operator != (const Map& o) const {
		return !(*this == o);
	}

	void toggle(uint16_t pos) {
		(*this)[pos / 32] ^= 1 << (pos % 32);
	}

	Map adjacent() const {
		Map A;
		for (size_t i = 0; i < 4; i++) {
			auto left  = _mm256_slli_epi32(v[i], 1);
			auto right = _mm256_srli_epi32(v[i], 1);
			auto down  = _mm256_loadu_si256((__m256i *) ((uint32_t *) &v[i] - 1));
			auto up    = _mm256_loadu_si256((__m256i *) ((uint32_t *) &v[i] + 1));
			auto left_right = _mm256_or_si256(left, right);
			auto down_up    = _mm256_or_si256(down, up);
			A.v[i] = _mm256_or_si256(left_right, down_up);
			A.v[i] = _mm256_or_si256(A.v[i], v[i]);
		}
		return A;
	}

	uint16_t first() const {
		alignas(__m256i) uint8_t arr[32];
		for (int i = 0; i < 4; i++) {
			uint32_t mask = _mm256_movemask_epi8(
					_mm256_cmpeq_epi8(v[i], _mm256_setzero_si256()));
			mask = ~mask;
			if (mask) {
				_mm256_store_si256((__m256i *) arr, v[i]);
				int idx = _mm_tzcnt_32(mask);
				return i * 256 + idx * 8 + _mm_tzcnt_32(arr[idx]);
			}
		}
		return -1;
	}
};

struct Unit {
	uint8_t hp, elf;
	Unit(bool elf) : hp(200), elf(elf) { }
};

struct Day15 {
	Map M, E, G;
	std::vector<Unit> U;
	std::vector<uint16_t> U_pos;

	std::array<uint8_t, 32> U_lookup_pad0; // padding
	std::array<uint8_t, 1024> U_lookup;
	std::array<uint8_t, 32> U_lookup_pad1; // padding

	Day15(input_t input) :
		M(), E(), G(), U(), U_pos()
	{
		U_lookup_pad0.fill(0xff);
		U_lookup.fill(0xff);
		U_lookup_pad1.fill(0xff);

		// Expects a 32x32 map with proper line termination
		if (input.len != 1056) {
			fprintf(stderr, "Day 15: Expected 1056 bytes of input, got %lu\n", input.len);
			abort();
		}

		auto add_units = [&](uint16_t base_pos, uint32_t mask, bool elf) {
			for (; mask; mask = _blsr_u32(mask)) {
				uint16_t pos = base_pos + _tzcnt_u32(mask);
				U_lookup[pos] = U.size();
				U_pos.push_back(pos);
				U.push_back(elf);
			}
		};

		for (uint16_t y = 0; y < 32; y++) {
			auto row = _mm256_loadu_si256((__m256i *) input.s);
			input.s += 33;

			M[y] = _mm256_movemask_epi8(_mm256_cmpeq_epi8(row, _mm256_set1_epi8('.')));
			E[y] = _mm256_movemask_epi8(_mm256_cmpeq_epi8(row, _mm256_set1_epi8('E')));
			G[y] = _mm256_movemask_epi8(_mm256_cmpeq_epi8(row, _mm256_set1_epi8('G')));

			add_units(y * 32, E[y], true);
			add_units(y * 32, G[y], false);
		}
	}

	template<bool Part2>
	int solve(uint8_t elf_power = 3) {
		const Map ZERO;

		std::array<uint8_t, 2> power = { 3, elf_power };
		int32_t total_hp = U.size() * U[0].hp;

		for (int32_t rounds = 0; ; rounds++) {
			std::sort(U_pos.begin(), U_pos.end());
			while (U_pos.back() == 0xffff) {
				U_pos.pop_back();
			}

			for (auto&& pos : U_pos) {
				if (pos == 0xffff) { // Was killed earlier this round
					continue;
				}

				// Get unit at position
				auto &u = U[U_lookup[pos]];

				// Identify friends/enemies
				auto &Friends = u.elf ? E : G;
				auto &Enemies = u.elf ? G : E;

				// Check win condition
				if (Enemies == ZERO) {
					return rounds * total_hp;
				}

				auto Self = Map(pos);

				// Get target squares (adjacent to enemies)
				auto T = Enemies.adjacent();
				if ((T & Self) == ZERO) { // Need to move
					T &= M;

					// Pathfind from self to identify nearest targets
					Map Path = Self, T_found;
					bool found_target = false;
					for (;;) {
						auto Pa = Path.adjacent();
						T_found = Pa & T;
						if (T_found != ZERO) {
							found_target = true;
							break;
						}
						Pa &= M;
						if (Pa == Path) { // No path found
							break;
						}
						Path = Pa;
					}
					if (!found_target) {
						continue;
					}

					// Pathfind from first target in reading order to
					// identify potential moves
					Path = T_found.first();
					T = Self.adjacent() & M;
					for (;;) {
						auto Pa = Path.adjacent();
						T_found = Pa & T;
						if (T_found != ZERO) {
							break;
						}
						Path = Pa & M;
					}

					// Choose first move in reading order
					auto pos_new = T_found.first();

					// Move
					std::swap(U_lookup[pos], U_lookup[pos_new]);
					M.toggle(pos); Friends.toggle(pos);
					pos = pos_new;
					M.toggle(pos); Friends.toggle(pos);
				}

				// Choose an adjacent target to attack
				uint16_t v_pos = 0xffff;
				uint8_t best_hp = 0xff;
				for (auto offset : { -32, -1, 1, 32 }) {
					auto v_idx = U_lookup[pos + offset];
					if (v_idx == 0xff) {
						continue;
					}
					auto &v = U[v_idx];
					if (v.elf != u.elf && v.hp < best_hp) {
						v_pos = pos + offset;
						best_hp = v.hp;
					}
				}
				if (v_pos == 0xffff) { // Nobody to attack
					continue;
				}

				auto &v = U[U_lookup[v_pos]];
				auto damage = power[u.elf];
				if (damage < v.hp) {
					total_hp -= damage;
					v.hp -= damage;
				} else { // Kill
					if (Part2 && v.elf) {
						return 0;
					}

					total_hp -= v.hp;
					v.hp = 0;
					U_lookup[v_pos] = 0xff;
					M.toggle(v_pos); Enemies.toggle(v_pos);

					// Remove victim from initiative
					for (auto&& k : U_pos) {
						if (k == v_pos) {
							k = 0xffff;
							break;
						}
					}
				}
			}
		}
	}
};

}

void day15(input_t input) {
	Day15 d15(input);

	int part1 = Day15(d15).solve<false>();
	int part2 = 0;
	for (uint8_t elf_power = 4; !part2; elf_power++) {
		part2 = Day15(d15).solve<true>(elf_power);
	}
	printf("Day 15 Part 1: %d\nDay 15 Part 2: %d\n", part1, part2);
}
