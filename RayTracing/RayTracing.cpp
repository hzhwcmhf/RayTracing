#include "stdafx.h"

#include "Bitmap.h"
#include "Point.h"
#include "Face.h"
#include "Object.h"
#include "KDtree.h"



class RayTracing
{
private:

public:
	Bitmap work()
	{
		//initialize


		//¹âÏß×·×Ù

	}
};



int main()
{
	Object obj;
	obj.Load("model/fixed.perfect.dragon.100K.0.07.obj");
	KDtree tree;
	tree.addObject(obj);
	tree.buildTree();
	srand(time(0));
	for (int i = 1;i <= 10000;i++) {
		Point s{ rand() % 10 - 5.,(rand() % 1000)/100. - 5.,rand() % 10 - 5. };
		Point d = obj.p[rand() % obj.p.size()] - s;
		const Face* p = tree.query(s, d);
		if(p)std::cerr << p << std::endl;
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

