#include "../include/Clipping2D.h"
#include "../include/Line2D.h"

namespace Graphics {

Clipping2D::Clipping2D(HDC hdc) 
    : m_hdc(hdc), 
      m_window(0, 0, 0, 0) {
}

void Clipping2D::setClipWindow(const ClipWindow& window) {
    m_window = window;
}

void Clipping2D::setClipWindow(double left, double top, double right, double bottom) {
    m_window.left = left;
    m_window.top = top;
    m_window.right = right;
    m_window.bottom = bottom;
}

bool Clipping2D::clipLine(Point& start, Point& end) {
    if (!m_window.isValid()) {
        return false;
    }

    // Cohen-Sutherland line clipping algorithm
    double x0 = start.x;
    double y0 = start.y;
    double x1 = end.x;
    double y1 = end.y;
    
    int outcode0 = computeOutCode(x0, y0);
    int outcode1 = computeOutCode(x1, y1);
    bool accept = false;
    
    while (true) {
        if (!(outcode0 | outcode1)) {
            // Both endpoints inside the clip window
            accept = true;
            break;
        } else if (outcode0 & outcode1) {
            // Both endpoints outside the clip window and share an outside region
            break;
        } else {
            // At least one endpoint outside the clip window
            double x, y;
            
            // Select an outside point
            int outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;
            
            // Find intersection point
            if (outcodeOut & TOP) {
                x = x0 + (x1 - x0) * (m_window.top - y0) / (y1 - y0);
                y = m_window.top;
            } else if (outcodeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (m_window.bottom - y0) / (y1 - y0);
                y = m_window.bottom;
            } else if (outcodeOut & RIGHT) {
                y = y0 + (y1 - y0) * (m_window.right - x0) / (x1 - x0);
                x = m_window.right;
            } else if (outcodeOut & LEFT) {
                y = y0 + (y1 - y0) * (m_window.left - x0) / (x1 - x0);
                x = m_window.left;
            }
            
            // Replace the outside point with the intersection point
            if (outcodeOut == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = computeOutCode(x0, y0);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = computeOutCode(x1, y1);
            }
        }
    }
    
    if (accept) {
        // Update the points with the clipped coordinates
        start.x = x0;
        start.y = y0;
        end.x = x1;
        end.y = y1;
        return true;
    }
    
    return false;
}

std::vector<Point> Clipping2D::clipPolygon(const std::vector<Point>& polygon) {
    if (!m_window.isValid() || polygon.empty()) {
        return {};
    }
    
    // Sutherland-Hodgman polygon clipping algorithm
    std::vector<Point> outputPolygon = polygon;
    
    // Clip against each edge
    outputPolygon = clipAgainstEdge(outputPolygon, LEFT);
    outputPolygon = clipAgainstEdge(outputPolygon, TOP);
    outputPolygon = clipAgainstEdge(outputPolygon, RIGHT);
    outputPolygon = clipAgainstEdge(outputPolygon, BOTTOM);
    
    return outputPolygon;
}

void Clipping2D::drawClipWindow(COLORREF color) {
    if (!m_window.isValid()) {
        return;
    }
    
    // Draw the clip window borders
    Line2D line(m_hdc);
    line.setColor(color);
    
    // Top edge
    line.setPoints(Point(m_window.left, m_window.top), Point(m_window.right, m_window.top));
    line.draw();
    
    // Right edge
    line.setPoints(Point(m_window.right, m_window.top), Point(m_window.right, m_window.bottom));
    line.draw();
    
    // Bottom edge
    line.setPoints(Point(m_window.right, m_window.bottom), Point(m_window.left, m_window.bottom));
    line.draw();
    
    // Left edge
    line.setPoints(Point(m_window.left, m_window.bottom), Point(m_window.left, m_window.top));
    line.draw();
}

int Clipping2D::computeOutCode(double x, double y) {
    int code = INSIDE;
    
    if (x < m_window.left) {
        code |= LEFT;
    } else if (x > m_window.right) {
        code |= RIGHT;
    }
    
    if (y < m_window.top) {
        code |= TOP;
    } else if (y > m_window.bottom) {
        code |= BOTTOM;
    }
    
    return code;
}

std::vector<Point> Clipping2D::clipAgainstEdge(
    const std::vector<Point>& inputPolygon, int edgeCode) {
    
    std::vector<Point> outputPolygon;
    
    if (inputPolygon.empty()) {
        return outputPolygon;
    }
    
    // Start with the last point in the polygon
    Point s = inputPolygon.back();
    
    // Process all vertices
    for (const Point& e : inputPolygon) {
        if (isInside(e, edgeCode)) {
            // Case 1: Current point is inside the clip edge
            if (!isInside(s, edgeCode)) {
                // Case 1a: Moving from outside to inside
                outputPolygon.push_back(computeIntersection(s, e, edgeCode));
            }
            // Always add the current point
            outputPolygon.push_back(e);
        } else if (isInside(s, edgeCode)) {
            // Case 2: Current point is outside, but previous was inside
            // Add the intersection point
            outputPolygon.push_back(computeIntersection(s, e, edgeCode));
        }
        // Case 3: Both outside, add nothing
        
        // Current point becomes the previous for next iteration
        s = e;
    }
    
    return outputPolygon;
}

bool Clipping2D::isInside(const Point& p, int edgeCode) {
    switch (edgeCode) {
        case LEFT:
            return p.x >= m_window.left;
        case RIGHT:
            return p.x <= m_window.right;
        case TOP:
            return p.y >= m_window.top;
        case BOTTOM:
            return p.y <= m_window.bottom;
        default:
            return false;
    }
}

Point Clipping2D::computeIntersection(const Point& p1, const Point& p2, int edgeCode) {
    Point intersection;
    
    switch (edgeCode) {
        case LEFT:
            intersection.x = m_window.left;
            intersection.y = p1.y + (p2.y - p1.y) * (m_window.left - p1.x) / (p2.x - p1.x);
            break;
        case RIGHT:
            intersection.x = m_window.right;
            intersection.y = p1.y + (p2.y - p1.y) * (m_window.right - p1.x) / (p2.x - p1.x);
            break;
        case TOP:
            intersection.y = m_window.top;
            intersection.x = p1.x + (p2.x - p1.x) * (m_window.top - p1.y) / (p2.y - p1.y);
            break;
        case BOTTOM:
            intersection.y = m_window.bottom;
            intersection.x = p1.x + (p2.x - p1.x) * (m_window.bottom - p1.y) / (p2.y - p1.y);
            break;
    }
    
    return intersection;
}

} // namespace Graphics 