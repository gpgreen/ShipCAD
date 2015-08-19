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

#ifndef VISIBILITY_H
#define VISIBILITY_H

#include <vector>
#include <string>
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"

namespace ShipCAD {

class ShipCADModel;
class FileBuffer;

//////////////////////////////////////////////////////////////////////////////////////

class Visibility : public QObject
{
    Q_OBJECT
public:

    explicit Visibility(ShipCADModel* owner);
    ~Visibility() {}

    void setCursorIncrement(float val);
    void setCurvatureScale(float val);
    void setShowButtocks(bool show);
    void setShowControlNet(bool show);
    void setShowCurvature(bool show);
    void setShowDiagonals(bool show);
    void setShowFlowlines(bool show);
    void setShowGrid(bool show);
    model_view_t getModelView() const
        {return _model_view;}
    void setModelView(model_view_t vw);
    void setShowInteriorEdges(bool show);
    bool isShowMarkers() const
        {return _show_markers;}
    void setShowMarkers(bool show);
    void setShowNormals(bool show);
    void setShowStations(bool show);
    void setShowWaterlines(bool show);
    void setShowControlCurves(bool show);
    void setShowHydrostaticData(bool show);

    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);

    void clear();

signals:

    void onChangeCursorIncrement();

public slots:

    void decreaseCurvatureScale();
    void increaseCurvatureScale();

protected:

private:

    ShipCADModel* _owner;
    bool _show_control_net;
    bool _show_interior_edges;
    bool _show_stations;
    bool _show_buttocks;
    bool _show_waterlines;
    bool _show_diagonals;
    model_view_t _model_view;
    bool _show_normals;
    bool _show_grid;
    bool _show_markers;
    bool _show_control_curves;
    bool _show_curvature;
    bool _show_hydrostatic_data;
    bool _show_hydro_displacement;
    bool _show_hydro_lateral_area;
    bool _show_hydro_sectional_areas;
    bool _show_hydro_metacentric_height;
    bool _show_hydro_lcf;
    bool _show_flow_lines;
    float _curvature_scale;
    float _cursor_increment;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */


#endif // VISIBILITY_H

