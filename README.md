# advent2018-fast

[Advent of Code 2018](http://adventofcode.com/2018/) optimized C++ solutions.

Here are the timings from an example run on an i7-7700K CPU overclocked at 4.60 GHz.  The total is greater than the sum of the individual days because of rounding.

    Day  1       72 μs
    Day  2       27 μs
    Day  3      205 μs
    Day  4       82 μs
    Day  5      414 μs
    Day  6    1,346 μs
    Day  7        3 μs
    Day  8       62 μs
    Day  9    4,385 μs
    Day 10       21 μs
    Day 11      485 μs
    Day 12       61 μs
    Day 13      642 μs
    Day 14   19,067 μs
    Day 15    2,132 μs
    Day 16       80 μs
    Day 17      544 μs
    Day 18      307 μs
    Day 19        2 μs
    Day 20       92 μs
    Day 21      101 μs
    Day 22    3,546 μs
    Day 23      383 μs
    Day 24    6,711 μs
    Day 25      177 μs
    ------------------
    Total    40,959 μs

Solutions should work with any puzzle input, provided it is byte-for-byte an exact copy of the file downloaded from Advent of Code.  When an input is given as a number inline with the puzzle text (days 11 and 14), the input file should contain only the number itself followed by a single line feed character (ASCII 12).

This code makes extensive use of SIMD techniques and requires a CPU that supports the AVX2 instruction set.

# Summary of solutions

Here are a few brief notes about each solution.

## Day 1

See [this post](https://www.reddit.com/r/adventofcode/comments/a20646/2018_day_1_solutions/eaukxu5) on the subreddit.

## Day 2

Each 26-character word fits in a 32-byte AVX2 register, which allows for very fast comparisons.  Exploits regularities in the input to eliminate many candidate words from consideration.

## Day 3

Represents a row of fabric as a 1024-bit mask (16 64-bit integers.)  Scans the fabric vertically one row at a time using a sweep-line algorithm, considering only those claims which intersect the current row, 20-25 claims on average.

## Day 4

SIMD solution that stores the 60 minutes of the hour in two 32x8 bit AVX2 vectors.

## Day 5

Straightforward stack-based solution.  Tried various ways to prune the Part 2 search, but the added bookkeeping outweighed the time saved.

## Day 6

Part 1 is a basic flood fill; part 2 is a sweep-line algorithm that traces the outline of the "safe" region.

## Day 7

A SIMD take on topological sorting.  The letters A-Z fit nicely within a 32x8 AVX2 vector.

## Day 8

Straightforward recursive solution.

## Day 9

A shuffle/permute heavy SIMD implementation of the marble game's rules, in 23-turn increments.  Only needs to fully simulate ~43% of the game; the score for the remainder of the game can be found by tallying up every 16th marble in the array.

## Day 10

Calculates Part 2 based on the fastest-moving particles in the y-direction.  The final position of each star is added to a bitmap of eight 64-bit integers (each letter is 6x10.)  The resulting values are converted to letters by hashtable lookup.

## Day 11

Builds an integral image (summed area table) of the 300x300 area, and does so in a way that the compiler can vectorize.  Searching for the solutions is done via explicit SIMD.  Stops the search at "6 sigmas" of confidence.

## Day 12

Live/dead cells are stored as a bool vector, which C++ implements as a bitset.  Comparing the previous and current states is very fast; my input stabilizes at a vector length of 190, which is only 3 64-bit integers.  The rules are stored as a single 32-bit integer, and are evaluated by iterating over the current state with a 5-bit rolling window, which is used to index individual bits in the rules integer.

## Day 13

Fairly straightforward simulation of the minecarts.

## Day 14

This was the hardest puzzle to optimize.  The irregular write pattern limits the ability to vectorize the loop.  See [this post](https://www.reddit.com/r/adventofcode/comments/a6wpwa/2018_day_14_breaking_the_1_billion_recipes_per/) for a detailed description.

## Day 15

The 32x32 map fits in four 256-bit AVX2 registers when treated as a bitmap.  This makes for quick pathfinding by bitwise flood fill.  The map is split into three 1024-bit overlays: open space, elves, and goblins.

## Day 16

Mostly straightforward, using bit fields to keep track of which opcodes are valid for which instructions.

## Day 17

Recursively traces where the water flows.  I didn't spend much time trying to optimizing this; it is mostly just a copy/paste from my original solution repository.

## Day 18

A SIMD bonanza, even when "parsing" the input (which just does a SIMD bitwise AND with `0x11` on 32 input characters at a time to produce `0x00` for open space, `0x10` for trees, and `0x01` for lumberyards.)  This is actually a SIMD/SWAR hybrid, because the trees and lumberyards each occupy 4-bit fields within each 8-bit field of the 32-byte AVX2 register.

## Day 19

Executes instructions until it reaches the main loop.  Uses wheel factorization to find the prime factors, which are used to compute the divisor sum.

## Day 20

Exploits how the input was generated to solve both parts using a small stack of coordinate/distance and an even smaller cache of recently visited coordinates.

## Day 21

Reads only one number from the input (the only one that matters), and uses it as input to an optimized SIMD version of the generator.  Uses Brent's cycle-detection algorithm to solve Part 2.

## Day 22

Very minimalistic A\* using a 17-bucket priority queue.  Tool switching is done by bitwise arithmetic between tool and terrain.

## Day 23

See [this discussion](https://www.reddit.com/r/adventofcode/comments/a9co1u/day_23_part_2_adversarial_input_for_recursive/ecmpxad) on the subreddit.

## Day 24

Predetermines which groups are valid targets of each other, keeping a separate list of targets that are weak to the attack type.  These lists are stored as bitmasks.  Avoids sorting the array of attackers each round; instead, when a group is damaged, it bubbles down the target-selection list (they rarely move more than 1-2 slots at a time, if any.)  Otherwise, this is just a straight simulation of the battles.

## Day 25

Union-find algorithm, in cooperation with SIMD pairwise comparisons between each of the points.
