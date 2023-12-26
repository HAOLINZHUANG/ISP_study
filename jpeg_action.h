#ifndef JPEG_ACTION_H
#define JPEG_ACTION_H
#include <vector>
#include "Rgb_yuv.h"
#if 1
using namespace std;
#endif

void write_jpeg_file(const char* filename, vector<vector<Pixel>>& data, int width, int height, int quality);

#endif