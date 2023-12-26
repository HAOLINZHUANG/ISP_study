#include "Pre_correction.h"
#include "Rgb_yuv.h"
#include <stdlib.h>
#include <string.h>
/**
 * @author. zhl 2023-3-24
 * @brief 从指定文件中读取图像数据，返回一个二维的unsigned char类型的vector给定RAW文件名，传入像素大小
 * @param[in]  filename 文件名
 * @param[in] rows 图像的行数
 * @param[in] cols 图像的列数
 * @return 返回值为把参数按一定格式读取入vector后的vector。
 * @note 无。
 */
vector<vector<uint16_t>> read_raw(const char *filename, int rows, int cols)
{
    // 创建一个指定行数和列数的二维uint16_t类型的vector，并将其初始化为0
    vector<vector<uint16_t>> image(rows, vector<uint16_t>(cols));
    // 创建一个文件指针
    FILE *fp;
    // 创建一个缓冲区，用于从文件中读取数据
    uint16_t buffer[1];
    // 创建两个循环变量，用于遍历图像的所有像素
    int i, j;
    // 打开指定的文件，如果文件不存在或无法打开，则输出错误信息并退出程序
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(1);
    }
    // 遍历图像的所有像素，从文件中读取数据并将其存储到vector中
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            // 从文件中读取一个uint16_t类型的数据到缓冲区中，如果读取失败则输出错误信息并退出程序
            if (fread(buffer, sizeof(uint16_t), 1, fp) != 1)
            {
                printf("Error reading file: row %d, col %d\n", i, j);
                exit(1);
            }
            image[i][j] = buffer[0];
        }
    }
    // 关闭文件
    fclose(fp);
    // 返回存储图像数据的vector
    return image;
}

/**
 * @author. zhl 2023-3-27
 * @brief 坏点矫正
 * @param[in] image 定格式读取入vector后的vector和某一成员
 * @param[in] bad_pixel_x 传入坏点X坐标
 * @param[in] bad_pixel_y 传入坏点Y坐标
 * @return NULL
 * @note 不包括传入坏点。
 */
uint16_t correct_bad_Pixel(vector<vector<uint16_t>> &image, int bad_pixel_x, int bad_pixel_y)
{
    int width = image.size(), height = image[0].size(), count = 0;
    uint16_t sum = 0, direction_avaerage = 0, direction_up = 0, direction_dowm = 0, direction_right = 0, direction_left = 0; // 周围元素的总和
    if (bad_pixel_x < 0 || bad_pixel_x >= width || bad_pixel_y < 0 || bad_pixel_y >= height)
    {
        return -1;
    }
    if (bad_pixel_x - 2 >= 0)
    {
        direction_up = image[bad_pixel_x - 2][bad_pixel_y];
        count++;
    }
    if (bad_pixel_x + 2 < width)
    {
        direction_dowm = image[bad_pixel_x + 2][bad_pixel_y];
        count++;
    }
    if (bad_pixel_y - 2 >= 0)
    {
        direction_right = image[bad_pixel_x][bad_pixel_y - 2];
        count++;
    }
    if (bad_pixel_y + 2 < height)
    {
        direction_left = image[bad_pixel_x][bad_pixel_y + 2];
        count++;
    }
    sum = direction_up + direction_dowm + direction_right + direction_left;
    direction_avaerage = sum / count;
    return direction_avaerage;
}

/**
 * @author. zhl 2023-3-27
 * @brief 检测坏点
 * @param[in] image 定格式读取入vector后的vector和某一成员
 * @param[in] NULL
 * @param[in] NULL
 * @return 无。
 * @note 无。
 */
uint16_t seek_bad_Pixel(vector<vector<uint16_t>> &image)
{
    int width = image.size(), height = image[0].size(), m = 0, n = 0;
    uint16_t min_pixel = image[0][0];
    // 寻找坏点
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // 判断坏点
            if (abs((correct_bad_Pixel(image, i, j) - image[i][j])) > 0x80)
            {
                image[i][j] = correct_bad_Pixel(image, i, j);
            }

            if (image[i][j] < min_pixel)
            {
                min_pixel = image[i][j];
            }
        }
    }
    return min_pixel;
}

/**
 * @author. zhl 2023-3-28
 * @brief 黑电平矫正
 * @param[in] image 定格式读取入vector后的vector和某一成员
 * @param[in] NULL
 * @param[in] NULL
 * @return 返回补偿值。
 * @note 黑电平减多变绿，减少变红， 黑电平调整导致r/b gain 不平衡。
 */
int Black_Level_Correction(vector<vector<uint16_t>> &image)
{
    int width = image.size(), heigth = image[0].size(), i, j;
    uint16_t min_pixel = image[0][0];
    // 第一遍遍历整个数组找到最小的像素值作为黑电平的补偿数
    for (i = 0; i < width; i++)
    {
        for (j = 0; j < heigth; j++)
        {
            if (image[i][j] < min_pixel)
            {
                min_pixel = image[i][j];
            }
        }
    }
    // 第二遍遍历数组用黑电平补偿数对图片每个像素进行补偿
    for (i = 0; i < width; i++)
    {
        for (j = 0; j < heigth; j++)
        {
            image[i][j] -= min_pixel;
        }
    }
    return min_pixel;
}

/**
 * @author. zhl 2023-3-28
 * @brief 16bit1像素低十位有效raw转bmp格式
 * @param[in] image 定格式读取入vector后的vector和某一成员
 * @param[in] filename 生成的文件名
 * @param[in] NULL
 * @return 无。
 * @note headerSize：
    文件头所占的字节数，通常为40字节；
    planes：位面数，通常为1；
    bitsPerPixel：每个像素占用的位数，即色深，例如16位色深的图像每个像素占用2个字节；
    compression：压缩类型，通常为0表示不压缩；
    bmpDataSize：位图数据区的大小，即像素数据的字节数；
    resolutionX：水平分辨率，单位为像素每米；
    resolutionY：垂直分辨率，单位为像素每米；
    colors：调色板使用的颜色数目，0表示使用所有颜色；
    importantColors：重要的颜色数目，对于本次操作，可以设置为0。
 */
