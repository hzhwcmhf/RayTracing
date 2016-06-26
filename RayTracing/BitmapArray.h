#pragma once

#include "Bitmap.h"

//使用浮点数记录颜色的类
class BitmapArray
{
public:
	struct IMAGEDATA
	{
		union {
			struct {
				double b;
				double g;
				double r;
			};
			struct {
				double c[3];
			};
		};
		IMAGEDATA(int _r, int _g, int _b)
		{
			b = _b, g = _g, r = _r;
		}
		IMAGEDATA() {}
	};
private:
	IMAGEDATA* data;
	int width, height;
public:
	BitmapArray() {};
	BitmapArray(int width, int height);
	BitmapArray(const char* filename);
	BitmapArray(const BitmapArray &b);
	BitmapArray(BitmapArray &&b);
	~BitmapArray();
	BitmapArray& operator =(const BitmapArray &b);
	BitmapArray& operator =(BitmapArray &&b);

	Bitmap transformToBitmap(int brightness);//转换为Bitmap
	
	void limitMax();	//限制单次图片最大亮度，避免因为算法卡入局部造成的亮度不均
	void save(const char* filename);
	
	void load(const char* filename);
	bool isEmpty() const;
	
	IMAGEDATA* operator[](int x);

};