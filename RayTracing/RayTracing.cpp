#include "stdafx.h"
#include "RayTracing.h"

#include "Bitmap.h"
#include "Point.h"
#include "Face.h"
#include "Object.h"
#include "KDtree.h"






//折射强度和角度有关吗？？？

int main()
{
	Object obj;
	obj.Load("model/fixed.perfect.dragon.100K.0.07.obj");
	KDtree tree;
	tree.addObject(obj);
	tree.buildTree();
	srand((int)time(0));
	for (int i = 1;i <= 10000;i++) {
		Point s{ rand() % 10 - 5.,(rand() % 1000)/100. - 5.,rand() % 10 - 5. };
		Point d = obj.p[rand() % obj.p.size()] - s;
		auto p = tree.query(s, d);
		if(std::get<0>(p))std::cerr << std::get<0>(p) << std::endl;
		/*const Face* q = tree.queryBF(s, d);
		if(q)std::cerr << q << std::endl;
		if (p != q) {
			tree.query(s, d);
			double a = queryIntersectTime(*p, s, d);
			double b = queryIntersectTime(*q, s, d);
			//a = b;
			std::cerr << "";
			assert(abs(a - b) < eps);
		}*/
		
		
		//assert(p == q);
	}
	std::cerr << clock() << std::endl;
	return 0;
}
