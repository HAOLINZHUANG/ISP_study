/**
 * @author. zhl 2023-4-11
 * @brief PRE_CORRECTION 头文件
 * @note 以下这个头文件是用来做一些相关RAW格式前期要做的一些矫正和处理的手段函数的头文件可以单独处理
 * 同时也支持利用多线程一起高速的处理作为一种快速的手段。
 */
#ifndef PRE_CORRECTION_H
#define PRE_CORRECTION_H
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include "Rgb_yuv.h"
#include <bitset>
#include <fstream>
#include <zlib.h>
// 如果确定是C++ 或者是std命名空间就打开这个
#if 1
using namespace std;
#endif

#define BLOCK_SIZE 8

// 以下是单独步骤处理的函数。
vector<vector<uint16_t>> read_raw(const char *filename, int rows, int cols);
uint16_t correct_bad_Pixel(vector<vector<uint16_t>> &image, int bad_pixel_x, int bad_pixel_y);
uint16_t seek_bad_Pixel(vector<vector<uint16_t>> &image);
int Black_Level_Correction(vector<vector<uint16_t>> &image);
void Raw_to_Bmp_Pixel16_Enablelow10(vector<vector<Pixel>> &image, const char *filename);
void Shadow_Correction(vector<vector<uint16_t>> &image, double compensation);
vector<vector<Pixel>> color_interpolation(vector<vector<uint16_t>> &image);
vector<vector<Pixel>> color_interpolation_grey(vector<vector<uint16_t>> &image);
vector<vector<YUYV>> convertRGB888toYUYV(vector<vector<Pixel>> &src) ;
vector<vector<Pixel>> color_interpolation_RAW(vector<vector<uint16_t>> &image);
Return_GainR_GrainB calculate_white_balance_nums(vector<vector<Pixel>> &image);
vector<vector<Pixel>> Gamma_correction(vector<vector<Pixel>> &image, float gamma);
void matmul3x3_3x1(vector<vector<Pixel>> &mat3x1);
void Turn_around(vector<vector<uint16_t>> &image);
void Black_Level_Correction_respective(vector<vector<uint16_t>> &image);
void white_balance(Return_GainR_GrainB RB,vector<vector<uint16_t>> &image);
void reduce_red(vector<vector<Pixel>> &RGB);
void cruculation_R_G_B(vector<vector<Pixel>> &RGB);
//JPeG test
vector<vector<Ycrcb>> rgb2ycbcr(vector<vector<Pixel>>& mat3x1);
void write_rgb_file(vector<vector<Pixel>> &mat3x1,const char *filename);
void read_rgb_file(vector<vector<Pixel>> &mat3x1,const char * filename);
vector<vector<int>> Quantization(const vector<vector<Ycrcb>>& YCbCrData, const vector<vector<int>>& luminanceQuantizationTable, const vector<vector<int>>& chrominanceQuantizationTable);
vector<vector<double>> DCT(const vector<vector<int>>& quantizedData);
void quantize(vector<vector<double>> &dct_coefficients,vector<vector<int>> &quant_table, int N);
void DTC(vector<vector<int>>& image, vector<vector<double>>& dct_coeffs);
string fading_entropy_encode(const vector<vector<double>>& quant_coeffs);
string compress(const string& str);
bool write_jpg_file(const string& filename, const unsigned char* data, int width, int height);
#endif
