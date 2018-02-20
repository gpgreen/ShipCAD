/*##############################################################################################
 *    ShipCAD
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>
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

#include "dialogdata.h"
#include "subdivlayer.h"
#include "shipcadmodel.h"
#include "preferences.h"

using namespace ShipCAD;
using namespace std;

InsertPlaneDialogData::InsertPlaneDialogData()
    : accepted(false),
      addControlCurveSelected(false),
      planeSelected(transverse),
      distance(0.0), min(ZERO), max(ZERO)
{
    // does nothing
}

IntersectLayersDialogData::IntersectLayersDialogData()
    : accepted(false), layer1(0), layer2(0)
{
    // does nothing
}

ExtrudeEdgeDialogData::ExtrudeEdgeDialogData()
    : accepted(false), vector(ZERO)
{
    // does nothing
}

ChooseColorDialogData::ChooseColorDialogData(const QString& title, const QColor& initial)
    : accepted(false), title(title), initial(initial), options(QColorDialog::ColorDialogOptions())
{
    // does nothing
}

ChooseLayerDialogData::ChooseLayerDialogData(vector<SubdivisionLayer*> list_of_layers,
    LayerSelectMode m)
    : accepted(false), include_points(false), mode(m)
{
    for (size_t i=0; i<list_of_layers.size(); i++)
        layers.push_back(make_pair(list_of_layers[i], true));
}

MirrorDialogData::MirrorDialogData(bool connect, plane_selected_t init, float d)
    : accepted(false), connect_points(connect), which_plane(init), distance(d)
{
    // does nothing
}

RotateDialogData::RotateDialogData(const QString& title, const QString& unitstr)
    : accepted(false), dialog_title(title), units(unitstr)
{
    // does nothing
}

LayerDialogData::LayerDialogData(vector<SubdivisionLayer*> list_of_layers,
                                 SubdivisionLayer* active_layer)
    : active(active_layer)
{
    for (size_t i=0; i<list_of_layers.size(); i++)
        layers.push_back(list_of_layers[i]->getProperties());
}

IntersectionsDialogData::IntersectionsDialogData(ShipCADModel* model)
    : intersection_type(fiStation), add_range(false),
      delete_all_intersections(false), delete_intersections(false), changed(false),
      stations(false), waterlines(false), buttocks(false), diagonals(false)
{
    stations.insert(stations.begin(), model->getStations().begin(), model->getStations().end());
    buttocks.insert(buttocks.begin(), model->getButtocks().begin(), model->getButtocks().end());
    waterlines.insert(waterlines.begin(), model->getWaterlines().begin(), model->getWaterlines().end());
    diagonals.insert(diagonals.begin(), model->getDiagonals().begin(), model->getDiagonals().end());
}

NewModelDialogData::NewModelDialogData()
    : accepted(false), length(0.0), breadth(0.0), depth(0.0), rows(0), cols(0), units(fuMetric)
{
    // does nothing
}

ColorChanger::ColorChanger(QColor* addr)
    : orig(*addr), setColor(addr)
{
    // does nothing
}

PreferencesDialogData::PreferencesDialogData(Preferences& p)
    : accepted(false), undo_memory(p.getMaxUndoMemory()), control_point_size(p.getPointSize())
{
    p.getColorDialogMap(colors);
}

ProjectSettingsDialogData::ProjectSettingsDialogData(ShipCADModel* model)
    : accepted(false), settings(model), visibility(model),
      units(model->getProjectSettings().getUnits())
{
    settings.copy_to_dialog(model->getProjectSettings());
    visibility.copy_to_dialog(model->getVisibility());
}
