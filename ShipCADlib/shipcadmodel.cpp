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

#include <iostream>
#include <stdexcept>
#include <cstring>
#include "shipcadmodel.h"
#include "filebuffer.h"
#include "subdivsurface.h"
#include "undoobject.h"
#include "utility.h"
#include "subdivlayer.h"
#include "subdivface.h"
#include "flowline.h"
#include "viewport.h"
#include "subdivpoint.h"
#include "subdivedge.h"

using namespace std;
using namespace ShipCAD;

ShipCADModel::ShipCADModel()
    : _precision(fpLow), _file_version(k_current_version), _edit_mode(emSelectItems), _prefs(this),
      _active_control_point(0), _file_changed(false), _filename(""),
      _stations(true), _waterlines(true), _buttocks(true), _diagonals(true),
      _markers(true), _vis(this), _filename_set(false), _currently_moving(false),
      _stop_asking_for_file_version(false), _settings(this), _calculations(true),
      _design_hydrostatics(0), _undo_pos(0), _prev_undo_pos(0), _flowlines(true),
      _background_images(true)
{
	memset(&_delft_resistance, 0, sizeof(DelftSeriesResistance));
	memset(&_kaper_resistance, 0, sizeof(KAPERResistance));
}

void ShipCADModel::clear()
{
    _precision = fpLow;
    _file_version = k_current_version;
    _file_changed = false;
    _surface.clear();
    _filename = ShipCADModel::tr("New model");
    _vis.clear();
    _edit_mode = emSelectItems;
    _active_control_point = 0;
    _markers.clear();
    _stations.clear();
    _buttocks.clear();
    _waterlines.clear();
    _diagonals.clear();
    _calculations.clear();
    _settings.clear();
    _filename_set = false;
    _stop_asking_for_file_version = false;
    _selected_flowlines.clear();
    _flowlines.clear();
    _background_images.clear();
}

// FreeShipUnit.pas:11564
void ShipCADModel::buildValidFrameTable(SplineVector& dest, bool close_at_deck)
{
    float min = 0;
    IntersectionVector& stations = getStations();
    for (size_t i=0; i<stations.size(); ++i) {
        Intersection* intersection = stations.get(i);
        if (!intersection->isBuild())
            intersection->rebuild();
        SplineVector tmp(false);
        SplineVector& splines = intersection->getSplines();
        for (size_t j=0; j<splines.size(); j++) {
            Spline* spline = new Spline(*(splines.get(j)));
            // quick check to determine if the frame runs from bottom to top
            if (spline->value(0.0).z() > spline->value(1.0).z()) {
                // if not reverse the points
                spline->invert_direction();
            }
            tmp.add(spline);
        }
        // take all segments and join into one
        if (tmp.size() > 1)
            JoinSplineSegments(0.01, true, tmp);
        for (size_t j=0; j<tmp.size(); j++) {
            Spline* spline = tmp.get(j);
            if (close_at_deck) {
                if (spline->getLastPoint().y() != 0.0) {
                    QVector3D p = spline->getLastPoint();
                    p.setY(0.0);
                    spline->add(p);
                    spline->setKnuckle(spline->numberOfPoints() - 2, true);
                }
            }
            dest.add(spline);
            if (i == 0)
                min = spline->getMin().z();
            else if (spline->getMin().z() < min)
                min = spline->getMin().z();
        }
    }
    // now shift all stations up or down so that the lowest point
    // of all stations is on the baseline z=0.0
    if (min != 0.0) {
        for (size_t i=0; i<dest.size(); i++) {
            Spline* spline = dest.get(i);
            for (size_t j=0; j<spline->numberOfPoints(); j++) {
                QVector3D p = spline->getPoint(j);
                p.setZ(p.z() - min);
                spline->setPoint(j, p);
            }
        }
    }
}

// FreeShipUnit.pas:12648
void ShipCADModel::extents(QVector3D& min, QVector3D& max)
{
    if (_surface.numberOfControlFaces()) {
        _surface.setDrawMirror(true);
        _vis.setModelView(mvBoth);
        min = QVector3D(1e6, 1e6, 1e6);
        max = QVector3D(-1e6, -1e6, -1e6);
        _surface.extents(min, max);
        if (_vis.isShowMarkers()) {
            MarkerVectorIterator i = _markers.begin();
            for (; i!=_markers.end(); ++i)
                (*i)->extents(min, max);
        }
    } else {
        if (_surface.numberOfControlPoints() > 1) {
            SubdivisionControlPoint* p = _surface.getControlPoint(0);
            min = p->getCoordinate();
            max = p->getCoordinate();
            for (size_t i=1; i<_surface.numberOfControlPoints(); i++)
                MinMax(_surface.getControlPoint(i)->getCoordinate(), min, max);
        } else {
            min = QVector3D(-1, -1, -1);
            max = QVector3D(1, 1, 1);
        }
    }
}

void ShipCADModel::setBuild(bool set)
{
    _surface.setBuild(set);
    if (!set) {
        for (size_t i=0; i<getStations().size(); i++)
            getStations().get(i)->setBuild(false);
        for (size_t i=0; i<getButtocks().size(); i++)
            getButtocks().get(i)->setBuild(false);
        for (size_t i=0; i<getWaterlines().size(); i++)
            getWaterlines().get(i)->setBuild(false);
        for (size_t i=0; i<getDiagonals().size(); i++)
            getDiagonals().get(i)->setBuild(false);
        for (size_t i=0; i<getHydrostaticCalculations().size(); i++)
            getHydrostaticCalculations().get(i)->setCalculated(false);
        for (size_t i=0; i<_flowlines.size(); i++)
            _flowlines.get(i)->setBuild(false);
    }
}

