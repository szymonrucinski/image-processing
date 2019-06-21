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

CImg<double> dft(const char *name)
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
    cout<<first.size()<<endl;

    for(int i =0;i<512;i++)
    {
        sum=sum+first[i].size();
    }
    cout <<sum<<endl;


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
            cout<<"B:"<<b<<endl;
        }
        if (i == image.width() - 1) i = -1;
        b = 0;
        a++;
        cout<<"A:"<<b<<endl;

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

CImg<double> idft(const char *name)
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

CImg<double> fft(const char *name)
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









