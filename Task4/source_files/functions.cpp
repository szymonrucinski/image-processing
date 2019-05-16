#include <iostream>
#include <cstdlib>
#include "header_files/functions.h"
#include "header_files/CImg.h"
#include <complex>
#include <cmath>
#include <fstream>
#include <vector>

using namespace std;
using namespace cimg_library;

double PI = atan(1)*4;

void help()
{

}

int checkFile(const char* img)
{
    fstream fileCheck(img);
    if(!fileCheck.good())
    {
        cout<<"File doesn't exist"<<endl;
        fileCheck.close();
        return 0;
    }
    else return 1;
}

void slow_fourier(const char* img_name)
{
    const clock_t begin_time = clock();
    CImg<int> original(img_name);
    int width = original.width();
    int height = original.height();
    CImg<complex<double>> result_normal(width, height);
    CImg<complex<double>> result_inverse(width, height);
    CImg<int> result_display(width, height);
    result_normal = make_fourier(original, width, height, 0);
    //////////////////////////////////////////////////////////////
    normalization(result_normal, width, height);
    //////////////////////////////////////////////////////////////
    result_inverse = make_fourier(result_normal, width, height, 1);
    /////////////////////////////////////////////////////////////////
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_display(x, y) = abs(result_inverse(x, y));
        }
    }
    cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC;
    result_display.save("OUTPUT/SlowFourier_result.bmp");
    //normalization(result_inverse, width, height, 1);
}

CImg<complex<double>>& shift(CImg<complex<double>> to_shift, int width, int height)
{
    CImg<complex<double>>* shifted = new CImg<complex<double>>(width, height);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            (*shifted)(((x + width/2) % width), ((y + height/2) % height)) = to_shift(x, y);
        }
    }
    return *shifted;
    delete shifted;
}

CImg<complex<double>>& make_fourier(CImg<complex<double>> original, int width, int height, bool inverse)
{ int calculate = 0;
int repetition =0;
    CImg<complex<double>>* horizontal = new CImg<complex<double>>(width, height);
    CImg<complex<double>>* vertical = new CImg<complex<double>>(width, height);
    complex<double> sum(0,0);
    complex<double> W(0,0);
    double angle;
    //ROW
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            sum = 0;
            for(int n = 0; n < width; n++)
            {
                //n{0;511}
                if(inverse == true) angle = 2*PI*n*x/width;
                else angle = -2*PI*n*x/width;

                W = cos(angle) +1i * sin(angle);
                W *= original(n, y);
                sum += W;
                calculate =n;
                repetition++;

            }


            Sleep(5000);
            if(inverse == false) (*horizontal)(x, y) = sum;
            else (*horizontal)(x, y) = sum/(complex<double>)(width);
            cout<<sum<<endl;
            cout<<x<<"::"<<y<<endl;
            Sleep(1000);

                    }
    }
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y++)
        {
            sum = 0;
            for(int n = 0; n < height; n++)
            {
                if(inverse == true) angle = 2*PI*n*y/height;
                else angle = -2*PI*n*y/height;
                W = cos(angle) + 1i * sin(angle);
                W *=(*horizontal)(x, n);
                sum += W;
            }
            if(inverse == false) (*vertical)(x, y) = sum;
            else (*vertical)(x, y) = sum/(complex<double>)(height);
        }
    }
    return *vertical;
    delete vertical;
    delete horizontal;
}














void normalization(CImg<complex<double>> to_normalize, int width, int height)
{
    CImg<int> normalized1(width, height);
    CImg<int> normalized2(width, height);
    double maximum = 0;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            normalized1(x, y) = abs(to_normalize(x, y));
            if(maximum < normalized1(x, y)) maximum = normalized1(x, y);
        }
    }
    double c = 255/log(1 + maximum);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            normalized2(((x + width/2) % width), ((y + height/2) % height)) = c * log(1 + abs(normalized1(x, y)));
        }
    }
    normalized2.save("OUTPUT/normalized.bmp");
}

