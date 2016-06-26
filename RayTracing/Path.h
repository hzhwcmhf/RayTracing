#pragma once

#include "BitmapArray.h"
#include "Point.h"
#include "Face.h"
#include "ReflectRecord.h"

class RayTracing;

//������ֱ�ӵ�·��
struct SubPath
{
	RayTracing* rt;
	HalfReflectRecord startR, endR;//·����ʼ�ͽ����������

	std::vector<ReflectRecord> inner;//��¼���������

	//ÿһ�� 1/r2 * ÿ���� L
	Color luminiance;
	//ÿһ�� 1/r2 * ÿ���� P
	double randomProbability;

	SubPath(RayTracing* r);
	
	//��startPos��ʼ��startDir��չ����ȡinner��luminiance������end��(ע�ⳬ������Ҫ����)
	ReflectRecord extend(const HalfReflectRecord &start);
	ReflectRecord extend();
	//����ԭ��·�����е���
	ReflectRecord extendAdjust(const HalfReflectRecord &start, const SubPath &ori);
	

	//����ֱ������
	bool checkShadow(const HalfReflectRecord &start, const HalfReflectRecord &end);


	//��ת·��
	void reverse();
};

//����·��
class Path
{
private:
	RayTracing* rt;
	//˫��������·����¼
	std::vector<ReflectRecord> eyeBRDF, lightBRDF;
	//˫��Subpath��¼
	std::vector<SubPath> eyePath, lightPath;

	//���Ӵ��Ĺ���״��
	ReflectRecord shadowEyeBRDF, shadowLightBRDF;
	SubPath shadowPath;//���Ӵ���Subpath

	Color luminiance;
	double diffuseAndLightProbability;//ɢ������͹�Դ��ɵ�ȡ������
	double randomProbability; //�������ʱ��ȡ������

	//����luminiance��randomProbability
	void calLuminianceAndRandomProbability();
	//����shadowDisatance
	bool checkShadow();

	//����Ƿ���ͼ��
	bool queryInImage();

public:
	Path(RayTracing* r);
	//�������·��
	static Path makeRandomPath(RayTracing* r);
	//���������ͼ��·��
	static Path makeRandomPathInImage(RayTracing* r);
	//������ɵ��η���·��
	static bool makeOneDiffusePath(RayTracing* r, const Point &dir);

	//�Ŷ�����
	std::tuple<Path, double> mutateRotate() const;
	//���ѱ���
	std::tuple<Path, double> mutateSplit() const;
	//����Ƿ��ܹ�����
	bool canMutateSplit() const;
	//����
	std::tuple<Path, double> mutate() const;
	//��Ȩֵ��¼��ɫ
	void record(BitmapArray& barr, double w);
	//�����ȳ��Ը���
	double queryInitLuminianceDivProbability();

	//������
	static double debugQueryLuminianceInImage(RayTracing* r, double x, double y);
	//��ѯ·������
	int debugQueryDiffuseTimes();
	//��ѯ�ӵ�·������
	int debugEyeDiffuseTimes();
	//������
	void debugMutateRotate() const;

	//��ȡ·����ͼ�ڵ�����
	std::tuple<int, int> queryImagePos();
};