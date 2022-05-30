#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include<mpi.h>
#include<stdio.h>
#include <ctime>// include this header 
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

	//*Read Image and save it to local arrayss*	
	//Read Image and save it to local arrayss

	System::Drawing::Bitmap BM(imagePath);

	OriginalImageWidth = BM.Width;
	OriginalImageHeight = BM.Height;
	*w = BM.Width;
	*h = BM.Height;
	int* Red = new int[BM.Height * BM.Width];
	int* Green = new int[BM.Height * BM.Width];
	int* Blue = new int[BM.Height * BM.Width];
	input = new int[BM.Height * BM.Width];
	for (int i = 0; i < BM.Height; i++)
	{
		for (int j = 0; j < BM.Width; j++)
		{
			System::Drawing::Color c = BM.GetPixel(j, i);

			Red[i * BM.Width + j] = c.R;
			Blue[i * BM.Width + j] = c.B;
			Green[i * BM.Width + j] = c.G;

			input[i * BM.Width + j] = ((c.R + c.B + c.G) / 3); //gray scale value equals the average of RGB values

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
			if (image[i * width + j] < 0)
			{
				image[i * width + j] = 0;
			}
			if (image[i * width + j] > 255)
			{
				image[i * width + j] = 255;
			}
			System::Drawing::Color c = System::Drawing::Color::FromArgb(image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j], image[i * MyNewImage.Width + j]);
			MyNewImage.SetPixel(j, i, c);
		}
	}
	MyNewImage.Save("..//Data//Output//10N_new10_" + index + ".png");
	cout << "result Image Saved " << index << endl;
}


int main()
{
	MPI_Init(NULL, NULL);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int numbers  = 10;
	int  total_sum = 0;
	for (int k = 0; k < numbers; k++)
	{

		int ImageWidth = 4, ImageHeight = 4;
		double  TotalTime_S = 0;
		double start_s, stop_s, TotalTime_Ms = 0;

		System::String^ imagePath;
		std::string img;
		img = "..//Data//Input//5N.png";

		imagePath = marshal_as<System::String^>(img);
		int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);//1d aray


		///  ////////////6  * 4
		int arr_size = ImageHeight * ImageWidth;//24
		int size = world_size;//edit//number of processes
		int chucnksize = (arr_size / size);///edit//24/2=12

		//kernel

		/*int kernel[9] =
		{ 0,-1,0,
		 -1,4,-1,
		 0,-1,0 };
		int k_size = 9;*/
		int kernel[25]=
		{ 0, -1, 0,-1,0,
		0, 0, 4,0 ,0,
		-1, -1,0,-1, -1,
		0, 0, 4,0 ,0,
		0, -1,0,-1, 0 };
		int k_size = 25;



		MPI_Bcast(&kernel, k_size, MPI_INT, 0, MPI_COMM_WORLD);
		int* res_chunk = new int[chucnksize]; // 12

		if (rank == 0)
		{
			start_s = clock();
		}
		MPI_Scatter(imageData, chucnksize, MPI_INT, res_chunk, chucnksize, MPI_INT, 0, MPI_COMM_WORLD);

		int index;
		int sum;
		int* dst = new int[chucnksize];  // 6*4  //final result for each processor//hight=rows / width columns
		for (int i = 0; i < (ImageHeight / size); i++) {
			for (int j = 0; j < ImageWidth; j++)
			{
				sum = 0;

				for (int x = 0; x < k_size; x++)
				{
					index = i * ImageWidth + j + x;

					sum += (res_chunk[index] * kernel[x]);
				}
				dst[i * ImageWidth + j] = sum;
			}
		}
		int* final_array = new int[arr_size];

		MPI_Gather(dst, chucnksize, MPI_INT, final_array, chucnksize, MPI_INT, 0, MPI_COMM_WORLD);
		if (rank == 0)
		{
			stop_s = clock();
			TotalTime_Ms += (stop_s - start_s) / (CLOCKS_PER_SEC) * 1000;
			cout << "time Ms: " << TotalTime_Ms << endl;
			total_sum += TotalTime_Ms;
			createImage(final_array, ImageWidth, ImageHeight, 0);
		}
	

		//free arrays
		//delete[]res_chunk;
	//	res_chunk = NULL;

		//delete[]dst;
		//dst = NULL;

		//delete[]final_array;
		//final_array = NULL;
		
		free(imageData);
		free(res_chunk);
		free(dst);
		free(final_array);
	}
	if (numbers == 10)
		cout << total_sum << endl;
	MPI_Finalize();
	
	return 0;
}