void fast_fourier(const char* img_name)
{
    const clock_t begin_time = clock();
    CImg<int> original(img_name);
    int width = original.width();
    int height = original.height();
    CImg<complex<double>> result_normal(width, height);
    CImg<complex<double>> result_inverse(width, height);
    CImg<int> result_display(width, height);
    result_normal = fast_fast_fourier(original, width, height, 0);
    normalization(result_normal, width, height);
    result_inverse = fast_fast_fourier(result_normal, width, height, 1);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_display(x, y) = abs(result_inverse(x, y));
        }
    }
    cout << float( clock () - begin_time ) /  CLOCKS_PER_SEC;
    result_display.save("OUTPUT/result_display.bmp");
    //normalization(result_inverse, width, height, 1);
}

CImg<complex<double>>& make_fast_fourier(CImg<complex<double>> original, int width, int height, bool inverse)
{
    CImg<complex<double>>* horizontal = new CImg<complex<double>>(width, height);
    CImg<complex<double>>* vertical = new CImg<complex<double>>(width, height);
    complex<double> sum1(0,0);
    complex<double> sum2(0,0);
    complex<double> W1(0,0);
    complex<double> W2(0,0);
    double angle;
    double factorW;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x += 2)
        {
            sum1 = 0;
            sum2 = 0;
            for(int n = 0; n < width/2; n++)
            {
                if(inverse == false)
                {
                   angle = -2*PI*n*x/width;
                   factorW = -2*PI*n/width;
                }
                else
                {
                    angle = 2*PI*n*x/width;
                    factorW = 2*PI*n/width;
                }
                W1 = cos(factorW) + 1i * sin(factorW);
                W2 = cos(angle) + 1i * sin(angle);
                sum1 += (original(n, y)+original(n+width/2, y))*W2;
                sum2 += (original(n, y)-original(n+width/2, y))*W2*W1;
            }
            if(inverse == false)
            {
                (*horizontal)(x, y) = sum1;
                (*horizontal)(x + 1, y) = sum2;
            }

            else
            {
                (*horizontal)(x, y) = sum1/(complex<double>)(width);
                (*horizontal)(x + 1, y) = sum2/(complex<double>)(width);
            }
        }
    }
    for(int x = 0; x < width; x++)
    {
        for(int y = 0; y < height; y += 2)
        {
            sum1 = 0;
            sum2 = 0;
            for(int n = 0; n < height/2; n++)
            {
                if(inverse == false)
                {
                    angle = -2*PI*n*y/height;
                    factorW = -2*PI*n/height;
                }
                else
                {
                    angle = 2*PI*n*y/height;
                    factorW = 2*PI*n/height;
                }
                W1 = cos(factorW) + 1i * sin(factorW);
                W2 = cos(angle) + 1i * sin(angle);
                sum1 += ((*horizontal)(x, n)+(*horizontal)(x, n + height/2))*W2;
                sum2 += ((*horizontal)(x, n)-(*horizontal)(x, n + height/2))*W2*W1;
            }
            if(inverse == false)
            {
                (*vertical)(x, y) = sum1;
                (*vertical)(x, y + 1) = sum2;
            }
            else
            {
                (*vertical)(x, y) = sum1/(complex<double>)(height);
                (*vertical)(x, y + 1) = sum2/(complex<double>)(height);
            }
        }
    }
    return *vertical;
    delete vertical;
    delete horizontal;
}


void pass_filter(const char* img_name, int radius, int option)
{
    CImg<int> original(img_name);
    int width = original.width();
    int height = original.width();
    CImg<int> mask(width, height);
    CImg<complex<double>> in_frequency(width, height);
    CImg<complex<double>> after_filtering(width, height);
    CImg<complex<double>> result_complex(width, height);
    CImg<int> result_normal(width, height);
    in_frequency = fast_fast_fourier(original, width, height, 0);
    in_frequency = shift(in_frequency, width, height);
    if(option == 0) after_filtering = pass(in_frequency, width, height, radius, option);
    else after_filtering = pass(in_frequency, width, height, radius, option);
    after_filtering = shift(after_filtering, width, height);
    normalization(after_filtering, width, height);
    result_complex = fast_fast_fourier(after_filtering, width, height, 1);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_normal(x, y) = abs(result_complex(x, y));
        }
    }
    if(option == 0) result_normal.save("result_low_pass.bmp");
    else result_normal.save("result_high_pass.bmp");
    mask.save("mask.bmp");
}

