#include "stdafx.h"
#include "Object.h"

#pragma warning(disable:4996)
bool Object::Parse(FILE * fp)
{
	char buf[256];
	int nVertices, nTriangles;
	std::vector<Point> vecVertices;
	std::vector<std::array<int,3>> vecTriangles;

	nVertices = 0;
	nTriangles = 0;
	vecVertices.clear();
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
					vecVertices.push_back(vP);
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
			int v, n, t;
			std::array<int, 3> vIndices;
			if (fscanf(fp, "%s", buf) != 1)
			{
				fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
				return false;
			}

			/* can be one of %d, %d//%d, %d/%d, %d/%d/%d %d//%d */
			if (strstr(buf, "//"))
			{
				/* v//n */
				if (sscanf(buf, "%d//%d", &vIndices[0], &n) == 2 &&
					fscanf(fp, "%d//%d", &vIndices[1], &n) == 2 &&
					fscanf(fp, "%d//%d", &vIndices[2], &n) == 2)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
				}
				else
				{
					fprintf(stderr, "Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}

			}
			else if (sscanf(buf, "%d/%d/%d", &v, &t, &n) == 3)
			{
				/* v/t/n */
				vIndices[0] = v;
				if (fscanf(fp, "%d/%d/%d", &vIndices[1], &t, &n) == 3 &&
					fscanf(fp, "%d/%d/%d", &vIndices[2], &t, &n) == 3)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}
			else if (sscanf(buf, "%d/%d", &v, &t) == 2)
			{
				/* v/t */
				vIndices[0] = v;
				if (fscanf(fp, "%d/%d", &vIndices[1], &t) == 2 &&
					fscanf(fp, "%d/%d", &vIndices[2], &t) == 2)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}
			else
			{
				/* v */
				if (sscanf(buf, "%d", &vIndices[0]) == 1 &&
					fscanf(fp, "%d", &vIndices[1]) == 1 &&
					fscanf(fp, "%d", &vIndices[2]) == 1)
				{
					nTriangles++;
					vecTriangles.push_back(vIndices);
				}
				else
				{
					printf("Error: Wrong Face at Line %d\n", lineNumber);
					return false;
				}
			}

		}

		break;

		default:
			/* eat up rest of line */
			fgets(buf, sizeof(buf), fp);
			break;
		}
	}

	p = std::move(vecVertices);
	for (auto i : vecTriangles) {
		f.emplace_back(this, &p[i[0]-1], &p[i[1]-1], &p[i[2]-1]);
	}
	return true;
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
