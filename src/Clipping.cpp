#include <windows.h>
#include <algorithm> // For min and max functions
#include <vector>
#include <stdio.h>  // For printf and sprintf
using std::min;
using std::max;
using std::vector;

/*****************************************************************************
 * DATA STRUCTURES AND TYPE DEFINITIONS
 *****************************************************************************/

// Point structure for 2D coordinates
struct Point {
    double x, y;
};

// Vertex structure for polygon clipping
struct Vertex {
    double x, y;
    Vertex(int x1 = 0, int y1 = 0) : x(x1), y(y1) {}
};

// Region code structure for Cohen-Sutherland algorithm
union Code {
    unsigned all : 4;
    struct {
        unsigned left : 1;
        unsigned right : 1;
        unsigned bottom : 1;
        unsigned top : 1;
    };
};

// Type definitions for polygon clipping
typedef vector<Vertex> VertexList;
typedef bool (*IsInFunc)(Vertex& v, int edge);
typedef Vertex(*IntersectFunc)(Vertex& v1, Vertex& v2, int edge);

/*****************************************************************************
 * FUNCTION DECLARATIONS
 *****************************************************************************/

// Drawing utilities
void DrawCircle(HDC hdc, int xc, int yc, int radius, COLORREF color = RGB(255, 0, 0));
void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, COLORREF color = RGB(0, 0, 255));
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color = RGB(0, 255, 0));

// Debug utilities
void debugCoordinates(const char* message, int x, int y);
static int Round(double x);

// Clipping window management
void drawWindowBorder(HWND hwnd);
void drawClippingWindow(HWND hwnd);
void getSquareWindow();
void initClippingWindow();
void resetClippingStates();

// Line clipping
Code getCode(double x, double y);
void verticalIntersection(double xs, double ys, double xe, double ye, int x, double* xi, double* yi);
void horizontalIntersection(double xs, double ys, double xe, double ye, int y, double* xi, double* yi);
void CohenSuth(HDC hdc, double xs, double ys, double xe, double ye);
void PointClipping(HWND hwnd, Point p);

// Polygon clipping
bool InLeft(Vertex& v, int edge);
bool InRight(Vertex& v, int edge);
bool InTop(Vertex& v, int edge);
bool InBottom(Vertex& v, int edge);
Vertex VIntersect(Vertex& v1, Vertex& v2, int xedge);
Vertex HIntersect(Vertex& v1, Vertex& v2, int yedge);
VertexList ClipWithEdge(VertexList p, int edge, IsInFunc In, IntersectFunc Intersect);
void PolygonClip(HDC hdc, POINT* p, int n, int xleft, int ytop, int xright, int ybottom);

// User interaction handlers
bool handleLineClippingInput(HWND hwnd, int x, int y, bool isRightClick);
bool handlePolygonClippingInput(HWND hwnd, int x, int y, bool isRightClick);
void startLineClippingMode();
void startPolygonClippingMode();

/*****************************************************************************
 * GLOBAL VARIABLES
 *****************************************************************************/

// Clipping state flags
bool clippingMode = false;
bool lineClippingMode = false;
bool polygonClippingMode = false;
bool definingClipWindow = false;
bool drawingPolygon = false;
bool clipWindowDefined = false;
bool polygonDrawn = false;

// Clipping window coordinates
double xleft, xright, ytop, ybottom;
const int defaultXLeft = 200, defaultYTop = 100, defaultXRight = 400, defaultYBottom = 300;

// Input tracking
Point in[2];  // Input points for clipping window
const int polygonSize = 2;
Point linePoints[2];  // Line endpoints
int cnt = 0;  // Counter for window corner points
int cnt2 = 0; // Counter for line points
POINT clipPolygon[100];  // Polygon vertices
int pointCount = 0;  // Number of polygon vertices

/*****************************************************************************
 * UTILITY FUNCTIONS
 *****************************************************************************/

// Round a double to the nearest integer
static int Round(double x) {
    return (int)(x + 0.5);
}

// Debug function to show coordinates
void debugCoordinates(const char* message, int x, int y) {
    char debugMsg[256];
    sprintf(debugMsg, "%s: Coordinates (%d, %d)", message, x, y);
    OutputDebugStringA(debugMsg);
}

/*****************************************************************************
 * DRAWING FUNCTIONS
 *****************************************************************************/

