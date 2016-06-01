#pragma once

#include "Face.h"

class Object
{
private:
	bool Parse(FILE* fp);
public:
	std::vector<Point> p, pn;
	std::vector<Face> f;
	
	Color kd, ks, tf; //������ϵ�������淴��ϵ��������ϵ��
	double kdL, ksL, tfL; //���������ȣ����淴�����ȣ��������ȣ���֤��Ϊ1
	double Ni; //������

	double scale;
	double rotatex, rotatey, rotatez;
	Point pos;

	Object(double _scale = 1, 
		double _rotatex = 0, double _rotatey = 0, double _rotatez = 0,
		Point _pos = Point(0,0,0)) {
		scale = _scale;
		rotatex = _rotatex, rotatey = _rotatey, rotatez = _rotatez;
		pos = _pos;
	}

	Point transform(Point p) const;
	Point transformN(Point p) const;

	bool Load(const char* filename);
};
