#pragma once
#include <Halide.h>

Halide::Func calcL2Norm(Halide::Func base, std::pair<Halide::Expr, Halide::Expr> offsets);

Halide::Expr calcGradUpRight(Halide::Func base);
Halide::Expr calcGradDownRight(Halide::Func base);
Halide::Expr calcGradHorizontal(Halide::Func base);
Halide::Expr calcGradVertical(Halide::Func base);

Halide::Expr calcWeight(Halide::Expr grad);

Halide::Expr calcSinc(Halide::Expr x);
