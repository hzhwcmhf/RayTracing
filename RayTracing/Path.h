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

	
	//从startPos开始沿startDir拓展，获取inner、F，返回end面(注意超过层数要返回)
	Face* extend(const Point &_startPos, const Point &_startDir);
	Face* extend();
	//检查是否遮挡，获取F
	void checkShadow(const Point &_startPos, const Point &_endPos);
	void checkShadow();
	//翻转路径
	void reverse();
};

class Path
{
private:
	RayTracing* rt;
	std::vector<BRDF*> brdfs;
	std::vector<SubPath> subpaths;

	//漫反射次数和光源造成的概率
	double diffuseAndLightProbability;
public:
	static Path makeRandomPath(RayTracing* r);
	std::tuple<Path, double> mutate();
	void record(BitmapArray& barr, double w);
	double queryInitLuminianceDivProbability();
};