#include "AccessPatternGenerator.h"

using namespace Halide;
using namespace std;

namespace SearchSupport
{
	template<class T>
	Halide::Buffer<int> create_LUT(Halide::Buffer<T>& kernel, T threshold)
	{
		Halide::RDom r(kernel);
		Halide::Func lut_size_f;

		lut_size_f() = Halide::sum(
			Halide::select(
				Halide::abs(kernel(r.x, r.y)) > threshold, 1,
				0
			)
		);
		Halide::Buffer<int> lut_size = lut_size_f.realize();
		Halide::Buffer<int> LUT(lut_size(), 2);

		int lut_index = 0;
		for (int y = kernel.dim(1).min(); y <= kernel.dim(1).max(); y++)
		{
			for (int x = kernel.dim(0).min(); x <= kernel.dim(0).max(); x++)
			{
				if (abs(kernel(x, y)) > threshold)
				{
					LUT(lut_index, 0) = x;
					LUT(lut_index, 1) = y;
					lut_index++;
				}
			}
		}
		return LUT;
	}

	template<class T>
	Halide::Func generate_pattern(Halide::Buffer<T>& kernel, Halide::Buffer<int>& LUT, Halide::Expr& flag)
	{
		Halide::Var x, y;
		Halide::Expr _x, _y;
		Halide::Func generated;
		Halide::RDom r(0, LUT.dim(0).extent());

		r.where((flag & (1 << r)) > 0);

		_x = Halide::clamp(LUT(r, 0), kernel.dim(0).min(), kernel.dim(0).max());
		_y = Halide::clamp(LUT(r, 1), kernel.dim(1).min(), kernel.dim(1).max());

		generated(x, y) = Halide::cast<T>(0);
		generated(_x, _y) = kernel(_x, _y) / Halide::sum(kernel(_x, _y));

		return generated;
	}

	template<class T>
	Halide::Func reduce_convolution(Halide::Buffer<T>& kernel, Halide::Buffer<int>& LUT, Halide::Func& src)
	{
		Halide::Var x, y, c;
		Halide::Expr _x, _y;
		Halide::RDom r(0, LUT.dim(0).extent());
		Halide::Func interpolated;

		_x = Halide::clamp(LUT(r, 0), kernel.dim(0).min(), kernel.dim(0).max());
		_y = Halide::clamp(LUT(r, 1), kernel.dim(1).min(), kernel.dim(1).max());

		interpolated(x, y, c) = Halide::sum(kernel(_x, _y) * src(x + _x, y + _y, c));

		return interpolated;
	}

	template<class T>
	AccessPatternGenerator<T>::AccessPatternGenerator()
		: base_kernel(), generated_kernel(), access_pattern_kernel(), x(), y(), x_min(0), y_min(0), x_extent(-1), y_extent(-1), LUT_threshold(-1)
	{
	}

	template<class T>
	void AccessPatternGenerator<T>::set_args(Var x, Var y)
	{
		this->x = x;
		this->y = y;
	}


	template<class T>
	AccessPatternGenerator<T>& AccessPatternGenerator<T>::set_mins(int x_min, int y_min)
	{
		this->x_min = x_min;
		this->y_min = y_min;
		return *this;
	}

	template<class T>
	AccessPatternGenerator<T>& AccessPatternGenerator<T>::set_extents(int x_extent, int y_extent)
	{
		if (x_extent <= 0 || y_extent <= 0)
		{
			cerr << "x_extent and y_extent must be positive value" << endl;
			assert(false);
		}
		this->x_extent = x_extent;
		this->y_extent = y_extent;
		return *this;
	}

	template<class T>
	AccessPatternGenerator<T>& AccessPatternGenerator<T>::set_LUT_threshold(T threshold)
	{
		if (threshold < 0)
		{
			cerr << "threshold must be positive value" << endl;
			assert(false);
		}
		this->LUT_threshold = threshold;
		return *this;
	}

	template<class T>
	AccessPatternGenerator<T>& AccessPatternGenerator<T>::transpose()
	{
		access_pattern_kernel(x, y) = generated_kernel(y, x);
		return *this;
	}

	template<class T>
	AccessPatternGenerator<T>& AccessPatternGenerator<T>::generate(Halide::Expr flag)
	{
		if (x_extent == -1 || y_extent == -1)
		{
			cerr << "Call set_extents(). generate() requires kernel extents." << endl;
			assert(false);
		}

		if (LUT_threshold < 0)
		{
			cerr << "Call set_LUT_threshold(). generate() requires LUT threshold." << endl;
			assert(false);
		}

		Halide::Buffer<T> base_buffer(x_extent, y_extent);
		base_buffer.set_min(x_min, y_min);
		base_kernel.realize(base_buffer);

		Halide::Buffer<int> base_LUT = create_LUT(base_buffer, LUT_threshold);
		generated_kernel = generate_pattern(base_buffer, base_LUT, flag);

		return *this;
	}

	template<class T>
	Halide::Func AccessPatternGenerator<T>::convolution(Halide::Func src, bool is_reduce)
	{
		if (x_extent == -1 || y_extent == -1)
		{
			cerr << "Call set_extents(). convolution() requires kernel extents." << endl;
			assert(false);
		}

		if (LUT_threshold < 0)
		{
			cerr << "Call set_LUT_threshold(). convolution() requires LUT threshold." << endl;
			assert(false);
		}

		if (!generated_kernel.defined())
		{
			cerr << "Call generate(). convolution() requires generated access pattern." << endl;
			assert(false);
		}

		if (!access_pattern_kernel.defined())
		{
			access_pattern_kernel(x, y) = generated_kernel(x, y);
		}

		if (is_reduce)
		{
			Halide::Buffer<T> access_pattern_buffer(x_extent, y_extent);
			access_pattern_buffer.set_min(x_min, y_min);
			access_pattern_kernel.realize(access_pattern_buffer);

			Halide::Buffer<int> access_pattern_LUT = create_LUT(access_pattern_buffer, LUT_threshold);
			return reduce_convolution(access_pattern_buffer, access_pattern_LUT, src);
		}
		else
		{
			RDom r(x_min, x_extent, y_min, y_extent);
			vector<Var> args = src.args();
			vector<Expr> expr_args(src.dimensions());
			for (int i = 0; i < args.size(); i++)
			{
				expr_args[i] = args[i];
			}
			expr_args[0] += r.x;
			expr_args[1] += r.y;

			Func convolution;
			convolution(args) = sum(src(expr_args) * access_pattern_kernel(r.x, r.y));
			return convolution;
		}
	}
}
