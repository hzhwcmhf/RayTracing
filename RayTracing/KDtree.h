#pragma once

#include "Face.h"
#include "Object.h"

class KDtree
{
public:
	//包围盒
	struct BorderBox
	{
		union {
			struct {
				/*数组索引*/
				double a[3], b[3];
			};
			struct {
				/*变量索引*/
				double x1, y1, z1, x2, y2, z2;
			};
		};
		void init(const Face &f);//使用面来初始化包围盒
		void addPoint(const Point &p);//在包围盒中增加一个点
		void addFace(const Face &f);//在包围盒中增加一个面
		void squeeze(const BorderBox &o);//使用外部盒来挤压包围盒
		double surface() const;//计算表面积
		bool contain(const Point &p) const;//判断点是否在包围盒内

		//计算射线与包围盒的相交时间
		friend double queryIntersectTime(const BorderBox &b, const Point &s, const Point &dir);
		//friend double queryIntersectTimeX(const BorderBox)

	};

private:
	std::vector<const Face*> fp;//面片池
	typedef std::vector<const Face*>::iterator FaceIterator;
	
	//kd树节点
	struct Node
	{
		enum Direction{ x=0, y=1, z=2, leaf } direction;
		double pos;
		int l, r;
		//std::vector<Face*> fp;
		BorderBox box;
		Node *lc, *rc;
	};
	
	Node* root;

	const static int FaceNumOfLeaf = 16;//kd树的叶子面数
	const static int LeastDecrease = 0;//kd树节点比父亲至少减少的面片数量
	//建树递归函数
	Node* buildTree_subtree(const std::vector<const Face*> &fp, const BorderBox &box, int height);
	//计算切割位置（传入面片集合，切割方向，和外部包围盒）
	std::tuple<double, double, int, int>
		buildTree_calculate(const std::vector<const Face*> &fp, Node::Direction d, const BorderBox &box);
	//分裂面片集合
	std::tuple<std::vector<const Face*>, std::vector<const Face*>>
		buildTree_partition(const std::vector<const Face*> &fp, Node::Direction d, double borderPos);

	//查询递归函数
	std::tuple<const Face*, double> queryNode(const Node* node, const Point & s, const Point & dir, const Face* ignore) const;
public:
	
	void addObject(const Object& p);//加入物体
	void buildTree();//建树

	//查询
	std::tuple<const Face*, Point> query(const Point &s, const Point &dir, const Face* ignore = nullptr) const;
	//暴力查询
	std::tuple<const Face*, Point> queryBF(const Point &s, const Point &dir, const Face* ignore = nullptr) const;

	template<class T>
	inline static void tension(T &a, const T &b)
	{
		if (a > b) a = b;
	}
	template<class T>
	inline static void relax(T &a, const T &b)
	{
		if (a < b) a = b;
	}
};