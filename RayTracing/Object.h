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

	Object() {}

	bool Load(const char* filename);
};
