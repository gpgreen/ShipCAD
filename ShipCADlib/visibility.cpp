/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#include "visibility.h"
#include "filebuffer.h"
#include "shipcadmodel.h"

using namespace ShipCAD;

Visibility::Visibility(ShipCADModel *owner)
    : _owner(owner)
{
    clear();
}

void Visibility::clear()
{
    _model_view = mvPort;
    _show_interior_edges = false;
    _show_control_net = true;
    _show_grid = true;
    _show_normals = true;
    _show_stations = true;
    _show_buttocks = true;
    _show_waterlines = true;
    _show_diagonals = true;
    _show_markers = true;
    _show_curvature = true;
    _show_control_curves = true;
    _curvature_scale = 1.0;
    _cursor_increment = 0.1f;
    _show_hydrostatic_data = true;
    _show_hydro_displacement = true;
    _show_hydro_lateral_area = true;
    _show_hydro_metacentric_height = true;
    _show_hydro_lcf = true;
    _show_flow_lines = true;
    emit onChangeCursorIncrement();
}

void Visibility::loadBinary(FileBuffer &source)
{
    clear();
    int i;
    source.load(i);
    _model_view = static_cast<model_view_t>(i);
    source.load(_show_control_net);
    source.load(_show_interior_edges);
    source.load(_show_stations);
    source.load(_show_buttocks);
    source.load(_show_waterlines);
    source.load(_show_normals);
    source.load(_show_grid);
    source.load(_show_diagonals);
    source.load(_show_markers);
    source.load(_show_curvature);
    source.load(_curvature_scale);
    if (_owner->getFileVersion() >= fv195) {
        source.load(_show_control_curves);
        if (_owner->getFileVersion() >= fv210) {
            source.load(_cursor_increment);
            if (fabs(_cursor_increment)<1e-5)
                _cursor_increment = 0.1f;
            emit onChangeCursorIncrement();
            if (_owner->getFileVersion() >= fv220) {
                source.load(_show_hydrostatic_data);
                source.load(_show_hydro_displacement);
                source.load(_show_hydro_sectional_areas);
                source.load(_show_hydro_metacentric_height);
                source.load(_show_hydro_lcf);
                if (_owner->getFileVersion() >= fv250) {
                    source.load(_show_flow_lines);
                }
            }
        }
    }
}

void Visibility::saveBinary(FileBuffer &dest)
{
    dest.add(static_cast<int>(_model_view));
    dest.add(_show_control_net);
    dest.add(_show_interior_edges);
    dest.add(_show_stations);
    dest.add(_show_buttocks);
    dest.add(_show_waterlines);
    dest.add(_show_grid);
    dest.add(_show_diagonals);
    dest.add(_show_markers);
    dest.add(_show_markers);
    dest.add(_show_curvature);
    dest.add(_curvature_scale);
    if (_owner->getFileVersion() >= fv195) {
        dest.add(_show_control_curves);
        if (_owner->getFileVersion() >= fv210) {
            dest.add(_cursor_increment);
            if (_owner->getFileVersion() >= fv220) {
                dest.add(_show_hydrostatic_data);
                dest.add(_show_hydro_displacement);
                dest.add(_show_hydro_lateral_area);
                dest.add(_show_hydro_sectional_areas);
                dest.add(_show_hydro_metacentric_height);
                dest.add(_show_hydro_lcf);
                if (_owner->getFileVersion() >= fv250) {
                    dest.add(_show_flow_lines);
                }
            }
        }
    }
}

void Visibility::decreaseCurvatureScale()
{
    _curvature_scale /= 1.1f;
    _owner->setFileChanged(true);
    _owner->visibilityOptionsChanged();
}

void Visibility::increaseCurvatureScale()
{
    _curvature_scale *= 1.1f;
    _owner->setFileChanged(true);
    _owner->visibilityOptionsChanged();
}

void Visibility::setModelView(model_view_t vw)
{
    if (vw != _model_view) {
        _model_view = vw;
        _owner->getSurface()->setDrawMirror(vw == mvBoth);
        _owner->setFileChanged(true);
        _owner->visibilityOptionsChanged();
    }
}

void Visibility::setShowControlNet(bool show)
{
	if (show != _show_control_net) {
		_show_control_net = show;
        _owner->getSurface()->setShowControlNet(show);
        _owner->setFileChanged(true);
        _owner->visibilityOptionsChanged();
    }
}

void Visibility::setShowInteriorEdges(bool show)
{
	if (show != _show_interior_edges) {
		_show_interior_edges = show;
        _owner->getSurface()->setShowInteriorEdges(show);
        _owner->setFileChanged(true);
        _owner->visibilityOptionsChanged();
    }
}

void Visibility::setShowControlCurves(bool show)
{
    if (show != _show_control_curves) {
        _show_control_curves = show;
        _owner->getSurface()->setShowControlCurves(show);
        _owner->setFileChanged(true);
        _owner->visibilityOptionsChanged();
    }
}

void Visibility::setShowCurvature(bool show)
{
    if (show != _show_curvature) {
        _show_curvature = show;
        _owner->getSurface()->setShowCurvature(show);
        _owner->setFileChanged(true);
        _owner->visibilityOptionsChanged();
    }
}