void Raw_to_Bmp_Pixel16_Enablelow10(vector<vector<Pixel>> &image, const char *filename)
{
    int width = image.size();
    int height = image[0].size();
    FILE *pFile = fopen(filename, "wb");
    if (!pFile)
    {
        printf("Error: Unable to open file %s\n", filename);
        return;
    }
    // 54个字节的报头
    uint32_t bmpSize = width * height * sizeof(uint16_t) + 54;
    uint32_t offset = 54;
    // 14个字节的格式头和40个字节的信息头
    fwrite("BM", sizeof(char), 2, pFile);
    fwrite(&bmpSize, sizeof(uint32_t), 1, pFile);
    fwrite("\0\0\0\0", sizeof(char), 4, pFile);
    fwrite(&offset, sizeof(uint32_t), 1, pFile);
    uint32_t headerSize = 40;
    uint32_t planes = 1;
    uint32_t bitsPerPixel = 24;
    uint32_t compression = 0;
    uint32_t bmpDataSize = width * height * sizeof(uint16_t);
    uint32_t resolutionX = 0;
    uint32_t resolutionY = 0;
    uint32_t colors = 0;
    uint32_t importantColors = 0;
    fwrite(&headerSize, sizeof(uint32_t), 1, pFile);
    fwrite(&width, sizeof(uint32_t), 1, pFile);
    fwrite(&height, sizeof(uint32_t), 1, pFile);
    fwrite(&planes, sizeof(uint16_t), 1, pFile);
    fwrite(&bitsPerPixel, sizeof(uint16_t), 1, pFile);
    fwrite(&compression, sizeof(uint32_t), 1, pFile);
    fwrite(&bmpDataSize, sizeof(uint32_t), 1, pFile);
    fwrite(&resolutionX, sizeof(uint32_t), 1, pFile);
    fwrite(&resolutionY, sizeof(uint32_t), 1, pFile);
    fwrite(&colors, sizeof(uint32_t), 1, pFile);
    fwrite(&importantColors, sizeof(uint32_t), 1, pFile);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            Pixel pixel = image[height - 1 - i][j];
            /*
                 1 2   的读取方式 3 4 1 2
                 3 4
            */
            // uint16_t lowTen = pixel.r & 0x3FF;   // 0000001111111111
            fwrite(&pixel.r, sizeof(uint8_t), 1, pFile);
            fwrite(&pixel.g, sizeof(uint8_t), 1, pFile);
            fwrite(&pixel.b, sizeof(uint8_t), 1, pFile);
        }
    }
    fclose(pFile);
}

/**
 * @author. zhl 2023-4-7
 * @brief 阴影矫正
 * @param[in] image 定格式读取入vector后的vector和某一成员
 * @param[in] compensation 补偿系数
 * @param[in] NULL
 * @return 无。
 * @note 根据各像素与图片中心的距离来乘以一个补偿系数
 * 网上查的：对于整个镜头，可将其视为一个凸透镜。
 * 由于凸透镜中心的聚光能力远大于其边缘，从而导致Sensor中心的光线强度大于四周。
 * 此种现象也称之为边缘光照度衰减。表现为图像中心亮、四周暗。
 * 图像从中心向四周衰减的速率基本符合COSθ法则。具体公式如下所示，Io表示中心光强，θ表示入射光线与水平轴的夹角。I = Io* COSθ
 */
void Shadow_Correction(vector<vector<uint16_t>> &image, double compensation)
{
    // 获取中心亮度，这里用中心的几个100像素平均亮度为例
    // 如果增益后大心亮度值则降到中间亮度值域
    int width = image.size(), heigth = image[0].size(), arv_x = 0, arv_y = 0, i, j, count = 0, count_1 = 0;
    float R = 0.0;
    arv_x = (int)width / 2;
    arv_y = (int)heigth / 2;
    // 计算中心每个像素到中心距离并且矫正
    for (i = 0; i < width; i++)
    {
        for (j = 0; j < heigth; j++)
        {
            // 计算距离
            R = (float)abs(sqrt(pow(arv_x - i, 2) + pow(arv_y - j, 2)));
            if (R > 2800)
            {
                R = 0;
            }
            image[i][j] = (float)((float)10 + (float)(R / 280.0) * compensation) * image[i][j];
            image[i][j] = image[i][j] / 10;

            if (image[i][j] > 0x3ff)
            {
                image[i][j] = 0x3ff; // 防止曝光过度
            }
        }
    }
}

/**
 * @author. zhl 2023-4-5
 * @brief RGB颜色插值彩色
 * @param[in] image image raw二维数组
 * @param[in] NULL
 * @param[in] NULL
 * @return NULL。
 *G B G B G B G B
 *R G R G R G R G
 *G B G B G B G B
 *R G R G R G R G
 *G B G B G B G B
 *R G R G R G R G
 *G B G B G B G B
 * @note 无。
 */
