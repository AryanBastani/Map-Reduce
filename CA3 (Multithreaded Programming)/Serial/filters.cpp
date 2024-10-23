#include "filters.hpp"

static const int blurGaussian[3][3]{
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}};

void VerticalInversion(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < rows / 2; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            Pixel tmpPixel = picture[i][j];
            picture[i][j] = picture[rows - i - 1][j];
            picture[rows - i - 1][j] = tmpPixel;
        }
    }
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Vertical Inversion Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;
}

Pixel convolution(vector<vector<Pixel>> &pic, const int kernel[3][3], int rows_, int cols_)
{
    int red = 0, green = 0, blue = 0;
    for (int i = -1; i < 2; i++)
    {
        for (int j = -1; j < 2; j++)
        {
            int r = rows_ + i;
            int c = cols_ + j;
            if (r < 0)
                r = 0;
            if (r > rows_ - 1)
                r = rows_;
            if (c < 0)
                c = 0;
            if (c > cols_ - 1)
                c = cols_;
            Pixel tmp = pic[r][c];
            red += ((tmp.red) * kernel[i + 1][j + 1])/16;
            green += ((tmp.green) * kernel[i + 1][j + 1])/16;
            blue += ((tmp.blue) * kernel[i + 1][j + 1])/16;
        }
    }
    Pixel res;
    if (red < 0)
        red = 0;
    if (red > 255)
        red = 255;
    if (green < 0)
        green = 0;
    if (green > 255)
        green = 255;
    if (blue < 0)
        blue = 0;
    if (blue > 255)
        blue = 255;
    res.red = red;
    res.green = green;
    res.blue = blue;
    return res;
}

vector<vector<Pixel>> blurPicture(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    vector<vector<Pixel>> pic = picture;
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            pic[i][j] = convolution(picture, blurGaussian, i, j);
        }
    }
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Blur Filter Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

    return pic;
}

void purplize(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // Pixel tmp = picture[i][j];
            int tmp_green = (0.16f * (picture[i][j].red) + 0.5f * (picture[i][j].green) + 0.16f * (picture[i][j].blue));
            if ((tmp_green) > 255)
                tmp_green = 255;
            if ((tmp_green) < 0)
                tmp_green = 0;
            int tmp_blue = (0.6f * (picture[i][j].red) + 0.2f * (picture[i][j].green) + 0.8f * (picture[i][j].blue));
            if ((tmp_blue) > 255)
                tmp_blue = 255;
            if ((tmp_blue) < 0)
                tmp_blue = 0;
            int tmp_red = (0.5f * (picture[i][j].red) + 0.3f * (picture[i][j].green) + 0.5f * (picture[i][j].blue));
            if ((tmp_red) > 255)
                tmp_red = 255;
            if ((tmp_red) < 0)
                tmp_red = 0;
            picture[i][j].red = (unsigned)tmp_red;
            picture[i][j].green = (unsigned)tmp_green;
            picture[i][j].blue = (unsigned)tmp_blue;
        }
    }
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Purple Haze Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

}

void diagonalHatching(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    float slope = (float)rows/cols;
    for(int i = 0; i < cols; i++)
        picture[rows - 1 - (int)(i * slope)][i] = {255, 255, 255};
    for(int i = 0; i < cols/2; i++)
        picture[rows - 1 - (int)(i * slope)][cols/2 + i] = {255, 255, 255};
    for(int i = 0; i < cols/2; i++)
        picture[rows/2 - 1 - (int)(i * slope)][i] = {255, 255, 255};
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Hatching Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()<< endl;

}