#ifndef SEGMENTATION
#define SEGMENTATION
#include <iostream>
#include "CImg.h"
#include <vector>
#include "header_files/functions.h"

using namespace std;
using namespace cimg_library;

struct Pixel
{
    Pixel();
    Pixel(int x, int y);
    int x;
    int y;
};

struct SegmentRegion
{
    Pixel seed;
    vector<Pixel> pixels;
};

void applySegmentation(const char* img_name, int x, int y, int threshold);
bool linearThreshold(CImg<int> &original, int threshold, int x1, int y1, int x2, int y2);
bool euclideanThreshold(CImg<int> &original, int threshold, int x1, int y1, int x2, int y2);
void segmentationRecursive(CImg<int> &original, CImg<int> &edited, int x, int y, int threshold, bool** alreadyInRegion, Pixel seed);

#endif //SEGMENTATION