// Draw a circle using midpoint algorithm
void DrawCircle(HDC hdc, int xc, int yc, int radius, COLORREF color) {
    int x = 0;
    int y = radius;
    int d = 1 - radius;
    int deltaE = 3;
    int deltaSE = -2 * radius + 5;

    while (y >= x) {
        // Draw 8 octants of the circle
        SetPixel(hdc, xc + x, yc + y, color);
        SetPixel(hdc, xc - x, yc + y, color);
        SetPixel(hdc, xc + x, yc - y, color);
        SetPixel(hdc, xc - x, yc - y, color);
        SetPixel(hdc, xc + y, yc + x, color);
        SetPixel(hdc, xc - y, yc + x, color);
        SetPixel(hdc, xc + y, yc - x, color);
        SetPixel(hdc, xc - y, yc - x, color);

        if (d < 0) {
            d += deltaE;
            deltaE += 2;
            deltaSE += 2;
        } else {
            d += deltaSE;
            deltaE += 2;
            deltaSE += 4;
            y--;
        }
        x++;
    }
}

// Draw a line using midpoint algorithm
void DrawLine(HDC hdc, int x0, int y0, int x1, int y1, COLORREF color) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    
    // If the line is steep, swap x and y
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    
    // Make sure x0 <= x1
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
            SetPixel(hdc, y, x, color);
        } else {
            SetPixel(hdc, x, y, color);
        }

        if (d > 0) {
            y += ystep;
            d = d + 2 * (dy - dx);
        } else {
            d = d + 2 * dy;
        }
    }
}

// Alternative line drawing implementation using midpoint algorithm
// Used specifically for clipped lines
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int x = x1, y = y1;

    if (dy <= dx) {
        // Slope <= 1
        int d = 2 * dy - dx;
        int incrE = 2 * dy;
        int incrNE = 2 * (dy - dx);
        
        SetPixel(hdc, x, y, color);
        for (int i = 0; i < dx; ++i) {
            if (d <= 0) {
                d += incrE;
                x += sx;
            } else {
                d += incrNE;
                x += sx;
                y += sy;
            }
            SetPixel(hdc, x, y, color);
        }
    } else {
        // Slope > 1
        int d = 2 * dx - dy;
        int incrN = 2 * dx;
        int incrNE = 2 * (dx - dy);
        
        SetPixel(hdc, x, y, color);
        for (int i = 0; i < dy; ++i) {
            if (d <= 0) {
                d += incrN;
                y += sy;
            } else {
                d += incrNE;
                x += sx;
                y += sy;
            }
            SetPixel(hdc, x, y, color);
        }
    }
}

/*****************************************************************************
 * CLIPPING WINDOW MANAGEMENT
 *****************************************************************************/

// Calculate square window from input points
void getSquareWindow() {
    xleft = min(in[0].x, in[1].x);
    xright = max(in[0].x, in[1].x);
    int side = abs((int)(in[0].x - in[1].x));
    
    if (in[1].y < in[0].y) {
        ytop = in[0].y - side;
        ybottom = in[0].y;
    } else {
        ytop = in[0].y;
        ybottom = in[0].y + side;
    }
    
    char debugMsg[256];
    sprintf(debugMsg, "Square window calculated: left=%d, top=%d, right=%d, bottom=%d", 
            (int)xleft, (int)ytop, (int)xright, (int)ybottom);
    OutputDebugStringA(debugMsg);
}

// Draw the clipping window border
void drawWindowBorder(HWND hwnd) {
    HDC hdc = GetDC(hwnd);

    char debugMsg[256];
    sprintf(debugMsg, "Drawing window border at: left=%d, top=%d, right=%d, bottom=%d", 
            (int)xleft, (int)ytop, (int)xright, (int)ybottom);
    OutputDebugStringA(debugMsg);

    // Draw the rectangle for the clipping window
    DrawLine(hdc, xleft, ytop, xright, ytop, RGB(255, 0, 0));
    DrawLine(hdc, xright, ytop, xright, ybottom, RGB(255, 0, 0));
    DrawLine(hdc, xright, ybottom, xleft, ybottom, RGB(255, 0, 0));
    DrawLine(hdc, xleft, ybottom, xleft, ytop, RGB(255, 0, 0));

    ReleaseDC(hwnd, hdc);
}

// Draw the current clipping window
void drawClippingWindow(HWND hwnd) {
    if (!clipWindowDefined) return;
    drawWindowBorder(hwnd);
}

