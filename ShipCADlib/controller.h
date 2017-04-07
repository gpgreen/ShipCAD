/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                                *
 *    e-mail                  : Info@FREEship.org                                              *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                      *
 *    FREE!ship homepage      : www.FREEship.org                                               *
 *                                                                                             *
 *    This program is free software; you can redistribute it and/or modify it under            *
 *    the terms of the GNU General Public License as published by the                          *
 *    Free Software Foundation; either version 2 of the License, or (at your option)           *
 *    any later version.                                                                       *
 *                                                                                             *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY          *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 *
 *                                                                                             *
 *    You should have received a copy of the GNU General Public License along with             *
 *    this program; if not, write to the Free Software Foundation, Inc.,                       *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    *
 *                                                                                             *
 *#############################################################################################*/

#ifndef CONTROLLER_H
#define CONTOLLER_H

#include <QtCore>
#include <QtGui>

#include "shipcadlib.h"
#include "shipcadmodel.h"
#include "dialogdata.h"

namespace ShipCAD {

class UndoObject;
class Intersection;
class SubdivisionControlPoint;
class SubdivisionLayer;
class Marker;
class Viewport;
struct PickRay;
    
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief ShipCAD model controller, contains editing actions
 */
class Controller : public QObject
{
	Q_OBJECT

public:

    /*! \brief constructor
     */
    explicit Controller(ShipCADModel* model);

    /*! \brief destructor
     */
	~Controller() {}

    /*! \brief get the model this is controlling
     */
    ShipCADModel* getModel()
        {return _model;}

    /*! \brief get the model this is controlling const version
     */
    const ShipCADModel* getModel() const
        {return _model;}

    /*! \brief shoot a pick ray, change items to selected
     *
     * \param vp the viewport that has the pick ray
     * \param ray the pick ray
     * \return true if pick ray found items and selected them, false if nohting picked
     */
    bool shootPickRay(Viewport& vp, const PickRay& ray);
    
signals:

	/*! \brief signal when undo data is changed
	 */
	void updateUndoData();

    /*! \brief signal when active layer changes
     */
	void changeActiveLayer();

    /*! \brief signal when selection of items changes
     * signal isn't required when modifiedModel is issued
     */
    void changeSelectedItems();

    /*! \brief signal when model has changes
	 */
	void modifiedModel();

    /*! \brief signal when new model loaded
     */
    void modelLoaded();

    /*! \brief show or disable control point dialog
     */
    void showControlPointDialog(bool);

    /*! \brief update the control point values
     */
    void updateControlPointValue(ShipCAD::SubdivisionControlPoint* pt);

    /*! \brief execute the Insert Plane Control Points dialog
     */
    void exeInsertPlanePointsDialog(ShipCAD::InsertPlaneDialogData& data);
    
    /*! \brief execute the Intersect Layers dialog
     */
    void exeIntersectLayersDialog(ShipCAD::IntersectLayersDialogData& data);

    /*! \brief execute the Extrude Edge dialog
     */
    void exeExtrudeEdgeDialog(ShipCAD::ExtrudeEdgeDialogData& data);

    /*! \brief execute the Choose Color dialog
     */
    void exeChooseColorDialog(ShipCAD::ChooseColorDialogData& data);

    /*! \brief show an info dialog
     */
    void displayInfoDialog(const QString& msg);

    /*! \brief show a warning dialog
     */
    void displayWarningDialog(const QString& msg);

    /*! \brief show an error dialog
     */
    void displayErrorDialog(const QString& msg);

public slots:

    /*! \brief delete the background image
     */
    void deleteBackgroundImage();

    /*! \brief open a background image
     */
    void openBackgroundImage();

    /*! \brief add a new controlcurve
	 */
	void addCurve();

    /*! \brief remove an edge by replacing the two connected faces with one face
	 */
	void collapseEdges();

    /*! \brief create a new edge by connecting 2 points belonging to the same face
	 */
	void connectEdges();

    /*! \brief switch selected edges between normal or crease edges
	 */
	void creaseEdges();

    /*! \brief extrude selected boundary edge
	 */
	void extrudeEdges();

    /*! \brief create new controlpoints by splitting control edge into two
	 */
	void splitEdges();

