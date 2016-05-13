#pragma once

#include "Point.h"

class Object;

class Face
{
private:
	
public:
	Object* objectp;
	union {
		struct { /*数组索引*/Vertex p[3]; };
		struct { /*变量索引*/Vertex a, b, c; };
	};
	union {
		struct { /*数组索引*/Vertex pn[3]; };
		struct { /*变量索引*/Vertex an, bn, cn; };
	};

	Face() {
		objectp = nullptr;
		a = b = c = nullptr;
		an = bn = cn = nullptr;
	}
	Face(Object* obj, Vertex _a, Vertex _b, Vertex _c, Vertex _an, Vertex _bn, Vertex _cn) :
		objectp(obj), a(_a), b(_b), c(_c), an(_an), bn(_bn), cn(_cn)
	{}

	inline friend double queryIntersectTime(const Face &f, const Point &s, const Point &dir);
	inline friend bool checkPointInFace(const Face &f, const Point &s);

	inline Point getNormalVector(const Point &hitpoint) const;
};


inline double queryIntersectTime(const Face & f, const Point & s, const Point & dir)
{
	Point n = cross(*f.a, *f.b, *f.c);
	double t = dot(((*f.a) - s), n) / dot(dir, n);
	if (t < 0 || isnan(t)) return INFINITY;
	if (checkPointInFace(f, s + t * dir)) return t;
	return INFINITY;
}

inline bool checkPointInFace(const Face & f, const Point & s)
{
	Point2 p1{ f.a->x, f.a->y }, p2{ f.b->x, f.b->y }, p3{ f.c->x, f.c->y }, ss{ s.x, s.y };
	double pp,a1, a2, a3;
	int t1, t2, t3;

#define DBLCMPP(p) ((p)>eps?2:((p)<-eps?0:1))

#define POINTINTRAN pp = cross(p1, p2, p3);\
	if (pp > eps || pp <-eps) {\
		a1 = cross(p1, ss, p2);\
		a2 = cross(p2, ss, p3);\
		t1 = DBLCMPP(a1), t2 = DBLCMPP(a2);\
		if (~(0xfefb >> (t1 << 2 | t2)) & 1) return false;\
		a3 = cross(p3, ss, p1);\
		t3 = DBLCMPP(a3);\
		return (0xfffffee8fefbf8bbULL >> ((t1<<4)|(t2<<2)|t3))&1;\
	}

	POINTINTRAN;

	p1 = { f.a->x, f.a->z };
	p2 = { f.b->x, f.b->z };
	p3 = { f.c->x, f.c->z };
	ss = { s.x, s.z };

	POINTINTRAN;

	p1 = { f.a->y, f.a->z };
	p2 = { f.b->y, f.b->z };
	p3 = { f.c->y, f.c->z };
	ss = { s.y, s.z };

	POINTINTRAN;

#undef POINTINTRAN
#undef DBLCMPP

	return false;
}

/*inline bool checkPointInFace(const Face & f, const Point & s)
{
	double ASB = norm(cross(*f.a, s, *f.b));
	double BSC = norm(cross(*f.b, s, *f.c));
	if ((ASB > eps && BSC < -eps) || (ASB < -eps && BSC > eps)) return false;
	double CSA = norm(cross(*f.c, s, *f.a));
	if (ASB > eps && BSC > eps && CSA > eps) return true;
	if (ASB <-eps && BSC <-eps && CSA <-eps) return true;

	int res = ((ASB>0)<<1 | (CSA>0))
	double sum = ASB + BSC + CSA;

}*/

inline Point Face::getNormalVector(const Point &hitpoint) const
{
	double ASB = norm(cross(*a, hitpoint, *b));
	double BSC = norm(cross(*b, hitpoint, *c));
	double CSA = norm(cross(*c, hitpoint, *a));
	Point n = ((*an) * BSC + (*bn) * CSA + (*cn) * ASB);
	return n / abs(n);
}