#include "stdafx.h"
#include "ReflectRecord.h"
#include "Object.h"

void ReflectRecord::generateDiffuse()
{
	randomProbability = face->objectp->kdL;
	type = diffuse;

	Point nv = face->getNormalVector(hitpoint);

	double u = (double)rand() / RAND_MAX;	//按面积取样 TODO：按射出cos取样
	double phi = rand() * PI * 2 / RAND_MAX;
	double z = u, t = sqrt(1 - u*u);
	double x = t*cos(phi), y = t*sin(phi);

	double incos = -dot(nv, indir);
	if (incos < 0) {
		randomProbability = -1;
		return;
	}
	randomProbability *= 0.5 / PI;
	randomProbability *= incos;

	luminiance = z * face->objectp->kd * incos;

	//旋转到法向量方向
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

	double incos = -dot(nv, indir);

	if (incos < 0) {
		randomProbability = -1;
		return;
	}

	randomProbability *= 0.5 / PI;
	randomProbability *= incos;

	luminiance = z * face->objectp->kd * incos;
}

void ReflectRecord::makeEye(const Point &_outdir)
{
	//randomProbability = 3.8421 / 4 / PI / pow(_outdir.z, 3);
	//randomProbability = 0.25 / PI / pow(_outdir.z, 3);
	randomProbability = 0.5 / PI;
	luminiance = Color{ 1, 1, 1 } / pow(_outdir.z, 3);
	outdir = _outdir;
}

void ReflectRecord::makeLight(const Point & _outdir)
{
	//randomProbability = 0.25 / PI;
	//luminiance = Color{ 1,1,1 };
	outdir = _outdir;
	luminiance = Color{ 1,1,1 };
	randomProbability = 1 / PI;
	if (outdir.z > -0.5) luminiance = Color(0, 0, 0);
}

void ReflectRecord::generateSpecular()
{
	randomProbability = face->objectp->ksL;
	type = specular;

	Point nv = face->getNormalVector(hitpoint);
	double costheta = -dot(nv, indir);
	outdir = costheta * nv * 2 + indir;

	//randomProbability *= costheta;
	//luminiance = face->objectp->ks * costheta * costheta;
	luminiance = face->objectp->ks;
}

void ReflectRecord::generateRefractive()
{
	randomProbability = face->objectp->tfL;
	if (randomProbability <= eps) return;
	type = refractive;

	luminiance = face->objectp->tf;
	Point nv = face->getNormalVector(hitpoint);
	double costheta = -dot(nv, indir);

	if (costheta > 0) {	//外向里
		double sintheta = sqrt(1 - costheta * costheta);
		double theta = acos(costheta);
		double phi = asin(sintheta / face->objectp->Ni);
		double gamma = theta - phi;

		double a = 1. / face->objectp->Ni;
		double b = sin(gamma) / sin(theta);

		outdir = indir * a - nv * b;
		//assert(abs(abs(outdir) - 1) < eps);
	}
	else {
		double sintheta = sqrt(1 - costheta * costheta);
		double theta = acos(-costheta);
		double sinphi = sintheta * face->objectp->Ni;
		if (sinphi < 1) {
			//sinphi = 1;
			double phi = asin(sinphi);
			double gamma = phi - theta;

			//double a = sin(phi) / sin(theta);
			//double b = sin(gamma) / sin(theta);
			double a = sinphi / sintheta;
			double b = sin(gamma) / sintheta;

			outdir = indir * a - nv * b;

			//assert(abs(abs(outdir) - 1) < eps);
		} else {
			luminiance = Color{ 1,1,1 };
			outdir = costheta * nv * 2 + indir;
			//assert(abs(abs(outdir) - 1) < eps);
		}
	}

	//randomProbability *= costheta;
	//luminiance *= costheta * (-dot(nv, outdir));
}

ReflectRecord ReflectRecord::randomReflect(const Face * _face, const Point & _indir, const Point & _hitPoint)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;

	double kdL = now.face->objectp->kdL;
	double ksL = now.face->objectp->ksL;
	double tmp = rand();
	double kdlimit = (RAND_MAX+1) * kdL;
	double kslimit = kdlimit + (RAND_MAX+1) * ksL;

	if (tmp < kdlimit) {
		now.generateDiffuse();
	}else if(tmp < kslimit){
		now.generateSpecular();
	}else {
		now.generateRefractive();
	}
	return now;
}

ReflectRecord ReflectRecord::adjustReflect(const Face * _face, const Point & _indir, const Point & _hitPoint, const ReflectRecord & tar)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;

	if (tar.type == specular) {
		now.generateSpecular();
	}
	else if (tar.type == refractive) {
		now.generateRefractive();
	} else {
		assert(false);
	}
	return now;
}

ReflectRecord ReflectRecord::randomDiffuse(const Face * _face, const Point & _indir, const Point & _hitPoint)
{
	ReflectRecord now;
	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;

	if (now.face->objectp->kdL == 0) {
		now.randomProbability = -1;
	} else {
		now.generateDiffuse();
	}

	return now;
}

//ReflectRecord ReflectRecord::adjustDiffuse(const Face * _face, const Point & _indir, const Point & _hitPoint, const Point & _outdir)
//{
//	ReflectRecord now;
//	now.face = _face, now.indir = _indir, now.hitpoint = _hitPoint;
//	
//	now.adjustDiffuse(_outdir);
//
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

//double ReflectRecord::queryOutCos() const
//{
//	if (!face) return 1;
//	Point nv = face->getNormalVector(hitpoint);
//	return dot(nv, outdir);
//}
double ReflectRecord::queryInCos() const
{
	if (!face) return 1;
	Point nv = face->getNormalVector(hitpoint);
	return -dot(nv, indir);
}

void ReflectRecord::reverse()
{
	Point tmp = indir;
	indir = -outdir;
	//makeDiffuse(-tmp);
	outdir = -tmp;
}

double HalfReflectRecord::queryInCos() const
{
	if (!face) return 1;
	Point nv = face->getNormalVector(hitpoint);
	return -dot(nv, dir);
}
