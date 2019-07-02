#include <iostream>
#include <cstdlib>
#include "header_files/functions.h"
#include "header_files/CImg.h"

using namespace std;
using namespace cimg_library;




void save(CImg<unsigned char> image, const char *name, const char *suffix)
{
    char result[25];
    strcpy(result, name);
    int size = 0;
    while (result[size] != '\0') size++;
    result[size - 4] = '\0';
    strcat(result, suffix);
    image.save(result);
}


int main(int argc, char *argv[])
{

    char *name;
    string command;
    name = argv[2];
    command = argv[1];

        cimg::exception_mode(0);
        try
        {


             if (command == "--dft")
            {
                clock_t start, end;
                start = clock();
                save(dft(name), name, "_dft.bmp");
                end = clock();
                double duration_sec = double(end - start) / CLOCKS_PER_SEC;
                cout << "The operation took " << duration_sec << " seconds.";
            }
            else if (command == "--fft")
            {
                clock_t start, end;
                start = clock();
                save(fft(name), name, "_fft.bmp");
                end = clock();
                double duration_sec = double(end - start) / CLOCKS_PER_SEC;
                cout << "The operation took " << duration_sec << " seconds.";
            }
            else if (command == "--idft")
            {
                clock_t start, end;
                start = clock();
                dft(name);
                save(idft(name), name, "_idft.bmp");
                end = clock();
                double duration_sec = double(end - start) / CLOCKS_PER_SEC;
                cout << "The operation took " << duration_sec << " seconds.";
            }
            else if(command == "--ifft")
            {
                clock_t start, end;
                start = clock();
                fft(name);
                save(ifft(name), name, "_ifft.bmp");
                end = clock();
                double duration_sec = double(end - start) / CLOCKS_PER_SEC;
                cout << "The operation took " << duration_sec << " seconds.";
            }
            else if (command == "--lowp")
            {
                fft(name);
                lowpass(name, atoi(argv[3]));
                save(ifft(name), name, "_lowp.bmp");
            }
            else if (command == "--highp")
            {
                fft(name);
                highpass(name, atoi(argv[3]));
                save(ifft(name), name, "_highp.bmp");
            }
            else if (command == "--bpass")
            {
                fft(name);
                bandpass(name, atoi(argv[3]), atoi(argv[4]));
                save(ifft(name), name, "_bpass.bmp");
            }
            else if (command == "--bcut")
            {
                fft(name);
                bandcut(name, atoi(argv[3]), atoi(argv[4]));
                save(ifft(name), name, "_bcut.bmp");
            }
            else if (command == "--pmod")
            {
                fft(name);
                pmod(name, atoi(argv[3]), atoi(argv[4]));
                save(ifft(name), name, "_pmod.bmp");
            }
            else if (command == "--hpassed")
            {
                fft(name);
                highpassedgedet(name, argv[3]);
                save(ifft(name), name, "_hpassed.bmp");
            }
            else if (command == "--hpassedm")
            {
                fft(name);
                save(highpassedgedetmask(name, atof(argv[3]), atof(argv[4]), atof(argv[5])), name, "_hpassedmask.bmp");
                save(ifft(name), name, "_hpassedm.bmp");
            }
            else cout << "No such function, try --help\n";
        }

        catch (CImgIOException &failure)
        {
            return 0;
        }

    return 0;
}