void ShipCADModel::rebuildModel(bool redo_intersections)
{
    if (redo_intersections)
        setBuild(false);

    // get preferences settings
    _prefs.setSurfaceColors(_surface);
    _surface.setControlPointSize(_prefs.getPointSize());

    // get visibility settings
    Visibility& v = getVisibility();
    _surface.setShowControlNet(v.isShowControlNet());
    _surface.setShowInteriorEdges(v.isShowInteriorEdges());
    _surface.setDrawMirror(v.getModelView() == mvBoth);
    _surface.setShowNormals(v.isShowNormals());
    _surface.setShowCurvature(v.isShowCurvature());
    _surface.setShowControlCurves(v.isShowControlCurves());

    // project settings
    ProjectSettings& ps = getProjectSettings();
    _surface.setShadeUnderWater(ps.isShadeUnderwaterShip());
    _surface.setMainframeLocation(ps.getMainframeLocation());
    _surface.setUnderWaterColor(ps.getUnderWaterColor());
    if (ps.isShadeUnderwaterShip()) {
        Plane pln(0, 0, 1.0f, -(findLowestHydrostaticsPoint() + ps.getDraft()));
        _surface.setWaterlinePlane(pln);
    }
    _surface.setDesiredSubdivisionLevel(static_cast<int>(_precision)+1);
    _surface.rebuild();
}

void ShipCADModel::setPrecision(precision_t precision)
{
    if (_precision != precision) {
        _precision = precision;
        _surface.setDesiredSubdivisionLevel(static_cast<int>(_precision) + 1);
        setFileChanged(true);
        setBuild(false);
    }
}

void ShipCADModel::setEditMode(edit_mode_t mode)
{
	if (mode != _edit_mode) {
		_edit_mode = mode;
	}
}

size_t ShipCADModel::countSelectedItems() const
{
    size_t count = 0;
    const SubdivisionSurface* s = getSurface();
    count += s->numberOfSelectedControlCurves()
            + s->numberOfSelectedControlEdges()
            + s->numberOfSelectedControlFaces()
            + s->numberOfSelectedControlPoints()
            + _selected_flowlines.size()
            + _selected_markers.size();
    return count;
}

void ShipCADModel::clearSelectedItems()
{
    getSurface()->clearSelection();
    _selected_markers.clear();
    _selected_flowlines.clear();
}

QString ShipCADModel::getFilename() const
{
	if (_filename == "") {
        return ShipCADModel::tr("New model");
	}
	return ChangeFileExt(_filename, kFileExtension);
}

void ShipCADModel::setFilename(const QString& name)
{
    QString tmp(name);
    if (tmp.length() == 0) {
        tmp = ShipCADModel::tr("New model");
    }
    tmp = ChangeFileExt(tmp, kFileExtension);
    if (_filename != tmp) {
        _filename = tmp;
    }
    _filename_set = true;
}

// FreeShipUnit.pas:4595
UndoObject* ShipCADModel::createUndo(const QString& undotext, bool accept)
{
    version_t version = getFileVersion();
    bool preview = getProjectSettings().isSavePreview();
	UndoObject* rd = new UndoObject(this,
                                    getFilename(),
									getEditMode(),
                                    isFileChanged(),
                                    isFilenameSet(),
									false);
    rd->setUndoText(undotext);
    cout << "create undo:'" << undotext.toStdString() << "'" << endl;
    try {
        // delete all undo objects after the current one
        while (_undo_list.size() > _undo_pos + 1) {
            UndoObject* last = _undo_list.back();
            _undo_list.pop_back();
            delete last;
        }
        // temporarily set to the latest fileversion so no data will be lost
		setFileVersion(k_current_version);
        // temp disable saving of preview image
		getProjectSettings().setSavePreview(false);
		saveBinary(rd->getUndoData());
        if (accept) {
            rd->accept();
            emit undoDataChanged();
        }
    } catch(...) {
        // restore the original version
        setFileVersion(version);
        getProjectSettings().setSavePreview(preview);
		delete rd;
		rd = 0;
        cout << "*** create undo failed" << endl;
    }
    return rd;
}

// FreeShipUnit.pas:4564
UndoObject* ShipCADModel::createRedo()
{
	version_t version = getFileVersion();
    bool preview = getProjectSettings().isSavePreview();
    UndoObject* rd = new UndoObject(this,
                                    getFilename(),
									getEditMode(),
                                    isFileChanged(),
                                    isFilenameSet(),
									true);
    cout << "create redo" << endl;
	try {
        // temporarily set to the latest fileversion so no data will be lost
		setFileVersion(k_current_version);
        // temp disable saving of preview image
		getProjectSettings().setSavePreview(false);
		saveBinary(rd->getUndoData());
		rd->accept();
	} catch(...) {
		// restore the original version
		setFileVersion(version);
		getProjectSettings().setSavePreview(preview);
		delete rd;
		rd = 0;
        cout << "*** create redo failed" << endl;
	}
	return rd;
}

