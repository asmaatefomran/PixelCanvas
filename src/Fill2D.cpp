#include "../include/Fill2D.h"
#include <algorithm>
#include <queue>

namespace Graphics {

Fill2D::Fill2D(HDC hdc)
    : m_hdc(hdc),
      m_color(RGB(0, 0, 0)) {
}

bool Fill2D::isConvex(const std::vector<Point>& polygon) {
    if (polygon.size() < 3) {
        return false; // Not a polygon
    }
    
    bool sign = false;
    bool signSet = false;
    
    for (size_t i = 0; i < polygon.size(); i++) {
        size_t j = (i + 1) % polygon.size();
        size_t k = (i + 2) % polygon.size();
        
        double dx1 = polygon[j].x - polygon[i].x;
        double dy1 = polygon[j].y - polygon[i].y;
        double dx2 = polygon[k].x - polygon[j].x;
        double dy2 = polygon[k].y - polygon[j].y;
        
        double crossProduct = dx1 * dy2 - dy1 * dx2;
        
        // Set the sign of the first non-zero cross product
        if (crossProduct != 0) {
            bool currentSign = (crossProduct > 0);
            if (!signSet) {
                sign = currentSign;
                signSet = true;
            } else if (sign != currentSign) {
                return false; // Sign change means non-convex
            }
        }
    }
    
    return true;
}

void Fill2D::fillPolygon(const std::vector<Point>& polygon, FillAlgorithm algorithm) {
    switch (algorithm) {
        case FillAlgorithm::ScanLine:
            fillScanLine(polygon);
            break;
        case FillAlgorithm::ConvexFill:
            fillConvexPolygon(polygon);
            break;
        default:
            // Default to scan line if algorithm not supported
            fillScanLine(polygon);
            break;
    }
}

void Fill2D::fillScanLine(const std::vector<Point>& polygon) {
    if (polygon.size() < 3) {
        return; // Not a polygon
    }
    
    // Find min and max y coordinates
    int yMin = static_cast<int>(polygon[0].y);
    int yMax = yMin;
    
    for (const auto& point : polygon) {
        yMin = std::min(yMin, static_cast<int>(point.y));
        yMax = std::max(yMax, static_cast<int>(point.y));
    }
    
    // Build edge table
    std::vector<std::vector<Edge>> edgeTable(yMax - yMin + 1);
    buildEdgeTable(polygon, edgeTable);
    
    // Process scanlines
    scanlineProcess(edgeTable, yMin, yMax);
}

void Fill2D::buildEdgeTable(const std::vector<Point>& polygon, std::vector<std::vector<Edge>>& edgeTable) {
    int yMin = static_cast<int>(polygon[0].y);
    
    for (size_t i = 0; i < polygon.size(); i++) {
        size_t j = (i + 1) % polygon.size();
        
        int y1 = static_cast<int>(polygon[i].y);
        int y2 = static_cast<int>(polygon[j].y);
        
        // Skip horizontal edges
        if (y1 == y2) {
            continue;
        }
        
        double x1 = polygon[i].x;
        double x2 = polygon[j].x;
        
        // Create and add the edge
        Edge edge(y1, y2, x1, x2);
        edgeTable[edge.yMin - yMin].push_back(edge);
    }
}

void Fill2D::scanlineProcess(std::vector<std::vector<Edge>>& edgeTable, int yMin, int yMax) {
    // Active edge list
    std::vector<Edge> activeEdges;
    
    // Process each scanline
    for (int y = yMin; y <= yMax; y++) {
        // Add edges starting at current scanline to AEL
        for (const auto& edge : edgeTable[y - yMin]) {
            activeEdges.push_back(edge);
        }
        
        // Remove edges that end at current scanline
        activeEdges.erase(
            std::remove_if(activeEdges.begin(), activeEdges.end(),
                [y](const Edge& edge) { return edge.yMax == y; }),
            activeEdges.end()
        );
        
        // Sort active edges by x-coordinate
        std::sort(activeEdges.begin(), activeEdges.end(),
            [](const Edge& a, const Edge& b) { return a.x < b.x; });
        
        // Fill between pairs of edges
        for (size_t i = 0; i < activeEdges.size(); i += 2) {
            if (i + 1 < activeEdges.size()) {
                int x1 = static_cast<int>(activeEdges[i].x);
                int x2 = static_cast<int>(activeEdges[i + 1].x);
                
                // Draw a horizontal line
                for (int x = x1; x <= x2; x++) {
                    SetPixel(m_hdc, x, y, m_color);
                }
            }
        }
        
        // Update x-coordinates for the next scanline
        for (auto& edge : activeEdges) {
            edge.update();
        }
    }
}

void Fill2D::fillConvexPolygon(const std::vector<Point>& polygon) {
    if (!isConvex(polygon)) {
        // Fallback to general polygon fill
        fillScanLine(polygon);
        return;
    }
    
    // For convex polygons, we can simplify the scanline algorithm
    fillScanLine(polygon);
}

void Fill2D::floodFill(int x, int y, COLORREF boundaryColor) {
    // Base case: out of bounds
    if (x < 0 || y < 0) {
        return;
    }
    
    // Get current pixel color
    COLORREF currentColor = getPixelColor(x, y);
    
    // If current pixel is the boundary color or already filled, return
    if (currentColor == boundaryColor || currentColor == m_color) {
        return;
    }
    
    // Fill current pixel
    SetPixel(m_hdc, x, y, m_color);
    
    // Recursive calls for 4-connected neighbors
    floodFill(x + 1, y, boundaryColor);
    floodFill(x - 1, y, boundaryColor);
    floodFill(x, y + 1, boundaryColor);
    floodFill(x, y - 1, boundaryColor);
}

void Fill2D::floodFillQueue(int x, int y, COLORREF boundaryColor) {
    // Queue-based approach to avoid stack overflow
    std::queue<std::pair<int, int>> pixelQueue;
    pixelQueue.push({x, y});
    
    while (!pixelQueue.empty()) {
        auto [currX, currY] = pixelQueue.front();
        pixelQueue.pop();
        
        // Skip out of bounds
        if (currX < 0 || currY < 0) {
            continue;
        }
        
        // Get current pixel color
        COLORREF currentColor = getPixelColor(currX, currY);
        
        // If current pixel is the boundary color or already filled, skip
        if (currentColor == boundaryColor || currentColor == m_color) {
            continue;
        }
        
        // Fill current pixel
        SetPixel(m_hdc, currX, currY, m_color);
        
        // Add 4-connected neighbors to queue
        pixelQueue.push({currX + 1, currY});
        pixelQueue.push({currX - 1, currY});
        pixelQueue.push({currX, currY + 1});
        pixelQueue.push({currX, currY - 1});
    }
}

COLORREF Fill2D::getPixelColor(int x, int y) {
    return GetPixel(m_hdc, x, y);
}

} // namespace Graphics 