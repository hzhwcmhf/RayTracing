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
	
	//��startPos��ʼ��startDir��չ����ȡinner��luminiance������end��(ע�ⳬ������Ҫ����)
	ReflectRecord extend(const HalfReflectRecord &start);
	ReflectRecord extend();
	

	//emplictLight
	bool checkShadow(const HalfReflectRecord &start, const Face* endFace);


	//��ת·��
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
	double diffuseAndLightProbability;//ɢ������͹�Դ��ɵ�ȡ������
	double randomProbability; //�������ʱ��ȡ������

	//����luminiance��randomProbability
	void calLuminianceAndRandomProbability();
	//����shadowDisatance
	bool checkShadow();
public:
	Path(RayTracing* r);
	static Path makeRandomPath(RayTracing* r);
	std::tuple<Path, double> mutate();
	void record(BitmapArray& barr, double w);
	double queryInitLuminianceDivProbability();
};