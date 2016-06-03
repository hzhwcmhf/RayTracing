#include "stdafx.h"
#include "BitmapArray.h"

#pragma warning(disable:4996)

BitmapArray::BitmapArray(int width, int height)
{
	this->width = width;
	this->height = height;
	data = new IMAGEDATA[width * height];
	memset(data, 0, sizeof(IMAGEDATA) * width * height);
}

BitmapArray::BitmapArray(const char * filename)
{
	data = nullptr;
	load(filename);
}

BitmapArray::BitmapArray(const BitmapArray & b)
{
	width = b.width, height = b.height;
	data = new IMAGEDATA[width * height];
	memcpy(data, b.data, sizeof(IMAGEDATA) * width * height);
}

BitmapArray::BitmapArray(BitmapArray && b)
{
	data = b.data;
	width = b.width, height = b.height;
	b.data = nullptr;
}

BitmapArray::~BitmapArray()
{
	if (data) delete[] data;
}

BitmapArray & BitmapArray::operator=(const BitmapArray & b)
{
	if (data) delete[] data;
	width = b.width, height = b.height;
	data = new IMAGEDATA[width * height];
	memcpy(data, b.data, sizeof(IMAGEDATA) * width * height);
	return *this;
}

BitmapArray & BitmapArray::operator=(BitmapArray && b)
{
	if (data) delete[] data;
	data = b.data;
	width = b.width, height = b.height;
	b.data = nullptr;
	return *this;
}

Bitmap BitmapArray::transformToBitmap(int brightness)
{
	Bitmap res(width, height);
	double vmax = 0;
	double sum = 0;
	for (int i = 0;i < width; i++) {
		for (int j = 0;j < height;j++) {
			for (int k = 0;k < 3;k++){
				//vmax = (*this)[i][j].c[k];
				sum += (*this)[i][j].c[k];
			}
		}
	}
	sum /= width * height * 3;
	for (int i = 0;i < width;i++) {
		for (int j = 0;j < height;j++) {
			for (int k = 0;k < 3;k++) {
				//double tmp = (*this)[i][j].c[k] / vmax * brightness * 1.4;
				double tmp = (*this)[i][j].c[k] / sum * brightness * 1;
				res[j][i].c[k] = tmp>255?255:(BYTE)tmp;
			}
		}
	}
	return res;
}

void BitmapArray::save(const char * filename)
{
	FILE* file = fopen(filename, "w");
	fprintf(file, "%d %d\n", width, height);
	for (int i = 0; i < width; i++) {
		for (int j = 0;j < height; j++) {
			for (int k = 0;k < 3;k++) {
				fprintf(file, "%lf ", (*this)[i][j].c[k]);
			}
			fprintf(file, "\n");
		}
	}
}

void BitmapArray::load(const char * filename)
{
	if (data) {
		delete[] data;
		data = nullptr;
	}

	FILE* file = fopen(filename, "r");
	if (!file) return;
	int w, h;
	fscanf(file, "%d %d", &w, &h);
	width = w, height = h;
	data = new IMAGEDATA[width * height];
	assert(w == width && h == height);
	for (int i = 0; i < width; i++) {
		for (int j = 0;j < height; j++) {
			for (int k = 0;k < 3;k++) {
				fscanf(file, "%lf", &(*this)[i][j].c[k]);
			}
		}
	}
}

bool BitmapArray::isEmpty() const
{
	return !data;
}

BitmapArray::IMAGEDATA * BitmapArray::operator[](int x)
{
	return data + height * x;
}
