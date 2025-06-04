#include "../include/Curve2D.h"
#include "../include/Line2D.h"
#include <cmath>

namespace Graphics {

Curve2D::Curve2D(HDC hdc)
    : Shape(hdc),
      m_tension(0.5),
      m_algorithm(Algorithm::Bezier) {
}

void Curve2D::setControlPoints(const std::vector<Point>& points) {
    m_controlPoints = points;
}

void Curve2D::addControlPoint(const Point& point) {
    m_controlPoints.push_back(point);
}

void Curve2D::clearControlPoints() {
    m_controlPoints.clear();
    m_derivatives.clear();
}

void Curve2D::setDerivatives(const std::vector<Derivative>& derivatives) {
    m_derivatives = derivatives;
}

void Curve2D::draw() {
    // Use the current algorithm
    draw(m_algorithm);
}

void Curve2D::draw(Algorithm algorithm) {
    m_algorithm = algorithm;
    
    switch (algorithm) {
        case Algorithm::Bezier:
            drawBezier();
            break;
        case Algorithm::Hermite:
            drawHermite();
            break;
        case Algorithm::Cardinal:
            drawCardinal();
            break;
        default:
            // Default to Bezier if algorithm not supported
            drawBezier();
            break;
    }
}

void Curve2D::drawBezier() {
    // Ensure we have enough control points (must be multiple of 3 plus 1)
    if (m_controlPoints.size() < 4) {
        return;
    }
    
    // Draw control points (optional)
    /*
    for (const auto& point : m_controlPoints) {
        SetPixel(m_hdc, static_cast<int>(point.x), static_cast<int>(point.y), RGB(255, 0, 0));
    }
    */
    
    // Draw each segment
    for (size_t i = 0; i + 3 < m_controlPoints.size(); i += 3) {
        const Point& p1 = m_controlPoints[i];
        const Point& p2 = m_controlPoints[i + 1];
        const Point& p3 = m_controlPoints[i + 2];
        const Point& p4 = m_controlPoints[i + 3];
        
        // Number of points to evaluate
        const int numPoints = 100;
        
        Point prevPoint = p1;
        
        for (int j = 1; j <= numPoints; j++) {
            double t = static_cast<double>(j) / numPoints;
            Point currPoint = evaluateBezier(p1, p2, p3, p4, t);
            
            // Draw line segment
            Line2D line(m_hdc);
            line.setColor(m_color);
            line.setPoints(prevPoint, currPoint);
            line.draw(Algorithm::DDA);
            
            prevPoint = currPoint;
        }
    }
}

void Curve2D::drawHermite() {
    // Ensure we have enough control points and derivatives
    if (m_controlPoints.size() < 2 || m_derivatives.size() < 2) {
        return;
    }
    
    // Draw each segment
    for (size_t i = 0; i + 1 < m_controlPoints.size() && i + 1 < m_derivatives.size(); i += 1) {
        const Point& p1 = m_controlPoints[i];
        const Point& p2 = m_controlPoints[i + 1];
        const Derivative& d1 = m_derivatives[i];
        const Derivative& d2 = m_derivatives[i + 1];
        
        // Number of points to evaluate
        const int numPoints = 100;
        
        Point prevPoint = p1;
        
        for (int j = 1; j <= numPoints; j++) {
            double t = static_cast<double>(j) / numPoints;
            Point currPoint = evaluateHermite(p1, p2, d1, d2, t);
            
            // Draw line segment
            Line2D line(m_hdc);
            line.setColor(m_color);
            line.setPoints(prevPoint, currPoint);
            line.draw(Algorithm::DDA);
            
            prevPoint = currPoint;
        }
    }
}

void Curve2D::drawCardinal() {
    // Cardinal spline requires at least 4 control points
    if (m_controlPoints.size() < 4) {
        return;
    }
    
    // Draw each segment
    for (size_t i = 0; i + 3 < m_controlPoints.size(); i++) {
        const Point& p0 = m_controlPoints[i];
        const Point& p1 = m_controlPoints[i + 1];
        const Point& p2 = m_controlPoints[i + 2];
        const Point& p3 = m_controlPoints[i + 3];
        
        // Number of points to evaluate
        const int numPoints = 100;
        
        Point prevPoint = p1;
        
        for (int j = 1; j <= numPoints; j++) {
            double t = static_cast<double>(j) / numPoints;
            Point currPoint = evaluateCardinal(p0, p1, p2, p3, t);
            
            // Draw line segment
            Line2D line(m_hdc);
            line.setColor(m_color);
            line.setPoints(prevPoint, currPoint);
            line.draw(Algorithm::DDA);
            
            prevPoint = currPoint;
        }
    }
}

Point Curve2D::evaluateBezier(const Point& p1, const Point& p2, const Point& p3, const Point& p4, double t) {
    // Cubic Bezier formula:
    // B(t) = (1-t)^3 * P1 + 3(1-t)^2 * t * P2 + 3(1-t) * t^2 * P3 + t^3 * P4
    double oneMinusT = 1.0 - t;
    double oneMinusTCubed = oneMinusT * oneMinusT * oneMinusT;
    double oneMinusTSquared = oneMinusT * oneMinusT;
    double tSquared = t * t;
    double tCubed = t * t * t;
    
    Point result;
    result.x = oneMinusTCubed * p1.x + 3 * oneMinusTSquared * t * p2.x + 
               3 * oneMinusT * tSquared * p3.x + tCubed * p4.x;
    result.y = oneMinusTCubed * p1.y + 3 * oneMinusTSquared * t * p2.y + 
               3 * oneMinusT * tSquared * p3.y + tCubed * p4.y;
    
    return result;
}

Point Curve2D::evaluateHermite(const Point& p1, const Point& p2, const Derivative& d1, const Derivative& d2, double t) {
    // Hermite formula:
    // H(t) = (2t^3 - 3t^2 + 1) * P1 + (t^3 - 2t^2 + t) * D1 + (-2t^3 + 3t^2) * P2 + (t^3 - t^2) * D2
    double tSquared = t * t;
    double tCubed = tSquared * t;
    
    double h1 = 2 * tCubed - 3 * tSquared + 1;    // Hermite basis function 1
    double h2 = tCubed - 2 * tSquared + t;        // Hermite basis function 2
    double h3 = -2 * tCubed + 3 * tSquared;       // Hermite basis function 3
    double h4 = tCubed - tSquared;                // Hermite basis function 4
    
    Point result;
    result.x = h1 * p1.x + h2 * d1.dx + h3 * p2.x + h4 * d2.dx;
    result.y = h1 * p1.y + h2 * d1.dy + h3 * p2.y + h4 * d2.dy;
    
    return result;
}

Point Curve2D::evaluateCardinal(const Point& p0, const Point& p1, const Point& p2, const Point& p3, double t) {
    // Cardinal spline formula (using Hermite basis)
    // Uses tension parameter to control curve tightness
    // s = (1-tension)/2
    double s = (1.0 - m_tension) / 2.0;
    
    // Convert to Hermite form
    Derivative d1, d2;
    d1.dx = s * (p2.x - p0.x);
    d1.dy = s * (p2.y - p0.y);
    d2.dx = s * (p3.x - p1.x);
    d2.dy = s * (p3.y - p1.y);
    
    return evaluateHermite(p1, p2, d1, d2, t);
}

} // namespace Graphics 