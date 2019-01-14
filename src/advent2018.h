#ifndef _ADVENT2018_H
#define _ADVENT2018_H

#include <string>
#include <array>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdint>
#include <climits>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <x86intrin.h>

/* Need this to safely do horrible things such as taking a
 * std::vector<int8_t> and casting it to __m256i for SIMD
 */
#include <boost/align/aligned_allocator.hpp>
using boost::alignment::aligned_allocator;

struct input_t {
	char *s;
	ssize_t len;
};

struct advent_t {
	void (*fn)(input_t);
	std::string input_file;
};

void day01(input_t);
void day02(input_t);
void day03(input_t);
void day04(input_t);
void day05(input_t);
void day06(input_t);
void day07(input_t);
void day08(input_t);
void day09(input_t);
void day10(input_t);
void day11(input_t);
void day12(input_t);
void day13(input_t);
void day14(input_t);
void day15(input_t);
void day16(input_t);
void day17(input_t);
void day18(input_t);
void day19(input_t);
void day20(input_t);
void day21(input_t);
void day22(input_t);
void day23(input_t);
void day24(input_t);
void day25(input_t);

#endif
