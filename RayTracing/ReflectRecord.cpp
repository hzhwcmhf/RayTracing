#include "stdafx.h"
#include "ReflectRecord.h"
#include "Object.h"

void ReflectRecord::generateDiffuse()
{
	randomProbability = face->objectp->kdL;
	type = diffuse;

	double t = pow((double)rand() / RAND_MAX, 1. / 3); //sqrt(x2+y2) 随机取样 按cos(theta)的概率分布
	double deg = rand() * PI * 2 / RAND_MAX;
	double z = sqrt(1 - t * t);
	double x = cos(deg) * t, y = sin(deg) * t;

	randomProbability *= z;
	luminiance = z * face->objectp->kd;

	//旋转到法向量方向
	Point nv = face->getNormalVector(hitpoint);
	//double alpha = asin(-nv.y);
	//double beta = atan2(nv.x, nv.z);

	double sina = -nv.y, cosa = sqrt(1 - nv.y*nv.y);
	double xz = sqrt(nv.x * nv.x + nv.z * nv.z);
	double sinb = nv.x /xz , cosb = nv.z / xz;
	if (xz < eps) sinb = 0, cosb = 1;
	outdir.x = x * cosb + y * sina * sinb + z * cosa * sinb;
	outdir.y = y * cosa - z * sina;
	outdir.z = -x * sinb + y * sina * cosb + z * cosa * cosb;;
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

	randomProbability *= z;
	luminiance = z * face->objectp->kd;
}

void ReflectRecord::makeEyeOrLight(const Point & _outdir)
{
	randomProbability = 1;
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

ReflectRecord ReflectRecord::adjustReflect(const Face * _face, const Point & _indir, const Point & _hitPoint, const ReflectRecord & tar)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;

	if (tar.type == diffuse) {
		now.makeDiffuse(tar.outdir);
	} else if (tar.type == specular) {
		now.generateRefractive();
	}else if(tar.type == refractive){
		now.generateRefractive();
	}else if (tar.type == eyeOrLight) {
		now.makeEyeOrLight(tar.outdir);
	}
	return now;
}

ReflectRecord ReflectRecord::adjustDiffuse(const Face * _face, const Point & _indir, const Point & _hitPoint, const Point & _outdir)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;
	if (_face) {
		now.makeDiffuse(_outdir);
	} else {
		now.makeEyeOrLight(_outdir);
	}
	return now;
}

HalfReflectRecord ReflectRecord::makeHalfOut() const
{
	return HalfReflectRecord{face, outdir, hitpoint};
}

ReflectRecord ReflectRecord::reverse() const
{
	ReflectRecord now;
	now.type = type;
	now.face = face;
	now.indir = -outdir, now.outdir = -indir;
	now.hitpoint = hitpoint;
	now.randomProbability = randomProbability;
	now.luminiance = luminiance;
	return now;
}
