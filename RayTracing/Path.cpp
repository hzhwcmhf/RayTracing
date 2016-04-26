#include "stdafx.h"
#include "Path.h"
#include "RayTracing.h"

SubPath::SubPath(RayTracing* r)
{
	rt = r;
}



Path Path::makeRandomPath(RayTracing * r)
{
	Path path;
	path.rt = r;
	path.diffuseAndLightProbability = 1;

	std::vector<SubPath> eyePath;
	std::vector<SubPath> lightPath;
	std::vector<BRDF*> eyePathBRDF;
	std::vector<BRDF*> lightPathBRDF;


	std::tuple<Point, BRDF*> eye = r->queryEye();
	const std::vector<std::tuple<Point, BRDF*>> &lights = r->queryLights();
	std::tuple<Point, BRDF*> light = lights[rand() % lights.size()];
	path.diffuseAndLightProbability /= lights.size();

	int diffuseTimes = 1;
	while (diffuseTimes < PathMaxDiffuseTimes) {
		if (rand() > PathDiffuseProbability * RAND_MAX) break;
		diffuseTimes++;
		path.diffuseAndLightProbability *= PathDiffuseProbability;
	}

	int eyeDiffuseTimes = rand() % diffuseTimes;

	for (int i = 0;i < eyeDiffuseTimes; i++) {
		SubPath pnow(r);
		if (i == 0) {
			pnow.startPos = std::get<0>(eye);
			BRDF* brdf = std::get<1>(eye);
			pnow.startDir = brdf->generateDirection(pnow.startPos, Point());
		}else{
			pnow.startPos = eyePath.back().endPos;
			pnow.startDir = eyePathBRDF.back()->generateDirection(pnow.startPos, eyePath.back().endDir);
		}
		Face* nextFace = pnow.extend();

		eyePath.push_back(std::move(pnow));
		eyePathBRDF.push_back(nextFace->queryBRDF());
	}

	for (int i = 0;i < diffuseTimes - eyeDiffuseTimes; i++) {
		SubPath pnow(r);
		if (i == 0) {
			pnow.startPos = std::get<0>(light);
			BRDF* brdf = std::get<1>(light);
			pnow.startDir = brdf->generateDirection(pnow.startPos, Point());
		}else{
			pnow.startPos = lightPath.back().endPos;
			pnow.startDir = lightPathBRDF.back()->generateDirection(pnow.startPos, lightPath.back().endDir);
		}
		Face* nextFace = pnow.extend();

		lightPath.push_back(std::move(pnow));
		eyePathBRDF.push_back(nextFace->queryBRDF());
	}

	SubPath shadowTest(r);
	shadowTest.startPos = eyePath.back().endPos;
	shadowTest.endPos = lightPath.back().endPos;
	shadowTest.checkShadow();

	path.subpaths.insert(path.subpaths.end(), eyePath.begin(), eyePath.end());
	path.subpaths.push_back(std::move(shadowTest));
	for (auto &i : lightPath) {
		i.reverse();
	}
	reverse(lightPath.begin(), lightPath.end());
	path.subpaths.insert(path.subpaths.end(), lightPath.begin(), lightPath.end());

	path.brdfs.insert(path.brdfs.begin(), eyePathBRDF.begin(), eyePathBRDF.end());
	reverse(lightPathBRDF.begin(), lightPathBRDF.end());
	path.brdfs.insert(path.brdfs.end(), lightPathBRDF.begin(), lightPathBRDF.end());

	return path;
}


