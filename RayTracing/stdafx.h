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