#include "stdafx.h"
#include "Bitmap.h"

#include "Bitmap.h"
#include <iostream>
#pragma warning(disable:4996)

bool Bitmap::read(const std::string &filename)
{
	FILE *fpi = fopen(filename.c_str(), "rb");
	if (fpi == NULL) { // 文件打开失败
		return false;
	}

	if (imagedata != NULL) {
		for (int i = 0; i < strInfo.biHeight; ++i)
			delete imagedata[i];
		delete imagedata;
		imagedata = NULL;
	}

	fread(&strHead, 1, sizeof(BITMAPFILEHEADER), fpi);
	if (strHead.bfType != 0x4d42) { // bmp标识错误，非bmp文件
		return false;
	}
	fread(&strInfo, 1, sizeof(BITMAPINFOHEADER), fpi);

	//读取调色板，真彩色调色板为0 
	for (DWORD nCounti = 0; nCounti < strInfo.biClrUsed; nCounti++) {
		//存储的时候，一般去掉保留字rgbReserved   
		fread((char *)&strPla[nCounti].rgbBlue, 1, sizeof(BYTE), fpi);
		fread((char *)&strPla[nCounti].rgbGreen, 1, sizeof(BYTE), fpi);
		fread((char *)&strPla[nCounti].rgbRed, 1, sizeof(BYTE), fpi);
	}

	//读出图片的像素数据
	imagedata = new IMAGEDATA*[strInfo.biHeight];
	for (int i = 0; i < strInfo.biHeight; ++i) {
		imagedata[i] = new IMAGEDATA[strInfo.biWidth];
		memset(imagedata[i], 0, sizeof(IMAGEDATA) * strInfo.biWidth);
		fread(imagedata[i], sizeof(IMAGEDATA), strInfo.biWidth, fpi);
	}

	fclose(fpi);
	//showInfo();

	return true;
}

bool Bitmap::save(const std::string &filename)
{
	FILE *fpw = fopen(filename.c_str(), "wb");
	if (fpw == NULL) { // 创建文件失败
		return false;
	}

	// 宽度修正到4的倍数，计算文件头部分有关大小的信息
	LONG initWidth = strInfo.biWidth;
	LONG delta = 0;
	if (initWidth & 3)
		delta = 4 - (initWidth & 3);
	strInfo.biWidth += delta;
	strInfo.biSizeImage = strInfo.biWidth * strInfo.biHeight * 3;
	strHead.bfSize = strInfo.biSizeImage + 54;

	//showInfo();
	// 输出bmp文件头
	fwrite(&strHead, sizeof(BITMAPFILEHEADER), 1, fpw);
	fwrite(&strInfo, sizeof(BITMAPINFOHEADER), 1, fpw);

	//保存调色板数据   
	for (DWORD nCounti = 0; nCounti<strInfo.biClrUsed; nCounti++) {
		fwrite(&strPla[nCounti].rgbBlue, sizeof(BYTE), 1, fpw);
		fwrite(&strPla[nCounti].rgbGreen, sizeof(BYTE), 1, fpw);
		fwrite(&strPla[nCounti].rgbRed, sizeof(BYTE), 1, fpw);
	}

	//保存像素数据   
	BYTE zero[15];
	memset(zero, 0, sizeof zero);

	for (int i = 0; i < strInfo.biHeight; ++i) {
		fwrite(imagedata[i], sizeof(IMAGEDATA), initWidth, fpw);
		if (delta) { // 以黑色像素填充缺少部分
			fwrite(zero, sizeof(BYTE), 3 * delta, fpw);
		}
	}

	fclose(fpw);

	// 还原图像信息
	strInfo.biWidth = initWidth;
	strInfo.biSizeImage = strInfo.biWidth * strInfo.biHeight * 3;
	strHead.bfSize = strInfo.biSizeImage + 54;

	//showInfo();

	return true;
}

