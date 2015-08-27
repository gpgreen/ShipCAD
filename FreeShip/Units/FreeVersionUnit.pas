{#############################################################################################}
{    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
{    open source surface-modelling program based on subdivision surfaces and intended for     }
{    designing ships.                                                                         }
{                                                                                             }
{    Copyright © 2005, by Martijn van Engeland                                                }
{    e-mail                  : Info@FREEship.org                                              }
{    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
{    FREE!ship homepage      : www.FREEship.org                                               }
{                                                                                             }
{    This program is free software; you can redistribute it and/or modify it under            }
{    the terms of the GNU General Public License as published by the                          }
{    Free Software Foundation; either version 2 of the License, or (at your option)           }
{    any later version.                                                                       }
{                                                                                             }
{    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
{    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
{    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
{                                                                                             }
{    You should have received a copy of the GNU General Public License along with             }
{    this program; if not, write to the Free Software Foundation, Inc.,                       }
{    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
{                                                                                             }
{#############################################################################################}

unit FreeVersionUnit;

// Unit to keep track of fileversions, bux fixes and release dates

interface

uses SysUtils,
     Dialogs;

type TFreeFileVersion     = (fv100,fv110,fv120,fv130,fv140,fv150,fv160,fv165,fv170,fv180,fv190,fv191,fv195,fv198,fv200,
                             fv201,fv210,fv220,fv230,fv240,fv250,fv260);
const CurrentVersion      = fv260;   // Current (latest) version of the FREE!ship project.
                                     // All new created models are initialized to this version
      ReleasedDate        = 'April 21, 2006';
function VersionString(Version:TFreeFileVersion):String;

{
Version 2.60
Released April 21, 2006

New features:
   *  Select and delete markers
   *  Multilanguage support completed. Current complete translations include:
            English, Dutch, Suomi, Deutsch, Castellano and French
   *  Select all command added
   *  Several modifications to the controlpoint form, also made it transparent
      so the model shines through
   *  Shade submerged surfaces of developed plates in a different color
   *  Support asymmetrical layers (only layers that are not used for hydrostatics)
   *  Transparency setting of layers for transparent shading
   *  Export to STL file (ASCII version)
   *  Cross curves calculation
   *  Export boundary coordinates of developed plates to textfile
Bugs fixes or improvements:
   *  Fixed bug in intersection routine (sometimes no knuckles were inserted)
   *  Support filenames with multiple dots in the recent files list
   *  Made some modifications to the GHS export format as suggested by
      Creative Systems Inc
   *  Correct displaying and exporting of intersectioncurves on mirrored
      developed plates
   *  Dragging/positioning of background images with the mouse

Version 2.50
Released March 3rd, 2006

New features:
   *  Mirroring of developed plates
   *  Multiple language support
   *  Add scaled background images to viewports in order to reproduce the
      lines of an existing hull.
   *  Export to GHS
   *  Visualize geometry based flowlines
   *  Add cylindrical surfaces
Bugs fixes or improvements:
   *  Bug fixed in the check model procedure
   *  Added the dimensioning of cornerpoints of developed plates
   *  Bug in Delft resistance when length dwl was 0.0
   *  Increased visibility of thin lines of preview image
   *  Old file format (.free files, prior to Version 1.91) no longer supported
   *  Max. limit on undo memory
   *  Draft and trim settings for hydrostatic calculations are stored in file

Version 2.40
Released Februari 5th, 2006

New features:
   *  Keel and rudder wizard dialog added
   *  find the intersection of edges of one layer with faces of a second layer
   *  Zebra stripe shading to check fairness
   *  Redo
   *  Clear undo history
   *  Browse through undo history to view or select any previous state
   *  Import PolyCad files
   *  Remove unused points from the model
   *  Mainform size and position is stored when the program terminates 
Bugs fixes or improvements:
   *  Replaced Richedit component in design-hydrostatics report by a TMemo component
      to fix a WINE related bug
   *  Fixed the sometimes incorrect numbering of new layers
   *  Correct displaying of markers and controlcurves in bodyplan view
   *  Improved the automated normal direction test
   *  Error in IGES file so that SolidWorks and some other packages now import it
   *  Shifting points of the aftship with arrowkeys in the bodyplan view
   *  Viewports from the Lackenby hullform transformation are made sizeable

Version 2.30
Released January 05, 2006

New features:
   *  Show geometry info on statusbar
   *  Added vert. prismatic coefficient in design hydrostatics
   *  Lackenby hullform transformation (autom. adapting Cb,Cp and LCB)
   *  Export NURBS surfaces to an IGES file
   *  Split bodyplan view
   *  Import and export part of the geometry as "parts" to a file
Bugs fixes or improvements:
   *  Improved routine for developing surfaces, also shows difference between the
      3D surface area and the unfolded 2D area 
   *  Add switch for removing points from intersections (simplifying) in project settings
   *  Export stations, buttocks and waterlines to (individual) 2D DXF file(s) with
      adjustable precision. (no diagonals, edges and controlcurves are exported)
   *  Surface export to dxf now sends meshes instead of 3D faces for all faces with
      4 points. All the other faces are still exported as 3D faces
   *  Fixed error in intersections which caused loops in some intersections
   *  Rewrote the entire VRML import routine
   *  Added normal direction test before unfolding developable surfaces
   *  Display the new filename in the captionbar after file->Saveas
   *  Restyled the intersections dialog

Version 2.20
Released December 02, 2005

New features:
   *  Import Michlet wave elevations
   *  Some hydrostatic properties are shown in the viewports, such as LCB, SAC, metacentric height etc. (mainparticuars must have been set)
   *  Mirror selected faces or layers
   *  Show Gaussian curvature of the surfaces in shading mode
   *  Resistance for yachts according to Delft Systematic Yacht Series
Bugs fixes or improvements:
   *  Rearranged the project settings dialog, it now has tabs for different data and settings
   *  Prevented the controlpoint-form to recieve focus while dragging a point, which caused the active
      viewport not to respond to mousewheel zooming after dragging a point
   *  Use escape key to quickly deselect all
   *  Added shortcuts for easier switching between the different views and shading modes
   *  Ctrl-L shortcut opens layer dialog
   *  Output to DXF 3D-Polylines now also includes crease edges (knuckle lines)
   *  Optimized DXF output of linesplan a bit so that it does uses less line-segments for
      the crease edges as before. Data is now written as 2D polylines instead of 3D polylines
   *  Developed plates now written as 2D polylines instead of 3D polylines to a DXF file
   *  Improved lighting model used for shading the surfaces
   *  Scaling, rotating and moving now also applicable to individual layers
   *  Bug fixed in edge collapse
   *  Deselect a point when it is selected and ctrl-clicked
   *  ("No seedface could be found") bug fixed in unfolding routine 


Version 2.10
Released November 03, 2005

New features:
   *  Show/hide layers in the linesplan
   *  Save a preview of the model in the file (used for browsing designs at the FREE!ship website)
   *  Choose in project settings if hydrostatic coefficients should be calculated using the dimensions
      from the project settings or the actual waterline dimensions
   *  Export files to Michlet
   *  Zoom in/out by using the mousewheel, rotation of perspective viewports by keeping middle
      mousebutton (or mousewheel) pressed
   *  KAPER resistance calculation for canoes and kayaks, according to John Winters
   *  Move points with arrow keys (not in perspective view). Increase/decrease stepsize by
      pressing + or -. Click the panel saying "Incr. distance" on the statusbar to manually set
      the distance.
   *  Added the following waterplane properties to the hydrostatics: waterplane area, coeff., LCF,
      entrance angle and Trv/long. moment of inertia
   *  Added metacentric height to hydrostatics
   *  Ctrl selecting of faces
   *  Added sectional area info to design hydrostatics
   *  Project selected points on a straight line segment

Bugs fixes or improvements:
   *  Fixed bug in offsets import from round bottomed hulls that ignored the metric/imperial
      settings in the file
   *  Prevent the program from zooming out on the model when undo is used
   *  When layers are turned on/off, viewports that have been zoomed in or out are simply redrawn,
      others viewports are zoomed out to the extents of the entire model
   *  Severe error occurring when an edge was collapsed with two faces attached with inconsistent
      normal directions
   *  Fixed locked points that where not scaled when the project units was changed to metric/imperal
      are correctly processed
   *  Corrected an error in the routine were imperial distances were translated into a text containing
      the number of feet and fractional inches. This error occurred when the value was negative but larger
      the -1.0 foot. In those cases the negative sign was omitted.

Version 2.0
Released October 06, 2005

   *  Added a database with FREE!ship designs to the website. These designs may be freely
      downloaded from www.freeship.org/designs Users willing to share their own designs are
      kindly invited to send their designs to designs@freeship.org

   *  Added import functionality of chines from a textfile (default meters, use either comma or period)
   *  Import markers from a textfile
   *  Modified input-format of offsets of rounded hulls
   *  Implemented controlcurves as a new feature
   *  Controlcurves are added to the chines of imported chine hulls
   *  Create one panel for bottom panels and transoms (and other surfaces) that are flat
   *  Switch visibility of partnames on or off in developed plates dialog
   *  Export all controlpoint coordinates to a textfile which can be imported in Rhino
   *  Modified the offsets output, also exports controlcurves
   *  The order of layers in the layer dialog can be modified. Also developed plates appear in the same
      order as the layers in the layer dialog
   *  View surface area, weight and COG from within the layer properties dialog
   *  Insert multiple points in one pass by intersecting visible edges with a 3D plane
   *  Lock/unlock points
   *  Modified the developability-shading procedure by lowering the value used to test for developability
      so that it produces less noise
   *  Extended the current hydrostatic calculations and modified the output format

Version 1.91
Released September 08, 2005
   *  Major modification to the file format and file extension. FREE!hip models are from
      now on stored as binary files. This reduces the average filesize (and undo memory) to
      approx. 40 percent. This modification is done in preparation for web distribution of
      FREE!ship models (user database) and for the inclusion of a preview thumbnail in the file.
      The extension if the files is modified from .free to .fbm (Freeship Binary Model) Old
      files can still be imported but saving in the old FREE!ship file-format is no longer
      supported.
   *  Monochrome drawing of linesplan
   *  If no diagonals are present in the model, the plan view can optionally
      be mirrored in the linesplan, so that both port and starboardside are visible
   *  New startup screen
   *  Export the formatted linesplan to an AutoCad dxf file (lines only, no fill colors used)
   *  Added comment and file-creator fields to project settings dialog
   *  Import of XYZ files describing longitudinal chines, created with Carene
   *  Added dimensioning of unfolded parts to the developed plates window
   *  Enabled manual input of the rotation angle of unfolded plates
   *  Updated imperial coordinate input
   *  Save viewports as bmp image
   *  Select/deselect edgeloops by ctrl-clicking an edge
   *  Fixed bug which caused faces created by edge-extrusion to be assigned to another
      layer then the currently active layer
   *  Only shade the submerged surfaces in another color if the layer is also used for hydrostatics
   *  Import and interpolate a surface defined as a number of curves in a textfile
   *  Curvature plot of intersectionlines
   *  Added material properties to layers used to calculate weight of the surface area of a layer
   *  Program remembers settings of the intersection dialog
   *  Separate save and saveas functions
   *  Shading of submerged parts on the linesplan is turned of when it is disabled in the projectsettings.

Version 1.80
Released August 18, 2005
   *  Export stations now also to Archimedes (single body) in addition to
      ArchimedesMB (ref. manual section 3.5.6)
   *  Improved Archimedes export routine by merging stations containing multiple
      segments into one larger station (ref. manual section 3.5.6)
   *  Modified selecting procedure of controlpoints. Selecting multiple
      controlpoints from this release and on is done by keeping the ctrl-key
      pressed. If not, then only 1 controlpoint is selected at a time. (ref.
      manual section 2.2)
   *  Input relative values and imperial units in the controlpoint form (ref.
      manual section 2.4)
   *  Extra buttons and layer properties in layer properties dialog (ref.
      manual section 9.8)
   *  Keep as much from the original data when assigning faces to new layers
      using the autogroup function
   *  Improved unfolding routine, highlight edges which are stretched or compressed
      and added intersection with diagonals (ref. manual section 12.3)
   *  Linesplan view added (ref. manual section 4.2)
   *  Diagonals added to intersection lines. (ref. manual section 13.1)
   *  Delete all intersections in one pass from intersection dialog
   *  Printing from multiple windows (ref. manual section 2.6)
   *  Program now gives a warning when the current model has been changed and a
      new one is about to be loaded
   *  Created a dialog to control the image size when saving data to a bitmap
   *  Added list with recently opened files to the menu for faster acces
   *  Drawing of leak points in a different color (ref. manual section 12.1)

Version 1.70
Released July 29, 2005
   *  Unfold developable plates
   *  After auto-group of layers, the proper active layer is displayed
   *  When importing a .hul file, one new layer is created between two subsequent chines
      and one layer for faces which are added to close the hull at the centerline
   *  Store separate (last used) paths for import, export, save or load files
   *  Export stations to ArchimedesMB
   *  Implemented different shading modes

Version 1.65
Released July 14, 2005
   *  Set decimal seperator to a point.
   *  Fixed some minor points on .hul input
   *  Remember settings from hydrostatics dialog
   *  Fixed shading of underwaterview when the lowest part of the hull was not on
      Z=0.0 plane (height of waterline was incorrect)
   *  Model-check routine now also indicates location of leak points.
   *  Automatic model-checking can be swtched off in ptojectsettings dialog.
   *  When a new default model is started the user can also specify if the project
      should use metric or imperial units.
   *  Information exported to a DXF file is the same as what is seen on screen.
   *  3D translation of the entire model
   *  Mainframe location is also adapted when the model is scaled or moved
   *  Bug fixed that occurred when first a point was selected in a orthogonal view
      and secondly an edge was selected. The selected point then incorrectly moved
      to a new location.
   *  Manual add a new point in 3D space.

Version 1.60
Released July 7, 2005
   *  Export faces to wavefront .Obj format
   *  Changed capital characters in filenames to lowercase characters for Linux users
   *  Bug fixed which occurred when shading developable surfaces when the model exceeded
      the right side of the viewport
   *  Invert the normal direction of selected controlfaces
   *  Included user definable mainframe location in projectsettings
   *  Program preferences such as colors and controlpointsize can now
      be specified by the user
   *  Import a 3D bodyplan from textfile and fit a surface
   *  Export stations, buttocks and waterlines to 3D DXF file
   *  Export surface as 3Dfaces to DXF
   *  Import .hul files created with Carlson's freeware Hulls program
   *  Program performs an automated check for inconsistencies each time hydrostatics are calculated
   *  Bug fixed which caused incorrect length/beam waterline when interior edges started or
      ended exactly on the waterline at the calculated draft
   *  Export all intersections as offsets to a textfile.

Version 1.50
Released June 23, 2005 
   *  Fixed VRML import error, when all edges were set as crease edges
   *  Added metric/imperial units header to hydrostatics report when calculating multiple drafts
   *  Bug fixed which caused "spikes" on the surface after an undo operation
   *  Corrected memoryleak in undo-object data
   *  Mark layers as developable. Developable layers are shaded differently. Parts
      that have zero gauss curvature (thus are developale) are shaded green. Areas which are not
      developable are shaded red.

Version 1.40
Released June 22, 2005
   *  Save hydrostatics to richtext or plain textfile
   *  Import a VRML 1.0 file (for example from Hull program by Carlson design)
   *  Automatic removal of all controlfaces entirely on the starboardside of the hull
   *  Bug fixed, when deleting faces the remaing boundaryedges where set crease=true, but their
      start/endpoints remained regular vertices instead of creasevertices
   *  Rotate the model in 3D space
   *  3D scaling of the entire model
   *  Included wetted surface in hydrostatics
   *  Error messages are shown in hydrostatics
   *  Fixed bug that occurred when selecting controlpoints in viewports with a very high zoom ratio
   *  Made another modification to the fileversioning system
   *  Smoother zooming and panning
   *  Draw grid of intersections in profile, plan and bodyplan views
   *  Open email program or web browser when clicked on the hyperlinks in the startup/about screen
   *  Removed the Del-shortcut which caused selected items to be deleted when the delete key was
      pressed within the controlpoint form
   *  Implemented undo-function
   *  Calculate and output hydrostatic data for a range of drafts and trim

Version 1.30
Released June 14, 2005
   *  Draw normals of selected controlfaces when the visibility of interior faces is set to true
   *  Pan settings of the viewport are adapted when zooming in or out
   *  Implemented surface-analyzation to check for consistent and correct normal direction
   *  Interior edges descending from controledges are drawn slightly darker then normal interior edges.
   *  Implemented first simple hydrostatic calculations, based on surface panels rather then ordinates
   *  Switch between metric and imperial units
   *  Open file on double-click in windows explorer

Version 1.20
Released June 9, 2005
   *  Update the coordinate information of the active controlpoint in the controlpoint form
      when a point is being dragged with the mouse
   *  Included multiple project settings
   *  Shading of the underwaterpart in a different color
   *  Create a backup of old file when saving to disk

Version 1.10
Released June 7, 2005
   *  Added GNU General Public License info and copyright information to all units
   *  Bug fix: The crease property of boundary edges may not be set to false
   *  Made a change to the file-version which is now saved as a floatingpoint value
   *  Implemented a controlpoint form, for manual editing of a controlpoint

Version 1.0
Released June 3, 2005
   *  Initial release of FREE!ship and sourcecode
}

implementation

uses FreeLanguageSupport;

function VersionString(Version:TFreeFileVersion):String;
begin
   Case Version of
      fv100  : Result:='1.0';
      fv110  : Result:='1.1';
      fv120  : Result:='1.2';
      fv130  : Result:='1.3';
      fv140  : Result:='1.4';
      fv150  : Result:='1.5';
      fv160  : Result:='1.6';
      fv165  : Result:='1.65';
      fv170  : Result:='1.7';
      fv180  : Result:='1.8';
      fv190  : Result:='1.9';
      fv191  : Result:='1.91';
      fv195  : Result:='1.95';
      fv198  : Result:='1.98';
      fv200  : Result:='2.0';
      fv201  : Result:='2.01';
      fv210  : Result:='2.1';
      fv220  : Result:='2.2';
      fv230  : Result:='2.3';
      fv240  : Result:='2.4';
      fv250  : Result:='2.5';
      fv260  : Result:='2.6';
      else MessageDlg(Userstring(204)+'!',mtError,[mbok],0);
   end
end;{VersionString}

end.
