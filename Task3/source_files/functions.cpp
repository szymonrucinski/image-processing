#include <iostream>
#include <stack>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <header_files/RegionGrowing.h>

using namespace std;
using namespace cimg_library;
int mask1[3][3] = {{1,1,1},{1,0,0},{1,1,1}};
int mask2[3][3] = {{1,1,1},{1,0,1},{1,0,1}};
int mask3[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

int HMTmasks[4][3][3] = {
        {
                {0,   5,   5},
                {0,   255, 5},
                {0,   5,   5}
        },
        {
                {0,   0,   0},
                {5, 255,   5},
                {5,   5,   5}
        },
        {
                {5,   5,   0},
                {5,   255, 0},
                {5,   5,   0}
        },
        {
                {5,   5,   5},
                {5,   255, 5},
                {0,   0,   0}
        },

};

void help()
{
    cout << "There are 4 groups of commands:" << endl;
    cout << "1. With syntax: 'name of executable file' 'command' 'name of an image'"<< endl;
    cout << "Commands in this group:" << endl;

    cout << "2. With syntax: 'name of executable file' 'command' 'name of an image' 'integer value'" << endl;
    cout << "Commands in this group:" << endl;

    cout << "3. With syntax: 'name of executable file' 'command' 'name of an image' 'float or integer value'" << endl;

    cout << "4. With syntax: 'name of executable file' 'command' 'name of an original image' 'name of image with noise' 'filtered image" << endl;

    cout << "5. With syntax: 'name of executable file' 'command' 'name of an original image' 'integer value' 'integer value" << endl;

}

void display(const char* img_name)
{
    CImg<unsigned char> image(img_name);
    CImgDisplay main_disp(image, "image.bmp");
    while (!main_disp.is_closed())
    {
        main_disp.wait();
    }
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

void erosion(const char* img_name)
{
    int k=0;
    CImg<unsigned char> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<unsigned char> image_eroded(height, width);
    int mask[3][3] = {{0,0,0},{0,0,0},{0,0,0}}; //1 for white, 0 for black



   for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            //if pixels are near the edge do not change the values
            if(x==0||x==width-1||y==0||y==height-1)
            {
                //replace pixels from original to eroded
                image_eroded(x, y) = image_original(x, y);
            }
            else
            {
                //if given pixel is black , same as central mask point
                if(image_original(x, y)==mask[1][1])
                {

                    //checking 3X3 pixel mask and neighbourhood
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {

                            //if at least one pixel is white/ not black then
                            //we copy them white in the second image
                            if(image_original(i, j)!=mask[1][1])
                            {
                                image_eroded(x, y) = 255;
                                //we finish the loops

                                i = x+2;
                                j = y+2;


                            }
                        }
                        //if loop was finished properly all pixels from the neighbourhood were black, cental pixel is black
                        if(j == y+1){ image_eroded(x, y) = image_original(x, y);}

                    }
                }
                //if the minimum requirement is not met then just copy pixel from original image
                else image_eroded(x, y) = image_original(x, y);

            }
        }
    }
    image_eroded.save("image_eroded.bmp");
}


void dilation(const char* img_name)
{
    CImg<unsigned char> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<unsigned char> image_dilated(height, width);
    int mask[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            //if pixels are near the edge do not change the values
            if(x==0||x==width-1||y==0||y==height-1)
            {

                //replace pixels from original to dilated
                image_dilated(x, y) = image_original(x, y);
            }
            else
            {
                //if given pixel is black , same as central mask point
                if(image_original(x, y)==255 * mask[1][1])
                {
                    //checking 3X3 pixel mask and neighbourhood
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {
                           // if any pixel from 3x3 neighbourhood is equal to black , new dilated_pixel becomes blacks
                            if(mask[a][b] == mask[1][1]) image_dilated(i, j) = 255 * mask[a][b];
                            cout<<i<<"::"<<j<<endl;

                        }
                    }

                    cout<<"FINITO"<<endl;
                }
                else image_dilated(x, y) = image_original(x, y);
            }
        }
    }
    image_dilated.save("image_dilated.bmp");
}

