#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <iostream>

using namespace std;

int checkFile(const char* img);
void error();
void histogram(const char* img, int spectrum);
void help();
void new_contrast(const char* img_name, float value);
void hraleigh(const char* img_name, int gmin, int gmax);
void slineid(const char* img_name);
void oslineid(const char* img_name);
void orobertsi(const char* img_name);
void cmean(const char* img_name);
void cvariance(const char* img_name);
void cstdev(const char* img_name);
void cvarcoi(const char* img_name);
void casyco(const char* img_name);
void flaco(const char* img_name);
void cvarcoii(const char* img_name);
void centropy(const char* img_name);

#endif
