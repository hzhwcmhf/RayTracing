#pragma once

#include "Face.h"

//记录单侧光线
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

//记录入射出射光线
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
	//随机生成漫反射
	void generateDiffuse();
	//按照给定方向生成漫反射
	void makeDiffuse(const Point &_outdir);
	//按照给定方向生成视点出射
	void makeEye(const Point &_outdir);
	//按照给定方向生成光源出射
	void makeLight(const Point &_outdir);
	//生成镜面反射
	void generateSpecular();
	//生成折射
	void generateRefractive();


public:
	//按照面片属性随机生成反射
	static ReflectRecord randomReflect(const Face* _face, const Point &_indir, const Point &_hitPoint);
	
	//调整出射方向
	//调整可能失败 失败时randomProbability置为负数
	static ReflectRecord adjustReflect(const Face* _face, const Point &_indir, const Point &_hitPoint, const ReflectRecord &tar);

	//随机漫反射
	static ReflectRecord randomDiffuse(const Face* _face, const Point &_indir, const Point &_hitPoint);
	//调整漫反射
	void adjustDiffuse(const Point &_outdir);

	//通过完整的生成出射的HalfReflect
	HalfReflectRecord makeHalfOut() const;

	//double queryOutCos() const;
	//查询射入角cos
	double queryInCos() const;

	//翻转
	void reverse();
};