// called from the undo object
void ShipCADModel::acceptUndo(UndoObject* undo)
{
    if (_undo_list.size() > 0) {
        if (_undo_list.back()->isTempRedoObject()) {
            UndoObject* last = _undo_list.back();
            _undo_list.pop_back();
            delete last;
        }
    }
    // delete all undo objects after the current one
    while (_undo_list.size() > _undo_pos + 1) {
        UndoObject* last = _undo_list.back();
        _undo_list.pop_back();
        delete last;
    }
    _undo_list.push_back(undo);
    _undo_pos++;
    // remove objects from the front of the list until memory is within limits or we have 2 items
    while (((getUndoMemory() / (1024*1024)) > getPreferences().getMaxUndoMemory())
           && _undo_list.size() > 2) {
        UndoObject* first = _undo_list.front();
        _undo_list.pop_front();
        delete first;
        _undo_pos--;
        _prev_undo_pos--;
    }
    emit undoDataChanged();
    cout << "undo accepted" << endl;
    cout << "undo list:" << _undo_list.size() << " pos:" << _undo_pos << " prev_pos:" << _prev_undo_pos
         << " mem:" << getUndoMemory() << endl;
}

size_t ShipCADModel::getUndoMemory() const
{
	size_t mem_used = 0;
	for (size_t i=0; i<_undo_list.size(); i++)
		mem_used += _undo_list[i]->getMemory();
	return mem_used;
}

void ShipCADModel::undo()
{
    cout << "ShipCADModel::undo" << endl;
    if (_undo_list.size() > 0) {
        bool preview = getProjectSettings().isSavePreview();
        try {
            if (_undo_pos == _undo_list.size()) {
                if (!_undo_list.back()->isTempRedoObject())
                    createRedo();
            }
            if (_prev_undo_pos < _undo_pos)
                --_undo_pos;
            _prev_undo_pos = _undo_pos;
            --_undo_pos;
            UndoObject* uo = _undo_list[_undo_pos];
            uo->restore();
            emit undoDataChanged();
        } catch(...) {
            getProjectSettings().setSavePreview(preview);
            cout << "*** ShipCADModel::undo failed" << endl;
        }
        cout << "undo list:" << _undo_list.size() << " pos:" << _undo_pos << " prev_pos:"
             << _prev_undo_pos << " mem:" << getUndoMemory() << endl;
    }
}

void ShipCADModel::redo()
{
    cout << "ShipCADModel::redo" << endl;
    if (_undo_list.size() > 0) {
        bool preview = getProjectSettings().isSavePreview();
        try {
            if (_prev_undo_pos > _undo_pos)
                ++_undo_pos;
            _prev_undo_pos = _undo_pos;
            ++_undo_pos;
            UndoObject* uo = _undo_list[_undo_pos-1];
            uo->restore();
            emit undoDataChanged();
        } catch(...) {
            getProjectSettings().setSavePreview(preview);
            cout << "*** ShipCADModel::redo failed" << endl;
        }
        cout << "undo list:" << _undo_list.size() << " pos:" << _undo_pos << " prev_pos:"
             << _prev_undo_pos << " mem:" << getUndoMemory() << endl;
    }
}

bool ShipCADModel::canUndo() const
{
    return _undo_list.size() > 0 && _undo_pos > 0;
}

bool ShipCADModel::canRedo() const
{
    return _undo_list.size() > 0 && _undo_pos < _undo_list.size();
}

void ShipCADModel::clearUndo()
{
    for (size_t i=0; i<_undo_list.size(); i++)
        delete _undo_list[i];
    _undo_list.clear();
    _undo_pos = 0;
    _prev_undo_pos = 0;
    emit undoDataChanged();
}

void ShipCADModel::drawGrid(Viewport& /*vp*/, LineShader* /*lineshader*/)
{
    // TODO
}

struct draw_intersection
{
    Viewport& _vp;
    LineShader* _ls;
    draw_intersection(Viewport& vp, LineShader* lineshader)
        : _vp(vp), _ls(lineshader)
    {}
    void operator()(Intersection* itsection)
    {
        itsection->draw(_vp, _ls);
    }
};

void ShipCADModel::draw(Viewport& vp)
{
    LineShader* lineshader = vp.setLineShader();
    draw_intersection di(vp, lineshader);
    // draw intersection lines BEFORE the surface is drawn
    // so that the controlnet appears on top
    // but the intersections that should be drawn last depends on the view
    if (vp.getViewportType() != fvPerspective) {
        if (_vis.isShowGrid())
            drawGrid(vp, lineshader);
        else {
            if (vp.getViewportType() != fvBodyplan && _vis.isShowStations()) {
                for_each(_stations.begin(), _stations.end(), di);
            }
            if (vp.getViewportType() != fvProfile && _vis.isShowButtocks()) {
                for_each(_buttocks.begin(), _buttocks.end(), di);
            }
            if (vp.getViewportType() != fvPlan && _vis.isShowWaterlines()) {
                for_each(_waterlines.begin(), _waterlines.end(), di);
            }
            if (_vis.isShowDiagonals()) {
                for_each(_diagonals.begin(), _diagonals.end(), di);
            }
        }
        if (vp.getViewportType() == fvBodyplan && _vis.isShowStations()) {
            for_each(_stations.begin(), _stations.end(), di);
        }
        if (vp.getViewportType() == fvProfile && _vis.isShowButtocks()) {
            for_each(_buttocks.begin(), _buttocks.end(), di);
        }
        if (vp.getViewportType() == fvPlan && _vis.isShowWaterlines()) {
            for_each(_waterlines.begin(), _waterlines.end(), di);
        }
        if (vp.getViewportType() != fvBodyplan && _vis.isShowDiagonals()) {
            for_each(_diagonals.begin(), _diagonals.end(), di);
        }
    } else {
        if (_vis.isShowStations()) {
            for_each(_stations.begin(), _stations.end(), di);
        }
        if (_vis.isShowButtocks()) {
            for_each(_buttocks.begin(), _buttocks.end(), di);
        }
        if (_vis.isShowWaterlines()) {
            for_each(_waterlines.begin(), _waterlines.end(), di);
        }
        if (_vis.isShowDiagonals()) {
            for_each(_diagonals.begin(), _diagonals.end(), di);
        }
    }
    // draw markers
    if (_vis.isShowMarkers() && vp.getViewportMode() == vmWireFrame) {
        for (size_t i=0; i<_markers.size(); i++)
            _markers.get(i)->draw(vp, lineshader);
    }
    getSurface()->draw(vp);
    if (vp.getViewportType() != fvPerspective && vp.getViewportMode() != vmWireFrame && _vis.isShowGrid()) {
        // shaded viewport is a special case when visibility.drawgrid is true
        LineShader* lineshader1 = vp.setLineShader();
        draw_intersection di1(vp, lineshader1);
        if (_vis.isShowStations()) {
            for_each(_stations.begin(), _stations.end(), di);
        }
        if (_vis.isShowButtocks()) {
            for_each(_buttocks.begin(), _buttocks.end(), di);
        }
        if (_vis.isShowWaterlines()) {
            for_each(_waterlines.begin(), _waterlines.end(), di);
        }
        if (_vis.isShowDiagonals()) {
            for_each(_diagonals.begin(), _diagonals.end(), di);
        }
    }
    if (vp.getViewportMode() == vmWireFrame && _vis.isShowHydrostaticData()) {
        // TODO
    }
    if (_vis.isShowFlowlines()) {
        // TODO
    }
    if (vp.getViewportMode() == vmShadeGauss && getSurface()->numberOfControlFaces() > 0
            && (getSurface()->getMaxGausCurvature() - getSurface()->getMinGausCurvature()) > 1E-7) {
        // Draw legend with gaussian curvature values
        //TODO
    }
}

