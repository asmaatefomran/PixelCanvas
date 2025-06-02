# Cardinal Spline Implementation in PixelCanvas

## Overview

A Cardinal Spline is a type of interpolating spline that passes through all control points while maintaining smooth transitions between segments. This implementation adds Cardinal Spline curve drawing functionality to the PixelCanvas application.

## Features

- Draw smooth Cardinal Splines that pass through all control points
- Adjustable tension parameter to control curve smoothness
- Visual feedback with control points displayed during drawing
- Double-click to complete the curve

## How to Use

1. Select "Curve" from the shape dropdown
2. Select "Cardinal" from the algorithm dropdown
3. Adjust the tension parameter using the dropdown (values range from 0.0 to 1.0)
4. Click to place control points on the canvas
5. Double-click to complete the curve

## Tension Parameter

The tension parameter controls how "tight" the curve is:

- **0.0**: Produces a looser curve with more gradual transitions
- **0.5**: Default value, balanced smoothness
- **1.0**: Produces a tighter curve with more pronounced corners

## Technical Implementation

The Cardinal Spline implementation uses the following components:

1. **Control UI**: A dropdown menu for tension selection
2. **Drawing Logic**: The `DrawCardinalSpline` function
3. **Mouse Handling**: Click to place points, double-click to finish

The Cardinal Spline formula uses the following basis functions:
- h₁(t) = 2t³ - 3t² + 1
- h₂(t) = -2t³ + 3t²
- h₃(t) = t³ - 2t² + t
- h₄(t) = t³ - t²

Where t is the parameter ranging from 0 to 1, and the tension parameter s = (1-tension)/2.

## Troubleshooting

If you encounter compilation errors with duplicate case statements in the WinAPI version, you'll need to manually edit the source file to remove the duplicate `case WM_MOUSEMOVE:` statement.

```cpp
// Add dropdown for Cardinal Spline tension when Cardinal Spline is selected
if (currentShape == CARDINAL_SPLINE) {
    // Define tension options
    static const char* tensionLabels[] = {
        "Tension: 0.0", 
        "Tension: 0.25", 
        "Tension: 0.5 (Default)", 
        "Tension: 0.75", 
        "Tension: 1.0"
    };
    
    static const float tensionValues[] = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    static const int numOptions = 5;
    
    // Position the dropdown in the console
    static const int dropdownWidth = 180;
    static const int dropdownHeight = 24;
    static const int dropdownX = SCREEN_WIDTH - dropdownWidth - 10;
    static const int dropdownY = SCREEN_HEIGHT - 27;
    
    // Draw the tension label and dropdown UI
    // ... (UI drawing code)
    
    // When an option is selected, update the cardinalTension global variable
    cardinalTension = tensionValues[selectedOption];
}
```

The global `cardinalTension` variable is used in the `DrawCardinalSpline()` method to control the curve tightness:

```cpp
// Calculate the tangents based on adjacent points using the global tension parameter
float tx1 = cardinalTension * (controlPoints[i+1].x - controlPoints[i-1].x);
float ty1 = cardinalTension * (controlPoints[i+1].y - controlPoints[i-1].y);
```

## Visual Appearance
The dropdown resembles the one shown in the reference image, appearing at the bottom of the screen in the status bar area. It has a white background with black text and borders, consistent with the application's UI style.

## Using the Feature
1. Select the Cardinal Spline tool from the menu
2. Click the dropdown in the bottom right of the screen to see tension options
3. Select a tension value to update the Cardinal Spline's appearance
4. Add control points by left-clicking, then right-click to complete the spline
5. The spline will be drawn using the selected tension parameter 