# PixelCanvas with Sutherland-Hodgman Clipping

This application demonstrates the Sutherland-Hodgman polygon clipping algorithm, allowing you to clip both lines and polygons against a user-defined clipping window.

## How to Run

Simply execute the `PixelCanvas_SutherlandHodgman.exe` file or run the `build_final_solution.bat` script to compile and run the application.

## Usage Instructions

### Defining a Clipping Window
1. Start by right-clicking twice in the canvas to define the clipping window:
   - First click sets the top-left corner
   - Second click sets the bottom-right corner
2. The clipping window will be displayed with a blue outline

### Line Clipping
1. After defining the clipping window, the application defaults to line clipping mode
2. Left-click to set the first point of the line
3. Left-click again to set the second point and see the clipped result
4. The original line is shown as a dotted green line, while the clipped line is shown in solid red

### Polygon Clipping
1. Press 'P' to switch to polygon clipping mode
2. Left-click to add points to your polygon (minimum 3 points needed)
3. Right-click to complete the polygon and see the clipped result
4. The clipped polygon is shown in red

### Additional Controls
- Press 'C' to clear the canvas and start over
- Press 'L' to switch back to line clipping mode
- Press 'P' to switch to polygon clipping mode

## Algorithms Used
- **Sutherland-Hodgman Algorithm**: For both line and polygon clipping

## Implementation Details
This implementation uses the Clipping2D class from the Graphics namespace to perform:
- Line clipping using the Sutherland-Hodgman algorithm
- Polygon clipping using the Sutherland-Hodgman algorithm 