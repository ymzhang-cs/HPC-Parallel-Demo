#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <time.h>
#include <string>
#include <stdint.h>

namespace py = pybind11;
using namespace cv;

#pragma pack(1)
typedef struct
{
    unsigned char bfType[2];      // "BM"
    uint32_t bfSize;              // 文件大小
    uint16_t bfReserved1;         // 保留字段1
    uint16_t bfReserved2;         // 保留字段2
    uint32_t bfOffBits;           // 数据偏移量
} fileHeader;

typedef struct
{
    uint32_t biSize;              // 信息头大小
    int32_t biWidth;              // 图像宽度
    int32_t biHeight;             // 图像高度
    uint16_t biPlanes;            // 平面数
    uint16_t biBitCount;          // 每像素位数
    uint32_t biCompression;       // 压缩类型
    uint32_t biSizeImage;         // 图像数据大小
    int32_t biXPixPerMeter;       // 水平分辨率
    int32_t biYPixPerMeter;       // 垂直分辨率
    uint32_t biClrUsed;           // 使用的颜色数
    uint32_t biClrImportant;      // 重要颜色数
} fileInfo;

typedef struct
{
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} rgbq;
#pragma pack()

unsigned char clamp(int val)
{
    if (val < 0)
        return 0;
    if (val > 255)
        return 255;
    return (unsigned char)val;
}

void write_bmp_header(FILE *fp, fileHeader *fh, fileInfo *fi, rgbq *palette)
{
    fwrite(fh, sizeof(fileHeader), 1, fp);
    fwrite(fi, sizeof(fileInfo), 1, fp);
    fwrite(palette, sizeof(rgbq), 256, fp);
}

// 封装为Python可调用的函数
double convert_to_grayscale_py(const std::string &input, const std::string &output)
{
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader *fh = (fileHeader *)malloc(sizeof(fileHeader));
    fileInfo *fi = (fileInfo *)malloc(sizeof(fileInfo));
    fread(fh, sizeof(fileHeader), 1, in);
    fread(fi, sizeof(fileInfo), 1, in);

    double start_time, end_time;
    start_time = omp_get_wtime();
    int width = fi->biWidth, height = fi->biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    // 初始化新的文件头
    fileHeader newFh;
    fileInfo newFi;
    
    // 复制文件头信息
    memcpy(&newFh, fh, sizeof(fileHeader));
    memcpy(&newFi, fi, sizeof(fileInfo));
    
    // 设置新的参数
    newFi.biBitCount = 8;
    newFi.biCompression = 0;
    newFi.biSizeImage = ((width + 3) / 4) * 4 * height;
    newFi.biPlanes = 1;
    newFi.biClrUsed = 256;
    newFi.biClrImportant = 256;
    
    newFh.bfOffBits = sizeof(fileHeader) + sizeof(fileInfo) + 256 * sizeof(rgbq);
    newFh.bfSize = newFh.bfOffBits + newFi.biSizeImage;

    rgbq *palette = (rgbq *)malloc(256 * sizeof(rgbq));
#pragma omp parallel for
    for (int i = 0; i < 256; i++)
    {
        palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = i;
        palette[i].rgbReserved = 0;
    }

    fwrite(&newFh, sizeof(fileHeader), 1, out);
    fwrite(&newFi, sizeof(fileInfo), 1, out);
    fwrite(palette, sizeof(rgbq), 256, out);

    // 分配内存并读取整个图像
    unsigned char *rgbData = (unsigned char *)malloc(rowSize * height);
    unsigned char *grayData = (unsigned char *)malloc(((width + 3) / 4) * 4 * height);
    
    // 读取所有行
    bool bottom_up = (fi->biHeight > 0);
    int actual_height = abs(fi->biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(rgbData + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(rgbData + i * rowSize, 1, rowSize, in);
        }
    }

    // 并行处理像素转换
#pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int b = rgbData[i * rowSize + j * 3];
            int g = rgbData[i * rowSize + j * 3 + 1];
            int r = rgbData[i * rowSize + j * 3 + 2];
            grayData[i * ((width + 3) / 4) * 4 + j] = (r + g + b) / 3;
        }
    }

    // 写入输出文件
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(grayData + i * ((width + 3) / 4) * 4, 1, ((width + 3) / 4) * 4, out);
    }
    
    end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;

    free(fh);
    free(fi);
    free(palette);
    free(rgbData);
    free(grayData);
    fclose(in);
    fclose(out);

    return elapsed_time;
}

