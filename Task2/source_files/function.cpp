#include <iostream>
#include <cstdlib>
#include "../header_files/function.h"
#include "../header_files/CImg.h"

#include <algorithm>
#include <fstream>
#include <cmath>

using namespace std;
using namespace cimg_library;

int checkFile(const char* img)
{
	fstream fileCheck(img);
	if (!fileCheck.good())
	{
		cout << "File doesn't exist" << endl;
		fileCheck.close();
		return 0;
	}
	else return 1;
}

void error()
{
	cout << "Ooops! Something went wrong! Try to write '--help' as a parameter to get help." << endl;
}

void histogram(const char* img, int z)   //fucking not sure!
{
    double sum;
	CImg<unsigned char> image(img);
	int height = image.height();
	int width = image.width();
	CImg<unsigned char>histogram(256, 256, 1, 3); //256x256 pixels, 2D, 3 channels
	int occurence[256] = { 0 };

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			occurence[image(x, y, z)]++;                //there are 0-255 cell indexes
			                                            //increment value in one cell that correspods to an amount of pixels present on an image
		}
	}

	bool graph;
	for (int x = 0; x < 256; x++)
	{
		graph = false;
		for (int y = 0; y <256; y++)
		{
			for (int s = 0; s < image.spectrum(); s++)
			{


                if (s == z)     //if value of chosen RGB channel is equal to current channel from spectrum
				{
					if (y == (int)((occurence[x]) / 30) || graph == true) //y value (0-255) is equal to occurence[x]/30
					{
						histogram(x, 255 - y, s) = 0;       //assign "black color" to an image histogram(x,255-y,channel)

						if (graph == false) graph = true;
					}
					else histogram(x, 255 - y, s) = 255; //255-y because of horizontal mirror flip
				}
				else histogram(x, 255-y, s) = 0; //assign black not our channel;
			}
		}
	}
	CImgDisplay main_disp(histogram);
	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	histogram.save("histogram.bmp");
}

void hraleigh(const char* img_name, int gmin, int gmax)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	int range = gmax - gmin;
	float sigma = (float)range / 4;
	double factor = 2 * pow(sigma, 2);
	double N = height*width;
	int **occurence = new int*[spectrum]();
	for (int i = 0; i < spectrum; ++i)
	{
		occurence[i] = new int[256]();
	}
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				int v = image(x, y, z);
				occurence[z][v]++;
			}
		}
	}
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				double sum = 0;
				for (int i = 0; i <= image(x, y, z); i++)
				{
					sum += occurence[z][i];
				}
				double logarithm = log((double)N / (double)sum);
				double value = 0;
				value = gmin + sqrt(logarithm*factor);
				if (value>255)   image(x, y, z) = 255;
				else if (value <= 0)  image(x, y, z) = 0;
				else image(x, y, z) = value;
			}
		}
	}
	CImgDisplay main_disp(image, img_name);
	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	image.save("newOne.bmp");
	if (spectrum == 3)
	{
		histogram("newOne.bmp", 0);
		histogram("newOne.bmp", 1);
		histogram("newOne.bmp", 2);
	}
	else histogram("newOne.bmp", 0);
	for (int i = 0; i < spectrum; i++)
	{
		delete[] occurence[i];
	}
	delete[] occurence;
}

