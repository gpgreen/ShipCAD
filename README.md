# ShipCAD

Models Ship hulls using subdivision surfaces.

Port of the Free!Ship program to C++ and Qt.

The original progam is located at: http://sourceforge.net/projects/freeship. It was last updated on 2013-04-19.

# Prerequisites
* c++11 or later
* Qt 5.7
* Boost 1.55 or greater
* Eigen 67e894c6cd8f linear algebra library

## Build notes

To build the manual, latex is required.
```
latex manual.tex
latex manual.tex
dvipdft manual.dvi
```

## Functionality to do:

Graphics:
  zebra shading has some artifacts
  grids, hydrostatic, curvature display

Main Window:
  display current mouse position

Dialogs:
  DXF Export
  Background Blending
  Crosscurves
  Cylinder
  Expanded Plates
  Hullform - Partial, missing view window operations, captions, background image
  Hydrostatics
  Hydrostatics results
  Keel Wizard
  Lackenby
  Linesplan
  Michlet Output
  Resistance Delft
  Resistance Kaper
  Save Image
  Splash
  Undo History
  
Controller:
  import/export Background Image
  all import/export methods except native FreeShip
  Flowlines
  import frames
  lackenby transformation
  undo/redo/undo history
  add cylinder
  keel and rudder wizard

Spline:
  draw doesn't show points

Flowline:
  draw, rebuild

Resistance:
  all methods

BackgroundImage:
  updateData, updateViews

DevelopedPatch
  draw