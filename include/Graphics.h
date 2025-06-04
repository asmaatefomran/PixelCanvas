#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>

namespace Graphics {

// Common 2D point structure
struct Point {
    double x, y;
    
    Point() : x(0), y(0) {}
    Point(double x, double y) : x(x), y(y) {}
    
    // Utility methods
    double distanceTo(const Point& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
    }
};

// Base class for all drawable shapes
class Shape {
public:
    Shape(HDC hdc) : m_hdc(hdc), m_color(RGB(0, 0, 0)), m_thickness(1) {}
    virtual ~Shape() = default;
    
    // Set/get properties
    void setColor(COLORREF color) { m_color = color; }
    COLORREF getColor() const { return m_color; }
    
    void setThickness(int thickness) { m_thickness = thickness; }
    int getThickness() const { return m_thickness; }
    
    // Virtual draw method to be implemented by derived classes
    virtual void draw() = 0;
    
protected:
    HDC m_hdc;
    COLORREF m_color;
    int m_thickness;
    
    // Utility functions for drawing
    void setPixel(int x, int y, COLORREF color) {
        SetPixel(m_hdc, x, y, color);
    }
    
    static int round(double value) {
        return static_cast<int>(value + 0.5);
    }
    
    // Color blending for gradients and interpolation
    static COLORREF blendColors(COLORREF c1, COLORREF c2, float t) {
        int r = GetRValue(c1) + t * (GetRValue(c2) - GetRValue(c1));
        int g = GetGValue(c1) + t * (GetGValue(c2) - GetGValue(c1));
        int b = GetBValue(c1) + t * (GetBValue(c2) - GetBValue(c1));
        return RGB(r, g, b);
    }
};

// Common fill styles
enum class FillStyle {
    None,
    Solid,
    Pattern
};

// Common algorithm types
enum class Algorithm {
    // Line algorithms
    DDA,
    Midpoint,
    Parametric,
    
    // Circle/Ellipse algorithms
    MidpointCircle,
    PolarCircle,
    DirectEllipse,
    
    // Curve algorithms
    Bezier,
    Hermite,
    Cardinal,
    
    // Clipping algorithms
    CohenSutherland,
    SutherlandHodgman
};

// Common fill region for circle quarters
enum class FillQuarter {
    Full,
    TopRight,
    TopLeft,
    BottomLeft,
    BottomRight
};

} // namespace Graphics

#endif // GRAPHICS_H 