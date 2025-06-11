#include "PolygonFill.h"
#include <cmath>
#include <vector>

// Externally defined in PixelCanvas.cpp
extern bool clipWindowSet;
extern int clipMinX, clipMaxX, clipMinY, clipMaxY;

int Round(double x) { return (int)(x + 0.5); }

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
    int xi = Round(x);
    int yi = Round(y);
    if (!clipWindowSet || (xi >= clipMinX && xi <= clipMaxX && yi >= clipMinY && yi <= clipMaxY))
        SetPixel(hdc, xi, yi, c);
    for (int i = 0; i < steps; i++) {
        x += x_increment;
        y += y_increment;
        xi = Round(x);
        yi = Round(y);
        if (!clipWindowSet || (xi >= clipMinX && xi <= clipMaxX && yi >= clipMinY && yi <= clipMaxY))
            SetPixel(hdc, xi, yi, c);
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

EdgeNode* edgeTable[800];

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
            DrawLineDDA(hdc, x1, (int)y, x2, (int)y, c);
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

// Sutherland-Hodgman polygon clipping for rectangle
std::vector<point> ClipPolygonToRect(const point* poly, int n) {
    std::vector<point> input(poly, poly + n);
    std::vector<point> output;
    if (!clipWindowSet) return input;
    // Clip against each edge: left, right, bottom, top
    struct Edge { int x1, y1, x2, y2; };
    Edge edges[4] = {
        {clipMinX, clipMinY, clipMinX, clipMaxY}, // Left
        {clipMaxX, clipMinY, clipMaxX, clipMaxY}, // Right
        {clipMinX, clipMinY, clipMaxX, clipMinY}, // Bottom
        {clipMinX, clipMaxY, clipMaxX, clipMaxY}  // Top
    };
    auto inside = [](const point& p, int edge) {
        switch (edge) {
            case 0: return p.x >= clipMinX;
            case 1: return p.x <= clipMaxX;
            case 2: return p.y >= clipMinY;
            case 3: return p.y <= clipMaxY;
        }
        return false;
    };
    auto intersect = [](const point& p1, const point& p2, int edge) -> point {
        double x = 0, y = 0;
        double dx = p2.x - p1.x, dy = p2.y - p1.y;
        switch (edge) {
            case 0: // Left
                x = clipMinX;
                y = p1.y + dy * (clipMinX - p1.x) / dx;
                break;
            case 1: // Right
                x = clipMaxX;
                y = p1.y + dy * (clipMaxX - p1.x) / dx;
                break;
            case 2: // Bottom
                y = clipMinY;
                x = p1.x + dx * (clipMinY - p1.y) / dy;
                break;
            case 3: // Top
                y = clipMaxY;
                x = p1.x + dx * (clipMaxY - p1.y) / dy;
                break;
        }
        return point(x, y);
    };
    for (int e = 0; e < 4; ++e) {
        output.clear();
        for (size_t i = 0; i < input.size(); ++i) {
            point curr = input[i];
            point prev = input[(i + input.size() - 1) % input.size()];
            bool currIn = inside(curr, e);
            bool prevIn = inside(prev, e);
            if (currIn) {
                if (!prevIn) output.push_back(intersect(prev, curr, e));
                output.push_back(curr);
            } else if (prevIn) {
                output.push_back(intersect(prev, curr, e));
            }
        }
        input = output;
    }
    return input;
}

void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c) {
    std::vector<point> clipped = ClipPolygonToRect(p, n);
    if (clipped.size() < 3) return;
    EdgeNode* tbl[800];
    initEdgeTable(tbl);
    buildPolygonEdgeTable(tbl, clipped.data(), (int)clipped.size());
    renderPolygonFromTable(hdc, tbl, c);
    cleanupEdgeTable(tbl);
}

struct EdgeTableEntry {
    int left, right;
};

