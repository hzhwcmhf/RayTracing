#pragma once

class Point
{
public:
	union {
		struct { /*��������*/double p[3]; };
		struct { /*��������*/double x, y, z; };
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
	Point operator-=(const Point &b)
	{
		x -= b.x, y -= b.y, z -= b.z;
	}
	Point operator*(double p) const
	{
		return Point{ p*x, p*y, p*z };
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
};

typedef Point* Vertex;

class Point2
{
public:
	union {
		struct { /*��������*/double p[2]; };
		struct { /*��������*/double x, y; };
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