// FreeShipUnit.pas:9995
void ShipCADModel::scaleModel(const QVector3D& scale, bool override_lock, bool adjust_markers)
{
    for (size_t i=0; i<getSurface()->numberOfControlPoints(); i++) {
        SubdivisionControlPoint* pt = getSurface()->getControlPoint(i);
        if (!pt->isLocked() || override_lock)
            pt->setCoordinate(pt->getCoordinate() * scale);
    }
    // update mainparticulars
    ProjectSettings& settings = getProjectSettings();
    settings.setLength(abs(settings.getLength() * scale.x()));
    settings.setBeam(abs(settings.getBeam() * scale.y()));
    settings.setDraft(abs(settings.getDraft() * scale.z()));
    if (!settings.useDefaultMainframeLocation())
        settings.setMainframeLocation(settings.getMainframeLocation() * scale.x());
    // update markers
    if (adjust_markers) {
        MarkerVectorIterator itr = getMarkers().begin();
        for (; itr!=getMarkers().end(); ++itr) {
            for (size_t j=0; j<(*itr)->numberOfPoints(); j++) {
                (*itr)->setPoint(j, (*itr)->getPoint(j) * scale);
            }
        }
    }
    // update stations, buttocks, and waterlines
    IntersectionVector& stations = getStations();
    for (size_t i=0; i<stations.size(); i++)
        stations.get(i)->getPlane().setD(stations.get(i)->getPlane().d() * scale.x());
    IntersectionVector& waterlines = getWaterlines();
    for (size_t i=0; i<waterlines.size(); i++)
        waterlines.get(i)->getPlane().setD(waterlines.get(i)->getPlane().d() * scale.y());
    IntersectionVector& buttocks = getButtocks();
    for (size_t i=0; i<buttocks.size(); i++)
        buttocks.get(i)->getPlane().setD(buttocks.get(i)->getPlane().d() * scale.z());
    // reset any present hydrostatic calculations
    HydrostaticCalcVector& calc = getHydrostaticCalculations();
    for (size_t i=0; i<calc.size(); i++) {
        calc.get(i)->setDraft(abs(calc.get(i)->getDraft() * scale.z()));
        calc.get(i)->setTrim(abs(calc.get(i)->getTrim() * scale.z()));
        calc.get(i)->setCalculated(false);
    }
    // scale data used for KAPER series resistance calculations
    // TODO
    // scale data used for DELFT series resistance calculations
    // TODO
}

void ShipCADModel::moveFaces(vector<SubdivisionControlPoint*>& points,
                             const QVector3D& vec, bool adjust_markers)
{
    for (size_t i=0; i<points.size(); i++) {
        if (!points[i]->isLocked())
            points[i]->setCoordinate(points[i]->getCoordinate() + vec);
    }
    if (points.size() == getSurface()->numberOfControlPoints()) {
        // update main dimensions
        if (!getProjectSettings().useDefaultMainframeLocation())
            getProjectSettings().setMainframeLocation(
                        getProjectSettings().getMainframeLocation() + vec.x());
        // update stations buttocks and waterlines
        IntersectionVector& stations = getStations();
        for (size_t i=0; i<stations.size(); i++)
            stations.get(i)->getPlane().setD(stations.get(i)->getPlane().d() - vec.x());
        IntersectionVector& waterlines = getWaterlines();
        for (size_t i=0; i<waterlines.size(); i++)
            waterlines.get(i)->getPlane().setD(waterlines.get(i)->getPlane().d() - vec.y());
        IntersectionVector& buttocks = getButtocks();
        for (size_t i=0; i<buttocks.size(); i++)
            buttocks.get(i)->getPlane().setD(buttocks.get(i)->getPlane().d() - vec.z());
        if (adjust_markers) {
            MarkerVectorIterator itr = getMarkers().begin();
            for (; itr!=getMarkers().end(); ++itr) {
                for (size_t j=0; j<(*itr)->numberOfPoints(); j++) {
                    (*itr)->setPoint(j, (*itr)->getPoint(j) + vec);
                }
            }
        }
    }
}

