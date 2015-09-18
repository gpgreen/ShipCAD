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

namespace ShipCAD {

class ShipCADModel;
class SubdivisionSurface;
    
//////////////////////////////////////////////////////////////////////////////////////

class Preferences : public QObject
{
    Q_OBJECT
public:

    explicit Preferences(ShipCADModel* owner);
    ~Preferences() {}

    QString getExportDirectory() const;
    QString getImportDirectory() const;
    QString getOpenDirectory() const;
    QString getSaveDirectory() const;

    void setViewportColor(QColor col);

    void edit();
    void load();
    void resetColors();
    void save();

    void setSurfaceColors(SubdivisionSurface& surface);
    
	/*! \brief get maximum amount of undo memory
	 *
	 * \return max amount of undo memory in mb
	 */
	size_t getMaxUndoMemory() const
        {return _max_undo_memory;}

    QColor getButtockColor() const
        {return _buttock_color;}
    QColor getWaterlineColor() const
        {return _waterline_color;}
    QColor getStationColor() const
        {return _station_color;}
    QColor getDiagonalColor() const
        {return _diagonal_color;}
    QColor getEdgeColor() const
        {return _edge_color;}
    QColor getCreaseColor() const
        {return _crease_color;}
    QColor getCreaseEdgeColor() const
        {return _crease_edge_color;}
    QColor getGridColor() const
        {return _grid_color;}
    QColor getGridFontColor() const
        {return _grid_font_color;}
    QColor getCreasePointColor() const
        {return _crease_point_color;}
    QColor getRegularPointColor() const
        {return _regular_point_color;}
    QColor getCornerPointColor() const
        {return _corner_point_color;}
    QColor getDartPointColor() const
        {return _dart_point_color;}
    QColor getSelectColor() const 
        {return _select_color;}
    QColor getLayerColor() const
        {return _layer_color;}
    QColor getNormalColor() const
        {return _normal_color;}
    QColor getUnderwaterColor() const
        {return _underwater_color;}
    QColor getViewportColor() const
        {return _viewport_color;}
    QColor getLeakPointColor() const
        {return _leakpoint_color;}
    QColor getMarkerColor() const
        {return _marker_color;}
    QColor getCurvaturePlotColor() const
        {return _curvature_plot_color;}
    QColor getControlCurveColor() const
        {return _control_curve_color;}
    QColor getHydrostaticsFontColor() const
        {return _hydrostatics_font_color;}
    QColor getZebraStripeColor() const
        {return _zebra_stripe_color;}

    void clear();

private:

    ShipCADModel* _owner;
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
    size_t _max_undo_memory;

};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // PREFERENCES_H

