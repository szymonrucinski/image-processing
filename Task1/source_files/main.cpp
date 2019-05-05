#include "../header_files/CImg.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>

using namespace cimg_library;
using namespace std;
//elementary
void help()
{
    cout << "There are 4 groups of commands:" << endl;
    cout << "1. With syntax: 'name of executable file' 'command' 'name of an image'"<< endl;
    cout << "Commands in this group:" << endl;
    cout << "       --negative " << endl;
    cout << "       --hflip     - It flips the image horizontally." << endl;
    cout << "       --vflip     - It flips the image vertically." << endl;
    cout << "       --dflip     - It flips the image diagonally." << endl;
    cout << "2. With syntax: 'name of executable file' 'command' 'name of an image' 'integer value'" << endl;
    cout << "Commands in this group:" << endl;
    cout << "       --brightness  positive values of arguments are used for increasing the brightness and negative for decreasing the brightness"<< endl;
    cout << "       --contrast    positive values of arguments are used for increasing the contrast and negative for decreasing the contrast"<< endl;
    cout << "       --median      applies the median filter for the picture (use only non-negative integer arguments to increase the radius)"<< endl;
    cout << "       --hmean       applies the harmonic mean filter for the picture (use only non-negative integer arguments to increase the radius)" << endl;
    cout << "3. With syntax: 'name of executable file' 'command' 'name of an image' 'float or integer value'" << endl;
    cout << "       --enlarge    enlarges the given image";
    cout << "       --shrink     shrinks the given image";
    cout << "4. With syntax: 'name of executable file' 'command' 'name of an original image' 'name of image with noise' 'filtered image" << endl;
    cout << "       --mse        counts the mean square error between the images." << endl;
    cout << "       --pmse       counts the peak mean square error between the images." << endl;
    cout << "       --snr        counts the signal to noise ratio between the images." << endl;
    cout << "       --psnr       counts the peak signal to noise ratio between the images." << endl;
    cout << "       --md         counts the maximum difference between the images." << endl;
}

void brightness(const char* img_name, int value) //brightness done
{
    CImg<unsigned char> image(img_name);
    int height = image.height();
    int width = image.width();
    int spectrum = image.spectrum();
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                if(image(x,y,z) + value > 255) image(x,y,z) = 255;
                else if (image(x,y,z) + value < 0) image(x,y,z) = 0;
                else image(x,y,z) += value;
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




void contrast(const char* img_name, float value) //contrast done
{
    CImg<unsigned char> image(img_name);                    //g(x)=(pixel_value−128)/value+128
    int height = image.height();
    int width = image.width();
    int spectrum = image.spectrum();
    if (value<0)
    { //contrast decrease
        value=(-value);
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                for(int z = 0; z < spectrum; z++)
                {
                    if(((image(x,y,z)-128)/value + 128)>255)
                    {
                        image(x,y,z)=255;
                    }
                    else if (((image(x,y,z)-128)/value + 128)<0)
                    {
                        image(x,y,z)=0;
                    }
                    else
                    {
                        image(x,y,z)=(image(x,y,z)-128)/value + 128;
                    }
                }
            }
        }
    }

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                if((value * (image(x,y,z)-128) + 128)>255)             //f(x)=user_input*(pixel_value−128)+128

                {
                    image(x,y,z)=255;
                }
                else if ((value * (image(x,y,z)-128) + 128)<0)
                {
                    image(x,y,z)=0;
                }
                else
                {
                    image(x,y,z)=value * (image(x,y,z)-128) + 128;
                }
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







