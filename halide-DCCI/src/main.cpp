#pragma once
#include "common.h"
#include "experiments.h"

using namespace std;
using namespace Halide;

int main()
{
	// test
	if (false)
	{
		testPSNR();
	}

	// test ssim
	if (false)
	{
		testSSIM();
	}

	// createImage
	if (false)
	{
		createImage();
	}

	// benchmark
	if (false)
	{
		benchmark();
	}

	// local optimize psnr
	if (false)
	{
		searchDCCILocalOptimizePSNR();
	}

	// local optimize ssim
	if (false)
	{
		searchDCCILocalOptimizeSSIM();
	}
}
