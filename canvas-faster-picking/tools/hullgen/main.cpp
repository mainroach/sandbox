
/*
A simple program that will calculate the convex hull of a sprite based upon alpha.

*/

#include <vector>
#include "imgHelper.h"

struct point
{
	int x;
	int y;
	bool operator <(const point &p) const {
                return x < p.x || (x == p.x && y < p.y);
        }

};

//FROM http://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
// Implementation of Andrew's monotone chain 2D convex hull algorithm.
// Asymptotic complexity: O(n log n).
// Practical performance: 0.5-1.0 seconds for n=1000000 on a 1GHz machine.
#include <algorithm>

 //-----------------------------------
// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
int cross(const point &O, const point &A, const point &B)
{
        return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}
 //-----------------------------------
// Returns a list of points on the convex hull in counter-clockwise order.
// Note: the last point in the returned list is the same as the first one.
std::vector<point> convex_hull(std::vector<point> P)
{
        int n = P.size(), k = 0;
        std::vector<point> H(2*n);
 
        // Sort points lexicographically
        sort(P.begin(), P.end());
 
        // Build lower hull
        for (int i = 0; i < n; i++) {
                while (k >= 2 && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
                H[k++] = P[i];
        }
 
        // Build upper hull
        for (int i = n-2, t = k+1; i >= 0; i--) {
                while (k >= t && cross(H[k-2], H[k-1], P[i]) <= 0) k--;
                H[k++] = P[i];
        }
 
        H.resize(k);
        return H;
}
//-----------------------------------

void calcOuterPoints(ImageData& id, std::vector<point>& outPoints, const uint32 threshold)
{
	//walk each scanline
	for(int y = 0; y < id.height; y++)
	{
		int idx = (y * id.width) * 4;

		int start = 0;
		//find the outer bounds of this scan-line
		for(int x = 0; x < id.width; x++)
		{
			if(id.pImgData[idx + 3] >= threshold)
			{
				point pt;
				pt.x = x;
				pt.y = y;
				outPoints.push_back(pt);
				start = x;
				break;
			}
	
			idx+=4;
		}

		//find the outer bounds of this scan-line
		idx = (((y+1) * id.width) * 4) -4;
		for(int x = id.width-1; x >start; x--)
		{
			if(id.pImgData[idx + 3] != 0)
			{
				point pt;
				pt.x = x;
				pt.y = y;
				outPoints.push_back(pt);
				break;
			}
	
			idx-=4;
		}

	}
}
//-----------------------------------
void writeJSON(const char* pFilename, std::vector<point>& pts)
{
	char outName[512];
	sprintf(outName,"%s.json",pFilename);
	
	char outStr[128];
	FILE* pOut = fopen(outName,"wt");

	sprintf(outStr,"{\"name\":\"%s\", \"hull\":[{\"x\":%i,\"y\":%i}",pFilename,pts[0].x, pts[0].y);
	fwrite(outStr,strlen(outStr),1,pOut);

	for(int i =1; i < pts.size();i++)
	{
		sprintf(outStr,", {\"x\":%i,\"y\":%i}",pts[i].x, pts[i].y);
		fwrite(outStr,strlen(outStr),1,pOut);
	}

	sprintf(outStr,"]}\n");
	fwrite(outStr,strlen(outStr),1,pOut);

	fclose(pOut);
}

//-----------------------------------
//RUN with "hullbuild.exe textures/3.png 50"
int main(int argc, char *argv[])
{
	
	if (argc < 3)
	{
		fprintf(stderr,
			"\nGenSpriteHull <imagefile> <threshold 0-255>\n\n"
		);

		return 0;
	}



	const char *pFilename = argv[1];
	const int threshold = atoi(argv[2]);


	ImageData id;

	if(!loadImageData(pFilename, id))
	{
		fprintf(stderr,"\n ERROR could not load image data\n\n");
		return -1;
	}

	std::vector<point> outPoints;
	calcOuterPoints(id,  outPoints, threshold);

	if(outPoints.size() == 0)
	{
		fprintf(stderr,"\n ERROR no points returned from calculation of outer points?!?!\n\n");
		return -1;
	}

	std::vector<point> pts = convex_hull(outPoints);

	if(pts.size() == 0)
	{
		fprintf(stderr,"\n ERROR no points returned from convex hull creation?!?!\n\n");
		return -1;
	}

	writeJSON(pFilename,pts);
	

	return 0;
}