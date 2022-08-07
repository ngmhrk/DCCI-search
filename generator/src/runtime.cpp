#include <Halide.h>

using namespace Halide;

/// <summary>
/// HalideのRuntimeを生成するためだけに作成したGenerator
/// 機能としてはただのコピー(呼び出すつもりはない)
/// </summary>
class RuntimeGenerator : public Halide::Generator<RuntimeGenerator>
{
public:
	Input<Buffer<uint8_t>> input{ "input", 3 };
	Output<Buffer<uint8_t>> output{ "output", 3 };


	void generate()
	{
		output(x, y, c) = input(x, y, c);
	}
private:
	Var x{ "x" }, y{ "y" }, c{ "c" };
};

HALIDE_REGISTER_GENERATOR(RuntimeGenerator, halide_runtime_gen)