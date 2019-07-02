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
vector<vector<complex<double>>> first;
vector<vector<complex<double>>> second;
vector<vector<complex<double>>> swap1;

double PI = atan(1)*4;

//direct fourier
CImg<double> dft(char *name)
{
    CImg<double> image(name);
    CImg<double> final = image;

    //each row
    for (int p = 0; p < image.height(); p++)
    {
        //create vector of 512 elements
        vector<complex<double>> row(image.width());
        //move to 'first'
        first.push_back(row);
        for (int q = 0; q < image.width(); q++)
        {
            complex<double> sum = (0., 0.);
            for (int w = 0; w < image.width(); w++)
            {
                complex<double> temp(cos(2 * M_PI * w * q / image.width()), -sin(2 * M_PI * w * q / image.width()));
                sum += image(w, p) * temp;
            }
            first[p][q] = sum;
        }
    }

    long int sum=0;

//    cout<<first.size()<<endl;
//
//    for(int i =0;i<512;i++)
//    {
//        sum=sum+first[i].size();
//    }
//    cout <<sum<<endl;


    for (int p = 0; p < image.width(); p++)
    {
        vector<complex<double>> row(image.height());
        second.push_back(row);
        swap1.push_back(row);
        for (int q = 0; q < image.height(); q++)
        {
            complex<double> sum = (0., 0.);
            for (int w = 0; w < image.height(); w++)
            {
                complex<double> temp(cos(2 * M_PI * w * q / image.height()), -sin(2 * M_PI * w * q / image.height()));
                sum += first[w][p] * temp;
            }
            second[p][q] = sum;
        }
    }

    sum=0;
    cout<<second.size()<<endl;

    for(int i =0;i<512;i++)
    {
        sum=sum+second[i].size();
    }
    cout <<sum<<endl;




    int a = 0;
    int b = 0;
    for (int i = image.width() / 2; a < image.width(); i++)
    {
        for (int j = image.height() / 2; b < image.height(); j++)
        {
            swap1[j][i] = second[b][a];
            if (j == image.height() - 1) j = -1;
            b++;
           // cout<<"B:"<<b<<endl;
        }
        if (i == image.width() - 1) i = -1;
        b = 0;
        a++;
       // cout<<"A:"<<b<<endl;

    }

    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            second[j][i] = swap1[j][i];
            for (int k = 0; k < image.spectrum(); k++)
            {
                final(i, j, 0, k) = log(abs(second[j][i])) * 15.;
            }
        }
    }

    //final.display();
    return final;
}

//Inverse direcet fourier

CImg<double> idft(char *name)
{
    CImg<double> image(name);
    CImg<double> final = image;

    for (int p = 0; p < image.width(); p++)
    {
        for (int q = 0; q < image.height(); q++)
        {
            complex<double> sum = (0., 0.);
            for (int w = 0; w < image.height(); w++)
            {
                complex<double> temp(cos(2 * M_PI * w * q / image.height()), sin(2 * M_PI * w * q / image.height()));
                sum += second[w][p] * temp;
            }
            first[q][p] = sum / (double) image.height();
        }
    }

    for (int p = 0; p < image.height(); p++)
    {
        for (int q = 0; q < image.width(); q++)
        {
            complex<double> sum = (0., 0.);
            for (int w = 0; w < image.width(); w++)
            {
                complex<double> temp(cos(2 * M_PI * w * q / image.width()), sin(2 * M_PI * w * q / image.width()));
                sum += first[p][w] * temp;
            }
            for (int k = 0; k < image.spectrum(); k++)
            {
                final(p, q, k) = (double) (abs(sum) / image.width());
            }
        }
    }
    return final;
}


