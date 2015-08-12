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

#ifndef SHIPCAD_H_
#define SHIPCAD_H_

#include <vector>
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

namespace ShipCAD {

class Marker;
class Flowline;
class FileBuffer;
class SubdivisionControlPoint;
class SubdivisionFace;
class SubdivisionSurface;
class SubdivisionLayer;
class Viewport;

//////////////////////////////////////////////////////////////////////////////////////

class ShipCADModel : public QObject
{
    Q_OBJECT
public:

    explicit ShipCADModel();
    ~ShipCADModel();

    ProjectSettings& getProjectSettings() {return _settings;}
    Preferences& getPreferences() {return _prefs;}

    void buildValidFrameTable(bool close_at_deck);
    SubdivisionLayer* getActiveLayer();
    // getBackgroundImage()
    bool getBuild();
    version_t getFileVersion() {return _file_version;}
    IntersectionVector& getStations() {return _stations;}
    IntersectionVector& getWaterlines() {return _waterlines;}
    IntersectionVector& getButtocks() {return _buttocks;}
    IntersectionVector& getDiagonals() {return _diagonals;}
    SubdivisionControlCurveVector& getControlCurves() {return _control_curves;}
    Flowline* getFlowline(size_t index);
    QString getFilename();
    HydrostaticCalcVector& getHydrostaticCalculations() {return _calculations;}
    size_t getNumberOfLayers();
    SubdivisionLayer* getLayer(size_t index);
    size_t getNumberOfFlowlines();
    size_t getNumberOfLockedPoints();
    size_t getNumberOfViewports();

    // marker
    bool isSelectedMarker(Marker* mark);
    void setSelectedMarker(Marker* mark);
    void removeSelectedMarker(Marker* mark);
    Marker* getMarker(size_t index);
    void deleteMarker(Marker* mark);
    size_t getNumberOfMarkers();

    // viewport? we might want to move this into the gui window class
    void addViewport(Viewport* vp);
    bool adjustMarkers();

    SubdivisionSurface* getSurface();

	void setFileChanged(bool changed);

	size_t numberOfHydrostaticCalculation();
	
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);
    void savePart(std::vector<SubdivisionFace*> faces);

    void rebuildModel();

	void clear();
    void clearUndo();

signals:
    void onFileChanged();
    void updateUndoData();
    void updateRecentFileList();
    void onChangeCursorIncrement();
    void onUpdateGeometryInfo();

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
    SubdivisionSurface* _surface;
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
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