void slineid(const char* img_name)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	CImg<unsigned char> image1(height, width, 1, spectrum);
	CImg<unsigned char> image2(height, width, 1, spectrum);
	CImg<unsigned char> image3(height, width, 1, spectrum);
	CImg<unsigned char> image4(height, width, 1, spectrum);
	int h1[] = { -1, 2, -1, -1, 2, -1, -1, 2, -1 };
	int h2[] = { -1, -1, -1, 2, 2, 2, -1, -1, -1 };
	int h3[] = { -1, -1, 2, -1, 2, -1, 2, -1, -1 };
	int h4[] = { 2, -1, -1, -1, 2, -1, -1, -1, 2 };
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				int Size = 9;
				int help = 0;
				int sum1 = 0;
				int sum2 = 0;
				int sum3 = 0;
				int sum4 = 0;
				for (int j = y - 1; j <= y + 1; j++)
				{
					for (int i = x - 1; i <= x + 1; i++)
					{
						if ((j<0) || (j >= height) || (i<0) || (i >= width))
						{
							Size--;
						}
						else
						{
							sum1 += image(i, j, z) * h1[help];
							sum2 += image(i, j, z) * h2[help];
							sum3 += image(i, j, z) * h3[help];
							sum4 += image(i, j, z) * h4[help];
						}
						help++;
					}
				}
				if (sum1 / Size < 0) image1(x, y, z) = 0;
				else if (sum1 / Size >= 256) image1(x, y, z) = 255;
				else image1(x, y, z) = sum1 / Size;
				if (sum2 / Size < 0) image2(x, y, z) = 0;
				else if (sum2 / Size >= 256) image2(x, y, z) = 255;
				else image2(x, y, z) = sum2 / Size;
				if (sum3 / Size < 0) image3(x, y, z) = 0;
				else if (sum3 / Size >= 256) image3(x, y, z) = 255;
				else image3(x, y, z) = sum3 / Size;
				if (sum4 / Size < 0) image4(x, y, z) = 0;
				else if (sum4 / Size >= 256) image4(x, y, z) = 255;
				else image4(x, y, z) = sum4 / Size;
			}
		}
	}
	CImgDisplay main_disp1(image1);
	while (!main_disp1.is_closed())
	{
		main_disp1.wait();
	}
	image1.save("newOne1.bmp");
	CImgDisplay main_disp2(image2);
	while (!main_disp2.is_closed())
	{
		main_disp2.wait();
	}
	image2.save("newOne2.bmp");
	CImgDisplay main_disp3(image3);
	while (!main_disp3.is_closed())
	{
		main_disp3.wait();
	}
	image3.save("newOne3.bmp");
	CImgDisplay main_disp4(image4);
	while (!main_disp4.is_closed())
	{
		main_disp4.wait();
	}
	image4.save("newOne4.bmp");
}

void oslineid(const char* img_name)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	CImg<unsigned char> image1(height, width, 1, spectrum);
	int mask[] = { -1, 2, -1 };
	int help[6];
	for (int z = 0; z < spectrum; z++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			for (int i = x - 1, c = 0; i <= x + 1; i++, c++)
			{
				help[c] = image(i, 0, z) * mask[c];
				help[c + 3] = image(i, 1, z) *mask[c];
			}
			int help_sum = 0;
			int sum = 0;
			for (int i = 0; i < 6; i++)
			{
				help_sum += help[i];
			}
			for (int y = 1; y < height - 1; y++)
			{
				int result;
				for (int a = x - 1, b = 0; a <= x + 1; a++, b++)
				{
					result = image(a, y + 1, z) * mask[b];
					sum += result;
					help[b] = help[b + 3];
					help[b + 3] = result;
				}
				sum += help_sum;
				if (sum / 9 < 0) image1(x, y, z) = 0;
				else if (sum / 9 >= 256) image1(x, y, z) = 255;
				else image1(x, y, z) = sum / 9;
				help_sum = 0; sum = 0;
				for (int i = 0; i < 6; i++)
				{
					help_sum += help[i];
				}
			}
		}
	}
	CImgDisplay main_disp(image1);
	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	image1.save("newOne.bmp");
}


void orobertsi(const char* img_name)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	CImg<unsigned char> image1(height, width, 1, spectrum);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				if (x + 1 >= width || y + 1 >= height)
				{
					image1(x, y, z) = image(x, y, z);
				}
				else
				{
					int component1 = pow(image(x, y, z) - image(x + 1, y + 1, z), 2);
					int component2 = pow(image(x, y + 1, z) - image(x + 1, y, z), 2);
					int value = sqrt(component1 + component2);
					if (value < 0) image1(x, y, z) = 0;
					else if (value >= 256) image1(x, y, z) = 255;
					else image1(x, y, z) = value;
				}
			}
		}
	}
	CImgDisplay main_disp(image1);
	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	image1.save("newOne.bmp");
}

