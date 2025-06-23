#include "PolygonFill.h"
#include <cmath>
#include <vector>

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

std::vector<point> ClipPolygonToRect(const point* poly, int n) {
    std::vector<point> input(poly, poly + n);
    std::vector<point> output;
    if (!clipWindowSet) return input;
    struct Edge { int x1, y1, x2, y2; };
    Edge edges[4] = {
        {clipMinX, clipMinY, clipMinX, clipMaxY}, 
        {clipMaxX, clipMinY, clipMaxX, clipMaxY}, 
        {clipMinX, clipMinY, clipMaxX, clipMinY}, 
        {clipMinX, clipMaxY, clipMaxX, clipMaxY}  
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
            case 0: 
                x = clipMinX;
                y = p1.y + dy * (clipMinX - p1.x) / dx;
                break;
            case 1: 
                x = clipMaxX;
                y = p1.y + dy * (clipMaxX - p1.x) / dx;
                break;
            case 2: 
                y = clipMinY;
                x = p1.x + dx * (clipMinY - p1.y) / dy;
                break;
            case 3: 
                y = clipMaxY;
                x = p1.x + dx * (clipMaxY - p1.y) / dy;
                break;
        }
        return point(x, y);
    };
    for (int e = 0; e < 4; ++e) {
        output.clear();
        if (input.empty()) break;
        point p1 = input.back();
        for (const auto& p2 : input) {
            bool p1_inside = inside(p1, e);
            bool p2_inside = inside(p2, e);
            if (p1_inside && p2_inside) {
                output.push_back(p2);
            }
            else if (p1_inside && !p2_inside) {
                output.push_back(intersect(p1, p2, e));
            }
            else if (!p1_inside && p2_inside) {
                output.push_back(intersect(p1, p2, e));
                output.push_back(p2);
            }
            p1 = p2;
        }
        input = output;
    }
    return output;
}

void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c) {
    std::vector<point> clippedPoly = ClipPolygonToRect(p, n);
    if (clippedPoly.size() < 3) return;
    initEdgeTable(edgeTable);
    buildPolygonEdgeTable(edgeTable, clippedPoly.data(), clippedPoly.size());
    renderPolygonFromTable(hdc, edgeTable, c);
    cleanupEdgeTable(edgeTable);
}

struct EdgeTableEntry {
    int left, right;
};

void init(EdgeTableEntry tbl[800]) {
    for (int i = 0; i < 800; i++) {
        tbl[i].left = INT_MAX;
        tbl[i].right = INT_MIN;
    }
}
void edge2table(EdgeTableEntry tbl[800], point v1, point v2){
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
    for (int y = ymin; y < ymax; y++)
    {
        if (x < tbl[y].left) tbl[y].left = (int)ceil(x);
        if (x > tbl[y].right) tbl[y].right = (int)floor(x);
        x += dx;
    }
}
void polygon2table(EdgeTableEntry tbl[800], point p[], int n) {
    point v1 = p[n - 1];
    for (int i = 0; i < n; i++) {
        point v2 = p[i];
        edge2table(tbl, v1, v2);
        v1 = p[i];
    }
}
void table2screen(HDC hdc, EdgeTableEntry tbl[800], COLORREF c) {
    for (int y = 0; y < 800; y++) {
        if (tbl[y].left < tbl[y].right)
            DrawLineDDA(hdc, tbl[y].left, y, tbl[y].right, y, c);
    }
}
void convexfill(HDC hdc, point p[], int n, COLORREF c) {
    std::vector<point> clippedPoly = ClipPolygonToRect(p, n);
    if (clippedPoly.size() < 3) return;
    EdgeTableEntry tbl[800];
    init(tbl);
    polygon2table(tbl, clippedPoly.data(), clippedPoly.size());
    table2screen(hdc, tbl, c);
}

void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc)
{
    COLORREF c = GetPixel(hdc, x, y);
    if (c == bc || c == fc)
        return;
    SetPixel(hdc, x, y, fc);
    myFloodFill(hdc, x + 1, y, bc, fc);
    myFloodFill(hdc, x - 1, y, bc, fc);
    myFloodFill(hdc, x, y + 1, bc, fc);
    myFloodFill(hdc, x, y - 1, bc, fc);
}

void myFloodFillqueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc)
{
    std::queue<point> q;
    q.push(point(x, y));
    while (!q.empty()) {
        point p = q.front();
        q.pop();
        COLORREF c = GetPixel(hdc, p.x, p.y);
        if (c == bc || c == fc)
            continue;
        SetPixel(hdc, p.x, p.y, fc);
        q.push(point(p.x + 1, p.y));
        q.push(point(p.x - 1, p.y));
        q.push(point(p.x, p.y + 1));
        q.push(point(p.x, p.y - 1));
    }
}