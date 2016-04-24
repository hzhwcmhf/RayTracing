#pragma once

#include "Face.h"
#include "Object.h"

class KDtree
{
public:
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
		void init(const Face &f);
		void addPoint(const Point &p);
		void addFace(const Face &f);
		void squeeze(const BorderBox &o);
		double surface() const;

		friend double queryIntersectTime(const BorderBox &b, const Point &s, const Point &dir);
		//friend double queryIntersectTimeX(const BorderBox)

	};

private:
	std::vector<const Face*> fp;
	typedef std::vector<const Face*>::iterator FaceIterator;
	
	struct Node
	{
		enum Direction{ x=0, y=1, z=2, leaf } direction;
		double pos;
		int l, r;
		std::vector<Face*> fp;
		BorderBox box;
		Node *lc, *rc;
	};
	
	Node* root;

	const static int FaceNumOfLeaf = 64;
	const static int LeastDecrease = 4;
public:

	void addObject(const Object& p);
	Node* buildTree_subtree(const std::vector<const Face*> &fp, const BorderBox &box);
	std::tuple<double, double, int, int>
		buildTree_calculate(const std::vector<const Face*> &fp, Node::Direction d, const BorderBox &box);
	std::tuple<std::vector<const Face*>, std::vector<const Face*>>
		buildTree_partition(const std::vector<const Face*> &fp, Node::Direction d, double borderPos);
	void buildTree();

	const Face* queryNode(const Node* node, const Point & s, const Point & dir, const Face* ignore) const;
	const Face* query(const Point &s, const Point &dir, const Face* ignore = nullptr) const;
	const Face* queryBF(const Point &s, const Point &dir, const Face* ignore = nullptr) const;

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