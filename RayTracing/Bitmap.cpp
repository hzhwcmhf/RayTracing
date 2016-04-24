#include "stdafx.h"
#include "Bitmap.h"

#include "Bitmap.h"
#include <iostream>
#pragma warning(disable:4996)

bool Bitmap::read(const std::string &filename)
{
	FILE *fpi = fopen(filename.c_str(), "rb");
	if (fpi == NULL) { // �ļ���ʧ��
		return false;
	}

	if (imagedata != NULL) {
		for (int i = 0; i < strInfo.biHeight; ++i)
			delete imagedata[i];
		delete imagedata;
		imagedata = NULL;
	}

	fread(&strHead, 1, sizeof(BITMAPFILEHEADER), fpi);
	if (strHead.bfType != 0x4d42) { // bmp��ʶ���󣬷�bmp�ļ�
		return false;
	}
	fread(&strInfo, 1, sizeof(BITMAPINFOHEADER), fpi);

	//��ȡ��ɫ�壬���ɫ��ɫ��Ϊ0 
	for (DWORD nCounti = 0; nCounti < strInfo.biClrUsed; nCounti++) {
		//�洢��ʱ��һ��ȥ��������rgbReserved   
		fread((char *)&strPla[nCounti].rgbBlue, 1, sizeof(BYTE), fpi);
		fread((char *)&strPla[nCounti].rgbGreen, 1, sizeof(BYTE), fpi);
		fread((char *)&strPla[nCounti].rgbRed, 1, sizeof(BYTE), fpi);
	}

	//����ͼƬ����������
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
	if (fpw == NULL) { // �����ļ�ʧ��
		return false;
	}

	// ���������4�ı����������ļ�ͷ�����йش�С����Ϣ
	LONG initWidth = strInfo.biWidth;
	LONG delta = 0;
	if (initWidth & 3)
		delta = 4 - (initWidth & 3);
	strInfo.biWidth += delta;
	strInfo.biSizeImage = strInfo.biWidth * strInfo.biHeight * 3;
	strHead.bfSize = strInfo.biSizeImage + 54;

	//showInfo();
	// ���bmp�ļ�ͷ
	fwrite(&strHead, sizeof(BITMAPFILEHEADER), 1, fpw);
	fwrite(&strInfo, sizeof(BITMAPINFOHEADER), 1, fpw);

	//�����ɫ������   
	for (DWORD nCounti = 0; nCounti<strInfo.biClrUsed; nCounti++) {
		fwrite(&strPla[nCounti].rgbBlue, sizeof(BYTE), 1, fpw);
		fwrite(&strPla[nCounti].rgbGreen, sizeof(BYTE), 1, fpw);
		fwrite(&strPla[nCounti].rgbRed, sizeof(BYTE), 1, fpw);
	}

	//������������   
	BYTE zero[15];
	memset(zero, 0, sizeof zero);

	for (int i = 0; i < strInfo.biHeight; ++i) {
		fwrite(imagedata[i], sizeof(IMAGEDATA), initWidth, fpw);
		if (delta) { // �Ժ�ɫ�������ȱ�ٲ���
			fwrite(zero, sizeof(BYTE), 3 * delta, fpw);
		}
	}

	fclose(fpw);

	// ��ԭͼ����Ϣ
	strInfo.biWidth = initWidth;
	strInfo.biSizeImage = strInfo.biWidth * strInfo.biHeight * 3;
	strHead.bfSize = strInfo.biSizeImage + 54;

	//showInfo();

	return true;
}

void Bitmap::showInfo()
{
	std::cout << "λͼ�ļ�ͷ:" << std::endl;
	std::cout << "�ļ���С:" << strHead.bfSize << std::endl;
	std::cout << "������_1:" << strHead.bfReserved1 << std::endl;
	std::cout << "������_2:" << strHead.bfReserved2 << std::endl;
	std::cout << "ʵ��λͼ���ݵ�ƫ���ֽ���:" << strHead.bfOffBits << '\n' << std::endl;

	std::cout << "λͼ��Ϣͷ:" << std::endl;
	std::cout << "�ṹ��ĳ���:" << strInfo.biSize << std::endl;
	std::cout << "λͼ��:" << strInfo.biWidth << std::endl;
	std::cout << "λͼ��:" << strInfo.biHeight << std::endl;
	std::cout << "biPlanesƽ����:" << strInfo.biPlanes << std::endl;
	std::cout << "biBitCount������ɫλ��:" << strInfo.biBitCount << std::endl;
	std::cout << "ѹ����ʽ:" << strInfo.biCompression << std::endl;
	std::cout << "biSizeImageʵ��λͼ����ռ�õ��ֽ���:" << strInfo.biSizeImage << std::endl;
	std::cout << "X����ֱ���:" << strInfo.biXPelsPerMeter << std::endl;
	std::cout << "Y����ֱ���:" << strInfo.biYPelsPerMeter << std::endl;
	std::cout << "ʹ�õ���ɫ��:" << strInfo.biClrUsed << std::endl;
	std::cout << "��Ҫ��ɫ��:" << strInfo.biClrImportant << std::endl;

	std::cout << "��ɫ��:" << std::endl;
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
	* ������λͼʱĬ��Ϊ���ɫ���޵�ɫ������
	* ѹ����ʽΪ0����ѹ����
	* XY����ֱ��ʾ�Ϊ2835
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