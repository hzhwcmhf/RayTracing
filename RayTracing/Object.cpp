#include "stdafx.h"
#include "Object.h"
#include "KDtree.h"

#pragma warning(disable:4996)
bool Object::Parse(FILE * fp)
{
	char buf[256];
	std::vector<Point> vecVertices;
	std::vector<Point> vecNormals;
	std::vector<std::array<int,3>> vecTriangles;
	std::vector<std::array<int, 3>> vecTrianglesNormal;

	int nVertices = 0;
	int nNormals = 0;
	int nTriangles = 0;
	vecVertices.clear();
	vecNormals.clear();
	vecTriangles.clear();
	int lineNumber = 0;

	while (fscanf(fp, "%s", buf) != EOF)
	{
		lineNumber++;
		switch (buf[0])
		{
		case '#':				/* comment */
								/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		case 'v':				/* v, vn, vt */
			switch (buf[1])
			{
			case '\0':			    /* vertex */
				{
					Point vP;
					if (fscanf(fp, "%lf %lf %lf",
						&vP.x,
						&vP.y,
						&vP.z) == 3)
					{
						nVertices++;
						vecVertices.push_back(transform(vP));
					}
					else
					{
						fprintf(stderr, "Error: Wrong Number of Values(Should be 3). at Line %d\n", lineNumber);
						return false;
					}
				}
				break;
			case 'n':
				{
					Point vn;
					if (fscanf(fp, "%lf %lf %lf",
						&vn.x,
						&vn.y,
						&vn.z) == 3)
					{
						nNormals++;
						vecNormals.push_back(transformN(vn));
					}
					else
					{
						fprintf(stderr, "Error: Wrong Number of Values(Should be 3). at Line %d\n", lineNumber);
						return false;
					}

				}
				break;
			default:
				/* eat up rest of line */
				fgets(buf, sizeof(buf), fp);
				break;
			}
			break;

		case 'f':				/* face */
		{
			
			if (fscanf(fp, "%s", buf) != 1)
			{
				fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
				return false;
			}

			int v, n, t;
			if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
			{
				/* v/t/n */

				std::array<int, 3> vIndices, vnIndices;

				vIndices[0] = v;
				vnIndices[0] = n;
				if (fscanf(fp, "%d/%d/%d", &vIndices[1], &t, &vnIndices[1]) == 3 &&
					fscanf(fp, "%d/%d/%d", &vIndices[2], &t, &vnIndices[2]) == 3)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
					vecTrianglesNormal.push_back(vnIndices);
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			} else if(sscanf(buf, "%d//%d", &v, &n) == 2){
				/* v//n */

				std::array<int, 3> vIndices, vnIndices;

				vIndices[0] = v;
				vnIndices[0] = n;
				if (fscanf(fp, "%d//%d", &vIndices[1], &vnIndices[1]) == 2 &&
					fscanf(fp, "%d//%d", &vIndices[2], &vnIndices[2]) == 2)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
					vecTrianglesNormal.push_back(vnIndices);
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}

			} else {
				printf("Error: Wrong Face at Line %d\n", lineNumber);
				return false;
			}

			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			//if (strstr(buf, "//"))
			//{
			//	/* v//n */
			//	int v, n, t;
			//	std::array<int, 3> vIndices;
			//	if (sscanf(buf, "%d//%d", &vIndices[0], &n) == 2 &&
			//		fscanf(fp, "%d//%d", &vIndices[1], &n) == 2 &&
			//		fscanf(fp, "%d//%d", &vIndices[2], &n) == 2)
			//	{
			//		nTriangles++;
			//		vecTriangles.push_back(vIndices);
			//	}
			//	else
			//	{
			//		fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
			//		return false;
			//	}

			//}
			//else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
			//{
			//	/* v/t/n */
			//	vIndices[0] = v;
			//	if (fscanf(fp, "%d/%d/%d", &vIndices[1], &t, &n) == 3 &&
			//		fscanf(fp, "%d/%d/%d", &vIndices[2], &t, &n) == 3)
			//	{
			//		nTriangles++;
			//		vecTriangles.push_back(vIndices);
			//	}
			//	else
			//	{
			//		printf("Error: Wrong Face at Line %d\n", lineNumber);
			//		return false;
			//	}
			//}
			//else if (sscanf(buf, "%d/%d", &v, &t) == 2)
			//{
			//	/* v/t */
			//	vIndices[0] = v;
			//	if (fscanf(fp, "%d/%d", &vIndices[1], &t) == 2 &&
			//		fscanf(fp, "%d/%d", &vIndices[2], &t) == 2)
			//	{
			//		nTriangles++;
			//		vecTriangles.push_back(vIndices);
			//	}
			//	else
			//	{
			//		printf("Error: Wrong Face at Line %d\n", lineNumber);
			//		return false;
			//	}
			//}
			//else
			//{
			//	/* v */
			//	if (sscanf(buf, "%d", &vIndices[0]) == 1 &&
			//		fscanf(fp, "%d", &vIndices[1]) == 1 &&
			//		fscanf(fp, "%d", &vIndices[2]) == 1)
			//	{
			//		nTriangles++;
			//		vecTriangles.push_back(vIndices);
			//	}
			//	else
			//	{
			//		printf("Error: Wrong Face at Line %d\n", lineNumber);
			//		return false;
			//	}
			//}

		}

		break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		}
	}

	p = std::move(vecVertices);
	pn = std::move(vecNormals);
	for(int i = 0;i<nTriangles; i++){
		f.emplace_back(this, &p[vecTriangles[i][0]-1], &p[vecTriangles[i][1]-1], &p[vecTriangles[i][2]-1],
							&pn[vecTrianglesNormal[i][0]-1], &pn[vecTrianglesNormal[i][1]-1], &pn[vecTrianglesNormal[i][2]-1]);
	}
	return true;
}

