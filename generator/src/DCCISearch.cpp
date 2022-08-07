#pragma once
#include <Halide.h>
#include "DCCIUtils.h"
#include "AccessPatternGenerator/AccessPatternGenerator.h"

using namespace Halide;
using namespace SearchSupport;

class DCCISearch : public Halide::Generator<DCCISearch>
{
public:
	GeneratorParam<float> threshold{ "threshold", 1.15f };

	Input<Buffer<uint8_t>> input{ "input", 3 };
	Input<int> flagA{ "flagA", pow(2, 14) - 1 };
	Input<int> flagB{ "flagB", pow(2, 14) - 1 };
	Input<int> flagC{ "flagC", pow(2, 14) - 1 };
	Input<int> flagD{ "flagD", pow(2, 14) - 1 };
	Output<Buffer<uint8_t>> output{ "output", 3 };

	void generate()
	{
		Var x("x"), y("y"), c("c");

		Func clamped("clamped");
		clamped = BoundaryConditions::repeat_edge(input);

		Func input32F("input32F");
		input32F(x, y, c) = cast<float>(clamped(x, y, c) / Expr(255.f));

		Func initialized("initialize");
		initialized(x, y, c) = select(
			x % 2 == 0 && y % 2 == 0, input32F(x / 2, y / 2, c),
			0.f
		);

		Func step1("step1");
		{
			Expr G1 = calcGradUpRight(initialized); // 45度方向の勾配
			Expr G2 = calcGradDownRight(initialized); // 135度方向の勾配

			Expr W1 = calcWeight(G1);
			Expr W2 = calcWeight(G2);

			Expr normalized_W1 = W1 / (W1 + W2);
			Expr normalized_W2 = W2 / (W1 + W2);

			Func kernel_W1;
			kernel_W1(x, y) = select(
				(1.f + G1) / (1.f + G2) > threshold, 0.f,
				(1.f + G2) / (1.f + G1) > threshold, 1.f,
				normalized_W1
			);
			Func kernel_W2;
			kernel_W2(x, y) = select(
				(1.f + G1) / (1.f + G2) > threshold, 1.f,
				(1.f + G2) / (1.f + G1) > threshold, 0.f,
				normalized_W2
			);

			Func base_step1;
			base_step1(x, y) = select(
				x % 2 == 1 && y % 2 == 1, calcSinc(sqrt(x * x + y * y) / (2.f * sqrt(2.f))),
				0.f
				);
			AccessPatternGenerator<float> generatorA;
			generatorA(x, y) = base_step1(x, y);
			generatorA(-3, -3) = 0.f;
			generatorA(+3, +3) = 0.f;
			AccessPatternGenerator<float> generatorB;
			generatorB(x, y) = base_step1(x, y);
			generatorB(-3, +3) = 0.f;
			generatorB(+3, -3) = 0.f;

			Func kernel1 = generatorA.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagA).get_generated_kernel();
			Func kernel2 = generatorB.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagB).get_generated_kernel();

			RDom r(-3, 7, -3, 7);
			Expr kernel_step1 = kernel1(r.x, r.y) * kernel_W1(x, y) + kernel2(r.x, r.y) * kernel_W2(x, y);

			Func oddOdd("oddOdd");
			oddOdd(x, y, c) = sum(initialized(x + r.x, y + r.y, c) * kernel_step1);

			step1(x, y, c) = select(
				x % 2 == 0 && y % 2 == 0, initialized(x, y, c),
				x % 2 == 1 && y % 2 == 1, oddOdd(x, y, c),
				0.f
			);
		}

		Func step2("step2");
		{
			Expr G1 = calcGradHorizontal(step1);
			Expr G2 = calcGradVertical(step1);

			Expr W1 = calcWeight(G1);
			Expr W2 = calcWeight(G2);

			Expr normalized_W1 = W1 / (W1 + W2);
			Expr normalized_W2 = W2 / (W1 + W2);

			Func kernel_W1;
			kernel_W1(x, y) = select(
				(1.f + G1) / (1.f + G2) > threshold, 0.f,
				(1.f + G2) / (1.f + G1) > threshold, 1.f,
				normalized_W1
			);
			Func kernel_W2;
			kernel_W2(x, y) = select(
				(1.f + G1) / (1.f + G2) > threshold, 1.f,
				(1.f + G2) / (1.f + G1) > threshold, 0.f,
				normalized_W2
			);

			Func base_step2;
			base_step2(x, y) = select(
				abs(x) + abs(y) > 3, 0.f,
				(x + y) % 2 == 1, calcSinc(sqrt(x * x + y * y) / (2.f)),
				0.f
			);
			AccessPatternGenerator<float> generatorC;
			generatorC(x, y) = base_step2(x, y);
			generatorC(0, -3) = 0.f;
			generatorC(0, +3) = 0.f;
			AccessPatternGenerator<float> generatorD;
			generatorD(x, y) = base_step2(x, y);
			generatorD(-3, 0) = 0.f;
			generatorD(+3, 0) = 0.f;

			Func evenOdd;
			{
				Func kernel1 = generatorC.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagC).get_generated_kernel();
				Func kernel2 = generatorD.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagD).get_generated_kernel();

				RDom r(-3, 7, -3, 7);
				Expr kernel_step2 = kernel1(r.x, r.y) * kernel_W1(x, y) + kernel2(r.x, r.y) * kernel_W2(x, y);

				evenOdd(x, y, c) = sum(step1(x + r.x, y + r.y, c) * kernel_step2);
			}

			Func oddEven;
			{
				Func kernel1 = generatorD.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagD).transpose().get_access_pattern_kernel();
				Func kernel2 = generatorC.set_mins(-3, -3).set_extents(7, 7).set_LUT_threshold(FLT_EPSILON).generate(flagC).transpose().get_access_pattern_kernel();

				RDom r(-3, 7, -3, 7);
				Expr kernel_step2 = kernel1(r.x, r.y) * kernel_W1(x, y) + kernel2(r.x, r.y) * kernel_W2(x, y);

				oddEven(x, y, c) = sum(step1(x + r.x, y + r.y, c) * kernel_step2);
			}

			step2(x, y, c) = select(
				x % 2 == 0 && y % 2 == 0, step1(x, y, c),
				x % 2 == 1 && y % 2 == 1, step1(x, y, c),
				x % 2 == 0 && y % 2 == 1, evenOdd(x, y, c),
				x % 2 == 1 && y % 2 == 0, oddEven(x, y, c),
				0.f
			);
		}

		output(x, y, c) = saturating_cast<uint8_t>(round(step2(x, y, c) * 255.f));
	}

	void schedule()
	{
		if (auto_schedule)
		{
			input.set_estimates({ {0, 256},{0, 256},{0, 3} });
			output.set_estimates({ {0, 512},{0, 512},{0, 3} });

			flagA.set_estimate(pow(2, 14) - 1);
			flagB.set_estimate(pow(2, 14) - 1);
			flagC.set_estimate(pow(2, 14) - 1);
			flagD.set_estimate(pow(2, 14) - 1);
		}
	}
};


HALIDE_REGISTER_GENERATOR(DCCISearch, halide_dcci_search_gen)