vector<complex<double>> part(vector<complex<double>> &row, bool inverse)
{
    int N = row.size();
    vector<complex<double>> evens(N / 2);
    vector<complex<double>> odds(N / 2);
    if (N == 1)
        return row;


    for (int i = 0; i < N / 2; i++)
    {
        evens[i] = row[2 * i];
        odds[i] = row[2 * i + 1];
    }

    vector<complex<double>> part1 = part(evens, inverse);
    vector<complex<double>> part2 = part(odds, inverse);

    for (int k = 0; k < N / 2; k++)
    {
        complex<double> temp;
        if (inverse)
        {
            temp = exp(complex<double>(0, 2. * M_PI * k / N));
            row[k] = (part1[k] + temp * part2[k]);
            row[k + N / 2] = (part1[k] - temp * part2[k]);
        } else
        {
            temp = exp(complex<double>(0, -2. * M_PI * k / N));
            row[k] = part1[k] + temp * part2[k];
            row[k + N / 2] = part1[k] - temp * part2[k];
        }
    }
    return row;
}


//fast fourier

CImg<double> fft(  char *name)
{

    CImg<double> image(name);
    CImg<double> final = image;

    int width = image.width();
    int height = image.height();

    for (int p = 0; p < height; p++)
    {
        vector<complex<double>> row(width);
        for (int i = 0; i < width; i++)
        {
            row[i] = image(i, p);
        }
        first.push_back(part(row, false));
    }


    for (int p = 0; p < image.width(); p++)
    {
        vector<complex<double>> row(height);
        for (int i = 0; i < height; i++)
        {
            row[i] = first[i][p];
        }
        second.push_back(part(row, false));
    }

    swap1 = first;
    int a = 0;
    int b = 0;
    for (int i = image.width() / 2; a < image.width(); i++)
    {
        for (int j = image.height() / 2; b < image.height(); j++)
        {
            swap1[j][i] = second[b][a];
            if (j == image.height() - 1) j = -1;
            b++;
        }
        if (i == image.width() - 1) i = -1;
        b = 0;
        a++;
    }

    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            second[j][i] = swap1[j][i];
            for (int k = 0; k < image.spectrum(); k++)
            {
                final(i, j, 0, k) = log(abs(second[j][i])) * 15.;
            }
        }
    }

//    final.display();
    return final;
}

CImg<double> lowpass(  char *name, int threshold)
{
    CImg<double> image(name);
    for (int p = 0; p < image.width(); p++)
    {
        for (int q = 0; q < image.height(); q++)
        {
            double d = sqrt((pow(p - (image.width() / 2), 2) + pow(q - (image.height() / 2), 2)));
            if (d > threshold)
            {
                second[q][p] = (0., 0.);
            }
        }
    }

    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            for (int k = 0; k < image.spectrum(); k++)
            {
                image(i, j, 0, k) = log(abs(second[j][i])) * 15.;
            }
        }
    }

    image.display();
    return image;
}

CImg<double> highpass(char *name, int threshold)
{
    CImg<double> image(name);
    for (int p = 0; p < image.width(); p++)
    {
        for (int q = 0; q < image.height(); q++)
        {
            double d = sqrt((pow(p - (image.width() / 2), 2) + pow(q - (image.height() / 2), 2)));
            if (d <= threshold)
            {
                second[q][p] = (0., 0.);
            }
        }
    }


//    for (int i = 0; i < image.width(); i++)
//    {
//        for (int j = 0; j < image.height(); j++)
//        {
//            for(int k=0;k<image.spectrum();k++)
//            {
//                image(i, j, 0, k) = log(abs(second[j][i]))*15.;
//            }
//        }
//    }
//
//    image.display();
    return image;
}

CImg<double> bandpass(char *name, int threshold, int bandwidth)
{
    CImg<double> image(name);

    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            double d = sqrt((pow(i - (image.width() / 2), 2) + pow(j - (image.height() / 2), 2)));
            if (d < (threshold - bandwidth / 2) || d > (threshold + bandwidth / 2))
            {
                second[j][i] = (0., 0.);
            }
        }
    }

    for (int w = 0; w < image.width(); w++)
    {
        for (int z = 0; z < image.height(); z++)
        {
            for (int k = 0; k < image.spectrum(); k++)
            {
                image(w, z, 0, k) = log(abs(second[z][w])) * 15.;
            }
        }
    }

    image.display();
    return image;
}

