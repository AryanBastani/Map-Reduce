#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bmp.hpp"

const char * OUTPUT_FILE = "out.bmp";
int rows;
int cols;

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
    auto startTime = chrono::high_resolution_clock::now();
    std::ifstream file(fileName);
    if (!file)
    {
        std::cout << "File" << fileName << " doesn't exist!" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Fill and Allocate Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;
    return true;
}

void getPixelsFromBMP24(int end, char *fileReadBuffer, vector<vector<Pixel>> &picture)
{
    int count = 1;
    int extra = cols % 4;
    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    picture[i][j].red = (unsigned char)fileReadBuffer[end - count];
                    break;
                case 1:
                    picture[i][j].green = (unsigned char)fileReadBuffer[end - count];
                    break;
                case 2:
                    picture[i][j].blue = (unsigned char)fileReadBuffer[end - count];
                    break;
                }
                count++;
            }
        }
    }
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Read Buffer Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;
}

void writeOutBmp24(char *fileBuffer, int bufferSize, vector<vector<Pixel>> picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    std::ofstream write(OUTPUT_FILE);
    if (!write)
    {
        std::cout << "Failed to write " << OUTPUT_FILE<< std::endl;
        return;
    }

    int count = 1;
    int extra = cols % 4;
    for (int i = 0; i < rows; i++)
    {
        count += extra;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    fileBuffer[bufferSize - count] = (char)(picture[i][j].red);
                    break;
                case 1:
                    fileBuffer[bufferSize - count] = (char)(picture[i][j].green);
                    break;
                case 2:
                    fileBuffer[bufferSize - count] = (char)(picture[i][j].blue);
                    break;
                }
                count++;
            }
        }
    }
    write.write(fileBuffer, bufferSize);
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Write to buffer Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;
}
