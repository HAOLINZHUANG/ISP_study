#include "Thread_information.h"
#include "Pre_correction.h"
#include "Rgb_yuv.h"

/**
 * @author. zhl 2023-4-11
 * @brief 采用多进程读取文件。
 * @param[in] filename 要读取的文件名
 * @param[in] image_width 图像像素宽度
 * @param[in] image_height 图像像素高度
 * @return vector<vector<uint16_t>>类型的数组
 * @note 采用多线程读取文件，更高速。
 */
void *read_image(void *arg)
{
    ThreadArg *thread_arg = (ThreadArg *)arg; // 将传递给该线程的参数强制转换为 ThreadArg 指针
    int start_row = thread_arg->start_row;    // 获取该线程需要处理的起始行号和结束行号
    int end_row = thread_arg->end_row;
    int image_width = thread_arg->image_width; // 获取图像的宽度、文件路径以及二维矢量的指针
    const char *filename = thread_arg->filename;
    vector<vector<uint16_t>> *image_data = thread_arg->image_data;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        cerr << "Failed to open file " << filename << endl;
        pthread_exit(NULL);
    }
    fseek(fp, start_row * image_width * PIXEL_SIZE, SEEK_SET); // 将文件指针定位到该线程需要处理的起始位置
    for (int i = start_row; i < end_row; i++)                  // 循环读取该线程需要处理的所有行
    {
        vector<uint16_t> row_data(image_width); // 创建一个临时的一维矢量，用于存储当前行的数据
        fread(row_data.data(), PIXEL_SIZE, image_width, fp);
        (*image_data)[i] = row_data;
    }
    fclose(fp);
    pthread_exit(NULL);
}

vector<vector<uint16_t>> thread_read_raw(const char *filename, int image_width, int image_height)
{
    vector<vector<uint16_t>> *image_data = new vector<vector<uint16_t>>(image_height, vector<uint16_t>(image_width));
    pthread_t threads[THREAD_NUM]; // 创建多个线程，每个线程读取图像的一部分
    ThreadArg *thread_args = new ThreadArg[THREAD_NUM];
    int rows_per_thread = image_height / THREAD_NUM;
    int i;
    for (i = 0; i < THREAD_NUM - 1; i++) // 计算该线程需要处理的起始行号和结束行号
    {
        thread_args[i].start_row = i * rows_per_thread;
        thread_args[i].end_row = thread_args[i].start_row + rows_per_thread;
        thread_args[i].filename = filename;
        thread_args[i].image_data = image_data;
        thread_args[i].image_width = image_width;
        pthread_create(&threads[i], NULL, read_image, (void *)&thread_args[i]); // 创建线程并启动
    }
    // 处理最后一个线程需要处理的行数可能不足 rows_per_thread 的情况
    thread_args[i].start_row = i * rows_per_thread;
    thread_args[i].end_row = image_height;
    thread_args[i].filename = filename;
    thread_args[i].image_data = image_data;
    thread_args[i].image_width = image_width;
    pthread_create(&threads[i], NULL, read_image, (void *)&thread_args[i]); // 创建线程并启动

    for (i = 0; i < THREAD_NUM; i++) // 等待所有线程执行完毕
    {
        pthread_join(threads[i], NULL);
    }
    vector<vector<uint16_t>> result(*image_data); // 将指针指向的二维矢量复制到一个新的二维矢量中
    delete image_data;
    delete[] thread_args;
    return result;
}

/**
 * @author. zhl 2023-4-11
 * @brief 采用多进程对像素文件多功能处理。
 * @param[in] image 要做处理的像素数组
 * @return NULL
 * @note 采用多线程处理二维像素数组，更高速。这里的处理包括 （坏点矫正、黑电平处理、阴影矫正）
 */
