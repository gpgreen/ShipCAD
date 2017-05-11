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
#include <set>
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
#include "resistance.h"
#include "flowline.h"
#include "backgroundimage.h"

namespace ShipCAD {

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
    ~ShipCADModel() {}

    SubdivisionSurface* getSurface() {return &_surface;}
    const SubdivisionSurface* getSurface() const {return &_surface;}

    Visibility& getVisibility() {return _vis;}
    const Visibility& getVisibility() const {return _vis;}
    ProjectSettings& getProjectSettings() {return _settings;}
    Preferences& getPreferences() {return _prefs;}

    /*! \brief create a new model
     *
     * \param units units to use in model
     * \param length length of new hull
     * \param breadth beam of new hull
     * \param draft draft of new hull
     * \param rows how many rows of control points
     * \param cols how many cols of control points
     */
    void newModel(unit_type_t units, float length, float breadth, float draft,
                  size_t rows, size_t cols);
    
    /*! \brief Assembles all stations and builds a 2D bodyplan
     *
     * \param dest the spline list with assembled stations
     * \param close_at_deck
     */
    void buildValidFrameTable(SplineVector& dest, bool close_at_deck);

    // getBackgroundImage()
    bool isBuild() const {return _surface.isBuild();}
    void setBuild(bool set);

    /*! \brief scale the entire model and all associated data such as sttions
     *
     * \param scale 3D vector for scale transformation
     * \param override_lock if points are locked, scale them anyway if true
     * \param adjust_markers scale markers if true
     */
    void scaleModel(const QVector3D& scale, bool override_lock, bool adjust_markers);
    
    /*! \brief move selected faces and all associated data
     *
     * \param vect displacement vector
     * \param adjust_markers move markers if true
     */
    void moveFaces(std::vector<SubdivisionControlPoint*>& points,
                   const QVector3D& vec, bool adjust_markers);

    /*! \brief create an intersection
     *
     * \param type which type of intersection
     * \param distance location of intersection
     * \return the created intersection, or 0 if not created
     */
    Intersection* createIntersection(intersection_type_t ty, float distance);
    
    /*! \brief get the active control point
     *
     * \return the active control point or 0
     */
    SubdivisionControlPoint* getActiveControlPoint() const
        {return _active_control_point;}

    /*! \brief set the active control point
     *
     * \param pt the new active control point or 0
     */
    void setActiveControlPoint(SubdivisionControlPoint* pt)
        {_active_control_point = pt;}

    /*! \brief get precision of model
     *
     * \return precision of model
     */
    precision_t getPrecision() const {return _precision;}
    /*! \brief set precision of model
     *
     * \param precision new precision for model
     */
    void setPrecision(precision_t precision);

    version_t getFileVersion() const {return _file_version;}
    void setFileVersion(version_t v);

    IntersectionVector& getStations() {return _stations;}
    IntersectionVector& getWaterlines() {return _waterlines;}
    IntersectionVector& getButtocks() {return _buttocks;}
    IntersectionVector& getDiagonals() {return _diagonals;}
    
	edit_mode_t getEditMode() const {return _edit_mode;}
	void setEditMode(edit_mode_t mode);

    /*! \brief count the number of currently selected items
     *
     * \return number of selected items
     */
    size_t countSelectedItems() const;

    /*! \brief clear all selected items
     */
    void clearSelectedItems();

	/*! \brief name of file
	 *
	 * \return the file name
	 */
    QString getFilename() const;
	/*! \brief set the name of the file
	 *
	 * \param name name of the file
	 */
	void setFilename(const QString& name);
	/*! \brief get flag for filename set
	 *
	 * \return true if filename has been set
	 */
    bool isFilenameSet() const
        {return _filename_set;}
	/*! \brief set flag for filename set
	 *
	 * \param set new flag for filename set
	 */
    void setFilenameSet(bool flag)
        {_filename_set=flag;}
	
    HydrostaticCalcVector& getHydrostaticCalculations() {return _calculations;}

    // marker
    MarkerVector& getMarkers() {return _markers;}
    size_t numberOfMarkers() const {return _markers.size();}
    Marker* addMarker();
    bool isSelectedMarker(Marker* mark) const;
    void setSelectedMarker(Marker* mark);
    void removeSelectedMarker(Marker* mark);
    size_t numberOfSelectedMarkers() const
        {return _selected_markers.size();}
    void deleteMarker(Marker* mark);
    