double convert_to_binary_py(const std::string &input, const std::string &output, int threshold)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader *fh = (fileHeader *)malloc(sizeof(fileHeader));
    fileInfo *fi = (fileInfo *)malloc(sizeof(fileInfo));
    fread(fh, sizeof(fileHeader), 1, in);
    fread(fi, sizeof(fileInfo), 1, in);

    int width = fi->biWidth, height = fi->biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    // 初始化新的文件头
    fileHeader newFh;
    fileInfo newFi;
    
    // 复制文件头信息
    memcpy(&newFh, fh, sizeof(fileHeader));
    memcpy(&newFi, fi, sizeof(fileInfo));
    
    // 设置新的参数
    newFi.biBitCount = 8;
    newFi.biCompression = 0;
    newFi.biSizeImage = ((width + 3) / 4) * 4 * height;
    newFi.biPlanes = 1;
    newFi.biClrUsed = 256;
    newFi.biClrImportant = 256;
    
    newFh.bfOffBits = sizeof(fileHeader) + sizeof(fileInfo) + 256 * sizeof(rgbq);
    newFh.bfSize = newFh.bfOffBits + newFi.biSizeImage;

    rgbq *palette = (rgbq *)malloc(256 * sizeof(rgbq));
#pragma omp parallel for
    for (int i = 0; i < 256; i++)
    {
        palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = i;
        palette[i].rgbReserved = 0;
    }

    fwrite(&newFh, sizeof(fileHeader), 1, out);
    fwrite(&newFi, sizeof(fileInfo), 1, out);
    fwrite(palette, sizeof(rgbq), 256, out);

    // 分配内存并读取整个图像
    unsigned char *rgbData = (unsigned char *)malloc(rowSize * height);
    unsigned char *binData = (unsigned char *)malloc(((width + 3) / 4) * 4 * height);

    // 读取所有行
    bool bottom_up = (fi->biHeight > 0);
    int actual_height = abs(fi->biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(rgbData + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(rgbData + i * rowSize, 1, rowSize, in);
        }
    }

    // 并行处理像素转换
#pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int b = rgbData[i * rowSize + j * 3];
            int g = rgbData[i * rowSize + j * 3 + 1];
            int r = rgbData[i * rowSize + j * 3 + 2];
            int gray = (r + g + b) / 3;
            binData[i * ((width + 3) / 4) * 4 + j] = (gray >= threshold) ? 255 : 0;
        }
    }

    // 写入输出文件
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(binData + i * ((width + 3) / 4) * 4, 1, ((width + 3) / 4) * 4, out);
    }

    free(fh);
    free(fi);
    free(palette);
    free(rgbData);
    free(binData);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

double adjust_brightness_py(const std::string &input, const std::string &output, int delta)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像亮度调整");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    // 分配内存并读取整个图像
    unsigned char *imageData = (unsigned char *)malloc(rowSize * height);
    
    // 读取所有行
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(imageData + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(imageData + i * rowSize, 1, rowSize, in);
        }
    }

    // 并行处理图像数据
#pragma omp parallel for collapse(2)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char *pixel = imageData + i * rowSize + j * 3;
            pixel[0] = clamp(pixel[0] + delta);
            pixel[1] = clamp(pixel[1] + delta);
            pixel[2] = clamp(pixel[2] + delta);
        }
    }

    // 写入输出文件
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(imageData + i * rowSize, 1, rowSize, out);
    }

    free(imageData);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

void generate_gaussian_kernel(float **kernel, int size, float sigma)
{
    int half = size / 2;
    float sum = 0.0f;

#pragma omp parallel for reduction(+ : sum) collapse(2)
    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            float value = expf(-(i * i + j * j) / (2 * sigma * sigma));
            kernel[i + half][j + half] = value;
            sum += value;
        }
    }

#pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            kernel[i][j] /= sum;
        }
    }
}

double apply_gaussian_blur_py(const std::string &input, const std::string &output, int kernel_size, float sigma)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行高斯模糊");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }

    // 动态分配卷积核内存
    float **kernel = (float **)malloc(kernel_size * sizeof(float *));
    for (int i = 0; i < kernel_size; i++) {
        kernel[i] = (float *)malloc(kernel_size * sizeof(float));
    }
    
    generate_gaussian_kernel(kernel, kernel_size, sigma);
    int half = kernel_size / 2;

#pragma omp parallel for
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float sumB = 0, sumG = 0, sumR = 0;
            for (int i = -half; i <= half; i++)
            {
                for (int j = -half; j <= half; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    if (nx < 0)
                        nx = 0;
                    if (ny < 0)
                        ny = 0;
                    if (nx >= width)
                        nx = width - 1;
                    if (ny >= height)
                        ny = height - 1;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    float weight = kernel[i + half][j + half];
                    sumB += p[0] * weight;
                    sumG += p[1] * weight;
                    sumR += p[2] * weight;
                }
            }
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = clamp((int)(sumB + 0.5));
            outPix[1] = clamp((int)(sumG + 0.5));
            outPix[2] = clamp((int)(sumR + 0.5));
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }

    // 释放卷积核内存
    for (int i = 0; i < kernel_size; i++) {
        free(kernel[i]);
    }
    free(kernel);
    
    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

double apply_custom_convolution_py(const std::string &input, const std::string &output,
                                 const std::vector<std::vector<int>> &kernel_vec, float divisor)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行卷积操作");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }
    memset(dst, 0, rowSize * height);

    // 将vector转换为数组
    int kernel[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            kernel[i][j] = kernel_vec[i][j];
        }
    }