// Initialize clipping window with default values
void initClippingWindow() {
    xleft = defaultXLeft;
    ytop = defaultYTop;
    xright = defaultXRight;
    ybottom = defaultYBottom;
}

// Reset all clipping states
void resetClippingStates() {
    clippingMode = false;
    lineClippingMode = false;
    polygonClippingMode = false;
    definingClipWindow = false;
    drawingPolygon = false;
    clipWindowDefined = false;
    cnt = 0;
    cnt2 = 0;
    pointCount = 0;
    polygonDrawn = false;
}

/*****************************************************************************
 * LINE CLIPPING FUNCTIONS - COHEN-SUTHERLAND ALGORITHM
 *****************************************************************************/

// Get region code for Cohen-Sutherland algorithm
Code getCode(double x, double y) {
    Code code = { 0 };
    if (x < xleft) code.left = 1;
    else if (x > xright) code.right = 1;
    if (y < ytop) code.top = 1;
    else if (y > ybottom) code.bottom = 1;
    return code;
}

// Find vertical intersection with clipping boundary
void verticalIntersection(double xs, double ys, double xe, double ye, int x, double* xi, double* yi) {
    *xi = x;
    *yi = ys + (ye - ys) * (x - xs) / (xe - xs);
}

// Find horizontal intersection with clipping boundary
void horizontalIntersection(double xs, double ys, double xe, double ye, int y, double* xi, double* yi) {
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

// Point clipping check
void PointClipping(HWND hwnd, Point p) {
    if (p.x >= xleft && p.x <= xright && p.y >= ytop && p.y <= ybottom) {
        HDC hdc = GetDC(hwnd);
        SetPixel(hdc, p.x, p.y, RGB(255, 0, 0));
        ReleaseDC(hwnd, hdc);
    }

    printf("Clicked point: (%d, %d)\n", (int)p.x, (int)p.y);
    printf("xleft: %d, xright: %d, ytop: %d, ybottom: %d\n", (int)xleft, (int)xright, (int)ytop, (int)ybottom);
}

// Cohen-Sutherland line clipping algorithm
void CohenSuth(HDC hdc, double xs, double ys, double xe, double ye) {
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    Code c1 = getCode(x1, y1);
    Code c2 = getCode(x2, y2);

    while ((c1.all || c2.all) && !(c1.all & c2.all)) {
        double xi, yi;
        
        if (c1.all) {
            // Clip first endpoint
            if (c1.left) 
                verticalIntersection(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (c1.right) 
                verticalIntersection(x1, y1, x2, y2, xright, &xi, &yi);
            else if (c1.top) 
                horizontalIntersection(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (c1.bottom) 
                horizontalIntersection(x1, y1, x2, y2, ybottom, &xi, &yi);
                
            x1 = xi;
            y1 = yi;
            c1 = getCode(x1, y1);
        } else {
            // Clip second endpoint
            if (c2.left) 
                verticalIntersection(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (c2.right) 
                verticalIntersection(x1, y1, x2, y2, xright, &xi, &yi);
            else if (c2.top) 
                horizontalIntersection(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (c2.bottom) 
                horizontalIntersection(x1, y1, x2, y2, ybottom, &xi, &yi);
                
            x2 = xi;
            y2 = yi;
            c2 = getCode(x2, y2);
        }
    }

    // Draw the line if it's visible
    if (!(c1.all | c2.all)) {
        DrawLineMidpoint(hdc, Round(x1), Round(y1), Round(x2), Round(y2));
    }
}

/*****************************************************************************
 * POLYGON CLIPPING FUNCTIONS - SUTHERLAND-HODGMAN ALGORITHM
 *****************************************************************************/

// Check if vertex is inside left edge
bool InLeft(Vertex& v, int edge) { 
    return v.x >= edge; 
}

// Check if vertex is inside right edge
bool InRight(Vertex& v, int edge) { 
    return v.x <= edge; 
}

// Check if vertex is inside top edge
bool InTop(Vertex& v, int edge) { 
    return v.y >= edge; 
}

// Check if vertex is inside bottom edge
bool InBottom(Vertex& v, int edge) { 
    return v.y <= edge; 
}

// Find intersection with vertical edge
Vertex VIntersect(Vertex& v1, Vertex& v2, int xedge) {
    Vertex res;
    res.x = xedge;
    res.y = v1.y + (xedge - v1.x) * (v2.y - v1.y) / (v2.x - v1.x);
    return res;
}

// Find intersection with horizontal edge
Vertex HIntersect(Vertex& v1, Vertex& v2, int yedge) {
    Vertex res;
    res.y = yedge;
    res.x = v1.x + (yedge - v1.y) * (v2.x - v1.x) / (v2.y - v1.y);
    return res;
}

// Clip polygon against one edge
VertexList ClipWithEdge(VertexList p, int edge, IsInFunc In, IntersectFunc Intersect) {
    VertexList OutList;
    Vertex v1 = p[p.size() - 1];
    bool v1_in = In(v1, edge);
    
    for (int i = 0; i < (int)p.size(); i++) {
        Vertex v2 = p[i];
        bool v2_in = In(v2, edge);
        
        if (!v1_in && v2_in) {
            // Case 1: Outside to inside
            OutList.push_back(Intersect(v1, v2, edge));
            OutList.push_back(v2);
        } else if (v1_in && v2_in) {
            // Case 2: Inside to inside
            OutList.push_back(v2);
        } else if (v1_in && !v2_in) {
            // Case 3: Inside to outside
            OutList.push_back(Intersect(v1, v2, edge));
        }
        // Case 4: Outside to outside - add nothing
        
        v1 = v2;
        v1_in = v2_in;
    }
    
    return OutList;
}

// Sutherland-Hodgman polygon clipping algorithm
void PolygonClip(HDC hdc, POINT* p, int n, int xleft, int ytop, int xright, int ybottom) {
    // Convert POINT array to VertexList
    VertexList vlist;
    for (int i = 0; i < n; i++)
        vlist.push_back(Vertex(p[i].x, p[i].y));

    // Clip against each edge
    vlist = ClipWithEdge(vlist, xleft, InLeft, VIntersect);
    vlist = ClipWithEdge(vlist, ytop, InTop, HIntersect);
    vlist = ClipWithEdge(vlist, xright, InRight, VIntersect);
    vlist = ClipWithEdge(vlist, ybottom, InBottom, HIntersect);

    if (vlist.size() < 2) return;  // No visible polygon

    // Draw the clipped polygon
    Vertex v1 = vlist[vlist.size() - 1];
    for (int i = 0; i < (int)vlist.size(); i++) {
        Vertex v2 = vlist[i];
        DrawLine(hdc, Round(v1.x), Round(v1.y), Round(v2.x), Round(v2.y), RGB(0, 0, 255));
        v1 = v2;
    }
}

/*****************************************************************************
 * USER INTERACTION HANDLERS
 *****************************************************************************/

// Handle line clipping input
bool handleLineClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (!lineClippingMode) return false;

    // Debug the incoming coordinates
    debugCoordinates("Line Clipping Input", x, y);
    
    char debugMsg[256];
    sprintf(debugMsg, "Line Clipping: Right=%d, DefiningWindow=%d, WindowDefined=%d, cnt=%d, cnt2=%d, coords=(%d,%d)",
            isRightClick ? 1 : 0, definingClipWindow ? 1 : 0, clipWindowDefined ? 1 : 0, cnt, cnt2, x, y);
    OutputDebugStringA(debugMsg);

    if (isRightClick && !clipWindowDefined) {
        // Define window with right-clicks
        if (cnt == 0) {
            // First corner
            in[0].x = x;
            in[0].y = y;
            cnt++;
            
            // Draw dot for first corner
            HDC hdc = GetDC(hwnd);
            sprintf(debugMsg, "Drawing first window corner at: (%d, %d)", x, y);
            OutputDebugStringA(debugMsg);
            DrawCircle(hdc, x, y, 3, RGB(255, 0, 0));
            ReleaseDC(hwnd, hdc);
            
            MessageBoxW(hwnd, L"First corner of clipping window set.\nRight-click again to set second corner.", 
                      L"Define Clipping Window", MB_OK | MB_ICONINFORMATION);
            return true;
        } else if (cnt == 1) {
            // Second corner
            in[1].x = x;
            in[1].y = y;
            cnt++;
            
            // Calculate and draw window
            sprintf(debugMsg, "Second corner at: (%d, %d), about to calculate window", x, y);
            OutputDebugStringA(debugMsg);
            
            getSquareWindow();
            drawWindowBorder(hwnd);
            definingClipWindow = true;
            clipWindowDefined = true;
            
            MessageBoxW(hwnd, L"Clipping window defined.\nNow use left clicks to define line endpoints.", 
                      L"Line Clipping", MB_OK | MB_ICONINFORMATION);
            return true;
        }
    } else if (clipWindowDefined && !isRightClick) {
        // Draw line with left clicks after window is defined
        if (cnt2 < polygonSize) {
            // Add line endpoint
            linePoints[cnt2].x = x;
            linePoints[cnt2].y = y;
            cnt2++;
            
            // Draw dot for line point
            HDC hdc = GetDC(hwnd);
            sprintf(debugMsg, "Drawing line point %d at: (%d, %d)", cnt2, x, y);
            OutputDebugStringA(debugMsg);
            DrawCircle(hdc, x, y, 3, RGB(0, 255, 0));
            ReleaseDC(hwnd, hdc);
            
            sprintf(debugMsg, "Line point %d added at (%d, %d)", cnt2, x, y);
            OutputDebugStringA(debugMsg);
            
            if (cnt2 == polygonSize) {
                // Both endpoints defined, perform clipping
                HDC hdc = GetDC(hwnd);
                
                // Draw original line
                DrawLine(hdc, linePoints[0].x, linePoints[0].y, linePoints[1].x, linePoints[1].y, RGB(255, 255, 255));
                
                // Apply clipping algorithm
                CohenSuth(hdc, linePoints[0].x, linePoints[0].y, linePoints[1].x, linePoints[1].y);
                
                ReleaseDC(hwnd, hdc);
                cnt2 = 0; // Reset for next line
                
                MessageBoxW(hwnd, L"Line clipped. You can define another line by left-clicking twice again.", 
                          L"Line Clipping", MB_OK | MB_ICONINFORMATION);
            }
            return true;
        }
    }

    return false;
}

// Handle polygon clipping input
bool handlePolygonClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (!polygonClippingMode) return false;

    // Debug the incoming coordinates
    debugCoordinates("Polygon Clipping Input", x, y);
    
    char debugMsg[256];
    sprintf(debugMsg, "Polygon Clipping: Right=%d, DefiningWindow=%d, WindowDefined=%d, DrawingPoly=%d, cnt=%d, pointCount=%d, coords=(%d,%d)",
            isRightClick ? 1 : 0, definingClipWindow ? 1 : 0, clipWindowDefined ? 1 : 0, drawingPolygon ? 1 : 0, cnt, pointCount, x, y);
    OutputDebugStringA(debugMsg);

    if (isRightClick && !clipWindowDefined) {
        // Define window with right-clicks (same as line clipping)
        if (cnt == 0) {
            // First corner
            in[0].x = x;
            in[0].y = y;
            cnt++;
            
            // Draw dot for first corner
            HDC hdc = GetDC(hwnd);
            sprintf(debugMsg, "Drawing first polygon window corner at: (%d, %d)", x, y);
            OutputDebugStringA(debugMsg);
            DrawCircle(hdc, x, y, 3, RGB(255, 0, 0));
            ReleaseDC(hwnd, hdc);
            
            MessageBoxW(hwnd, L"First corner of clipping window set.\nRight-click again to set second corner.", 
                       L"Define Clipping Window", MB_OK | MB_ICONINFORMATION);
            return true;
        } else if (cnt == 1) {
            // Second corner
            in[1].x = x;
            in[1].y = y;
            cnt++;
            
            // Calculate and draw window
            sprintf(debugMsg, "Second corner at: (%d, %d), about to calculate window", x, y);
            OutputDebugStringA(debugMsg);
            
            getSquareWindow();
            drawWindowBorder(hwnd);
            definingClipWindow = true;
            clipWindowDefined = true;
            
            MessageBoxW(hwnd, L"Clipping window defined.\nNow use left clicks to draw polygon vertices.\nRight-click to complete polygon.", 
                       L"Polygon Clipping", MB_OK | MB_ICONINFORMATION);
            return true;
        }
    } else if (clipWindowDefined) {
        if (!isRightClick && !polygonDrawn && pointCount < 100) {
            // Add polygon vertex with left click
            clipPolygon[pointCount].x = x;
            clipPolygon[pointCount].y = y;
            pointCount++;
            
            sprintf(debugMsg, "Added polygon point %d at (%d, %d)", pointCount, x, y);
            OutputDebugStringA(debugMsg);
            
            // Draw a dot for the vertex
            HDC hdc = GetDC(hwnd);
            sprintf(debugMsg, "Drawing polygon vertex at: (%d, %d)", x, y);
            OutputDebugStringA(debugMsg);
            DrawCircle(hdc, x, y, 3, RGB(0, 0, 255));
            
            // Draw connecting lines
            if (pointCount > 1) {
                DrawLine(hdc, clipPolygon[pointCount-2].x, clipPolygon[pointCount-2].y, 
                         clipPolygon[pointCount-1].x, clipPolygon[pointCount-1].y, RGB(255, 255, 255));
            }
            
            ReleaseDC(hwnd, hdc);
            drawingPolygon = true;
            return true;
        } else if (isRightClick && drawingPolygon && pointCount > 2) {
            // Complete polygon with right click
            HDC hdc = GetDC(hwnd);
            
            // Draw closing edge
            DrawLine(hdc, clipPolygon[pointCount-1].x, clipPolygon[pointCount-1].y, 
                    clipPolygon[0].x, clipPolygon[0].y, RGB(255, 255, 255));
            
            // Apply clipping algorithm
            PolygonClip(hdc, clipPolygon, pointCount, xleft, ytop, xright, ybottom);
            
            ReleaseDC(hwnd, hdc);
            polygonDrawn = true;
            
            // Reset for next polygon
            drawingPolygon = false;
            pointCount = 0;
            
            MessageBoxW(hwnd, L"Polygon clipped. You can draw another polygon with left clicks.", 
                      L"Polygon Clipping", MB_OK | MB_ICONINFORMATION);
            
            return true;
        }
    }

    return false;
}

// Start line clipping mode
void startLineClippingMode() {
    // Don't reset everything if window already defined
    if (!clipWindowDefined) {
        resetClippingStates();
    } else {
        // Keep the window but reset other state
        lineClippingMode = true;
        polygonClippingMode = false;
        drawingPolygon = false;
        cnt2 = 0;
        pointCount = 0;
        polygonDrawn = false;
    }
    
    lineClippingMode = true;
    clippingMode = true;
    
    if (clipWindowDefined) {
        definingClipWindow = true;
        // Draw the window again to make it visible
        drawClippingWindow(GetActiveWindow());
        
        // Show instructions for line drawing
        MessageBoxW(NULL, 
                  L"Line Clipping:\n\n"
                  L"Using existing clipping window.\n"
                  L"Left-click twice to define the line to be clipped\n"
                  L"The clipped line will be displayed in green",
                  L"Line Clipping Instructions", MB_OK | MB_ICONINFORMATION);
    } else {
        // Display initial instructions
        MessageBoxW(NULL, 
                  L"Line Clipping:\n\n"
                  L"1. Right-click twice to define the clipping window\n"
                  L"2. Left-click twice to define the line to be clipped\n"
                  L"3. The clipped line will be displayed in green",
                  L"Line Clipping Instructions", MB_OK | MB_ICONINFORMATION);
    }
}

// Start polygon clipping mode
void startPolygonClippingMode() {
    // Don't reset everything if window already defined
    if (!clipWindowDefined) {
        resetClippingStates();
    } else {
        // Keep the window but reset other state
        lineClippingMode = false;
        polygonClippingMode = true;
        drawingPolygon = false;
        cnt2 = 0;
        pointCount = 0;
        polygonDrawn = false;
    }
    
    polygonClippingMode = true;
    clippingMode = true;
    
    if (clipWindowDefined) {
        definingClipWindow = true;
        // Draw the window again to make it visible
        drawClippingWindow(GetActiveWindow());
        
        // Show instructions for polygon drawing with existing window
        MessageBoxW(NULL, 
                  L"Polygon Clipping:\n\n"
                  L"Using existing clipping window.\n"
                  L"1. Left-click to add polygon vertices\n"
                  L"2. Right-click to complete the polygon and apply clipping",
                  L"Polygon Clipping Instructions", MB_OK | MB_ICONINFORMATION);
    } else {
        // Display initial instructions
        MessageBoxW(NULL, 
                  L"Polygon Clipping:\n\n"
                  L"1. Right-click twice to define the clipping window\n"
                  L"2. Left-click to add polygon vertices\n"
                  L"3. Right-click to complete the polygon and apply clipping",
                  L"Polygon Clipping Instructions", MB_OK | MB_ICONINFORMATION);
    }
} 