vector<vector<Pixel>> color_interpolation(vector<vector<uint16_t>> &image)
{
    int height = image[0].size(), width = image.size();
    vector<vector<Pixel>> rgbImage(width, vector<Pixel>(height));
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int Gcount = 0, Rcount = 0, Bcount = 0;
            bool Green = ((i % 2 == 0) && (j % 2 == 0)) || ((i % 2 == 1) && (j % 2 == 1));
            bool Red = (i % 2 == 1) && (j % 2 == 0);
            bool Blue = (i % 2 == 0) && (j % 2 == 1);
            uint16_t temporary_R = 0, temporary_G = 0, temporary_B = 0; // 防止溢出16位来让八位累加！！
            if (Green)
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        // 判断边界内有效数据
                        if (x >= 0 && x < width && y >= 0 && y < height && !(x == i && y == j))
                        {
                            if ((i % 2 == 0)) // 偶数行红蓝读取
                            {
                                // 红色
                                if ((x == i && y == j - 1) || (x == i && y == j + 1))
                                {
                                    temporary_R += image[x][y];
                                    Rcount++;
                                }
                                // 蓝色
                                if ((x == i - 1 && y == j) || (x == i + 1 && y == j))
                                {
                                    temporary_B += image[x][y];
                                    Bcount++;
                                }
                            }
                            if ((i % 2 == 1))
                            {
                                // 红色
                                if ((x == i - 1 && y == j) || (x == i + 1 && y == j))
                                {
                                    temporary_R += image[x][y];
                                    Rcount++;
                                }
                                // 蓝色
                                if ((x == i && y == j - 1) || (x == i && y == j + 1))
                                {
                                    temporary_B += image[x][y];
                                    Bcount++;
                                }
                            }
                        }
                    }
                }
                rgbImage[i][j].g = (image[i][j] / 1) >> 2;
                rgbImage[i][j].r = (temporary_R / Rcount) >> 2;
                rgbImage[i][j].b = (temporary_B / Bcount) >> 2;
            }
            if (Blue)
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        if (x >= 0 && x < width && y >= 0 && y < height && !(x == i && y == j))
                        {
                            // 绿色插值上下左右
                            if ((x == i && y == j - 1) || (x == i && y == j + 1) || (x == i - 1 && y == j) || (x == i + 1 && y == j))
                            {
                                temporary_G += image[x][y];
                                Gcount++;
                            }
                            // 对蓝色四个角插值
                            if ((x == i - 1 && y == j - 1) || (x == i + 1 && y == j - 1) || (x == i - 1 && y == j + 1) || (x == i + 1 && y == j + 1))
                            {
                                temporary_B += image[x][y];
                                Bcount++;
                            }
                        }
                    }
                }
                rgbImage[i][j].g = (temporary_G / (uint16_t)Gcount) >> 2;
                rgbImage[i][j].r = (image[i][j] / (uint16_t)1) >> 2;
                rgbImage[i][j].b = (temporary_B / (uint16_t)Bcount) >> 2;
            }
            if (Red)
            {
                for (int x = i - 1; x <= i + 1; x++)
                {
                    for (int y = j - 1; y <= j + 1; y++)
                    {
                        if (x >= 0 && x < width && y >= 0 && y < height && !(x == i && y == j))
                        {
                            // 绿色插值上下左右
                            if ((x == i && y == j - 1) || (x == i && y == j + 1) || (x == i - 1 && y == j) || (x == i + 1 && y == j))
                            {
                                temporary_G += image[x][y];
                                Gcount++;
                            }
                            // 对四角红色插值
                            if ((x == i - 1 && y == j - 1) || (x == i + 1 && y == j - 1) || (x == i - 1 && y == j + 1) || (x == i + 1 && y == j + 1))
                            {
                                temporary_R += image[x][y];
                                Rcount++;
                            }
                        }
                    }
                }
                rgbImage[i][j].g = (temporary_G / (uint16_t)Gcount) >> 2;
                rgbImage[i][j].r = (temporary_R / (uint16_t)Rcount) >> 2;
                rgbImage[i][j].b = (image[i][j] / (uint16_t)1) >> 2;
            }
        }
    }
    return rgbImage;
}

/**
 * @author. zhl 2023-4-3
 * @brief RGB颜色插值灰色
 * @param[in] image image raw二维数组
 * @param[in] NULL
 * @param[in] NULL
 * @return NULL。
 *G B G B G B G B
 *R G R G R G R G
 *G B G B G B G B
 *R G R G R G R G
 *G B G B G B G B
 * @note 无。
 */
vector<vector<Pixel>> color_interpolation_grey(vector<vector<uint16_t>> &image)
{
    int height = image[0].size(), width = image.size();
    vector<vector<Pixel>> rgbImage(width, vector<Pixel>(height));
    for (int j = 0; j < width; j++)
    {
        for (int i = 0; i < height; i++)
        {
            // 抹干净高六位
            image[i][j] = image[i][j] & 0x3FF; // 0000001111111111
            // 右移两位
            image[i][j] = image[i][j] >> 2;
            rgbImage[i][j].b = image[i][j];
            rgbImage[i][j].g = image[i][j];
            rgbImage[i][j].r = image[i][j];
        }
    }
    return rgbImage;
}

vector<vector<Pixel>> color_interpolation_RAW(vector<vector<uint16_t>> &image)
{
    int height = image[0].size(), width = image.size();
    vector<vector<Pixel>> rgbImage(width, vector<Pixel>(height));
    for (int j = 0; j < width; j++)
    {
        for (int i = 0; i < height; i++)
        {
            bool Green = ((i % 2 == 0) && (j % 2 == 0)) || ((i % 2 == 1) && (j % 2 == 1));
            bool Red = (i % 2 == 1) && (j % 2 == 0);
            bool Blue = (i % 2 == 0) && (j % 2 == 1);
            // 抹干净高六位
            image[i][j] = image[i][j] & 0x3FF; // 0000001111111111
            // 右移两位
            image[i][j] = image[i][j] >> 2;
            if (Green)
            {
                rgbImage[i][j].b = 0;
                rgbImage[i][j].g = image[i][j];
                rgbImage[i][j].r = 0;
            }
            if (Red)
            {
                rgbImage[i][j].b = 0;
                rgbImage[i][j].g = 0;
                rgbImage[i][j].r = image[i][j];
            }
            if (Blue)
            {
                rgbImage[i][j].b = image[i][j];
                rgbImage[i][j].g = 0;
                rgbImage[i][j].r = 0;
            }
        }
    }
    return rgbImage;
}

vector<vector<YUYV>> convertRGB888toYUYV(vector<vector<Pixel>> &src)
{
    vector<vector<YUYV>> dst(src.size(), vector<YUYV>(src[0].size() / 2));
    for (int y = 0; y < src.size(); y++)
    {
        for (int x = 0; x < src[y].size(); x += 2)
        {
            Pixel p1 = src[y][x];
            Pixel p2 = src[y][x + 1];
            uint8_t Y1 = 0.299 * p1.r + 0.587 * p1.g + 0.114 * p1.b;
            uint8_t Y2 = 0.299 * p2.r + 0.587 * p2.g + 0.114 * p2.b;
            int16_t U = -0.14713 * p1.r - 0.28886 * p1.g + 0.436 * p1.b;
            int16_t V = 0.615 * p1.r - 0.51498 * p1.g - 0.10001 * p1.b;
            dst[y][x / 2] = {Y1, static_cast<uint8_t>((U + V) / 2.0), Y2, static_cast<uint8_t>((U + V) / 2.0)};
        }
    }
    return dst;
}