#pragma omp parallel for collapse(2)
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int sumR = 0, sumG = 0, sumB = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    sumB += p[0] * kernel[i + 1][j + 1];
                    sumG += p[1] * kernel[i + 1][j + 1];
                    sumR += p[2] * kernel[i + 1][j + 1];
                }
            }
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = clamp(sumB / divisor);
            outPix[1] = clamp(sumG / divisor);
            outPix[2] = clamp(sumR / divisor);
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }
    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

double apply_sobel_edge_detection_py(const std::string &input, const std::string &output)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行Sobel边缘检测");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }
    memset(dst, 0, rowSize * height);

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    int Gy[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

#pragma omp parallel for collapse(2)
    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gx = 0, gy = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    int gray = (p[0] + p[1] + p[2]) / 3;
                    gx += gray * Gx[i + 1][j + 1];
                    gy += gray * Gy[i + 1][j + 1];
                }
            }
            int magnitude = (int)sqrt(gx * gx + gy * gy);
            unsigned char edge = clamp(magnitude);
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = outPix[1] = outPix[2] = edge;
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }

    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 拼接优化相关结构体与全局变量
typedef struct
{
    cv::Point2f left_top;
    cv::Point2f left_bottom;
    cv::Point2f right_top;
    cv::Point2f right_bottom;
} four_corners_t;

four_corners_t corners;

void OptimizeSeam(cv::Mat &img1, cv::Mat &trans, cv::Mat &dst)
{
    // 计算重叠区域的起始位置
    int start = std::max(0, (int)std::min(corners.left_top.x, corners.left_bottom.x));
    int end = std::min(dst.cols, (int)std::max(corners.right_top.x, corners.right_bottom.x));
    
    if (start >= end) {
        return; // 没有重叠区域
    }
    
    double processWidth = end - start;
    int rows = dst.rows;
    double alpha = 1;
    
    for (int i = 0; i < rows; i++)
    {
        for (int j = start; j < end; j++)
        {
            if (j >= dst.cols) break;
            
            // 检查transformed图像中是否有有效像素
            bool has_trans_pixel = false;
            if (j < trans.cols && i < trans.rows) {
                uchar *t = trans.ptr<uchar>(i);
                has_trans_pixel = (t[j * 3] != 0 || t[j * 3 + 1] != 0 || t[j * 3 + 2] != 0);
            }
            
            if (!has_trans_pixel)
            {
                alpha = 1;
            }
            else
            {
                alpha = (processWidth - (j - start)) / processWidth;
                alpha = std::max(0.0, std::min(1.0, alpha)); // 限制在[0,1]范围内
            }
            
            uchar *d = dst.ptr<uchar>(i);
            if (j < img1.cols && i < img1.rows) {
                uchar *p = img1.ptr<uchar>(i);
                d[j * 3] = p[j * 3] * alpha + (has_trans_pixel ? trans.ptr<uchar>(i)[j * 3] * (1 - alpha) : 0);
                d[j * 3 + 1] = p[j * 3 + 1] * alpha + (has_trans_pixel ? trans.ptr<uchar>(i)[j * 3 + 1] * (1 - alpha) : 0);
                d[j * 3 + 2] = p[j * 3 + 2] * alpha + (has_trans_pixel ? trans.ptr<uchar>(i)[j * 3 + 2] * (1 - alpha) : 0);
            }
        }
    }
}

void CalcCorners(const cv::Mat &H, const cv::Mat &src)
{
    double v2[] = {0, 0, 1};
    double v1[3];
    cv::Mat V2 = cv::Mat(3, 1, CV_64FC1, v2);
    cv::Mat V1 = cv::Mat(3, 1, CV_64FC1, v1);
    V1 = H * V2;
    corners.left_top.x = v1[0] / v1[2];
    corners.left_top.y = v1[1] / v1[2];

    v2[0] = 0;
    v2[1] = src.rows;
    V1 = H * cv::Mat(3, 1, CV_64FC1, v2);
    corners.left_bottom.x = v1[0] / v1[2];
    corners.left_bottom.y = v1[1] / v1[2];

    v2[0] = src.cols;
    v2[1] = 0;
    V1 = H * cv::Mat(3, 1, CV_64FC1, v2);
    corners.right_top.x = v1[0] / v1[2];
    corners.right_top.y = v1[1] / v1[2];

    v2[0] = src.cols;
    v2[1] = src.rows;
    V1 = H * cv::Mat(3, 1, CV_64FC1, v2);
    corners.right_bottom.x = v1[0] / v1[2];
    corners.right_bottom.y = v1[1] / v1[2];
}

