#include "advent2018.h"

static constexpr size_t LEN = 27;

namespace {

union va_t {
	__m256i v;
	uint8_t a[32];
	va_t(__m256i v = _mm256_setzero_si256()) : v(v) { }
	operator const __m256i & () const { return v; }
};

}

static uint64_t hash(const __m256i &m);

void day02(input_t input) {
	std::vector<va_t> v;
	for (; (input.len -= LEN) >= 0; input.s += LEN) {
		v.push_back({});
		memcpy(&v.back(), input.s, LEN);
		v.back().a[LEN - 1] = 0;
	}

	/* Part 1 */
	int n_two = 0, n_three = 0;
	for (auto &&n : v) {
		va_t tmp = _mm256_and_si256(n, _mm256_set1_epi8(31));
		va_t counts;
		for (auto p = tmp.a, e = p + LEN - 1; p != e; p++) {
			counts.a[*p]++;
		}
		n_two += !!_mm256_movemask_epi8(
				_mm256_cmpeq_epi8(counts,
					_mm256_set1_epi8(2)));
		n_three += !!_mm256_movemask_epi8(
				_mm256_cmpeq_epi8(counts,
					_mm256_set1_epi8(3)));
	}
	auto part1 = n_two * n_three;

	/* Part 2 */
	// Hash two disjoint sections of each input word, and create a
	// vector of the hashes and array indices
	std::vector<uint64_t> hv;
	hv.reserve(v.size());
	for (size_t i = 0; i < v.size(); i++) {
		auto h0 = hash(_mm256_and_si256(v[i],
					_mm256_set1_epi16(0x00ff)));
		auto h1 = hash(_mm256_and_si256(v[i],
					_mm256_set1_epi16(0xff00)));
		hv.emplace_back((h0 << 32) | i);
		hv.emplace_back((h1 << 32) | i);
	}

	// Sort 'hv' by hash value using a 3-pass histogram sort
	std::vector<uint64_t> hv_tmp(hv.size());
	std::array<uint32_t, 2048> hist;
	for (int shift : { 32, 43, 54 }) {
		uint32_t total = 0;
		hist.fill(0);
		for (auto h : hv) hist[(h >> shift) & 0x3ff]++;
		for (auto &n : hist) total += std::exchange(n, total);
		for (auto h : hv) hv_tmp[hist[(h >> shift) & 0x3ff]++] = h;
		hv.swap(hv_tmp);
	}

	// Collect indexes for hashes that appear more than once
	std::vector<uint32_t> idxv;
	for (size_t i = 1; i < hv.size(); i++) {
		if ((hv[i - 1] >> 32) == (hv[i] >> 32)) {
			idxv.emplace_back(hv[i - 1]);
			idxv.emplace_back(hv[i]);
		}
	}

	// Sort and deduplicate the index list; this also ensures the
        // candidates remain in the same relative order
        std::sort(idxv.begin(), idxv.end());
        idxv.erase(std::unique(idxv.begin(), idxv.end()), idxv.end());

	// Remove all non-candidates from the input list
	decltype(v) vv;
	size_t new_size = 0;
	for (auto idx : idxv) {
		vv.emplace_back(v[idx]);
	}

	for (auto x = vv.begin(); x != vv.end(); x++) {
		for (auto y = x + 1; y != vv.end(); y++) {
			// Parallel compare 8x32 vector, convert to bitmask of differences
			auto cmp_ne = ~_mm256_movemask_epi8(_mm256_cmpeq_epi8(*x, *y));
			// Get lowest bit in difference mask
			auto diff1 = _blsi_u32(cmp_ne);
			// Was it the only difference?
			if (diff1 == cmp_ne) {
				auto s = reinterpret_cast<char *>(&*x);
				// Get the index of the difference
				auto diff_idx = _bit_scan_forward(diff1);
				// Output all but the difference
				printf("Day 02 Part 1: %u\nDay 02 Part 2: %.*s%s\n", part1, diff_idx, s, s + diff_idx + 1);
				return;
			}
		}
	}
}

uint64_t hash(const __m256i &m) {
	auto u64 = reinterpret_cast<const uint64_t *>(&m);
	uint64_t h = 0;
	for (int i = 0; i < 4; i++) {
		h = _mm_crc32_u64(h, u64[i]);
	}
	return h;
}
