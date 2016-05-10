#pragma once

#include "Bitmap.h"
#include "BitmapArray.h"
#include "Path.h"
#include "KDtree.h"

class RayTracing
{
private:
	BitmapArray MLT_process(Path &p)
	{
		BitmapArray barr(FinalWidth, FinalHeight);
		for (int mt = 0;mt < MutateTimes; mt++) {
			auto tmp = p.mutate();
			auto &p2 = std::get<0>(tmp);
			double pro = std::get<1>(tmp);
			p.record(barr, 1 - pro);
			p2.record(barr, pro);
			if (rand() < pro * RAND_MAX) {
				p = std::move(p2);
			}
		}
		return barr;
	}

public:

	Bitmap metropisLightTransport()
	{
		static BitmapArray samples[SampleTimes];
		static double w[SampleTimes];
		BitmapArray sampleSum;

#pragma omp parallel for
		for (int i = 0;i < SampleTimes; i++) {
			Path p = Path::makeRandomPath(this);
			w[i] = p.queryInitLuminianceDivProbability();
			while (w[i] < eps) {
				p = Path::makeRandomPath(this);
				w[i] = p.queryInitLuminianceDivProbability();
			}
			samples[i] = MLT_process(p);
		}

		for (int wi = 0;wi < FinalWidth; wi++) {
			for (int he = 0; he < FinalHeight; he++) {
				for (int i = 0;i < SampleTimes; i++) {
					for (int j = 0;j < 3;j++) {
						sampleSum[wi][he].c[j] += w[i] * samples[i][wi][he].c[j];
					}
				}
			}
		}
		sampleSum.save("finalResultWithoutBrightness.txt");
		return sampleSum.transformToBitmap(FinalRGBMax);
	}

	ReflectRecord queryEye();
	ReflectRecord queryLight();
	std::tuple<int, int> queryImagePos(const Point &dir);
	const KDtree* queryKDtree();
};