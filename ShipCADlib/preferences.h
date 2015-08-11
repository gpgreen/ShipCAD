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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtCore>
#include <QtGui>

namespace ShipCADGeometry {

class ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

class Preferences : public QObject
{
    Q_OBJECT
public:

    explicit Preferences(ShipCAD* owner);
    ~Preferences();

    static Preferences* construct(ShipCAD* owner);

    QString getExportDirectory();
    QString getImportDirectory();
    QString getOpenDirectory();
    QString getSaveDirectory();

    void setViewportColor(QColor col);

    void edit();
    void load();
    void resetColors();
    void save();

    QColor getStationColor() {return _station_color;}
    QColor getButtockColor() {return _buttock_color;}
    QColor getWaterlineColor() {return _waterline_color;}
    QColor getDiagonalColor() {return _diagonal_color;}

    void clear();

public slots:

protected:

private:

    ShipCAD* _owner;
    int _point_size;
    QColor _buttock_color;
    QColor _waterline_color;
    QColor _station_color;
    QColor _diagonal_color;
    QColor _edge_color;
    QColor _crease_color;
    QColor _crease_edge_color;
    QColor _grid_color;
    QColor _grid_font_color;
    QColor _crease_point_color;
    QColor _regular_point_color;
    QColor _corner_point_color;
    QColor _dart_point_color;
    QColor _select_color;
    QColor _layer_color;
    QColor _normal_color;
    QColor _underwater_color;
    QColor _viewport_color;
    QColor _leakpoint_color;
    QColor _marker_color;
    QColor _curvature_plot_color;
    QColor _control_curve_color;
    QColor _hydrostatics_font_color;
    QColor _zebra_stripe_color;
    QString _open_directory;
    QString _save_directory;
    QString _import_directory;
    QString _export_directory;
    QString _lang_file;
    int _max_undo_memory;

};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // PREFERENCES_H

