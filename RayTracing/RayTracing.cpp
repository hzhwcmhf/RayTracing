#include "stdafx.h"
#include "RayTracing.h"

#include "Bitmap.h"
#include "Point.h"
#include "Face.h"
#include "Object.h"
#include "KDtree.h"
#include "MUtility.h"


BitmapArray RayTracing::MLT_process(Path & p)
{
	BitmapArray barr(FinalWidth, FinalHeight);
	
	for (int mt = 0;mt < MutateTimes; mt++) {
		if (mt % 50000 == 0)
			std::cerr << mt << std::endl;
		if (mt == 598)
			std::cerr << "";
		auto tmp = p.mutate();
		auto &p2 = std::get<0>(tmp);
		double pro = std::get<1>(tmp);

		int x, y;
		std::tie(x, y) = p.queryImagePos();
		p.record(barr, (1 - pro) * (*initialWeights)[x][y]);
		if (pro > eps) {
			std::tie(x, y) = p2.queryImagePos();
			p2.record(barr, pro * (*initialWeights)[x][y]);
		}


		//if(p2.queryInitLuminianceDivProbability() > eps)
		//	p2.record(barr, 1);


		//std::cerr << pro << std::endl;

		static int cnt[PathMaxDiffuseTimes + 1][PathMaxDiffuseTimes + 1];
		cnt[p.debugQueryDiffuseTimes()][p.debugEyeDiffuseTimes()] ++;

		if (mt % 200000 == 0) {
			//std::cerr << "!" << cnt10 << " " << cnt11 << " " << cnt2 << " " << cnt3 << " " << cnt4 << std::endl;
			for (int i = 0;i <= PathMaxDiffuseTimes; i++) {
				for (int j = 0;j <= i;j++)
				{
					std::cerr << cnt[i][j] << " ";
				}
				std::cerr << std::endl;
			}
			//cnt10 = cnt11 = cnt2 = cnt3 = cnt4 = 0;
			memset(cnt, 0, sizeof(cnt));
		}
		if (rand() < pro * RAND_MAX) {
			p = std::move(p2);
		}
	}
	
	/*for (int i = 0;i < FinalWidth; i++) {
		for (int j = 0;j < FinalHeight; j++) {
			for (int k = 0; k < 3;k++) {
				if (barr[i][j].c[k] != 0)
					std::cerr << &barr[i][j] - &barr[0][0] << std::endl;
			}
		}
	}*/

	return barr;
}

RayTracing::RayTracing()
{
	initialWeights = new std::array<std::array<double, FinalWidth>, FinalHeight>();
}

RayTracing::~RayTracing()
{
	for (auto x : vecObjects) {
		delete x;
	}
	delete initialWeights;
}

Bitmap RayTracing::metropisLightTransport()
{
	//direct light
	for (int i = 0;i < FinalWidth; i++) {
		for (int j = 0;j < FinalHeight; j++) {
			Point dir = camera.generateDir(i + 0.5, j + 0.5);
			if (Path::makeOneDiffusePath(this, dir)) {
				(*initialWeights)[i][j] = 20;
			}
		}
	}

	static BitmapArray samples[SampleTimes];
	//static double w[SampleTimes];
	BitmapArray sampleSum(FinalWidth, FinalHeight);

	//sampleSum.load("finalResultWithoutBrightness.txt");
	//return sampleSum.transformToBitmap(FinalRGBMax);
	int startID = SampleStartID;

	for (int turn = 0; turn < SampleTurns; turn++) {
#pragma omp parallel for
		for (int i = 0;i < SampleTimes; i++) {
			std::stringstream filename;
			filename << "data/finalResultWithoutBrightness" << turn * SampleTimes + i + startID << ".txt";
			samples[i].load(filename.str().c_str());

			if (samples[i].isEmpty() || OverWrite) {
				double w[StartPathNum];
				std::vector<Path> wp;

				for (int j = 0;j < StartPathNum; j++) {
					wp.push_back(Path::makeRandomPathInImage(this));
					w[j] = wp.back().queryInitLuminianceDivProbability();
				}
				int bestpos = discrete_distribution(w, w + StartPathNum);
				assert(w[bestpos] > eps);
				samples[i] = MLT_process(wp[bestpos]);

				/*double vmax = 0;
				for (int wi = 0;wi < FinalWidth; wi++) {
					for (int he = 0; he < FinalHeight; he++) {
						for (int j = 0;j < 3;j++) {
							//samples[i][wi][he].c[j] *= w[i];
							if (vmax < samples[i][wi][he].c[j]) vmax = samples[i][wi][he].c[j];
						}
					}
				}
				for (int wi = 0;wi < FinalWidth; wi++) {
					for (int he = 0; he < FinalHeight; he++) {
						if ((*initialWeights)[wi][he] > 100) {
							for (int j = 0;j < 3;j++) {
								samples[i][wi][he].c[j] = vmax;
							}
						}
					}
				}*/
				samples[i].save(filename.str().c_str());
			}

			samples[i].limitMax();
		}

		for (int wi = 0;wi < FinalWidth; wi++) {
			for (int he = 0; he < FinalHeight; he++) {
				for (int i = 0;i < SampleTimes; i++) if (!samples[i].isEmpty()) {
					for (int j = 0;j < 3;j++) {
						sampleSum[wi][he].c[j] += samples[i][wi][he].c[j];
						/*if(samples[i][wi][he].c[j] != 0)
							std::cerr << samples[i][wi][he].c[j] << std::endl;*/
					}
				}
			}
		}
		sampleSum.save("finalResultWithoutBrightness.txt");

		std::stringstream tmpname;
		tmpname << "tmp" << turn << ".bmp";
		sampleSum.transformToBitmap(FinalRGBMax).save(tmpname.str().c_str());
	}
	return sampleSum.transformToBitmap(FinalRGBMax);
}

