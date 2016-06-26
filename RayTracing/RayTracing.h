#pragma once

#include "Bitmap.h"
#include "BitmapArray.h"
#include "Path.h"
#include "KDtree.h"
#include "Object.h"


class RayTracing
{
private:
	KDtree tree;
	std::vector<Object*> vecObjects;
	
	//���ε���
	BitmapArray MLT_process(Path &p);

	//�������
	struct Camera
	{
		double realWidth, realHeight;

		Point generateDir() const;
		Point generateDir(double x, double y) const;
		std::tuple<int, int> queryPos(const Point &dir) const;
		bool queryInImage(const Point &dir) const;
	}camera;

public:

	//�����ü�Ȩweight
	std::array<std::array<double,FinalWidth>, FinalHeight> *initialWeights;

	RayTracing();
	~RayTracing();

	//������
	Bitmap metropisLightTransport();

	ReflectRecord queryEye();
	ReflectRecord queryLight();
	const KDtree* queryKDtree();
	const Camera* queryCamera();

	//����ͼ�ĳ�ʼ������
	void Init1();
	void Init2();
	void Init3();

};