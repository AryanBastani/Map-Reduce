#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bmp.hpp"

const char *OUTPUT_FILE = "out.bmp";
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

void *threadreadRows(void *args)
{
    ReadWriteArgs *threadArgs = static_cast<ReadWriteArgs *>(args);
    char *fileBuffer = threadArgs->fileBuffer;
    int count = threadArgs->count;
    int end = threadArgs->bufferSize;
    int offset = threadArgs->offset;
    vector<vector<Pixel>> *picture = threadArgs->picture;
    for (int i = threadArgs->rowsStart; i < threadArgs->rowsEnd; i++)
    {
        count += offset;
        for (int j = cols - 1; j >= 0; j--)
        {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    (*picture)[i][j].red = (unsigned char)fileBuffer[end - count];
                    break;
                case 1:
                    (*picture)[i][j].green = (unsigned char)fileBuffer[end - count];
                    break;
                case 2:
                    (*picture)[i][j].blue = (unsigned char)fileBuffer[end - count];
                    break;
                }
                count++;
            }
        }
    }
    return nullptr;
}

void getPixelsFromBMP24(int bufferSize, char *fileReadBuffer, vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    int count = 1;
    int threadCount = THREAD_COUNT;
    int offset = cols % 4;
    ReadWriteArgs args[threadCount];
    pthread_t threads[threadCount];
    int rowsPerThread = rows / threadCount;
    int remainingRows = rows % threadCount;
    for (int i = 0; i < threadCount; i++)
    {
        int start = i * rowsPerThread;
        int end = (i + 1) * rowsPerThread;
        end = (i == threadCount - 1) ? end + remainingRows : end;
        count = (i * rowsPerThread) * (cols * 3 + offset) + 1;
        args[i].cols_ = cols;
        args[i].rowsStart = start;
        args[i].rowsEnd = end;
        args[i].count = count;
        args[i].offset = offset;
        args[i].bufferSize = bufferSize;
        args[i].fileBuffer = fileReadBuffer;
        args[i].picture = &picture;
        if(pthread_create(&threads[i], nullptr, threadreadRows, &args[i])) 
        {
            cerr << "Unable to create thread: " << i << endl;
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < threadCount; i++)
    {
        pthread_join(threads[i], nullptr);
    }
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Parallel Read buffer Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

}

void *threadWriteToRows(void *args)
{
    ReadWriteArg *threadArgs = static_cast<ReadWriteArgs *>(args);
    char *fileBuffer = threadArgs->fileBuffer;
    int bufferSize = threadArgs->bufferSize;
    int count = threadArgs->count;
    int offset = threadArgs->offset;
    vector<vector<Pixel>> *picture = threadArgs->picture;
    for (int i = threadArgs->rowsStart; i < threadArgs->rowsEnd; i++)
    {
        count += offset;
        for (int j = cols - 1; j >= 0; j--) {
            for (int k = 0; k < 3; k++)
            {
                switch (k)
                {
                case 0:
                    fileBuffer[bufferSize - count] = (char)((*picture)[i][j].red);
                    break;
                case 1:
                    fileBuffer[bufferSize - count] = (char)((*picture)[i][j].green);
                    break;
                case 2:
                    fileBuffer[bufferSize - count] = (char)((*picture)[i][j].blue);
                    break;
                }
                count++;
            }
        }
    }
    return nullptr;
}

void writeOutBmp24(char *fileBuffer, int bufferSize, vector<vector<Pixel>> picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    std::ofstream write(OUTPUT_FILE);
    if (!write)
    {
        std::cout << "Failed to write " << OUTPUT_FILE << std::endl;
        return;
    }

    int threadsCount = THREAD_COUNT;
    pthread_t threads[threadsCount];
    ReadWriteArgs args[threadsCount];
    int rowsPerThread = rows / threadsCount;
    int remainingRows = rows % threadsCount;
    int count = 1;
    int offset = cols % 4;
    for(int i = 0; i < threadsCount; i++)
    {
        int start = i * rowsPerThread;
        int end = (i + 1) * rowsPerThread;
        end = (i == threadsCount - 1) ? end + remainingRows : end;
        count = (i * rowsPerThread) * (cols * 3 + offset) + 1;
        args[i].cols_ = cols;
        args[i].rowsStart = start;
        args[i].rowsEnd = end;
        args[i].count = count;
        args[i].offset = offset;
        args[i].bufferSize = bufferSize;
        args[i].fileBuffer = fileBuffer;
        args[i].picture = &picture;
        if(pthread_create(&threads[i], NULL, threadWriteToRows, &args[i])) 
        {
            cerr << "Unable to create thread: " << i << endl;
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < threadsCount; i++)
    {
        pthread_join(threads[i], nullptr);
    }
    write.write(fileBuffer, bufferSize);
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Parallel Write to buffer Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

}
