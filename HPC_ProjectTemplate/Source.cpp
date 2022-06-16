#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#include<mpi.h>
#pragma once

#using <mscorlib.dll>
#using <System.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

using namespace std;
using namespace msclr::interop;

int* inputImage(int* w, int* h, System::String^ imagePath) //put the size of image in w & h
{
	int* input;


	int OriginalImageWidth, OriginalImageHeight;

	//*********************************************************Read Image and save it to local arrayss*************************	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int *Red = new int[BM.Height * BM.Width];
	int *Green = new int[BM.Height * BM.Width];
	int *Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height*BM.Width];
	cout << "0\n";
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i*BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values
			
			
		}

	}
	return input;
}
void createImage(int* image, int width, int height, int index)
{
	System::Drawing::Bitmap MyNewImage(width, height);


	for (int i = 0; i < MyNewImage.Height; i++)
	{
		for (int j = 0; j < MyNewImage.Width; j++)
		{
			//i * OriginalImageWidth + j
			if (image[i*width + j] < 0)
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255)
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j], image[i*MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//outputRes" + index + ".png");
	cout << "result Image Saved " << index << endl;
}

void seqHisto(string img);
void parHisto(string img);

int main()
{
	cout << "------------------------------------------------\nWELLCOME TO PARALELL AND SEQUENTIAL IMAGE QUANTIZIER\n------------------------------------------------\n";
	cout << "please add link of any new images you want to try\n";
		cout << "do you want to work in paralell or sequential? |S,P|";
		char userInput;
		cin >> userInput;
		if (userInput=='s'|| userInput=='S')
		{
			cout << "Enter the link to the photo you want to do paralell image histogram Quantizier on\n do not forget to put image type at the end ex: .jpg";
			string imgLink;
			cin >> imgLink;
			seqHisto(imgLink);
			cout << "open output folder to see output\n";
		}
		else if (userInput == 'p' || userInput == 'P')
		{
			cout << "------------------------------------------------------------\nNOTE: you should instull mpi library and setup project to work in paralell\n------------------------------------------------------------\n";
			cout << "Enter the link to the photo you want to do paralell image histogram Quantizer on\nDO NOT forget to put image type at the end ex: .jpg\n";
			string imgLink;
			cin >> imgLink;
			cout << "build the program then run it on cmd \n STEPS:\n1) open solution on file explorer\n2) open debug file\n3) open cms from debug file \n4) write mpiexec -n ""number of cores you want to work in paralell"" projectName.exe";
			parHisto("imgLink");//using paralllel code
			cout << "open output folder to see output\n";


		}	
}
void seqHisto(string img)
{
	int ImageWidth = 4, ImageHeight = 4;

	int start_s, stop_s, TotalTime = 0;

	System::String^ imagePath;
	//std::string img;
	

	imagePath = marshal_as<System::String^>(img);
	int* input = inputImage(&ImageWidth, &ImageHeight, imagePath);
	 start_s = clock();
	float freqArr[256];
	int* finres = new int[ImageWidth *ImageHeight];

	int Crange = 256;
	for (int i = 0; i < 256; i++)
	{
		freqArr[i] = 0;
	}

	for (int i = 0; i < ImageHeight; i++)
	{
		for (int j = 0; j < ImageWidth; j++)
		{
			//cout << input[i * BM.Width + j];

			freqArr[input[i * ImageWidth + j]]++;
		}
	}
	for (int i = 0; i < Crange; i++)
	{
		freqArr[i] /= (ImageWidth * ImageHeight);
	}
	float sum = 0;
	for (int i = 0; i < Crange; i++)
	{
		sum += freqArr[i];
		freqArr[i] = sum;
	}
	for (int i = 0; i < Crange; i++)
	{
		freqArr[i] *= 256;
		freqArr[i] = round(freqArr[i]);

	}
	for (int i = 0; i < ImageWidth * ImageHeight; i++)
	{
		finres[i] = (int)freqArr[input[i] - 1];
	}
	 stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time in seq: " << TotalTime << endl;
		cout << "size is " << ImageHeight << " * " << ImageWidth;
		createImage(finres, ImageWidth, ImageHeight, 0);
	
	free(input);
}

void parHisto(string img)
{
	MPI_Init(NULL, NULL);
	int size;

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int* input=NULL;
	int ImageWidth = 4, ImageHeight = 4;
	float freqArrAt0[256];
	int start_s, stop_s, TotalTime = 0;
	int* Allfinres = nullptr;

	if (rank==0)
	{

		System::String^ imagePath;
	//	std::string img;
		

		imagePath = marshal_as<System::String^>(img);
		 input = inputImage(&ImageWidth, &ImageHeight, imagePath);
		 start_s = clock();
		 Allfinres = new int[ImageHeight * ImageWidth];


	}
	MPI_Bcast(&ImageHeight, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&ImageWidth, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	float freqArr[256];
	int Crange = 256;
	for (int i = 0; i < 256; i++)
	{
		freqArr[i] = 0;
	}
	//cout << BM.Width << "width " << BM.Height << "height\n";

	
	
	int* finres = new int[int(ImageHeight * ImageWidth/size)];
	int* local_arr = new int[(ImageHeight * ImageWidth / size)];

	MPI_Scatter(input, int(ImageHeight * ImageWidth / size), MPI_INT, local_arr, int(ImageHeight * ImageWidth / size),MPI_INT, 0, MPI_COMM_WORLD);
	for (long j = 0; j < (ImageHeight * ImageWidth/size); j++)
	{
		freqArr[local_arr[j]]++;
	}
	MPI_Reduce(freqArr, freqArrAt0, 256, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

	if (rank==0)
	{
		float sum = 0;
		for (int i = 0; i < Crange; i++)
		{
			freqArrAt0[i] /= (ImageHeight * ImageWidth);
		}
		for (int i = 0; i < Crange; i++)
		{
			sum += freqArrAt0[i];
			freqArrAt0[i] = sum;
		}
		for (int i = 0; i < Crange; i++)
		{
			freqArrAt0[i] *= 256;
			freqArrAt0[i] = round(freqArrAt0[i]);

		}
	}
	MPI_Bcast(freqArrAt0, 256, MPI_FLOAT, 0, MPI_COMM_WORLD);
	for (int i = 0; i < int(ImageHeight * ImageWidth/size); i++)
	{
		finres[i] = freqArrAt0[local_arr[i] - 1];
	}
	MPI_Gather(finres, int(ImageHeight * ImageWidth / size), MPI_INT, Allfinres, int(ImageHeight * ImageWidth / size), MPI_INT, 0, MPI_COMM_WORLD);
	//MPI_Barrier(MPI_COMM_WORLD);

	if (rank==0)
	{
	//	Allfinres = Allfinres;
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time in parralel: " << TotalTime << endl;
		cout << "size is " << ImageHeight << " * " << ImageWidth;

		createImage(Allfinres, ImageWidth, ImageHeight, 0);
		free(input);
	}
	free(Allfinres);

	MPI_Finalize();
	return;
}