ReflectRecord RayTracing::queryEye()
{
	ReflectRecord ans;
	ans.type = ReflectRecord::eye;
	ans.indir = Point(0,0,0);
	ans.hitpoint = Point(0, 0, 0);

	//ans.outdir = camera.generateDir();
	double u = (double)(rand()+1) / (RAND_MAX+1); //按面积取样
	double phi = rand() * PI * 2 / RAND_MAX;
	double z = u, t = sqrt(1 - u*u);
	double x = t*cos(phi), y = t*sin(phi);

	ans.outdir = Point(x, y, z);


	//需要和makeeye同时更改
	ans.luminiance = Color(1, 1, 1) / pow(ans.outdir.z, 3);
	if (z < 0) ans.luminiance = Color(0, 0, 0);
	//ans.luminiance = Color(1, 1, 1) / pow(ans.outdir.z, 3);
	//ans.randomProbability = 3.8421 / 4 / PI / pow(ans.outdir.z, 3);
	//ans.randomProbability = 0.25 / PI/  pow(ans.outdir.z, 3);
	ans.randomProbability = 0.5 / PI;
	ans.face = nullptr;


	//int xx, yy;
	//std::tie(xx, yy) = this->camera.queryPos(ans.outdir);
	//if (abs(xx-78) <= 1 && abs(yy-8)<=1)
	//	std::cerr << '!';
	return ans;
}

ReflectRecord RayTracing::queryLight()
{
	ReflectRecord ans;
	ans.type = ReflectRecord::light;
	ans.indir = Point(0, 0, 0);

	//ans.hitpoint = Point( 5, 0, 5);
	ans.hitpoint = Point(0, 8, 30);

	//double u = (double)rand() / RAND_MAX / 2 - 0.5; //按面积取样
	double u = (double)rand() / RAND_MAX / 2 + 0.5; //按面积取样
	double phi = rand() * PI * 2 / RAND_MAX;
	double y = u, t = sqrt(1-u*u);
	double x = t*cos(phi), z = t*sin(phi);

	ans.outdir = Point(x, y, z);
	ans.luminiance = Color(1, 1, 1);
	ans.randomProbability = 1 / PI;
	ans.face = nullptr;
	return ans;
}

const KDtree * RayTracing::queryKDtree()
{
	return &tree;
}

const RayTracing::Camera * RayTracing::queryCamera()
{
	return &camera;
}