Point Object::transform(Point p) const
{
	p = p.rotatex(rotatex);
	p = p.rotatey(rotatey);
	p = p.rotatez(rotatez);
	return p;
}

Point Object::transformN(Point p) const
{
	p = p.rotatex(rotatex);
	p = p.rotatey(rotatey);
	p = p.rotatez(rotatez);
	return p;
}

void Object::replace(double xmin, double xmax, double ymin, double ymax, double zmin, double zmax)
{
	KDtree::BorderBox box;
	box.init(f[0]);
	for (auto &x : p) {
		box.addPoint(x);
	}
	double scale = std::min({
		(xmax - xmin) / (box.x2 - box.x1),
		(ymax - ymin) / (box.y2 - box.y1),
		(zmax - zmin) / (box.z2 - box.z1)
	});
	for (auto &x : p) {
		x.x = (x.x - (box.x1 + box.x2) / 2) * scale + (xmin + xmax) / 2;
		x.y = (x.y - (box.y1 + box.y2) / 2) * scale + (ymin + ymax) / 2;
		x.z = (x.z - (box.z1 + box.z2) / 2) * scale + (zmin + zmax) / 2;
	}
	std::cerr << "replace pos:"
		<< (box.x1 - (box.x1 + box.x2) / 2) * scale + (xmin + xmax) / 2 << "-"
		<< (box.x2 - (box.x1 + box.x2) / 2) * scale + (xmin + xmax) / 2 << ","
		<< (box.y1 - (box.y1 + box.y2) / 2) * scale + (ymin + ymax) / 2 << "-"
		<< (box.y2 - (box.y1 + box.y2) / 2) * scale + (ymin + ymax) / 2 << ","
		<< (box.z1 - (box.z1 + box.z2) / 2) * scale + (zmin + zmax) / 2 << "-"
		<< (box.z2 - (box.z1 + box.z2) / 2) * scale + (zmin + zmax) / 2 << std::endl;
}

void Object::rerotate(double rotatex, double rotatey, double rotatez)
{
	this->rotatex = rotatex;
	this->rotatey = rotatey;
	this->rotatez = rotatez;
	for (auto &x : p) {
		x = transform(x);
	}
	for (auto &x : pn) {
		x = transformN(x);
	}
}

bool Object::Load(const char * filename)
{
	FILE* fp = fopen(filename, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Error: Loading %s failed.\n", filename);
		return false;
	}
	else
	{
		if (Parse(fp))
		{
			fprintf(stderr, "Loading from %s successfully.\n", filename);
			fprintf(stderr, "Vertex Number = %d\n", p.size());
			fprintf(stderr, "Triangle Number = %d\n",f.size());
			fclose(fp);
			return true;
		}
		else
		{
			fclose(fp);
			return false;
		}
	}
}
