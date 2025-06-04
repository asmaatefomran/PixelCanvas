#ifndef LINE2D_H
#define LINE2D_H

#include "Graphics.h"

namespace Graphics {

class Line2D : public Shape {
public:
    // Constructors
    Line2D(HDC hdc);
    Line2D(HDC hdc, const Point& start, const Point& end);
    
    // Set/get points
    void setPoints(const Point& start, const Point& end);
    void setStartPoint(const Point& start);
    void setEndPoint(const Point& end);
    
    Point getStartPoint() const { return m_start; }
    Point getEndPoint() const { return m_end; }
    
    // Draw with specified algorithm
    void draw() override;
    void draw(Algorithm algorithm);
    
    // Specialized drawing methods
    void drawDDA();
    void drawMidpoint();
    void drawParametric();
    
    // Draw with color interpolation
    void drawInterpolated(COLORREF startColor, COLORREF endColor);
    
    // Get line length
    double length() const;
    
private:
    Point m_start;
    Point m_end;
    Algorithm m_algorithm;
};

} // namespace Graphics

#endif // LINE2D_H 