void RayTracing::Init1()
{
	//四面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -10,10 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 40 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		addFace(0, 2, 4, 0);
		addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		addFace(7, 2, 6, 4);
		addFace(7, 2, 3, 4);
		addFace(1, 3, 7, 3);
		addFace(1, 5, 7, 3);

		room.kdL = 1;
		room.kd = Color(0.3, 0.3, 0.3);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		//room.ks = Color(0.2, 0.2, 0.2);
		room.ksL = 0;
		room.tfL = 0;

		tree.addObject(room);
	}
	//下面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -10,10 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 40 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		//addFace(0, 2, 4, 0);
		//addFace(2, 4, 6, 0);
		addFace(1, 0, 4, 1);
		addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3);

		room.kdL = 0.8;
		room.kd = Color(0.3, 0.3, 0.3);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		room.ks = Color(0.2, 0.2, 0.2);
		room.ksL = 0.2;
		room.tfL = 0;

		tree.addObject(room);
	}
	//左面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -10,10 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 40 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		addFace(0, 1, 2, 2);
		addFace(1, 3, 2, 2);
		//addFace(0, 2, 4, 0);
		//addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3)*

		room.kdL = 1;
		room.kd = Color(0.2, 0.1, 0.36);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		//room.ks = Color(0.2, 0.2, 0.2);
		room.ksL = 0;
		room.tfL = 0;

		tree.addObject(room);
	}
	//右面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -10,10 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 40 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		//addFace(0, 2, 4, 0);
		//addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		addFace(4, 5, 6, 5);
		addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3);

		room.kdL = 1;
		room.kd = Color(0.2, 0.36, 0.1);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		room.ks = Color(0, 0, 0);
		room.ksL = 0;
		room.tfL = 0;

		tree.addObject(room);
	}
	//球1
	{
		vecObjects.push_back(new Object(0, 0, 0));
		Object &ball = *vecObjects.back();
		ball.Load("model/sphereComplex.obj");
		//ball.replace(-4, 4, -4, 4, 15, 19);
		ball.replace(-8, -2, -9.9, -4, 32, 38);

		ball.kdL = 0;
		ball.ksL = 1;
		ball.ks = Color(1, 1, 1);
		ball.tfL = 0;
		//ball.tf = Color(1, 1, 1);
		//ball.Ni = 1.5;

		tree.addObject(ball);
	}
	//球2
	{
		vecObjects.push_back(new Object(0, 0, 0));
		Object &ball = *vecObjects.back();
		ball.Load("model/sphereComplex.obj");
		ball.replace(2, 8, -9.9, -4, 22, 28);

		ball.kdL = 0;
		ball.ksL = 0;
		ball.tfL = 1;
		ball.tf = Color(1, 1, 1);
		ball.Ni = 1.5;

		tree.addObject(ball);
	}

	/*{
		vecObjects.push_back(new Object(-PI / 2, 0, 0));
		Object &cup = *vecObjects.back();
		cup.Load("model/p.obj");
		cup.replace(-8, 0, -9.9, -5, 25, 32);

		cup.kdL = 0;
		cup.kd = Color(0.3, 0.3, 0.3);
		cup.ksL = 0.05;
		cup.tf = Color(0.25, 0.28, 0.25);
		cup.tfL = 0.95;
		cup.tf = Color(0.9, 0.92, 0.9);
		cup.Ni = 1.5;

		tree.addObject(cup);
	}*/

	/*{
		vecObjects.push_back(new Object(PI, 0, 0));
		Object &cup = *vecObjects.back();
		cup.Load("model/p2.obj");
		cup.replace(2, 6, 2, 6, 13, 19.9);

		cup.kdL = 0;
		cup.kd = Color(0.3, 0.3, 0.3);
		cup.ksL = 0.2;
		cup.tf = Color(0.95, 0.95, 0.95);
		cup.tfL = 0.8;
		cup.tf = Color(0.95, 0.95, 0.95);
		cup.Ni = 1.5;

		tree.addObject(cup);
	}*/

	/*{
		vecObjects.push_back(new Object(4, 0, 0 , 0, Point(-2, -2, 6)));
		Object &dragon = *vecObjects.back();
		dragon.Load("model/dragon.obj");

		dragon.kdL = 1;
		dragon.ksL = 0;
		dragon.kd = Color(0.3, 0.3, 0.3);
		dragon.tfL = 0;

		tree.addObject(dragon);
	}*/
	tree.buildTree();

	camera.realWidth = RealWidth;
	camera.realHeight = RealHeight;

	auto makeRegion = [&](int x1, int y1, int x2, int y2, double k) {
		double xmid = (x1 + x2) / 2.;
		double ymid = (y1 + y2) / 2.;
		double diss = abs(xmid - x1) + abs(ymid - y1);
		for (int i = x1; i < x2; i++) {
			for (int j = y1; j < y2;j++) {
				double dis = std::abs(i - xmid) + std::abs(j - ymid);
				(*initialWeights)[i][j] = (*initialWeights)[i][j] * dis / diss + k * (diss - dis) / diss;
			}
		}
	};
	for (int i = 0; i < FinalWidth; i++) {
		for (int j = 0;j < FinalHeight; j++) {
			(*initialWeights)[i][j] = 1;
		}
	}
	//makeRegion(220, FinalHeight - 128, 390, FinalHeight - 67, 10);
	makeRegion(150, 100, 270, 250, 0.5);
	makeRegion(330, 0, 520, 250, 0.5);

	//Point s = {-0.0905,9.9,9.3123 }, dir = { -0.1501,-0.9417,0.2747 };
	//auto ppp = tree.queryBF(s, dir);
	//auto pp = tree.query(s, dir);
	//auto ans = checkPointInFace(*std::get<0>(pp), std::get<1>(pp));
	//double xxbf = queryIntersectTime(*std::get<0>(ppp), s, dir);
	//double xx = queryIntersectTime(*std::get<0>(pp), s, dir);
	//auto ppppp = s + dir * xx;
}

