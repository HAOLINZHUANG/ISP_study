/**
 * @author. zhl 2023-4-11
 * @brief RGB_YUV_H 头文件
 * @note 以下这个头文件存放RAM——RGB变幻的一些需要的参数
 */

#ifndef RGB_YUV_H
#define RGB_YUV_H
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
using namespace std;
// 定义RGB图像的像素结构体
struct Pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct YUV_float
{
    double YUV_y;
    double YUV_u;
    double YUV_v;
    int type;
};

//JPEG的亮度量化和色度量化函数 传入
struct  Ycrcb
{
    uint8_t y;
    uint8_t cr;
    uint8_t cb;
};


struct RGB_total
{
    uint64_t R_total;
    uint64_t G_total;
    uint64_t B_total;
    double avg_R;
    double avg_G;
    double avg_B;
    int total;
};

struct Return_GainR_GrainB
{
    double GainR;
    double GainB;
};

struct YUYV
{
    uint8_t Y;
    uint8_t U;
    uint8_t Y_;
    uint8_t V;
};


const vector<vector<int>> LuminanceQuantizationTable = {
    { 16, 11, 10, 16, 24, 40, 51, 61 },
    { 12, 12, 14, 19, 26, 58, 60, 55 },
    { 14, 13, 16, 24, 40, 57, 69, 56 },
    { 14, 17, 22, 29, 51, 87, 80, 62 },
    { 18, 22, 37, 56, 68, 109, 103, 77 },
    { 24, 35, 55, 64, 81, 104, 113, 92 },
    { 49, 64, 78, 87, 103, 121, 120, 101 },
    { 72, 92, 95, 98, 112, 100, 103, 99 }
};
const vector<vector<int>> ChrominanceQuantizationTable = {
    { 17, 18, 24, 47, 99, 99, 99, 99 },
    { 18, 21, 26, 66, 99, 99, 99, 99 },
    { 24, 26, 56, 99, 99, 99, 99, 99 },
    { 47, 66, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99, 99, 99, 99, 99 },
    { 99, 99, 99, 99, 99, 99, 99, 99 }
};
//DTC 量化啊表
// vector<vector<int>> q = {
//     {16,  11,  10,  16,  24,  40,  51,  61},
//     {12,  12,  14,  19,  26,  58,  60,  55},
//     {14,  13,  16,  24,  40,  57,  69,  56},
//     {14,  17,  22,  29,  51,  87,  80,  62},
//     {18,  22,  37,  56,  68, 109, 103,  77},
//     {24,  35,  55,  64,  81, 104, 113,  92},
//     {49,  64,  78,  87, 103, 121, 120, 101},
//     {72,  92,  95,  98, 112, 100, 103,  99}
// };


/*
1.546875        -0.397460938        -0.149414063
-0.247070313        1.258789063        -0.01171875
-0.102539063        -0.844726563        1.947265625
*/
//颜色矩阵



// 白平衡色度权重
#define Y128_Deviation_20_Weight 1
#define Y128_Deviation_40_Weight 0.5
#define Y128_Deviation_64_Weight 0.2
// RGBtoYUV
#define Y(r, g, b) 0.299 * (r)+0.587 * (g)+0.114 * (b)
#define U(r, g, b) -0.147 * (r)-0.298 * (g)+0.436 * (b)
#define V(r, g, b) 0.615 * (r)-0.515 * (g)-0.100 * (b)
// YUVtoRGB
#define R(y, v) (y) + 1.140 * (v)
#define G(y, u, v) (y) - 0.395 * (u)-0.581 * (v)
#define B(y, u) (y) + 2.032 * (u)
// 白平衡灰度判断标准
#define R_Y 0.60
#define B_Y 0.60
// UV标准判断
#define U_V 0.15

// 大小端转换，写成内联函数效率高
inline uint16_t swap_endian(uint16_t num)
{
    return (num >> 8) | (num << 8);
}

#endif