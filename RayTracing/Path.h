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

	//ÿһ�� 1/r2 * ÿ���� L
	Color luminiance;
	//ÿһ�� 1/r2 * ÿ���� P
	double randomProbability;

	SubPath(RayTracing* r);
	
	//��startPos��ʼ��startDir��չ����ȡinner��luminiance������end��(ע�ⳬ������Ҫ����)
	ReflectRecord extend(const HalfReflectRecord &start);
	ReflectRecord extend();
	ReflectRecord extendAdjust(const HalfReflectRecord &start, const SubPath &ori);
	

	//emplictLight
	bool checkShadow(const HalfReflectRecord &start, const HalfReflectRecord &end);


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

	Color luminiance;
	double diffuseAndLightProbability;//ɢ������͹�Դ��ɵ�ȡ������
	double randomProbability; //�������ʱ��ȡ������

	//����luminiance��randomProbability
	void calLuminianceAndRandomProbability();
	//����shadowDisatance
	bool checkShadow();

	
	bool queryInImage();

public:
	Path(RayTracing* r);
	static Path makeRandomPath(RayTracing* r);
	static Path makeRandomPathInImage(RayTracing* r);

	std::tuple<Path, double> mutateRotate() const;
	std::tuple<Path, double> mutateSplit() const;
	bool canMutateSplit() const;
	std::tuple<Path, double> mutate() const;	//tmp
	void record(BitmapArray& barr, double w);
	double queryInitLuminianceDivProbability();

	static double debugQueryLuminianceInImage(RayTracing* r, double x, double y);
	int debugQueryDiffuseTimes();
	int debugEyeDiffuseTimes();
	void debugMutateRotate() const;

	std::tuple<int, int> queryImagePos();
};