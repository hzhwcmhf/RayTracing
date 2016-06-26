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

	double scale;//�Ŵ����
	double rotatex, rotatey, rotatez;//��ת�Ƕ�
	Point pos;//ƽ��λ��

	Object(double _rotatex = 0, double _rotatey = 0, double _rotatez = 0) {
		rotatex = _rotatex, rotatey = _rotatey, rotatez = _rotatez;
	}

	Point transform(Point p) const;//�任��
	Point transformN(Point p) const;//�任������
	//���·������壨���ֱ�����
	void replace(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax);
	//������ת����
	void rerotate(double rotatex, double rotatey, double rotatez);

	//��������
	bool Load(const char* filename);
};
