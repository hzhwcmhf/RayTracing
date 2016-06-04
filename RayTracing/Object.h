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

	double scale;
	double rotatex, rotatey, rotatez;
	Point pos;

	Object(double _rotatex = 0, double _rotatey = 0, double _rotatez = 0) {
		rotatex = _rotatex, rotatey = _rotatey, rotatez = _rotatez;
	}

	Point transform(Point p) const;
	Point transformN(Point p) const;
	void replace(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
	void rerotate(double rotatex, double rotatey, double rotatez);

	bool Load(const char* filename);
};