/**
 * author. zhl 2023-4-7
 *
 * @brief 计算白平衡常数
 * @param[in] image RGB数组
 * @param[in] NULL
 * @param[in] NULL
 * @return 返回白平衡常数值。
 *
 * @note
 *  //1.求RGB每个像素点的亮度
 *  //2.对不同的亮度点进行分类
 *  //3.判断是否接近灰色
 *  //4.求每种不同类型亮度的权重
 *  //5.R B 总和求平均值
 */
Return_GainR_GrainB calculate_white_balance_nums(vector<vector<Pixel>> &image)
{
    // 开辟返回值结构体
    Return_GainR_GrainB return_GB;
    // 算出传入RGB数组的大小
    int width = image.size(), height = image[0].size();
    // 返回一个八位的值
    uint8_t balance_num = 0;
    double GainR = 0, GainB = 0;
    // 64位累加x像素值防止溢出 开辟三个结构体 存三组数据总和
    RGB_total rgb_total[3];
    rgb_total[0].total = 0, rgb_total[1].total = 0, rgb_total[2].total = 0;
    rgb_total[0].R_total = 0, rgb_total[1].R_total = 0, rgb_total[2].R_total = 0;
    rgb_total[0].G_total = 0, rgb_total[1].G_total = 0, rgb_total[2].G_total = 0;
    rgb_total[0].B_total = 0, rgb_total[1].B_total = 0, rgb_total[2].B_total = 0;
    // memset(&rgb_total, 0, sizeof(rgb_total)*3);
    //  创建VUV分量数组
    vector<vector<YUV_float>> yuvImage(width, vector<YUV_float>(height));
    // 循环遍历所有像素
    // 横轴
    int count = 0;
    for (int i = 0; i < width; i++)
    {
        // 竖轴
        for (int j = 0; j < height; j++)
        {
            // 对每个像素利用矩阵算出它的 Y 值并且存入YUV_float

            yuvImage[i][j].YUV_y = (double)Y((double)image[i][j].r, (double)image[i][j].g, (double)image[i][j].b);
            // cout << yuvImage[i][j].YUV_y << "  " <<(double)image[i][j].r << "  " <<(double)image[i][j].g << " "<<(double)image[i][j].b<< endl;
            // cout << (double)image[i][j].r /(double)image[i][j].g  <<" " <<(double)image[i][j].b /(double)image[i][j].g<<endl;
            // cout << (double)((double)image[i][j].r / yuvImage[i][j].YUV_y) <<" " <<(double)((double)image[i][j].b / yuvImage[i][j].YUV_y)<<endl;
            // 判断灰度范围
            // 改成大于0.6左右 R/G约等于0.6
            if (yuvImage[i][j].YUV_y != 0)
            {
                // cout << ((double)image[i][j].r / (double)image[i][j].g) << endl;
                // if (((double)((double)image[i][j].r / (double)image[i][j].g) > R_Y) && ((double)((double)image[i][j].b / (double)image[i][j].g) > B_Y))
                if (((yuvImage[i][j].YUV_u < 30) && (yuvImage[i][j].YUV_u > -30)) && ((yuvImage[i][j].YUV_v > -30) && (yuvImage[i][j].YUV_v < 30)))
                {
                    // 判断亮度范围

                    if ((yuvImage[i][j].YUV_y >= 0x40) && (yuvImage[i][j].YUV_y < 0xc0)) // 128±64
                    {
                        yuvImage[i][j].type = 64;
                        if ((yuvImage[i][j].YUV_y >= 0x58) && (yuvImage[i][j].YUV_y < 0xa8)) // 128±40
                        {
                            yuvImage[i][j].type = 40;
                            if ((yuvImage[i][j].YUV_y >= 0x6c) && (yuvImage[i][j].YUV_y < 0x94)) // 128±20
                            {
                                yuvImage[i][j].type = 20;
                            }
                        }
                    }
                }
            }
            // 判断类型并且累计
            if (yuvImage[i][j].type == 64)
            {
                rgb_total[0].R_total += image[i][j].r;
                rgb_total[0].G_total += image[i][j].g;
                rgb_total[0].B_total += image[i][j].b;
                rgb_total[0].total++;
                count++;
            }
            if (yuvImage[i][j].type == 40)
            {
                rgb_total[1].R_total += image[i][j].r;
                rgb_total[1].G_total += image[i][j].g;
                rgb_total[1].B_total += image[i][j].b;
                rgb_total[1].total++;
            }
            if (yuvImage[i][j].type == 20)
            {
                rgb_total[2].R_total += image[i][j].r;
                rgb_total[2].G_total += image[i][j].g;
                rgb_total[2].B_total += image[i][j].b;
                rgb_total[2].total++;
            }
        }
    }
    // cout << rgb_total[0].total <<endl;
    // cout << rgb_total[1].total <<endl;
    // cout << rgb_total[2].total <<endl;
    // cout << rgb_total[0].R_total << " " << rgb_total[0].G_total << " " << rgb_total[0].B_total << " " << rgb_total[0].total << endl;
    // cout << rgb_total[1].R_total << " " << rgb_total[1].G_total << " " << rgb_total[1].B_total << " " << rgb_total[1].total << endl;
    // cout << rgb_total[2].R_total << " " << rgb_total[2].G_total << " " << rgb_total[2].B_total << " " << rgb_total[2].total << endl;

    // 求平均值 128±64
    rgb_total[0].avg_R = (double)rgb_total[0].R_total / (double)rgb_total[0].total;
    rgb_total[0].avg_G = (double)rgb_total[0].G_total / (double)rgb_total[0].total;
    rgb_total[0].avg_B = (double)rgb_total[0].B_total / (double)rgb_total[0].total;
    // // 128±40
    rgb_total[1].avg_R = (double)rgb_total[1].R_total / (double)rgb_total[1].total;
    rgb_total[1].avg_G = (double)rgb_total[1].G_total / (double)rgb_total[1].total;
    rgb_total[1].avg_B = (double)rgb_total[1].B_total / (double)rgb_total[1].total;
    // // 128±20
    rgb_total[2].avg_R = (double)rgb_total[2].R_total / (double)rgb_total[2].total;
    rgb_total[2].avg_G = (double)rgb_total[2].G_total / (double)rgb_total[2].total;
    rgb_total[2].avg_B = (double)rgb_total[2].B_total / (double)rgb_total[2].total;
    // // // 配置权重 对64配置
    rgb_total[0].avg_R = rgb_total[0].avg_R * 0.2;
    rgb_total[0].avg_G = rgb_total[0].avg_G * 0.2;
    rgb_total[0].avg_B = rgb_total[0].avg_B * 0.2;
    // 对40配置
    rgb_total[1].avg_R = rgb_total[1].avg_R * 0.5;
    rgb_total[1].avg_G = rgb_total[1].avg_G * 0.5;
    rgb_total[1].avg_B = rgb_total[1].avg_B * 0.5;
    // 求平均
    GainR = (rgb_total[0].avg_G + rgb_total[1].avg_G + rgb_total[2].avg_G) / (rgb_total[0].avg_R + rgb_total[1].avg_R + rgb_total[2].avg_R);
    GainB = (rgb_total[0].avg_G + rgb_total[1].avg_G + rgb_total[2].avg_G) / (rgb_total[0].avg_B + rgb_total[1].avg_B + rgb_total[2].avg_B);
    //  返回值
    return_GB.GainR = GainR;
    return_GB.GainB = GainB;
    return return_GB;
}

