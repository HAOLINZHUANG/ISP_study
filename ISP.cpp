#include <iostream>
#include "Thread_information.h"
#include "Pre_correction.h"
#include "jpeg_action.h"
#include "Rgb_yuv.h"
using namespace std;

#define RAW_FILE_NAME "/home/gec/HDD_STORAGE/ISP_1.0/RAW/9B002.raw"
#define BMP_FILE_NAME "/home/gec/HDD_STORAGE/ISP_1.0/BMP/"
#define JPG_FILE_NAME "/home/gec/HDD_STORAGE/ISP_1.0/JPG/"
#define RGB_FILE_NAME "/home/gec/HDD_STORAGE/ISP_1.0/RGB"

char *timer(const char *, const char *);

vector<vector<int>> q = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68, 109, 103, 77},
    {24, 35, 55, 64, 81, 104, 113, 92},
    {49, 64, 78, 87, 103, 121, 120, 101},
    {72, 92, 95, 98, 112, 100, 103, 99}};


vector<std::vector<uint16_t>> shrink_array(const std::vector<std::vector<uint16_t>>& arr) {
    size_t height = arr.size();
    size_t width = arr[0].size();
    std::vector<std::vector<uint16_t>> new_arr(height / 2, std::vector<uint16_t>(width / 2));
    for (size_t i = 0; i < height; i += 2) {
        for (size_t j = 0; j < width; j += 2) {
            uint16_t sum = 0;
            for (size_t k = i; k < i + 2; k++) {
                for (size_t l = j; l < j + 2; l++) {
                    sum += arr[k][l];
                }
            }
            new_arr[i / 2][j / 2] = sum / 4;
        }
    }
    return new_arr;
}

int main()
{
    char *pic;
    vector<vector<uint16_t>> image;
#if 0
    image = thread_read_raw(RAW_FILE_NAME,5600,5600);//读取一张图片
    seek_bad_Pixel(image);//去坏点
    Black_Level_Correction_respective(image);//四通道黑电平
    Shadow_Correction(image, 0.2);//阴影矫正
    vector<vector<Pixel>> RGB = color_interpolation(image);//跳转到RGB域
    Return_GainR_GrainB RB = calculate_white_balance_nums(RGB);//获取白平衡系数
    white_balance(RB,image);//用拿到的系数做白平衡
    RGB = color_interpolation(image);//重新跳转到RGB域
    matmul3x3_3x1(RGB); //色彩空间矩阵校正
    reduce_red(RGB);//去过曝光过度产生的红斑
    //Gamma_correction(RGB,1.1); //伽马矫正
    cruculation_R_G_B(RGB);
    //cout << RB.GainB << "  " << RB.GainR <<endl;
#endif
#if 0
    image = thread_read_raw(RAW_FILE_NAME,5600,5600);
    thread_pre_correction(image);
    vector<vector<Pixel>> RGB = thread_color_interpolation(image);
    Return_GainR_GrainB RB = calculate_white_balance_nums(RGB);
    cout << RB.GainB << "  " << RB.GainR << endl;
#endif
#if 0
    image = thread_read_raw(RAW_FILE_NAME,5600,5600);//读取一张图片
    vector<vector<uint16_t>> minni =shrink_array(image);
    vector<vector<Pixel>> RGB = thread_color_interpolation(minni);//跳转到RGB域
    Return_GainR_GrainB RB = calculate_white_balance_nums(RGB);//获取白平衡系数
    thread_pre_correction(minni,RB); // 去坏点 黑电平 阴影矫正  白平衡
    RGB = thread_color_interpolation(minni);//重新跳转到RGB域
    //matmul3x3_3x1(RGB); //色彩空间矩阵校正
    //Gamma_correction(RGB,1.6); //伽马矫正
#endif
#if 0
    Raw_to_Bmp_Pixel16_Enablelow10(RGB,timer(BMP_FILE_NAME,(const char *)"bmp"));
#endif
#if 0
    write_jpeg_file(timer(JPG_FILE_NAME, (const char *)"jpg"), RGB, 5600, 5600, 50);
#endif
#if 1
    image = thread_read_raw(RAW_FILE_NAME,5600,5600);//读取一张图片
    vector<vector<Pixel>> RGB = thread_color_interpolation(image);//跳转到RGB域
    vector<vector<Ycrcb>> YCrcb =  rgb2ycbcr(RGB);//把RGB域转到Ycbcr域
    vector<vector<int>> quantization = Quantization(YCrcb,LuminanceQuantizationTable,ChrominanceQuantizationTable);//色度亮度量化
    vector<vector<double>> dtc(quantization.size(), vector<double>(quantization[0].size()));
    DTC(quantization,dtc);//离散余弦变换，转到频域除高频
    quantize(dtc,q,8);//量化DCT系数矩阵
    string image_pic = fading_entropy_encode(dtc);//熵编码
    string image_pic0 = compress(image_pic);//zip压缩
    write_jpg_file(timer(JPG_FILE_NAME, (const char *)"jpg"),(unsigned char*)image_pic0.c_str(), 5600, 5600);//写入JPG
#endif
#if 0
    image = thread_read_raw(RAW_FILE_NAME, 5600, 5600);
    Turn_around(image);//镜像图片
    vector<vector<Pixel>> RGB = color_interpolation_grey(image);//灰色
    write_jpeg_file(timer(JPG_FILE_NAME, (const char *)"jpg"), RGB, 5600, 5600, 100);
#endif
    return 0;
}

char *timer(const char *filename, const char *format)
{
    time_t timestamp = time(NULL);
    struct tm *local_time = localtime(&timestamp);
    int year = local_time->tm_year + 1900; // 年份从1900年开始计算，需要加上1900
    int month = local_time->tm_mon + 1;    // 月份从0开始计算，需要加上1
    int day = local_time->tm_mday;         // 日期
    int hour = local_time->tm_hour;
    int min = local_time->tm_min;
    int sec = local_time->tm_sec;
    char *timer_num = new char[100];
    sprintf(timer_num, "%s%04d-%02d-%02d %02d:%02d:%02d.%s", filename, year, month, day, hour, min, sec, format);
    return timer_num;
}