double stitch_images_surf_py(const std::string &input1, const std::string &input2, const std::string &output)
{
    cv::Mat image01 = cv::imread(input1);
    cv::Mat image02 = cv::imread(input2);
    if (image01.empty() || image02.empty())
    {
        throw std::runtime_error("无法读取输入图像");
    }

    double start_time = omp_get_wtime();

    cv::Mat image1, image2;
    cv::cvtColor(image01, image1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(image02, image2, cv::COLOR_BGR2GRAY);

    cv::Ptr<cv::ORB> detector = cv::ORB::create(1000); // 增加特征点数量
    std::vector<cv::KeyPoint> keypoints1, keypoints2;
    cv::Mat descriptors1, descriptors2;
    detector->detectAndCompute(image1, cv::noArray(), keypoints1, descriptors1);
    detector->detectAndCompute(image2, cv::noArray(), keypoints2, descriptors2);

    // 检查是否检测到足够的特征点
    if (keypoints1.size() < 10 || keypoints2.size() < 10) {
        throw std::runtime_error("检测到的特征点数量不足（图像1: " + std::to_string(keypoints1.size()) + 
                                ", 图像2: " + std::to_string(keypoints2.size()) + "），无法进行匹配");
    }

    // 使用BFMatcher替代FlannBasedMatcher，避免FLANN兼容性问题
    cv::BFMatcher matcher(cv::NORM_HAMMING);
    std::vector<std::vector<cv::DMatch>> matches;
    std::vector<cv::DMatch> goodMatches;
    matcher.knnMatch(descriptors1, descriptors2, matches, 2);

    for (size_t i = 0; i < matches.size(); i++)
    {
        if (matches[i][0].distance < 0.6 * matches[i][1].distance) // 放宽匹配条件
            goodMatches.push_back(matches[i][0]);
    }

    // 检查是否有足够的良好匹配
    if (goodMatches.size() < 4) {
        // 如果还是不够，尝试更宽松的条件
        goodMatches.clear();
        for (size_t i = 0; i < matches.size(); i++)
        {
            if (matches[i][0].distance < 0.8 * matches[i][1].distance)
                goodMatches.push_back(matches[i][0]);
        }
        
        if (goodMatches.size() < 4) {
            throw std::runtime_error("良好匹配的特征点数量不足（需要至少4个，当前只有" + std::to_string(goodMatches.size()) + "个）");
        }
    }

    std::vector<cv::Point2f> pts1, pts2;
    for (auto &m : goodMatches)
    {
        pts1.push_back(keypoints1[m.queryIdx].pt);
        pts2.push_back(keypoints2[m.trainIdx].pt);
    }

    cv::Mat H = cv::findHomography(pts1, pts2, cv::RANSAC);
    if (H.empty()) {
        // 如果无法计算单应性矩阵，尝试使用简单的平移拼接
        std::cout << "警告：无法计算单应性矩阵，使用简单平移拼接" << std::endl;
        
        // 计算简单的平移
        cv::Point2f center1(0, 0), center2(0, 0);
        for (const auto& pt : pts1) center1 += pt;
        for (const auto& pt : pts2) center2 += pt;
        center1 *= 1.0f / pts1.size();
        center2 *= 1.0f / pts2.size();
        
        cv::Point2f translation = center2 - center1;
        
        // 创建简单的平移矩阵
        H = cv::Mat::eye(3, 3, CV_64F);
        H.at<double>(0, 2) = translation.x;
        H.at<double>(1, 2) = translation.y;
    }
    
    CalcCorners(H, image01);
    
    // 计算变换后图像的边界
    float min_x = std::min({corners.left_top.x, corners.left_bottom.x, 0.0f});
    float max_x = std::max({corners.right_top.x, corners.right_bottom.x, (float)image02.cols});
    float min_y = std::min({corners.left_top.y, corners.right_top.y, 0.0f});
    float max_y = std::max({corners.left_bottom.y, corners.right_bottom.y, (float)image02.rows});
    
    // 确保边界为正数
    int dst_width = std::max(1, (int)(max_x - min_x));
    int dst_height = std::max(1, (int)(max_y - min_y));
    
    // 创建变换矩阵，包含平移
    cv::Mat H_translated = cv::Mat::eye(3, 3, CV_64F);
    H_translated.at<double>(0, 2) = -min_x;
    H_translated.at<double>(1, 2) = -min_y;
    cv::Mat H_final = H_translated * H;
    
    cv::Mat imageTransform1;
    cv::warpPerspective(image01, imageTransform1, H_final, cv::Size(dst_width, dst_height));
    
    cv::Mat dst(dst_height, dst_width, CV_8UC3);
    dst.setTo(0);
    
    // 先复制变换后的第一张图像
    cv::Rect roi1(0, 0, std::min(imageTransform1.cols, dst.cols), std::min(imageTransform1.rows, dst.rows));
    imageTransform1(roi1).copyTo(dst(roi1));
    
    // 计算第二张图像在dst中的位置
    int x_offset = (int)(-min_x);
    int y_offset = (int)(-min_y);
    
    // 确保偏移量在合理范围内
    x_offset = std::max(0, std::min(x_offset, dst.cols - 1));
    y_offset = std::max(0, std::min(y_offset, dst.rows - 1));
    
    // 计算第二张图像的有效区域
    int img2_width = std::min(image02.cols, dst.cols - x_offset);
    int img2_height = std::min(image02.rows, dst.rows - y_offset);
    
    if (img2_width > 0 && img2_height > 0) {
        cv::Rect roi2(x_offset, y_offset, img2_width, img2_height);
        cv::Rect roi2_src(0, 0, img2_width, img2_height);
        image02(roi2_src).copyTo(dst(roi2));
    }
    
    // 更新角点坐标以匹配新的坐标系
    corners.left_top.x -= min_x;
    corners.left_top.y -= min_y;
    corners.left_bottom.x -= min_x;
    corners.left_bottom.y -= min_y;
    corners.right_top.x -= min_x;
    corners.right_top.y -= min_y;
    corners.right_bottom.x -= min_x;
    corners.right_bottom.y -= min_y;
    
    // 改进的混合逻辑：检查重叠区域并正确混合
    for (int y = 0; y < dst.rows; y++) {
        for (int x = 0; x < dst.cols; x++) {
            cv::Vec3b& pixel = dst.at<cv::Vec3b>(y, x);
            
            // 检查是否在第二张图像的有效区域内
            bool in_img2 = (x >= x_offset && x < x_offset + image02.cols && 
                           y >= y_offset && y < y_offset + image02.rows);
            
            // 检查是否在变换后的第一张图像的有效区域内
            bool in_img1 = false;
            if (x < imageTransform1.cols && y < imageTransform1.rows) {
                cv::Vec3b trans_pixel = imageTransform1.at<cv::Vec3b>(y, x);
                in_img1 = (trans_pixel[0] != 0 || trans_pixel[1] != 0 || trans_pixel[2] != 0);
            }
            
            // 如果在重叠区域，进行混合
            if (in_img1 && in_img2) {
                cv::Vec3b pixel1 = imageTransform1.at<cv::Vec3b>(y, x);
                cv::Vec3b pixel2 = image02.at<cv::Vec3b>(y - y_offset, x - x_offset);
                
                // 使用距离权重进行混合
                float dist_from_left = (float)(x - x_offset) / image02.cols;
                float weight = std::max(0.0f, std::min(1.0f, dist_from_left));
                
                pixel[0] = (uchar)(pixel1[0] * (1 - weight) + pixel2[0] * weight);
                pixel[1] = (uchar)(pixel1[1] * (1 - weight) + pixel2[1] * weight);
                pixel[2] = (uchar)(pixel1[2] * (1 - weight) + pixel2[2] * weight);
            }
            // 如果只在第一张图像中
            else if (in_img1) {
                pixel = imageTransform1.at<cv::Vec3b>(y, x);
            }
            // 如果只在第二张图像中
            else if (in_img2) {
                pixel = image02.at<cv::Vec3b>(y - y_offset, x - x_offset);
            }
        }
    }
    cv::imwrite(output, dst);

    double end_time = omp_get_wtime();
    return end_time - start_time;
}

// 串行版本的灰度转换函数
double convert_to_grayscale_serial_py(const std::string &input, const std::string &output)
{
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader *fh = (fileHeader *)malloc(sizeof(fileHeader));
    fileInfo *fi = (fileInfo *)malloc(sizeof(fileInfo));
    fread(fh, sizeof(fileHeader), 1, in);
    fread(fi, sizeof(fileInfo), 1, in);

    double start_time, end_time;
    start_time = omp_get_wtime();
    int width = fi->biWidth, height = fi->biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    // 初始化新的文件头
    fileHeader newFh;
    fileInfo newFi;
    
    // 复制文件头信息
    memcpy(&newFh, fh, sizeof(fileHeader));
    memcpy(&newFi, fi, sizeof(fileInfo));
    
    // 设置新的参数
    newFi.biBitCount = 8;
    newFi.biCompression = 0;
    newFi.biSizeImage = ((width + 3) / 4) * 4 * height;
    newFi.biPlanes = 1;
    newFi.biClrUsed = 256;
    newFi.biClrImportant = 256;
    
    newFh.bfOffBits = sizeof(fileHeader) + sizeof(fileInfo) + 256 * sizeof(rgbq);
    newFh.bfSize = newFh.bfOffBits + newFi.biSizeImage;

    rgbq *palette = (rgbq *)malloc(256 * sizeof(rgbq));
    for (int i = 0; i < 256; i++)
    {
        palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = i;
        palette[i].rgbReserved = 0;
    }

    fwrite(&newFh, sizeof(fileHeader), 1, out);
    fwrite(&newFi, sizeof(fileInfo), 1, out);
    fwrite(palette, sizeof(rgbq), 256, out);

    unsigned char *rgbRow = (unsigned char *)malloc(rowSize);
    unsigned char *grayRow = (unsigned char *)calloc(((width + 3) / 4) * 4, 1);
    unsigned char *grayData = (unsigned char *)malloc(((width + 3) / 4) * 4 * height);

    // 读取所有行并转换为灰度
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi->biHeight > 0);
    int actual_height = abs(fi->biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(rgbRow, 1, rowSize, in);
            for (int j = 0; j < width; j++)
            {
                int b = rgbRow[j * 3];
                int g = rgbRow[j * 3 + 1];
                int r = rgbRow[j * 3 + 2];
                grayData[i * ((width + 3) / 4) * 4 + j] = (r + g + b) / 3;
            }
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(rgbRow, 1, rowSize, in);
            for (int j = 0; j < width; j++)
            {
                int b = rgbRow[j * 3];
                int g = rgbRow[j * 3 + 1];
                int r = rgbRow[j * 3 + 2];
                grayData[i * ((width + 3) / 4) * 4 + j] = (r + g + b) / 3;
            }
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        memcpy(grayRow, grayData + i * ((width + 3) / 4) * 4, ((width + 3) / 4) * 4);
        fwrite(grayRow, 1, ((width + 3) / 4) * 4, out);
    }
    end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;

    free(fh);
    free(fi);
    free(palette);
    free(rgbRow);
    free(grayRow);
    free(grayData);
    fclose(in);
    fclose(out);

    return elapsed_time;
}

