# PixelCanvas

PixelCanvas is a Windows application that implements various computer graphics algorithms. This application allows users to draw lines, circles, ellipses, and more using different algorithms.

## Features

- **Drawing Tools**:
  - Lines (DDA, Midpoint, Parametric)
  - Circles (Direct, Polar, Iterative Polar, Midpoint, Modified Midpoint)
  - Ellipses (Direct, Polar, Midpoint)
  - Curves (Bezier, Hermite, Cardinal Spline)
  - Polygons
  - Clipping Windows

- **Cardinal Spline Implementation**:
  - Uses Hermite curve calculation for smooth interpolation
  - Interactive control point placement with visual feedback
  - Tension parameter of 0.5 (Catmull-Rom spline)
  - Support for multiple control points (minimum 4 required)
  - Visual display of control points for easy editing

- **Color Selection**:
  - Choose from a palette of 10 colors
  - Color selection displayed in status bar

- **User Interface**:
  - Custom pencil-shaped cursor for drawing
  - Status bar with helpful instructions
  - Keyboard shortcuts:
    - Ctrl+S: Save canvas
    - Ctrl+O: Load canvas
    - Ctrl+N: Clear canvas
    - Ctrl+Z: Toggle custom cursor
  - Mouse-based interaction for drawing

- **File Operations**:
  - Save canvas data to file
  - Save screenshot of canvas
  - Load canvas data from file
  - Clear canvas

## Building the Application

You can build the application using one of the following methods:

- `build_and_run.bat` - Build and run using CMake
- `run_with_installed_raylib.bat` - Run with system-installed Raylib
- `build_cardinal_raylib.bat` - Build and run with Cardinal Spline support

## Usage

1. Select a drawing tool from the top menu.
2. Select a color from the color palette.
3. Use the mouse to draw on the canvas:
   - For lines, circles, and ellipses: Click and drag to set start and end points.
   - For polygons: Left-click to add points, right-click to complete.
   - For curves: Click to set control points.
   - For Cardinal Splines: Left-click to add at least 4 control points, right-click to complete and draw the spline.

## Cardinal Spline Drawing

To draw a Cardinal Spline:
1. Select "Cardinal Spline" from the top menu.
2. Left-click on the canvas to place control points (at least 4 are required).
3. Control points are displayed as small circles on the canvas.
4. Right-click to complete and draw the smooth curve through the control points.
5. The spline uses a tension parameter of 0.5, which creates a Catmull-Rom spline with C1 continuity.

## Implementation Details

The application implements various computer graphics algorithms, including:

- Line Drawing: DDA, Midpoint, Parametric
- Circle Drawing: Direct, Polar, Midpoint
- Ellipse Drawing: Direct, Polar, Midpoint
- Curve Drawing: Bezier, Hermite, Cardinal Spline
- Filling Algorithms: Scan Line, Flood Fill
- Clipping Algorithms: Cohen-Sutherland, Liang-Barsky

## Recent Updates

- Added improved Cardinal Spline implementation with visual control points
- White background for better visibility
- Custom drawing cursor
- Status bar with helpful instructions
- Enhanced save/load functionality
- Keyboard shortcuts for common operations

## License

This project is open-source and available under the MIT License.