// FreeShipUnit.pas:10540
Intersection* ShipCADModel::createIntersection(intersection_type_t ty, float distance)
{
    IntersectionVector* targetlist = 0;
    switch(ty) {
    case fiStation:
        targetlist = &_stations;
        break;
    case fiWaterline:
        targetlist = &_waterlines;
        break;
    case fiButtock:
        targetlist = &_buttocks;
        break;
    case fiDiagonal:
        targetlist = &_diagonals;
        break;
    case fiFree:
        return 0;
    }
    // check if an intersection already exists at this location
    for (size_t i=0; i<targetlist->size(); i++) {
        Intersection* inter = targetlist->get(i);
        if (abs(-(inter->getPlane().d()) - distance) < 1E-5) {
            // it exists, don't make a new one
            return 0;
        }
    }
    // add the new one
    Plane pln;
    switch(ty) {
    case fiStation:
        pln = Plane(1.0, 0, 0, -distance);
        break;
    case fiButtock:
        pln = Plane(0, 1.0, 0, -distance);
        break;
    case fiWaterline:
        pln = Plane(0, 0, 1.0, -distance);
        break;
    case fiDiagonal:
        pln = Plane(0, 1.0/sqrt(2.0), 1.0/sqrt(2.0), -(1.0/sqrt(2.0))*distance);
        break;
    case fiFree:
        return 0;
    }
    Intersection* result = new Intersection(this, ty, pln, false);
    result->rebuild();
    // only add if an intersection has been found
    if (result->getSplines().size() == 0) {
        delete result;
        return 0;
    }
    targetlist->add(result);
    return result;
}

void ShipCADModel::loadBinary(FileBuffer& source)
{
	// remember the filename because it is erased by the clear method
	QString tmpstr = _filename;
	clear();
	_filename = tmpstr;
	source.reset();
	QString hdr;
	source.load(hdr);
	if (hdr == "FREE!ship") {
		quint8 v, cver;
		source.load(v);
		_file_version = static_cast<version_t>(v);
		cver = static_cast<quint8>(k_current_version);
		source.setVersion(_file_version);
		if (_file_version <= cver) {
			quint32 n;
			source.load(n);
			_precision = static_cast<precision_t>(n);
			_vis.loadBinary(source);
			_settings.loadBinary(source, 0);
			// load the subdivision surface
            _surface.loadBinary(source);
			// stations
			source.load(n);
			for (size_t i=0; i<n; i++) {
                Intersection* intersection = new Intersection(this);
				intersection->loadBinary(source);
				_stations.add(intersection);
			}
			// buttocks
			source.load(n);
			for (size_t i=0; i<n; i++) {
                Intersection* intersection = new Intersection(this);
				intersection->loadBinary(source);
				_buttocks.add(intersection);
			}
			// waterlines
			source.load(n);
			for (size_t i=0; i<n; i++) {
                Intersection* intersection = new Intersection(this);
				intersection->loadBinary(source);
				_waterlines.add(intersection);
			}
			if (_file_version >= fv180) {
				// diagonals
				source.load(n);
				for (size_t i=0; i<n; i++) {
                    Intersection* intersection = new Intersection(this);
					intersection->loadBinary(source);
					_diagonals.add(intersection);
				}
				if (_file_version >= fv191) {
					// markers
                    source.load(n);
                    for (size_t i=0; i<n; i++) {
                        Marker* marker = new Marker(this);
                        marker->loadBinary(source);
                        _markers.add(marker);
                    }
                    if (_file_version >= fv210) {
                        source.load(&_delft_resistance);
                        source.load(&_kaper_resistance);
                        if (_file_version >= fv250) {
                            source.load(n);
                            for (size_t i=0; i<n; i++) {
                                BackgroundImage* img = new BackgroundImage(this);
                                _background_images.add(img);
                                img->loadBinary(source);
                            }
                            source.load(n);
                            for (size_t i=0; i<n; i++) {
                                Flowline* flow = Flowline::construct(this);
                                _flowlines.add(flow);
                                flow->loadBinary(source);
                            }
                        }
                    }
                }
            }
        }
        else {
            // TODO version is later than this can handle
        }
    }
    else {
        // TODO this is not a free ship binary file
    }
    _file_changed = false;
    rebuildModel(false);
}

void ShipCADModel::saveBinary(FileBuffer& dest)
{
    dest.add("FREE!ship");
    dest.add(static_cast<quint8>(_file_version));
    dest.add(static_cast<quint32>(_precision));
    _vis.saveBinary(dest);      // starting at offset hex 76 freeship and ours differ
    _settings.saveBinary(dest);
    _surface.saveBinary(dest);
    // save stations
    dest.add(_stations.size());
    for (size_t i=0; i<_stations.size(); i++)
        _stations.get(i)->saveBinary(dest);
    // save buttocks
    dest.add(_buttocks.size());
    for (size_t i=0; i<_buttocks.size(); i++)
        _buttocks.get(i)->saveBinary(dest);
    // save waterlines
    dest.add(_waterlines.size());
    for (size_t i=0; i<_waterlines.size(); i++)
        _waterlines.get(i)->saveBinary(dest);
    if (getFileVersion() >= fv180) {
        // save diagonals
        dest.add(_diagonals.size());
        for (size_t i=0; i<_diagonals.size(); i++)
            _diagonals.get(i)->saveBinary(dest);
        if (getFileVersion() >= fv191) {
            // save markers
            dest.add(_markers.size());
            for (size_t i=0; i<_markers.size(); i++)
                _markers.get(i)->saveBinary(dest);
            if (getFileVersion() >= fv210) {
                dest.add(&_delft_resistance);
                dest.add(&_kaper_resistance);
                if (getFileVersion() >= fv250) {
                    dest.add(_background_images.size());
                    for (size_t i=0; i<_background_images.size(); i++)
                        _background_images.get(i)->saveBinary(dest);
                    dest.add(_flowlines.size());
                    for (size_t i=0; i<_flowlines.size(); i++)
                        _flowlines.get(i)->saveBinary(dest);
                }
            }
        }
    }
}