/**
 * author. zhl 2023-4-7
 *
 * @brief 伽马矫正
 * @param[in] image RGB 格式数组
 * @param[in] gamma 表示伽马值，即取幂的指数
 * @param[in] NULL
 * @return 伽马矫正后的数组
 *
 * @note 在数字图像处理领域中，一般认为伽马值在 1.8 到 2.2 之间最为常见。
 * 在实际应用中，可以根据具体情况来确定伽马值，例如在显示器校正时，可以使用专业的校色仪来测量伽马值。
 */
vector<vector<Pixel>> Gamma_correction(vector<vector<Pixel>> &image, float gamma)
{
    for (int i = 0; i < image.size(); i++)
    {
        for (int j = 0; j < image[i].size(); j++)
        {
            /*
            / 255.0f 表示将该像素值除以 255.0，将像素值归一化到 [0, 1] 的范围内。
            这一步是为了将像素值转化为浮点类型，方便进行伽马矫正运算。
            pow(image[i][j].r / 255.0f, gamma) 表示对归一化后的像素值进行伽马矫正运算，其中 pow() 函数表示对指定数值取幂。
            第一个参数是要取幂的数值，第二个参数是幂指数，即伽马值。
            */
            image[i][j].r = pow((double)image[i][j].r / 255.0f, gamma) * 255.0f;
            image[i][j].g = pow((double)image[i][j].g / 255.0f, gamma) * 255.0f;
            image[i][j].b = pow((double)image[i][j].b / 255.0f, gamma) * 255.0f;
        }
    }
    return image;
}

// 1.546875        -0.397460938        -0.149414063
// -0.247070313        1.258789063        -0.01171875
// -0.102539063        -0.844726563        1.947265625
// 颜色矩阵  乘积
void matmul3x3_3x1(vector<vector<Pixel>> &mat3x1)
{
    // 检查输入矩阵的形状是否正确
    vector<vector<double>> mat3x3(3, vector<double>(3));
    mat3x3[0][0] = 1.546875, mat3x3[0][1] = -0.397460938, mat3x3[0][2] = -0.149414063;
    mat3x3[1][0] = -0.247070313, mat3x3[1][1] = 1.258789063, mat3x3[1][2] = -0.01171875;
    mat3x3[2][0] = -0.102539063, mat3x3[2][1] = -0.844726563, mat3x3[2][2] = 1.947265625;
    // 对每一个像素计算矩阵乘积
    for (int i = 0; i < mat3x1.size(); i++)
    {
        for (int j = 0; j < mat3x1[0].size(); j++)
        {
            int number = (10 * (double)mat3x1[i][j].r * mat3x3[0][0]) / 10 + (10 * (double)mat3x1[i][j].g * mat3x3[0][1]) / 10 + (10 * (double)mat3x1[i][j].b * mat3x3[0][2]) / 10;

            // cout << "AAA "<<(10*(double)mat3x1[i][j].r*mat3x3[0][0])/10  + (10*(double)mat3x1[i][j].g*mat3x3[0][1])/10 + (10*(double)mat3x1[i][j].b*mat3x3[0][2])/10 << endl;
            // mat3x1[i][j].g = ((10*(double)mat3x1[i][j].r*mat3x3[1][0])/10 + (10*(double)mat3x1[i][j].g*mat3x3[1][1])/10 + (10*(double)mat3x1[i][j].b*mat3x3[1][2])/10);
            // mat3x1[i][j].b = ((10*(double)mat3x1[i][j].r*mat3x3[2][0])/10 + (10*(double)mat3x1[i][j].g*mat3x3[2][1])/10 + (10*(double)mat3x1[i][j].b*mat3x3[2][2])/10);
        }
    }
}

void write_rgb_file(vector<vector<Pixel>> &mat3x1, const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        cout << "Open file failed!" << endl;
        return;
    }
    // 写入宽度和高度
    int width = mat3x1[0].size();
    int height = mat3x1.size();
    fwrite(&width, sizeof(int), 1, fp);
    fwrite(&height, sizeof(int), 1, fp);
    // 写入RGB值
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fwrite(&mat3x1[i][j].r, sizeof(uint8_t), 1, fp);
            fwrite(&mat3x1[i][j].g, sizeof(uint8_t), 1, fp);
            fwrite(&mat3x1[i][j].b, sizeof(uint8_t), 1, fp);
        }
    }
    fclose(fp);
}

