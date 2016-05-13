#pragma once

#include "BitmapArray.h"
#include "Point.h"
#include "Face.h"
#include "ReflectRecord.h"

class RayTracing;

struct SubPath
{
	RayTracing* rt;
	HalfReflectRecord startR, endR;

	std::vector<ReflectRecord> inner;

	Color luminiance;
	double randomProbability;

	SubPath(RayTracing* r);
	
	//从startPos开始沿startDir拓展，获取inner、luminiance，返回end面(注意超过层数要返回)
	ReflectRecord extend(const HalfReflectRecord &start);
	ReflectRecord extend();
	

	//emplictLight
	bool checkShadow(const HalfReflectRecord &start, const HalfReflectRecord &end);


	//翻转路径
	void reverse();
};

class Path
{
private:
	RayTracing* rt;
	std::vector<ReflectRecord> eyeBRDF, lightBRDF;
	std::vector<SubPath> eyePath, lightPath;

	ReflectRecord shadowEyeBRDF, shadowLightBRDF;
	SubPath shadowPath;
	double shadowDistance;

	Color luminiance;
	double diffuseAndLightProbability;//散射次数和光源造成的取样概率
	double randomProbability; //随机采样时的取样概率

	//计算luminiance和randomProbability
	void calLuminianceAndRandomProbability();
	//计算shadowDisatance
	bool checkShadow();

	
	bool queryInImage();

public:
	Path(RayTracing* r);
	static Path makeRandomPath(RayTracing* r);
	static Path makeRandomPathInImage(RayTracing* r);
	std::tuple<Path, double> mutate();	//tmp
	void record(BitmapArray& barr, double w);
	double queryInitLuminianceDivProbability();

	static double debugQueryLuminianceInImage(RayTracing* r, double x, double y);
	int debugQueryDiffuseTimes();
	int debugEyeDiffuseTimes();

	std::tuple<int, int> queryImagePos();
};