void negative(const char* img_name)  //negative done
{
     CImg<unsigned char> image(img_name);
    int height = image.height();
    int width = image.width();
    int spectrum = image.spectrum();
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                image(x,y,z)=255-image(x,y,z);
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
//filters

void median(const char* img_name, int radius)  //median filter
{
     CImg<unsigned char> image(img_name);
    int height = image.height();
    int width = image.width();
    int spectrum = image.spectrum();
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                int arrSize=(radius*2+1)*(radius*2+1);
                int help=0;
                int *arr=new int[arrSize];
                for(int j = y-radius; j<=y+radius; j++)
                {

                    for(int i = x-radius; i<=x+radius; i++)
                    {
                        if((j<0)||(j>=height)||(i<0)||(i>=width))
                        {
                            arrSize--;
                        }
                        else
                        {
                            arr[help]=(int)image(i,j,z);
                            help++;
                        }
                    }
                }
                sort(arr, arr + arrSize);          //sorting_values
                image(x, y, z)=arr[arrSize/2];    //median_value
                delete[] arr;
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

void hmean(const char* img_name, int radius)
{
     CImg<unsigned char> image(img_name);
    int height = image.height();
    int width = image.width();
    int spectrum = image.spectrum();
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                int arrSize=(radius*2+1)*(radius*2+1);
                int help=0;
                int *arr=new int[arrSize];
                for(int j = y-radius; j<=y+radius; j++)
                {
                    for(int i = x-radius; i<=x+radius; i++)
                    {
                        if((j<0)||(j>=height)||(i<0)||(i>=width))
                        {
                            arrSize--;
                        }
                        else
                        {
                            arr[help]=(int)image(i,j,z);
                            help++;
                        }
                    }
                }
                double denominator=0;
                for(int i=0; i<arrSize; i++)
                {
                    if(arr[i]==0)
                    {
                        denominator+=1;
                        continue;
                    }
                    denominator+=(1/(double)arr[i]);
                }
                double mean=arrSize/denominator;
                image(x,y,z)=(int)(mean);
                delete[] arr;
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
//flips
void hflip(const char* img_name)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width/2 ; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				int temp = image(x, y, z);
				image(x, y, z) = image(width - x-1 , y, z);
				image(width - x-1 , y, z) = temp;
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

void vflip(const char* img_name)
{
	CImg<unsigned char> image(img_name), visu(500, 400, 1, 3, 0);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();

	for (int y = 0; y < height/2; y++)
	{
		for (int x = 0; x < width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				int temp = image(x, y, z);
				image(x, y, z) = image(x, height - y-1, z);
				image(x, height - y-1 , z) = temp;
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

void dflip(const char* img_name)
{
	CImg<unsigned char> image(img_name), visu(500, 400, 1, 3, 0);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width/2; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				int temp = image(x, y, z);
				image(x, y, z) = image(width - x - 1, height - y -1, z);
				image(width - x - 1, height - y - 1, z) = temp;
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

void enlarge(const char* img_name, double value)
{
	CImg<unsigned char> image(img_name);
	int height = image.height();
	int width = image.width();
	int spectrum = image.spectrum();
	int new_height = floor(height*value);
	int new_width = floor(width*value);
	CImg<unsigned char>image_enlarge(new_height, new_width, 1, 3); //value^2 times bigger image

	for (int y = 0; y < new_height; y++)
	{
        for (int x = 0; x < new_width; x++)
		{
			for (int z = 0; z < spectrum; z++)
			{
				{

                    image_enlarge(x,y,z) = image(floor(x/value), floor(y/value), z); //assigning every row of pixel n times.

                }
			}
        }
    }
    CImgDisplay main_disp(image_enlarge);
    while (!main_disp.is_closed())
	{
		main_disp.wait();
	}
	image_enlarge.save("newOne.bmp");
}

void shrink(const char* img_name, double value)
{
    CImg<unsigned char> image(img_name);
    int height = round(image.height()/value);
    int width = round(image.width()/value);
    int spectrum = image.spectrum();
    CImg<unsigned char>image_shrink(height, width , 1, 3);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            for (int z = 0; z<spectrum; z++)
            {
                image_shrink(x,y,z) = image(x*value, y*value, z); //assigning every nth pixel from original image to the new shrinked image

            }
        }
    }
    CImgDisplay main_disp(image_shrink);
    while (!main_disp.is_closed())
    {
        main_disp.wait();
    }
    image_shrink.save("newOne.bmp");
}
//ratios
void mse(const char* img1_name, const char* img2_name, const char* img3_name)
{
	CImg<unsigned char> image_original(img1_name);
	CImg<unsigned char> image_wnoise(img2_name);
	CImg<unsigned char> image_filtered(img3_name);
	int height = image_original.height();
	int width = image_original.width();
	int spectrum = image_original.spectrum();
    int MSE1_help=0;
	int MSE2_help=0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
		    for( int z=0; z < spectrum; z++)
            {
                MSE1_help+=pow(image_original(x,y,z)-image_wnoise(x,y,z), 2);
                MSE2_help+=pow(image_original(x,y,z)-image_filtered(x,y,z), 2);
            }
		}
	}
	int MSE1 = ((MSE1_help) /(height*width));
	int MSE2 = ((MSE2_help) /(height*width));
	cout << "The mean square error between the original image and the image with noise is: ";
    cout << MSE1 << endl;
    cout << "The mean square error between the original image and the filtered image is: ";
    cout << MSE2 << endl;
    cout << "The mean square error between the image with noise and the filtered image is: ";
    cout << abs(MSE1-MSE2) << endl;
}

void pmse(const char* img1_name, const char* img2_name, const char* img3_name)
{
	CImg<unsigned char> image_original(img1_name);
	CImg<unsigned char> image_wnoise(img2_name);
	CImg<unsigned char> image_filtered(img3_name);
	int height = image_original.height();
	int width = image_original.width();
	int spectrum = image_original.spectrum();
	int max = image_original(0,0,0);
    double MSE1_help=0;
    double MSE2_help=0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
            for(int z = 0; z < spectrum; z++)
            {
                if(max < image_original(x,y,z)) max = image_original(x,y,z);
                MSE1_help+=pow(image_original(x,y,z)-image_wnoise(x,y,z), 2);
                MSE2_help+=pow(image_original(x,y,z)-image_filtered(x,y,z), 2);
            }
		}
	}
	double pmse1 = ((MSE1_help) / ((height*width)*pow(max, 2)));
	double pmse2 = ((MSE2_help) / ((height*width)*pow(max, 2)));
	cout << "The peak mean square error between the original image and the image with noise is: ";
    cout << pmse1 << endl;
    cout << "The peak mean square error between the original image and the filtered image is: ";
    cout << pmse2 << endl;
    cout << "The peak mean square error between the image with noise and the filtered image is: ";
    cout << abs(pmse1-pmse2) << endl;
}

void snr(const char* img1_name, const char* img2_name, const char* img3_name)
{
    CImg<unsigned char> image_original(img1_name);
	CImg<unsigned char> image_wnoise(img2_name);
	CImg<unsigned char> image_filtered(img3_name);
	int height = image_original.height();
	int width = image_original.width();
	int spectrum = image_original.spectrum();
	double squaresum=0;
	double msesum1=0;
	double msesum2=0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
            for(int z = 0; z < spectrum; z++)
            {
                squaresum+= pow(image_original(x,y,z),2);
                msesum1+=pow(image_original(x,y,z)-image_wnoise(x,y,z), 2);
                msesum2+=pow(image_original(x,y,z)-image_filtered(x,y,z), 2);
            }
		}
	}
	float snr1 = 10*log10(squaresum/msesum1);
	float snr2 = 10*log10(squaresum/msesum2);
	cout << "The signal to noise ratio between the original image and the image with noise is: ";
    cout << snr1 << "dB" << endl;
    cout << "The signal to noise ratio between the original image and the filtered image is: ";
    cout << snr2 <<"dB" << endl;
    cout << "The signal to noise ratio between the image with noise and the filtered image is: ";
    cout << abs(snr1-snr2) << "dB" <<endl;
}

