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

using namespace std;
using namespace ShipCAD;

ShipCADModel::ShipCADModel()
    : _precision(fpLow), _file_version(k_current_version), _edit_mode(emSelectItems), _prefs(this),
      _active_control_point(0), _file_changed(false), _filename(""),
      _stations(true), _waterlines(true), _buttocks(true), _diagonals(true), _control_curves(true),
      _markers(true), _vis(this), _filename_set(false), _currently_moving(false),
      _stop_asking_for_file_version(false), _settings(this), _calculations(true),
      _design_hydrostatics(0), _undo_pos(0), _prev_undo_pos(0), _flowlines(true),
      _background_images(true)
{
	memset(&_delft_resistance, 0, sizeof(DelftSeriesResistance));
	memset(&_kaper_resistance, 0, sizeof(KAPERResistance));
	connect(&_surface, SIGNAL(changedLayerData()), SIGNAL(changedLayerData()));
	connect(&_surface, SIGNAL(changeActiveLayer()), SIGNAL(changeActiveLayer()));
}

ShipCADModel::~ShipCADModel()
{
    // does nothing
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
    clearUndo();
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

void ShipCADModel::buildValidFrameTable(bool /*close_at_deck*/)
{
    // TODO
}

void ShipCADModel::extents(QVector3D& min, QVector3D& max)
{
    if (_surface.numberOfControlFaces()) {
        _surface.setDrawMirror(true);
        _vis.setModelView(mvBoth);
        min = QVector3D(1e6, 1e6, 1e6);
        max = QVector3D(-1e6, -1e6, -1e6);
        _surface.extents(min, max);
        if (_vis.isShowMarkers()) {
            // TODO extents on markers
        }
    } else {
        // TODO iterate on control points
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
        // TODO flowlines
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

    emit onUpdateGeometryInfo();
}

void ShipCADModel::setPrecision(precision_t precision)
{
    if (_precision != precision) {
        _precision = precision;
        _surface.setDesiredSubdivisionLevel(static_cast<int>(_precision) + 1);
        setFileChanged(true);
        setBuild(false);
        emit onUpdateGeometryInfo();
    }
}

void ShipCADModel::setEditMode(edit_mode_t mode)
{
	if (mode != _edit_mode) {
		_edit_mode = mode;
        emit onUpdateGeometryInfo();
	}
}

size_t ShipCADModel::countSelectedItems()
{
    size_t count = 0;
    SubdivisionSurface* s = getSurface();
    count += s->numberOfSelectedControlCurves()
            + s->numberOfSelectedControlEdges()
            + s->numberOfSelectedControlFaces()
            + s->numberOfSelectedControlPoints();
    return count;
}

void ShipCADModel::clearSelectedItems()
{
    getSurface()->clearSelection();
    setFileChanged(true);
}

void ShipCADModel::setFileChanged(bool set)
{
	if (set != _file_changed) {
		_file_changed = set;
		emit onFileChanged();
	}
}

QString ShipCADModel::getFilename()
{
	if (_filename == "") {
        return ShipCADModel::tr("New model");
	}
	return ChangeFileExt(_filename, kFileExtension);
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

size_t ShipCADModel::getUndoMemory()
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
            UndoObject* uo = _undo_list[_undo_pos];
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
    emit onUpdateGeometryInfo();
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

float ShipCADModel::findLowestHydrostaticsPoint()
{
    float result = _surface.getMin().z();
    bool first = true;
    for (size_t i=0; i<numberOfLayers(); i++) {
        SubdivisionLayer* layer = getLayer(i);
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

void ShipCADModel::redraw()
{
    emit onUpdateGeometryInfo();
}

void ShipCADModel::setFileVersion(version_t v)
{
    if (v != _file_version) {
        _file_version = v;
        setFileChanged(true);
    }
}

bool ShipCADModel::isSelectedMarker(Marker* mark)
{
    return find(_selected_markers.begin(),
                _selected_markers.end(), mark) != _selected_markers.end();
}

void ShipCADModel::setSelectedMarker(Marker* mark)
{
    if (!isSelectedMarker(mark))
        _selected_markers.push_back(mark);
}

void ShipCADModel::removeSelectedMarker(Marker* mark)
{
    vector<Marker*>::iterator i = find(
        _selected_markers.begin(),
        _selected_markers.end(), mark);
    if (i != _selected_markers.end())
        _selected_markers.erase(i);
}

// TODO this is a dialog, doesn't belong here
bool ShipCADModel::adjustMarkers()
{
    return false;
}

bool ShipCADModel::isSelectedFlowline(const Flowline* flow) const
{
    return find(_selected_flowlines.begin(), _selected_flowlines.end(), flow) !=
        _selected_flowlines.end();
}

void ShipCADModel::setSelectedFlowline(const Flowline* flow)
{
    if (!isSelectedFlowline(flow))
        _selected_flowlines.push_back(flow);
}

void ShipCADModel::removeSelectedFlowline(const Flowline* flow)
{
    vector<const Flowline*>::iterator i = find(_selected_flowlines.begin(),
                                               _selected_flowlines.end(),
                                               flow);
    if (i != _selected_flowlines.end())
        _selected_flowlines.erase(i);
}

void ShipCADModel::deleteSelected()
{
    // BUGBUG markers, flowlines
    getSurface()->deleteSelected();
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
    emit onUpdateGeometryInfo();
}


