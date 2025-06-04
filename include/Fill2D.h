#ifndef FILL2D_H
#define FILL2D_H

#include "Graphics.h"
#include <vector>
#include <queue>

namespace Graphics {

// Fill algorithm types
enum class FillAlgorithm {
    ScanLine,       // Scan line for general polygons
    ConvexFill,     // Optimized for convex polygons
    FloodFill,      // Recursive flood fill
    FloodFillQueue  // Queue-based flood fill
};

class Fill2D {
public:
    // Constructor
    Fill2D(HDC hdc);
    
    // Set color
    void setColor(COLORREF color) { m_color = color; }
    COLORREF getColor() const { return m_color; }
    
    // Check if a polygon is convex
    static bool isConvex(const std::vector<Point>& polygon);
    
    // Fill a polygon with the specified algorithm
    void fillPolygon(const std::vector<Point>& polygon, FillAlgorithm algorithm = FillAlgorithm::ScanLine);
    
    // Specific fill methods
    void fillScanLine(const std::vector<Point>& polygon);
    void fillConvexPolygon(const std::vector<Point>& polygon);
    void floodFill(int x, int y, COLORREF boundaryColor);
    void floodFillQueue(int x, int y, COLORREF boundaryColor);
    
private:
    HDC m_hdc;
    COLORREF m_color;
    
    // Helper structure for scan line fill
    struct Edge {
        int yMin;       // Minimum y-coordinate
        int yMax;       // Maximum y-coordinate
        double x;       // Current x-coordinate at yMin
        double slope;   // 1/m (inverse of slope)
        
        Edge(int y1, int y2, double x1, double x2)
            : yMin(std::min(y1, y2)), 
              yMax(std::max(y1, y2)) {
            if (y1 < y2) {
                x = x1;
                slope = (x2 - x1) / (y2 - y1);
            } else {
                x = x2;
                slope = (x1 - x2) / (y1 - y2);
            }
        }
        
        // Update x for the next scan line
        void update() {
            x += slope;
        }
    };
    
    // Helper methods for scanline fill
    void buildEdgeTable(const std::vector<Point>& polygon, std::vector<std::vector<Edge>>& edgeTable);
    void scanlineProcess(std::vector<std::vector<Edge>>& edgeTable, int yMin, int yMax);
    
    // Helper method for checking pixel color
    COLORREF getPixelColor(int x, int y);
};

} // namespace Graphics

#endif // FILL2D_H 