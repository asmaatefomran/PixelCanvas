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

EdgeNode* edgeTable[800];

struct point {
    double x, y;
    point(double x = 0, double y = 0) : x(x), y(y) {}
};

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
    EdgeNode* tbl[800];
    initEdgeTable(tbl);
    buildPolygonEdgeTable(tbl, p, n);
    renderPolygonFromTable(hdc, tbl, c);
    cleanupEdgeTable(tbl);
}

typedef struct {
    int left, right;
}edgeTable[800];


void init(edgeTable tbl) {
    for (int i = 0;i < 800;i++) {
        tbl[i].left = INT_MAX;
        tbl[i].right = INT_MIN;
    }
}

void edge2table(edgeTable tbl, point v1, point v2){
    if (v1.y == v2.y)
        return;
    if (v1.y > v2.y)
        swap(v1, v2);
    int y = v1.y;
    double x = v1.x;
    double mi = (v2.x - v1.x) / (v2.y - v2.x);
    while (y < v2.y) {
        if (x < tbl[y].left)
            tbl[y].left = (int)ceil(x);
        if (x > tbl[y].right)
            tbl[y].right = (int)floor(x);
        y++;
        x += mi;
    }
}

void polygon2table(edgeTable tbl, point p[], int n) {
    point v1 = p[n - 1];
    for (int i = 0;i < n;i++) {
        point v2 = p[i];
        edge2table(tbl, v1, v2);
        v1 = p[i];
    }
}

void table2screen(HDC hdc, edgeTable tbl, COLORREF c) {
    for (int i = 0;i < 800;i++) {
        if (tbl[i].left < tbl[i].right)
            DrawLineDDA(hdc, tbl[i].left, i, tbl[i].right, i, c);
    }
}

void convexfill(HDC hdc, point p[], int n, COLORREF c) {
    edgeTable tbl;
    init(tbl);
    polygon2table(tbl, p, n);
    table2screen(hdc, tbl, c);
}

void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    COLORREF c = GetPixel(hdc, x, y);
    if (c == bc or c == fc)
        return;
    SetPixel(hdc, x, y, fc);
    myFloodFill(hdc, x + 1, y, bc, fc);
    myFloodFill(hdc, x - 1, y, bc, fc);
    myFloodFill(hdc, x, y + 1, bc, fc);
    myFloodFill(hdc, x, y - 1, bc, fc);
}

void myFloodFillqueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    queue<point> q;
    q.push(point(x, y));
    while (!q.empty()) {
        point p = q.front();
        q.pop();
        COLORREF c = GetPixel(hdc, p.x, p.y);
        if (c == bc or c == fc)
            continue;
        SetPixel(hdc, p.x, p.y, fc);
        q.push(point(p.x + 1, p.y));
        q.push(point(p.x - 1, p.y));
        q.push(point(p.x, p.y + 1));
        q.push(point(p.x, p.y - 1));
    }
}