void help()
{
	cout << "There are 4 groups of commands:" << endl;
	cout << "1. With syntax: 'name of executable file' 'command' 'name of an image'" << endl;
	cout << "Commands in this group:" << endl;
	cout << "       --negative " << endl;
	cout << "       --hflip     - It flips the image horizontally." << endl;
	cout << "       --vflip     - It flips the image vertically." << endl;
	cout << "       --dflip     - It flips the image diagonally." << endl;
	cout << "       --slineid   - It modifies the image using all 4 line id matrices." << endl;
	cout << "       --oslineid  - It is the optimalized algorithm modifying the image using only 1 line id matrix." << endl;
	cout << "       --orobertsi - It modifies the image using Roberts Operator I." << endl;
	cout << "2. With syntax: 'name of executable file' 'command' 'name of an image' 'integer value'" << endl;
	cout << "Commands in this group:" << endl;
	cout << "       --brightness  positive values of arguments are used for increasing the brightness and negative for decreasing the brightness" << endl;
	cout << "       --contrast    positive values of arguments are used for increasing the contrast and negative for decreasing the contrast" << endl;
	cout << "       --median      applies the median filter for the picture (use only non-negative integer arguments to increase the radius)" << endl;
	cout << "       --hmean       applies the harmonic mean filter for the picture (use only non-negative integer arguments to increase the radius)" << endl;
	cout << "       --histogram   displays histogram of an image" << endl;
	cout << "3. With syntax: 'name of executable file' 'command' 'name of an image' 'float or integer value'" << endl;
	cout << "       --enlarge    enlarges the given image";
	cout << "       --shrink     shrinks the given image";
	cout << "4. With syntax: 'name of executable file' 'command' 'name of an original image' 'name of image with noise' 'filtered image" << endl;
	cout << "       --mse        counts the mean square error between the images." << endl;
	cout << "       --pmse       counts the peak mean square error between the images." << endl;
	cout << "       --snr        counts the signal to noise ratio between the images." << endl;
	cout << "       --psnr       counts the peak signal to noise ratio between the images." << endl;
	cout << "       --md         counts the maximum difference between the images." << endl;
	cout << "       --cmean       counts the mean of pixel intesity values." << endl;
	cout << "       --cvariance   calculates variance." << endl;
	cout << "       --cstdev      counts the standard deviation of pixel values of an image." << endl;
	cout << "       --cvarcoi     calculates the coefficient of variatios." << endl;
	cout << "       --casyco      counts the skewness of an image." << endl; 
	cout << "       --flaco       counts the kurtosis of an image." << endl;
	cout << "       --cvarcoii    counts the 2nd variation coefficient  of an image." << endl;
	cout << "       --centropy    counts the information source entropy of an image." << endl;

	cout << "5. With syntax: 'name of executable file' 'command' 'name of an original image' 'integer value' 'integer value" << endl;
	cout << "       --hraleigh   modifies the original image using Raleigh final probability density function." << endl;
	cout << "                    You must give min and max values of pixels." << endl;
}



void new_contrast(const char* img_name, float value)
{
	if (value <= 0)
	{
		cout << "The value is not in a range (0, infinity)" << endl;
		return;
	}
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				double base = ((double)image(x, y, z) / 255);
				double exponent = pow(base, (double)value);
				if ((255 * exponent)>255) image(x, y, z) = 255;
				else image(x, y, z) = floor(255 * exponent);
			}
		}
	}
	CImgDisplay main_disp(image, img_name);
	while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	image.save("newOne.bmp");
}