// 串行版本的二值化函数
double convert_to_binary_serial_py(const std::string &input, const std::string &output, int threshold)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader *fh = (fileHeader *)malloc(sizeof(fileHeader));
    fileInfo *fi = (fileInfo *)malloc(sizeof(fileInfo));
    fread(fh, sizeof(fileHeader), 1, in);
    fread(fi, sizeof(fileInfo), 1, in);

    int width = fi->biWidth, height = fi->biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    // 初始化新的文件头
    fileHeader newFh;
    fileInfo newFi;
    
    // 复制文件头信息
    memcpy(&newFh, fh, sizeof(fileHeader));
    memcpy(&newFi, fi, sizeof(fileInfo));
    
    // 设置新的参数
    newFi.biBitCount = 8;
    newFi.biCompression = 0;
    newFi.biSizeImage = ((width + 3) / 4) * 4 * height;
    newFi.biPlanes = 1;
    newFi.biClrUsed = 256;
    newFi.biClrImportant = 256;
    
    newFh.bfOffBits = sizeof(fileHeader) + sizeof(fileInfo) + 256 * sizeof(rgbq);
    newFh.bfSize = newFh.bfOffBits + newFi.biSizeImage;

    rgbq *palette = (rgbq *)malloc(256 * sizeof(rgbq));
    for (int i = 0; i < 256; i++)
    {
        palette[i].rgbRed = palette[i].rgbGreen = palette[i].rgbBlue = i;
        palette[i].rgbReserved = 0;
    }

    fwrite(&newFh, sizeof(fileHeader), 1, out);
    fwrite(&newFi, sizeof(fileInfo), 1, out);
    fwrite(palette, sizeof(rgbq), 256, out);

    unsigned char *rgbRow = (unsigned char *)malloc(rowSize);
    unsigned char *binRow = (unsigned char *)calloc(((width + 3) / 4) * 4, 1);
    unsigned char *binData = (unsigned char *)malloc(((width + 3) / 4) * 4 * height);

    // 读取所有行并转换为二值图
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi->biHeight > 0);
    int actual_height = abs(fi->biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(rgbRow, 1, rowSize, in);
            for (int j = 0; j < width; j++)
            {
                int b = rgbRow[j * 3];
                int g = rgbRow[j * 3 + 1];
                int r = rgbRow[j * 3 + 2];
                int gray = (r + g + b) / 3;
                binData[i * ((width + 3) / 4) * 4 + j] = (gray >= threshold) ? 255 : 0;
            }
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(rgbRow, 1, rowSize, in);
            for (int j = 0; j < width; j++)
            {
                int b = rgbRow[j * 3];
                int g = rgbRow[j * 3 + 1];
                int r = rgbRow[j * 3 + 2];
                int gray = (r + g + b) / 3;
                binData[i * ((width + 3) / 4) * 4 + j] = (gray >= threshold) ? 255 : 0;
            }
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        memcpy(binRow, binData + i * ((width + 3) / 4) * 4, ((width + 3) / 4) * 4);
        fwrite(binRow, 1, ((width + 3) / 4) * 4, out);
    }

    free(fh);
    free(fi);
    free(palette);
    free(rgbRow);
    free(binRow);
    free(binData);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 串行版本的亮度调整函数
