#ifndef __FILTER_H__
#define __FILTER_H__

#include "bmp.hpp"

using namespace std;

void VerticalInversion(vector<vector<Pixel>> &picture);
vector<vector<Pixel>> blurPicture(vector<vector<Pixel>> &picture);
void purplize(vector<vector<Pixel>> &picture);
void diagonalHatching(vector<vector<Pixel>> &picture);

#endif