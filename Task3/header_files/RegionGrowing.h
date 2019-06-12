#ifndef REGGROW
#define REGGROW


#include "header_files/CImg.h"
#include <vector>

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

CImg<int>& applySegmentation(CImg<int> &original, int x, int y, int threshold);
bool euclideanThreshold(CImg<int> &original, int threshold, int x1, int y1, int x2, int y2);
void segmentationRecursive(CImg<int> &original, CImg<int> &edited, int x, int y, int threshold, bool** alreadyInRegion, Pixel seed);

#endif //SEGMENTATION