double adjust_brightness_serial_py(const std::string &input, const std::string &output, int delta)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像亮度调整");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *row = (unsigned char *)malloc(rowSize);
    unsigned char *imageData = (unsigned char *)malloc(rowSize * height);

    // 读取所有行
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(row, 1, rowSize, in);
            memcpy(imageData + i * rowSize, row, rowSize);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(row, 1, rowSize, in);
            memcpy(imageData + i * rowSize, row, rowSize);
        }
    }

    // 处理图像数据
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            unsigned char *pixel = imageData + i * rowSize + j * 3;
            pixel[0] = clamp(pixel[0] + delta);
            pixel[1] = clamp(pixel[1] + delta);
            pixel[2] = clamp(pixel[2] + delta);
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        memcpy(row, imageData + i * rowSize, rowSize);
        fwrite(row, 1, rowSize, out);
    }

    free(row);
    free(imageData);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 串行版本的高斯核生成函数
void generate_gaussian_kernel_serial(float **kernel, int size, float sigma)
{
    int half = size / 2;
    float sum = 0.0f;

    for (int i = -half; i <= half; i++)
    {
        for (int j = -half; j <= half; j++)
        {
            float value = expf(-(i * i + j * j) / (2 * sigma * sigma));
            kernel[i + half][j + half] = value;
            sum += value;
        }
    }

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            kernel[i][j] /= sum;
        }
    }
}

