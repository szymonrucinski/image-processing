#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <iostream>
#include "CImg.h"
#include <complex>
#include <vector>

using namespace std;
using namespace cimg_library;

CImg<double > dft(const char* img_name);
CImg<double > idft(const char* img_name);
CImg<double> fft(const char* name);




#endif
