#pragma once

/*
* 像素点RGB信息
*/
struct IMAGEDATA
{
	//公用体，能够通过两种方式访问
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
* 存储bmp
* 通过[width][height]访问每个像素点的RGB值
* 仅支持真彩色位图操作
*/
class Bitmap
{
public:
	// 创建大小为width*height的空bmp图
	Bitmap(int width, int height);
	// 以文件filename创建
	Bitmap(const std::string &filename);
	Bitmap(const Bitmap &data);
	~Bitmap();
	Bitmap& operator =(const Bitmap &data);

public:
	// 从文件filename读入bmp，操作将丢失原有数据
	bool read(const std::string &filename);
	// 输出bmp至文件
	bool save(const std::string &filename);

	// 图片长宽
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
	// bmp文件头信息
	BITMAPFILEHEADER strHead;
	// 256色调色板   
	RGBQUAD strPla[256];
	// bmp文件信息
	BITMAPINFOHEADER strInfo;
	// 存储像素信息
	IMAGEDATA **imagedata;

private:
	// 图片信息输出
	void showInfo();
};