    /*! \brief assemble
	 */
	void assembleFace();

    /*! \brief delete all faces on the starboard side of the hull
	 */
	void deleteNegativeFaces();

    /*! \brief invert normal-direction of all selected faces
	 */
	void flipFaces();

    /*! \brief mirror all selected faces on a plane
	 */
	void mirrorPlaneFace();

    /*! \brief create a new control face from selected control points
	 */
	void newFace();

    /*! \brief rotate selected faces around an axis
	 */
	void rotateFaces();

    /*! \brief scale selected faces
	 */
	void scaleFaces();

    /*! \brief move selected faces
	 */
	void moveFaces();

    /*! \brief export stations to archimedes
	 */
	void exportFileArchimedes();

    /*! \brief export coordinates of controlpoints to a textfile
	 */
	void exportCoordinates();

    /*! \brief export intersections to DXF file as 2D polylines
	 */
	void export2DPolylinesDXF();

    /*! \briefbrief export all lines to a 3D DXF file as polylines
	 */
	void export3DPolylinesDXF();

    /*! \brief export all faces to a 3D DXF file
	 */
	void exportFacesDXF();

    /*! \brief export Freeship exchange format (FEF) file
	 */
	void exportFEF();

    /*! \brief export GHS file format
	 */
	void exportGHS();

    /*! \brief export geometry as a part file
	 */
	void exportPart();

    /*! \brief export file for CFD program Michlet
	 */
	void exportMichlet();

    /*! \brief import file from CFD program Michlet
	 */
	void importMichletWaves();

    /*! \brief export model as wavefront .obj file
	 */
	void exportObj();

    /*! \brief export intersections as a textfile of 3D points
	 */
	void exportOffsets();

    /*! \brief export surface to STL file
	 */
	void exportSTL();

    /*! \brief export an IGES file
     */
    void exportIGES();

    /*! \brief import a Carene XYZ file, create multi-chine boat with developable surfaces
	 */
	void importCarene();

    /*! \brief import chines from a textfile and fit a surface through them
	 */
	void importChines();

    /*! \brief import a Freeship exchange format (FEF) file
	 */
	void importFEF();

    /*! \brief import a file created with Carlsson's Hulls program
	 */
    void importHull();

    /*! \brief import a partfile and add to current geometry
	 */
	void importPart();

    /*! \brief import a polycad file
	 */
	void importPolycad();

    /*! \brief import a number of curves and fit a surface
	 */
	void importSurface();

    /*! \brief import a VRML 1.0 file
	 */
	void importVRML();

    /*! \brief load a FREE!Ship file by name
	 */
	void loadFile(const QString& filename);

    /*! \brief save a FREE!Ship file without asking for filename
	 */
	void saveFile();

    /*! \brief save a FREE!Ship file with given filename
	 */
	void saveFileAs(const QString& filename);

    /*! \brief
	 */
	void addFlowline(const QVector2D& source, viewport_type_t view);

    /*! \brief calculate hydrostatics
	 *
	 * \param draft draft of calculation
	 * \param heel_angle angle of heel for calculation
	 * \param trim trim for calculation
	 */
	void calculateHydrostatics();

    /*! \brief calculate crosscurves
	 */
	void crossCurvesHydrostatics();

    /*! \brief calculate a range of hydrostatics
	 */
	void hydrostaticsDialog();

    /*! \brief Load a bodyplan and try to fit surface to it
     */
    void importFrames();

    /*! \brief add a new intersection at the specified location
     */
    void addIntersection();

    /*! \brief add an intersection to the appropriate list
     */
    void addIntersectionToList(Intersection* inter);

    /*! \brief the intersection dialog
     */
    void intersectionDialog();

    /*! \brief all connected patches surrounded by creases grouped into new layer
     */
    void autoGroupLayer();

    /*! \brief develop all developable layers
     */
    void developLayers();

    /*! \brief the layer dialog
     */
    void layerDialog();

    /*! \brief delete all empty layers from the model
     */
    void deleteEmptyLayers();

    /*! \brief add a new empty layer
     */
    void newLayer();

    /*! \brief add a marker
     */
    void addMarker(Marker* marker);

