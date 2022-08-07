#pragma once
#include <opencv2/opencv.hpp>
#pragma comment(lib, "opencv_core450.lib")
#pragma comment(lib, "opencv_imgcodecs450.lib")
#pragma comment(lib, "opencv_highgui450.lib")
#pragma comment(lib, "opencv_imgproc450.lib")

#include <opencv2/quality.hpp>
#pragma comment(lib, "opencv_quality450.lib")

#include <opencp.hpp>
#pragma comment(lib, "OpenCP.lib")

#include <Halide.h>
#pragma comment(lib, "Halide.lib")

#include <vector>
#include <string>
#include <functional>

#define XSTR(X) #X
#define STR(X) XSTR(X)
#define DCCI_OUTPUT_DIR ../../generator/dest/host

#include STR(DCCI_OUTPUT_DIR/halide_runtime.h)
#pragma comment(lib, "halide_runtime.lib")

#include STR(DCCI_OUTPUT_DIR/halide_runtime.h)
#pragma comment(lib, "halide_runtime.lib")

#include STR(DCCI_OUTPUT_DIR/halide_dcci_search_auto_scheduled_adams2019.h)
#pragma comment(lib, "halide_dcci_search_auto_scheduled_adams2019.lib")

#include STR(DCCI_OUTPUT_DIR/halide_dcci_auto_scheduled_adams2019.h)
#pragma comment(lib, "halide_dcci_auto_scheduled_adams2019.lib")

#include "utils.h"
#include "DCCIBase.hpp"

#undef XSTR
#undef STR
#undef DCCI_OUTPUT_DIR
