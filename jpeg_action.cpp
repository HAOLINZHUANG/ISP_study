#include "jpeg_action.h"
#include "Rgb_yuv.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "jpeglib.h"
using namespace std;



// 将 RGB888 格式的图像转换为 JPEG 格式并保存到文件
// 参数：
//   filename: 保存的 JPEG 文件名
//   data: RGB888 格式的图像数据，大小为 width * height * 3 字节
//   width: 图像宽度
//   height: 图像高度
//   quality: JPEG 压缩质量，取值范围为 0-100，值越高表示质量越好
void write_jpeg_file(const char* filename, vector<vector<Pixel>>& data, int width, int height, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* outfile;
    JSAMPROW row_pointer[1];
    int row_stride;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    if ((outfile = fopen(filename, "wb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        exit(1);
    }
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3; // R, G, B
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * 3;
    while (cinfo.next_scanline < cinfo.image_height) {
        int y = cinfo.next_scanline;
        row_pointer[0] = &data[y][0].r;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
}

