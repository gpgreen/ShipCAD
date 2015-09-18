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

#include "preferences.h"
#include "shipcadmodel.h"

using namespace ShipCAD;

Preferences::Preferences(ShipCADModel* owner)
    : _owner(owner), _point_size(2), _max_undo_memory(20)
{
	resetColors();
}

void Preferences::clear()
{
	resetColors();
	_point_size = 2;
	_max_undo_memory = 20;
}

void Preferences::resetColors()
{
    _buttock_color = QColor(0x80,0x80,0x40);
    _waterline_color = QColor(0x80,0x80,0x40);
    _station_color = QColor(0x80,0x80,0x40);
    _diagonal_color = QColor(0x80, 0x80, 0x40);
    _edge_color = QColor(0x6f,0x6f,0x6f);
    _crease_color = Qt::black;
    _crease_edge_color = Qt::red;
    _grid_color = QColor(0xc0, 0xc0, 0xc0);
    _grid_font_color = Qt::white;
    _crease_point_color = QColor(0, 0x40, 0x80);
    _regular_point_color = QColor(0xe1, 0xe1, 0xe1);
    _corner_point_color = QColor(0xb9, 0x5c, 0);
    _dart_point_color = QColor(0xff, 0x00, 0x80);
    _select_color = Qt::yellow;
    _layer_color = QColor(0, 128, 0);
    _normal_color = Qt::white;
    _underwater_color = QColor(240, 240, 240);
    _viewport_color = QColor(0x9f, 0x9f, 0x9f);
    _leakpoint_color = QColor(0x99, 0xff, 0);
    _marker_color = QColor(0x80, 0, 0xff);
    _curvature_plot_color = QColor(0xff, 0x00, 0x80);
    _control_curve_color = QColor(0xff, 0x00, 0x80);
    _hydrostatics_font_color = QColor(0x80, 0, 0);
    _zebra_stripe_color = QColor(230, 230, 230);
}

