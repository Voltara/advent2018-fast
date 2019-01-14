#include "advent2018.h"

static constexpr int SPRING_X = 500, SPRING_Y = 0;

enum Element { SAND, CLAY, WATER, FLOW, VOID };

namespace {

struct Vein {
	int x0, x1, y0, y1;

	// Default to a degenerate bounding box
	Vein() : x0(INT_MAX), x1(INT_MIN), y0(INT_MAX), y1(INT_MIN) {
	}

	Vein(char axis, int x0_, int y0_, int y1_) : x0(x0_), y0(y0_), y1(y1_) {
		// Convert to half-open intervals: [x0,x1) [y0,y1)
		x1 = x0 + 1;
		y1++;
		if (axis == 'y') {
			std::swap(x0, y0);
			std::swap(x1, y1);
		}
	}

	void envelop(const Vein &o) {
		x0 = std::min(x0, o.x0 - 1); // margin left
		x1 = std::max(x1, o.x1 + 1); // margin right
		y0 = std::min(y0, o.y0 - 1); // margin top
		y1 = std::max(y1, o.y1);
	}

	void rebase(const Vein &bbox) {
		x0 -= bbox.x0;
		x1 -= bbox.x0;
		y0 -= bbox.y0;
		y1 -= bbox.y0;
	}
};

class Map {
    private:
	unsigned dim_x, dim_y;
	std::vector< std::vector<Element> > E;

    public:
	Map(unsigned dim_x, int dim_y) : dim_x(dim_x), dim_y(dim_y) {
		for (int x = 0; x < dim_x; x++) {
			E.emplace_back(dim_y);
		}
	}

	void add_vein(Vein v) {
		for (int x = v.x0; x < v.x1; x++) {
			for (int y = v.y0; y < v.y1; y++) {
				set(x, y, CLAY);
			}
		}
	}

	bool in_bounds(unsigned x, unsigned y) const {
		return (x < dim_x) & (y < dim_y);
	}

	Element at(int x, int y) const {
		return in_bounds(x, y) ? E[x][y] : VOID;
	}

	void set(int x, int y, Element e) {
		if (in_bounds(x, y)) {
			E[x][y] = e;
		}
	}

	int answer(bool part2) const {
		int ans = 0;
		for (int x = 0; x < dim_x; x++) {
			for (int y = 1; y < dim_y; y++) {
				auto e = at(x, y);
				if (e == WATER || (!part2 && e == FLOW)) {
					ans++;
				}
			}
		}
		return ans;
	}
};

}

static void fill(Map &M, int x, int y, int dir) {
	if (M.at(x, y) == FLOW) {
		fill(M, x + dir, y, dir);
		M.set(x, y, WATER);
	}
}

static bool flow(Map &M, int x, int y, int dir = 0) {
	auto e = M.at(x, y);
	if (e != SAND) {
		return (e != CLAY) && (e != WATER);
	}

	M.set(x, y, FLOW);

	// Try to flow down
	bool leaky = flow(M, x, y + 1, 0);
	if (leaky) {
		return true;
	}

	// Down is not leaky, flow laterally
	leaky  = (dir <= 0) && flow(M, x - 1, y, -1);
	leaky |= (dir >= 0) && flow(M, x + 1, y,  1);
	if (leaky) {
		return true;
	}

	if (dir == 0) {
		// Entire layer is watertight, fill it up
		fill(M, x,     y, -1);
		fill(M, x + 1, y,  1);
	}

	return false;
}

void day17(input_t input) {
	// Parse the input into a vector of veins
	std::vector<Vein> V;
	std::array<int, 4> tmp;
	for (int i = 0, n = 0, have = 0; input.len--; input.s++) {
		uint8_t c = *input.s - '0';
		if (c < 10) {
			n = 10 * n + c;
			have = 1;
		} else if (have) {
			tmp[i] = n;
			n = have = 0;
			if (++i == 4) {
				V.emplace_back(tmp[0], tmp[1], tmp[2], tmp[3]);
				i = 0;
			}
		} else if (!i) {
			tmp[i++] = *input.s;
		}
	}

	// Get bounding box
	Vein bbox;
	for (auto&& v : V) {
		bbox.envelop(v);
	}

	// Convert all veins to 0,0 array base
	for (auto& v : V) {
		v.rebase(bbox);
	}

	// Initialize the map
	Map M(bbox.x1 - bbox.x0, bbox.y1 - bbox.y0);
	for (auto&& v : V) {
		M.add_vein(v);
	}

	// Spring forth
	flow(M, SPRING_X - bbox.x0, std::max(0, SPRING_Y - bbox.y0));

	printf("Day 17 Part 1: %d\nDay 17 Part 2: %d\n", M.answer(false), M.answer(true));
}
