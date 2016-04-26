#pragma once

#include "BitmapArray.h"
#include "Point.h"
#include "Face.h"
#include "BRDF.h"

class RayTracing;

struct SubPath
{
	RayTracing* rt;
	Point startPos, startDir;
	Point endPos, endDir;

	std::vector<Face*> inner;

	Color F;

	SubPath(RayTracing* r);

	
	//��startPos��ʼ��startDir��չ����ȡinner��F������end��(ע�ⳬ������Ҫ����)
	Face* extend(const Point &_startPos, const Point &_startDir);
	Face* extend();
	//����Ƿ��ڵ�����ȡF
	void checkShadow(const Point &_startPos, const Point &_endPos);
	void checkShadow();
	//��ת·��
	void reverse();
};

class Path
{
private:
	RayTracing* rt;
	std::vector<BRDF*> brdfs;
	std::vector<SubPath> subpaths;

	//����������͹�Դ��ɵĸ���
	double diffuseAndLightProbability;
public:
	static Path makeRandomPath(RayTracing* r);
	std::tuple<Path, double> mutate();
	void record(BitmapArray& barr, double w);
	double queryInitLuminianceDivProbability();
};