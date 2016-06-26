#pragma once

#include "Face.h"
#include "Object.h"

class KDtree
{
public:
	//��Χ��
	struct BorderBox
	{
		union {
			struct {
				/*��������*/
				double a[3], b[3];
			};
			struct {
				/*��������*/
				double x1, y1, z1, x2, y2, z2;
			};
		};
		void init(const Face &f);//ʹ��������ʼ����Χ��
		void addPoint(const Point &p);//�ڰ�Χ��������һ����
		void addFace(const Face &f);//�ڰ�Χ��������һ����
		void squeeze(const BorderBox &o);//ʹ���ⲿ������ѹ��Χ��
		double surface() const;//��������
		bool contain(const Point &p) const;//�жϵ��Ƿ��ڰ�Χ����

		//�����������Χ�е��ཻʱ��
		friend double queryIntersectTime(const BorderBox &b, const Point &s, const Point &dir);
		//friend double queryIntersectTimeX(const BorderBox)

	};

private:
	std::vector<const Face*> fp;//��Ƭ��
	typedef std::vector<const Face*>::iterator FaceIterator;
	
	//kd���ڵ�
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

	const static int FaceNumOfLeaf = 16;//kd����Ҷ������
	const static int LeastDecrease = 0;//kd���ڵ�ȸ������ټ��ٵ���Ƭ����
	//�����ݹ麯��
	Node* buildTree_subtree(const std::vector<const Face*> &fp, const BorderBox &box, int height);
	//�����и�λ�ã�������Ƭ���ϣ��и�򣬺��ⲿ��Χ�У�
	std::tuple<double, double, int, int>
		buildTree_calculate(const std::vector<const Face*> &fp, Node::Direction d, const BorderBox &box);
	//������Ƭ����
	std::tuple<std::vector<const Face*>, std::vector<const Face*>>
		buildTree_partition(const std::vector<const Face*> &fp, Node::Direction d, double borderPos);

	//��ѯ�ݹ麯��
	std::tuple<const Face*, double> queryNode(const Node* node, const Point & s, const Point & dir, const Face* ignore) const;
public:
	
	void addObject(const Object& p);//��������
	void buildTree();//����

	//��ѯ
	std::tuple<const Face*, Point> query(const Point &s, const Point &dir, const Face* ignore = nullptr) const;
	//������ѯ
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