void *process_chunk(void *arg)
{
    ThreadArg *args = (ThreadArg *)arg;
    int start = args->start_row;
    int end = args->end_row;
    vector<std::vector<uint16_t>> *image = args->image_data;
    Return_GainR_GrainB *RB = args->RB;
    uint16_t min_pixel = args->min_p;
    int heigth = (*image)[0].size();
    float R = 0.0;
    int arv_x = (int)(*image).size() / 2;
    int arv_y = heigth / 2;
    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < heigth; j++)
        {
            // 黑电平矫正
            (*image)[i][j] -= min_pixel;
            // 阴影矫正
            R = (float)abs(sqrt(pow(arv_x - i, 2) + pow(arv_y - j, 2)));
            if (R > 2800)
            {
                R = 0;
            }
            (*image)[i][j] = (float)((float)10 + (float)(R / 280.0)) * (*image)[i][j];
            (*image)[i][j] = (*image)[i][j] / 10;
            //白平衡
            if(i%2==0&&j%2==1)//B(偶，奇)
            {
                (*image)[i][j] = 10*((*image)[i][j])*RB->GainB;
                (*image)[i][j] = (*image)[i][j] / 10;
            }
            if(i%2==1&&j%2==0)//R(奇，偶)
            {
                (*image)[i][j] = 10*((*image)[i][j])*RB->GainR;
                (*image)[i][j] = (*image)[i][j] / 10;
            }
            // 防止曝光过度
            if ((*image)[i][j] > 0x3ff)
            {
                (*image)[i][j] = 0x3ff; 
            }
        }
    }
    delete args;
    pthread_exit(NULL);
}

