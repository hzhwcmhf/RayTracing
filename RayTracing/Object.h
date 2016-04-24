#pragma once

#include "Face.h"

class Object
{
private:
	bool Parse(FILE* fp);
public:
	std::vector<Point> p;
	std::vector<Face> f;
	
	Object() {}

	bool Load(const char* filename);
};