void RayTracing::Init2()
{
	//4面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -5,5 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 15 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		addFace(0, 2, 4, 0);
		addFace(2, 4, 6, 0);
		addFace(1, 0, 4, 1);
		addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		addFace(7, 2, 6, 4);
		addFace(7, 2, 3, 4);
		addFace(1, 3, 7, 3);
		addFace(1, 5, 7, 3);

		room.kdL = 1;
		room.kd = Color(0.3, 0.3, 0.3);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		room.ksL = 0;
		room.tfL = 0;
		room.rerotate(-PI / 4, 0, 0);

		tree.addObject(room);
	}

	//左面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -5,5 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 15 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		addFace(0, 1, 2, 2);
		addFace(1, 3, 2, 2);
		//addFace(0, 2, 4, 0);
		//addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3)*

		room.kdL = 1;
		room.kd = Color(0.2, 0.1, 0.36);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		//room.ks = Color(0.2, 0.2, 0.2);
		room.ksL = 0;
		room.tfL = 0;
		room.rerotate(-PI / 4, 0, 0);
		tree.addObject(room);

	}

	//光源
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { 3,7 };
		double y[2] = { -2, 2 };
		double z[2] = { 4, 20 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		addFace(0, 2, 4, 0);
		addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		//addFace(4, 5, 6, 5);
		//addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3);

		room.kdL = 1;
		room.kd = Color(0.3, 0.3, 0.3);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		room.ksL = 0;
		room.tfL = 0;
		//room.rerotate(-PI / 4, 0, 0);

		tree.addObject(room);
	}

	//右面
	{
		vecObjects.push_back(new Object());
		Object &room = *vecObjects.back();
		double x[2] = { -5,5 };
		double y[2] = { -10, 10 };
		double z[2] = { -0.1, 15 };
		for (int i = 0;i < 2;i++) {
			for (int j = 0;j < 2;j++) {
				for (int k = 0;k < 2;k++) {
					room.p.emplace_back(x[i], y[j], z[k]);
				}
			}
		}
		room.pn.emplace_back(0, 0, 1);
		room.pn.emplace_back(0, 1, 0);
		room.pn.emplace_back(1, 0, 0);
		room.pn.emplace_back(0, 0, -1);
		room.pn.emplace_back(0, -1, 0);
		room.pn.emplace_back(-1, 0, 0);
		auto addFace = [&](int a, int b, int c, int d) {
			room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
		};
		//addFace(0, 1, 2, 2);
		//addFace(1, 3, 2, 2);
		//addFace(0, 2, 4, 0);
		//addFace(2, 4, 6, 0);
		//addFace(1, 0, 4, 1);
		//addFace(1, 5, 4, 1);
		addFace(4, 5, 6, 5);
		addFace(7, 5, 6, 5);
		//addFace(7, 2, 6, 4);
		//addFace(7, 2, 3, 4);
		//addFace(1, 3, 7, 3);
		//addFace(1, 5, 7, 3);

		room.kdL = 1;
		room.kd = Color(0.2, 0.36, 0.1);
		//std::cerr << queryLuminiance(room.kd) << std::endl;
		room.ks = Color(0, 0, 0);
		room.ksL = 0;
		room.tfL = 0;
		room.rerotate(-PI / 4, 0, 0);
		tree.addObject(room);
	}

	/*
	//球1
	{
		vecObjects.push_back(new Object(0, 0, 0));
		Object &ball = *vecObjects.back();
		ball.Load("model/mysphere.obj");
		//ball.replace(-4, 4, -4, 4, 15, 19);
		ball.replace(-8, -2, -9.9, -4, 32, 38);

		ball.kdL = 0;
		ball.ksL = 1;
		ball.ks = Color(1, 1, 1);
		ball.tfL = 0;
		//ball.tf = Color(1, 1, 1);
		//ball.Ni = 1.5;

		tree.addObject(ball);
	}
	//球2
	{
		vecObjects.push_back(new Object(0, 0, 0));
		Object &ball = *vecObjects.back();
		ball.Load("model/mysphere.obj");
		//ball.replace(-4, 4, -4, 4, 15, 19);
		ball.replace(2, 8, -9.9, -4, 22, 28);

		ball.kdL = 0;
		ball.ksL = 0;
		ball.tfL = 1;
		ball.tf = Color(1, 1, 1);
		ball.Ni = 1.5;

		tree.addObject(ball);
	}*/

	/*{
	vecObjects.push_back(new Object(-PI / 2, 0, 0));
	Object &cup = *vecObjects.back();
	cup.Load("model/p2.obj");
	cup.replace(-4, 0, -9.9, -3, 14, 18);

	cup.kdL = 0;
	cup.kd = Color(0.3, 0.3, 0.3);
	cup.ksL = 0.2;
	cup.tf = Color(0.95, 0.95, 0.95);
	cup.tfL = 0.8;
	cup.tf = Color(0.95, 0.95, 0.95);
	cup.Ni = 1.5;

	tree.addObject(cup);
	}

	{
	vecObjects.push_back(new Object(PI, 0, 0));
	Object &cup = *vecObjects.back();
	cup.Load("model/p2.obj");
	cup.replace(2, 6, 2, 6, 13, 19.9);

	cup.kdL = 0;
	cup.kd = Color(0.3, 0.3, 0.3);
	cup.ksL = 0.2;
	cup.tf = Color(0.95, 0.95, 0.95);
	cup.tfL = 0.8;
	cup.tf = Color(0.95, 0.95, 0.95);
	cup.Ni = 1.5;

	tree.addObject(cup);
	}*/

	{
		vecObjects.push_back(new Object(0, 0 , 0));
		Object &dragon = *vecObjects.back();
		dragon.Load("model/dragon.obj");
		dragon.replace(0, 2, -9.999, -8, 10, 12);

		dragon.kdL = 1;
		dragon.ksL = 0;
		dragon.kd = Color(0.3, 0.3, 0.3);
		dragon.tfL = 0;
		dragon.rerotate(-PI / 4, 0, 0);
		tree.addObject(dragon);
	}


	{
		vecObjects.push_back(new Object(0, 0, 0));
		Object &huan = *vecObjects.back();
		huan.Load("model/yuanhuan.obj");
		huan.replace(-3, 3, -9.999, -9.5, 7, 13);

		huan.kdL = 0;
		huan.kd = Color(0.3, 0.3, 0.3);
		huan.ksL = 1;
		huan.ks = Color(0.8, 0.8, 0.8);
		//cup.tfL = 0;
		//cup.tf = Color(0.95, 0.95, 0.95);
		//cup.Ni = 1.5;
		huan.rerotate(-PI / 4, 0, 0);

		tree.addObject(huan);
	}


	tree.buildTree();

	camera.realWidth = RealWidth;
	camera.realHeight = RealHeight;

	auto makeRegion = [&](int x1, int y1, int x2, int y2, double k) {
		double xmid = (x1 + x2) / 2.;
		double ymid = (y1 + y2) / 2.;
		double diss = abs(xmid - x1) + abs(ymid - y1);
		for (int i = x1; i < x2; i++) {
			for (int j = y1; j < y2;j++) {
				double dis = std::abs(i - xmid) + std::abs(j - ymid);
				(*initialWeights)[i][j] = (*initialWeights)[i][j] * dis / diss + k * (diss - dis) / diss;
			}
		}
	};
	for (int i = 0; i < FinalWidth; i++) {
		for (int j = 0;j < FinalHeight; j++) {
			(*initialWeights)[i][j] = 1;
		}
	}
	//makeRegion(0, 0, FinalWidth, FinalHeight, 1);
	//makeRegion(220, FinalHeight - 128, 390, FinalHeight - 67, 10);
	//makeRegion(150, 100, 270, 250, 0.5);
	//makeRegion(330, 0, 520, 250, 0.5);

	//Point s = {-0.0905,9.9,9.3123 }, dir = { -0.1501,-0.9417,0.2747 };
	//auto ppp = tree.queryBF(s, dir);
	//auto pp = tree.query(s, dir);
	//auto ans = checkPointInFace(*std::get<0>(pp), std::get<1>(pp));
	//double xxbf = queryIntersectTime(*std::get<0>(ppp), s, dir);
	//double xx = queryIntersectTime(*std::get<0>(pp), s, dir);
	//auto ppppp = s + dir * xx;
}


