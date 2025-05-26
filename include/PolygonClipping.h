#include <windows.h>
#include <vector>
using namespace std;

struct Vertex {
    double x, y;
    Vertex(int x1 = 0, int y1 = 0) : x(x1), y(y1) {}
};

void DrawPixel(HDC hdc, int x, int y, COLORREF color = RGB(0, 0, 255)) {
    SetPixel(hdc, x, y, color);
}

void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, COLORREF color = RGB(0, 0, 255)) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int d = 2 * dy - dx;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (steep) {
            DrawPixel(hdc, y, x, color);
        }
        else {
            DrawPixel(hdc, x, y, color);
        }

        if (d > 0) {
            y += ystep;
            d = d + 2 * (dy - dx);
        }
        else {
            d = d + 2 * dy;
        }
    }
}
typedef vector<Vertex> VertexList;
typedef bool (*IsInFunc)(Vertex& v, int edge);
typedef Vertex(*IntersectFunc)(Vertex& v1, Vertex& v2, int edge);
bool InLeft(Vertex& v, int edge) { return v.x >= edge; }
bool InRight(Vertex& v, int edge) { return v.x <= edge; }
bool InTop(Vertex& v, int edge) { return v.y >= edge; }
bool InBottom(Vertex& v, int edge) { return v.y <= edge; }
Vertex VIntersect(Vertex& v1, Vertex& v2, int xedge) {
    Vertex res;
    res.x = xedge;
    res.y = v1.y + (xedge - v1.x) * (v2.y - v1.y) / (v2.x - v1.x);
    return res;
}
Vertex HIntersect(Vertex& v1, Vertex& v2, int yedge) {
    Vertex res;
    res.y = yedge;
    res.x = v1.x + (yedge - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
    return res;
}
int Round(double x) {
    return int(x + 0.5);
}
VertexList ClipWithEdge(VertexList p, int edge, IsInFunc In, IntersectFunc Intersect) {
    VertexList OutList;
    Vertex v1 = p[p.size() - 1];
    bool v1_in = In(v1, edge);
    for (int i = 0; i < (int)p.size(); i++) {
        Vertex v2 = p[i];
        bool v2_in = In(v2, edge);
        if (!v1_in && v2_in) {
            OutList.push_back(Intersect(v1, v2, edge));
            OutList.push_back(v2);
        }
        else if (v1_in && v2_in) {
            OutList.push_back(v2);
        }
        else if (v1_in && !v2_in) {
            OutList.push_back(Intersect(v1, v2, edge));
        }
        v1 = v2;
        v1_in = v2_in;
    }
    return OutList;
}
void PolygonClip(HDC hdc, POINT* p, int n, int xleft, int ytop, int xright, int ybottom) {
    VertexList vlist;
    for (int i = 0; i < n; i++)
        vlist.push_back(Vertex(p[i].x, p[i].y));

    vlist = ClipWithEdge(vlist, xleft, InLeft, VIntersect);
    vlist = ClipWithEdge(vlist, ytop, InTop, HIntersect);
    vlist = ClipWithEdge(vlist, xright, InRight, VIntersect);
    vlist = ClipWithEdge(vlist, ybottom, InBottom, HIntersect);

    if (vlist.size() < 2) return;

    Vertex v1 = vlist[vlist.size() - 1];
    for (int i = 0; i < (int)vlist.size(); i++) {
        Vertex v2 = vlist[i];
        DrawLine(hdc, Round(v1.x), Round(v1.y), Round(v2.x), Round(v2.y));
        v1 = v2;
    }
}
POINT polygon[100];
int pointCount = 0;
bool polygonDrawn = false;
const int xleft = 200, ytop = 100, xright = 400, ybottom = 300;
// LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//     HDC hdc;
//     PAINTSTRUCT ps;

//     switch (msg) {
//     case WM_LBUTTONDOWN:
//         if (!polygonDrawn && pointCount < 100) {
//             polygon[pointCount].x = LOWORD(lParam);
//             polygon[pointCount].y = HIWORD(lParam);
//             pointCount++;
//         }
//         break;

//     case WM_RBUTTONDOWN:
//         polygonDrawn = true;
//         InvalidateRect(hwnd, NULL, TRUE);
//         break;

//     case WM_PAINT:
//         hdc = BeginPaint(hwnd, &ps);

//         DrawLine(hdc, xleft, ytop, xright, ytop);      // Top edge
//         DrawLine(hdc, xright, ytop, xright, ybottom);  // Right edge
//         DrawLine(hdc, xright, ybottom, xleft, ybottom); // Bottom edge
//         DrawLine(hdc, xleft, ybottom, xleft, ytop);    // Left edge


//         if (polygonDrawn && pointCount > 1) {
//             PolygonClip(hdc, polygon, pointCount, xleft, ytop, xright, ybottom);
//         }

//         EndPaint(hwnd, &ps);
//         break;

//     case WM_KEYDOWN:
//         if (wParam == VK_BACK) {
//             pointCount = 0;
//             polygonDrawn = false;
//             InvalidateRect(hwnd, NULL, TRUE);
//         }
//         break;

//     case WM_DESTROY:
//         PostQuitMessage(0);
//         break;
//     }

//     return DefWindowProc(hwnd, msg, wParam, lParam);
// }

// int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
//     WNDCLASSW wc = { 0 };
//     wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
//     wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//     wc.hInstance = hInst;
//     wc.lpszClassName = L"PolygonClipper";
//     wc.lpfnWndProc = WndProc;

//     if (!RegisterClassW(&wc))
//         return -1;

//     CreateWindowW(L"PolygonClipper", L"Sutherland-Hodgman Polygon Clipping",
//         WS_OVERLAPPEDWINDOW | WS_VISIBLE,
//         100, 100, 800, 600, NULL, NULL, NULL, NULL);

//     MSG msg = { 0 };

//     while (GetMessage(&msg, NULL, 0, 0)) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }

//     return 0;
// }
