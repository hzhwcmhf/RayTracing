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
	randomProbability = 1;
	luminiance = Color{ 1,1,1 };

	HalfReflectRecord now = startR;
	while (true) {
		if (++times > SubpathMaxSpecularTimes) {
			randomProbability = -1;
			return ReflectRecord();
		}
		auto nextpos = tree->query(now.hitpoint, now.dir, now.face);
		if (!std::get<0>(nextpos)) {
			randomProbability = -1;
			return ReflectRecord();
		}
		auto nextReflect = ReflectRecord::randomReflect(std::get<0>(nextpos), now.dir, std::get<1>(nextpos));
		assert(!(isnan(nextReflect.outdir.x) || isnan(nextReflect.outdir.y) || isnan(nextReflect.outdir.z)));
		
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

bool SubPath::checkShadow(const HalfReflectRecord & start, const HalfReflectRecord &end)
{
	const KDtree* tree = rt->queryKDtree();
	auto nextpos = tree->query(start.hitpoint, start.dir, start.face);
	//TODO tmp
	if (end.face && std::get<0>(nextpos) != end.face) return false; //注意直接连到光源的情况
	startR = start;
	endR.face = end.face;
	endR.hitpoint = end.hitpoint;
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
	//randomProbability *= shadowDistance;
}

bool Path::checkShadow()
{
	ReflectRecord r1 = std::move(eyeBRDF.back()), r2 = std::move(lightBRDF.back());
	eyeBRDF.pop_back();
	lightBRDF.pop_back();

	shadowDistance = norm(r2.hitpoint - r1.hitpoint) + MinShadowDistance;//控制距离，太近的话微元取得太大导致奇异
	//if (shadowDistance < MinShadowDistance) shadowDistance = MinShadowDistance;	
	Point dir = (r2.hitpoint - r1.hitpoint) / sqrt(shadowDistance);
	shadowEyeBRDF = ReflectRecord::adjustDiffuse(r1.face, r1.indir, r1.hitpoint, dir);
	shadowLightBRDF = ReflectRecord::adjustDiffuse(r2.face, r2.indir, r2.hitpoint, -dir);
	if (shadowEyeBRDF.randomProbability <= 0 || shadowLightBRDF.randomProbability <= 0) return false;
	if(!shadowPath.checkShadow(shadowEyeBRDF.makeHalfOut(), shadowLightBRDF.makeHalfOut())) return false;
	
	return true;
}

std::tuple<int, int> Path::queryImagePos()
{
	if (eyePath.size()) {
		return rt->queryCamera()->queryPos(eyePath.front().startR.dir);
	} else {
		return rt->queryCamera()->queryPos(shadowPath.startR.dir);
	}
}

bool Path::queryInImage()
{
	if (eyePath.size()) {
		return rt->queryCamera()->queryInImage(eyePath.front().startR.dir);
	} else {
		return rt->queryCamera()->queryInImage(shadowPath.startR.dir);
	}
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
	while (diffuseTimes < PathMaxDiffuseTimes) {
		if (rand() > PathDiffuseProbability * RAND_MAX) break;
		diffuseTimes++;
		path.diffuseAndLightProbability *= PathDiffuseProbability;
	}
	if (diffuseTimes != PathMaxDiffuseTimes)
		path.diffuseAndLightProbability *= 1 - PathDiffuseProbability;;
	//双向路径分裂位置采样
	path.diffuseAndLightProbability /= diffuseTimes;

	int eyeDiffuseTimes = rand() % diffuseTimes + 1;

	//int diffuseTimes = 2;
	//int eyeDiffuseTimes = 2;

	auto extendPath = [&r](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF, int times,
		const ReflectRecord &startR)
	{
		vBRDF.push_back(startR);
		for (int i = 0;i < times; i++) {
			SubPath pnow(r);
			ReflectRecord nextBRDF = pnow.extend(vBRDF.back().makeHalfOut());

			if (nextBRDF.randomProbability <= 0) return false;

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

	if (!path.checkShadow() || !path.queryInImage()) {
		path.randomProbability = -1;
		return path;
	}

	path.calLuminianceAndRandomProbability();

	return path;
}

Path Path::makeRandomPathInImage(RayTracing * r)
{
	Path p = makeRandomPath(r);
	while (p.randomProbability <= 0) p=makeRandomPath(r);

	return p;
}

std::tuple<Path, double> Path::mutate()
{
	Path p = makeRandomPath(rt);
	
	double pro = queryLuminiance(p.luminiance) * randomProbability * diffuseAndLightProbability
		/ queryLuminiance(luminiance) / p.randomProbability / p.diffuseAndLightProbability;
	if (pro > 1) pro = 1;
	if (p.randomProbability <= 0) pro = 0;
	return std::make_tuple(std::move(p), pro);
}

void Path::record(BitmapArray & barr, double w)
{
	if (w < eps) return;
	int x, y;
	std::tie(x, y) = queryImagePos();
	assert(!(isnan(luminiance.x) || isnan(luminiance.y) || isnan(luminiance.z)));

	assert(x >= 0 && x < FinalWidth && y >= 0 && y < FinalHeight);
	double lu = queryLuminiance(luminiance);

	assert(lu > 0);

	barr[x][y].r += w * luminiance.x / lu;
	barr[x][y].g += w * luminiance.y / lu;
	barr[x][y].b += w * luminiance.z / lu;
}

double Path::queryInitLuminianceDivProbability()
{
	double lu = queryLuminiance(luminiance);
	return (lu < eps || randomProbability <= 0) ? 0 : lu / randomProbability / diffuseAndLightProbability;
}

double Path::debugQueryLuminianceInImage(RayTracing *r, double x, double y)
{
	Path path(r);
	path.diffuseAndLightProbability = 1;

	ReflectRecord eye;
	eye.type = ReflectRecord::eyeOrLight;
	eye.indir = Point(0, 0, 0);
	eye.hitpoint = Point(0, 0, 0);

	eye.outdir = r->queryCamera() ->generateDir(x, y);

	eye.luminiance = Color(1, 1, 1);
	eye.randomProbability = 1;
	eye.face = nullptr;

	ReflectRecord light = r->queryLight();

	//漫反射次数采样
	int diffuseTimes = 1;
	
	int eyeDiffuseTimes = 1;

	auto extendPath = [&r](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF, int times,
		const ReflectRecord &startR)
	{
		vBRDF.push_back(startR);
		for (int i = 0;i < times; i++) {
			SubPath pnow(r);
			ReflectRecord nextBRDF = pnow.extend(vBRDF.back().makeHalfOut());

			if (nextBRDF.randomProbability <= 0) return false;

			vPath.push_back(std::move(pnow));
			vBRDF.push_back(nextBRDF);
		}
		return true;
	};

	if (!extendPath(path.eyePath, path.eyeBRDF, eyeDiffuseTimes, eye)) {
		path.randomProbability = -1;
		return -1;
	}
	if (!extendPath(path.lightPath, path.lightBRDF, diffuseTimes - eyeDiffuseTimes, light)) {
		path.randomProbability = -1;
		return -1;
	}

	if (!path.checkShadow() || !path.queryInImage()) {
		path.randomProbability = -1;
		return -1;
	}

	path.calLuminianceAndRandomProbability();

	return queryLuminiance(path.luminiance);
}

int Path::debugQueryDiffuseTimes()
{
	return eyeBRDF.size() + lightBRDF.size();
}

int Path::debugEyeDiffuseTimes()
{
	return eyeBRDF.size();
}


