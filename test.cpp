#include <vector>
#include <cmath>
using namespace std;
vector<vector<int>> DCT(const vector<vector<int>>& imageData)
{
    int N = imageData.size();  // 获取图像数据的大小，即N*N
    vector<vector<double>> dctData(N, vector<double>(N));  // 定义DCT系数矩阵，初始化为0
    double alpha, beta;  // 定义alpha和beta两个系数
    for (int u = 0; u < N; u++)  // 双重循环遍历每个DCT系数
    {
        for (int v = 0; v < N; v++)
        {
            // 根据DCT变换公式计算alpha和beta系数
            if (u == 0)
            {
                alpha = sqrt(1.0 / N);
            }
            else
            {
                alpha = sqrt(2.0 / N);
            }
            if (v == 0)
            {
                beta = sqrt(1.0 / N);
            }
            else
            {
                beta = sqrt(2.0 / N);
            }
            double sum = 0;  // 定义sum用来累加计算每个DCT系数的值
            for (int i = 0; i < N; i++)  // 双重循环遍历输入的图像数据
            {
                for (int j = 0; j < N; j++)
                {
                    // 根据DCT变换公式计算每个DCT系数的值
                    sum += imageData[i][j] * cos((2.0 * i + 1.0) * u * M_PI / (2.0 * N)) * cos((2.0 * j + 1.0) * v * M_PI / (2.0 * N));
                }
            }
            dctData[u][v] = alpha * beta * sum;  // 将计算得到的DCT系数存储到DCT系数矩阵中
        }
    }
    vector<vector<int>> quantizedData(N, vector<int>(N));  // 定义量化后的数据矩阵
    for (int i = 0; i < N; i++)  // 双重循环遍历DCT系数矩阵
    {
        for (int j = 0; j < N; j++)
        {
            // 对于每个DCT系数，将其除以相应的量化因子，再四舍五入取整，得到量化后的数据
            quantizedData[i][j] = round(dctData[i][j] / quantizationTable[i][j]);
        }
    }
    return quantizedData;  // 返回量化后的数据矩阵
}