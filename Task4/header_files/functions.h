#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <iostream>
#include "CImg.h"
#include <complex>
#include <vector>

using namespace std;
using namespace cimg_library;

CImg<double > dft( char* img_name);
CImg<double > idft( char* img_name);
CImg<double> fft( char* name);
CImg<double> ifft( char* name);
CImg<double> lowpass( char *name, int threshold);
CImg<double> highpass( char *name, int threshold);
CImg<double> bandpass( char *name, int threshold, int bandwidth);
CImg<double> bandcut( char *name, int threshold, int bandwidth);
CImg<double> pmod( char *name, int l, int k);
CImg<double> highpassedgedet( char *name,  char *name2);
CImg<double> highpassedgedetmask( char *name, float a, float b, float r);

#endif
