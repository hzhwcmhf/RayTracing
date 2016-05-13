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
	std::vector<Object> vecObjects;

	BitmapArray MLT_process(Path &p);

	struct Camera
	{
		double realWidth, realHeight;

		Point generateDir() const;
		Point generateDir(double x, double y) const;
		std::tuple<int, int> queryPos(const Point &dir) const;
		bool queryInImage(const Point &dir) const;
	}camera;

public:

	Bitmap metropisLightTransport();

	ReflectRecord queryEye();//tmp
	ReflectRecord queryLight();//tmp
	const KDtree* queryKDtree();
	const Camera* queryCamera();

	void tmpInit();//tmp

};