#include <iostream>
#include <stack>
#include <cstdlib>
#include "header_files/functions.h"
#include "header_files/CImg.h"
#include <algorithm>
#include <fstream>
#include <cmath>

using namespace std;
using namespace cimg_library;

int mask1[3][3] = {{1,1,1},{1,0,0},{1,1,1}};
int mask2[3][3] = {{1,1,1},{1,0,1},{1,0,1}};
int mask3[3][3] = {{1,1,1},{1,1,1},{1,1,1}};
int mask4[3][3] = {{1,0,1},{0,0,0},{1,0,1}};
int mask41[3][3] = {{0,1,0},{1,1,1},{0,1,0}};
int mask5[3][3] = {{1,1,1},{1,0,0},{1,0,1}};
int mask6[3][3] = {{1,1,1},{1,1,0},{1,0,1}};
int mask7[3][3] = {{1,1,1},{0,0,0},{1,1,1}};
int mask8[3][3] = {{1,1,1},{0,1,0},{1,1,1}};
int mask9[3][3] = {{1,1,1},{0,0,1},{0,1,1}};
int mask10[3][3] = {{1,0,0},{1,0,1},{1,1,1}};
int mask11[4][3][3] = {{{0,2,2},{0,1,2},{0,2,2}},{{0,0,0},{2,1,2},{2,2,2}},
                       {{2,2,0},{2,1,0},{2,2,0}},{{2,2,2},{2,1,2},{0,0,0}}}; // 0 for black, 1 for white, 2 for inactive
int maskB1[3][3] = {{1,0,0},{1,0,0},{1,0,0}};
int maskB2[3][3] = {{0,1,1},{0,1,1},{0,1,1}};

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
            //if coordinate are near the edge
            if(x==0||x==width-1||y==0||y==height-1)
            {
                //replace pixels from original to eroded
                image_eroded(x, y) = image_original(x, y);
            }
            else
            {
                //if given pixel is black and corresponds to given mask element
                //check if we have got a pm,sm,nm
                if(image_original(x, y)==mask[1][1])
                {
                    //cout<<"black"<<endl;

                    //checking 3X3 pixel mask and neighbourhood
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {

                            //if pixel is white
                            //partial match, no match that means that we turn black values into white
                            if(image_original(i, j)!=mask[1][1])
                            {
                               // cout<<"white"<<endl;
                                image_eroded(x, y) = 255;
                                i = x+2;
                                j = y+2;
                                //we finish the loops


                            }
                        }
                        //if pixels are black
                        if(j == y+1){ image_eroded(x, y) = image_original(x, y);k++;cout<<k<<endl;}

                    }
                }
                //if pixels are white just copy them
                else image_eroded(x, y) = image_original(x, y);

            }
        }
    }
    image_eroded.save("image_eroded.bmp");
    cout<<"Murzyn"<<endl;

}

void erosion_task(const char* img_name, int option)
{
    int mask[3][3];
    if(option == 1)
    {
        for(int b = 0; b < 3; b++)
        {
            for(int a = 0; a < 3; a++)
            {
                mask[a][b] = maskB1[b][a];
            }
        }
    }
    else if(option == 2)
    {
        for(int b = 0; b < 3; b++)
        {
            for(int a = 0; a < 3; a++)
            {
                mask[a][b] = maskB2[b][a];
            }
        }
    }
    CImg<int> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<int> image_eroded(height, width); //1 for white, 0 for black
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(x==0||x==width-1||y==0||y==height-1)
            {
                image_eroded(x, y) = image_original(x, y);
            }
            else
            {
                if(image_original(x, y)==mask[1][1]*255)
                {
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {
                            if(mask[a][b]==0&&image_original(i, j)!=mask[1][1]*255)
                            {
                                image_eroded(x, y) = 255 * abs(1-mask[1][1]);
                                i = x+2;
                                j = y+2;
                            }
                        }
                        if(j == y+1) image_eroded(x, y) = image_original(x, y);
                    }
                }
                else image_eroded(x, y) = image_original(x, y);
            }
        }
    }
    if(option == 1) image_eroded.save("image_eroded1.bmp");
    else if (option == 2) image_eroded.save("image_eroded2.bmp");
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
            if(x==0||x==width-1||y==0||y==height-1)
            {
                image_dilated(x, y) = image_original(x, y);
            }
            else
            {
                if(image_original(x, y)==255 * mask[1][1])
                {
                    for(int j = y-1, b = 0; j <= y+1; j++, b++)
                    {
                        for(int i = x-1, a = 0; i <= x+1; i++, a++)
                        {
                            if(mask[a][b] == mask[1][1]) image_dilated(i, j) = 255 * mask[a][b];
                        }
                    }
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

void HMT(const char* img_name)
{
    CImg<unsigned char> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<unsigned char> image_processed(height, width);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(x==0||x==width-1||y==0||y==height-1)
            {
                image_processed(x, y) = image_original(x, y);
            }
            else
            {
                if(mask11[0][1][1]*255 == image_original(x, y)) //because every mask has the same color of middle pixel
                {
                    for(int masknr = 0; masknr < 4; masknr++)
                    {
                        for(int j = y-1, b = 0; j <= y+1; j++, b++)
                        {
                            for(int i = x-1, a = 0; i <= x+1; i++, a++)
                            {
                                if(mask11[masknr][a][b] == 2) continue;
                                if(mask11[masknr][a][b] * 255 != image_original(i, j))
                                {
                                    if(masknr == 3)
                                    {
                                        image_processed(x, y) = abs(1 - mask11[0][1][1]) * 255;
                                    }
                                    i = x+2;
                                    j = y+2;
                                }
                                if(a == 2&& b == 2 && mask11[masknr][a][b] * 255 == image_original(i, j))
                                {
                                    image_processed(x, y) = mask11[0][1][1] * 255;
                                    masknr = 4;
                                }
                            }
                        }
                    }
                }
                else image_processed(x, y) = image_original(x, y);
            }
        }
    }
    image_processed.save("image_processed.bmp");
}

