#include "stdafx.h"
#include "Path.h"
#include "RayTracing.h"

SubPath::SubPath(RayTracing* r)
{
	rt = r;
}

ReflectRecord SubPath::extend(const HalfReflectRecord &_start)
{
	startR = _start;
	return extend();
}

ReflectRecord SubPath::extend()
{
	const KDtree* tree = rt->queryKDtree();
	int times = 0;
	
	HalfReflectRecord now = startR;
	while (true) {
		if (++times > SubpathMaxSpecularTimes) {
			randomProbability = -1;
			return ReflectRecord();
		}
		auto nextpos = tree->query(now.hitpoint, now.dir, now.face);
		auto nextReflect = ReflectRecord::randomReflect(std::get<0>(nextpos), now.dir, std::get<1>(nextpos));
		if (nextReflect.type == ReflectRecord::diffuse) {
			endR.face = nextReflect.face;
			endR.dir = nextReflect.indir;
			endR.hitpoint = nextReflect.hitpoint;
			return nextReflect;
		}else {
			now.face = nextReflect.face;
			now.dir = nextReflect.outdir;
			now.hitpoint = nextReflect.hitpoint;
			luminiance *= nextReflect.luminiance;
			randomProbability *= nextReflect.randomProbability;
			inner.push_back(std::move(nextReflect));
		}
	}
}

bool SubPath::checkShadow(const HalfReflectRecord & start, const Face* endFace)
{
	const KDtree* tree = rt->queryKDtree();
	auto nextpos = tree->query(start.hitpoint, start.dir, start.face);
	if (std::get<0>(nextpos) != endFace) return false;
	startR = start;
	endR.face = std::get<0>(nextpos);
	endR.hitpoint = std::get<1>(nextpos);
	endR.dir = start.dir;
	return true;
}



void Path::calLuminianceAndRandomProbability()
{
	randomProbability = 1;
	luminiance = Color{ 1,1,1 };
	for (auto& i : eyeBRDF) {
		randomProbability *= i.randomProbability;
		luminiance *= i.luminiance;
	}
	for (auto& i : lightBRDF) {
		randomProbability *= i.randomProbability;
		luminiance *= i.luminiance;
	}
	for (auto& i : eyePath) {
		randomProbability *= i.randomProbability;
		luminiance *= i.luminiance;
	}
	for (auto& i : lightPath) {
		randomProbability *= i.randomProbability;
		luminiance *= i.luminiance;
	}
	luminiance *= shadowEyeBRDF.luminiance;
	luminiance *= shadowLightBRDF.luminiance;
	luminiance *= 1 / shadowDistance;
}

bool Path::checkShadow()
{
	ReflectRecord& r1 = eyeBRDF.back(), &r2 = lightBRDF.back();
	shadowDistance = norm(r2.hitpoint - r1.hitpoint);
	Point dir = (r2.hitpoint - r1.hitpoint) / sqrt(shadowDistance);
	shadowEyeBRDF = ReflectRecord::makeDiffuse(r1.face, r1.indir, r1.hitpoint, dir);
	shadowLightBRDF = ReflectRecord::makeDiffuse(r2.face, r2.indir, r2.hitpoint, -dir);
	if(!shadowPath.checkShadow(shadowEyeBRDF.makeHalfOut(), r2.face)) return false;
	
	eyeBRDF.pop_back();
	lightBRDF.pop_back();
	return true;
}

Path::Path(RayTracing * r) : rt(r), shadowPath(r) {}

Path Path::makeRandomPath(RayTracing * r)
{
	Path path(r);
	path.diffuseAndLightProbability = 1;

	ReflectRecord eye = r->queryEye();
	ReflectRecord light = r->queryLight();

	//漫反射次数采样
	int diffuseTimes = 1;
	path.diffuseAndLightProbability *= PathDiffuseProbability;
	while (diffuseTimes < PathMaxDiffuseTimes) {
		if (rand() > PathDiffuseProbability * RAND_MAX) break;
		diffuseTimes++;
		path.diffuseAndLightProbability *= PathDiffuseProbability;
	}
	//双向路径分裂位置采样
	path.diffuseAndLightProbability /= diffuseTimes;

	int eyeDiffuseTimes = rand() % diffuseTimes;

	auto extendPath = [&r](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF, int times,
		const ReflectRecord &startR)
	{
		vBRDF.push_back(startR);
		for (int i = 0;i < times; i++) {
			SubPath pnow(r);
			ReflectRecord nextBRDF = pnow.extend(vBRDF.back().makeHalfOut());

			if (nextBRDF.randomProbability < 0) return false;

			vPath.push_back(std::move(pnow));
			vBRDF.push_back(nextBRDF);
		}
		return true;
	};

	if (!extendPath(path.eyePath, path.eyeBRDF, eyeDiffuseTimes, eye)) {
		path.randomProbability = -1;
		return path;
	}
	if (!extendPath(path.lightPath, path.lightBRDF, diffuseTimes - eyeDiffuseTimes, light)) {
		path.randomProbability = -1;
		return path;
	}

	if (!path.checkShadow()) {
		path.randomProbability = -1;
		return path;
	}

	path.calLuminianceAndRandomProbability();

	return path;
}

void Path::record(BitmapArray & barr, double w)
{
	if (w < eps) return;
	int x, y;
	if (eyePath.size()) {
		std::tie(x, y) = rt->queryImagePos(eyePath.front().startR.dir);
	}else{
		std::tie(x, y) = rt->queryImagePos(shadowPath.startR.dir);
	}
	assert(x >= 0 && x < FinalWidth && y >= 0 && y < FinalHeight);
	barr[x][y].r += w * luminiance.x;
	barr[x][y].g += w * luminiance.y;
	barr[x][y].b += w * luminiance.z;
}

double Path::queryInitLuminianceDivProbability()
{
	double lu = queryLuminiance(luminiance);
	return (lu < eps || randomProbability < 0) ? 0 : lu / randomProbability / diffuseAndLightProbability;
}


