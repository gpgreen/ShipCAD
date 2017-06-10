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
#include <Eigen/Dense>

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
#include "pointgrid.h"
#include "exception.h"

using namespace std;
using namespace ShipCAD;
using namespace Eigen;

ShipCADModel::ShipCADModel()
    : _precision(fpLow), _file_version(k_current_version), _edit_mode(emSelectItems), _prefs(this),
      _active_control_point(0), _file_changed(false), _filename(""),
      _stations(true), _waterlines(true), _buttocks(true), _diagonals(true),
      _markers(true), _vis(this), _filename_set(false), _currently_moving(false),
      _stop_asking_for_file_version(false), _settings(this), _calculations(true),
      _design_hydrostatics(0), _undo_pos(0), _prev_undo_pos(-1), _flowlines(true),
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
        // save the current model view setting, we temporarily set it to both
        TempVarChange<model_view_t> visview = _vis.tempChangeModelView(mvBoth);
        TempVarChange<bool> mirror = _surface.tempChangeMirror(true);
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
         << " mem:" << getUndoMemory() << "\n***" << endl;
    for (size_t i=0; i<_undo_list.size(); i++)
        cout << i << " " << _undo_list[i]->getUndoText().toStdString() << endl;
    cout << "***" << endl;
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
            if (_prev_undo_pos < static_cast<int>(_undo_pos))
                --_undo_pos;
            _prev_undo_pos = static_cast<int>(_undo_pos);
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
            if (_prev_undo_pos > static_cast<int>(_undo_pos))
                ++_undo_pos;
            _prev_undo_pos = static_cast<int>(_undo_pos);
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
    _prev_undo_pos = -1;
    emit undoDataChanged();
}

void ShipCADModel::drawWithPainter(Viewport& vp, QPainter* painter)
{
    if (vp.getViewportType() != fvPerspective && _vis.isShowGrid())
        drawGrid(vp, painter);
}

