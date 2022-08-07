#include "common.h"
#include "experiments.h"

using namespace cv;
using namespace Halide;
using namespace std;

struct SearchResult
{
	int flagA;
	int flagB;
	int flagC;
	int flagD;
	std::vector<float> values;
	float average;
};

template<class T>
std::vector<float> testKodak24PSNR(std::string kodakImageDirPath)
{
	const string kodakFilePrefix = "kodim";
	vector<float> values;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;
		string filename = kodakImageDirPath + kodakFilePrefix + sout.str() + ".png";

		cv::Mat src = cv::imread(filename, 1);

		T upSampleMethod(src);
		upSampleMethod();
		float psnr = upSampleMethod.psnr();

		values.push_back(psnr);
	}
	return values;
}

template<class T>
std::vector<float> testKodak24SSIM(std::string kodakImageDirPath)
{
	const string kodakFilePrefix = "kodim";
	vector<float> ssims;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;
		string filename = kodakImageDirPath + kodakFilePrefix + sout.str() + ".png";

		cv::Mat src = cv::imread(filename, 1);

		T upSampleMethod(src);
		upSampleMethod();
		float ssim = upSampleMethod.ssim();

		ssims.push_back(ssim);
	}
	return ssims;
}

template<class T>
void testKodak24WriteImage(std::string kodakImageDirPath, std::string outputDirPath)
{
	const string kodakFilePrefix = "kodim";
	vector<float> values;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;
		string filename = kodakImageDirPath + kodakFilePrefix + sout.str() + ".png";

		cv::Mat src = cv::imread(filename, 1);

		T upSampleMethod(src);
		upSampleMethod();

		cv::imwrite(outputDirPath + kodakFilePrefix + sout.str() + "_" + T::getName() + ".png", upSampleMethod.getDest());
	}
}

void outputCSVSearchResult(std::string outputFileName, vector<SearchResult> results)
{
	std::ofstream file(outputFileName, std::ios::out);
	file << "flagA,flagB,flagC,flagD,ssimAverage,";
	for (int i = 0; i < results[0].values.size(); i++)
	{
		file << "ssim" << i << ",";
	}
	file << endl;

	for (const SearchResult& result : results)
	{
		file << result.flagA << ",";
		file << result.flagB << ",";
		file << result.flagC << ",";
		file << result.flagD << ",";
		file << result.average << ",";
		for (int i = 0; i < result.values.size(); i++)
		{
			file << result.values[i] << ",";
		}
		file << endl;
	}
}

template<class T>
void searchOneKodak24PSNR(const std::string SearchImageDirPath, std::vector<SearchResult>& results, SearchResult initial, int flagNum)
{
	vector<cv::Mat> mats;

	// imread kodak24
	const string kodakFilePrefix = "kodim";
	vector<float> values;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;
		string filename = SearchImageDirPath + kodakFilePrefix + sout.str() + ".png";

		mats.push_back(cv::imread(filename, 1));
	}

	// 24–‡‚Å‹ÇŠÅ“K‰»
	vector<T> upsampleTests;
	for (const cv::Mat& mat : mats)
	{
		upsampleTests.push_back(T(mat));
	}

	auto start_time = std::chrono::system_clock::now();
	for (int i = 0; i < pow(2, 14) - 1; i++)
	{
		SearchResult result;

		result.flagA = initial.flagA;
		result.flagB = initial.flagB;
		result.flagC = initial.flagC;
		result.flagD = initial.flagD;

		switch (flagNum)
		{
		case 0: result.flagA = i; break;
		case 1: result.flagB = i; break;
		case 2: result.flagC = i; break;
		case 3: result.flagD = i; break;
		default:
			cerr << "[searchOne] unsupported num" << endl;
			assert(false);
			break;
		}

		float average = 0.f;
		for (T& upsampleTest : upsampleTests)
		{
			upsampleTest.search(result.flagA, result.flagB, result.flagC, result.flagD);
			float psnr = upsampleTest.psnr();
			average += psnr;
			result.values.push_back(psnr);
		}
		average /= upsampleTests.size();
		result.average = average;

		results.push_back(result);

		// log
		if (i % 1000 == 0)
		{
			auto end_time = std::chrono::system_clock::now();
			auto duration = end_time - start_time;
			auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
			cout << i << " loop finished.: " << sec << endl;
			start_time = end_time;
		}
	}
}

