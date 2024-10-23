#ifndef __BMP_H__
#define __BMP_H__

// #pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

extern const char* OUTPUT_FILE;

#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pack(pop)

extern int rows;
extern int cols;

typedef struct Pixel
{
    unsigned char red, green, blue;
} Pixel;

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize);
void getPixelsFromBMP24(int end, char *fileReadBuffer, vector<vector<Pixel>> &picture);
void writeOutBmp24(char *fileBuffer, int bufferSize, vector<vector<Pixel>> picture);

#endif