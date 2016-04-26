#pragma once

#include "Bitmap.h"
#include "BitmapArray.h"
#include "Path.h"

class RayTracing
{
private:
	BitmapArray MLT_process(Path &p)
	{
		BitmapArray barr(FinalWidth, FinalHeight);
		for (int mt = 0;mt < MutateTimes; mt++) {
			double pro;
			Path p2;
			std::tie(p2, pro) = p.mutate();
			p.record(barr, 1 - pro);
			p2.record(barr, pro);
			if (rand() < pro * RAND_MAX) {
				p = p2;
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

	std::tuple<Point, BRDF*> queryEye();
	std::vector<std::tuple<Point, BRDF*>> queryLights();
};