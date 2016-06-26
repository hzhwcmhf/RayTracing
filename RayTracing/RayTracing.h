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
	
	//单次迭代
	BitmapArray MLT_process(Path &p);

	//摄像机类
	struct Camera
	{
		double realWidth, realHeight;

		Point generateDir() const;
		Point generateDir(double x, double y) const;
		std::tuple<int, int> queryPos(const Point &dir) const;
		bool queryInImage(const Point &dir) const;
	}camera;

public:

	//收敛用加权weight
	std::array<std::array<double,FinalWidth>, FinalHeight> *initialWeights;

	RayTracing();
	~RayTracing();

	//主函数
	Bitmap metropisLightTransport();

	ReflectRecord queryEye();
	ReflectRecord queryLight();
	const KDtree* queryKDtree();
	const Camera* queryCamera();

	//三张图的初始化代码
	void Init1();
	void Init2();
	void Init3();

};