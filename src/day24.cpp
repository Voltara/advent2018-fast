#include "advent2018.h"

static constexpr int N_TYPES = 5;

namespace {

struct Group {
	uint32_t units;
	uint32_t hp;
	uint32_t power;
	uint32_t initiative;
	uint32_t weak;
	uint32_t immune;
	uint16_t dmg;
	uint8_t mark;
	uint8_t faction;
	uint8_t type;
	bool operator < (const Group &o) const {
		return (power == o.power) ? initiative < o.initiative : power > o.power;
	}
};

}

static constexpr uint16_t operator "" _u(const char *str, const std::size_t len) {
	return uint16_t(str[0]) << 8 | str[1];
}

void day24(input_t input) {
	std::array<Group, 20> V = { };
	// Bitmask of alive groups within each faction
	std::array<uint32_t, 2> alive = { 0, 0 };

	uint32_t n = 0, faction = 0, k;
	uint16_t last = 0;
	std::array<uint32_t, 3> types = { 0, 0, 0 };
	Group tmp;
	for (; input.len--; input.s++) {
		last = (last << 8) | *input.s;
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (*input.s == '\n' && n) {
			n = 20 - n; // lower initiative is better
			alive[faction] |= (tmp.initiative = 1 << n);
			tmp.faction = faction;
			tmp.type   = _tzcnt_u32(types[0]);
			tmp.weak   = ~types[1];
			tmp.immune = ~types[2];
			tmp.power  = tmp.units * tmp.dmg;
			V[n] = tmp;
			types.fill(0);
			n = 0;
		} else switch (last) {
		    case " u"_u: tmp.units = n; n = 0; break;
		    case " h"_u: tmp.hp    = n; n = 0; break;
		    case "da"_u: tmp.dmg   = n; n = 0; break;
		    case "bl"_u: types[k] |=  1; break;
		    case "co"_u: types[k] |=  2; break;
		    case "fi"_u: types[k] |=  4; break;
		    case "ra"_u: types[k] |=  8; break;
		    case "sl"_u: types[k] |= 16; break;
		    case "tt"_u: k = 0; break;
		    case "we"_u: k = 1; break;
		    case "im"_u: k = 2; break;
		    case "n:"_u: faction = 1; break;
		}
	}

	// Generate vulnerability masks
	std::array<uint32_t, N_TYPES> weak = { }, vuln = { };
	for (int i = 0; i < N_TYPES; i++) {
		uint32_t m = 1 << i;
		for (auto &g : V) {
			if (~g.weak   & m) weak[i] |= g.initiative;
			if ( g.immune & m) vuln[i] |= g.initiative;
		}
	}

	auto orig_V = V;
	auto orig_alive = alive;

	auto fight = [&](bool part2) {
		uint32_t result = 0;

		alignas(__m256i) uint8_t Vidx[32] = {
			 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
			10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

		auto Vlast = Vidx + V.size() - 1;
		auto v_Vidx  = (__m256i *) Vidx;

		std::sort(Vidx, Vlast + 1, [&V](uint8_t a, uint8_t b) { return V[a] < V[b]; });

		for (;;) {
			if (!alive[0] || !alive[1]) {
				break;
			}

			// In decreasing order of effective power
			auto marks = alive;
			auto attackers = 0;
			for (auto vip = Vidx; vip <= Vlast; vip++) {
				auto &att = V[*vip];

				// the group to which it would deal the most damage
				auto m = marks[!att.faction];
				m &= (m & weak[att.type]) ? weak[att.type] : vuln[att.type];

				uint32_t best_power = 0;
				while (m) {
					auto d_idx = _tzcnt_u32(m);
					m = _blsr_u32(m);
					auto &def = V[d_idx];
					// Tie #1: largest effective power
					// Tie #2: highest initiative (implied by search order)
					if (def.power > best_power) {
						best_power = def.power;
						att.mark = d_idx;
					}
				}

				if (best_power) {
					attackers |= att.initiative;
					marks[!att.faction] ^= 1 << att.mark;
				}
			}

			// Attack
			uint32_t progress = false;
			for (auto m = attackers; m; ) {
				auto idx = _tzcnt_u32(m);
				m = _blsr_u32(m);
				auto &att = V[idx];
				auto &def = V[att.mark];
				auto dmg = att.power << !((1 << att.type) & def.weak);
				auto kills = std::min(dmg / def.hp, def.units);
				progress |= kills;
				def.units -= kills;
				def.power = def.units * def.dmg;
				if (!def.power) {
					alive[def.faction] &= ~def.initiative;
					m &= ~def.initiative;
				}
				if (kills) {
					// Bubble the defender down in target selection order
					auto i = _tzcnt_u32(_mm256_movemask_epi8(_mm256_cmpeq_epi8(*v_Vidx, _mm256_set1_epi8(att.mark))));
					for (auto p = Vidx + i; p != Vlast && V[p[1]] < V[p[0]]; p++) {
						std::swap(p[0], p[1]);
					}
					Vlast -= !def.power;
				}
			}

			if (!progress) {
				return result;
			}
		}

		if (part2 & !alive[0]) {
			return result;
		}

		for (auto &g : V) {
			result += g.units;
		}
		return result;
	};

	uint32_t part1 = fight(false), part2 = 0;
	do {
		alive = orig_alive;
		for (auto m = alive[0]; m; ) {
			auto &g = orig_V[_tzcnt_u32(m)];
			m = _blsr_u32(m);
			g.dmg++;
			g.power += g.units;
		}
		V = orig_V;
		part2 = fight(true);
	} while (!part2);

	printf("Day 24 Part 1: %u\nDay 24 Part 2: %u\n", part1, part2);
}
