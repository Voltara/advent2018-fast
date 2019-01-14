#include "advent2018.h"

static constexpr uint8_t  Xdim        = 128;
static constexpr uint8_t  Xmax        = 80;
static constexpr uint16_t Ymax_margin = 8;

static constexpr uint8_t ROCKY = 1;
static constexpr uint8_t TORCH = 2;

namespace {

struct Node {
	uint16_t cost, y;
	uint8_t  x, tool;

	Node() {
	}

	Node(uint8_t x, uint16_t y, uint16_t cost, uint8_t tool) :
		x(x), y(y), cost(cost), tool(tool)
	{
	}
};

struct Map {
	uint8_t terrain, tool_done;
	Map() : terrain(), tool_done() { }
};

}

void day22(input_t input) {
	std::array<uint16_t, 3> A = { 0, 0, 0 };
	for (uint16_t n = 0, i = 0; input.len-- && i < 3; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
		} else if (n) {
			std::swap(A[i++], n);
		}
	}

	uint16_t depth = A[0];
	uint8_t  Tx    = A[1];
	uint16_t Ty    = A[2];

	const uint16_t Ymax = Ty + Ymax_margin;
	Map map[Ymax][Xdim];

	// First row
	uint32_t r[Xmax];
	for (uint16_t x = 0, prev = depth; x < Xmax; x++) {
		map[0][x].terrain = 1 << (0x55555556 * prev >> 30);
		prev += 16807;
		prev -= -(prev >= 20183) & 20183;
		r[x] = prev;
	}

	// Subsequent rows
	uint16_t Tx1 = Tx - 1;
	for (uint16_t y = 1, x0 = depth + 7905; y < Ymax; y++) {
		for (uint16_t x = 0, prev = x0; x < Xmax; x++) {
			map[y][x].terrain = 1 << (0x55555556 * prev >> 30);
			if (y == Ty && x == Tx1) {
				prev = 0;
			}
			prev = (r[x] * prev + depth) % 20183;
			r[x] = prev;
		}
		x0 += 7905;
		x0 -= -(x0 >= 20183) & 20183;
	}

	// Part 1
	uint32_t part1 = 0;
	for (uint16_t y = 0; y <= Ty; y++) {
		for (uint16_t x = 0; x <= Tx; x++) {
			part1 += map[y][x].terrain >> 1;
		}
	}

	// Part 2 - Basically A*

	/* 17 bucket priority queue
	 *
	 * Cost+heurstic delta is always within 0..16
	 *    0 : Move toward goal
	 *        (cost+1, heuristic-1)
	 *   16 : Move away from goal, switch tool away from TORCH
	 *        (cost+8, heuristic+8)
	 */
	std::array<std::vector<Node>, 17> Q;

	auto expand = [&](const Node &cur, uint8_t x, uint16_t y, int away, uint8_t valid_tool) {
		if ((x < Xmax) & (y < Ymax)) {
			Node neighbor(x, y, cur.cost + 1, cur.tool);
			auto delta = away * 2;
			auto Mn = map[y][x];
			if (Mn.terrain == cur.tool) {
				neighbor.tool ^= valid_tool;
				neighbor.cost += 7;
				delta         += 7 & -(cur.tool      == TORCH);
				delta         += 7 & -(neighbor.tool != TORCH);
			}
			if (!(Mn.tool_done & neighbor.tool)) {
				Q[delta].push_back(neighbor);
			}
		}
	};

	auto &TOP = Q[0];
	for (auto&& q : Q) {
		q.reserve(2048);
	}
	TOP.emplace_back(0, 0, 0, TORCH);

	uint32_t part2 = 0;
	for (;;) {
		while (!TOP.empty()) {
			Node cur = TOP.back();
			TOP.pop_back();

			auto M = map[cur.y][cur.x];
			if (M.tool_done & cur.tool) {
				continue;
			}
			map[cur.y][cur.x].tool_done |= cur.tool;

			if ((cur.x == Tx) & (cur.y == Ty)) {
				part2 = (cur.tool == TORCH) ? cur.cost : cur.cost + 7;
				goto done;
			}

			uint8_t valid_tool = 7 ^ M.terrain;
			expand(cur, cur.x - 1, cur.y,     cur.x <= Tx, valid_tool);
			expand(cur, cur.x + 1, cur.y,     cur.x >= Tx, valid_tool);
			expand(cur, cur.x,     cur.y - 1, cur.y <= Ty, valid_tool);
			expand(cur, cur.x,     cur.y + 1, cur.y >= Ty, valid_tool);
		}

		// Rotate the queue
		for (size_t i = 1; i < Q.size(); i++) {
			Q[i - 1].swap(Q[i]);
		}
	}
done:
	printf("Day 22 Part 1: %u\nDay 22 Part 2: %u\n", part1, part2);
}