template<class T>
void searchLocalOptimalSolutionKodak24PSNR(const std::string SearchImageDirPath, const std::string outputCSVDirPath, SearchResult initial)
{
	vector<SearchResult> results;

	SearchResult best = initial;

	struct ResultGreater
	{
		bool operator() (const SearchResult& a, const SearchResult& b) const noexcept {
			return std::tie(a.average) > std::tie(b.average);
		}
	};

	cout << "search flagD start." << endl;
	searchOneKodak24PSNR<T>(SearchImageDirPath, results, best, 3);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_D.csv", results);
	results.clear();

	cout << "search flagC start." << endl;
	searchOneKodak24PSNR<T>(SearchImageDirPath, results, best, 2);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_C.csv", results);
	results.clear();

	cout << "search flagB start." << endl;
	searchOneKodak24PSNR<T>(SearchImageDirPath, results, best, 1);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_B.csv", results);
	results.clear();

	cout << "search flagA start." << endl;
	searchOneKodak24PSNR<T>(SearchImageDirPath, results, best, 0);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_A.csv", results);
	results.clear();
}

template<class T>
void searchOneKodak24SSIM(const std::string SearchImageDirPath, std::vector<SearchResult>& results, SearchResult initial, int flagNum)
{
	vector<cv::Mat> mats;

	// imread kodak24
	const string kodakFilePrefix = "kodim";
	vector<float> ssims;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;
		string filename = SearchImageDirPath + kodakFilePrefix + sout.str() + ".png";

		mats.push_back(cv::imread(filename, 1));
	}

	// 24–‡‚Å‹ÇŠÅ“K‰»
	vector<T> upsampleTests;
	for (const cv::Mat& mat : mats)
	{
		upsampleTests.push_back(T(mat));
	}

	auto start_time = std::chrono::system_clock::now();
	for (int i = 0; i < pow(2, 14) - 1; i++)
	{
		SearchResult result;

		result.flagA = initial.flagA;
		result.flagB = initial.flagB;
		result.flagC = initial.flagC;
		result.flagD = initial.flagD;

		switch (flagNum)
		{
		case 0: result.flagA = i; break;
		case 1: result.flagB = i; break;
		case 2: result.flagC = i; break;
		case 3: result.flagD = i; break;
		default:
			cerr << "[searchOne] unsupported num" << endl;
			assert(false);
			break;
		}

		float ssimAverage = 0.f;
		for (T& upsampleTest : upsampleTests)
		{
			upsampleTest.search(result.flagA, result.flagB, result.flagC, result.flagD);
			float ssim = upsampleTest.ssim();
			ssimAverage += ssim;
			result.values.push_back(ssim);
		}
		ssimAverage /= upsampleTests.size();
		result.average = ssimAverage;

		results.push_back(result);

		// log
		if (i % 1000 == 0)
		{
			auto end_time = std::chrono::system_clock::now();
			auto duration = end_time - start_time;
			auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
			cout << i << " loop finished.: " << sec << endl;
			start_time = end_time;
		}
	}
}

template<class T>
void searchLocalOptimalSolutionKodak24SSIM(const std::string SearchImageDirPath, const std::string outputCSVDirPath, SearchResult initial)
{
	vector<SearchResult> results;

	SearchResult best = initial;

	struct ResultGreater
	{
		bool operator() (const SearchResult& a, const SearchResult& b) const noexcept {
			return std::tie(a.average) > std::tie(b.average);
		}
	};

	cout << "search flagD start." << endl;
	searchOneKodak24SSIM<T>(SearchImageDirPath, results, best, 3);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_SSIM_D.csv", results);
	results.clear();

	cout << "search flagC start." << endl;
	searchOneKodak24SSIM<T>(SearchImageDirPath, results, best, 2);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_SSIM_C.csv", results);
	results.clear();

	cout << "search flagB start." << endl;
	searchOneKodak24SSIM<T>(SearchImageDirPath, results, best, 1);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_SSIM_B.csv", results);
	results.clear();

	cout << "search flagA start." << endl;
	searchOneKodak24SSIM<T>(SearchImageDirPath, results, best, 0);
	std::sort(results.begin(), results.end(), ResultGreater{});
	best = results[0];
	outputCSVSearchResult(outputCSVDirPath + "kodak24_" + T::getName() + "_SSIM_A.csv", results);
	results.clear();
}

template<class T>
double benchmark(const std::string ImageDirPath, int iterations, int samples)
{
	cv::Mat src = cv::imread(ImageDirPath + "lenna.png", 1);
	T upsampleMethod(src);
	return upsampleMethod.benchmark(iterations, samples);
}

