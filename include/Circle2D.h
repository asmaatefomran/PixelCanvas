#ifndef CIRCLE2D_H
#define CIRCLE2D_H

#include "Graphics.h"

namespace Graphics {

class Circle2D : public Shape {
public:
    // Constructors
    Circle2D(HDC hdc);
    Circle2D(HDC hdc, const Point& center, double radius);
    
    // Set/get properties
    void setCenter(const Point& center);
    void setRadius(double radius);
    
    Point getCenter() const { return m_center; }
    double getRadius() const { return m_radius; }
    
    // Set fill properties
    void setFilled(bool filled) { m_filled = filled; }
    void setFillQuarter(FillQuarter quarter) { m_fillQuarter = quarter; }
    
    bool isFilled() const { return m_filled; }
    FillQuarter getFillQuarter() const { return m_fillQuarter; }
    
    // Draw methods
    void draw() override;
    void draw(Algorithm algorithm);
    
    // Specialized drawing methods
    void drawMidpoint();
    void drawPolar();
    
    // Fill methods
    void fill();
    void fillQuarter(FillQuarter quarter);
    
private:
    Point m_center;
    double m_radius;
    bool m_filled;
    FillQuarter m_fillQuarter;
    Algorithm m_algorithm;
    
    // Helper methods for drawing circle points
    void drawCirclePoints(int cx, int cy, int x, int y, COLORREF color);
    void drawQuarterPoints(int cx, int cy, int x, int y, COLORREF color, FillQuarter quarter);
};

} // namespace Graphics

#endif // CIRCLE2D_H 