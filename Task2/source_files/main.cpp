#include <iostream>
#include <cstdlib>
#include "../header_files/function.h"
#include "../header_files/CImg.h"


using namespace std;
using namespace cimg_library;
int main(int argc, char* argv[])
{
	if (argc<2)
	{
		cout << "There is no argument!" << endl;
		return 0;
	}
	else if (argc == 2)
	{
		if (strcmp(argv[1], "--help") == 0)
		{
			help();
			return 0;
		}
		else
		{
			cout << "There is not enough arguments!" << endl;
			return 0;
		}
	}
	else if (argc == 3)
	{
		const char* command = argv[1];
		const char* img = argv[2];
		if (checkFile(img) == 0) return 0;
		else if ((strcmp(command, "--slineid") == 0)) slineid(img);
		else if ((strcmp(command, "--oslineid") == 0)) oslineid(img);
		else if ((strcmp(command, "--orobertsi") == 0)) orobertsi(img);
		else if ((strcmp(command, "--cmean") == 0)) cmean(img);
		else if ((strcmp(command, "--cvariance") == 0)) cvariance(img);
		else if ((strcmp(command, "--cstdev") == 0)) cstdev(img);
		else if ((strcmp(command, "--cvarcoi") == 0)) cvarcoi(img);
		else if ((strcmp(command, "--casyco") == 0)) casyco(img);
		else if ((strcmp(command, "--flaco") == 0)) flaco(img);
		else if ((strcmp(command, "--cvarcoii") == 0)) cvarcoii(img);
		else if ((strcmp(command, "--centropy") == 0)) centropy(img);
		else error();
	}
	else if (argc == 4)
	{
		const char* command = argv[1];
		const char* img = argv[2];
		if (checkFile(img) == 0) return 0;
		if ( (strcmp(command, "--histogram") == 0))
		{
			int value = atoi(argv[3]);

			 if ((strcmp(command, "--histogram") == 0))
			{
				if (value == 0 || value == 1 || value == 2)
				{
					histogram(img, value);
				}
				else error();
				return 0;
			}
			else error();
		}
		else if ( (strcmp(command, "--new_contrast") == 0))
		{
			float value = atof(argv[3]);
			 if ((strcmp(command, "--new_contrast") == 0)) new_contrast(img, value);
			else error();
		}
	}
	else if (argc == 5)
	{
		const char* command = argv[1];
		if (strcmp(command, "--hraleigh") == 0)
		{
			const char* img = argv[2];
			if (checkFile(img) == 0) return 0;
			int gmin = atof(argv[3]);
			int gmax = atoi(argv[4]);
			if ((strcmp(command, "--hraleigh") == 0)) hraleigh(img, gmin, gmax);
		}
		else
		{
			const char* img1 = argv[2];
			const char* img2 = argv[3];
			const char* img3 = argv[4];
			if (checkFile(img1) == 0) return 0;
			if (checkFile(img2) == 0) return 0;
			if (checkFile(img3) == 0) return 0;

			else error();
		}
	}
	else
	{
		error();
	}
	return 0;
}