void read_rgb_file(vector<vector<Pixel>> &mat3x1, const char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        cout << "Open file failed!" << endl;
        return;
    }
    // 读取宽度和高度
    int width, height;
    fread(&width, sizeof(int), 1, fp);
    fread(&height, sizeof(int), 1, fp);
    // 创建空的图像矩阵
    mat3x1.resize(height);
    for (int i = 0; i < height; i++)
    {
        mat3x1[i].resize(width);
    }
    // 读取RGB值
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fread(&mat3x1[i][j].r, sizeof(uint8_t), 1, fp);
            fread(&mat3x1[i][j].g, sizeof(uint8_t), 1, fp);
            fread(&mat3x1[i][j].b, sizeof(uint8_t), 1, fp);
        }
    }
    fclose(fp);
}

// /////////RGB -> JPEG
// //第一步 把RGB域转到YCbCr域
vector<vector<Ycrcb>> rgb2ycbcr(vector<vector<Pixel>> &mat3x1)
{
    vector<vector<Ycrcb>> ycrcb(mat3x1.size(), vector<Ycrcb>(mat3x1[0].size()));
    for (int i = 0; i < mat3x1.size(); i++)
    {
        for (int j = 0; j < mat3x1[i].size(); j++)
        {
            unsigned int y = 0.299 * mat3x1[i][j].r + 0.587 * mat3x1[i][j].g + 0.114 * mat3x1[i][j].b;
            unsigned int cb = -0.1687 * mat3x1[i][j].r - 0.3313 * mat3x1[i][j].g + 0.5 * mat3x1[i][j].b + 128;
            unsigned int cr = 0.5 * mat3x1[i][j].r - 0.4187 * mat3x1[i][j].g - 0.0813 * mat3x1[i][j].b + 128;
            ycrcb[i][j].y = y;
            ycrcb[i][j].cb = cb;
            ycrcb[i][j].cr = cr;
        }
    }
    return ycrcb;
}

// 亮度量化，色度量化
//  亮度量化表
/*
在亮度量化函数中，将YCbCr图像数据中的亮度分量除以亮度量化表中对应的量化因子，得到量化后的亮度数据。
在色度量化函数中，将YCbCr图像数据中的色度分量分别除以色度量化表中对应的量化因子，得到量化后的色度数据。
最终，将量化后的数据以vector<vector>的形式返回。
*/

// 亮度量化
vector<vector<int>> Quantization(const vector<vector<Ycrcb>> &YCbCrData, const vector<vector<int>> &luminanceQuantizationTable, const vector<vector<int>> &chrominanceQuantizationTable)
{
    vector<vector<int>> quantizedData(YCbCrData.size(), vector<int>(YCbCrData[0].size()));
    for (int i = 0; i < YCbCrData.size(); i++)
    {
        for (int j = 0; j < YCbCrData[0].size(); j++)
        {
            quantizedData[i][j] = YCbCrData[i][j].y / luminanceQuantizationTable[i % 8][j % 8];
            quantizedData[i][j] = YCbCrData[i][j].cb / chrominanceQuantizationTable[i % 8][j % 8];
            quantizedData[i][j] = YCbCrData[i][j].cr / chrominanceQuantizationTable[i % 8][j % 8];
        }
    }
    return quantizedData;
}

/*
离散余弦变换（DCT）是将一个长度为N的一维信号或大小为N×N的二维信号，转换为一组基函数的线性组合，其中基函数是余弦函数。
DCT可以将信号从时域转换到频域，是一种常用的信号处理技术，在JPEG图像压缩中也被广泛应用。
下面是对于输入的二维数据进行DCT变换的函数实现，其中输入的是量化后的YCbCr数据，输出的是DCT系数矩阵：
对于每个DCT系数(i,j)，都需要利用输入的量化后的数据计算得到，其中alpha和beta分别是根据DCT变换公式得到的系数。
在双重循环中，计算每个DCT系数的值，最后将计算得到的DCT系数存储到一个新的二维向量中，并返回该二维向量作为函数的输出结果。
*/

// https://blog.csdn.net/newchenxf/article/details/51719597
const double PI = 3.14159265358979323846;
// 定义DCT函数，输入为量化后的数据二维向量，输出为DCT系数二维向量
// vector<vector<double>> DCT(const vector<vector<int>>& quantizedData)
// {
//     int N = quantizedData.size();  // 获取量化后的数据的大小，即N*N
//     vector<vector<double>> dctData(N, vector<double>(N));  // 定义DCT系数矩阵，初始化为0
//     double alpha, beta;  // 定义alpha和beta两个系数
//     for (int u = 0; u < N; u++)  // 双重循环遍历每个DCT系数
//     {
//         for (int v = 0; v < N; v++)
//         {
//             // 根据DCT变换公式计算alpha和beta系数
//             if (u == 0)
//             {
//                 alpha = sqrt(1.0 / N);
//             }
//             else
//             {
//                 alpha = sqrt(2.0 / N);
//             }
//             if (v == 0)
//             {
//                 beta = sqrt(1.0 / N);
//             }
//             else
//             {
//                 beta = sqrt(2.0 / N);
//             }
//             double sum = 0;  // 定义sum用来累加计算每个DCT系数的值
//             for (int i = 0; i < N; i++)  // 双重循环遍历输入的量化后的数据
//             {
//                 for (int j = 0; j < N; j++)
//                 {
//                     // 根据DCT变换公式计算每个DCT系数的值
//                     sum += quantizedData[i][j] * cos((2.0 * i + 1.0) * u * M_PI / (2.0 * N)) * cos((2.0 * j + 1.0) * v * M_PI / (2.0 * N));

