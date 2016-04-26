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
#include <cassert>
#include <fstream>

#include <omp.h>  
#include "windows.h"
#undef min
#undef max

const double eps = 1e-10;

const static int SampleTimes = 100;
const static int FinalWidth = 480, FinalHeight = 360;
const static int FinalRGBMax = 255;
const static int MutateTimes = 10000;
const static int PathMaxDiffuseTimes = 2;
const static double PathDiffuseProbability = 0.5;