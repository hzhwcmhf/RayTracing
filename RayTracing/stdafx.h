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


const double eps = 1e-10; //��������
const double PI = acos(-1);

const int MaxKDtreeHeight = 30;//kd���������

const bool OverWrite = false;//�Ƿ񸲸Ǽ�¼

const int SampleStartID = 50000;//��¼��ʼ����
const int SampleTurns = 50;//��¼����
const int SampleTimes = 32;//���μ�¼��������
const int FinalWidth = 600, FinalHeight = 600;//����ͼ���
const int FinalRGBMax = 255;//���RGBֵ
const int MutateTimes = 10000000;//���ε�����������
const int StartPathNum = 10;//��ʼ��ѡ·������
const int PathMaxDiffuseTimes = 4;//������������
const int SubpathMaxSpecularTimes = 60;//����·�������/�������
const double MinDistance = 1e-5;//��Ⱦ������ ��С��������

const double PathDiffuseProbability = 0.5;//�������·��ʱ������һ�����������
const double PathDirectLightProbability = 0.1;//�������ʱ���������������

const double PathMutateRotateStrategyProbability = 0.9;//�Ŷ��������
const double PathMutateSplitStrategyProbability = 0.05;//����·�����ѵ�������
const double PathMutateRandomStrategyProbability = 0.05;//��������·���������
const double MutateNormalSigma = 4. / FinalWidth;//�Ŷ� ��̬�ֲ�sigma

const double PathMutateRotateThisPointProbability = 0.7;//�Ŷ�����õ����
const double RealWidth = 0.5;//��ͷʵ�ʿ��
const double RealHeight = 0.5;//��ͷʵ�ʸ߶�

inline void assert(bool x)
{
	if (!x)
		throw 1;
}