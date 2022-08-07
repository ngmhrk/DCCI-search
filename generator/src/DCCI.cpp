#pragma once
#include <Halide.h>
#include "DCCIUtils.h"

using namespace Halide;

class DCCI : public Halide::Generator<DCCI>
{
public:
	GeneratorParam<float> threshold{ "threshold", 1.15f };

	Input<Buffer<uint8_t>> input{ "input", 3 };
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

		const float CCiFilter[4] = { -1.f / 16.f, 9.f / 16.f, 9.f / 16.f, -1.f / 16.f };
		Func step1("step1");
		{
			Expr G1 = calcGradUpRight(initialized); // 45度方向の勾配
			Expr G2 = calcGradDownRight(initialized); // 135度方向の勾配

			Expr W1 = calcWeight(G1);
			Expr W2 = calcWeight(G2);

			Expr pixel1
				= initialized(x - 3, y + 3, c) * CCiFilter[0]
				+ initialized(x - 1, y + 1, c) * CCiFilter[1]
				+ initialized(x + 1, y - 1, c) * CCiFilter[2]
				+ initialized(x + 3, y - 3, c) * CCiFilter[3];

			Expr pixel2
				= initialized(x - 3, y - 3, c) * CCiFilter[0]
				+ initialized(x - 1, y - 1, c) * CCiFilter[1]
				+ initialized(x + 1, y + 1, c) * CCiFilter[2]
				+ initialized(x + 3, y + 3, c) * CCiFilter[3];

			Expr pxSmooth = (W1 * pixel1 + W2 * pixel2) / (W1 + W2);

			Func oddOdd("oddOdd");
			oddOdd(x, y, c) = select(
				(1.f + G1) / (1.f + G2) > threshold, pixel2,
				(1.f + G2) / (1.f + G1) > threshold, pixel1,
				pxSmooth
			);

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

			Expr pixel1
				= step1(x - 3, y, c) * CCiFilter[0]
				+ step1(x - 1, y, c) * CCiFilter[1]
				+ step1(x + 1, y, c) * CCiFilter[2]
				+ step1(x + 3, y, c) * CCiFilter[3];

			Expr pixel2
				= step1(x, y - 3, c) * CCiFilter[0]
				+ step1(x, y - 1, c) * CCiFilter[1]
				+ step1(x, y + 1, c) * CCiFilter[2]
				+ step1(x, y + 3, c) * CCiFilter[3];

			Expr pxSmooth = (W1 * pixel1 + W2 * pixel2) / (W1 + W2);

			Func evenOddAndOddEven("evenOddAndOddEven");
			evenOddAndOddEven(x, y, c) = select(
				(1.f + G1) / (1.f + G2) > threshold, pixel2,
				(1.f + G2) / (1.f + G1) > threshold, pixel1,
				pxSmooth
				);

			step2(x, y, c) = select(
				x % 2 == 0 && y % 2 == 0, step1(x, y, c),
				x % 2 == 1 && y % 2 == 1, step1(x, y, c),
				x % 2 == 0 && y % 2 == 1, evenOddAndOddEven(x, y, c),
				x % 2 == 1 && y % 2 == 0, evenOddAndOddEven(x, y, c),
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
		}
	}
};

HALIDE_REGISTER_GENERATOR(DCCI, halide_dcci_gen)
