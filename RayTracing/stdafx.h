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


const double eps = 1e-10; //精度限制
const double PI = acos(-1);

const int MaxKDtreeHeight = 30;//kd树最高树高

const bool OverWrite = false;//是否覆盖记录

const int SampleStartID = 50000;//记录开始计数
const int SampleTurns = 50;//记录轮数
const int SampleTimes = 32;//单次记录迭代轮数
const int FinalWidth = 600, FinalHeight = 600;//生成图宽高
const int FinalRGBMax = 255;//最大RGB值
const int MutateTimes = 10000000;//单次迭代变异轮数
const int StartPathNum = 10;//开始候选路径数量
const int PathMaxDiffuseTimes = 4;//最大漫反射次数
const int SubpathMaxSpecularTimes = 60;//单次路径最大镜面/折射次数
const double MinDistance = 1e-5;//渲染方程中 最小距离限制

const double PathDiffuseProbability = 0.5;//随机生成路径时，增加一次漫反射概率
const double PathDirectLightProbability = 0.1;//随机生成时，单次漫反射概率

const double PathMutateRotateStrategyProbability = 0.9;//扰动变异概率
const double PathMutateSplitStrategyProbability = 0.05;//更改路径分裂点变异概率
const double PathMutateRandomStrategyProbability = 0.05;//重新生成路径变异概率
const double MutateNormalSigma = 4. / FinalWidth;//扰动 正态分布sigma

const double PathMutateRotateThisPointProbability = 0.7;//扰动变异该点概率
const double RealWidth = 0.5;//镜头实际宽度
const double RealHeight = 0.5;//镜头实际高度

inline void assert(bool x)
{
	if (!x)
		throw 1;
}