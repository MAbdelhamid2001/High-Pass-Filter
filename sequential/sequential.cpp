#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include<msclr\marshal_cppstd.h>
#include <ctime>// include this header 
#include<vector>
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
	MyNewImage.Save("..//Data//Output//output12" + index + ".png");
	cout << "result Image Saved" << index << endl;
}
int main()
{
	int ImageWidth = 4, ImageHeight = 4;
	int start_s, stop_s, TotalTime = 0;

	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//N.png";

	imagePath = marshal_as<System::String^>(img);
	int* imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);

	//cout << ImageWidth * ImageHeight;//3912000
	float n_rows = ImageWidth;
	float m_columns = ImageHeight;
	cout << "w*h" << ImageWidth * ImageHeight;//3912000
	int n = n_rows, k = m_columns;

	//create 2d
	int** arr = new int* [n];
	for (int i = 0; i < n; i++) {
		arr[i] = new int[k];
	}

	int p = 0;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < k; j++) {
			arr[i][j] = imageData[p++];
		}
	}

	cout << " p : " << p;

	int kernel[3][3] = {
		{0, -1, 0},
		{-1, 4, -1},
		{0, -1, 0}
	};
	int k_size = 3;
	//create ouptut 2d array
	int output_row = n_rows - k_size + 1;
	int output_column = m_columns - k_size + 1;
	int** dst = new int* [output_row];
	for (int i = 0; i < output_row; i++) {
		dst[i] = new int[output_column];
	}
	for (int i = 0; i < output_row; i++) {
		for (int j = 0; j < output_column; j++) {
			dst[i][j] = 0;
		}
	}

	// operation
/////////////////
	start_s = clock();
	for (int i = 0; i < output_row; i++) {
		for (int j = 0; j < output_column; j++) {
			float sum = 0;
			for (int m = 0; m < k_size; m++)
			{
				for (int n = 0; n < k_size; n++)
				{
					sum = sum + kernel[m][n] * arr[i + m][j + n];
				}
			}
			//cout << sum << "sum ";
			dst[i][j] = abs(sum);
		}
	}
	stop_s = clock();

	int* final_image = new int[output_row * output_column];
	int idx = 0;
	for (int i = 0; i < output_row; i++) {
		for (int j = 0; j < output_column; j++) {
			//	int v = (i * m_columns + j);
			final_image[idx] = dst[i][j];
			idx++;

		}
	}


	final_image;
	//start_s = clock();
	createImage(final_image, output_row, output_column, 0);
	//stop_s = clock();
	TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "time: " << TotalTime << endl;
	cout << "time displayed " << endl;

	for (int i = 0; i < n_rows; i++) {
		delete[] arr[i];
	}
	for (int i = 0; i < output_row; i++) {
		delete[] dst[i];
	}
	delete arr;
	delete dst;
	arr = NULL;
	dst = NULL;
	free(final_image);

	system("pause");
	return 0;


}