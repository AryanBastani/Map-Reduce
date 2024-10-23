#include "filters.hpp"

static const int blurGaussian[3][3]{
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}};

void *threadVerticalInversion(void *arguments)
{

    struct VerticalInversionArgs *args;
    args = static_cast<VerticalInversionArgs *>(arguments);

    for (int i = 0; i < args->rows_ / 2; i++)
    {
        for (int j = args->colsStart; j < args->closEnd; j++)
        {
            Pixel temp = (*args->picture)[i][j];
            (*args->picture)[i][j] = (*args->picture)[args->rows_ - i - 1][j];
            (*args->picture)[args->rows_ - i - 1][j] = temp;
        }
    }

    return nullptr;
}

void VerticalInversion(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    pthread_t *threads = new pthread_t[THREAD_COUNT];
    int threadPart = cols / THREAD_COUNT;
    auto *args = new VerticalInversionArgs[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        args[i].picture = &picture;
        args[i].rows_ = rows;
        int start = i * threadPart;
        int end = (i + 1) * threadPart;

        if (i == THREAD_COUNT - 1)
            end = cols;

        args[i].colsStart = start;
        args[i].closEnd = end;

        if (pthread_create(&threads[i], NULL, threadVerticalInversion, &args[i]))
        {
            cerr << "Unable to create thread: " << i << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] args;
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Parallel Vertical Inversion Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << endl;
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
            red += ((tmp.red) * kernel[i + 1][j + 1]) / 16;
            green += ((tmp.green) * kernel[i + 1][j + 1]) / 16;
            blue += ((tmp.blue) * kernel[i + 1][j + 1]) / 16;
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

void *threadBlurPicture(void *arguments)
{
    struct BlurArgs *args;
    args = static_cast<BlurArgs *>(arguments);

    for (int i = args->rowsStart; i < args->rowsEnd; i++)
    {
        for (int j = 0; j < args->cols_ - 1; j++)
        {
            (*args->pictureBlur)[i][j] = convolution(*args->picture, blurGaussian, i, j);
        }
    }

    return nullptr;
}

vector<vector<Pixel>> blurPicture(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    vector<vector<Pixel>> pic = picture;
    pthread_t *threads = new pthread_t[THREAD_COUNT];
    int threadPart = rows / THREAD_COUNT;
    auto *args = new BlurArgs[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        args[i].cols_ = cols;
        args[i].rowsStart = i * threadPart;
        args[i].rowsEnd = (i + 1) * threadPart;
        args[i].picture = &picture;
        args[i].pictureBlur = &pic;
        if (i == THREAD_COUNT - 1)
            args[i].rowsEnd = rows;
        if (pthread_create(&threads[i], NULL, threadBlurPicture, &args[i]))
        {
            cerr << "Unable to create thread: " << i << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] args;
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Parallel Blur Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << endl;
    return pic;
}

void *threadPurplize(void *argument)
{
    struct PurpleArgs *args;
    args = static_cast<PurpleArgs *>(argument);

    for (int i = args->rowsStart; i < args->rowsEnd; i++)
    {
        for (int j = 0; j < args->cols_; j++)
        {
            // cout << "vayyyyy\n";
            int tmp_green = (0.16f * ((*args->picture)[i][j].red) + 0.5f * ((*args->picture)[i][j].green) + 0.16f * ((*args->picture)[i][j].blue));
            if ((tmp_green) > 255)
                tmp_green = 255;
            if ((tmp_green) < 0)
                tmp_green = 0;
            int tmp_blue = (0.6f * ((*args->picture)[i][j].red) + 0.2f * ((*args->picture)[i][j].green) + 0.8f * ((*args->picture)[i][j].blue));
            if ((tmp_blue) > 255)
                tmp_blue = 255;
            if ((tmp_blue) < 0)
                tmp_blue = 0;
            int tmp_red = (0.5f * ((*args->picture)[i][j].red) + 0.3f * ((*args->picture)[i][j].green) + 0.5f * ((*args->picture)[i][j].blue));
            if ((tmp_red) > 255)
                tmp_red = 255;
            if ((tmp_red) < 0)
                tmp_red = 0;
            (*args->picture)[i][j].red = (unsigned)tmp_red;
            (*args->picture)[i][j].green = (unsigned)tmp_green;
            (*args->picture)[i][j].blue = (unsigned)tmp_blue;
        }
    }
    cout << "aaaaaaaaaaar\n";   
}

void purplize(vector<vector<Pixel>> &picture)
{
    auto startTime = chrono::high_resolution_clock::now();
    pthread_t *threads = new pthread_t[THREAD_COUNT];
    int threadPart = rows / THREAD_COUNT;
    auto *args = new PurpleArgs[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        args[i].picture = &picture;
        args[i].cols_ = cols;
        int start = i * threadPart;
        int end = (i + 1) * threadPart;

        if (i == THREAD_COUNT - 1)
            end = rows;

        args[i].rowsStart = start;
        args[i].rowsEnd = end;
        // cout << "annn\n";
        if (pthread_create(&threads[i], NULL, threadPurplize, &args[i]))
        {
            cerr << "Unable to create thread: " << i << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        pthread_join(threads[i], nullptr);
    }
    delete[] threads;
    delete[] args;
    auto endTime = chrono::high_resolution_clock::now();
    cout << "Parallel Purple Haze Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << endl;
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
    cout << "Hatching Time: " << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count() << endl;
}