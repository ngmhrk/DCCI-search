#pragma once
#include "common.h"
#include "utils.h"
#include "tools/halide_benchmark.h"

void DCCI_for_Confirmation(const cv::Mat& srcImg, cv::Mat& dstImg, const int n_pow, const float threshold, const int k_pow);
void DCCI_Scalar_32FC3(const cv::Mat& srcImg, cv::Mat& dstImg, const float threshold);

class DCCITestBase
{
protected:
	const cv::Mat* original8U = nullptr;
	cv::Mat src8U;
	cv::Mat dest8U;

	virtual void pre_psnr() { return; };
	virtual void pre_getDest() { return; };

public:
	DCCITestBase(const cv::Mat& original)
		:original8U(&original), src8U(original.rows / 2, original.cols / 2, original.type()), dest8U(original.size(), original.type())
	{
		assert(original.type() == CV_MAKETYPE(CV_8U, original.depth()));
		cv::resize(*original8U, src8U, src8U.size(), 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST);
	};

	virtual ~DCCITestBase() {};
	virtual void operator()() = 0;

	double benchmark(int iterations, int samples)
	{
		return Halide::Tools::benchmark(samples, iterations, [&]() {
			(*this)();
			});
	};
	double psnr()
	{
		pre_psnr();

		// 境界条件の違いによる差を吸収する場合
		//int border = 50;
		//cv::Rect rect(border, border, dest8U.cols - border, dest8U.rows - border);
		//return cv::PSNR(cv::Mat((*original8U)(rect)), cv::Mat(dest8U(rect)));

		return cv::PSNR(*original8U, dest8U);
	};
	virtual double ssim() = 0;

	cv::Mat& getDest()
	{
		pre_getDest();
		return dest8U;
	}
};

class DCCITestCVBase : public DCCITestBase
{
protected:
	// AVXの実装に合わせて以下はCV_32Fとする
	cv::Mat* src;
	cv::Mat* dest;
	cv::Size ssize;
	cv::Size dsize;
	float threshold;

	void pre_psnr() override
	{
		cv::Mat* srctmp = src;
		cv::Mat* desttmp = dest;

		int border = 32;
		src = new cv::Mat(cv::Size(src8U.cols + 2 * border, src8U.rows + 2 * border), CV_MAKETYPE(CV_32F, src8U.channels()));
		dest = new cv::Mat(cv::Size(dest8U.cols + 4 * border, dest8U.rows + 4 * border), CV_MAKETYPE(CV_32F, dest8U.channels()));
		ssize = src->size();
		dsize = dest->size();

		cv::Mat tmp(src->size(), CV_MAKETYPE(CV_8U, src->channels()));
		cv::copyMakeBorder(src8U, tmp, border, border, border, border, cv::BorderTypes::BORDER_REPLICATE);

		tmp.convertTo(*src, src->type(), 1. / 255.);

		(*this)();

		cv::Rect rect(border * 2, border * 2, dest8U.cols, dest8U.rows);
		cv::Mat((*dest)(rect)).convertTo(dest8U, dest8U.type(), 255.);

		delete src;
		delete dest;

		src = srctmp;
		dest = desttmp;
		ssize = src->size();
		dsize = dest->size();

		//dest->convertTo(dest8U, dest8U.type(), 255.);
	};

	void pre_getDest() override
	{
		dest->convertTo(dest8U, dest8U.type(), 255.);
	};

public:
	DCCITestCVBase(const cv::Mat& original) : DCCITestBase(original)
	{
		src = new cv::Mat(src8U.size(), CV_MAKETYPE(CV_32F, src8U.channels()));
		dest = new cv::Mat(dest8U.size(), CV_MAKETYPE(CV_32F, dest8U.channels()));
		ssize = src->size();
		dsize = dest->size();
		threshold = 1.15f;

		src8U.convertTo(*src, src->type(), 1. / 255.);
	};
	~DCCITestCVBase()
	{
		delete src;
		delete dest;
	};

	double ssim() override
	{
		cv::Mat* srctmp = src;
		cv::Mat* desttmp = dest;

		int border = 32;
		src = new cv::Mat(cv::Size(src8U.cols + 2 * border, src8U.rows + 2 * border), CV_MAKETYPE(CV_32F, src8U.channels()));
		dest = new cv::Mat(cv::Size(dest8U.cols + 4 * border, dest8U.rows + 4 * border), CV_MAKETYPE(CV_32F, dest8U.channels()));
		ssize = src->size();
		dsize = dest->size();

		cv::Mat tmp(src->size(), CV_MAKETYPE(CV_8U, src->channels()));
		cv::copyMakeBorder(src8U, tmp, border, border, border, border, cv::BorderTypes::BORDER_REPLICATE);

		tmp.convertTo(*src, src->type(), 1. / 255.);

		(*this)();

		cv::Rect rect(border * 2, border * 2, dest8U.cols, dest8U.rows);
		cv::Mat((*dest)(rect)).convertTo(dest8U, dest8U.type(), 255.);

		delete src;
		delete dest;

		src = srctmp;
		dest = desttmp;
		ssize = src->size();
		dsize = dest->size();

		cv::Scalar ssim = cv::quality::QualitySSIM::compute(*original8U, dest8U, cv::noArray());
		// RGBの平均値
		return (ssim[0] + ssim[1] + ssim[2]) / 3;
	}
};

