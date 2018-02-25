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

#include <QSettings>
#include "preferences.h"
#include "shipcadmodel.h"
#include "subdivsurface.h"

using namespace ShipCAD;
using namespace std;

Preferences::Preferences(ShipCADModel* owner)
    : _owner(owner), _point_size(4), _max_undo_memory(20)
{
    resetColors();
    readSettings();
}

void Preferences::clear()
{
    resetColors();
    _point_size = 4;
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

void Preferences::setSurfaceColors(SubdivisionSurface& surface)
{
    surface._control_point_size = _point_size;

    surface._crease_color = _crease_color;
    surface._crease_edge_color = _crease_edge_color;
    surface._underwater_color = _underwater_color;
    surface._edge_color = _edge_color;
    surface._selected_color = _select_color;
    surface._crease_point_color = _crease_point_color;
    surface._regular_point_color = _regular_point_color;
    surface._corner_point_color = _corner_point_color;
    surface._dart_point_color = _dart_point_color;
    surface._layer_color = _layer_color;
    surface._normal_color = _normal_color;
    surface._leak_color = _leakpoint_color;
    surface._curvature_color = _curvature_plot_color;
    surface._control_curve_color = _control_curve_color;
    surface._zebra_color = _zebra_stripe_color;
}

void Preferences::getColorDialogMap(map<int, ColorChanger>& colors)
{
    colors.insert(make_pair(0, ColorChanger(&_viewport_color)));
    colors.insert(make_pair(1, ColorChanger(&_grid_color)));
    colors.insert(make_pair(2, ColorChanger(&_grid_font_color)));
    colors.insert(make_pair(3, ColorChanger(&_layer_color)));
    colors.insert(make_pair(4, ColorChanger(&_underwater_color)));
    colors.insert(make_pair(5, ColorChanger(&_normal_color)));
    colors.insert(make_pair(6, ColorChanger(&_control_curve_color)));
    colors.insert(make_pair(7, ColorChanger(&_edge_color)));
    colors.insert(make_pair(8, ColorChanger(&_crease_color)));
    colors.insert(make_pair(9, ColorChanger(&_crease_edge_color)));
    colors.insert(make_pair(10, ColorChanger(&_regular_point_color)));
    colors.insert(make_pair(11, ColorChanger(&_crease_point_color)));
    colors.insert(make_pair(12, ColorChanger(&_corner_point_color)));
    colors.insert(make_pair(13, ColorChanger(&_dart_point_color)));
    colors.insert(make_pair(14, ColorChanger(&_leakpoint_color)));
    colors.insert(make_pair(15, ColorChanger(&_select_color)));
    colors.insert(make_pair(16, ColorChanger(&_curvature_plot_color)));
    colors.insert(make_pair(17, ColorChanger(&_marker_color)));
    colors.insert(make_pair(18, ColorChanger(&_station_color)));
    colors.insert(make_pair(19, ColorChanger(&_buttock_color)));
    colors.insert(make_pair(20, ColorChanger(&_waterline_color)));
    colors.insert(make_pair(21, ColorChanger(&_diagonal_color)));
    colors.insert(make_pair(22, ColorChanger(&_hydrostatics_font_color)));
    colors.insert(make_pair(23, ColorChanger(&_zebra_stripe_color)));
}

void Preferences::readSettings()
{
    // the colors
    map<int, ColorChanger> colors;
    getColorDialogMap(colors);
    QSettings settings;
    int size = settings.beginReadArray("preferences/color");
    if (size > 0) {
        for (int i=0; i<size && i<colors.size(); ++i) {
            settings.setArrayIndex(i);
            int red = settings.value("red").toInt();
            int green = settings.value("green").toInt();
            int blue = settings.value("blue").toInt();
            map<int, ColorChanger>::iterator j = colors.find(i);
            if (j == colors.end())
                continue;
            ColorChanger& changer = (*j).second;
            *(changer.setColor) = QColor(red, green, blue);
        }
    }
    settings.endArray();
    // the rest
    if (settings.contains("preferences/pointsize"))
        _point_size = settings.value("preferences/pointsize").toInt();
    if (settings.contains("preferences/maxundomemory"))
        _max_undo_memory = settings.value("preferences/maxundomemory").toInt();
    
}

void Preferences::saveSettings() const
{
    // colors
    map<int, ColorChanger> colors;
    const_cast<Preferences*>(this)->getColorDialogMap(colors);
    QSettings settings;
    settings.beginWriteArray("preferences/color");
    for (int i=0; i<colors.size(); ++i) {
        settings.setArrayIndex(i);
        map<int, ColorChanger>::iterator j = colors.find(i);
        if (j == colors.end())
            continue;
        ColorChanger& changer = (*j).second;
        settings.setValue("red", (*changer.setColor).red());
        settings.setValue("green", (*changer.setColor).green());
        settings.setValue("blue", (*changer.setColor).blue());
    }
    settings.endArray();
    // the rest
    settings.setValue("preferences/pointsize", _point_size);
    settings.setValue("preferences/maxundomemory", static_cast<int>(_max_undo_memory));
}
