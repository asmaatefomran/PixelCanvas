#include "../include/Line2D.h"

namespace Graphics {

Line2D::Line2D(HDC hdc) 
    : Shape(hdc), 
      m_start(0, 0), 
      m_end(0, 0),
      m_algorithm(Algorithm::DDA) {
}

Line2D::Line2D(HDC hdc, const Point& start, const Point& end) 
    : Shape(hdc), 
      m_start(start), 
      m_end(end),
      m_algorithm(Algorithm::DDA) {
}

void Line2D::setPoints(const Point& start, const Point& end) {
    m_start = start;
    m_end = end;
}

void Line2D::setStartPoint(const Point& start) {
    m_start = start;
}

void Line2D::setEndPoint(const Point& end) {
    m_end = end;
}

void Line2D::draw() {
    // Use the current algorithm
    draw(m_algorithm);
}

void Line2D::draw(Algorithm algorithm) {
    m_algorithm = algorithm;
    
    switch (algorithm) {
        case Algorithm::DDA:
            drawDDA();
            break;
        case Algorithm::Midpoint:
            drawMidpoint();
            break;
        case Algorithm::Parametric:
            drawParametric();
            break;
        default:
            // Default to DDA if algorithm not supported
            drawDDA();
            break;
    }
}

void Line2D::drawDDA() {
    int x1 = static_cast<int>(m_start.x);
    int y1 = static_cast<int>(m_start.y);
    int x2 = static_cast<int>(m_end.x);
    int y2 = static_cast<int>(m_end.y);
    
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    
    if (steps == 0) {
        // Handle the case of a single point
        setPixel(x1, y1, m_color);
        return;
    }
    
    float xInc = dx / static_cast<float>(steps);
    float yInc = dy / static_cast<float>(steps);
    
    float x = static_cast<float>(x1);
    float y = static_cast<float>(y1);
    
    for (int i = 0; i <= steps; ++i) {
        setPixel(round(x), round(y), m_color);
        x += xInc;
        y += yInc;
    }
}

void Line2D::drawMidpoint() {
    int x1 = static_cast<int>(m_start.x);
    int y1 = static_cast<int>(m_start.y);
    int x2 = static_cast<int>(m_end.x);
    int y2 = static_cast<int>(m_end.y);
    
    // Determine if the line is steep (|m| > 1)
    bool steep = std::abs(y2 - y1) > std::abs(x2 - x1);
    
    // If steep, swap x and y
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    
    // Ensure x1 <= x2
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    
    int dx = x2 - x1;
    int dy = std::abs(y2 - y1);
    int d = 2 * dy - dx;
    int yInc = (y1 < y2) ? 1 : -1;
    int y = y1;
    
    for (int x = x1; x <= x2; ++x) {
        if (steep) {
            setPixel(y, x, m_color);  // Swap back for drawing
        } else {
            setPixel(x, y, m_color);
        }
        
        if (d > 0) {
            y += yInc;
            d -= 2 * dx;
        }
        d += 2 * dy;
    }
}

void Line2D::drawParametric() {
    int x1 = static_cast<int>(m_start.x);
    int y1 = static_cast<int>(m_start.y);
    int x2 = static_cast<int>(m_end.x);
    int y2 = static_cast<int>(m_end.y);
    
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(std::abs(dx), std::abs(dy));
    
    if (steps == 0) {
        // Handle the case of a single point
        setPixel(x1, y1, m_color);
        return;
    }
    
    for (int i = 0; i <= steps; ++i) {
        float t = static_cast<float>(i) / steps;
        int x = round(x1 + dx * t);
        int y = round(y1 + dy * t);
        setPixel(x, y, m_color);
    }
}

void Line2D::drawInterpolated(COLORREF startColor, COLORREF endColor) {
    int x1 = static_cast<int>(m_start.x);
    int y1 = static_cast<int>(m_start.y);
    int x2 = static_cast<int>(m_end.x);
    int y2 = static_cast<int>(m_end.y);
    
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    if (std::abs(dx) >= std::abs(dy)) {
        // Line is more horizontal
        if (x1 > x2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
            std::swap(startColor, endColor);
        }
        
        float m = static_cast<float>(dy) / dx;
        float y = static_cast<float>(y1);
        
        for (int x = x1; x <= x2; ++x) {
            float t = (x2 == x1) ? 0 : static_cast<float>(x - x1) / (x2 - x1);
            setPixel(x, round(y), blendColors(startColor, endColor, t));
            y += m;
        }
    } else {
        // Line is more vertical
        if (y1 > y2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
            std::swap(startColor, endColor);
        }
        
        float m = static_cast<float>(dx) / dy;
        float x = static_cast<float>(x1);
        
        for (int y = y1; y <= y2; ++y) {
            float t = (y2 == y1) ? 0 : static_cast<float>(y - y1) / (y2 - y1);
            setPixel(round(x), y, blendColors(startColor, endColor, t));
            x += m;
        }
    }
}

double Line2D::length() const {
    return m_start.distanceTo(m_end);
}

} // namespace Graphics 