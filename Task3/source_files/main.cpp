#include <iostream>
#include <cstdlib>

#include "header_files/CImg.h"
#include "header_files/RegionGrowing.h"
#include "header_files/functions.h"


using namespace std;
using namespace cimg_library;
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
        if ((strcmp(command, "--dilation") == 0))
        {
            dilation(img);
            display("image_dilated.bmp");
        }
        else if ((strcmp(command, "--erosion") == 0))
        {
            erosion(img);
            display("image_eroded.bmp");
        }
        else if ((strcmp(command, "--opening") == 0))
        {
            opening(img);
            display("image_dilated.bmp");
        }
        else if ((strcmp(command, "--closing") == 0))
        {
            closing(img);
            display("image_eroded.bmp");
        }

        else if ((strcmp(command, "--M2") == 0))
        {
            M2(img);
            display("image_edited2.bmp");
        }
        else error();
    }

    else if(argc==4) {
        const char *command = argv[1];
        const char *img = argv[2];
        int mask = atof(argv[3]);

         if ((strcmp(command, "--HMT") == 0))
        {
            HMT(img,mask);
        }
    }



        else if (argc == 6)
    {
        const char* command = argv[1];

        if ((strcmp(command, "--reggrow") == 0))
        {

            const char* img = argv[2];
            if (checkFile(img) == 0) return 0;
            int threshold = atof(argv[5]);
            if (threshold>255) threshold=255;
            int x = atoi(argv[3]);
            int y = atoi(argv[4]);
            CImg<int>image1(img);
            CImg<int>image2;


            if ((strcmp(command, "--reggrow") == 0))
<<<<<<< HEAD
            {
                applySegmentation(img, x,  y, threshold);
=======
            {image2=applySegmentation(image1,x,y,threshold);
                cout<<"working"<<endl;
                image2.save("segmented.bmp");

>>>>>>> dev
            }
            else error();

        }




    }
    else
    {
        error();
    }
    return 0;
}




















