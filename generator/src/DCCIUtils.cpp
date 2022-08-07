#include "DCCIUtils.h"

using namespace Halide;
using namespace std;

Halide::Func calcL2Norm(Halide::Func base, std::pair<Halide::Expr, Halide::Expr> offsets)
{
	vector<Var> args = base.args();
	Func norm;
	norm(args[0], args[1]) = cast<float>(sqrt(
		pow(cast<double>(base(args[0], args[1], 0) - base(args[0] + offsets.first, args[1] + offsets.second, 0)), Expr(2.0)) +
		pow(cast<double>(base(args[0], args[1], 1) - base(args[0] + offsets.first, args[1] + offsets.second, 1)), Expr(2.0)) +
		pow(cast<double>(base(args[0], args[1], 2) - base(args[0] + offsets.first, args[1] + offsets.second, 2)), Expr(2.0))));
	return norm;
}

Halide::Expr calcGradUpRight(Halide::Func base)
{
	vector<Var> args = base.args();
	Func norm = calcL2Norm(base, {-2, 2});
	Expr grad = 0.f;

	int mn[3] = { 3, 1, -1 };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			grad += norm(args[0] + mn[i], args[1] - mn[j]);
		}
	}
	return grad;
}

Halide::Expr calcGradDownRight(Halide::Func base)
{
	vector<Var> args = base.args();
	Func norm = calcL2Norm(base, { -2, -2 });
	Expr grad = 0.f;

	int mn[3] = { 3, 1, -1 };
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			grad += norm(args[0] + mn[i], args[1] + mn[j]);
		}
	}
	return grad;
}

Halide::Expr calcGradHorizontal(Halide::Func base)
{
	vector<Var> args = base.args();
	Func norm = calcL2Norm(base, { 2, 0 });
	Expr grad = 0.f;

	{
		int m[2] = { 0, 2 };
		int n[2] = { -1, 1 };
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				grad += norm(args[0] - m[i], args[1] + n[j]);
			}
		}
	}
	{
		int n[3] = { -2, 0, 2 };
		for (int i = 0; i < 3; i++)
		{
			grad += norm(args[0] - 1, args[1] + n[i]);
		}
	}

	return grad;
}

Halide::Expr calcGradVertical(Halide::Func base)
{
	vector<Var> args = base.args();
	Func norm = calcL2Norm(base, { 0, 2 });
	Expr grad = 0.f;

	{
		int m[2] = { -1, 1 };
		int n[2] = { 0, 2 };
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				grad += norm(args[0] + m[i], args[1] - n[j]);
			}
		}
	}
	{
		int m[3] = { -2, 0, 2 };
		for (int i = 0; i < 3; i++)
		{
			grad += norm(args[0] + m[i], args[1] - 1);
		}
	}

	return grad;
}

Halide::Expr calcWeight(Halide::Expr grad)
{
	return cast<float>(Expr(1.0) / (Expr(1.0) + pow(cast<double>(grad), 5)));
}

Halide::Expr calcSinc(Halide::Expr x)
{
	Expr x1 = abs(x);
	Expr x2 = x1 * x1;
	Expr x3 = x2 * x1;
	float a = -0.5f;

	return select(
		x1 < 1.0f, (a + 2.0f)* x3 - (a + 3.0f) * x2 + 1,
		x1 < 2.0f, a* x3 - 5 * a * x2 + 8 * a * x1 - 4.0f * a,
		0.0f
	);
}