    /*! \brief delete all markers from the model
     */
    void deleteMarkers();

    /*! \brief import markers from a textfile
     */
    void importMarkers();

    /*! \brief check the surface for inconsistent normal directions and leaks
     */
    void checkModel();

    /*! \brief start a new model (with a predefined surface)
     */
    void newModel();

    /*! \brief Affine hullform transformation according to Lackenby
     */
    void lackenbyModelTransformation();

    /*! \brief scale the model and all equivalent data
     */
    void scaleModel();

    /*! \brief merge 2 selected edges by removing common controlpoint
     */
    void collapsePoint();

    /*! \brief remove any unused points from the model
     */
    void removeUnusedPoint();

    /*! \brief fill all intersection of visible edges and 3D plane, inserts a point on each of these edges
     */
    void insertPlane();

    /*! \brief calculate the intersection points of 2 layers
     */
    void intersectLayerPoint();

    /*! \brief locks all selected points
     */
    void lockPoints();

    /*! \brief add a new point to the model with no edges/faces attached
     */
    void newPoint();

    /*! \brief move a selected point in the model
     */
    void movePoint(QVector3D changedCoords);

    /*! \brief method to call when point is no longer being dragged
     */
    void stopMovePoint();
    
    /*! \brief project all selected points onto a straight line through the first and last
     */
    void projectStraightLinePoint();

    /*! \brief unlock all selected points
     */
    void unlockPoints();

    /*! \brief unlock all points
     */
    void unlockAllPoints();

    /*! \brief show a warning when edit commands are invoked and the model has locked points
     */
    bool proceedWhenLockedPoints();

    /*! \brief calculate resistance of yachts according to Delft systematic yacht series
     */
    void delftResistance();

    /*! \brief calculate resistance of slender hulls (canoes) according to John Winters
     */
    void kaperResistance();

    /*! \brief deselect all selected items
     */
    void clearSelections();

    /*! \brief delete all selected items
     */
    void deleteSelections();

    /*! \brief select all items
     */
    void selectAll();

    /*! \brief restore the state of the model before last modification
     */
    void undo();

    /*! \brief restore the state of the model after previous undo
     */
    void redo();

    /*! \brief clear the undo history
     */
    void clearUndo();

    /*! \brief show the undo history
     */
    void showHistoryUndo();

    /*! \brief control net visibility
     */
    void showControlNet(bool val);

    /*! \brief interior edges
     */
    void showInteriorEdges(bool val);

    /*! \brief show control curves
     */
    void showControlCurves(bool val);

    /*! \brief show curve curvature
     */
    void showCurvature(bool val);

    /*! \brief show surface normals
     */
    void showNormals(bool val);

    /*! \brief show both sides of hull
     */
    void showBothSides(bool val);

    /*! \brief show markers
     */
    void showMarkers(bool val);

    /*! \brief underwater color change shading
     */
    void shadeUnderwater(bool val);

    /*! \brief show stations
     */
    void showStations(bool val);

    /*! \brief show buttocks
     */
    void showButtocks(bool);

    /*! \brief show grid
     */
    void showGrid(bool val);

    /*! \brief show waterlines
     */
    void showWaterlines(bool);

    /*! \brief show diagonals
     */
    void showDiagonals(bool);

    /*! \brief show Hydrostatic features
     */
    void showHydroData(bool);

    /*! \brief show flowlines
     */
    void showFlowlines(bool);

    /*! \brief dialog to create keels and rudders
	 */
	void keelAndRudderWizard();

    /*! \brief add a cylinder to model
	 */
	void addCylinder();

    /*! \brief set active layer color
	 */
	void setActiveLayerColor();

    /*! \brief corner point selected/deselected
     * \param sel true if corner point has been selected, false if deselected
     */
    void cornerPointSelected(bool sel);

    /*! \brief control point dialog changed point coordinates
     * \param x current x value
     * \param y current y value
     * \param z current z value
     */
    void dialogUpdatedPointCoord(float x, float y, float z);

private:
    ShipCADModel* _model; /**< the model this is controlling */
    bool _point_first_moved; /**< flag set when point is first moved */
};
	
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // CONTROLLER_H