void cmean(const char* img_name)			//done
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0);
			g_sum = g_sum + (int)image(x, y, 1);
			b_sum = b_sum + (int)image(x, y, 2);

		}
	}


	float r_mean = (r_sum / (height*width));
	float g_mean = (g_sum / (height*width));
	float b_mean = (b_sum / (height*width));
	float rgb_mean = ((r_mean + g_mean + b_mean) / (3));

	cout << "R mean is equal to " << r_mean << endl;
	cout << "G mean is equal to " << g_mean << endl;
	cout << "B mean is equal to " << b_mean << endl;
	cout << "R-G-B mean is equal to " << rgb_mean << endl;


}

void cvariance(const char* img_name)	//done
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0);
			g_sum = g_sum + (int)image(x, y, 1);
			b_sum = b_sum + (int)image(x, y, 2);

		}
	}


	float r_mean = (r_sum / (height*width));
	float g_mean = (g_sum / (height*width));
	float b_mean = (b_sum / (height*width));
	float rgb_mean = (r_mean + g_mean + b_mean) / 3;

	cout << r_mean << endl;
	cout << g_mean << endl;
	cout << b_mean << endl;



	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			r_variance = r_variance + pow(((int)image(x, y, 0) - r_mean), 2);
			g_variance = g_variance + pow(((int)image(x, y, 1) - g_mean), 2);
			b_variance = b_variance + pow(((int)image(x, y, 2) - b_mean), 2);


		}
	}

	cout << "variance is equal to " << ((r_variance + g_variance + b_variance) / (height*width * 3)) << endl;


}

void cstdev(const char* img_name) //done
{

	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0);
			g_sum = g_sum + (int)image(x, y, 1);
			b_sum = b_sum + (int)image(x, y, 2);

		}
	}


	float r_mean = (r_sum / (height*width));
	float g_mean = (g_sum / (height*width));
	float b_mean = (b_sum / (height*width));
	float rgb_mean = (r_mean + g_mean + b_mean) / 3;



	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			r_variance = r_variance + pow(((int)image(x, y, 0) - r_mean), 2);
			g_variance = g_variance + pow(((int)image(x, y, 1) - g_mean), 2);
			b_variance = b_variance + pow(((int)image(x, y, 2) - b_mean), 2);


		}
	}

	cout << "standard RGB deviation is equal to " << sqrt((r_variance + g_variance + b_variance) / (height*width * 3)) << endl;
	cout << "standard R deviation is equal to " << sqrt((r_variance) / (height*width)) << endl;
	cout << "standard G deviation is equal to " << sqrt((g_variance) / (height*width)) << endl;
	cout << "standard B deviation is equal to " << sqrt((b_variance) / (height*width)) << endl;

	//checked using Photoshop




}

void cvarcoi(const char* img_name) //done
{
	//(standard_deviation)/(rgb_mean)

	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)					//sum of individual pixel channel values 
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0); //red
			g_sum = g_sum + (int)image(x, y, 1); //green
			b_sum = b_sum + (int)image(x, y, 2); //blue

		}
	}


	float r_mean = (r_sum / (height*width));		//average red value
	float g_mean = (g_sum / (height*width));		//avarage green value 
	float b_mean = (b_sum / (height*width));		//avarage blue value
	float rgb_mean = (r_mean + g_mean + b_mean) / 3;  //avarage RGB value

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			r_variance = r_variance + pow(((int)image(x, y, 0) - r_mean), 2);			//summing all (Red values - Red_channel avarage value)^2
			g_variance = g_variance + pow(((int)image(x, y, 1) - g_mean), 2);
			b_variance = b_variance + pow(((int)image(x, y, 2) - b_mean), 2);


		}
	}

	float stdev = sqrt((r_variance + g_variance + b_variance) / (height*width * 3));

	

	cout << "Variation coefficient is equal to " << stdev / rgb_mean << endl;




}

