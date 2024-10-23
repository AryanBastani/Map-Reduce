#include "bmp.hpp"
#include "filters.hpp"

int main(int argc, char *argv[])
{
    auto startTime = chrono::high_resolution_clock::now();
    char *fileBuffer;
    int bufferSize;
    if (!fillAndAllocate(fileBuffer, argv[1], rows, cols, bufferSize))
    {
        std::cout << "File read error" << std::endl;
        return 1;
    }
    vector<vector<Pixel>> picture(rows, vector<Pixel>(cols));
    getPixelsFromBMP24(bufferSize, fileBuffer, picture);
    VerticalInversion(picture);
    picture = blurPicture(picture); 
    purplize(picture);
    diagonalHatching(picture);
    writeOutBmp24(fileBuffer, bufferSize, picture);
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

    return 0;
}