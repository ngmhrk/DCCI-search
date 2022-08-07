#include <Halide.h>

using namespace Halide;

class BoxFilter : public Generator<BoxFilter>
{
public:
	Input<Buffer<uint8_t>> src{"src", 2};
	Output<Buffer<uint8_t>> blur_y{"blur_y", 2};
	void generate()
	{
		Func clamped("clamped"), blur_x("blur_x");
		Var x("x"), y("y");
		RDom r(-1, 3, "r");

		// algorithm
		clamped = BoundaryConditions::repeat_edge(src);
		blur_x(x, y) = sum(clamped(x + r, y));
		blur_y(x, y) = sum(blur_x(x, y + r)) / 9;
	}
	void schedule()
	{
		if (auto_schedule)
		{
			src.set_estimates({ {0, 256},{0, 256} });
			blur_y.set_estimates({ {0, 512},{0, 512}, });
		}
	}
};

HALIDE_REGISTER_GENERATOR(BoxFilter, box_filter_gen)