// FreeShipUnit.pas:12241
void ShipCADModel::drawGrid(Viewport& vp, QPainter* painter)
{
    QVector3D min;
    QVector3D max;
    extents(min, max);
    
    bool draw_stations = vp.getViewportType() != fvBodyplan;
    bool draw_buttocks = vp.getViewportType() != fvProfile;
    bool draw_waterlines = vp.getViewportType() != fvPlan;
    bool draw_diagonals = vp.getViewportType() == fvBodyplan;

    // blowup the boundary box by 3%
    QVector3D diff = (max - min) * 0.03;
    min -= diff;
    max += diff;
    
    QPen linepen1(_prefs.getGridColor(), 1);
    QPen linepen2(_prefs.getGridColor(), 2);
    QPen textpen(_prefs.getGridFontColor());
    QPen textredpen(Qt::red);
    QFont font("Arial", 8);
    QFontMetrics fm(font);
    painter->setFont(font);
    if (draw_stations || draw_buttocks || draw_waterlines || draw_diagonals) {
        if (vp.getViewportType() != fvProfile) {
            // draw centerline
            QVector3D p1(min);
            QVector3D p2(max);
            p1.setY(0.0);
            p2.setY(0.0);
            QPoint pt1 = vp.convert3D(p1);
            QPoint pt2 = vp.convert3D(p2);
            painter->setPen(linepen2);
            painter->drawLine(pt1, pt2);
            painter->setPen(textredpen);
            // msg 183
            QString msg(tr("Centerline"));
            int width = fm.width(msg);
            if (vp.getViewportType() == fvBodyplan) {
                painter->drawText(pt1.x() - (width / 2), pt1.y(), msg);
                painter->drawText(pt2.x() - (width / 2), pt2.y(), msg);
            } else {
                painter->drawText(pt1.x() - width, pt1.y(), msg);
                painter->drawText(pt2.x(), pt2.y(), msg);
            }
        }
        if (vp.getViewportType() != fvPlan) {
            // draw baseline
            QVector3D p1(min);
            QVector3D p2(max);
            float position = getSurface()->getMin().z();
            p1.setZ(position);
            p2.setZ(p1.z());
            QPoint pt1 = vp.convert3D(p1);
            QPoint pt2 = vp.convert3D(p2);
            painter->setPen(linepen2);
            painter->drawLine(pt1, pt2);
            painter->setPen(textredpen);
            // msg 184
            QString msg(tr("Base %1").arg(ConvertDimension(position, _settings.getUnits())));
            int width = fm.width(msg);
            painter->drawText(pt1.x(), pt1.y(), msg);
            painter->drawText(pt2.x()-width, pt2.y(), msg);
            // draw dwl
            if (_settings.isMainParticularsSet()) {
                position = getSurface()->getMin().z() + _settings.getDraft();
                p1.setZ(position);
                p2.setZ(p1.z());
                pt1 = vp.convert3D(p1);
                pt2 = vp.convert3D(p2);
                painter->setPen(linepen2);
                painter->drawLine(pt1, pt2);
                painter->setPen(textredpen);
                // msg 185
                QString msg(tr("DWL %1").arg(ConvertDimension(position, _settings.getUnits())));
                int width = fm.width(msg);
                painter->drawText(pt1.x() - width / 2, pt1.y(), msg);
                painter->drawText(pt2.x() - width / 2, pt2.y(), msg);
            }
        }
    }
    if (draw_stations) {
        QVector3D p1 = min;
        QVector3D p2 = max;
        for (size_t i=0; i<_stations.size(); ++i) {
            float position = -_stations.get(i)->getPlane().d();
            p1.setX(position);
            p2.setX(p1.x());
            QString str = ConvertDimension(position, _settings.getUnits());
            QPoint pt1 = vp.convert3D(p1);
            QPoint pt2 = vp.convert3D(p2);
            painter->setPen(linepen1);
            painter->drawLine(pt1, pt2);
            //int width = fm.width(str);
            int height = fm.height();
            painter->setPen(textpen);
            painter->drawText(pt1.x(), pt1.y() + height, str);
            painter->drawText(pt2.x(), pt2.y(), str);
        }
    }
    if (draw_diagonals) {
        QVector3D p1 = min;
        QVector3D p2 = max;
        for (size_t i=0; i<_diagonals.size(); ++i) {
            Intersection* diagonal = _diagonals.get(i);
            if (!diagonal->isBuild())
                diagonal->rebuild();
            for (size_t j=0; j<diagonal->getSplines().size(); ++j) {
                Spline* sp = diagonal->getSplines().get(j);
                p1 = sp->value(0 / 100.0);
                QPoint pt1 = vp.convert3D(p1);
                for (size_t n=1; n<=100; ++n) {
                    p2 = sp->value(n / 100.0);
                    QPoint pt2 = vp.convert3D(p2);
                    painter->setPen(linepen1);
                    painter->drawLine(pt1, pt2);
                    pt1 = pt2;
                }
                if (_vis.getModelView() == mvBoth || vp.getViewportType() == fvBodyplan) {
                    p1 = sp->value(0 / 100.0);
                    p1.setY(-p1.y());
                    QPoint pt1 = vp.convert3D(p1);
                    for (size_t n=1; n<=100; ++n) {
                        p2 = sp->value(n / 100.0);
                        p2.setY(-p2.y());
                        QPoint pt2 = vp.convert3D(p2);
                        painter->setPen(linepen1);
                        painter->drawLine(pt1, pt2);
                        pt1 = pt2;
                    }
                }
            }

        }
    }
    if (draw_buttocks) {
        QVector3D p1 = min;
        QVector3D p2 = max;
        for (size_t i=0; i<_buttocks.size(); ++i) {
            float position = -_buttocks.get(i)->getPlane().d();
            QString str = ConvertDimension(position, _settings.getUnits());
            p1.setY(position);
            p2.setY(p1.y());
            QPoint pt1 = vp.convert3D(p1);
            QPoint pt2 = vp.convert3D(p2);
            painter->setPen(linepen1);
            painter->drawLine(pt1, pt2);
            int width = vp.getViewportType() == fvBodyplan ? 0 : fm.width(str);
            painter->setPen(textpen);
            if (vp.getViewportType() == fvBodyplan) {
                painter->drawText(pt1.x(), pt1.y(), str);
                painter->drawText(pt2.x() - width, pt2.y(), str);
            } else {
                painter->drawText(pt1.x(), pt1.y(), str);
                painter->drawText(pt2.x() - width, pt2.y(), str);
            }
            if (_vis.getModelView() == mvBoth || vp.getViewportType() == fvBodyplan) {
                p1.setY(-position);
                p2.setY(p1.y());
                QString str1 = ConvertDimension(-position, _settings.getUnits());
                pt1 = vp.convert3D(p1);
                pt2 = vp.convert3D(p2);
                painter->setPen(linepen1);
                painter->drawLine(pt1, pt2);
                int width = fm.width(str);
                int height = fm.height();
                painter->setPen(textpen);
                if (vp.getViewportType() == fvBodyplan) {
                    painter->drawText(pt1.x() - width, pt1.y(), str1);
                    painter->drawText(pt2.x() - width, pt2.y(), str1);
                } else {
                    painter->drawText(pt2.x() - width, pt2.y() + height, str1);
                    painter->drawText(pt1.x(), pt1.y() + height, str1);
                }
            }
        }
    }
    if (draw_waterlines) {
        QVector3D p1 = min;
        QVector3D p2 = max;
        for (size_t i=0; i<_waterlines.size(); ++i) {
            float position = -_waterlines.get(i)->getPlane().d();
            p1.setZ(position);
            p2.setZ(p1.z());
            QString str = ConvertDimension(position, _settings.getUnits());
            QPoint pt1 = vp.convert3D(p1);
            QPoint pt2 = vp.convert3D(p2);
            painter->setPen(linepen1);
            painter->drawLine(pt1, pt2);
            int width = fm.width(str);
            painter->setPen(textpen);
            painter->drawText(pt1.x(), pt1.y(), str);
            painter->drawText(pt2.x() - width, pt2.y(), str);
        }
    }
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

// FreeShipUnit.pas:12186
void ShipCADModel::draw(Viewport& vp)
{
    LineShader* lineshader = vp.setLineShader();
    draw_intersection di(vp, lineshader);
    // draw intersection lines BEFORE the surface is drawn
    // so that the controlnet appears on top
    // but the intersections that should be drawn last depends on the view
    if (vp.getViewportType() != fvPerspective) {
        if (!_vis.isShowGrid()) {
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
        for (size_t i=0; i<_flowlines.size(); i++)
            _flowlines.get(i)->draw(vp, lineshader);
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
    IntersectionSorter sorter;
    sort(targetlist->begin(), targetlist->end(), sorter);
    return result;
}

// FreeShipUnit.pas:13056
void ShipCADModel::loadPreview(FileBuffer& source, QImage* image)
{
	// remember the filename because it is erased by the clear method
	QString tmpstr = _filename;
	clear();
	_filename = tmpstr;
	source.reset();
	QString hdr;
	source.load(hdr);
	if (hdr == "FREE!ship") {
        quint8 v;
		source.load(v);
		_file_version = static_cast<version_t>(v);
        if (_file_version >= fv210) {
			quint32 n;
			source.load(n);
			_precision = static_cast<precision_t>(n);
            _vis.loadBinary(source);
			_settings.loadBinary(source, image);
        }
    }
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
			_settings.loadBinary(source, nullptr);
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

Flowline* ShipCADModel::addFlowline(const QVector2D& pt, viewport_type_t ty)
{
    Flowline* flowline = Flowline::construct(this);
    flowline->initialize(pt, ty);
    flowline->rebuild();
    if (flowline->numberOfPoints() == 0) {
        delete flowline;
        flowline = nullptr;
    } else
        _flowlines.add(flowline);
    return flowline;
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
        buffer.add(getSurface()->indexOfLayer(face->getLayer()));
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

void ShipCADModel::loadChinesFromText(QTextStream& file, SplineVector& splines)
{
    size_t lineno = 0;
    bool unexpected = false;
    Spline* spline = nullptr;
    while (!unexpected && !file.atEnd()) {
        QString line = file.readLine();
        lineno++;
        bool ok;
        if (lineno == 1) {
            int units = line.toInt(&ok);
            if (ok && units == 0)
                _settings.setUnits(fuMetric);
            else if (ok && units == 1)
                _settings.setUnits(fuMetric);
            else
                throw ParseError(1, "first line is units, must be 0 or 1");
            continue;
        }
        // if an empty line, complete the spline
        // and get ready for more
        if (line.size() == 0 && spline != nullptr) {
            if (spline->numberOfPoints() > 1) {
                splines.add(spline);
            } else {
                delete spline;
            }
            spline = nullptr;
            continue;
        }
        if (line == "EOF")
            break;
        // split the line
        QStringList fields = line.split(" ");
        if (fields.size() != 3) {
            // try tabs
            fields = line.split("\t");
            if (fields.size() != 3) {
                unexpected = true;
                continue;
            }
        }
        // we have 3 pieces, turn it into a point
        float x = fields[0].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        float y = fields[1].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        float z = fields[2].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        // add the point to the spline
        QVector3D pt(x, y, z);
        if (spline == nullptr) {
            spline = new Spline();
        }
        spline->add(pt);
    }
    // complete the last spline
    if (!unexpected && spline != nullptr) {
        if (spline->numberOfPoints() > 1) {
            splines.add(spline);
        } else
            delete spline;
    }
    // got something wrong, bail
    if (unexpected) {
        throw ParseError(lineno, "");
    }
}

// FreeShipUnit.pas:12715
void ShipCADModel::importChines(size_t np, SplineVector& chines)
{
    SubdivisionSurface* surf = getSurface();
    for (size_t i=0; i<chines.size(); ++i) {
        SubdivisionLayer* layer = (i < surf->numberOfLayers()) ?
            surf->getLayer(i) : surf->addNewLayer();
        // msg 0186
        layer->setName(tr("Strake %1").arg(i+1));
        layer->setDevelopable(true);
    }
    // add special layer to close the hull at centerline
    SubdivisionLayer* layer = surf->addNewLayer();
    // msg 0187
    layer->setName(tr("Close hull"));
    // prepare matrices
    MatrixXd mat(np, np);
    for (size_t i=0; i<np; ++i)
        for (size_t j=0; j<np; ++j)
            mat(i,j) = 0.0;
    mat(0, 0) = 1.0;
    for (size_t i=2; i<np; i++) {
        mat(i-1, i-2) = 1/6.0;
        mat(i-1, i-1) = 2/3.0;
        mat(i-1, i) = 1/6.0;
    }
    mat(np-1, np-1) = 1.0;
    ColPivHouseholderQR<MatrixXd> qr(np, np);
    qr.compute(mat);
    MatrixXd row(np, 3);
    QVector3D min, max;
    CPointGrid points;
    points.setRows(np);
    points.setCols(chines.size());
    
    for (size_t i=0; i<chines.size(); ++i) {
        Spline* spline = chines.get(i);
        for (size_t j=0; j<np; ++j) {
            QVector3D p = spline->value(j / static_cast<float>(np - 1));
            row(j, 0) = p.x();
            row(j, 1) = p.y();
            row(j, 2) = p.z();
        }
        // calculate new points
        MatrixXd newpts = qr.solve(row);
        for (size_t j=0; j<np; ++j) {
            QVector3D pt(newpts(j, 0), newpts(j, 1), newpts(j, 2));
            if (pt.y() < 0)
                pt.setY(0.0);
            if (i == 0 && j == 0) {
                min = pt;
                max = min;
            } else
                MinMax(pt, min, max);
            points.setPoint(j, i, surf->addControlPoint(pt));
        }
    }
    // add chines as markers
    for (size_t i=0; i<chines.size(); ++i) {
        Spline* spline = chines.get(i);
        Marker* marker = addMarker();
        for (size_t j=0; j<spline->numberOfPoints(); ++j) {
            marker->add(spline->getPoint(j));
            marker->setKnuckle(j, spline->isKnuckle(j));
        }
    }
    // setup control faces
    vector<SubdivisionControlPoint*> pts;
    for (size_t i=1; i<np; ++i) {
        for (size_t j=1; j<chines.size(); ++j) {
            pts.clear();
            SubdivisionControlPoint* point = points.getPoint(i, j);
            if (find(pts.begin(), pts.end(), point) == pts.end())
                pts.push_back(point);
            point = points.getPoint(i-1, j);
            if (find(pts.begin(), pts.end(), point) == pts.end())
                pts.push_back(point);
            point = points.getPoint(i-1, j-1);
            if (find(pts.begin(), pts.end(), point) == pts.end())
                pts.push_back(point);
            point = points.getPoint(i, j-1);
            if (find(pts.begin(), pts.end(), point) == pts.end())
                pts.push_back(point);
            if (pts.size() > 2)
                surf->addControlFace(pts, true, surf->getLayer(j-1));
        }
    }
    for (size_t i=1; i<np; ++i) {
        for (size_t j=0; j<chines.size(); ++j) {
            SubdivisionControlEdge* edge = surf->controlEdgeExists(points.getPoint(i-1, j-1),
                                                                   points.getPoint(i, j));
            if (edge != nullptr)
                edge->setCrease(true);
        }
    }

    // add controlcurves
    for (size_t j=0; j<chines.size(); ++j) {
        SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(surf);
        surf->addControlCurve(curve);
        for (size_t i=0; i<np; ++i) {
            curve->addPoint(points.getPoint(i, j));
            if (i > 0) {
                SubdivisionControlEdge* edge = surf->controlEdgeExists(points.getPoint(i-1, j),
                                                                       points.getPoint(i, j));
                if (edge != nullptr)
                    edge->setCurve(curve);
            }
        }
    }

    // check for stem, keel, and stern points to be closed
    pts.clear();
    // first stern
    for (size_t i=chines.size(); i>=2; --i)
        pts.push_back(points.getPoint(np-1, i-1));
    // then keel
    for (size_t i=np; i>=1; --i)
        pts.push_back(points.getPoint(i-1, 0));
    // and finally stem
    for (size_t i=2; i<=chines.size(); ++i)
        pts.push_back(points.getPoint(0, i-1));
    vector<SubdivisionControlPoint*> pts2;
    for (size_t i=0; i<pts.size(); ++i) {
        SubdivisionControlPoint* point = pts[i];
        QVector3D p = point->getCoordinate();
        if (p.y() != 0) {
            p.setY(0);
            SubdivisionControlPoint* pt = surf->addControlPoint(p);
            pts2.push_back(pt);
        } else
            pts2.push_back(point);
    }
    for (size_t i=1; i<pts.size(); ++i) {
        vector<SubdivisionControlPoint*> temp;
        if (find(temp.begin(), temp.end(), pts2[i]) == temp.end())
            temp.push_back(pts2[i]);
        if (find(temp.begin(), temp.end(), pts2[i-1]) == temp.end())
            temp.push_back(pts2[i-1]);
        if (find(temp.begin(), temp.end(), pts[i-1]) == temp.end())
            temp.push_back(pts[i-1]);
        if (find(temp.begin(), temp.end(), pts[i]) == temp.end())
            temp.push_back(pts[i]);
        if (temp.size() > 2)
            surf->addControlFace(temp, false, layer);
    }

    // set transom as knuckle
    for (size_t j=1; j<chines.size(); ++j) {
        SubdivisionControlEdge* edge = surf->controlEdgeExists(points.getPoint(np-1, j-1),
                                                               points.getPoint(np-1, j));
        if (edge != nullptr)
            edge->setCrease(true);
    }
    // delete unused layers
    surf->deleteEmptyLayers();
    // delete unused control points
    for (size_t i=surf->numberOfControlPoints(); i>=1; --i)
        if (surf->getControlPoint(i-1)->numberOfFaces() == 0)
            surf->deleteControlPoint(surf->getControlPoint(i-1));
    surf->deleteElementsCollection();
    extents(min, max);
    _settings.setBeam(2*max.y());
    _settings.setLength(max.x() - min.x());
    _settings.setDraft(1.0);
    setPrecision(fpHigh);
    setBuild(false);
}