//折射强度和角度有关吗？？？

int main()
{
	//Object obj;
	//obj.Load("model/fixed.perfect.dragon.100K.0.07.obj");
	//KDtree tree;
	//tree.addObject(obj);
	//tree.buildTree();
	//srand((int)time(0));
	//for (int i = 1;i <= 10000;i++) {
	//	Point s{ rand() % 10 - 5.,(rand() % 1000)/100. - 5.,rand() % 10 - 5. };
	//	Point d = obj.p[rand() % obj.p.size()] - s;
	//	auto p = tree.query(s, d);
	//	if(std::get<0>(p))std::cerr << std::get<0>(p) << std::endl;
	//	/*const Face* q = tree.queryBF(s, d);
	//	if(q)std::cerr << q << std::endl;
	//	if (p != q) {
	//		tree.query(s, d);
	//		double a = queryIntersectTime(*p, s, d);
	//		double b = queryIntersectTime(*q, s, d);
	//		//a = b;
	//		std::cerr << "";
	//		assert(abs(a - b) < eps);
	//	}*/
	//	
	//	
	//	//assert(p == q);
	//}
	//std::cerr << clock() << std::endl;

	RayTracing r;
	r.Init1();



	//for(int i = 0;i<100;i++)
	//	std::cerr << Path::debugQueryLuminianceInImage(&r, FinalWidth*i/100., FinalHeight/2) << std::endl;
	////std::cerr << Path::debugQueryLuminianceInImage(&r, FinalWidth / 4, FinalHeight / 2) << std::endl;
	//return 0;

	auto image = r.metropisLightTransport();
	image.save("1.bmp");
	return 0;
}