// ----------------------------------experiments-------------------------------------------

void testPSNR()
{
	const std::string kodakImageDirPath = "../dataset/kodak/";
	const std::string outputFilePath = "dest/csv/kodak24_psnr.csv";

	// cpp vector
	vector<float> psnrs_DCCIAVX = testKodak24PSNR<DCCI_L2norm_Loop_Fusion_Split_AVX_32FC3_CLASS_Test>(kodakImageDirPath);

	// halide
	vector<float> psnrs_DCCIOriginal = testKodak24PSNR<HalideDCCI_Test>(kodakImageDirPath);
	vector<float> psnrs_DCCISearchTransposedSinc = testKodak24PSNR<HalideDCCISearch_Test>(kodakImageDirPath);

	// bicubic
	vector<float> psnrs_BiCubic = testKodak24PSNR<BiCubic_Test>(kodakImageDirPath);

	struct PSNRResult
	{
		std::string methodName;
		vector<float>& values;
	};

	vector<PSNRResult> results;
	results.push_back({ "DCCIAVX", psnrs_DCCIAVX });

	results.push_back({ "DCCIOriginal", psnrs_DCCIOriginal });
	results.push_back({ "DCCISearchTransposedSinc", psnrs_DCCISearchTransposedSinc });

	results.push_back({ "BiCubic", psnrs_BiCubic });

	std::ofstream file(outputFilePath, std::ios::out);
	file << "methodName" << ",";
	for (int i = 0; i < results.size(); i++)
	{
		file << results[i].methodName << ",";
	}
	file << endl;

	for (int j = 1; j <= 24; j++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << j;
		file << "kodim" << sout.str() << ",";
		for (int i = 0; i < results.size(); i++)
		{
			file << results[i].values[j - 1] << ",";
		}
		file << endl;
	}
}

void testSSIM()
{
	const std::string kodakImageDirPath = "../dataset/kodak/";
	const std::string outputFilePath = "dest/csv/kodak24_ssim.csv";

	// cpp vector
	vector<float> ssims_DCCIAVX = testKodak24SSIM<DCCI_L2norm_Loop_Fusion_Split_AVX_32FC3_CLASS_Test>(kodakImageDirPath);

	// halide
	vector<float> ssims_DCCIOriginal = testKodak24SSIM<HalideDCCI_Test>(kodakImageDirPath);
	vector<float> ssims_DCCISearchTransposedSinc = testKodak24SSIM<HalideDCCISearch_Test>(kodakImageDirPath);

	// bicubic
	vector<float> ssims_BiCubic = testKodak24SSIM<BiCubic_Test>(kodakImageDirPath);

	struct SSIMResult
	{
		std::string methodName;
		vector<float>& ssims;
	};

	vector<SSIMResult> results;
	results.push_back({ "DCCIAVX", ssims_DCCIAVX });

	results.push_back({ "DCCIOriginal", ssims_DCCIOriginal });
	results.push_back({ "DCCISearchTransposedSinc", ssims_DCCISearchTransposedSinc });

	results.push_back({ "BiCubic", ssims_BiCubic });

	std::ofstream file(outputFilePath, std::ios::out);
	file << "methodName" << ",";
	for (int i = 0; i < results.size(); i++)
	{
		file << results[i].methodName << ",";
	}
	file << endl;

	for (int j = 1; j <= 24; j++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << j;
		file << "kodim" << sout.str() << ",";
		for (int i = 0; i < results.size(); i++)
		{
			file << results[i].ssims[j - 1] << ",";
		}
		file << endl;
	}
}