class DCCITestHalideBase : public DCCITestBase
{
protected:
	Halide::Buffer<uint8_t> src_buf;
	Halide::Buffer<uint8_t> dest_buf;

	void pre_psnr() override
	{
		MyUtils::convertHalide2Mat<uint8_t>(dest_buf, dest8U);
	};

	void pre_getDest() override
	{
		MyUtils::convertHalide2Mat<uint8_t>(dest_buf, dest8U);
	};

public:
	DCCITestHalideBase(const cv::Mat& original)
		: DCCITestBase(original), src_buf(src8U.cols, src8U.rows, src8U.channels()), dest_buf(dest8U.cols, dest8U.rows, dest8U.channels())
	{
		MyUtils::convertMat2Halide<uint8_t>(src8U, src_buf);
	};

	double ssim() override
	{
		MyUtils::convertHalide2Mat<uint8_t>(dest_buf, dest8U);

		cv::Scalar ssim = cv::quality::QualitySSIM::compute(*original8U, dest8U, cv::noArray());
		// RGBの平均値
		return (ssim[0] + ssim[1] + ssim[2]) / 3;
	}
};

class DCCITestHalideSearchBase : public DCCITestHalideBase
{
public:
	DCCITestHalideSearchBase(const cv::Mat& original) : DCCITestHalideBase(original) {};
	virtual void search(int flagA, int flagB, int flagC, int flagD) = 0;
};

// 実装
class DCCI_L2norm_Loop_Fusion_Split_AVX_32FC3_CLASS_Test : public DCCITestCVBase
{
public:
	DCCI_L2norm_Loop_Fusion_Split_AVX_32FC3_CLASS_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override;
	static std::string getName() { return "DCCI_L2norm_Loop_Fusion_Split_AVX"; };
};

class DCCI_L2norm_Loop_Fusion_Reuse_Split_AVX_32FC3_CLASS_Test : public DCCITestCVBase
{
public:
	DCCI_L2norm_Loop_Fusion_Reuse_Split_AVX_32FC3_CLASS_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override;
	static std::string getName() { return "DCCI_L2norm_Loop_Fusion_Reuse_Split_AVX"; };
};

class DCCI_for_Confirmation_Test : public DCCITestCVBase
{
public:
	DCCI_for_Confirmation_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override
	{
		DCCI_for_Confirmation(*src, *dest, 1, 1.15f, 5);
	}
	static std::string getName() { return "DCCI_for_Confirmation"; };
};

class DCCI_Scalar_Test : public DCCITestCVBase
{
public:
	DCCI_Scalar_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override
	{
		DCCI_Scalar_32FC3(*src, *dest, 1.15f);
	}
	static std::string getName() { return "DCCI_Scalar"; };
};

class DCCI_Conventional_Test : public DCCITestCVBase
{
public:
	DCCI_Conventional_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override;
	static std::string getName() { return "DCCI_Conventional"; };
};

class BiCubic_Test : public DCCITestCVBase
{
public:
	BiCubic_Test(const cv::Mat& original) : DCCITestCVBase(original) {};
	void operator()() override
	{
		cv::resize(*src, *dest, dsize, 0.0, 0.0, cv::InterpolationFlags::INTER_CUBIC);
	};
	static std::string getName() { return "BiCubic"; };
};

class HalideDCCI_Test : public DCCITestHalideBase
{
public:
	HalideDCCI_Test(const cv::Mat& original) : DCCITestHalideBase(original) {};
	void operator()() override
	{
		halide_dcci_auto_scheduled_adams2019(src_buf.raw_buffer(), dest_buf.raw_buffer());
	};
	static std::string getName() { return "HalideDCCI"; };
};

class HalideDCCISearch_Test: public DCCITestHalideSearchBase
{
public:
	HalideDCCISearch_Test(const cv::Mat& original) : DCCITestHalideSearchBase(original) {};
	void operator()() override
	{
		// search ssim
		int flagA = 2340;
		int flagB = 529;
		int flagC = 2772;
		int flagD = 4914;
		halide_dcci_search_auto_scheduled_adams2019(
			src_buf.raw_buffer(),
			flagA,
			flagB,
			flagC,
			flagD,
			dest_buf.raw_buffer()
		);
	}
	void search(int flagA, int flagB, int flagC, int flagD) override
	{
		halide_dcci_search_auto_scheduled_adams2019(
			src_buf.raw_buffer(),
			flagA,
			flagB,
			flagC,
			flagD,
			dest_buf.raw_buffer()
		);
	}
	static std::string getName() { return "HalideDCCISearch"; };
};
