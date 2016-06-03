// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


// TODO:  �ڴ˴����ó�����Ҫ������ͷ�ļ�
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
const int SampleTurns = 50;
const int SampleTimes = 4;
const int FinalWidth = 800, FinalHeight = 600;
const int FinalRGBMax = 255;
const int MutateTimes = 10000000;
const int PathMaxDiffuseTimes = 3;
const int SubpathMaxSpecularTimes = 30;
const double MinDistance = 1e-5;
const double PathDiffuseProbability = 0.5;

const double PathMutateRotateStrategyProbability = 0.9;
const double PathMutateSplitStrategyProbability = 0.05;
const double PathMutateRandomStrategyProbability = 0.05;


const double PathMutateRotateThisPointProbability = 0.7;
const double RealWidth = 2;
const double RealHeight = 1.5;

inline void assert(bool x)
{
	if (!x)
		throw 1;
}