float ShipCADModel::findLowestHydrostaticsPoint() const
{
    float result = _surface.getMin().z();
    bool first = true;
    for (size_t i=0; i<_surface.numberOfLayers(); i++) {
        const SubdivisionLayer* layer = _surface.getLayer(i);
        if (layer->useInHydrostatics()) {
            for (size_t j=0; j<layer->numberOfFaces(); j++) {
                if (first) {
                    result = layer->getFace(j)->getMin().z();
                    first = false;
                } else if (layer->getFace(j)->getMin().z() < result) {
                    result = layer->getFace(j)->getMin().z();
                }
            }
        }
    }
    return result;
}

void ShipCADModel::setFileVersion(version_t v)
{
    if (v != _file_version) {
        _file_version = v;
        setFileChanged(true);
    }
}

bool ShipCADModel::isSelectedMarker(Marker* mark) const
{
    return find(_selected_markers.begin(),
                _selected_markers.end(), mark) != _selected_markers.end();
}

void ShipCADModel::setSelectedMarker(Marker* mark)
{
    _selected_markers.insert(mark);
}

void ShipCADModel::removeSelectedMarker(Marker* mark)
{
    set<Marker*>::iterator i = find(
        _selected_markers.begin(),
        _selected_markers.end(), mark);
    if (i != _selected_markers.end())
        _selected_markers.erase(i);
}

Marker* ShipCADModel::addMarker()
{
    Marker *mark = Marker::construct(this);
    _markers.add(mark);
    return mark;
}

void ShipCADModel::deleteMarker(Marker* mark)
{
    removeSelectedMarker(mark);
    _markers.del(mark);
}

bool ShipCADModel::isSelectedFlowline(Flowline* flow) const
{
    return find(_selected_flowlines.begin(), _selected_flowlines.end(), flow) !=
        _selected_flowlines.end();
}

void ShipCADModel::setSelectedFlowline(Flowline* flow)
{
    _selected_flowlines.insert(flow);
}

void ShipCADModel::removeSelectedFlowline(Flowline* flow)
{
    set<Flowline*>::iterator i = _selected_flowlines.find(flow);
    if (i != _selected_flowlines.end())
        _selected_flowlines.erase(i);
}

void ShipCADModel::deleteFlowline(Flowline* flow)
{
    removeSelectedFlowline(flow);
    _flowlines.del(flow);
}

void ShipCADModel::deleteSelected()
{
    getSurface()->deleteSelected();
    set<Marker*>::iterator i = _selected_markers.begin();
    for (; i!=_selected_markers.end(); ++i)
        deleteMarker(*i);
    set<Flowline*>::iterator j = _selected_flowlines.begin();
    for(; j!=_selected_flowlines.end(); ++j)
        deleteFlowline(*j);
}

float new_model_pts[] = {
    // station 0, stern
    0.00000,0.00000,1.56754,
    0.00000,0.05280,1.59170,
    0.00000,0.22171,1.77284,
    0.00000,0.28506,2.64108,
    0.00000,0.29135,3.48932,
    // station 1
    0.20880,0.00000,0.49656,
    0.20881,0.18796,0.53622,
    0.20880,0.33700,0.97840,
    0.20880,0.45607,2.05422,
    0.20882,0.47184,3.44280,
    // station 2
    0.41765,0.00000,0.00000,
    0.41765,0.23565,0.07524,
    0.41765,0.41555,0.67735,
    0.41765,0.49421,1.91004,
    0.41737,0.51468,3.45474,
    // station 3
    0.58471,0.00000,0.00000,
    0.58472,0.24072,0.02507,
    0.58472,0.39528,0.71080,
    0.58488,0.45356,2.04881,
    0.58472,0.46756,3.54662,
    // station 4
    0.75179,0.00000,0.28284,
    0.75178,0.13715,0.44098,
    0.75179,0.20950,0.87760,
    0.75179,0.30538,2.38232,
    0.75177,0.34473,3.67786,
    // station 5
    0.90672,0.00000,0.81860,
    0.90681,0.01887,0.98650,
    0.90658,0.04671,1.29873,
    0.90637,0.11195,2.83107,
    0.90672,0.14523,3.81697,
    // station 6, stem
    0.91580,0.00000,0.85643,
    0.92562,0.00000,1.17444,
    0.93387,0.00000,1.44618,
    0.97668,0.00000,3.03482,
    1.00000,0.00000,3.91366
};