void dilation_task(const char* img_name)
{
    CImg<int> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<int> image_complement(width, height);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            image_complement(x, y) = 255 - image_original(x, y);
        }
    }

    CImg<int> image_dilated(height, width);
    int mask[3][3] = {{0,1,1},{0,1,1},{0,1,1}};
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(x==0||x==width-1||y==0||y==height-1)
            {
                image_dilated(x, y) = image_complement(x, y);
            }
            else
            {
                if(image_complement(x, y)==255 * mask[1][1])
                {
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {
                            if(mask[a][b] == mask[1][1]) image_dilated(i, j) = 255 * mask[a][b];
                        }
                    }
                }
                else image_dilated(x, y) = image_complement(x, y);
            }
        }
    }
    image_dilated.save("image_dilated.bmp");
}

void opening(const char* img_name)
{
    erosion(img_name);
    dilation("image_eroded.bmp");
}

void closing(const char* img_name)
{
    dilation(img_name);
    erosion("image_dilated.bmp");
}

void HMT(const char *name, int mask)
{

        CImg<int> image(name);
        CImg<double> result = image;

        for (int i = 1; i < image.width() - 1; i++)
        {
            for (int j = 1; j < image.height() - 1; j++)
            {
                for (int k = 0; k < image.spectrum(); k++)
                {
                    bool test = true;
                    for (int y = -1; y <= 1; y++)
                    {
                        for (int x = -1; x <= 1; x++)
                        {
                            //checking if part of the mask is active 0 or 255
                            if (HMTmasks[mask][x + 1][y + 1] != 5)
                            {

                                //if at least one of values of the image that masked was placed on is different
                                if (image(i + x, j + y, k) != HMTmasks[mask][x + 1][y + 1])
                                    test = false;

                            }
                        }
                    }

                    if (test)
                        result(i, j, k) = 0;
                    else
                        result(i, j, k) = 255;
                }
            }
        }


     result.save("HMT.bmp");
}



int check_coordinates(int fun_width, int fun_height, int fun_x, int fun_y)
{
    if((fun_x >= 0 && fun_x < fun_width) && (fun_y >= 0 && fun_y < fun_height))
        return 1;
    else
    {
        cout << "You chose coordinates out of range. Try again." << endl;
        return 0;
    }
}

CImg<int>& dilation_to_fun(CImg<int> image_original, int mask[3][3], int width, int height)
{
    CImg<int>* image_dilated = new CImg<int>(width, height);
    if(mask[1][1] == 0)
    {
        for(int y = 0; y < height; y++)
        {
            for(int x = 0; x < width; x++)
            {
                (*image_dilated)(x, y) = 255;
            }
        }
    }
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(x==0||x==width-1||y==0||y==height-1)
            {
                (*image_dilated)(x, y) = image_original(x, y);
            }
            else
            {
                if(image_original(x, y)==255 * mask[1][1])
                {
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {
                            if(mask[a][b] == mask[1][1])
                            {
                                (*image_dilated)(i, j) = 255 * mask[a][b];
                            }
                        }
                    }
                }
                else if((*image_dilated)(x, y) != 255 * mask[1][1])
                {
                    (*image_dilated)(x, y) = image_original(x, y);
                }
            }
        }
    }
    return *image_dilated;
    delete image_dilated;
}

CImg<int>& complement(const char* img_name)
{
    CImg<int> original(img_name);
    int height = original.height();
    int width = original.width();
    CImg<int> *edited = new CImg<int>(width, height);

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(original(x, y) == 0) (*edited)(x, y) = 255;
            else (*edited)(x, y) = 0;
        }
    }
    return *edited;
    delete edited;
}



