#ifndef POLYGON_CLIPPING_H
#define POLYGON_CLIPPING_H

#include <windows.h>
#include <vector>
#include <algorithm> // For std::min and std::max
using namespace std;

struct Vertex {
    double x, y;
    Vertex(int x1 = 0, int y1 = 0) : x(x1), y(y1) {}
};

void DrawPixel(HDC hdc, int x, int y, COLORREF color = RGB(0, 0, 255));
void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, COLORREF color = RGB(0, 0, 255));

typedef vector<Vertex> VertexList;
typedef bool (*IsInFunc)(Vertex& v, int edge);
typedef Vertex(*IntersectFunc)(Vertex& v1, Vertex& v2, int edge);

bool InLeft(Vertex& v, int edge);
bool InRight(Vertex& v, int edge);
bool InTop(Vertex& v, int edge);
bool InBottom(Vertex& v, int edge);
Vertex VIntersect(Vertex& v1, Vertex& v2, int xedge);
Vertex HIntersect(Vertex& v1, Vertex& v2, int yedge);
int RoundPoly(double x);
VertexList ClipWithEdge(VertexList p, int edge, IsInFunc In, IntersectFunc Intersect);
void PolygonClip(HDC hdc, POINT* p, int n, int xleft, int ytop, int xright, int ybottom);

extern POINT clipPolygon[100];
extern int pointCount;
extern bool polygonDrawn;
extern const int defaultXLeft, defaultYTop, defaultXRight, defaultYBottom;

#endif // POLYGON_CLIPPING_H