void HMT_task(const char* img_name)
{

    CImg<int> image_original(img_name);
    int height = image_original.height();
    int width = image_original.width();
    CImg<int> image_eroded1("image_eroded1.bmp");
    CImg<int> image_eroded2("image_eroded2.bmp");
    CImg<int> image_processed(width, height);
    CImg<int> image_complement(width, height);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            image_complement(x, y) = 255 - image_original(x, y);
        }
    }
    image_complement.save("image_complement.bmp");
    erosion_task(img_name, 1);
    erosion_task("image_complement.bmp", 2);
    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            if(image_eroded1(x, y) == image_eroded2(x, y))
            {
                image_processed(x, y) = 255;
            }
            else
            {
                image_processed(x, y) = 0;
            }

        }
    }
    image_processed.save("image_processed.bmp");
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

void reggrow(const char* img_name, int threshold, int x, int y)
{

    CImg<unsigned char> image(img_name);  //our image


    int width = image.width();
    int height = image.height();
    int spectrum = image.spectrum();

    CImg<unsigned char> result (width, height, 1, spectrum, 0);


    //declaration of a 2 dimensional array
    //all of the indexes store XY picture-coordinates
    bool **checker = 0;
    checker = new bool *[width];

    //allocate all pointers in 2d array
    for (int i = 0; i < width; i++)
        checker[i] = new bool[height];

    for (int i = 0; i<width; i++)
        for (int j = 0;j<height; j++) {
            checker[i][j] = false;
        }

    //new coordinates
    int new_x = 0;
    int new_y = 0;

    //stack structure holding pair of coordinates of matching pixel
    stack<pair <int, int> >pixels;
    pixels.push(make_pair(x, y));

    while (!pixels.empty()) {

        pair<int, int> currentPair = pixels.top();
        new_x = currentPair.first;
        new_y = currentPair.second;
        pixels.pop();

        //RIGHT SIDE
        if (new_y > 0 && new_y < height - 1 && new_x > 0 && new_x<width - 1) {
            if (checker[new_x + 1][new_y] == false) {

                //checking wether it does not exceed the threshold
                if (abs((image(x, y) - image(new_x + 1, new_y))) < threshold) {

                    pixels.push(make_pair(new_x + 1, new_y));

                    for (int z = 0; z < spectrum; z++)
                        //assign one (white) color
                        image(new_x + 1, new_y, 0, z) = 255;

                    //assign in check array that pixel(x,y) satisfies condistions
                    checker[new_x + 1][new_y] = true;
                }
                else
                    checker[new_x + 1][new_y] = true;
            }

            //LEFT SIDE
            if (checker[new_x - 1][new_y] == false) {
                if (abs((image(x, y) - image(new_x - 1, new_y))) < threshold) {
                    pixels.push(make_pair(new_x - 1, new_y));

                    for (int k = 0; k<3; k++)
                        image(new_x - 1, new_y, 0, k) = 255;

                    checker[new_x - 1][new_y] = true;
                }
                else
                    checker[new_x - 1][new_y] = true;
            }

            //ABOVE
            if (checker[new_x][new_y + 1] == false) {
                if (abs(image(x, y) - image(new_x, new_y + 1)) < threshold) {
                    pixels.push(make_pair(new_x, new_y + 1));

                    for (int k = 0; k<3; k++)
                        image(new_x, new_y + 1, 0, k) = 255;

                    checker[new_x][new_y + 1] = true;
                }
                else
                    checker[new_x][new_y + 1] = true;
            }

            //BELOW
            if (checker[new_x][new_y - 1] == false) {
                if (abs((image(x, y) - image(new_x, new_y - 1))) < threshold) {
                    pixels.push(make_pair(new_x, new_y - 1));

                    for (int k = 0; k<3; k++)
                        image(new_x, new_y - 1, 0, k) = 255;

                    checker[new_x][new_y - 1] = true;
                }
                else
                    checker[new_x][new_y - 1] = true;
            }
        }
    }

    //free memory after dynamic allocation
    for (int i = 0; i < width; i++) delete[] checker[i];
    delete[] checker;



    CImgDisplay main_disp(image, img_name);
    while (!main_disp.is_closed())
    {
        main_disp.wait();
    }
    image.save("newOne.bmp");



}