void Bitmap::showInfo()
{
	std::cout << "位图文件头:" << std::endl;
	std::cout << "文件大小:" << strHead.bfSize << std::endl;
	std::cout << "保留字_1:" << strHead.bfReserved1 << std::endl;
	std::cout << "保留字_2:" << strHead.bfReserved2 << std::endl;
	std::cout << "实际位图数据的偏移字节数:" << strHead.bfOffBits << '\n' << std::endl;

	std::cout << "位图信息头:" << std::endl;
	std::cout << "结构体的长度:" << strInfo.biSize << std::endl;
	std::cout << "位图宽:" << strInfo.biWidth << std::endl;
	std::cout << "位图高:" << strInfo.biHeight << std::endl;
	std::cout << "biPlanes平面数:" << strInfo.biPlanes << std::endl;
	std::cout << "biBitCount采用颜色位数:" << strInfo.biBitCount << std::endl;
	std::cout << "压缩方式:" << strInfo.biCompression << std::endl;
	std::cout << "biSizeImage实际位图数据占用的字节数:" << strInfo.biSizeImage << std::endl;
	std::cout << "X方向分辨率:" << strInfo.biXPelsPerMeter << std::endl;
	std::cout << "Y方向分辨率:" << strInfo.biYPelsPerMeter << std::endl;
	std::cout << "使用的颜色数:" << strInfo.biClrUsed << std::endl;
	std::cout << "重要颜色数:" << strInfo.biClrImportant << std::endl;

	std::cout << "调色板:" << std::endl;
	for (DWORD nCounti = 0; nCounti<strInfo.biClrUsed; nCounti++) {
		std::cout << "strPla[nCounti].rgbBlue" << strPla[nCounti].rgbBlue << std::endl;
		std::cout << "strPla[nCounti].rgbGreen" << strPla[nCounti].rgbGreen << std::endl;
		std::cout << "strPla[nCounti].rgbRed" << strPla[nCounti].rgbRed << std::endl;
	}
	std::cout << "Info end" << std::endl;
}

Bitmap::Bitmap(const std::string &filename)
{
	imagedata = NULL;
	read(filename);
}

Bitmap::Bitmap(int width, int height)
{
	/*
	* 创建空位图时默认为真彩色（无调色盘数据
	* 压缩方式为0（不压缩）
	* XY方向分辨率均为2835
	*/
	strInfo.biSize = 40;
	strInfo.biWidth = width;
	strInfo.biHeight = height;
	strInfo.biPlanes = 1;
	strInfo.biBitCount = 24;
	strInfo.biCompression = 0;
	strInfo.biSizeImage = (DWORD)width * height * 3;
	strInfo.biXPelsPerMeter = 2835;
	strInfo.biYPelsPerMeter = 2835;
	strInfo.biClrUsed = 0;
	strInfo.biClrImportant = 0;

	strHead.bfType = 0x4d42;
	strHead.bfSize = strInfo.biSizeImage + 54;
	strHead.bfOffBits = 54;

	imagedata = new IMAGEDATA*[strInfo.biHeight];
	for (int i = 0; i < strInfo.biHeight; ++i) {
		imagedata[i] = new IMAGEDATA[strInfo.biWidth];
		memset(imagedata[i], 0, sizeof(IMAGEDATA) * strInfo.biWidth);
	}
}

Bitmap::Bitmap(const Bitmap &data)
{
	strHead = data.strHead;
	strInfo = data.strInfo;

	memcpy(strPla, data.strPla, sizeof(data.strPla));

	imagedata = new IMAGEDATA*[strInfo.biHeight];
	for (int i = 0; i < strInfo.biHeight; ++i) {
		imagedata[i] = new IMAGEDATA[strInfo.biWidth];
		memcpy(imagedata[i], data.imagedata[i], sizeof(IMAGEDATA) * strInfo.biWidth);
	}

	//showInfo();
}

Bitmap::~Bitmap()
{
	if (imagedata != NULL) {
		for (int i = 0; i < strInfo.biHeight; ++i)
			delete imagedata[i];
		delete imagedata;
		imagedata = NULL;
	}
}

Bitmap& Bitmap::operator =(const Bitmap &data)
{
	if (this == &data)
		return *this;

	if (imagedata != NULL) {
		for (int i = 0; i < strInfo.biHeight; ++i)
			delete imagedata[i];
		delete imagedata;
		imagedata = NULL;
	}

	strHead = data.strHead;
	strInfo = data.strInfo;

	memcpy(strPla, data.strPla, sizeof(data.strPla));

	imagedata = new IMAGEDATA*[strInfo.biHeight];
	for (int i = 0; i < strInfo.biHeight; ++i) {
		imagedata[i] = new IMAGEDATA[strInfo.biWidth];
		memcpy(imagedata[i], data.imagedata[i], sizeof(IMAGEDATA) * strInfo.biWidth);
	}

	//showInfo();
	return *this;
}

int Bitmap::width() const
{
	return strInfo.biWidth;
}

int Bitmap::height() const
{
	return strInfo.biHeight;
}