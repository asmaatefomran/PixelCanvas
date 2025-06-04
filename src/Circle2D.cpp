#include "../include/Circle2D.h"
#include "../include/Line2D.h"
#include <cmath>

namespace Graphics {

Circle2D::Circle2D(HDC hdc)
    : Shape(hdc),
      m_center(0, 0),
      m_radius(0),
      m_filled(false),
      m_fillQuarter(FillQuarter::Full),
      m_algorithm(Algorithm::MidpointCircle) {
}

Circle2D::Circle2D(HDC hdc, const Point& center, double radius)
    : Shape(hdc),
      m_center(center),
      m_radius(radius),
      m_filled(false),
      m_fillQuarter(FillQuarter::Full),
      m_algorithm(Algorithm::MidpointCircle) {
}

void Circle2D::setCenter(const Point& center) {
    m_center = center;
}

void Circle2D::setRadius(double radius) {
    m_radius = radius;
}

void Circle2D::draw() {
    // Use the current algorithm
    draw(m_algorithm);
    
    // Fill if needed
    if (m_filled) {
        fill();
    }
}

void Circle2D::draw(Algorithm algorithm) {
    m_algorithm = algorithm;
    
    switch (algorithm) {
        case Algorithm::MidpointCircle:
            drawMidpoint();
            break;
        case Algorithm::PolarCircle:
            drawPolar();
            break;
        default:
            // Default to midpoint if algorithm not supported
            drawMidpoint();
            break;
    }
}

void Circle2D::drawMidpoint() {
    int cx = static_cast<int>(m_center.x);
    int cy = static_cast<int>(m_center.y);
    int radius = static_cast<int>(m_radius);
    
    if (radius <= 0) {
        // Draw a single pixel for a zero radius
        setPixel(cx, cy, m_color);
        return;
    }
    
    int x = 0;
    int y = radius;
    int d = 1 - radius;
    
    // Draw initial circle points
    drawCirclePoints(cx, cy, x, y, m_color);
    
    while (y > x) {
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        
        drawCirclePoints(cx, cy, x, y, m_color);
    }
}

void Circle2D::drawPolar() {
    int cx = static_cast<int>(m_center.x);
    int cy = static_cast<int>(m_center.y);
    int radius = static_cast<int>(m_radius);
    
    if (radius <= 0) {
        // Draw a single pixel for a zero radius
        setPixel(cx, cy, m_color);
        return;
    }
    
    // Use polar coordinates to draw the circle
    double angle;
    int x, y;
    
    // Calculate number of steps based on radius
    int steps = std::max(20, 3 * radius); // Ensure enough steps for smoothness
    
    for (int i = 0; i < steps; i++) {
        angle = 2.0 * M_PI * i / steps;
        x = cx + static_cast<int>(radius * cos(angle));
        y = cy + static_cast<int>(radius * sin(angle));
        
        setPixel(x, y, m_color);
    }
}

void Circle2D::fill() {
    // Use the current quarter setting
    fillQuarter(m_fillQuarter);
}

void Circle2D::fillQuarter(FillQuarter quarter) {
    int cx = static_cast<int>(m_center.x);
    int cy = static_cast<int>(m_center.y);
    int radius = static_cast<int>(m_radius);
    
    if (radius <= 0) {
        return;
    }
    
    m_fillQuarter = quarter;
    
    // Fill using horizontal scan lines
    for (int y = cy - radius; y <= cy + radius; y++) {
        for (int x = cx - radius; x <= cx + radius; x++) {
            int dx = x - cx;
            int dy = y - cy;
            
            if (dx*dx + dy*dy <= radius*radius) {
                // Check which quarter the point belongs to
                bool shouldDraw = false;
                
                switch (quarter) {
                    case FillQuarter::Full:
                        shouldDraw = true;
                        break;
                    case FillQuarter::TopRight:
                        shouldDraw = (dx >= 0 && dy <= 0);
                        break;
                    case FillQuarter::TopLeft:
                        shouldDraw = (dx <= 0 && dy <= 0);
                        break;
                    case FillQuarter::BottomLeft:
                        shouldDraw = (dx <= 0 && dy >= 0);
                        break;
                    case FillQuarter::BottomRight:
                        shouldDraw = (dx >= 0 && dy >= 0);
                        break;
                }
                
                if (shouldDraw) {
                    setPixel(x, y, m_color);
                }
            }
        }
    }
}

void Circle2D::drawCirclePoints(int cx, int cy, int x, int y, COLORREF color) {
    // Draw all 8 octants of the circle
    setPixel(cx + x, cy + y, color);
    setPixel(cx - x, cy + y, color);
    setPixel(cx + x, cy - y, color);
    setPixel(cx - x, cy - y, color);
    setPixel(cx + y, cy + x, color);
    setPixel(cx - y, cy + x, color);
    setPixel(cx + y, cy - x, color);
    setPixel(cx - y, cy - x, color);
}

void Circle2D::drawQuarterPoints(int cx, int cy, int x, int y, COLORREF color, FillQuarter quarter) {
    switch (quarter) {
        case FillQuarter::Full:
            drawCirclePoints(cx, cy, x, y, color);
            break;
        case FillQuarter::TopRight:
            setPixel(cx + x, cy - y, color);
            setPixel(cx + y, cy - x, color);
            break;
        case FillQuarter::TopLeft:
            setPixel(cx - x, cy - y, color);
            setPixel(cx - y, cy - x, color);
            break;
        case FillQuarter::BottomLeft:
            setPixel(cx - x, cy + y, color);
            setPixel(cx - y, cy + x, color);
            break;
        case FillQuarter::BottomRight:
            setPixel(cx + x, cy + y, color);
            setPixel(cx + y, cy + x, color);
            break;
    }
}

} // namespace Graphics 