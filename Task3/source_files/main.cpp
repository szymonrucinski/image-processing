#include <iostream>
#include <cstdlib>
#include "header_files/functions.h"
#include "header_files/CImg.h"

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
        else if ((strcmp(command, "--HMT") == 0))
        {
            HMT(img);
            display("image_processed.bmp");
        }
        else if ((strcmp(command, "--HMT_task") == 0))
        {
            HMT_task(img);
            display("image_processed.bmp");
        }
        else if ((strcmp(command, "--M2") == 0))
        {
            M2(img);
            display("image_edited2.bmp");
        }
        else error();
    }

    /*else if(argc == 4)
    {
        const char* command = argv[1];
        const char* img = argv[2];
        int option = atoi(argv[3]);
        if (checkFile(img)==0) return 0;
        else if ((strcmp(command, "--HMT_task") == 0))
        {
            HMT_task(img, option);
            display("image_processed.bmp");
        }
    }*/


    else if (argc == 6)
    {
        const char* command = argv[1];

        if ((strcmp(command, "--reggrow") == 0))
        {
            const char* img = argv[2];
            if (checkFile(img) == 0) return 0;
            int threshold = atof(argv[3]);
            if (threshold>255) threshold=255;
            int x = atoi(argv[4]);
            int y = atoi(argv[5]);

            if ((strcmp(command, "--reggrow") == 0))
            {reggrow(img,  threshold,  x,  y) ;}
            else error();

        }




    }
    else
    {
        error();
    }
    return 0;
}




















