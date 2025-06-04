#ifndef CURVE2D_H
#define CURVE2D_H

#include "Graphics.h"
#include <vector>

namespace Graphics {

// Derivative structure for Hermite curves
struct Derivative {
    double dx, dy;
    
    Derivative() : dx(0), dy(0) {}
    Derivative(double dx, double dy) : dx(dx), dy(dy) {}
};

class Curve2D : public Shape {
public:
    // Constructors
    Curve2D(HDC hdc);
    
    // Set control points
    void setControlPoints(const std::vector<Point>& points);
    
    // Add a control point
    void addControlPoint(const Point& point);
    
    // Clear control points
    void clearControlPoints();
    
    // Get control points
    const std::vector<Point>& getControlPoints() const { return m_controlPoints; }
    
    // Set tension for Cardinal spline (0.0 to 1.0, default 0.5)
    void setTension(double tension) { m_tension = std::max(0.0, std::min(1.0, tension)); }
    double getTension() const { return m_tension; }
    
    // Set derivatives for Hermite curve
    void setDerivatives(const std::vector<Derivative>& derivatives);
    
    // Draw with specified algorithm
    void draw() override;
    void draw(Algorithm algorithm);
    
    // Specialized drawing methods
    void drawBezier();
    void drawHermite();
    void drawCardinal();
    
private:
    std::vector<Point> m_controlPoints;
    std::vector<Derivative> m_derivatives;
    double m_tension;
    Algorithm m_algorithm;
    
    // Helper functions
    Point evaluateBezier(const Point& p1, const Point& p2, const Point& p3, const Point& p4, double t);
    Point evaluateHermite(const Point& p1, const Point& p2, const Derivative& d1, const Derivative& d2, double t);
    Point evaluateCardinal(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t);
};

} // namespace Graphics

#endif // CURVE2D_H 