void M2(const char* img_name)
{
    CImg<int> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<int> image_edited1(width, height);
    CImg<int> image_edited2(width, height);
    CImg<int> image_negative(width, height);
    int x_coordinate, y_coordinate;
    int mask_used[3][3] = {{0,1,0},{1,1,1},{0,1,0}};

    do
    {
        cout << "Choose an x coordinate of your image: " ;
        cin >>  x_coordinate;
        cout << "Choose a y coordinate of your image: " ;
        cin >> y_coordinate;
    }while(check_coordinates(width, height, x_coordinate, y_coordinate) != 1);



    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            image_edited1(x, y) = 0;
            image_negative(x, y) = 255 - image_original(x, y);
        }
    }

    image_edited1(x_coordinate, y_coordinate) = image_negative(x_coordinate, y_coordinate);
    bool to_break = false;
    do
    {
        image_edited2 = dilation_to_fun(image_edited1, mask_used, width, height);
        for(int y = 1; y < height-1; y++)
        {
            for(int x = 1; x < width-1; x++)
            {
                if(image_edited2(x,y) == 255*mask_used[1][1] && image_negative(x,y) != 255*mask_used[1][1])
                {
                    image_edited2(x, y) = 255 - (255*mask_used[1][1]);
                }
            }
        }

        for(int y = 1; y < height-1; y++)
        {
            for(int x = 1; x < width-1; x++)
            {
                if(image_edited2(x, y) != image_edited1(x, y))
                {
                    y = height - 1;
                    x = width - 1;
                }
                else if(y == height-2 && x == width-2 && image_edited2(x, y) == image_edited1(x, y))
                {
                    to_break = true;
                }
            }
        }

    image_edited1 = image_edited2;

    }while(to_break == false);
    int sumXA;
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            sumXA = image_original(x, y) + image_edited1(x, y);
            if(sumXA >= 255) image_edited2(x, y) = 255;
            else image_edited2(x, y) = 0;
        }
    }
    image_edited2.save("image_edited2.bmp");

}



//////////////////////////////////////////
            //REGION GROWING

Pixel::Pixel(){};
Pixel::Pixel(int x, int y)
{
    this -> x = x;
    this -> y = y;
}


CImg<int>& applySegmentation(CImg<int> &original, int x, int y, int threshold){
    CImg<int>* edited = new CImg<int>(original.width(), original.height(), 1, 3, 0);
    //CImg<int>* edited = &original;





    //Create a 2d array of pointers
   bool** alreadyInRegion;
    alreadyInRegion = new bool*[original.width()];
    for (int i = 0; i < original.width(); i++)
        alreadyInRegion[i] = new bool[original.height()];


    //creating array of 'false' booleans
    for (int x = 0; x < original.width(); x++)
        for (int y = 0; y < original.height(); y++)
            alreadyInRegion[x][y] = false;

        //Create a seed and add it to region
        
    Pixel seed(x, y);
    alreadyInRegion[x][y] = true;

    //Recursive Segmenataion in given directions
    segmentationRecursive(original, *edited, x + 1, y, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, *edited, x - 1, y, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, *edited, x, y + 1, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, *edited, x, y - 1, threshold, alreadyInRegion, seed);

    return *edited;
}
void segmentationRecursive(CImg<int> &original, CImg<int> &edited, int x, int y, int threshold, bool** alreadyInRegion, Pixel seed){
   //Check if pixel won't pass the boarder
    if (x < 0 || x > original.width() - 1 || y < 0 || y > original.height() - 1) return;
    if (alreadyInRegion[x][y]) return;
    //If it doesn't meet the Euclidean condition
    if (!euclideanThreshold(original, threshold, x, y, seed.x, seed.y)) return;

    //set pixels to white
    edited(x, y, 0, 0) = 255;
    edited(x, y, 0, 1) = 255;
    edited(x, y, 0, 2) = 255;
    alreadyInRegion[x][y] = true;

//Run Recursion One again
    segmentationRecursive(original, edited, x + 1, y, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, edited, x - 1, y, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, edited, x, y + 1, threshold, alreadyInRegion, seed);
    segmentationRecursive(original, edited, x, y - 1, threshold, alreadyInRegion, seed);
}

bool euclideanThreshold(CImg<int> &original, int threshold, int x1, int y1, int x2, int y2){
    //calculate pixel value based of Eucdlidean distance with respect to seed;
    int distance = sqrt(
            (original(x2, y2, 0, 0) - original(x1, y1, 0, 0)) * (original(x2, y2, 0, 0) - original(x1, y1, 0, 0)) +
            (original(x2, y2, 0, 1) - original(x1, y1, 0, 1)) * (original(x2, y2, 0, 1) - original(x1, y1, 0, 1)) +
            (original(x2, y2, 0, 2) - original(x1, y1, 0, 2)) * (original(x2, y2, 0, 2) - original(x1, y1, 0, 2))
    );
    return distance < threshold;
}