void ShipCADModel::newModel(unit_type_t units,
                            float length, float breadth, float draft,
                            size_t rows, size_t cols)
{
    clear();

    _settings.setLength(length);
    _settings.setBeam(breadth);
    _settings.setDraft(draft);
    _settings.setUnits(units);
    
    // create temporary splines
    SplineVector trvsplines(true);
    // first create temporary splines in transverse direction
    for (size_t i=0; i<7; i++) {
        Spline* spline = new Spline();
        for (size_t j=0; j<5; j++) {
            QVector3D p(new_model_pts[i*15+j*3],
                    new_model_pts[i*15+j*3+1],
                    new_model_pts[i*15+j*3+2]);
            p.setX(p.x() * length);
            p.setY(p.y() * breadth);
            p.setZ(p.z() * draft);
            spline->add(p);
        }
        trvsplines.add(spline);
    }
    // now create temporary splines in longitudinal direction
    vector<vector<SubdivisionControlPoint*> > pts;
    SubdivisionControlPoint* stem_point = 0;
    for (size_t i=0; i<=rows; i++) {
        Spline spline2;
        for (size_t j=0; j<trvsplines.size(); j++) {
            Spline spline1(*(trvsplines.get(j)));
            QVector3D p(spline1.value(i/static_cast<float>(rows)));
            spline2.add(p);
        }
        // now calculate all points on the longitudinal spline and send to the surface
        vector<SubdivisionControlPoint*> row;
        for (size_t j=0; j<=cols; j++) {
            QVector3D p(spline2.value(j/static_cast<float>(cols)));
            SubdivisionControlPoint* pt = getSurface()->addControlPoint(p);
            row.push_back(pt);
            if (i == 0 && j == cols)
                stem_point = pt;
        }
        pts.push_back(row);
    }
    // finally create the controlfaces over the newly calculate points
    for (size_t i=1; i<=rows; i++) {
        for (size_t j=1; j<=cols; j++) {
            vector<SubdivisionControlPoint*> tmppoints;
            tmppoints.push_back(pts[i][j-1]);
            tmppoints.push_back(pts[i][j]);
            tmppoints.push_back(pts[i-1][j]);
            tmppoints.push_back(pts[i-1][j-1]);
            getSurface()->addControlFace(tmppoints, true);
        }
    }
    setPrecision(fpMedium);
    getSurface()->initialize(1,1);
    // collapse stempoint
    if (stem_point != 0 && stem_point->getVertexType() == svCorner)
        stem_point->setVertexType(svCrease);
    QVector3D min;
    QVector3D max;
    getSurface()->extents(min, max);
    // add 21 stations
    for (size_t i=0; i<21; i++)
        createIntersection(fiStation, i/20.0*(max.x() - min.x()));
    // add 7 buttocks
    for (size_t i=0; i<7; i++)
        createIntersection(fiButtock, i/7.0*(max.y() - min.y()));
    // add 11 waterlines
    for (size_t i=0; i<11; i++)
        createIntersection(fiWaterline, i/10.0*(max.z() - min.z()));
    _file_changed = false;
    rebuildModel(false);
    if (_stations.size() > 0)
        _vis.setShowStations(true);
    if (_waterlines.size() > 0)
        _vis.setShowWaterlines(true);
    if (_buttocks.size() > 0)
        _vis.setShowButtocks(true);
}

// FreeShipUnit.pas:13166
void ShipCADModel::savePart(const QString& filename, FileBuffer& buffer,
                            vector<SubdivisionControlFace*>& faces)
{
    set<SubdivisionLayer*> layers;
    set<SubdivisionControlPoint*> points;
    set<SubdivisionControlEdge*> edges;
    set<SubdivisionControlCurve*> curves;

    // extract controlpoints and controledges
    for (vector<SubdivisionControlFace*>::iterator i=faces.begin(); i!=faces.end(); ++i) {
        SubdivisionControlFace* face = *i;
        layers.insert(face->getLayer());
        SubdivisionControlPoint* p1 = dynamic_cast<SubdivisionControlPoint*>(face->getLastPoint());
        points.insert(p1);
        for (size_t j=0; j<face->numberOfPoints(); j++) {
            SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
            points.insert(p2);
            SubdivisionControlEdge* edge = _surface.controlEdgeExists(p1, p2);
            if (edge != nullptr)
                edges.insert(edge);
            p1 = p2;
        }
    }

    // process control curves
    for (size_t i=0; i<_surface.numberOfControlCurves(); ++i) {
        SubdivisionControlCurve* curve = _surface.getControlCurve(i);
        // in order to export this curve, all associated control edges must be in the edge list
        bool addcurve = curve->numberOfControlPoints() > 1;
        for (size_t j=1; j<curve->numberOfControlPoints(); j++) {
            SubdivisionControlPoint* p1 = curve->getControlPoint(j-1);
            SubdivisionControlPoint* p2 = curve->getControlPoint(j);
            SubdivisionControlEdge* edge = _surface.controlEdgeExists(p1, p2);
            if ((edge != nullptr && edges.find(edge) == edges.end()) || edge == nullptr)
                addcurve = false;
        }
        if (addcurve)
            curves.insert(curve);
    }

    // sort the curves
    vector<SubdivisionControlCurve*> curvelist(curves.begin(), curves.end());
    sort(curvelist.begin(), curvelist.end());
    vector<SubdivisionLayer*> layerlist(layers.begin(), layers.end());
    sort(layerlist.begin(), layerlist.end());

    // write the file
    buffer.add("FREE!ship partfile");
    // file version
    buffer.add(_file_version);
    // project units
    buffer.add(static_cast<quint32>(_settings.getUnits()));
    // number of layers
    buffer.add(layerlist.size());
    // the layers
    for (size_t i=0; i<layerlist.size(); i++)
        layerlist[i]->saveBinary(buffer);
    // controlpoints
    buffer.add(points.size());
    map<SubdivisionControlPoint*,size_t> indexer;
    size_t k=0;
    for (set<SubdivisionControlPoint*>::iterator itr=points.begin(); itr!=points.end(); ++itr) {
        (*itr)->save_binary(buffer);
        indexer.insert(make_pair((*itr), k++));
    }
    // save control edges
    buffer.add(edges.size());
    for (set<SubdivisionControlEdge*>::iterator eitr=edges.begin(); eitr!=edges.end(); ++eitr) {
        SubdivisionControlEdge* edge = *eitr;
        buffer.add(indexer.find(dynamic_cast<SubdivisionControlPoint*>(edge->startPoint()))->second);
        buffer.add(indexer.find(dynamic_cast<SubdivisionControlPoint*>(edge->endPoint()))->second);
        buffer.add(edge->isCrease());
    }
    // save control faces
    buffer.add(faces.size());
    for (vector<SubdivisionControlFace*>::iterator fitr=faces.begin(); fitr!=faces.end(); ++fitr) {
        SubdivisionControlFace* face = *fitr;
        buffer.add(face->numberOfPoints());
        for (size_t j=0; j<face->numberOfPoints(); j++)
            buffer.add(indexer.find(dynamic_cast<SubdivisionControlPoint*>(
                                        face->getPoint(j)))->second);
        vector<SubdivisionLayer*>::iterator l = find(layerlist.begin(), layerlist.end(),
                                                     face->getLayer());
        if (l == layerlist.end())
            throw runtime_error("Didn't find layer in ShipCADModel::savePart");
        buffer.add(static_cast<size_t>(l-layerlist.begin()));
    }
    // save control curves
    buffer.add(curvelist.size());
    for (size_t i=0; i<curvelist.size(); i++) {
        SubdivisionControlCurve* curve = curvelist[i];
        buffer.add(curve->numberOfControlPoints());
        for (size_t j=0; j<curve->numberOfControlPoints(); j++)
            buffer.add(indexer.find(curve->getControlPoint(j))->second);
    }
    buffer.add(ChangeFileExt(filename, ".part"));
}

