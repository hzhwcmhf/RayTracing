#pragma once

#include "Face.h"

struct HalfReflectRecord
{
	const Face* face;
	Point dir, hitpoint;
	HalfReflectRecord(const Face* _face, const Point &_dir, const Point &_hitpoint) :
		face(_face), dir(_dir), hitpoint(_hitpoint)
	{}
	HalfReflectRecord() {}

	double queryInCos() const;
};

class ReflectRecord
{
public:
	enum ReflectType
	{
		diffuse, specular, refractive, eye, light
	}type;
	const Face* face;
	Point indir, hitpoint, outdir;

	//����cos * ������� * �������͸���
	double randomProbability;

	//����cos * ���cos * BSDF
	Color luminiance;

private:
	void generateDiffuse();
	void makeDiffuse(const Point &_outdir);
	void makeEye(const Point &_outdir);
	void makeLight(const Point &_outdir);
	void generateSpecular();
	void generateRefractive();


public:
	static ReflectRecord randomReflect(const Face* _face, const Point &_indir, const Point &_hitPoint);
	//����ʧ�� ʧ��ʱrandomProbability��Ϊ����
	
	//static ReflectRecord adjustReflect(const Face* _face, const Point &_indir, const Point &_hitPoint, const ReflectRecord &tar);
	//static ReflectRecord adjustDiffuse(const Face* _face, const Point &_indir, const Point &_hitPoint, const Point &_outdir);
	void adjustDiffuse(const Point &_outdir);

	HalfReflectRecord makeHalfOut() const;
	//double queryOutCos() const;
	double queryInCos() const;

	void reverse();
};