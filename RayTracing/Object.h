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

	double scale;//放大比例
	double rotatex, rotatey, rotatez;//旋转角度
	Point pos;//平移位置

	Object(double _rotatex = 0, double _rotatey = 0, double _rotatez = 0) {
		rotatex = _rotatex, rotatey = _rotatey, rotatez = _rotatez;
	}

	Point transform(Point p) const;//变换点
	Point transformN(Point p) const;//变换法向量
	//重新放置物体（保持比例）
	void replace(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
	//重新旋转物体
	void rerotate(double rotatex, double rotatey, double rotatez);

	//读入物体
	bool Load(const char* filename);
};
