## ShipCAD

Models Ship hulls using subdivision surfaces.

Port of the Free!Ship program to C++ and Qt.

The original progam is located at: http://sourceforge.net/projects/freeship. It was last updated on 2013-04-19.

# Prerequisites
* c++
* Qt 5.7

# Functionality to do:

Graphics:
  zebra shading has some artifacts
  grids, hydrostatic, curvature display

Main Window:
  display of number of faces and points, current mouse position
  
Controller:
  import/export Background Image
  all import/export methods except native FreeShip
  Flowlines
  hydrostatics dialogs and displays
  import frames
  intersection dialogs
  import/delete markers
  check model
  lackenby transformation
  resistance dialogs and displays
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