void casyco(const char* img_name) //done
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0);
			g_sum = g_sum + (int)image(x, y, 1);
			b_sum = b_sum + (int)image(x, y, 2);

		}
	}


	float r_mean = (r_sum / (height*width));
	float g_mean = (g_sum / (height*width));
	float b_mean = (b_sum / (height*width));
	float rgb_mean = (r_mean + g_mean + b_mean) / 3;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			r_variance = r_variance + pow(((int)image(x, y, 0) - r_mean), 2);
			g_variance = g_variance + pow(((int)image(x, y, 1) - g_mean), 2);
			b_variance = b_variance + pow(((int)image(x, y, 2) - b_mean), 2);


		}
	}

	double sigma = sqrt((r_variance + g_variance + b_variance) / (height*width * 3));


	//checked using Photoshop
	///////////////////////////////////////////////////////////////////////////////////////////

	long double cr_moment = 0;
	long double cg_moment = 0;
	long double cb_moment = 0;

	y, x, z = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			cr_moment = cr_moment + pow(((int)image(x, y, 0)) - r_mean, 3);						//central moment4 //uk=E(X-b)^k
			cg_moment = cg_moment + pow(((int)image(x, y, 1)) - g_mean, 3);
			cb_moment = cb_moment + pow(((int)image(x, y, 2)) - b_mean, 3);

		}
	}

	double fraction = (((cr_moment + cg_moment + cb_moment) / pow(sigma, 3)));
	 float skewness = (fraction / (width*height));
	cout << "Skewness is equal to " << skewness << endl;



}

void flaco(const char* img_name) //done
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	double r_variance = 0;
	double g_variance = 0;
	double b_variance = 0;
	long double r_sum = 0;
	long double g_sum = 0;
	long double b_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			r_sum = r_sum + (int)image(x, y, 0);
			g_sum = g_sum + (int)image(x, y, 1);
			b_sum = b_sum + (int)image(x, y, 2);

		}
	}


	float r_mean = (r_sum / (height*width));
	float g_mean = (g_sum / (height*width));
	float b_mean = (b_sum / (height*width));
	float rgb_mean = (r_mean + g_mean + b_mean) / 3;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			r_variance = r_variance + pow(((int)image(x, y, 0) - r_mean), 2);
			g_variance = g_variance + pow(((int)image(x, y, 1) - g_mean), 2);
			b_variance = b_variance + pow(((int)image(x, y, 2) - b_mean), 2);


		}
	}

	double sigma = sqrt((r_variance + g_variance + b_variance) / (height*width * 3));


	//checked using Photoshop
	///////////////////////////////////////////////////////////////////////////////////////////

	long double cr_moment = 0;
	long double cg_moment = 0;
	long double cb_moment = 0;

	y, x, z = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{

			cr_moment = cr_moment + pow(((int)image(x, y, 0)) - r_mean, 4);						//central moment4 //uk=E(X-b)^k
			cg_moment = cg_moment + pow(((int)image(x, y, 1)) - g_mean, 4);
			cb_moment = cb_moment + pow(((int)image(x, y, 2)) - b_mean, 4);

		}
	}

	double fraction = (((cr_moment + cg_moment + cb_moment) / pow(sigma, 4)) - 3);
	 float kurtosis = (fraction / (width*height));
	cout << "Kurtosis is equal to " << kurtosis << endl;
}

void cvarcoii(const char* img_name) //done
{

	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();

	long double p_sum = 0;


	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			for (z = 0; z< 3; z++)
			{
				p_sum = p_sum + (pow((int)image(x, y, z), 2));
			}

		}
	}

	cout << "Variation coefficient II is equal to " << ((p_sum) / (pow(height, 2)*pow(width, 2))) << endl;
}

void centropy(const char* img_name) //done
{

	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	long  p_sum = 0;
	int y, x, z = 0;


	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)

		{
			for (z = 0; z < 3; z++)
			{
				p_sum = p_sum + ((int)image(x, y, z)*log2(((int)image(x, y, z))));
			}

		}
	}

	cout << "Entropy is equal to " << -p_sum << endl;

}