// FreeShipUnit.pas:7708
bool ShipCADModel::loadPart(FileBuffer& source, version_t& partversion)
{
    float scale = 1.0;
    bool changed = false;       // have we actually loaded something?
    quint32 n;
    SubdivisionControlPoint* p1;
    SubdivisionControlPoint* p2;
    SubdivisionControlEdge* edge;
    SubdivisionLayer* layer;
	source.reset();
	QString hdr;
	source.load(hdr);
	if (hdr == "FREE!ship partfile") {
        source.load(n);
        partversion = static_cast<version_t>(n);
		source.setVersion(partversion);
        if (partversion <= k_current_version) {
            vector<SubdivisionControlPoint*> points;
            vector<SubdivisionControlEdge*> edges;
            vector<SubdivisionLayer*> layers;
            // load units
            source.load(n);
            unit_type_t units = static_cast<unit_type_t>(n);
            if (units != _settings.getUnits()) {
                scale = (units == fuMetric) ? 1 / kFoot : kFoot;
            }
            // load number of layers
            source.load(n);
            // load layers
            for (size_t i=0; i<n; i++) {
                layer = getSurface()->addNewLayer();
                size_t lid = layer->getLayerID();
                layer->loadBinary(source);
                layer->setLayerID(lid);
                layers.push_back(layer);
                changed = true;
            }
            // control points
            source.load(n);
            for (size_t i=0; i<n; i++) {
                p2 = getSurface()->addControlPoint();
                p2->load_binary(source);
                p2->setCoordinate(p2->getCoordinate() * scale);
                points.push_back(p2);
                changed = true;
            }
            // control edges
            source.load(n);
            for (size_t i=0; i<n; i++) {
                quint32 index;
                p1 = nullptr;
                p2 = nullptr;
                edge = nullptr;
                source.load(index);
                if (index < points.size())
                    p1 = points[index];
                source.load(index);
                if (index < points.size())
                    p2 = points[index];
                if (p1 != nullptr && p2 != nullptr)
                    edge = getSurface()->addControlEdge(p1, p2);
                bool crease;
                source.load(crease);
                if (edge != nullptr) {
                    edge->setCrease(crease);
                    edges.push_back(edge);
                }
                changed = true;
            }
            // controlfaces
            source.load(n);
            for (size_t i=0; i<n; i++) {
                quint32 np;
                layer = nullptr;
                source.load(np);
                vector<SubdivisionControlPoint*> fpoints;
                for (size_t j=0; j<np; j++) {
                    quint32 index;
                    p1 = nullptr;
                    source.load(index);
                    if (index < points.size())
                        p1 = points[index];
                    if (p1 != nullptr)
                        fpoints.push_back(p1);
                }
                quint32 layerid;
                source.load(layerid);
                if (layerid < layers.size())
                    layer = layers[layerid];
                if (fpoints.size() == np && layer != nullptr) {
                    getSurface()->addControlFace(fpoints, false, layer);
                }
            }
            // reset crease on edges
            for (size_t i=0; i<edges.size(); i++) {
                if (edges[i]->numberOfFaces() != 2 && !edges[i]->isCrease())
                    edges[i]->setCrease(true);
            }
            // controlcurves
            source.load(n);
            for (size_t i=0; i<n; i++) {
                quint32 np;
                source.load(np);
                if (np > 1) {
                    SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(getSurface());
                    getSurface()->addControlCurve(curve);
                    for (size_t j=0; j<np; j++) {
                        quint32 index;
                        p2 = nullptr;
                        source.load(index);
                        if (index < points.size()) {
                            p2 = points[index];
                            curve->addPoint(p2);
                        }
                        if (j > 0) {
                            p1 = curve->getControlPoint(j-1);
                            edge = getSurface()->controlEdgeExists(p1, p2);
                            if (edge != nullptr)
                                edge->setCurve(curve);
                        }
                    }
                }
            }
        }
    }
    return changed;
}
