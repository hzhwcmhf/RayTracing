#pragma once

/*
* ���ص�RGB��Ϣ
*/
struct IMAGEDATA
{
	//�����壬�ܹ�ͨ�����ַ�ʽ����
	union {
		struct {
			BYTE b;
			BYTE g;
			BYTE r;
		};
		struct {
			BYTE c[3];
		};
	};
	IMAGEDATA(int _r, int _g, int _b)
	{
		b = _b, g = _g, r = _r;
	}
	IMAGEDATA() {}
};

/*
* �洢bmp
* ͨ��[width][height]����ÿ�����ص��RGBֵ
* ��֧�����ɫλͼ����
*/
class Bitmap
{
public:
	// ������СΪwidth*height�Ŀ�bmpͼ
	Bitmap(int width, int height);
	// ���ļ�filename����
	Bitmap(const std::string &filename);
	Bitmap(const Bitmap &data);
	~Bitmap();
	Bitmap& operator =(const Bitmap &data);

public:
	// ���ļ�filename����bmp����������ʧԭ������
	bool read(const std::string &filename);
	// ���bmp���ļ�
	bool save(const std::string &filename);

	// ͼƬ����
	int width() const;
	int height() const;

	IMAGEDATA* operator[](int label)
	{
		return imagedata[label];
	}
	const IMAGEDATA* operator[](int label) const
	{
		return imagedata[label];
	}

private:
	// bmp�ļ�ͷ��Ϣ
	BITMAPFILEHEADER strHead;
	// 256ɫ��ɫ��   
	RGBQUAD strPla[256];
	// bmp�ļ���Ϣ
	BITMAPINFOHEADER strInfo;
	// �洢������Ϣ
	IMAGEDATA **imagedata;

private:
	// ͼƬ��Ϣ���
	void showInfo();
};
