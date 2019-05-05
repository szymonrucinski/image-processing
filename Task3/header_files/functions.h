#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <iostream>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

int checkFile(const char* img);
void error();
void help();
//1
void dilation(const char* img_name);
//2
void erosion(const char* img_name);
//3
void display(const char* img_name);
void opening(const char* img_name);
void closing(const char* img_name);
void HMT(const char* img_name);
int check_coordinates(int fun_width, int fun_height, int fun_x, int fun_y);
void M2(const char* img_name);
CImg<int>& iterative_fun(CImg<int>* image_to_edition, CImg<int>* image_complement, CImg<int>* image_editted, int mask, int width, int height);
CImg<int>& complement(const char* img_name);
CImg<int>& dilation_to_fun(CImg<int>* image_original, int mask, int width, int height);
void reggrow(const char* img_name, int threshold, int x, int y);
void HMT_task(const char* img_name);
void erosion_task(const char* img_name, int option);

#endif
