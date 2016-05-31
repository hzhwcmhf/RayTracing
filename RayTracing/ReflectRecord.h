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

	//射入cos * 出射概率 * 反射类型概率
	double randomProbability;

	//射入cos * 射出cos * BSDF
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
	//可能失败 失败时randomProbability置为负数
	
	//static ReflectRecord adjustReflect(const Face* _face, const Point &_indir, const Point &_hitPoint, const ReflectRecord &tar);
	//static ReflectRecord adjustDiffuse(const Face* _face, const Point &_indir, const Point &_hitPoint, const Point &_outdir);
	void adjustDiffuse(const Point &_outdir);

	HalfReflectRecord makeHalfOut() const;
	//double queryOutCos() const;
	double queryInCos() const;

	void reverse();
};