CImg<complex<double>>& pass(CImg<complex<double>> before, int width, int height, int radius, int option)
{
    CImg<complex<double>>* after = new CImg<complex<double>>(width, height);
    CImg<int> filter(width, height);
    double centerx = width/2;
    double centery = height/2;
    double point;
    for(int y = 0; y < height; y ++)
    {
        for(int x = 0; x < width; x++)
        {
            if(option == 0)
            {
                if(sqrt(pow(x - centerx, 2) + pow(y - centery, 2)) <= radius) (*after)(x, y) = before(x, y);
                else (*after)(x, y) = 0;
            }
            else
            {
                if(sqrt(pow(x - centerx, 2) + pow(y - centery, 2)) > radius) (*after)(x, y) = before(x, y);
                else (*after)(x, y) = 0;
            }
        }
    }
    return *after;
    delete after;
}

void band_filter(const char* img_name, int radius1, int radius2, int option)
{
    CImg<int> original(img_name);
    int width = original.width();
    int height = original.width();
    CImg<complex<double>> in_frequency(width, height);
    CImg<complex<double>> after_filtering(width, height);
    CImg<complex<double>> result_complex(width, height);
    CImg<int> result_normal(width, height);
    in_frequency = fast_fast_fourier(original, width, height, 0);
    in_frequency = shift(in_frequency, width, height);
    if(option == 0) after_filtering = band(in_frequency, width, height, radius1, radius2, option);
    else after_filtering = band(in_frequency, width, height, radius1, radius2, option);
    after_filtering = shift(after_filtering, width, height);
    normalization(after_filtering, width, height);
    result_complex = fast_fast_fourier(after_filtering, width, height, 1);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_normal(x, y) = abs(result_complex(x, y));
        }
    }
    if(option == 0) result_normal.save("result_band_pass.bmp");
    else result_normal.save("result_band_cut.bmp");
}

CImg<complex<double>>& band(CImg<complex<double>> before, int width, int height, int radius1, int radius2, int option)
{
    CImg<complex<double>>* after = new CImg<complex<double>>(width, height);
    CImg<int> filter(width, height);
    double centerx = width/2;
    double centery = height/2;
    for(int y = 0; y < height; y ++)
    {
        for(int x = 0; x < width; x++)
        {
            if(option == 0)
            {
                if(sqrt(pow(x - centerx,2) + pow(y - centery,2)) <= radius2
                    && sqrt(pow(x - centerx,2) + pow(y - centery,2))  > radius1) (*after)(x, y) = before(x, y);
                else (*after)(x, y) = 0;
            }
            else
            {
                if(sqrt(pow(x - centerx,2) + pow(y - centery,2))  > radius2 || sqrt(pow(x - centerx,2) + pow(y - centery,2))  <= radius1) (*after)(x, y) = before(x, y);
                else (*after)(x, y) = 0;
            }
        }
    }
    return *after;
    delete after;
}

void phase_modifying_filter(const char* img_name, int k, int l)
{
    CImg<int> original(img_name);
    int width = original.width();
    int height = original.width();
    CImg<complex<double>> in_frequency(width, height);
    CImg<complex<double>> after_filtering(width, height);
    CImg<complex<double>> mask(width, height);
    CImg<complex<double>> result_complex(width, height);
    CImg<int> result_normal(width, height);
    in_frequency = fast_fast_fourier(original, width, height, 0);
    in_frequency = shift(in_frequency, width, height);
    mask = make_mask(k, l, width, height);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            after_filtering(x, y) = in_frequency(x, y) * mask(x, y);
        }
    }
    after_filtering = shift(after_filtering, width, height);
    result_complex = fast_fast_fourier(after_filtering, width, height, 1);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_normal(x, y) = abs(result_complex(x, y));
        }
    }
    result_normal.save("result_phase_mod.bmp");
}

CImg<complex<double>>& make_mask(int k, int l, int width, int height)
{
    CImg<complex<double>>* mask = new CImg<complex<double>>(width, height);
    double angle;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            angle = (((-x)*k*2*PI/width) - (y*l*2*PI/height) + (k+l)*PI);
            (*mask)(x, y) = cos(angle) + 1i*sin(angle);
        }
    }
    return *mask;
    delete mask;
}