//                 }
//             }
//             dctData[u][v] = alpha * beta * sum;  // 将计算得到的DCT系数存储到DCT系数矩阵中
//         }
//     }
//     return dctData;  // 返回DCT系数矩阵
// }
void DTC(vector<vector<int>> &image, vector<vector<double>> &dct_coeffs)
{
    int height = image.size();
    int width = image[0].size();
    int padded_height = ceil((double)height / BLOCK_SIZE) * BLOCK_SIZE;
    int padded_width = ceil((double)width / BLOCK_SIZE) * BLOCK_SIZE;
    // 填充图像边缘
    for (int i = height; i < padded_height; i++)
    {
        vector<int> empty_row(padded_width, 0);
        image.push_back(empty_row);
        vector<double> empty_row_dct(padded_width, 0.0);
        dct_coeffs.push_back(empty_row_dct);
    }
    for (int i = 0; i < height; i++)
    {
        for (int j = width; j < padded_width; j++)
        {
            image[i].push_back(0);
            dct_coeffs[i].push_back(0.0);
        }
    }
    // 对图像进行分块处理，每个块大小为 BLOCK_SIZE x BLOCK_SIZE
    for (int i = 0; i < padded_height; i += BLOCK_SIZE)
    {
        for (int j = 0; j < padded_width; j += BLOCK_SIZE)
        {
            // 取出一个块
            vector<vector<int>> block(BLOCK_SIZE, vector<int>(BLOCK_SIZE, 0));
            for (int k = 0; k < BLOCK_SIZE; k++)
            {
                for (int l = 0; l < BLOCK_SIZE; l++)
                {
                    block[k][l] = image[i + k][j + l];
                }
            }
            // 对块进行 DCT 变换
            vector<vector<double>> dct_block(BLOCK_SIZE, vector<double>(BLOCK_SIZE, 0.0));
            for (int u = 0; u < BLOCK_SIZE; u++)
            {
                for (int v = 0; v < BLOCK_SIZE; v++)
                {
                    double sum = 0.0;
                    for (int x = 0; x < BLOCK_SIZE; x++)
                    {
                        for (int y = 0; y < BLOCK_SIZE; y++)
                        {
                            sum += block[x][y] * cos((2 * x + 1) * u * M_PI / 16) * cos((2 * y + 1) * v * M_PI / 16);
                        }
                    }
                    sum *= (u == 0 ? 1.0 / sqrt(2) : 1.0) * (v == 0 ? 1.0 / sqrt(2) : 1.0) * 0.25;
                    dct_block[u][v] = sum;
                }
            }
            // 保存 DCT 系数
            for (int k = 0; k < BLOCK_SIZE; k++)
            {
                for (int l = 0; l < BLOCK_SIZE; l++)
                {
                    dct_coeffs[i + k][j + l] = dct_block[k][l];
                }
            }
        }
    }
}

/*
该函数接受三个输入参数：DCT系数矩阵dct_coefficients、量化表quant_table和矩阵大小N。
函数通过两层循环遍历DCT系数矩阵中的每个系数，将其按照量化表进行量化，并将量化后的系数四舍五入为整数。
最终得到的量化后的系数矩阵仍然保存在dct_coefficients数组中。
*/
void quantize(vector<vector<double>> &dct_coefficients, vector<vector<int>> &quant_table, int N)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            // 将DCT系数矩阵中的每个系数按照量化表进行量化
            dct_coefficients[i][j] = round(dct_coefficients[i][j] / quant_table[i][j]);
            // 将量化后的系数四舍五入为整数
            dct_coefficients[i][j] = (int)dct_coefficients[i][j];
        }
    }
}

// 获取整数的二进制位数
int get_bit_count(int n)
{
    return (n == 0) ? 1 : (int)log2(n) + 1;
}

string fading_entropy_encode(const vector<vector<double>> &quant_coeffs)
{
    // 游长编码函数
    auto run_length_encode = [](const vector<double> &quant_coeffs)
    {
        string binary_str;
        int zero_count = 0;
        for (int i = 0; i < quant_coeffs.size(); i++)
        {
            if (quant_coeffs[i] == 0)
            {
                zero_count++;
            }
            else
            {
                // 将上一个零元素的个数转换为二进制字符串
                if (zero_count > 0)
                {
                    int bit_count = get_bit_count(zero_count);
                    bitset<4> bit_str(bit_count);
                    string zero_count_str = bit_str.to_string().substr(4 - bit_count);
                    binary_str += zero_count_str;
                }
                // 将非零元素的绝对值转换为二进制字符串
                bitset<16> bit_str(abs(quant_coeffs[i]));
                if (quant_coeffs[i] < 0)
                {
                    bit_str.flip();
                }
                string coeff_str = bit_str.to_string();
                binary_str += coeff_str;
                zero_count = 0;
            }
        }
        // 如果最后一个元素是零元素，则需要将其个数写入二进制字符串中
        if (zero_count > 0)
        {
            int bit_count = get_bit_count(zero_count);
            bitset<4> bit_str(bit_count);
            string zero_count_str = bit_str.to_string().substr(4 - bit_count);
            binary_str += zero_count_str;
        }
        return binary_str;
    };
    string binary_str;
    int num_blocks = quant_coeffs.size();
    for (int i = 0; i < num_blocks; i++)
    {
        int block_size = quant_coeffs[i].size();
        // 将 DCT 系数转换为整数并进行淡出处理
        vector<double> double_coeffs(block_size);
        for (int j = 0; j < block_size; j++)
        {
            double_coeffs[j] = quant_coeffs[i][j] * pow(2, -0.5 * (j / 8 + j % 8));
        }
        // 进行游长编码
        binary_str += run_length_encode(double_coeffs);
    }
    return binary_str;
}

string compress(const string &str)
{
    // 将字符串转换为 char 数组
    const char *input = str.c_str();
    uLong input_size = (uLong)str.size();
    // 计算压缩后的最大长度
    uLong output_size = compressBound(input_size);
    // 分配存储压缩数据的空间
    char *output = new char[output_size];
    // 进行压缩
    compress2((Bytef *)output, &output_size, (const Bytef *)input, input_size, Z_BEST_COMPRESSION);
    // 将压缩后的数据转换为字符串
    string compressed_str(output, output + output_size);
    // 释放空间
    delete[] output;
    return compressed_str;
}

bool write_jpg_file(const string &filename, const unsigned char *data, int width, int height)
{
    // 打开 JPG 文件
    ofstream outfile(filename.c_str(), ios::out | ios::binary);
    if (!outfile.is_open())
    {
        cerr << "Error: Failed to open file " << filename << endl;
        return false;
    }
    // 写入 JPG 文件头
    const unsigned char header[] = {
        0xff, 0xd8,                   // SOI
        0xff, 0xe0,                   // APP0
        0x00, 0x10,                   // APP0 Length
        0x4a, 0x46, 0x49, 0x46, 0x00, // Identifier (JFIF\0)
        0x01, 0x02,                   // Version (1.2)
        0x00,                         // Units
        0x00, 0x01,                   // X density
        0x00, 0x01,                   // Y density
        0x00,                         // X thumbnail
        0x00                          // Y thumbnail
    };
    outfile.write((const char *)header, sizeof(header));
    // 写入 JPG 图像数据
    const int row_stride = width * 3;
    for (int y = 0; y < height; ++y)
    {
        outfile.write((const char *)&data[y * row_stride], row_stride);
    }
    // 写入 JPG 文件尾
    const unsigned char footer[] = {
        0xff, 0xd9 // EOI
    };
    outfile.write((const char *)footer, sizeof(footer));
    // 关闭文件
    outfile.close();
    return true;
}

