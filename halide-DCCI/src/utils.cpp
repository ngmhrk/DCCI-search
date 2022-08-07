#pragma once
#include <opencp.hpp>
#include "utils.h"

using namespace Halide;

Buffer<uint8_t> MyUtils::imread(cv::String name)
{
    cv::Mat a = cv::imread(name);
    if (a.empty()) std::cout << name << " is empty" << std::endl;

    Halide::Runtime::Buffer<uint8_t> ret(a.cols, a.rows, a.channels());
    convertMat2Halide(a, ret);

    return ret;
}

void MyUtils::imshow(cv::String name, const Halide::Runtime::Buffer<uint8_t>& src)
{
    cv::Mat a(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_8U, src.channels()));
    convertHalide2Mat(src, a);
    cv::imshow(name, a);
}

void MyUtils::imshow(cv::String name, const Halide::Buffer<uint8_t>& src)
{
    cv::Mat a(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_8U, src.channels()));
    convertHalide2Mat(src, a);
    cv::imshow(name, a);
}

template <class T>
void MyUtils::convertHalide2Mat(const Halide::Runtime::Buffer<T>& src, cv::Mat& dest)
{
    assert(typeid(T) == typeid(float) || typeid(T) == typeid(uint8_t));

    if (dest.empty()) {
        if (typeid(T) == typeid(uint8_t))
        {
            dest.create(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_8U, src.channels()));
        }
        else if (typeid(T) == typeid(float))
        {
            dest.create(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_32F, src.channels()));
        }
    }

    const int ch = dest.channels();
    if (ch == 1)
    {
        for (int j = 0; j < dest.rows; j++)
        {
            for (int i = 0; i < dest.cols; i++)
            {
                dest.at<T>(j, i) = src(i, j);
            }
        }
    }
    else if (ch == 3)
    {
        for (int j = 0; j < dest.rows; j++)
        {
            for (int i = 0; i < dest.cols; i++)
            {
                dest.at<T>(j, 3 * i + 0) = src(i, j, 0);
                dest.at<T>(j, 3 * i + 1) = src(i, j, 1);
                dest.at<T>(j, 3 * i + 2) = src(i, j, 2);
            }
        }
    }
}
template <class T>
void MyUtils::convertHalide2Mat(const Halide::Buffer<T>& src, cv::Mat& dest)
{
    assert(typeid(T) == typeid(float) || typeid(T) == typeid(uint8_t));

    if (dest.empty()) {
        if (typeid(T) == typeid(uint8_t))
        {
            dest.create(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_8U, src.channels()));
        }
        else if (typeid(T) == typeid(float))
        {
            dest.create(cv::Size(src.width(), src.height()), CV_MAKETYPE(CV_32F, src.channels()));
        }
    }

    const int ch = dest.channels();
    if (ch == 1)
    {
        for (int j = 0; j < dest.rows; j++)
        {
            for (int i = 0; i < dest.cols; i++)
            {
                dest.at<T>(j, i) = src(i, j);
            }
        }
    }
    else if (ch == 3)
    {
        for (int j = 0; j < dest.rows; j++)
        {
            for (int i = 0; i < dest.cols; i++)
            {
                dest.at<T>(j, 3 * i + 0) = src(i, j, 0);
                dest.at<T>(j, 3 * i + 1) = src(i, j, 1);
                dest.at<T>(j, 3 * i + 2) = src(i, j, 2);
            }
        }
    }
}

template <class T>
void MyUtils::convertMat2Halide(cv::Mat& src, Halide::Runtime::Buffer<T>& dest)
{
    const int ch = src.channels();
    if (ch == 1)
    {
        for (int j = 0; j < src.rows; j++)
        {
            for (int i = 0; i < src.cols; i++)
            {
                dest(i, j) = src.at<T>(j, i);
            }
        }
    }
    else if (ch == 3)
    {
        for (int j = 0; j < src.rows; j++)
        {
            for (int i = 0; i < src.cols; i++)
            {
                dest(i, j, 0) = src.at<T>(j, 3 * i);
                dest(i, j, 1) = src.at<T>(j, 3 * i + 1);
                dest(i, j, 2) = src.at<T>(j, 3 * i + 2);
            }
        }
    }
}
template <class T>
void MyUtils::convertMat2Halide(cv::Mat& src, Halide::Buffer<T>& dest)
{
    const int ch = src.channels();
    if (ch == 1)
    {
        for (int j = 0; j < src.rows; j++)
        {
            for (int i = 0; i < src.cols; i++)
            {
                dest(i, j) = src.at<T>(j, i);
            }
        }
    }
    else if (ch == 3)
    {
        for (int j = 0; j < src.rows; j++)
        {
            for (int i = 0; i < src.cols; i++)
            {
                dest(i, j, 0) = src.at<T>(j, 3 * i);
                dest(i, j, 1) = src.at<T>(j, 3 * i + 1);
                dest(i, j, 2) = src.at<T>(j, 3 * i + 2);
            }
        }
    }
}
