/**
 * @author. zhl 2023-4-11
 * @brief THREAD_INFORMATION 头文件
 * @note 以下这个头文件是用来保存公共线程所用到的参数传递信息，和一些和线程有关的公共使用参数存放在这里”
 */

#ifndef THREAD_INFORMATION_H
#define THREAD_INFORMATION_H

#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <pthread.h>
#include "Rgb_yuv.h"
#include <thread>
#include <fstream>
// 如果确定是C++ 或者是std命名空间就打开这个
#if 1
using namespace std;
#endif

#define THREAD_NUM 2 // 定义线程数  
#define PIXEL_SIZE 2  // 定义每个像素占用的字节数为2字节
#define BLOCK_SIZE_thread 8 //块数量

// 线程传递消息结构体
struct ThreadArg
{
    int start_row;                        // 线程处理的起始行号
    int end_row;                          // 线程处理的结束行号
    int image_width;                      // 图像的宽度
    int image_height;                     // 图像高度
    const char *filename;                 // 文件名
    vector<vector<uint16_t>> *image_data; // 传入的图像数组
    vector<vector<Pixel>> *rgb_data; // 传入的图像数组
    uint16_t min_p;                       // 最小值参数
    Return_GainR_GrainB *RB;
    // ThreadArg() = default; // 默认构造函数
};



vector<vector<uint16_t>> thread_read_raw(const char *filename, int image_width, int image_height);
void thread_pre_correction(vector<vector<uint16_t>>& image,Return_GainR_GrainB RB);
vector<vector<Pixel>> thread_color_interpolation(vector<vector<uint16_t>> &image);

#endif