// 串行版本的高斯模糊函数
double apply_gaussian_blur_serial_py(const std::string &input, const std::string &output, int kernel_size, float sigma)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行高斯模糊");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }

    // 动态分配卷积核内存
    float **kernel = (float **)malloc(kernel_size * sizeof(float *));
    for (int i = 0; i < kernel_size; i++) {
        kernel[i] = (float *)malloc(kernel_size * sizeof(float));
    }
    
    generate_gaussian_kernel_serial(kernel, kernel_size, sigma);
    int half = kernel_size / 2;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float sumB = 0, sumG = 0, sumR = 0;
            for (int i = -half; i <= half; i++)
            {
                for (int j = -half; j <= half; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    if (nx < 0)
                        nx = 0;
                    if (ny < 0)
                        ny = 0;
                    if (nx >= width)
                        nx = width - 1;
                    if (ny >= height)
                        ny = height - 1;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    float weight = kernel[i + half][j + half];
                    sumB += p[0] * weight;
                    sumG += p[1] * weight;
                    sumR += p[2] * weight;
                }
            }
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = clamp((int)(sumB + 0.5));
            outPix[1] = clamp((int)(sumG + 0.5));
            outPix[2] = clamp((int)(sumR + 0.5));
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }

    // 释放卷积核内存
    for (int i = 0; i < kernel_size; i++) {
        free(kernel[i]);
    }
    free(kernel);
    
    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 串行版本的自定义卷积函数
double apply_custom_convolution_serial_py(const std::string &input, const std::string &output,
                                         const std::vector<std::vector<int>> &kernel_vec, float divisor)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行卷积操作");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }
    memset(dst, 0, rowSize * height);

    // 将vector转换为数组
    int kernel[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            kernel[i][j] = kernel_vec[i][j];
        }
    }

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int sumR = 0, sumG = 0, sumB = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    sumB += p[0] * kernel[i + 1][j + 1];
                    sumG += p[1] * kernel[i + 1][j + 1];
                    sumR += p[2] * kernel[i + 1][j + 1];
                }
            }
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = clamp(sumB / divisor);
            outPix[1] = clamp(sumG / divisor);
            outPix[2] = clamp(sumR / divisor);
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }
    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 串行版本的Sobel边缘检测函数
double apply_sobel_edge_detection_serial_py(const std::string &input, const std::string &output)
{
    double start_time, end_time;
    start_time = omp_get_wtime();
    
    FILE *in = NULL, *out = NULL;
    in = fopen(input.c_str(), "rb");
    if (!in)
    {
        throw std::runtime_error("无法打开输入文件: " + input);
    }
    out = fopen(output.c_str(), "wb");
    if (!out)
    {
        fclose(in);
        throw std::runtime_error("无法创建输出文件: " + output);
    }

    fileHeader fh;
    fileInfo fi;
    fread(&fh, sizeof(fileHeader), 1, in);
    fread(&fi, sizeof(fileInfo), 1, in);

    if (fi.biBitCount != 24)
    {
        fclose(in);
        fclose(out);
        throw std::runtime_error("仅支持24位RGB图像进行Sobel边缘检测");
    }

    int width = fi.biWidth, height = fi.biHeight;
    int rowSize = ((width * 3 + 3) / 4) * 4;

    fwrite(&fh, sizeof(fileHeader), 1, out);
    fwrite(&fi, sizeof(fileInfo), 1, out);

    unsigned char *src = (unsigned char *)malloc(rowSize * height);
    unsigned char *dst = (unsigned char *)malloc(rowSize * height);
    
    // BMP格式的图像数据可能是从下到上存储的，需要检查biHeight的符号
    bool bottom_up = (fi.biHeight > 0);
    int actual_height = abs(fi.biHeight);
    
    if (bottom_up) {
        // 图像数据从下到上存储，需要从后往前读取
        for (int i = actual_height - 1; i >= 0; i--)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    } else {
        // 图像数据从上到下存储，正常读取
        for (int i = 0; i < actual_height; i++)
        {
            fread(src + i * rowSize, 1, rowSize, in);
        }
    }
    memset(dst, 0, rowSize * height);

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    int Gy[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int gx = 0, gy = 0;
            for (int i = -1; i <= 1; i++)
            {
                for (int j = -1; j <= 1; j++)
                {
                    int ny = y + i;
                    int nx = x + j;
                    unsigned char *p = src + ny * rowSize + nx * 3;
                    int gray = (p[0] + p[1] + p[2]) / 3;
                    gx += gray * Gx[i + 1][j + 1];
                    gy += gray * Gy[i + 1][j + 1];
                }
            }
            int magnitude = (int)sqrt(gx * gx + gy * gy);
            unsigned char edge = clamp(magnitude);
            unsigned char *outPix = dst + y * rowSize + x * 3;
            outPix[0] = outPix[1] = outPix[2] = edge;
        }
    }

    // BMP文件要求图像数据从下到上存储
    for (int i = height - 1; i >= 0; i--)
    {
        fwrite(dst + i * rowSize, 1, rowSize, out);
    }

    free(src);
    free(dst);
    fclose(in);
    fclose(out);
    
    end_time = omp_get_wtime();
    return end_time - start_time;
}

