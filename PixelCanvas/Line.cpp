#include "Line.h"
#include "PixelCanvas.h"
#include <algorithm>
using namespace std;

extern bool clipWindowSet;
extern int clipMinX, clipMaxX, clipMinY, clipMaxY;

// Cohen-Sutherland region codes
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

int ComputeOutCode(int x, int y) {
    int code = INSIDE;
    if (x < clipMinX) code |= LEFT;
    else if (x > clipMaxX) code |= RIGHT;
    if (y < clipMinY) code |= BOTTOM;
    else if (y > clipMaxY) code |= TOP;
    return code;
}

// Cohen-Sutherland line clipper
bool CohenSutherlandClip(int& x0, int& y0, int& x1, int& y1) {
    if (!clipWindowSet) return true;
    int outcode0 = ComputeOutCode(x0, y0);
    int outcode1 = ComputeOutCode(x1, y1);
    bool accept = false;
    while (true) {
        if (!(outcode0 | outcode1)) {
            accept = true;
            break;
        } else if (outcode0 & outcode1) {
            break;
        } else {
            int outcodeOut = outcode0 ? outcode0 : outcode1;
            int x, y;
            if (outcodeOut & TOP) {
                x = x0 + (x1 - x0) * (clipMaxY - y0) / (y1 - y0);
                y = clipMaxY;
            } else if (outcodeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (clipMinY - y0) / (y1 - y0);
                y = clipMinY;
            } else if (outcodeOut & RIGHT) {
                y = y0 + (y1 - y0) * (clipMaxX - x0) / (x1 - x0);
                x = clipMaxX;
            } else {
                y = y0 + (y1 - y0) * (clipMinX - x0) / (x1 - x0);
                x = clipMinX;
            }
            if (outcodeOut == outcode0) {
                x0 = x; y0 = y;
                outcode0 = ComputeOutCode(x0, y0);
            } else {
                x1 = x; y1 = y;
                outcode1 = ComputeOutCode(x1, y1);
            }
        }
    }
    return accept;
}

Line::Line(HDC hdc) : hdc(hdc) {}

void Line::DrawLineDDA(int x1, int y1, int x2, int y2, COLORREF c) {
    int cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
    if (!CohenSutherlandClip(cx1, cy1, cx2, cy2)) return;
    int dx = cx2 - cx1;
    int dy = cy2 - cy1;
    int steps = max(abs(dx), abs(dy));
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = cx1;
    float y = cy1;
    for (int i = 0; i <= steps; ++i) {
        SetPixel(hdc, round(x), round(y), c);
        x += xInc;
        y += yInc;
    }
}

void Line::DrawLineMidpoint(int x1, int y1, int x2, int y2, COLORREF c) {
    int cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
    if (!CohenSutherlandClip(cx1, cy1, cx2, cy2)) return;
    bool steep = abs(cy2 - cy1) > abs(cx2 - cx1);
    if (steep) {
        std::swap(cx1, cy1);
        std::swap(cx2, cy2);
    }
    if (cx1 > cx2) {
        std::swap(cx1, cx2);
        std::swap(cy1, cy2);
    }
    int dx = cx2 - cx1;
    int dy = abs(cy2 - cy1);
    int d = 2 * dy - dx;
    int yInc = (cy1 < cy2) ? 1 : -1;
    int y = cy1;
    for (int x = cx1; x <= cx2; ++x) {
        if (steep) SetPixel(hdc, y, x, c);
        else SetPixel(hdc, x, y, c);
        if (d > 0) {
            y += yInc;
            d -= 2 * dx;
        }
        d += 2 * dy;
    }
}

void Line::DrawLineParametric(int x1, int y1, int x2, int y2, COLORREF c) {
    int cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
    if (!CohenSutherlandClip(cx1, cy1, cx2, cy2)) return;
    int dx = cx2 - cx1;
    int dy = cy2 - cy1;
    int steps = max(abs(dx), abs(dy));
    for (int i = 0; i <= steps; ++i) {
        float t = i / (float)steps;
        int x = round(cx1 + dx * t);
        int y = round(cy1 + dy * t);
        SetPixel(hdc, x, y, c);
    }
}

void Line::DrawLineInterpolated(int x1, int y1, int x2, int y2, COLORREF c1, COLORREF c2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (abs(dx) >= abs(dy)) {
        if (x1 > x2) {
            swap(x1, x2);
            swap(y1, y2);
        }
        float m = (float)dy / dx;
        float y = y1;
        for (int x = x1; x <= x2; ++x) {
            float t = (float)(x - x1) / (x2 - x1);
            SetPixel(hdc, x, round(y), Mix(c1, c2, t));
            y += m;
        }
    } else {
        if (y1 > y2) {
            swap(x1, x2);
            swap(y1, y2);
        }
        float m = (float)dx / dy;
        float x = x1;
        for (int y = y1; y <= y2; ++y) {
            float t = (float)(y - y1) / (y2 - y1);
            SetPixel(hdc, round(x), y, Mix(c1, c2, t));
            x += m;
        }
    }
}

COLORREF Line::Mix(COLORREF c1, COLORREF c2, float t) {
    int r = GetRValue(c1) + t * (GetRValue(c2) - GetRValue(c1));
    int g = GetGValue(c1) + t * (GetGValue(c2) - GetGValue(c1));
    int b = GetBValue(c1) + t * (GetBValue(c2) - GetBValue(c1));
    return RGB(r, g, b);
} 