CImg<double> bandcut(char *name, int threshold, int bandwidth)
{
    CImg<double> image(name);
    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            double d = sqrt((pow(i - (image.width() / 2), 2) + pow(j - (image.height() / 2), 2)));
            if (d >= (threshold - bandwidth / 2) && d <= (threshold + bandwidth / 2))
            {
                second[j][i] = (0., 0.);
            }
        }
    }

    for (int w = 0; w < image.width(); w++)
    {
        for (int z = 0; z < image.height(); z++)
        {
            for (int k = 0; k < image.spectrum(); k++)
            {
                image(w, z, 0, k) = log(abs(second[z][w])) * 15.;
            }
        }
    }

    return image;
}





CImg<double> ifft(char *name)
{
    CImg<double> image(name);
    CImg<double> final = image;

    int width = image.width();
    int height = image.height();

    for (int p = 0; p < height; p++)
    {
        vector<complex<double>> row(width);
        vector<complex<double>> temp;
        for (int i = 0; i < width; i++)
        {
            row[i] = second[p][i];
        }
        temp = part(row, true);
        for(int x=0;x<width;x++)
        {
            first[p][x] = temp[x]/(double)width;
        }

    }

    for (int p = 0; p < image.width(); p++)
    {
        vector<complex<double>> row(height);
        vector<complex<double>> temp = row;
        for (int i = 0; i < height; i++)
        {
            row[i] = first[i][p];
        }
        temp = part(row, true);
        for (int i = 0; i < height; i++)
        {
            for (int s = 0; s < image.spectrum(); s++)
            {
                final(i, p, s) = (double)abs(temp[i])/(double)height;
            }
        }
    }

//    final.display();
    return final;
}

CImg<double> pmod(char *name, int l, int k)
{
    CImg<double> image(name);
    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            complex<double> temp(
                    cos((-2 * M_PI * k * i) / image.width() + (-2 * M_PI * l * j) / image.height() + (k + l) * M_PI),
                    sin((-2 * M_PI * k * i) / image.width() + (-2 * M_PI * l * j) / image.height() + (k + l) * M_PI));
            second[j][i] *= temp;
        }
    }

    for (int w = 0; w < image.width(); w++)
    {
        for (int z = 0; z < image.height(); z++)
        {
            for (int k = 0; k < image.spectrum(); k++)
            {
                image(w, z, 0, k) = abs(second[z][w]);
            }
        }
    }

    //image.display();
    return image;
}

CImg<double> highpassedgedet(char *name,  char *name2)
{
    CImg<double> image(name);
    CImg<double> mask(name2);
    for (int p = 0; p < image.width(); p++)
    {
        for (int q = 0; q < image.height(); q++)
        {
            if (mask(p, q) == 0)
            {
                second[q][image.width() - p - 1] = (0., 0.);
            }
        }
    }

//    image.display();
    return image;
}

CImg<double> highpassedgedetmask(char *name, float a, float b, float r)
{
    CImg<double> image(name);
    float alpha = (-a)/(180.0/3.141592653589793238463);
    float beta = (-b)/(180.0/3.141592653589793238463);

    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            float temp = ((float)(j - image.height()/2)/(float)(i - image.width()/2));
            if (((temp < tan(alpha+beta)) || (temp > tan(alpha-beta))) || sqrt(pow(i - image.width()/2, 2) + pow(j - image.height()/2, 2)) < r)
            {
                second[j][image.width() - i - 1] = (0., 0.);
            }
        }
    }
    for (int i = 0; i < image.width(); i++)
    {
        for (int j = 0; j < image.height(); j++)
        {
            for(int k=0;k<image.spectrum();k++)
            {
                image(i, j, 0, k) = log(abs(second[j][image.width() - i - 1]))*15.;
            }
        }
    }

    image.display();
    return image;
}





