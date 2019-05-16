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
   else if(argc == 3)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       if (checkFile(img)==0) return 0;
       else if(strcmp(command, "--slow_fourier") == 0)
       {
           slow_fourier(img);
       }
       else if(strcmp(command, "--fast_fourier") == 0)
       {
           fast_fourier(img);
       }
   }
   else if(argc == 4)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       int attribute = atoi(argv[3]);
       if (checkFile(img)==0) return 0;
        else if(strcmp(command, "--low_pass_filter") == 0)
       {
           pass_filter(img, attribute, 0);
       }
       else if(strcmp(command, "--high_pass_filter") == 0)
       {
           pass_filter(img, attribute, 1);
       }

    else if(strcmp(command, "--high_pass_filter_edges") == 0)
       {
           const char* mask = argv[3];
           //high_pass_filter_edges(img,mask);
       }
   }
   else if(argc == 5)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       int attribute1 = atoi(argv[3]);
       int attribute2 = atoi(argv[4]);
       if (checkFile(img)==0) return 0;
       else if(strcmp(command, "--band_pass_filter") == 0)
       {
           band_filter(img, attribute1, attribute2, 0);
       }
       else if(strcmp(command, "--band_cut_filter") == 0)
       {
           band_filter(img, attribute1, attribute2, 1);
       }
       else if(strcmp(command, "--phase_modifying_filter") == 0)
       {
           phase_modifying_filter(img, attribute1, attribute2);
       }
   }

   else if(argc == 6)
   {
       const char* command = argv[1];
       const char* img = argv[2];
       int attribute1 = atoi(argv[3]);
       int attribute2 = atoi(argv[4]);
       int attribute3 = atoi(argv[5]);
       if (checkFile(img)==0) return 0;
       else if(strcmp(command, "--F5") == 0)
       {
          F5(img, attribute1, attribute2, attribute3);
       }

   }

    return 0;
}
