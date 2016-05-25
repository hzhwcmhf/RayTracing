#pragma once

class Point
{
public:
	union {
		struct { /*数组索引*/double p[3]; };
		struct { /*变量索引*/double x, y, z; };
	};

	Point(){}
	Point(double _x, double _y, double _z):
		x(_x), y(_y), z(_z)
	{}

	Point operator+(const Point &b) const
	{
		return Point{x+b.x, y+b.y, z+b.z};
	}
	Point operator+=(const Point &b)
	{
		x += b.x, y += b.y, z += b.z;
	}
	Point operator-(const Point &b) const
	{
		return Point { x - b.x, y - b.y, z - b.z };
	}
	Point operator-() const
	{
		return Point(-x, -y, -z);
	}
	Point operator-=(const Point &b)
	{
		x -= b.x, y -= b.y, z -= b.z;
	}
	Point operator*(double p) const
	{
		return Point{ p*x, p*y, p*z };
	}
	Point& operator*=(const Point &p)
	{
		x *= p.x, y *= p.y, z *= p.z;
		return *this;
	}
	Point& operator*=(double p)
	{
		x *= p, y *= p, z *= p;
		return *this;
	}
	friend Point operator*(double p, const Point &x)
	{
		return Point{ p*x.x, p*x.y, p*x.z };
	}
	Point operator/(double p) const
	{
		return Point{ x / p,  y / p,  z / p };
	}
	friend double dot(const Point &a, const Point &b)
	{
		return a.x * b.x + a.y*b.y + a.z*b.z;
	}
	friend Point cross(const Point &a, const Point &b)
	{
		return Point{
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x
		};
	}
	friend Point cross(const Point &a, const Point &o, const Point &b)
	{
		return cross(a - o, b - o);
	}
	friend double norm(const Point &a)
	{
		return a.x*a.x + a.y*a.y + a.z*a.z;
	}
	friend double abs(const Point &a)
	{
		return sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
	}

	Point rotate(const Point &dir) const//将(0,0,1)转到dir方向
	{
		double sina = -dir.y, cosa = sqrt(1 - dir.y*dir.y);
		double xz = sqrt(dir.x * dir.x + dir.z * dir.z);
		double sinb = dir.x / xz, cosb = dir.z / xz;
		if (xz < eps) sinb = 0, cosb = 1;
		Point ans;
		ans.x = x * cosb + y * sina * sinb + z * cosa * sinb;
		ans.y = y * cosa - z * sina;
		ans.z = -x * sinb + y * sina * cosb + z * cosa * cosb;
		return ans;
	}
};

typedef Point* Vertex;

class Point2
{
public:
	union {
		struct { /*数组索引*/double p[2]; };
		struct { /*变量索引*/double x, y; };
	};

	Point2() {}
	Point2(double _x, double _y) :
		x(_x), y(_y)
	{}

	Point2 operator+(const Point2 &b) const
	{
		return Point2{ x + b.x, y + b.y };
	}
	Point2 operator+=(const Point2 &b)
	{
		x += b.x, y += b.y;
	}
	Point2 operator-(const Point2 &b) const
	{
		return Point2{ x - b.x, y - b.y};
	}
	Point2 operator-=(const Point2 &b)
	{
		x -= b.x, y -= b.y;
	}
	Point2 operator*(double p) const
	{
		return Point2{ p*x, p*y };
	}
	friend Point2 operator*(double p, const Point2 &x)
	{
		return Point2{ p*x.x, p*x.y };
	}
	Point2 operator/(double p) const
	{
		return Point2{ x / p, y / p };
	}
	friend double dot(const Point2 &a, const Point2 &b)
	{
		return a.x * b.x + a.y*b.y;
	}
	friend double cross(const Point2 &a, const Point2 &b)
	{
		return a.x*b.y - a.y*b.x;
	}
	friend double cross(const Point2 &a, const Point2 &o, const Point2 &b)
	{
		return cross(a - o, b - o);
	}
};


typedef Point Color;//r g b
inline double queryLuminiance(const Color &c)
{
	return c.x*0.299 + c.y*0.587 + c.z*0.114;
}