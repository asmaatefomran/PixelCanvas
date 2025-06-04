#pragma once
#include <windows.h>
#include "LineClipping.h"
#include "PolygonClipping.h"

// Function declarations for clipping operations
extern bool clippingMode;
extern bool lineClippingMode;
extern bool polygonClippingMode;
extern bool definingClipWindow;
extern bool drawingPolygon;

// Reset all clipping states
void resetClippingStates();

// Initialize clipping window with default values
void initClippingWindow();

// Handle line clipping input
bool handleLineClippingInput(HWND hwnd, int x, int y, bool isRightClick);

// Handle polygon clipping input
bool handlePolygonClippingInput(HWND hwnd, int x, int y, bool isRightClick);

// Draw the current clipping window
void drawClippingWindow(HWND hwnd);

// Start line clipping mode
void startLineClippingMode();

// Start polygon clipping mode
void startPolygonClippingMode(); 