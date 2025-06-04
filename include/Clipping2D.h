#ifndef CLIPPING2D_H
#define CLIPPING2D_H

#include "Graphics.h"
#include <vector>

namespace Graphics {

// Structure to store a clipping window
struct ClipWindow {
    double left, right, top, bottom;
    
    ClipWindow() : left(0), right(0), top(0), bottom(0) {}
    ClipWindow(double l, double t, double r, double b) : left(l), right(r), top(t), bottom(b) {}
    
    bool isValid() const { return (right > left) && (bottom > top); }
};

class Clipping2D {
public:
    // Constructor
    Clipping2D(HDC hdc);
    
    // Set clipping window
    void setClipWindow(const ClipWindow& window);
    void setClipWindow(double left, double top, double right, double bottom);
    
    // Get clipping window
    ClipWindow getClipWindow() const { return m_window; }
    
    // Line clipping using Cohen-Sutherland algorithm
    bool clipLine(Point& start, Point& end);
    
    // Polygon clipping using Sutherland-Hodgman algorithm
    std::vector<Point> clipPolygon(const std::vector<Point>& polygon);
    
    // Draw clipping window
    void drawClipWindow(COLORREF color = RGB(255, 0, 0));
    
private:
    HDC m_hdc;
    ClipWindow m_window;
    
    // Cohen-Sutherland helper functions
    enum OutCode {
        INSIDE = 0,
        LEFT = 1,
        RIGHT = 2,
        BOTTOM = 4,
        TOP = 8
    };
    
    int computeOutCode(double x, double y);
    
    // Sutherland-Hodgman helper functions
    std::vector<Point> clipAgainstEdge(const std::vector<Point>& inputPolygon, 
                                      int edgeCode);
    bool isInside(const Point& p, int edgeCode);
    Point computeIntersection(const Point& p1, const Point& p2, int edgeCode);
};

} // namespace Graphics

#endif // CLIPPING2D_H 