void thread_pre_correction(vector<vector<uint16_t>> &image,Return_GainR_GrainB RB)
{
    int width = image.size(), heigth = image[0].size(), i, j;
    uint16_t min_pixel;
    float R = 0.0;
    int arv_x = (int)width / 2;
    int arv_y = (int)heigth / 2;
    min_pixel = seek_bad_Pixel(image);              // 坏点矫正并且返回最小值
    pthread_t *threads = new pthread_t[THREAD_NUM]; // 创建线程数组
    int chunk_size = width / THREAD_NUM;            // 计算每个线程的任务量
    for (int t = 0; t < THREAD_NUM; t++)
    {
        int start = t * chunk_size;
        int end = (t == THREAD_NUM - 1) ? width : (t + 1) * chunk_size;
        ThreadArg *args = new ThreadArg;
        args->start_row = start;
        args->end_row = end;
        args->image_data = &image;
        args->min_p = min_pixel;
        args->RB = &RB;
        pthread_create(&threads[t], nullptr, &process_chunk, args);
    }
    for (int t = 0; t < THREAD_NUM; t++)
    {
        pthread_join(threads[t], nullptr); // 等待所有线程处理完毕
    }
    delete[] threads;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define IS_GREEN(i, j) (((i) % 2 == 0 && (j) % 2 == 0) || ((i) % 2 == 1 && (j) % 2 == 1))
#define IS_BLUE(i, j) ((i) % 2 == 1 && (j) % 2 == 0)
#define IS_RED(i, j) ((i) % 2 == 0 && (j) % 2 == 1)
#define IN_RANGE_AND_NOT_CENTER(x, y, i, j, width, height) ((x) >= 0 && (x) < (width) && (y) >= 0 && (y) < (height) && !((x) == (i) && (y) == (j)))



void *rgd_inputt(void *arg)
{
    ThreadArg *args = (ThreadArg *)arg;
    int start_row = args->start_row;
    int end_row = args->end_row;
    vector<vector<Pixel>> *rgbImage = args->rgb_data;
    vector<vector<uint16_t>> *image = args->image_data;
    int height = args->image_height;
    int width = args->image_width;
    int Gcount = 0, Rcount = 0, Bcount = 0;
    uint16_t temporary_R = 0, temporary_G = 0, temporary_B = 0; // 防止溢出16位来让八位累加！！
    for (int i = 0; i < width; i++)
    {
        for (int j = start_row; j < end_row; j++)
        {
            Gcount = 0, Rcount = 0, Bcount = 0;
            temporary_R = 0, temporary_G = 0, temporary_B = 0; // 防止溢出16位来让八位累加！！
            if (IS_GREEN(i, j))
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        // 判断边界内有效数据
                        if (IN_RANGE_AND_NOT_CENTER(x, y, i, j, width, height))
                        {
                            if ((i % 2 == 0)) // 偶数行红蓝读取
                            {
                                // 红色
                                if ((x == i && y == j - 1) || (x == i && y == j + 1))
                                {
                                    temporary_R += image->at(x).at(y);
                                    Rcount++;
                                }
                                // 蓝色
                                if ((x == i - 1 && y == j) || (x == i + 1 && y == j))
                                {
                                    temporary_B += image->at(x).at(y);
                                    Bcount++;
                                }
                            }
                            if ((i % 2 == 1))
                            {
                                // 红色
                                if ((x == i - 1 && y == j) || (x == i + 1 && y == j))
                                {
                                    temporary_R += image->at(x).at(y);
                                    Rcount++;
                                }
                                // 蓝色
                                if ((x == i && y == j - 1) || (x == i && y == j + 1))
                                {
                                    temporary_B += image->at(x).at(y);
                                    Bcount++;
                                }
                            }
                        }
                    }
                }
                rgbImage->at(i).at(j).g = (image->at(i).at(j) / 1) >> 2;
                rgbImage->at(i).at(j).r = (temporary_R / Rcount) >> 2;
                rgbImage->at(i).at(j).b = (temporary_B / Bcount) >> 2;
                continue;
            }
            if (IS_BLUE(i, j))
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        if (IN_RANGE_AND_NOT_CENTER(x, y, i, j, width, height))
                        {
                            // 绿色插值上下左右
                            if ((x == i && y == j - 1) || (x == i && y == j + 1) || (x == i - 1 && y == j) || (x == i + 1 && y == j))
                            {
                                temporary_G += image->at(x).at(y);
                                Gcount++;
                            }
                            // 对蓝色四个角插值
                            if ((x == i - 1 && y == j - 1) || (x == i + 1 && y == j - 1) || (x == i - 1 && y == j + 1) || (x == i + 1 && y == j + 1))
                            {
                                temporary_B += image->at(x).at(y);
                                Bcount++;
                            }
                        }
                    }
                }
                rgbImage->at(i).at(j).g = (temporary_G / (uint16_t)Gcount) >> 2;
                rgbImage->at(i).at(j).b = (image->at(i).at(j) / (uint16_t)1) >> 2;
                rgbImage->at(i).at(j).r = (temporary_B / (uint16_t)Bcount) >> 2;
                continue;
            }
            if (IS_RED(i, j))
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        if (IN_RANGE_AND_NOT_CENTER(x, y, i, j, width, height))
                        {
                            // 绿色插值上下左右
                            if ((x == i && y == j - 1) || (x == i && y == j + 1) || (x == i - 1 && y == j) || (x == i + 1 && y == j))
                            {
                                temporary_G += image->at(x).at(y);
                                Gcount++;
                            }
                            // 对四角红色插值
                            if ((x == i - 1 && y == j - 1) || (x == i + 1 && y == j - 1) || (x == i - 1 && y == j + 1) || (x == i + 1 && y == j + 1))
                            {
                                temporary_R += image->at(x).at(y);
                                Rcount++;
                            }
                        }
                    }
                }
                rgbImage->at(i).at(j).g = (temporary_G / (uint16_t)Gcount) >> 2;
                rgbImage->at(i).at(j).b = (temporary_R / (uint16_t)Rcount) >> 2;
                rgbImage->at(i).at(j).r = (image->at(i).at(j) / (uint16_t)1) >> 2;
                continue;
            }
        }
    }
    pthread_exit(NULL);
}


vector<vector<Pixel>> thread_color_interpolation(vector<vector<uint16_t>> &image)
{
    int height = image[0].size(), width = image.size();
    vector<vector<Pixel>> rgbImage(width, vector<Pixel>(height));
    pthread_t *threads = new pthread_t[THREAD_NUM]; // 创建线程数组
    int chunk_size = width / THREAD_NUM;            // 计算每个线程的任务量
    for (int t = 0; t < THREAD_NUM; t++)
    {
        int start = t * chunk_size;
        int end = (t == THREAD_NUM - 1) ? width : (t + 1) * chunk_size;
        ThreadArg *args = new ThreadArg;
        args->start_row = start;
        args->end_row = end;
        args->image_data = &image;
        args->rgb_data = &rgbImage;
        args->image_width = width;
        args->image_height = height;
        pthread_create(&threads[t], nullptr, &rgd_inputt, args);
    }
    for (int t = 0; t < THREAD_NUM; t++)
    {
        pthread_join(threads[t], nullptr); // 等待所有线程处理完毕
    }
    return rgbImage;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////





