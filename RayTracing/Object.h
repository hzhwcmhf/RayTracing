#pragma once

#include "Face.h"

class Object
{
private:
	bool Parse(FILE* fp);
public:
	std::vector<Point> p, pn;
	std::vector<Face> f;
	
	Color kd, ks, tf; //漫反射系数，镜面反射系数，折射系数
	double kdL, ksL, tfL; //漫反射亮度，镜面反射亮度，折射亮度，保证和为1
	double Ni; //折射率

	Object() {}

	bool Load(const char* filename);
};