void Turn_around(vector<vector<uint16_t>> &image)
{
    // 图片对称翻转
    uint16_t temp = 0; // 临时变量
    for (int i = 0; i < image.size(); i++)
    {
        for (int j = 0; j < i; j++)
        {
            if (i == j)
                continue;                                                       // 如果是中轴线则不调换
            temp = image[i][j];                                                 // 先保存变量
            image[i][j] = image[image.size() - i - 1][image[0].size() - j - 1]; // 把镜像变量赋值过来
            image[image.size() - i - 1][image[0].size() - j - 1] = temp;        // 赋值过去
        }
    }
}

// GBRG 适用于
// 四通道黑电平矫正
void Black_Level_Correction_respective(vector<vector<uint16_t>> &image)
{
    int height = image.size(), width = image[0].size(), i = 0, j = 0;
    uint16_t min_Green_1 = image.at(0).at(0);
    uint16_t min_Green_2 = image.at(1).at(1);
    uint16_t min_Red = image.at(0).at(1);
    uint16_t min_Blue = image.at(1).at(0);
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            bool Green_1 = ((i % 2 == 0) && (j % 2 == 0));
            bool Green_2 = ((i % 2 == 1) && (j % 2 == 1));
            bool Red = (i % 2 == 1) && (j % 2 == 0);
            bool Blue = (i % 2 == 0) && (j % 2 == 1);
            if (Green_1)
            { // 找出绿色奇数行min
                if (image.at(i).at(j) < min_Green_1)
                {
                    min_Green_1 = image.at(i).at(j);
                }
                continue;
            }
            if (Green_2)
            { // 找出绿色偶数行min
                if (image.at(i).at(j) < min_Green_2)
                {
                    min_Green_2 = image.at(i).at(j);
                }
                continue;
            }
            if (Red)
            { // 找出红色min
                if (image.at(i).at(j) < min_Red)
                {
                    min_Red = image.at(i).at(j);
                }
                continue;
            }
            if (Blue)
            { // 找出蓝色min
                if (image.at(i).at(j) < min_Blue)
                {
                    min_Blue = image.at(i).at(j);
                }
                continue;
            }
        }
    }
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            bool Green_1 = ((i % 2 == 0) && (j % 2 == 0));
            bool Green_2 = ((i % 2 == 1) && (j % 2 == 1));
            bool Red = (i % 2 == 1) && (j % 2 == 0);
            bool Blue = (i % 2 == 0) && (j % 2 == 1);
            if (Green_1)
            { // 找出绿色奇数行min
                image.at(i).at(j) = image.at(i).at(j) - min_Green_1;
                continue;
            }
            if (Green_2)
            { // 找出绿色偶数行min
                image.at(i).at(j) = image.at(i).at(j) - min_Green_2;
                continue;
            }
            if (Red)
            { // 找出红色min
                image.at(i).at(j) = image.at(i).at(j) - min_Red;
                continue;
            }
            if (Blue)
            { // 找出蓝色min
                image.at(i).at(j) = image.at(i).at(j) - min_Blue;
                continue;
            }
        }
    }
}

void white_balance(Return_GainR_GrainB RB, vector<vector<uint16_t>> &image)
{
    int height = image.size(), width = image[0].size(), i = 0, j = 0;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            bool Red = (i % 2 == 1) && (j % 2 == 0);
            bool Blue = (i % 2 == 0) && (j % 2 == 1);
            if (Red)
            {
                image.at(i).at(j) = image.at(i).at(j) * RB.GainB;
            }
            if (Blue)
            {
                image.at(i).at(j) = image.at(i).at(j) * RB.GainR;
            }
            if (image.at(i).at(j) > 0x3ff)
            {
                image.at(i).at(j) = 0x3ff;
            }
        }
    }
}

void reduce_red(vector<vector<Pixel>> &RGB)
{
    int height = RGB.size(), width = RGB[0].size(), i = 0, j = 0;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if(Y(RGB[i][j].r,RGB[i][j].g,RGB[i][j].b)>230)
            {
              RGB[i][j].r = RGB[i][j].g; 
              RGB[i][j].b = RGB[i][j].g; 
            }
        }
    }
}

void rawDenoising(int* img, int width, int height, int window_size, int threshold) {
    int half_size = window_size / 2;
    int* tmp = new int[window_size * window_size];
    for (int i = half_size; i < height - half_size; i++) {
        for (int j = half_size; j < width - half_size; j++) {
            int sum = 0;
            int count = 0;
            for (int k = -half_size; k <= half_size; k++) {
                for (int l = -half_size; l <= half_size; l++) {
                    tmp[count++] = *(img + (i + k) * width + j + l);
                    sum += *(img + (i + k) * width + j + l);
                }
            }
            int mean = sum / count;
            int var = 0;
            for (int k = 0; k < count; k++) {
                var += (tmp[k] - mean) * (tmp[k] - mean);
            }
            var /= count;
            if (var > threshold) {
                *(img + i * width + j) = mean;
            }
        }
    }
    delete[] tmp;
}


void cruculation_R_G_B(vector<vector<Pixel>> &RGB)
{
    int height = RGB.size(), width = RGB[0].size(), i = 0, j = 0;
    uint64_t R = 0,G = 0,B = 0;
    uint64_t RR = 0,GG = 0,BB = 0;
    int total = 0;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            R += RGB[i][j].r;
            G += RGB[i][j].g;
            B += RGB[i][j].b;
            total++;
        }
    }
    RR = R / total;
    GG = G / total;
    BB = B / total;
    cout << RR <<" "<< GG <<" "<< BB;
}