void createImage()
{
	// write image
	const std::string kodakImageDirPath = "../dataset/kodak/";
	const std::string outputDirPath = "dest/img/";
	testKodak24WriteImage<HalideDCCI_Test>(kodakImageDirPath, outputDirPath);
	testKodak24WriteImage<HalideDCCISearch_Test>(kodakImageDirPath, outputDirPath);
	testKodak24WriteImage<DCCI_L2norm_Loop_Fusion_Reuse_Split_AVX_32FC3_CLASS_Test>(kodakImageDirPath, outputDirPath);
	testKodak24WriteImage<BiCubic_Test>(kodakImageDirPath, outputDirPath);

	vector<float> DCCI_ssim = testKodak24SSIM<HalideDCCI_Test>(kodakImageDirPath);
	vector<float> AEU_ssim = testKodak24SSIM<HalideDCCISearch_Test>(kodakImageDirPath);

	// show image
	const string kodakFilePrefix = "kodim";
	vector<float> values;
	for (int i = 1; i <= 24; i++)
	{
		std::ostringstream sout;
		sout << std::setfill('0') << std::setw(2) << i;

		cv::Mat original;
		cv::Mat DCCI;
		cv::Mat AEU;
		cv::Mat AVX;
		cv::Mat BiCubic;
		{
			string filename = kodakImageDirPath + kodakFilePrefix + sout.str() + ".png";
			original = cv::imread(filename, 1);
			cv::imshow("original", original);
		}
		{
			string filename = outputDirPath + kodakFilePrefix + sout.str() + "_" + HalideDCCI_Test::getName() + ".png";
			DCCI = cv::imread(filename, 1);
			cv::imshow("DCCI", DCCI);
			cv::displayOverlay("DCCI", cv::format("SSIM : %f", DCCI_ssim[i - 1]));
		}
		{
			string filename = outputDirPath + kodakFilePrefix + sout.str() + "_" + HalideDCCISearch_Test::getName() + ".png";
			AEU = cv::imread(filename, 1);
			cv::imshow("AEU", AEU);
			cv::displayOverlay("AEU", cv::format("SSIM : %f", AEU_ssim[i - 1]));
		}
		{
			string filename = outputDirPath + kodakFilePrefix + sout.str() + "_" + DCCI_L2norm_Loop_Fusion_Reuse_Split_AVX_32FC3_CLASS_Test::getName() + ".png";
			AVX = cv::imread(filename, 1);
			cv::imshow("AVX", AVX);
		}
		{
			string filename = outputDirPath + kodakFilePrefix + sout.str() + "_" + BiCubic_Test::getName() + ".png";
			BiCubic = cv::imread(filename, 1);
			cv::imshow("BiCubic", BiCubic);
		}

		cv::Rect zoom(210, 190, 100, 100);
		cv::Size outputSize(100, 100);
		cv::Mat zoomOriginal, zoomDCCI, zoomAEU, zoomBiCubic;
		cv::resize(cv::Mat(original, zoom), zoomOriginal, outputSize, 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST_EXACT);
		cv::resize(cv::Mat(DCCI, zoom), zoomDCCI, outputSize, 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST_EXACT);
		cv::resize(cv::Mat(AEU, zoom), zoomAEU, outputSize, 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST_EXACT);
		cv::resize(cv::Mat(BiCubic, zoom), zoomBiCubic, outputSize, 0.0, 0.0, cv::InterpolationFlags::INTER_NEAREST_EXACT);

		string filenamePrefix = outputDirPath + kodakFilePrefix + sout.str() + "_zoom_";
		cv::imwrite(filenamePrefix + ".png", zoomOriginal);
		cv::imwrite(filenamePrefix + HalideDCCI_Test::getName() + ".png", zoomDCCI);
		cv::imwrite(filenamePrefix + HalideDCCISearch_Test::getName() + ".png", zoomAEU);
		cv::imwrite(filenamePrefix + BiCubic_Test::getName() + ".png", zoomBiCubic);
		cv::waitKey(0);
	}
}

void benchmark()
{
	string imageDirPath = "../dataset/search/";
	double time = benchmark<HalideDCCI_Test>(imageDirPath, 2000, 1);
	cout << time << endl;
}

void searchDCCILocalOptimizePSNR()
{
	const std::string kodakImageDirPath = "../dataset/kodak/";

	SearchResult initial;
	initial.flagA = static_cast<int>(0b00100100100100);
	initial.flagB = static_cast<int>(0b10001000010001);
	initial.flagC = static_cast<int>(0b00000111100000);
	initial.flagD = static_cast<int>(0b10001000010001);
	searchLocalOptimalSolutionKodak24PSNR<HalideDCCISearch_Test>(kodakImageDirPath, "dest/csv/", initial);
}

void searchDCCILocalOptimizeSSIM()
{
	const std::string kodakImageDirPath = "../dataset/kodak/";

	SearchResult initial;
	initial.flagA = static_cast<int>(0b00100100100100);
	initial.flagB = static_cast<int>(0b10001000010001);
	initial.flagC = static_cast<int>(0b00000111100000);
	initial.flagD = static_cast<int>(0b10001000010001);
	searchLocalOptimalSolutionKodak24SSIM<HalideDCCISearch_Test>(kodakImageDirPath, "dest/csv/", initial);
}