// 全局变量存储当前设置的线程数
static int current_omp_threads = omp_get_max_threads();

// 设置OpenMP线程数的函数
void set_omp_threads(int num_threads)
{
    omp_set_num_threads(num_threads);
    current_omp_threads = num_threads;
}

// 获取OpenMP线程数的函数
int get_omp_threads()
{
    return current_omp_threads;
}

// pybind11模块定义
PYBIND11_MODULE(image_processing, m)
{
    m.doc() = "OpenMP加速的图像处理模块"; // 模块文档字符串

    // 设置OpenMP线程数
    m.def("set_omp_threads", &set_omp_threads, "设置OpenMP线程数",
          py::arg("num_threads"));

    // 获取OpenMP线程数
    m.def("get_omp_threads", &get_omp_threads, "获取OpenMP最大线程数");

    // RGB转灰度图
    m.def("convert_to_grayscale", &convert_to_grayscale_py,
          "将RGB图像转换为灰度图",
          py::arg("input"), py::arg("output"));

    // RGB转二值图
    m.def("convert_to_binary", &convert_to_binary_py,
          "将RGB图像转换为二值图",
          py::arg("input"), py::arg("output"), py::arg("threshold"));

    // 亮度调整
    m.def("adjust_brightness", &adjust_brightness_py,
          "调整图像亮度",
          py::arg("input"), py::arg("output"), py::arg("delta"));

    // 高斯模糊
    m.def("apply_gaussian_blur", &apply_gaussian_blur_py,
          "应用高斯模糊",
          py::arg("input"), py::arg("output"), py::arg("kernel_size"), py::arg("sigma"));

    // 自定义卷积
    m.def("apply_custom_convolution", &apply_custom_convolution_py,
          "应用自定义卷积滤波器",
          py::arg("input"), py::arg("output"), py::arg("kernel"), py::arg("divisor"));

    // Sobel边缘检测
    m.def("apply_sobel_edge_detection", &apply_sobel_edge_detection_py,
          "应用Sobel边缘检测",
          py::arg("input"), py::arg("output"));

    // 图像拼接
    m.def("stitch_images_surf", &stitch_images_surf_py,
          "使用SURF特征进行图像拼接",
          py::arg("input1"), py::arg("input2"), py::arg("output"));

    // 串行版本的函数绑定
    // 串行版本RGB转灰度图
    m.def("convert_to_grayscale_serial", &convert_to_grayscale_serial_py,
          "将RGB图像转换为灰度图（串行版本）",
          py::arg("input"), py::arg("output"));

    // 串行版本RGB转二值图
    m.def("convert_to_binary_serial", &convert_to_binary_serial_py,
          "将RGB图像转换为二值图（串行版本）",
          py::arg("input"), py::arg("output"), py::arg("threshold"));

    // 串行版本亮度调整
    m.def("adjust_brightness_serial", &adjust_brightness_serial_py,
          "调整图像亮度（串行版本）",
          py::arg("input"), py::arg("output"), py::arg("delta"));

    // 串行版本高斯模糊
    m.def("apply_gaussian_blur_serial", &apply_gaussian_blur_serial_py,
          "应用高斯模糊（串行版本）",
          py::arg("input"), py::arg("output"), py::arg("kernel_size"), py::arg("sigma"));

    // 串行版本自定义卷积
    m.def("apply_custom_convolution_serial", &apply_custom_convolution_serial_py,
          "应用自定义卷积滤波器（串行版本）",
          py::arg("input"), py::arg("output"), py::arg("kernel"), py::arg("divisor"));

    // 串行版本Sobel边缘检测
    m.def("apply_sobel_edge_detection_serial", &apply_sobel_edge_detection_serial_py,
          "应用Sobel边缘检测（串行版本）",
          py::arg("input"), py::arg("output"));
}