Point RayTracing::Camera::generateDir() const
{
	//auto randomGenerate = [] {
	//	double theta = rand() * PI / RAND_MAX - PI / 2;
	//	double phi = rand() * 2 * PI / RAND_MAX;
	//	return Point(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
	//};
	//Point dir = randomGenerate();
	//while (!queryInImage(dir)) dir = randomGenerate

	Point dir = generateDir((double)rand() * FinalWidth / (RAND_MAX - 1), (double)rand() * FinalHeight / (RAND_MAX - 1));
	return dir;
}

Point RayTracing::Camera::generateDir(double x, double y) const
{
	Point dir(x * realWidth / FinalWidth - realWidth / 2, y * realHeight / FinalHeight - realHeight / 2, 1);
	return dir / abs(dir);
}

std::tuple<int, int> RayTracing::Camera::queryPos(const Point &dir) const
{
	if (dir.z < 0) return std::make_tuple(FinalWidth, FinalHeight);
	double x = dir.x / dir.z;
	double y = dir.y / dir.z;
	return std::make_tuple((int)floor(x / realWidth * FinalWidth + FinalWidth / 2),
		(int)floor(y / realHeight * FinalHeight + FinalHeight / 2));
}

bool RayTracing::Camera::queryInImage(const Point &dir) const
{
	int x, y;
	std::tie(x, y) = queryPos(dir);
	return x >= 0 && x<FinalWidth && y >= 0 && y<FinalHeight;
}


