#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include "advent2018.h"

// Allows solutions to read past the end of the input safely
static constexpr size_t BACKSPLASH_SIZE = 1 << 20;

static const advent_t advent2018[] = {
	{ day01, "input/day01.txt" },
	{ day02, "input/day02.txt" },
	{ day03, "input/day03.txt" },
	{ day04, "input/day04.txt" },
	{ day05, "input/day05.txt" },
	{ day06, "input/day06.txt" },
	{ day07, "input/day07.txt" },
	{ day08, "input/day08.txt" },
	{ day09, "input/day09.txt" },
	{ day10, "input/day10.txt" },
	{ day11, "input/day11.txt" },
	{ day12, "input/day12.txt" },
	{ day13, "input/day13.txt" },
	{ day14, "input/day14.txt" },
	{ day15, "input/day15.txt" },
	{ day16, "input/day16.txt" },
	{ day17, "input/day17.txt" },
	{ day18, "input/day18.txt" },
	{ day19, "input/day19.txt" },
	{ day20, "input/day20.txt" },
	{ day21, "input/day21.txt" },
	{ day22, "input/day22.txt" },
	{ day23, "input/day23.txt" },
	{ day24, "input/day24.txt" },
	{ day25, "input/day25.txt" },
};

static void load_input(input_t &input, const std::string &filename);
static void free_input(input_t &input);

int main() {
	double total_time = 0;

	for (auto &A : advent2018) {
		input_t input;

		load_input(input, A.input_file);
		auto t0 = std::chrono::steady_clock::now();
		(*A.fn)(input);
		auto elapsed = std::chrono::steady_clock::now() - t0;
		free_input(input);

		total_time += elapsed.count();
		printf("[%ld μs]\n", int64_t(elapsed.count() * 1e-3));
	}
	printf("Total: %ld μs\n", int64_t(total_time * 1e-3));

	return 0;
}

void load_input(input_t &input, const std::string &filename) {
	static void *backsplash = NULL;

	backsplash = mmap(backsplash, BACKSPLASH_SIZE, PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS|(backsplash ? MAP_FIXED : 0), -1, 0);
	if (backsplash == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	int fd = open(filename.c_str(), O_RDONLY);
	if (fd == -1) {
		perror(filename.c_str());
		exit(EXIT_FAILURE);
	}
	input.len = lseek(fd, 0, SEEK_END);
	if (input.len > BACKSPLASH_SIZE) {
		fprintf(stderr, "Why is your input so big?\n");
		exit(EXIT_FAILURE);
	}
	input.s = reinterpret_cast<char *>(mmap(backsplash, input.len, PROT_READ, MAP_PRIVATE|MAP_FIXED, fd, 0));
	if (input.s == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	if (input.s != backsplash) {
		fprintf(stderr, "Warning: Input not mapped at the expected location.\n");
	}
	if (close(fd) == -1) {
		perror(filename.c_str());
		exit(EXIT_FAILURE);
	}
}

void free_input(input_t &input) {
	if (munmap(input.s, input.len) == -1) {
		perror("munmap");
		exit(EXIT_FAILURE);
	}
}
