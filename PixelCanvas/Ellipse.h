#ifndef ELLIPSE_H
#define ELLIPSE_H

#include <windows.h>

void DrawEllipseDirect(HDC hdc, int xc, int yc, int a, int b, COLORREF color);
void DrawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF color);
void DrawEllipseMidpoint(HDC hdc, int xc, int yc, int a, int b, COLORREF color);

#endif // ELLIPSE_H 