void psnr(const char* img1_name,const char* img2_name, const char* img3_name)
{
    CImg<unsigned char> image_original(img1_name);
    CImg<unsigned char> image_wnoise(img2_name);
    CImg<unsigned char> image_filtered(img3_name);
    int height = image_original.height();
    int width = image_original.width();
    int spectrum = image_original.spectrum();
    int MSE1, MSE2;
    int max = image_original(0, 0, 0);
    float PSNR1, PSNR2;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            for(int z = 0; z < spectrum; z++)
            {
                if(max < image_original(x,y,z)) max = image_original(x,y,z);
                MSE1+=pow(image_original(x,y,z)-image_wnoise(x,y,z), 2);
                MSE2+=pow(image_original(x,y,z)-image_filtered(x,y,z), 2);
            }
        }
    }
    PSNR1 = 10 * log10( (height * width * pow(max, 2)/ MSE1));
    PSNR2 = 10 * log10( (height * width * pow(max, 2)/ MSE2));
    cout << "The peak signal to noise ratio between the original image and the image with noise is: ";
    cout << PSNR1 << "dB" << endl;
    cout << "The peak signal to noise ratio between the original image and the filtered image is: ";
    cout << PSNR2 << "dB" << endl;
    cout << "The peak signal to noise ratio between the image with noise and the filtered image is: ";
    cout << abs(PSNR1-PSNR2) << "dB" << endl;
}

