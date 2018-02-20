## ShipCAD

Models Ship hulls using subdivision surfaces.

Port of the Free!Ship program to C++ and Qt.

The original progam is located at: http://sourceforge.net/projects/freeship. It was last updated on 2013-04-19.

# Prerequisites
* c++
* Qt 5.7
* Boost 1.55 or greater
* Eigen 67e894c6cd8f linear algebra library

# Functionality to do:

Graphics:
  zebra shading has some artifacts
  grids, hydrostatic, curvature display

Main Window:
  display current mouse position

Dialogs:
  DXF Export
  Background Blending
  Control Point Form - Done
  Crosscurves
  Cylinder
  Expanded Plates
  Hullform - Partial, missing view window operations, captions, background image
  Hydrostatics
  Hydrostatics results
  Insert Plane - Done
  Intersection - Done
  Keel Wizard
  Lackenby
  Layers - Done
  Linesplan
  Michlet Output
  Mirror Plane - Done
  New Model - Done
  Preferences - Done
  Project Settings - Done
  Resistance Delft
  Resistance Kaper
  Rotate - Done
  Save Image
  Select Layer - Done
  Splash
  Undo History
  
Controller:
  import/export Background Image
  all import/export methods except native FreeShip
  Flowlines
  import frames
  intersection dialogs
  import/delete markers
  check model
  lackenby transformation
  undo/redo/undo history
  add cylinder
  keel and rudder wizard

Spline:
  draw doesn't show points

NurbSurface:
  all methods

Flowline:
  draw, rebuild

Resistance:
  all methods

Preferences:
  resetColors, persistence

BackgroundImage:
  updateData, updateViews

DevelopedPatch
  draw