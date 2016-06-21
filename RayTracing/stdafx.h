// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


// TODO:  在此处引用程序需要的其他头文件
#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <array>
#include <algorithm>
#include <ctime>
//#include <cassert>
#include <fstream>
#include <cmath>
#include <sstream>

#include <omp.h>  
#include "windows.h"
#undef min
#undef max

const double eps = 1e-10;
const double PI = acos(-1);

const int MaxKDtreeHeight = 30;

const bool OverWrite = false;

const int SampleStartID = 70000;
const int SampleTurns = 50;
const int SampleTimes = 16;
const int FinalWidth = 600, FinalHeight = 600;
const int FinalRGBMax = 255;
const int MutateTimes = 10000000;
const int StartPathNum = 10;
const int PathMaxDiffuseTimes = 4;
const int SubpathMaxSpecularTimes = 60;
const double MinDistance = 1e-5;
const double PathDiffuseProbability = 0.5;

const double PathDirectLightProbability = 0.1;
const double PathMutateRotateStrategyProbability = 0.9;
const double PathMutateSplitStrategyProbability = 0.05;
const double PathMutateRandomStrategyProbability = 0.05;
const double MutateNormalSigma = 4. / FinalWidth;

const double PathMutateRotateThisPointProbability = 0.7;
const double RealWidth = 0.5;
const double RealHeight = 0.5;

inline void assert(bool x)
{
	if (!x)
		throw 1;
}