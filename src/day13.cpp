#include "advent2018.h"

namespace {

struct Cart {
	int y, x, dir, state;
	Cart(int y, int x, int dir) : y(y), x(x), dir(dir), state() { }
	bool operator < (const Cart& o) {
		return (y == o.y) ? (x < o.x) : (y < o.y);
	}
};

}

void day13(input_t input) {
	static char G[256][256] = { };
	std::vector<Cart> C;

	int x = 0, y = 0;
	for (; input.len--; input.s++) {
		if (*input.s == '\n') {
			y++, x = 0;
		} else {
			// Bit 0x80 indicates the location of a minecart
			G[y][x] = *input.s ^ 0x80;
			switch (*input.s) {
			    case '^': C.emplace_back(y, x, 0); break;
			    case '>': C.emplace_back(y, x, 1); break;
			    case 'v': C.emplace_back(y, x, 2); break;
			    case '<': C.emplace_back(y, x, 3); break;
			    default: G[y][x] ^= 0x80;
			}
			x++;
		}
	}

	auto n_carts = C.size();
	Cart part1(0,0,0), part2(0,0,0);

	static constexpr int dy[] = { -1,  0,  1,  0 };
	static constexpr int dx[] = {  0,  1,  0, -1 };
	static constexpr int turn0[] = { 3, 2, 1, 0 };
	static constexpr int turn1[] = { 1, 0, 3, 2 };
	while (n_carts != 1) {
		// Establish move order and trim dead carts
		std::sort(C.begin(), C.end());
		while (C.back().state == -1) {
			C.pop_back();
		}

		for (auto&& c : C) {
			if (~G[c.y][c.x] & 0x80) {
				// Died earlier this turn, cleanup
				c.state = -1;
				c.y = INT_MAX;
				continue;
			}
			G[c.y][c.x] &= 0x7f;

			c.y += dy[c.dir];
			c.x += dx[c.dir];

			if (G[c.y][c.x] & 0x80) {
				// Crash
				G[c.y][c.x] &= 0x7f;
				c.state = -1;
				n_carts -= 2;
				if (!part1.state) {
					part1 = c;
				}
				c.y = INT_MAX;
			} else {
				switch (G[c.y][c.x]) {
				    case '\\': c.dir = turn0[c.dir]; break;
				    case '/':  c.dir = turn1[c.dir]; break;
				    case '+':
					       switch (c.state) {
						   case 0: c.state = 1; c.dir = (c.dir + 3) & 3; break;
						   case 1: c.state = 2; break;
						   case 2: c.state = 0; c.dir = (c.dir + 1) & 3; break;
					       }
					       break;
				}
				G[c.y][c.x] ^= 0x80;
			}
		}
	}

	for (auto&& c : C) {
		if (c.state == -1 || (~G[c.y][c.x] & 0x80)) {
			continue;
		}
		part2 = c;
		break;
	}

	printf("Day 13 Part 1: %d,%d\nDay 13 Part 2: %d,%d\n", part1.x, part1.y, part2.x, part2.y);
}
