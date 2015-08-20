/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief ShipCAD model controller, contains editing actions
 */
class Controller : public QObject
{
	Q_OBJECT
public:
	explicit Controller(ShipCADModel* model);
	~Controller();

	QStringList& getRecentFiles() const;
	void addRecentFiles(const QString& filename)
		{ _recent_files.push_back(filename);}

public slots:
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
	/*! \brief import a FREE!Ship file
	 */
	void loadFile();
	/*! \brief load a FREE!Ship file by name
	 */
	void loadFile(const QString& filename);
	/*! \brief save a FREE!Ship file without asking for filename
	 */
	void saveFile();
	/*! \brief save a FREE!Ship file, asking for name
	 */
	void saveAsFile();
	/*! \brief 
	 */
        void addFlowline(const QVector2D& source, viewport_type_t view);
	/*! \brief calculate hydrostatics
	 *
	 * \param draft draft of calculation
	 * \param heel_angle angle of heel for calculation
	 * \param trim trim for calculation
	 */
	void calculuteHydrostatics(float draft, float heel_angle, float trim);
	/*! \brief calculate crosscurves
	 */
	void crossCurvesHydrostatics();
	/*! \brief calculate a range of hydrostatics
	 */
	void hydrostaticsDialog();
	
private:
	ShipCADModel* _model;
	QStringList _recent_files;
};
	
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // CONTROLLER_H
