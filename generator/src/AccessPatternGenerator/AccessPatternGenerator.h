#pragma once
#include <Halide.h>

namespace SearchSupport
{
	template<class T>
	Halide::Buffer<int> create_LUT(Halide::Buffer<T>& kernel, T threshold);

	template<class T>
	Halide::Func generate_pattern(Halide::Buffer<T>& kernel, Halide::Buffer<int>& LUT, Halide::Expr& flag);

	template<class T>
	Halide::Func reduce_convolution(Halide::Buffer<T>& kernel, Halide::Buffer<int>& LUT, Halide::Func& src);

	template<class T>
	class AccessPatternGenerator
	{
	private:
		Halide::Func base_kernel;
		Halide::Func generated_kernel;
		Halide::Func access_pattern_kernel;

		Halide::Var x, y;
		int x_min, y_min;
		int x_extent, y_extent;
		T LUT_threshold;

		void set_args(Halide::Var x, Halide::Var y);
	public:
		AccessPatternGenerator();
		Halide::FuncRef operator()(Halide::Var x, Halide::Var y)
		{
			set_args(x, y);
			return base_kernel(x, y);
		};
		Halide::FuncRef operator()(Halide::Expr x, Halide::Expr y)
		{
			return base_kernel(x, y);
		};
		AccessPatternGenerator& set_mins(int x_min, int y_min);
		AccessPatternGenerator& set_extents(int x_extent, int y_extent);
		AccessPatternGenerator& set_LUT_threshold(T threshold);

		AccessPatternGenerator& transpose();
		AccessPatternGenerator& generate(Halide::Expr flag);
		Halide::Func convolution(Halide::Func src, bool is_reduce=false);

		// for debug and test
		Halide::Func get_base_kernel() { return base_kernel; };
		Halide::Func get_generated_kernel() { return generated_kernel; };
		Halide::Func get_access_pattern_kernel() { return access_pattern_kernel; };
	};

	template Halide::Buffer<int> create_LUT<float>(Halide::Buffer<float>& kernel, float threshold);
	template Halide::Func generate_pattern<float>(Halide::Buffer<float>& kernel, Halide::Buffer<int>& LUT, Halide::Expr& flag);
	template Halide::Func reduce_convolution<float>(Halide::Buffer<float>& kernel, Halide::Buffer<int>& LUT, Halide::Func& src);

	template class AccessPatternGenerator<float>;
}
