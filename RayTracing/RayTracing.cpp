#include "stdafx.h"
#include "RayTracing.h"

#include "Bitmap.h"
#include "Point.h"
#include "Face.h"
#include "Object.h"
#include "KDtree.h"


BitmapArray RayTracing::MLT_process(Path & p)
{
	BitmapArray barr(FinalWidth, FinalHeight);
	for (int mt = 0;mt < MutateTimes; mt++) {
		if(mt % 10000==0)
			std::cerr << mt << " ";
		auto tmp = p.mutate();
		auto &p2 = std::get<0>(tmp);
		double pro = std::get<1>(tmp);
		p.record(barr, 1 - pro);
		p2.record(barr, pro);

		//p2.record(barr, 1);

		//std::cerr << pro << std::endl;
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

Bitmap RayTracing::metropisLightTransport()
{
	static BitmapArray samples[SampleTimes];
	static double w[SampleTimes];
	BitmapArray sampleSum(FinalWidth, FinalHeight);

	//sampleSum.load("finalResultWithoutBrightness.txt");
	//return sampleSum.transformToBitmap(FinalRGBMax);

#pragma omp parallel for
	for (int i = 0;i < SampleTimes; i++) {
		Path p = Path::makeRandomPathInImage(this);
		w[i] = p.queryInitLuminianceDivProbability();
		while (w[i] < eps) {
			p = Path::makeRandomPathInImage(this);
			w[i] = p.queryInitLuminianceDivProbability();
		}
		samples[i] = MLT_process(p);
	}

	for (int wi = 0;wi < FinalWidth; wi++) {
		for (int he = 0; he < FinalHeight; he++) {
			for (int i = 0;i < SampleTimes; i++) {
				for (int j = 0;j < 3;j++) {
					sampleSum[wi][he].c[j] += w[i] * samples[i][wi][he].c[j];
					/*if(samples[i][wi][he].c[j] != 0)
						std::cerr << samples[i][wi][he].c[j] << std::endl;*/
				}
			}
		}
	}
	sampleSum.save("finalResultWithoutBrightness.txt");
	return sampleSum.transformToBitmap(FinalRGBMax);
}

ReflectRecord RayTracing::queryEye()
{
	ReflectRecord ans;
	ans.type = ReflectRecord::eyeOrLight;
	ans.indir = Point(0,0,0);
	ans.hitpoint = Point(0, 0, 0);

	ans.outdir = camera.generateDir();

	ans.luminiance = Color(1, 1, 1);
	ans.randomProbability = 1;
	ans.face = nullptr;
	return ans;
}

ReflectRecord RayTracing::queryLight()
{
	ReflectRecord ans;
	ans.type = ReflectRecord::eyeOrLight;
	ans.indir = Point(0, 0, 0);
	ans.hitpoint = Point(5, 0, 0);

	double theta = rand() * PI / RAND_MAX - PI / 2;
	double phi = rand() * 2 * PI / RAND_MAX;
	ans.outdir = Point(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta));
	ans.luminiance = Color(1, 1, 1);
	ans.randomProbability = 1;
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

void RayTracing::tmpInit()
{
	vecObjects.emplace_back();
	Object &room = vecObjects.back();
	double x[2] = { -10,10 };
	double y[2] = { -10, 10 };
	double z[2] = { -10, 10 };
	for (int i = 0;i < 2;i++) {
		for (int j = 0;j < 2;j++) {
			for (int k = 0;k < 2;k++) {
				room.p.emplace_back(x[i],y[j],z[k]);
			}
		}
	}
	room.pn.emplace_back(0, 0, 1);
	room.pn.emplace_back(0, 1, 0);
	room.pn.emplace_back(1, 0, 0);
	room.pn.emplace_back(0, 0, -1);
	room.pn.emplace_back(0, -1, 0);
	room.pn.emplace_back(-1, 0, 0);
	auto addFace = [&](int a, int b,int c, int d){
		room.f.emplace_back(&room, &room.p[a], &room.p[b], &room.p[c], &room.pn[d], &room.pn[d], &room.pn[d]);
	};
	addFace(0, 1, 2, 2);
	addFace(1, 3, 2, 2);
	addFace(0, 2, 4, 0);
	addFace(2, 4, 6, 0);
	addFace(1, 0, 4, 1);
	addFace(1, 5, 4, 1);
	addFace(4, 5, 6, 5);
	addFace(7, 5, 6, 5);
	addFace(7, 2, 6, 4);
	addFace(7, 2, 3, 4);
	addFace(1, 3, 7, 3);
	addFace(1, 5, 7, 3);
	
	room.kdL = 1;
	room.kd = Color(0.3, 0.3, 0.3);
	room.ksL = 0;
	room.tfL = 0;

	tree.addObject(room);
	tree.buildTree();


	camera.realWidth = 4;
	camera.realHeight = 3;
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
	r.tmpInit();

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

	Point dir = generateDir((double)rand() * FinalWidth / (RAND_MAX - 1), (double)rand() * FinalHeight / (RAND_MAX + 1));
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
	return std::make_tuple((int)(x / realWidth * FinalWidth + FinalWidth / 2),
		(int)(y / realHeight * FinalHeight + FinalHeight / 2));
}

bool RayTracing::Camera::queryInImage(const Point &dir) const
{
	int x, y;
	std::tie(x, y) = queryPos(dir);
	return x >= 0 && x<FinalWidth && y >= 0 && y<FinalHeight;
}


