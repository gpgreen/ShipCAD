/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
 *    Original Copyright header below														   *
 *																							   *
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

#ifndef SHIPCADMODEL_H_
#define SHIPCADMODEL_H_

#include <vector>
#include <deque>
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"
#include "version.h"
#include "hydrostaticcalc.h"
#include "intersection.h"
#include "subdivcontrolcurve.h"
#include "visibility.h"
#include "projsettings.h"
#include "preferences.h"
#include "marker.h"
#include "subdivsurface.h"

namespace ShipCAD {

class Marker;
class Flowline;
class FileBuffer;
class SubdivisionControlPoint;
class SubdivisionFace;
class SubdivisionLayer;
class Viewport;
class UndoObject;
	
//////////////////////////////////////////////////////////////////////////////////////

class ShipCADModel : public QObject
{
    Q_OBJECT
public:

    explicit ShipCADModel();
    ~ShipCADModel();

    Visibility& getVisibility() {return _vis;}
    ProjectSettings& getProjectSettings() {return _settings;}
    Preferences& getPreferences() {return _prefs;}

    /*! \brief Assembles all stations and builds a 2D bodyplan
     *
     * \param close_at_deck
     */
    void buildValidFrameTable(bool close_at_deck);
    // getBackgroundImage()
    bool isBuild() {return _surface.isBuild();}
    void setBuild(bool set);

    /*! \brief get precision of model
     *
     * \return precision of model
     */
    precision_t getPrecision() {return _precision;}
    /*! \brief set precision of model
     *
     * \param precision new precision for model
     */
    void setPrecision(precision_t precision);

    version_t getFileVersion() {return _file_version;}
    void setFileVersion(version_t v);

    IntersectionVector& getStations() {return _stations;}
    IntersectionVector& getWaterlines() {return _waterlines;}
    IntersectionVector& getButtocks() {return _buttocks;}
    IntersectionVector& getDiagonals() {return _diagonals;}
    SubdivisionControlCurveVector& getControlCurves() {return _control_curves;}
    Flowline* getFlowline(size_t index);

	edit_mode_t getEditMode() {return _edit_mode;}
	void setEditMode(edit_mode_t mode);

	/*! \brief name of file
	 *
	 * \return the file name
	 */
    QString getFilename();
	/*! \brief set the name of the file
	 *
	 * \param name name of the file
	 */
	void setFilename(const QString& name);
	/*! \brief get flag for filename set
	 *
	 * \return true if filename has been set
	 */
    bool isFilenameSet()
        {return _filename_set;}
	/*! \brief set flag for filename set
	 *
	 * \param set new flag for filename set
	 */
    void setFilenameSet(bool flag)
        {_filename_set=flag;}
	
    HydrostaticCalcVector& getHydrostaticCalculations() {return _calculations;}

    SubdivisionLayer* getActiveLayer() {return _surface.getActiveLayer();}
    size_t numberOfLayers() {return _surface.numberOfLayers();}
    SubdivisionLayer* getLayer(size_t index) {return _surface.getLayer(index);}

	size_t numberOfFlowlines();
    size_t numberOfLockedPoints();
    size_t numberOfViewports();

    // marker
    bool isSelectedMarker(Marker* mark);
    void setSelectedMarker(Marker* mark);
    void removeSelectedMarker(Marker* mark);
    bool adjustMarkers();

    // viewport? we might want to move this into the gui window class
    void addViewport(Viewport* vp);

    SubdivisionSurface* getSurface() {return &_surface;}

	/*! \brief file changed flag
	 *
	 * \return true if file changed
	 */
	bool isFileChanged() {return _file_changed;}	
	/*! \brief set file changed flag
	 *
	 * \param changed new value for flag
	 */
	void setFileChanged(bool changed);

	// undo
	/*! \brief add undo object
	 *
	 * \param new undo object
	 */
	void addUndoObject(UndoObject* newundo);
	/*! \brief get undo object at index
	 *
	 * \param index of undo object
	 * \return the undo object at index
	 * \throws range_error
	 */
	UndoObject* getUndoObject(size_t index);
	/*! \brief get undo object at index
	 *
	 * \param index of undo object
	 * \return the undo object at index
	 * \throws invalid_argument
	 */
	void deleteUndoObject(UndoObject* deleted);
	/*! \brief number of undo objects
	 *
	 * \return number of undo objects
	 */
	size_t undoCount() {return _undo_list.size();}
	/*! \brief current undo index position
	 *
	 * \return current undo index position
	 */
	size_t undoPosition() {return _undo_pos;}
	/*! \brief previous undo index position
	 *
	 * \return previous undo index position
	 */
	size_t prevUndoPosition() {return _prev_undo_pos;}
	/*! \brief set undo index position
	 *
	 * \param new index in undo list
	 * \throws range_error
	 */
	void setUndoPosition(size_t index);
	/*! \brief set previous undo index position
	 *
	 * \param new prev undo index
	 * \throws range_error
	 */
	void setPrevUndoPosition(size_t index);
	/*! \brief get amount of memory used for undo
	 *
	 * \return memory used in mb
	 */
	size_t getUndoMemory();

	/*! \brief redraw the model
	 *
	 * TODO this should be in the controller, not here
	 */
	void redraw();

    /*! \brief get the lowest underwater point of hull
	 *
     * \return lowest point of hull
	 */
	float findLowestHydrostaticsPoint();
	
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);
    void savePart(std::vector<SubdivisionFace*> faces);

    void rebuildModel();

	void clear();
    void clearUndo();

    /*! \brief find the bounding box of the model
     *
     * \param the minimum corner of the bounding box
     * \param the maximum corner of the bounding box
     */
    void extents(QVector3D& min, QVector3D& max);

signals:
	
    void onFileChanged();
    void onChangeCursorIncrement();
    void onUpdateGeometryInfo();
    void onUpdateVisibilityInfo();
	void changedLayerData();
	void changeActiveLayer();

public slots:

protected:

private:

    std::vector<Viewport*> _viewports;
    precision_t _precision;
    version_t _file_version;
    edit_mode_t _edit_mode;
    Preferences _prefs;
    SubdivisionControlPoint* _active_control_point;
    bool _file_changed;
    SubdivisionSurface _surface;
    QString _filename;
    // Edit class
    IntersectionVector _stations;
    IntersectionVector _waterlines;
    IntersectionVector _buttocks;
    IntersectionVector _diagonals;
    SubdivisionControlCurveVector _control_curves;
    MarkerVector _markers;
    Visibility _vis;
    // Frame class
    bool _filename_set;
    bool _currently_moving;
    bool _stop_asking_for_file_version;
    // previous cursor position
    ProjectSettings _settings;
    HydrostaticCalcVector _calculations;
    // undo objects
    // delft resistance
    // kaper resistance
    HydrostaticCalc* _design_hydrostatics;
	size_t _undo_pos;
	size_t _prev_undo_pos;
	std::deque<UndoObject*> _undo_list;
    std::vector<Marker*> _selected_markers;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