void F5(const char* img, int radius, int angle, int gape)
{
    CImg<int> original(img);
    int width = original.width();
    int height = original.width();
    CImg<complex<double>> in_frequency(width, height);
    CImg<complex<double>> after_filtering(width, height);
    CImg<complex<double>> result_complex(width, height);
    CImg<int> result_normal(width, height);
    in_frequency = fast_fast_fourier(original, width, height, 0);
    in_frequency = shift(in_frequency, width, height);
    mask_generate(radius, angle, gape, width, height);
    CImg<int> mask_used("maskF5.bmp");
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(mask_used(x, y) == 0) after_filtering(x, y) = 0;
            else after_filtering(x, y) = in_frequency(x, y);
        }
    }
    after_filtering = shift(after_filtering, width, height);
    normalization(after_filtering, width, height);
    result_complex = fast_fast_fourier(after_filtering, width, height, 1);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            result_normal(x, y) = abs(result_complex(x, y));
        }
    }
    result_normal.save("result_F5.bmp");
}

void mask_generate(int radius, int angle, int gape, int width, int height)
{
    if(gape > 90) gape = 90;
    CImg<int> mask(width, height);
    double anglerad2 = ((double)(((angle+gape)%180))/180)*PI;
    double anglerad1 = (((double)((angle-gape)%180))/180)*PI;
    double tangens1 = tan(anglerad1);
    double tangens2 = tan(anglerad2);
    double centerx = (double)(width/2);
    double centery = (double)(height/2);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x <width; x++)
        {
            if(tangens1 < tangens2)
            {
                if((centerx != x)&&((centery - y)/(x - centerx)>=tangens1) && ((centery - y)/(x - centerx) <= tangens2))
                {
                    mask(x, y) = 255;
                }
                else if((centerx == x) && anglerad2 >= PI &&anglerad1 <=PI) mask(x,y) = 255;
            }
            else if(tangens1 >= tangens2)
            {
                if((centerx != x)&&(((centery - y)/(x - centerx)>=tangens1) || ((centery - y)/(x - centerx) <= tangens2)))
                {
                    mask(x, y) = 255;
                }
                else if(centerx == x && (anglerad2 >= PI || anglerad1 <= PI)) mask(x, y) = 255;
            }
            else mask(x, y) = 0;
            if(sqrt(pow(x - centerx, 2) + pow(y - centery, 2)) <= radius) mask(x, y) = 0;
        }
    }
    mask.save("maskF5.bmp");
}

vector<complex<double>> split(vector<complex<double>> arr, bool inverse)
{
    if(arr.size() == 1) return arr;
    int half = arr.size()/2;
    int N = arr.size();
    vector<complex<double>> part1(half);
    vector<complex<double>> part2(half);
    complex<double> W;
    for (int i = 0; i < half; i++)
	{
	    if(inverse == false) W = cos(2 * PI * i / N) - 1i * sin(2 * PI * i / N);
		else  W = cos(2 * PI * i / N) + 1i * sin(2 * PI * i / N);
		part1[i] = arr[i] + arr[half + i];
		part2[i] = (arr[i] - arr[half + i]) * W;
	}

	vector <complex<double>> partition1 = split(part1, inverse);
	vector<complex<double>> partition2 = split(part2, inverse);

	for (int k = 0; k < half; k++)
	{
            arr[2 * k] = partition1[k];
            arr[2 * k + 1] = partition2[k];
	}
    return arr;
}

CImg<complex<double>>& fast_fast_fourier(CImg<complex<double>> original, int width, int height, bool inverse)
{
    CImg<complex<double>>* horizontal = new CImg<complex<double>>(width, height);
    CImg<complex<double>>* vertical = new CImg<complex<double>>(width, height);
    vector<complex<double>> row(width);
    vector<complex<double>> column(height);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            row[x] = original(x, y);
        }
        row = split(row, inverse);
        for(int x = 0; x < width; x++)
        {
            if(inverse == true) (*horizontal)(x, y) = row[x]/(complex<double>)(width);
            else (*horizontal)(x, y) = row[x];
        }
    }
    for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			column[y] = (*horizontal)(x, y);
		}
		column = split(column, inverse);
		for (int y = 0; y < width; y++)
		{
			if(inverse == true) (*vertical)(x, y) = column[y]/(complex<double>)(height);
            else (*vertical)(x, y) = column[y];
		}
	}
	return *vertical;
    delete vertical;
    delete horizontal;
}
