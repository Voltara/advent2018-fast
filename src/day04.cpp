#include "advent2018.h"

namespace {

struct event_t {
	int64_t date, event;
	event_t(int64_t date, int64_t event) : date(date), event(event) { }
	bool operator < (const event_t &o) const {
		return date < o.date;
	}
};

struct tally_t {
	__m256i t[2];
	tally_t() : t() { }
};

}

static const __m256i identity = _mm256_set_epi64x(
		0x1f1e1d1c1b1a1918, 0x1716151413121110,
		0x0f0e0d0c0b0a0908, 0x0706050403020100);

// Set all values to the vector max value
static __m256i horizontal_max(__m256i v) {
	v = _mm256_max_epu8(v, _mm256_alignr_epi8(v, v, 8));
	v = _mm256_max_epu8(v, _mm256_alignr_epi8(v, v, 4));
	v = _mm256_max_epu8(v, _mm256_alignr_epi8(v, v, 2));
	v = _mm256_max_epu8(v, _mm256_alignr_epi8(v, v, 1));
	return _mm256_max_epu8(v, _mm256_permute2x128_si256(v, v, 0x01));
}

void day04(input_t input) {
	std::vector<event_t> E;
	std::unordered_map<int64_t, int64_t> G;
	std::vector<int64_t> Gi;

	char *s = input.s;
	int64_t date = 0, guard = 0, *np = &date;
	for (; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			*np = 10 * *np + c;
		} else if (*input.s == '#') {
			*(np = &guard) = 0;
		} else if (*input.s == '\n') {
			if (input.s - s < 27) abort();
			switch (s[19]) {
			    case 'G':
				{
					auto [ it, ok ] = G.emplace(guard, G.size());
					if (ok) {
						Gi.push_back(guard);
					}
					E.emplace_back(date, it->second);
					break;
				}
			    case 'f':
				{
					E.emplace_back(date, -1);
					break;
				}
			    case 'w':
				{
					E.emplace_back(date, -2);
					break;
				}
			}
			s = input.s + 1;
			*(np = &date) = 0;
		}
	}

	std::vector<tally_t> T(G.size());
	std::vector<int64_t> S(G.size());

	int64_t cur_guard = 0, sleep_time = 0;
	std::sort(E.begin(), E.end());
	for (auto &e : E) {
		switch (e.event) {
		    case -2:
			{
				// Tally up the minutes spent sleeping by the guard
				auto wake_time = e.date % 100;
				__m256i sleep0 = _mm256_and_si256(
						_mm256_cmpgt_epi8(identity, _mm256_set1_epi8(sleep_time - 1)),
						_mm256_cmpgt_epi8(_mm256_set1_epi8(wake_time), identity));
				__m256i sleep1 = _mm256_and_si256(
						_mm256_cmpgt_epi8(identity, _mm256_set1_epi8(sleep_time - 33)),
						_mm256_cmpgt_epi8(_mm256_set1_epi8(wake_time - 32), identity));
				T[cur_guard].t[0] = _mm256_sub_epi8(T[cur_guard].t[0], sleep0);
				T[cur_guard].t[1] = _mm256_sub_epi8(T[cur_guard].t[1], sleep1);
				S[cur_guard] += wake_time - sleep_time;
				break;
			}
		    case -1:
			{
				sleep_time = e.date % 100;
				break;
			}
		    default:
			{
				cur_guard = e.event;
				break;
			}
		}
	}

	// Find guard who sleeps the most
	int64_t max_guard = 0;
	for (int i = 1; i < G.size(); i++) {
		if (S[i] > S[max_guard]) {
			max_guard = i;
		}
	}

	// Find that guard's max minute
	auto maxv = horizontal_max(_mm256_max_epu8(T[max_guard].t[0], T[max_guard].t[1]));
	auto cmp = uint64_t(_mm256_movemask_epi8(_mm256_cmpeq_epi8(maxv, T[max_guard].t[1]))) << 32 |
	                    _mm256_movemask_epi8(_mm256_cmpeq_epi8(maxv, T[max_guard].t[0]));

	int part1 = Gi[max_guard] * _mm_tzcnt_64(cmp);

	// Find max minute across all guards
	maxv = _mm256_setzero_si256();
	for (auto &t : T) {
		maxv = _mm256_max_epu8(maxv, t.t[0]);
		maxv = _mm256_max_epu8(maxv, t.t[1]);
	}
	maxv = horizontal_max(maxv);

	// Find the guard who had that max minute
	int part2 = 0;
	for (int i = 0; i < G.size(); i++) {
		auto cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(maxv, T[i].t[0]));
		if (cmp) {
			part2 = Gi[i] * _mm_tzcnt_32(cmp);
			break;
		}
		cmp = _mm256_movemask_epi8(_mm256_cmpeq_epi8(maxv, T[i].t[1]));
		if (cmp) {
			part2 = Gi[i] * (_mm_tzcnt_32(cmp) + 32);
			break;
		}
	}

	printf("Day 04 Part 1: %d\nDay 04 Part 2: %d\n", part1, part2);
}