void md(const char* img1_name, const char* img2_name, const char* img3_name)
{
    CImg<unsigned char> image_original(img1_name);
	CImg<unsigned char> image_wnoise(img2_name);
	CImg<unsigned char> image_filtered(img3_name);
	int height = image_original.height();
	int width = image_original.width();
	int spectrum = image_original.spectrum();
	int max1 = 0;
	int max2 = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
            for(int z = 0; z < spectrum; z++)
            {
                if(max1 < abs(image_original(x,y,z)-image_wnoise(x,y,z))) max1 = abs(image_original(x,y,z)-image_wnoise(x,y,z));
                if(max2 < abs(image_original(x,y,z)-image_filtered(x,y,z))) max2 = abs(image_original(x,y,z)-image_filtered(x,y,z));
            }
		}
	}
	cout << "The maximum difference between the original image and the image with noise is: ";
    cout << max1 << endl;
    cout << "The maximum difference between the original image and the filtered image is: ";
    cout << max2 << endl;
    cout << "The maximum difference between the image with noise and the filtered image is: ";
    cout << abs(max1-max2) << endl;
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

void error()
{
    cout<< "Ooops! Something went wrong! Try to write '--help' as a parameter to get help." << endl;
}

int main(int argc, char* argv[])
{
   if(argc<2)
   {
       cout << "There is no argument!" << endl;
       return 0;
   }
   else if(argc==2)
   {
       if(strcmp(argv[1], "--help") == 0)
       {
           help();
           return 0;
       }
        else
        {
            cout<< "There is not enough arguments!" << endl;
            return 0;
        }
   }
   else if(argc==3)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       if (checkFile(img)==0) return 0;
       if ((strcmp(command, "--negative") == 0)) negative(img);
       else if ((strcmp(command, "--hflip") == 0)) hflip(img);
       else if ((strcmp(command, "--vflip") == 0)) vflip(img);
       else if ((strcmp(command, "--dflip") == 0)) dflip(img);
       else error();
   }
   else if (argc==4)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       if(checkFile(img)==0) return 0;
       if((strcmp(command, "--brightness") == 0)||(strcmp(command, "--contrast") == 0)||(strcmp(command, "--median") == 0)||(strcmp(command, "--hmean") == 0))
       {
           int value = atoi(argv[3]);
            if ((strcmp(command, "--brightness") == 0)) brightness(img, value);
            else if ((strcmp(command, "--contrast") == 0)) contrast(img, value);
            else if ((strcmp(command, "--median") == 0)) median(img, value);
            else if ((strcmp(command, "--hmean") == 0)) hmean(img, value);
            else error();
       }
       else if((strcmp(command, "--enlarge") == 0)||(strcmp(command, "--shrink") == 0))
       {
           float value = atof(argv[3]);
           if ((strcmp(command, "--enlarge") == 0)) enlarge(img, value);
           else if ((strcmp(command, "--shrink") == 0)) shrink(img, value);
           else error();
       }
   }
   else if (argc==5)
   {
        const char* command = argv[1];
        const char* img1 = argv[2];
        const char* img2 = argv [3];
        const char* img3 = argv [4];
        if(checkFile(img1)==0) return 0;
        if(checkFile(img2)==0) return 0;
        if(checkFile(img3)==0) return 0;
        if ((strcmp(command, "--psnr") == 0)) psnr(img1, img2, img3);
        else if ((strcmp(command, "--pmse") == 0)) pmse(img1, img2, img3);
        else if ((strcmp(command, "--mse") == 0)) mse(img1, img2, img3);
        else if ((strcmp(command, "--snr") == 0)) snr(img1, img2, img3);
        else if ((strcmp(command, "--md") == 0)) md(img1, img2, img3);
        else error();
   }
   else
   {
       error();
   }
   return 0;
}
