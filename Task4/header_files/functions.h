#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <iostream>
#include "CImg.h"
#include <complex>
#include <vector>

using namespace std;
using namespace cimg_library;

void help();
int checkFile(const char* img);
//dopytac Krzycha
void slow_fourier(const char* img_name);
//dopytac
CImg<complex<double>>& make_fourier(CImg<complex<double>> original, int width, int height, bool inverse);
//dopytac
void normalization(CImg<complex<double>> to_normalize, int width, int height);

void fast_fourier(const char* img_name);
CImg<complex<double>>& make_fast_fourier(CImg<complex<double>> original, int width, int height, bool inverse);
CImg<complex<double>>& shift(CImg<complex<double>> to_shift, int width, int height);
void pass_filter(const char* img_name, int radius, int option);

CImg<complex<double>>& pass(CImg<complex<double>> before, int width, int height, int radius, int option);
void band_filter(const char* img_name, int radius1, int radius2, int option);
CImg<complex<double>>& band(CImg<complex<double>> before, int width, int height, int radius1, int radius2, int option);
CImg<complex<double>>& make_mask(int k, int l, int width, int height);
void phase_modifying_filter(const char* img_name, int k, int l);
void F5(const char* img, int radius, int angle, int gape);
void mask_generate(int radius, int angle, int gape, int width, int height);
CImg<complex<double>>& fast_fast_fourier(CImg<complex<double>> original, int width, int height, bool inverse);
vector<complex<double>> split(vector<complex<double>> arr);

#endif
