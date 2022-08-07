#pragma once
#include <Halide.h>
#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

namespace MyUtils
{
    Halide::Buffer<uint8_t> imread(cv::String name);
    void imshow(cv::String name, const Halide::Runtime::Buffer<uint8_t>& src);
    void imshow(cv::String name, const Halide::Buffer<uint8_t>& src);

    template <class T>
    void convertHalide2Mat(const Halide::Runtime::Buffer<T>& src, cv::Mat& dest);
    template <class T>
    void convertHalide2Mat(const Halide::Buffer<T>& src, cv::Mat& dest);

    template <class T>
    void convertMat2Halide(cv::Mat& src, Halide::Runtime::Buffer<T>& dest);
    template <class T>
    void convertMat2Halide(cv::Mat& src, Halide::Buffer<T>& dest);
}


template void MyUtils::convertHalide2Mat<float>(const Halide::Runtime::Buffer<float>& src, cv::Mat& dest);
template void MyUtils::convertHalide2Mat<uint8_t>(const Halide::Runtime::Buffer<uint8_t>& src, cv::Mat& dest);
template void MyUtils::convertHalide2Mat<float>(const Halide::Buffer<float>& src, cv::Mat& dest);
template void MyUtils::convertHalide2Mat<uint8_t>(const Halide::Buffer<uint8_t>& src, cv::Mat& dest);

template void MyUtils::convertMat2Halide<float>(cv::Mat& src, Halide::Runtime::Buffer<float>& dest);
template void MyUtils::convertMat2Halide<uint8_t>(cv::Mat& src, Halide::Runtime::Buffer<uint8_t>& dest);
template void MyUtils::convertMat2Halide<float>(cv::Mat& src, Halide::Buffer<float>& dest);
template void MyUtils::convertMat2Halide<uint8_t>(cv::Mat& src, Halide::Buffer<uint8_t>& dest);