    // flowlines
    Flowline* getFlowline(size_t index) {return _flowlines.get(index);}
    size_t numberOfFlowlines() const {return _flowlines.size();}
    /*! \brief is the flowline selected
     *
     * \param flow flowline to check
     * \return true if flowline selected
     */
    bool isSelectedFlowline(Flowline* flow) const;
    /*! \brief set a flowline as selected
     *
     * \param flow set this flowline as selected
     */
    void setSelectedFlowline(Flowline* flow);
    /*! \brief deselect a flowline
     *
     * \param flow deselect this flowline
     */
    void removeSelectedFlowline(Flowline* flow);
    /*! \brief count of selected flowlines
     *
     * \return count
     */
    size_t numberOfSelectedFlowlines() const
        {return _selected_flowlines.size();}
    /*! \brief delete a flowline from the model
     *
     * \param flow the flowline to delete from the model
     */
    void deleteFlowline(Flowline* flow);

	/*! \brief file changed flag
	 *
	 * \return true if file changed
	 */
	bool isFileChanged() const {return _file_changed;}	
	/*! \brief set file changed flag
	 *
	 * \param changed new value for flag
	 */
	void setFileChanged(bool changed) { _file_changed=changed;}

	// undo
	/*! \brief create an undo object
	 *
	 * \param undotext name of object shown in gui
     * \param accept whether to accept the object into undo list at creation
	 * \return the undo object
	 */
	UndoObject* createUndo(const QString& undotext, bool accept);
	/*! \brief add an undo to the undo list
	 *
     * \param undo the object to put in the undo list
	 */
	void acceptUndo(UndoObject* undo);
	/*! \brief undo the last operation
	 *
	 */
	void undo();
	/*! \brief redo the last undo
	 *
	 */
	void redo();
	/*! \brief get amount of memory used for undo
	 *
	 * \return memory used in mb
	 */
	size_t getUndoMemory() const;

    bool canUndo() const;
    bool canRedo() const;
    void clearUndo();
    
    /*! \brief get the lowest underwater point of hull
	 *
     * \return lowest point of hull
	 */
	float findLowestHydrostaticsPoint() const;
	
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);
    /*! \brief load the preview image from a file
     *
     * \param source the filebuffer to load image from
     * \param image where to put the loaded image
     */
    void loadPreview(FileBuffer& source, QImage* image);

    /*! \brief save selected faces as a part file
     *
     * \param filename string stored in file as the part name
     * \param buffer destination of the part data
     * \param faces the faces to store as the part
     */
    void savePart(const QString& filename, FileBuffer& buffer,
                  std::vector<SubdivisionControlFace*>& faces);
    /*! \brief load a part
     *
     * \param buffer the source of the part file
     * \return true if geometry actually loaded
     */
    bool loadPart(FileBuffer& source, version_t& partversion);

    /*! \brief load splines from a text file
     *
     * \param file the textstream to load from
     * \param splines the vector to store loaded splines
     */
    void loadChinesFromText(QTextStream& file, SplineVector& splines);

    /*! \brief imports chines and creates developable surface
     *
     * \param np number of control points to create per spline
     * \param chines a list of chines
     */
    void importChines(size_t np, SplineVector& chines);
    
    void rebuildModel(bool redo_intersections);

    void drawWithPainter(Viewport& vp, QPainter* painter);
    void draw(Viewport& vp);

    /*! \brief delete all selected items
     */
    void deleteSelected();
    
	void clear();

    /*! \brief find the bounding box of the model
     *
     * \param the minimum corner of the bounding box
     * \param the maximum corner of the bounding box
     */
    void extents(QVector3D& min, QVector3D& max);

signals:
	
    void undoDataChanged();
                               
public slots:

protected:

    void drawGrid(Viewport& vp, LineShader* lineshader);
	/*! \brief create temp redo object at end of undo list
	 *
	 * \return the undo object
	 */
	UndoObject* createRedo();

private:

    // define away copy constructor and assignment operator
    ShipCADModel(const ShipCADModel&);
    ShipCADModel& operator=(const ShipCADModel&);
    
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
    DelftSeriesResistance _delft_resistance;
	KAPERResistance _kaper_resistance;
    HydrostaticCalc* _design_hydrostatics;
	size_t _undo_pos;   /**< current undo position */
	int _prev_undo_pos; /**< make this an int so we can start at -1 **/
	std::deque<UndoObject*> _undo_list;
    std::set<Marker*> _selected_markers;
    std::set<Flowline*> _selected_flowlines;
    FlowlineVector _flowlines;
    BackgroundImageVector _background_images;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

