#pragma once

#include "BitmapArray.h"
#include "Point.h"
#include "Face.h"
#include "ReflectRecord.h"

class RayTracing;

//漫反射直接的路径
struct SubPath
{
	RayTracing* rt;
	HalfReflectRecord startR, endR;//路径开始和结束光线情况

	std::vector<ReflectRecord> inner;//记录反射和折射

	//每一段 1/r2 * 每个点 L
	Color luminiance;
	//每一段 1/r2 * 每个点 P
	double randomProbability;

	SubPath(RayTracing* r);
	
	//从startPos开始沿startDir拓展，获取inner、luminiance，返回end面(注意超过层数要返回)
	ReflectRecord extend(const HalfReflectRecord &start);
	ReflectRecord extend();
	//按照原有路径进行调整
	ReflectRecord extendAdjust(const HalfReflectRecord &start, const SubPath &ori);
	

	//尝试直接连线
	bool checkShadow(const HalfReflectRecord &start, const HalfReflectRecord &end);


	//翻转路径
	void reverse();
};

//光线路径
class Path
{
private:
	RayTracing* rt;
	//双向漫反射路径记录
	std::vector<ReflectRecord> eyeBRDF, lightBRDF;
	//双向Subpath记录
	std::vector<SubPath> eyePath, lightPath;

	//连接处的光线状况
	ReflectRecord shadowEyeBRDF, shadowLightBRDF;
	SubPath shadowPath;//连接处的Subpath

	Color luminiance;
	double diffuseAndLightProbability;//散射次数和光源造成的取样概率
	double randomProbability; //随机采样时的取样概率

	//计算luminiance和randomProbability
	void calLuminianceAndRandomProbability();
	//计算shadowDisatance
	bool checkShadow();

	//检查是否在图中
	bool queryInImage();

public:
	Path(RayTracing* r);
	//随机生成路径
	static Path makeRandomPath(RayTracing* r);
	//随机生成在图内路径
	static Path makeRandomPathInImage(RayTracing* r);
	//随机生成单次反射路径
	static bool makeOneDiffusePath(RayTracing* r, const Point &dir);

	//扰动变异
	std::tuple<Path, double> mutateRotate() const;
	//分裂变异
	std::tuple<Path, double> mutateSplit() const;
	//检查是否能够变异
	bool canMutateSplit() const;
	//变异
	std::tuple<Path, double> mutate() const;
	//按权值记录颜色
	void record(BitmapArray& barr, double w);
	//求亮度除以概率
	double queryInitLuminianceDivProbability();

	//调试用
	static double debugQueryLuminianceInImage(RayTracing* r, double x, double y);
	//查询路径长度
	int debugQueryDiffuseTimes();
	//查询视点路径长度
	int debugEyeDiffuseTimes();
	//调试用
	void debugMutateRotate() const;

	//获取路径在图内的坐标
	std::tuple<int, int> queryImagePos();
};