#include "stdafx.h"
#include "MUtility.h"
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
	double dis = 0;

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
		
		dis += abs(nextReflect.hitpoint - now.hitpoint);
		//double dis2 = norm(nextReflect.hitpoint - now.hitpoint) + MinDistance;

		if (nextReflect.type == ReflectRecord::diffuse) {
			//luminiance *= 1. / dis2;
			//randomProbability *= 1. / dis2;

			double dis2 = dis * dis + MinDistance; //tmp
			luminiance *= 1 / dis2;
			randomProbability /= dis2;

			endR.face = nextReflect.face;
			endR.dir = nextReflect.indir;
			endR.hitpoint = nextReflect.hitpoint;
			return nextReflect;
		}else {
			//assert(inner.size() <= 2);
			luminiance *= nextReflect.luminiance;
			randomProbability *= nextReflect.randomProbability;

			now.face = nextReflect.face;
			now.dir = nextReflect.outdir;
			now.hitpoint = nextReflect.hitpoint;
			inner.push_back(std::move(nextReflect));
		}
	}
}

ReflectRecord SubPath::extendAdjust(const HalfReflectRecord &_start, const SubPath &ori)
{
	startR = _start;

	const KDtree* tree = rt->queryKDtree();
	int times = 0;
	randomProbability = 1;
	luminiance = Color{ 1,1,1 };

	HalfReflectRecord now = startR;
	double dis = 0;

	int pos = 0;
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

		if (pos == ori.inner.size()) {
			auto nextReflect = ReflectRecord::randomDiffuse(std::get<0>(nextpos), now.dir, std::get<1>(nextpos));
			assert(!(isnan(nextReflect.outdir.x) || isnan(nextReflect.outdir.y) || isnan(nextReflect.outdir.z)));
			dis += abs(nextReflect.hitpoint - now.hitpoint);

			double dis2 = dis * dis + MinDistance; //tmp
			luminiance *= 1 / dis2;
			randomProbability /= dis2;

			endR.face = nextReflect.face;
			endR.dir = nextReflect.indir;
			endR.hitpoint = nextReflect.hitpoint;
			return nextReflect;
		} else {
			auto nextReflect = ReflectRecord::adjustReflect(std::get<0>(nextpos), now.dir, std::get<1>(nextpos), ori.inner[pos++]);
			if (nextReflect.randomProbability <= eps) {
				randomProbability = -1;
				return ReflectRecord();
			}
			assert(!(isnan(nextReflect.outdir.x) || isnan(nextReflect.outdir.y) || isnan(nextReflect.outdir.z)));
			dis += abs(nextReflect.hitpoint - now.hitpoint);

			luminiance *= nextReflect.luminiance;
			randomProbability *= nextReflect.randomProbability;

			now.face = nextReflect.face;
			now.dir = nextReflect.outdir;
			now.hitpoint = nextReflect.hitpoint;
			inner.push_back(std::move(nextReflect));
		}

	}
}

bool SubPath::checkShadow(const HalfReflectRecord & start, const HalfReflectRecord &end)
{
	const KDtree* tree = rt->queryKDtree();
	
	//注意直接连到光源的情况
	if (!end.face) {
		auto nextpos = tree->query(end.hitpoint, -start.dir, end.face);
		if (std::get<0>(nextpos) != start.face) return false;
	} else {
		auto nextpos = tree->query(start.hitpoint, start.dir, start.face);
		if (std::get<0>(nextpos) != end.face) return false;
	}
	startR = start;
	endR.face = end.face;
	endR.hitpoint = end.hitpoint;
	endR.dir = start.dir;

	double dis2 = norm(endR.hitpoint - startR.hitpoint) + MinDistance;
	luminiance = Color{ 1,1,1 } / dis2;
	randomProbability = 1 / dis2;
	return true;
}

void SubPath::reverse()
{
	std::swap(startR, endR);
	startR.dir = -startR.dir;
	endR.dir = -endR.dir;

	luminiance = Color{ 1,1,1 };
	randomProbability = 1;

	std::reverse(inner.begin(), inner.end());

	double dis = 0;
	Point lastPoint = startR.hitpoint;
	for (auto &x : inner) {
		x.reverse();
		luminiance *= x.luminiance;
		randomProbability *= x.randomProbability;
		dis += abs(x.hitpoint - lastPoint);
		lastPoint = x.hitpoint;
	}
	dis += abs(endR.hitpoint - lastPoint);
	double dis2 = dis * dis + MinDistance;

	randomProbability /= dis2;
	luminiance *= 1 / dis2;
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
	luminiance *= shadowPath.luminiance;

	randomProbability *= shadowEyeBRDF.queryInCos() * shadowLightBRDF.queryInCos();
}

