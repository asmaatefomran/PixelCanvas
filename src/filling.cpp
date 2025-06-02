#include <windows.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <climits>
#include "../include/filling.h"

// Implementation of the custom filling methods

void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps;

    if (abs(dx) > abs(dy))
        steps = abs(dx);
    else
        steps = abs(dy);

    double x_increment = (double)dx / steps,
        y_increment = (double)dy / steps,
        x = x1,
        y = y1;

    SetPixel(hdc, Round(x), Round(y), c);

    for (int i = 0; i < steps; i++) {
        x += x_increment;
        y += y_increment;
        SetPixel(hdc, Round(x), Round(y), c);
    }
}


struct EdgeNode {
    double x;
    double dx;
    int ymax;
    EdgeNode* next;

    EdgeNode(double x = 0, double dx = 0, int ymax = 0)
        : x(x), dx(dx), ymax(ymax), next(nullptr) {}
};

EdgeNode* edgeTableArray[800];

void initEdgeTable(EdgeNode* tbl[]) {
    for (int i = 0; i < 800; i++) {
        tbl[i] = nullptr;
    }
}

void insertEdgeToTable(EdgeNode* tbl[], int ymin, double x, double dx, int ymax) {
    EdgeNode* newNode = new EdgeNode(x, dx, ymax);

    if (tbl[ymin] == nullptr || tbl[ymin]->x > x) {
        newNode->next = tbl[ymin];
        tbl[ymin] = newNode;
    }
    else {
        EdgeNode* current = tbl[ymin];
        while (current->next != nullptr && current->next->x < x) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

void processEdgeToTable(EdgeNode* tbl[], point v1, point v2) {
    if (v1.y == v2.y)
        return;

    if (v1.y > v2.y)
        std::swap(v1, v2);

    int ymin = (int)ceil(v1.y);
    int ymax = (int)floor(v2.y);

    if (ymin >= ymax)
        return;

    double dx = (v2.x - v1.x) / (v2.y - v1.y);

    double x = v1.x + dx * (ymin - v1.y);

    insertEdgeToTable(tbl, ymin, x, dx, ymax);
}

void buildPolygonEdgeTable(EdgeNode* tbl[], point p[], int n) {
    point v1 = p[n - 1];
    for (int i = 0; i < n; i++) {
        point v2 = p[i];
        processEdgeToTable(tbl, v1, v2);
        v1 = p[i];
    }
}

void addEdgeToActiveList(EdgeNode*& aet, EdgeNode* edge) {
    EdgeNode* newNode = new EdgeNode(edge->x, edge->dx, edge->ymax);

    if (aet == nullptr || aet->x > newNode->x) {
        newNode->next = aet;
        aet = newNode;
    }
    else {
        EdgeNode* current = aet;
        while (current->next != nullptr && current->next->x < newNode->x) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

void updateAndRemoveActiveEdges(EdgeNode*& aet, int scanline) {
    EdgeNode* current = aet;
    EdgeNode* prev = nullptr;

    while (current != nullptr) {
        if (current->ymax <= scanline) {
            if (prev == nullptr) {
                aet = current->next;
                delete current;
                current = aet;
            }
            else {
                prev->next = current->next;
                delete current;
                current = prev->next;
            }
        }
        else {
            current->x += current->dx;
            prev = current;
            current = current->next;
        }
    }
}

void sortActiveEdgesByX(EdgeNode*& aet) {
    if (aet == nullptr || aet->next == nullptr)
        return;

    bool swapped;
    do {
        swapped = false;
        EdgeNode* current = aet;
        EdgeNode* prev = nullptr;

        while (current->next != nullptr) {
            if (current->x > current->next->x) {
                EdgeNode* next = current->next;
                current->next = next->next;
                next->next = current;

                if (prev == nullptr) {
                    aet = next;
                }
                else {
                    prev->next = next;
                }
                prev = next;
                swapped = true;
            }
            else {
                prev = current;
                current = current->next;
            }
        }
    } while (swapped);
}

void drawScanlinePairs(HDC hdc, EdgeNode* aet, int y, COLORREF c) {
    EdgeNode* current = aet;

    while (current != nullptr && current->next != nullptr) {
        int x1 = (int)ceil(current->x);
        int x2 = (int)floor(current->next->x);

        if (x1 <= x2) {
            DrawLineDDA(hdc, x1, y, x2, y, c);
        }

        current = current->next->next;
    }
}

void renderPolygonFromTable(HDC hdc, EdgeNode* tbl[], COLORREF c) {
    EdgeNode* aet = nullptr;

    for (int y = 0; y < 800; y++) {
        EdgeNode* current = tbl[y];
        while (current != nullptr) {
            addEdgeToActiveList(aet, current);
            current = current->next;
        }

        if (aet != nullptr) {
            drawScanlinePairs(hdc, aet, y, c);
        }

        updateAndRemoveActiveEdges(aet, y + 1);
        sortActiveEdgesByX(aet);
    }
}

void cleanupEdgeTable(EdgeNode* tbl[]) {
    for (int i = 0; i < 800; i++) {
        EdgeNode* current = tbl[i];
        while (current != nullptr) {
            EdgeNode* temp = current;
            current = current->next;
            delete temp;
        }
        tbl[i] = nullptr;
    }
}

void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c) {
    // Check for valid polygon
    if (n < 3) return;

    try {
        // Find the min and max y-values to determine scan-line range
        int minY = INT_MAX;
        int maxY = INT_MIN;
        for (int i = 0; i < n; i++) {
            if ((int)p[i].y < minY) minY = (int)p[i].y;
            if ((int)p[i].y > maxY) maxY = (int)p[i].y;
        }

        // Strict bounds checking to prevent array out-of-bounds
        minY = std::max(0, minY);
        maxY = std::min(799, maxY);  // Safe upper limit below the 800 array size

        // For each scan line
        for (int y = minY; y <= maxY; y++) {
            // Find all intersections with this scan line
            std::vector<int> intersections;
            intersections.reserve(n); // Reserve space to avoid reallocations
            
            for (int i = 0; i < n; i++) {
                // Get the current edge (p[i] to p[i+1])
                int next = (i + 1) % n;
                double y1 = p[i].y;
                double y2 = p[next].y;
                
                // Skip horizontal edges
                if (fabs(y1 - y2) < 0.001) continue;

                // Check if the scan line intersects this edge
                if ((y >= y1 && y < y2) || (y >= y2 && y < y1)) {
                    double x1 = p[i].x;
                    double x2 = p[next].x;
                    
                    // Calculate x-coordinate of intersection
                    // Using the line equation: x = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
                    double x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                    
                    // Bound check x coordinate to prevent extreme values
                    if (x > -10000 && x < 10000) {
                        intersections.push_back((int)x);
                    }
                }
            }

            // Sort intersections by x-coordinate
            if (!intersections.empty()) {
                std::sort(intersections.begin(), intersections.end());

                // Fill between pairs of intersections
                for (size_t i = 0; i < intersections.size() - 1; i += 2) {
                    if (i + 1 < intersections.size()) {
                        int left = std::max(0, intersections[i]);
                        int right = std::min(5000, intersections[i + 1]);
                        
                        if (left < right) {
                            // Draw the horizontal span
                            DrawLineDDA(hdc, left, y, right, y, c);
                        }
                    }
                }
            }
        }
    }
    catch (...) {
        // Handle any exceptions that might occur during filling
        // Just silently catch them to prevent crashes
    }
}

typedef struct {
    int left, right;
} EdgeTableEntry;

EdgeTableEntry convexEdgeTableArray[800];

void initConvexTable(EdgeTableEntry tbl[]) {
    for (int i = 0; i < 800; i++) {
        tbl[i].left = INT_MAX;
        tbl[i].right = INT_MIN;
    }
}

void edge2table(EdgeTableEntry tbl[], point v1, point v2){
    if (fabs(v1.y - v2.y) < 0.001)
        return;
    
    if (v1.y > v2.y)
        std::swap(v1, v2);
    
    double x = v1.x;
    double slope = (v2.x - v1.x) / (v2.y - v1.y);
    
    int startY = (int)ceil(v1.y);
    int endY = (int)floor(v2.y);
    
    for (int y = startY; y <= endY && y < 800; y++) {
        if (y >= 0) {
            int X = (int)ceil(x);
            if (X < tbl[y].left)
                tbl[y].left = X;
            if (X > tbl[y].right)
                tbl[y].right = X;
        }
        x += slope;
    }
}

void polygon2table(EdgeTableEntry tbl[], point p[], int n) {
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        edge2table(tbl, p[i], p[next]);
    }
}

void table2screen(HDC hdc, EdgeTableEntry tbl[], COLORREF c) {
    for (int y = 0; y < 800; y++) {
        if (tbl[y].left <= tbl[y].right) {
            DrawLineDDA(hdc, tbl[y].left, y, tbl[y].right, y, c);
        }
    }
}

void convexfill(HDC hdc, point p[], int n, COLORREF c) {
    EdgeTableEntry tbl[800];
    initConvexTable(tbl);
    polygon2table(tbl, p, n);
    table2screen(hdc, tbl, c);
}

// Global counter for recursive calls to prevent stack overflow
int g_recursionDepth = 0;
const int MAX_RECURSION_DEPTH = 1000; // Limit to prevent stack overflow

// Flood fill implementation (recursive)
void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    // Initialize counter on first call
    if (g_recursionDepth == 0) {
        g_recursionDepth = 0;
    }
    
    // Prevent stack overflow with recursion limit
    if (g_recursionDepth >= MAX_RECURSION_DEPTH) {
        // Show a message only on the first overflow detection
        if (g_recursionDepth == MAX_RECURSION_DEPTH) {
            MessageBoxW(NULL, L"Recursive fill depth limit reached. Switching to queue-based fill.", 
                      L"Fill Limit", MB_OK | MB_ICONINFORMATION);
            
            // Reset counter for next time
            g_recursionDepth = 0;
            
            // Use the queue-based fill as fallback
            myFloodFillQueue(hdc, x, y, bc, fc);
        }
        return;
    }
    
    // Bounds checking
    if (x < 0 || y < 0 || x >= 2000 || y >= 2000)
        return;
    
    // Safely get the pixel color
    COLORREF current;
    try {
        current = GetPixel(hdc, x, y);
    }
    catch (...) {
        g_recursionDepth = 0; // Reset counter on error
        return;
    }
    
    // Stop if we hit the boundary color or already filled color
    if (current == bc || current == fc) {
        return;
    }
    
    try {
        // Fill current pixel
        SetPixel(hdc, x, y, fc);
        
        // Increment recursion depth
        g_recursionDepth++;
        
        // Recursively fill the 4-connected neighbors
        myFloodFill(hdc, x + 1, y, bc, fc);
        myFloodFill(hdc, x - 1, y, bc, fc);
        myFloodFill(hdc, x, y + 1, bc, fc);
        myFloodFill(hdc, x, y - 1, bc, fc);
        
        // Decrement recursion depth
        g_recursionDepth--;
    }
    catch (...) {
        // Reset counter and switch to queue-based on exception
        g_recursionDepth = 0;
        myFloodFillQueue(hdc, x, y, bc, fc);
    }
    
    // Reset counter when the initial call completes
    if (g_recursionDepth == 0) {
        g_recursionDepth = 0;
    }
}

// Simple implementation for myFloodFillqueue
void myFloodFillqueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    // Just redirect to the proper implementation
    myFloodFillQueue(hdc, x, y, bc, fc);
}

bool IsPolygonConvex(const std::vector<point>& points) {
    if (points.size() < 3) return false;
    
    // A convex polygon has all interior angles less than 180 degrees
    // We can check this by verifying that all cross products have the same sign
    
    bool hasPositive = false;
    bool hasNegative = false;
    
    int n = (int)points.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;
        
        // Calculate vectors for consecutive edges
        double dx1 = points[j].x - points[i].x;
        double dy1 = points[j].y - points[i].y;
        double dx2 = points[k].x - points[j].x;
        double dy2 = points[k].y - points[j].y;
        
        // Calculate cross product (z-component in 2D space)
        double crossProduct = dx1 * dy2 - dy1 * dx2;
        
        // Check sign
        if (crossProduct > 0) hasPositive = true;
        if (crossProduct < 0) hasNegative = true;
        
        // If we have both positive and negative cross products, the polygon is not convex
        if (hasPositive && hasNegative) return false;
    }
    
    return true;
}