void init(EdgeTableEntry tbl[800]) {
    for (int i = 0;i < 800;i++) {
        tbl[i].left = INT_MAX;
        tbl[i].right = INT_MIN;
    }
}

void edge2table(EdgeTableEntry tbl[800], point v1, point v2){
    if (v1.y == v2.y)
        return;
    if (v1.y > v2.y)
        std::swap(v1, v2);
    int y = (int)v1.y;
    double x = v1.x;
    double mi = (v2.x - v1.x) / (v2.y - v1.y);
    while (y < v2.y) {
        if (x < tbl[y].left)
            tbl[y].left = (int)ceil(x);
        if (x > tbl[y].right)
            tbl[y].right = (int)floor(x);
        y++;
        x += mi;
    }
}

void polygon2table(EdgeTableEntry tbl[800], point p[], int n) {
    point v1 = p[n - 1];
    for (int i = 0;i < n;i++) {
        point v2 = p[i];
        edge2table(tbl, v1, v2);
        v1 = p[i];
    }
}

void table2screen(HDC hdc, EdgeTableEntry tbl[800], COLORREF c) {
    for (int i = 0;i < 800;i++) {
        if (tbl[i].left < tbl[i].right)
            DrawLineDDA(hdc, tbl[i].left, (int)i, tbl[i].right, (int)i, c);
    }
}

void convexfill(HDC hdc, point p[], int n, COLORREF c) {
    std::vector<point> clipped = ClipPolygonToRect(p, n);
    if (clipped.size() < 3) return;
    EdgeTableEntry tbl[800];
    init(tbl);
    polygon2table(tbl, clipped.data(), (int)clipped.size());
    table2screen(hdc, tbl, c);
}

void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    COLORREF c = GetPixel(hdc, x, y);
    if (c == bc || c == fc)
        return;
    if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
        SetPixel(hdc, x, y, fc);
    myFloodFill(hdc, x + 1, y, bc, fc);
    myFloodFill(hdc, x - 1, y, bc, fc);
    myFloodFill(hdc, x, y + 1, bc, fc);
    myFloodFill(hdc, x, y - 1, bc, fc);
}

void myFloodFillqueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    std::queue<point> q;
    q.push(point(x, y));
    if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
        SetPixel(hdc, x, y, fc); // Mark as filled immediately
    while (!q.empty()) {
        point p = q.front();
        q.pop();
        // Right
        COLORREF c = GetPixel(hdc, p.x + 1, p.y);
        if (c != bc && c != fc) {
            if (!clipWindowSet || (p.x + 1 >= clipMinX && p.x + 1 <= clipMaxX && p.y >= clipMinY && p.y <= clipMaxY))
                SetPixel(hdc, p.x + 1, p.y, fc);
            q.push(point(p.x + 1, p.y));
        }
        // Left
        c = GetPixel(hdc, p.x - 1, p.y);
        if (c != bc && c != fc) {
            if (!clipWindowSet || (p.x - 1 >= clipMinX && p.x - 1 <= clipMaxX && p.y >= clipMinY && p.y <= clipMaxY))
                SetPixel(hdc, p.x - 1, p.y, fc);
            q.push(point(p.x - 1, p.y));
        }
        // Down
        c = GetPixel(hdc, p.x, p.y + 1);
        if (c != bc && c != fc) {
            if (!clipWindowSet || (p.x >= clipMinX && p.x <= clipMaxX && p.y + 1 >= clipMinY && p.y + 1 <= clipMaxY))
                SetPixel(hdc, p.x, p.y + 1, fc);
            q.push(point(p.x, p.y + 1));
        }
        // Up
        c = GetPixel(hdc, p.x, p.y - 1);
        if (c != bc && c != fc) {
            if (!clipWindowSet || (p.x >= clipMinX && p.x <= clipMaxX && p.y - 1 >= clipMinY && p.y - 1 <= clipMaxY))
                SetPixel(hdc, p.x, p.y - 1, fc);
            q.push(point(p.x, p.y - 1));
        }
    }
} 