bool Path::checkShadow()
{
	ReflectRecord r1 = std::move(eyeBRDF.back()), r2 = std::move(lightBRDF.back());
	eyeBRDF.pop_back();
	lightBRDF.pop_back();

	double shadowDistance = norm(r2.hitpoint - r1.hitpoint);
	Point dir = (r2.hitpoint - r1.hitpoint) / sqrt(shadowDistance);
	shadowEyeBRDF = r1;
	shadowEyeBRDF.adjustDiffuse(dir);
	shadowLightBRDF = r2;
	shadowLightBRDF.adjustDiffuse(-dir);
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
		path.diffuseAndLightProbability *= 1 - PathDiffuseProbability;

	//int diffuseTimes = 3;
	//双向路径分裂位置采样
	//path.diffuseAndLightProbability /= (diffuseTimes + 1);

	int eyeDiffuseTimes = rand() % (diffuseTimes + 1);
	//int eyeDiffuseTimes = rand() % (diffuseTimes) +1;
	//int diffuseTimes = 2;
	//int eyeDiffuseTimes = 1;

	int directPathNum = 1;

	auto extendPath = [&r, &directPathNum](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF, int times,
		const ReflectRecord &startR)
	{
		vBRDF.push_back(startR);
		for (int i = 0;i < times; i++) {
			SubPath pnow(r);
			ReflectRecord nextBRDF = pnow.extend(vBRDF.back().makeHalfOut());

			if (nextBRDF.randomProbability <= 0) return false;

			if (pnow.inner.size() == 0) directPathNum++;

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

	path.diffuseAndLightProbability *= directPathNum;
	path.calLuminianceAndRandomProbability();

	return path;
}

Path Path::makeRandomPathInImage(RayTracing * r)
{
	Path p = makeRandomPath(r);
	while (p.randomProbability <= 0 || p.queryInitLuminianceDivProbability() < eps) p=makeRandomPath(r);
	return p;
}

std::tuple<Path, double> Path::mutateRotate() const
{
	Path p(*this);
	double pro = 1;

	auto rotateAndAdjust = [&p, &pro](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF) {
		int n = vBRDF.size() - 1;
		while (n > 0 && random_pro(1 - PathMutateRotateThisPointProbability)) {
			//pro *= 1 - PathMutateRotateThisPointProbability;
			n--;
		}
		//if (n > 0) pro *= PathMutateRotateThisPointProbability;

		double phi = normal_distribution(0, 4. / FinalWidth);
		double theta = std::uniform_real_distribution<double>(0, 2 * PI)(random_engine);
		//pro /= 2 * PI;
		double z = cos(phi), y = sin(phi) * sin(theta), x = sin(phi) * cos(theta);

		pro *= vPath[n].randomProbability * vPath[n].endR.queryInCos() * vBRDF[n].randomProbability;

		vBRDF[n].adjustDiffuse(Point(x, y, z).rotate(vBRDF[n].outdir));
		if (vBRDF[n].randomProbability <= 0) return false;

		

		for (int i = n; i < (int)vPath.size(); i++) {
			SubPath pnow(p.rt);
			ReflectRecord nextBRDF = pnow.extendAdjust(vBRDF[i].makeHalfOut(), vPath[i]);
			if (nextBRDF.randomProbability <= 0) return false;


			vPath[i] = std::move(pnow);
			if (i == n) {
				pro /= vPath[n].randomProbability * vPath[n].endR.queryInCos() * vBRDF[n].randomProbability;
			}

			if (i+1 < (int)vBRDF.size()) {
				nextBRDF.adjustDiffuse(vBRDF[i + 1].outdir);
				vBRDF[i + 1] = nextBRDF;
			} else {
				vBRDF.push_back(nextBRDF);
			}
		}
		return true;
	};

	double randomPro = 0.5;
	if (p.eyeBRDF.size() == 0) randomPro = 0;
	if (p.lightBRDF.size() == 0) randomPro = 1;
	if (random_pro(randomPro) == 1) {
		if (!rotateAndAdjust(p.eyePath, p.eyeBRDF)) {
			p.randomProbability = -1;
			return std::make_tuple(p, 0);
		}
		p.lightBRDF.push_back(p.shadowLightBRDF);
	}else{
		if (!rotateAndAdjust(p.lightPath, p.lightBRDF)) {
			p.randomProbability = -1;
			return std::make_tuple(p, 0);
		}
		p.eyeBRDF.push_back(p.shadowEyeBRDF);
	}

	if (!p.checkShadow() || !p.queryInImage()) {
		p.randomProbability = -1;
		return std::make_tuple(p, 0);
	}

	p.calLuminianceAndRandomProbability();

	return std::make_tuple(std::move(p), pro);
}

std::tuple<Path, double> Path::mutateSplit() const
{
	Path p(*this);

	int n = 0,m = 0;
	for (auto &x : eyePath) {
		if (x.inner.size() == 0) n++;
	}
	for(auto &x: lightPath){
		if (x.inner.size() == 0) m++;
	}
	int pos = random_range(0, n + m - 1);
	//double pro = 1. / (n + m);

	if (pos < n) {
		p.lightBRDF.push_back(std::move(p.shadowLightBRDF));
		p.shadowPath.reverse();
		p.lightPath.push_back(std::move(p.shadowPath));

		p.eyeBRDF.push_back(std::move(p.shadowEyeBRDF));
		while (n > pos) {
			if (p.eyePath.back().inner.size() == 0) n--;
			if (n == pos) break;
			p.lightBRDF.push_back(std::move(p.eyeBRDF.back()));
			p.lightBRDF.back().reverse();
			p.eyeBRDF.pop_back();
			p.lightPath.push_back(std::move(p.eyePath.back()));
			p.lightPath.back().reverse();
			p.eyePath.pop_back();
		}
		
		p.shadowLightBRDF = std::move(p.eyeBRDF.back());
		p.eyeBRDF.pop_back();
		p.shadowLightBRDF.reverse();
		p.shadowPath = std::move(p.eyePath.back());
		p.eyePath.pop_back();
		p.shadowEyeBRDF = std::move(p.eyeBRDF.back());
		p.eyeBRDF.pop_back();

		assert(p.shadowPath.inner.size() == 0);
	} else {
		p.eyeBRDF.push_back(std::move(p.shadowEyeBRDF));
		p.eyePath.push_back(std::move(p.shadowPath));

		p.lightBRDF.push_back(std::move(p.shadowLightBRDF));
		while (n <= pos) {
			if (p.lightPath.back().inner.size() == 0) n++;
			if (n == pos+1) break;
			p.eyeBRDF.push_back(std::move(p.lightBRDF.back()));
			p.eyeBRDF.back().reverse();
			p.lightBRDF.pop_back();
			p.eyePath.push_back(std::move(p.lightPath.back()));
			p.eyePath.back().reverse();
			p.lightPath.pop_back();
		}
		p.shadowEyeBRDF = std::move(p.lightBRDF.back());
		p.lightBRDF.pop_back();
		p.shadowEyeBRDF.reverse();

		p.shadowPath = std::move(p.lightPath.back());
		p.lightPath.pop_back();
		p.shadowPath.reverse();
		p.shadowLightBRDF = std::move(p.lightBRDF.back());
		p.lightBRDF.pop_back();

		assert(p.shadowPath.inner.size() == 0);
	}

	double tmp = queryLuminiance(p.luminiance);
	p.calLuminianceAndRandomProbability();
	double tmp1 = queryLuminiance(p.luminiance);
	assert(abs(tmp1 - tmp) < eps);
	return std::make_tuple(std::move(p), 1);
}

bool Path::canMutateSplit() const
{
	for (auto &x : eyePath) {
		if (x.inner.size() == 0) return true;
	}
	for (auto &x : lightPath) {
		if (x.inner.size() == 0) return true;
	}
	return false;
}

std::tuple<Path, double> Path::mutate() const
{
	double proRotate = PathMutateRotateStrategyProbability;
	double proSplit = canMutateSplit() ? PathMutateSplitStrategyProbability : 0;
	double proRandom = PathMutateRandomStrategyProbability;
	double sum = proRotate + proSplit + proRandom;

	if (random_pro(proRandom / sum)) {
		Path p = makeRandomPath(rt);
		double pro = queryLuminiance(p.luminiance) * randomProbability * diffuseAndLightProbability
			/ queryLuminiance(luminiance) / p.randomProbability / p.diffuseAndLightProbability;
		if (pro > 1) pro = 1;
		if (p.randomProbability <= 0) pro = 0;

		//if(pro > 0)p.debugMutateRotate();  //tmp

		return std::make_tuple(std::move(p), pro);
	}else if(random_pro(proRotate / (proRotate + proSplit))) {
		auto tmp = mutateRotate();
		Path &p = std::get<0>(tmp);
		double mpro = std::get<1>(tmp);
		/*double pro = queryLuminiance(p.luminiance) *
			(randomProbability * diffuseAndLightProbability * proRandom / sum + mpro * selectPro)
			/ queryLuminiance(luminiance) /
			(p.randomProbability * p.diffuseAndLightProbability * proRandom / sum + mpro * selectPro);*/
		//double pro = queryLuminiance(p.luminiance) / queryLuminiance(luminiance);
		//double pro = queryLuminiance(p.luminiance) / queryLuminiance(luminiance) * mpro;

		double pro = queryLuminiance(p.luminiance) * randomProbability * diffuseAndLightProbability
			/ queryLuminiance(luminiance) / p.randomProbability / p.diffuseAndLightProbability;

		//if (abs(pro - pro2)/pro2 > 0.1)
		//	std::cerr << pro << " " << pro2 << "!";
		if (pro > 1) pro = 1;
		if (p.randomProbability <= 0) pro = 0;

		//if(pro > 0)p.debugMutateRotate(); //tmp

		return std::make_tuple(std::move(p), pro);
	}else{
		auto tmp = mutateSplit();
		Path &p = std::get<0>(tmp);
		double mpro = std::get<1>(tmp);
		//double pro = queryLuminiance(p.luminiance) * randomProbability * diffuseAndLightProbability
		//	/ queryLuminiance(luminiance) / p.randomProbability / p.diffuseAndLightProbability;
		return std::make_tuple(std::move(p), 1);
	}
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

	//if (abs(x - 185) <= 10 && abs(y - 77) <= 10)
	//if (abs(x - 125)<=10 && abs(y- 74)<=10)
	//	std::cerr << "!";
	//else
	//	std::cerr << "#";

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
	eye.type = ReflectRecord::eye;
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

void Path::debugMutateRotate() const
{
	Path p(*this);
	double pro = 0.5;

	auto rotateAndAdjust = [&p, &pro](std::vector<SubPath> &vPath, std::vector<ReflectRecord> &vBRDF) {
		int n = vBRDF.size() - 1;
		while (n > 0 && random_pro(1 - PathMutateRotateThisPointProbability)) {
			pro *= 1 - PathMutateRotateThisPointProbability;
			n--;
		}
		if (n > 0) pro *= PathMutateRotateThisPointProbability;

		double phi = 0;
		double theta = 0;
		pro /= 2 * PI;
		double z = cos(phi), y = sin(phi) * sin(theta), x = sin(phi) * cos(theta);

		double tmpPro = vBRDF[n].randomProbability;
		Color tmpL = vBRDF[n].luminiance;
		vBRDF[n].adjustDiffuse(Point(x, y, z).rotate(vBRDF[n].outdir));

		assert(abs(tmpPro - vBRDF[n].randomProbability)<eps && abs(tmpL.x - vBRDF[n].luminiance.x) < eps);
		if (vBRDF[n].randomProbability <= 0) return false;

		for (int i = n; i < (int)vPath.size(); i++) {
			SubPath pnow(p.rt);
			ReflectRecord nextBRDF = pnow.extend(vBRDF[i].makeHalfOut());
			if (nextBRDF.randomProbability <= 0) return false;

			tmpPro = vPath[i].randomProbability;
			tmpL = vPath[i].luminiance;
			vPath[i] = std::move(pnow);
			assert(abs(tmpPro - vPath[i].randomProbability)<eps && abs(tmpL.x - vPath[i].luminiance.x) < eps);

			if (i + 1 < (int)vBRDF.size()) {

				tmpPro = vBRDF[i+1].randomProbability;
				tmpL = vBRDF[i+1].luminiance;
				nextBRDF.adjustDiffuse(vBRDF[i + 1].outdir);
				assert(abs(tmpPro - vBRDF[i + 1].randomProbability)<eps && abs(tmpL.x - vBRDF[i + 1].luminiance.x) < eps);
				vBRDF[i + 1] = nextBRDF;
			}
			else {
				vBRDF.push_back(nextBRDF);
			}
		}
		return true;
	};

	if (p.eyeBRDF.size() == 0) pro = 0;
	if (p.lightBRDF.size() == 0) pro = 1;
	if (random_pro(pro) == 1) {
		if (!rotateAndAdjust(p.eyePath, p.eyeBRDF)) {
			p.randomProbability = -1;
			return;
		}
		p.lightBRDF.push_back(p.shadowLightBRDF);
	}
	else {
		if (!rotateAndAdjust(p.lightPath, p.lightBRDF)) {
			p.randomProbability = -1;
			return;
		}
		p.eyeBRDF.push_back(p.shadowEyeBRDF);
	}

	if (!p.checkShadow() || !p.queryInImage()) {
		p.randomProbability = -1;
		return;
	}

	double tmpPro = randomProbability;
	Color tmpL = luminiance;
	p.calLuminianceAndRandomProbability();
	assert(abs(tmpPro - randomProbability)<eps && abs(tmpL.x - luminiance.x) < eps);

	return;
}


