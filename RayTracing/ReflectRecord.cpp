#include "stdafx.h"
#include "ReflectRecord.h"
#include "Object.h"

void ReflectRecord::generateDiffuse()
{
	randomProbability = face->objectp->kdL;
	type = diffuse;

	double u = (double)rand() / RAND_MAX;	//按面积取样 TODO tmp
	double phi = rand() * PI * 2 / RAND_MAX;
	double z = u, t = sqrt(1 - u*u);
	double x = t*cos(phi), y = t*sin(phi);
	randomProbability *= 0.5 / PI;

	luminiance = z * face->objectp->kd;

	//旋转到法向量方向
	Point nv = face->getNormalVector(hitpoint);
	//double alpha = asin(-nv.y);
	//double beta = atan2(nv.x, nv.z);

	outdir = Point(x, y, z).rotate(nv);
}

void ReflectRecord::makeDiffuse(const Point & _outdir)
{
	outdir = _outdir;

	randomProbability = face->objectp->kdL;
	type = diffuse;

	Point nv = face->getNormalVector(hitpoint);

	double z = dot(nv, outdir);

	if (z <= eps) {
		randomProbability = -1;
		return;
	}

	randomProbability *= 0.5 / PI;
	luminiance = z * face->objectp->kd;
}

void ReflectRecord::makeEye(const Point &_outdir)
{
	randomProbability = 3.8421 / pow(_outdir.z, 3);
	luminiance = Color{ 1, 1, 1 } / pow(_outdir.z, 3);
	outdir = _outdir;
}

void ReflectRecord::makeLight(const Point & _outdir)
{
	randomProbability = 0.25 / PI;
	luminiance = Color{ 1,1,1 };
	outdir = _outdir;
}

void ReflectRecord::generateSpecular()
{
	randomProbability = face->objectp->ksL;
	type = specular;

	luminiance = face->objectp->ks;

	Point nv = face->getNormalVector(hitpoint);
	double costheta = dot(nv, indir);
	outdir = -costheta * nv * 2 + indir;
}

void ReflectRecord::generateRefractive()
{
	randomProbability = face->objectp->tfL;
	type = refractive;

	luminiance = face->objectp->tf;
	Point nv = face->getNormalVector(hitpoint);
	double costheta = -dot(nv, indir);

	if (costheta > 0) {	//外向里
		double sintheta = sqrt(1 - costheta * costheta);
		double theta = acos(costheta);
		double phi = asin(sintheta * face->objectp->Ni);
		double gamma = theta - phi;

		double a = face->objectp->Ni;
		double b = sin(gamma) / sin(theta);

		outdir = indir * a - nv * b;
	}
	else {
		double sintheta = sqrt(1 - costheta * costheta);
		double theta = acos(costheta);
		double sinphi = sintheta / face->objectp->Ni;
		if (sinphi > 1) sinphi = 1;
		double phi = asin(sinphi);
		double gamma = phi - theta;

		double a = sin(phi) / sin(theta);
		double b = sin(gamma) / sin(theta);

		outdir = indir * a + nv * b;
	}
}

ReflectRecord ReflectRecord::randomReflect(const Face * _face, const Point & _indir, const Point & _hitPoint)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;

	double kdL = now.face->objectp->kdL;
	double ksL = now.face->objectp->ksL;
	double tmp = rand();
	double kdlimit = RAND_MAX * kdL;
	double kslimit = kdlimit + RAND_MAX * ksL;

	if (tmp <= kdlimit) {
		now.generateDiffuse();
	}else if(tmp <= kslimit){
		now.generateRefractive();
	}else {
		now.generateRefractive();
	}
	return now;
}

//ReflectRecord ReflectRecord::adjustReflect(const Face * _face, const Point & _indir, const Point & _hitPoint, const ReflectRecord & tar)
//{
//	ReflectRecord now;
//	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;
//
//	if (tar.type == diffuse) {
//		now.makeDiffuse(tar.outdir);
//	} else if (tar.type == specular) {
//		now.generateRefractive();
//	}else if(tar.type == refractive){
//		now.generateRefractive();
//	}else if (tar.type == eye) {
//		now.makeEye(tar.outdir);
//	}else if (tar.type == light) {
//		now.makeLight(tar.outdir);
//	}
//	return now;
//}
//
//ReflectRecord ReflectRecord::adjustDiffuse(const Face * _face, const Point & _indir, const Point & _hitPoint, const Point & _outdir)
//{
//	ReflectRecord now;
//	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;
//	
//	if (tar.type == diffuse) {
//		now.makeDiffuse(tar.outdir);
//	}else if (tar.type == eye) {
//		now.makeEye(tar.outdir);
//	}else if (tar.type == light) {
//		now.makeLight(tar.outdir);
//	}
//	return now;
//}

void ReflectRecord::adjustDiffuse(const Point & _outdir)
{
	if (type == diffuse) {
		makeDiffuse(_outdir);
	}else if (type == eye) {
		makeEye(_outdir);
	}else if (type == light) {
		makeLight(_outdir);
	}
}

HalfReflectRecord ReflectRecord::makeHalfOut() const
{
	return HalfReflectRecord{face, outdir, hitpoint};
}

double ReflectRecord::queryOutCos() const
{
	if (!face) return 1;
	Point nv = face->getNormalVector(hitpoint);
	return dot(nv, outdir);
}

void ReflectRecord::reverse()
{
	Point tmp = indir;
	indir = -outdir;
	makeDiffuse(-tmp);
}
