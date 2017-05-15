/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>								   *
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

#ifndef DIALOGDATA_H_
#define DIALOGDATA_H_

#include <QtCore>
#include <QColorDialog>
#include <QFrame>
#include <vector>
#include "shipcadlib.h"
#include "intersection.h"

namespace ShipCAD {

class SubdivisionLayer;
class Preferences;
    
//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for insert plane dialog exchange
 */
struct InsertPlaneDialogData
{
    bool accepted;
    bool addControlCurveSelected;
    plane_selected_t planeSelected;
    float distance;
    QVector3D min;
    QVector3D max;
    explicit InsertPlaneDialogData();
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for intersect layer dialog exchange
 */
struct IntersectLayersDialogData
{
    bool accepted;
    std::vector<SubdivisionLayer*> layers;
    size_t layer1;
    size_t layer2;
    explicit IntersectLayersDialogData();
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for extrude edge dialog exchange
 */
struct ExtrudeEdgeDialogData
{
    bool accepted;
    QVector3D vector;
    explicit ExtrudeEdgeDialogData();
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for color chooser dialog exchange
 */
struct ChooseColorDialogData
{
    bool accepted;
    QString title;
    const QColor& initial;
    QColorDialog::ColorDialogOptions options;
    QColor chosen;
    explicit ChooseColorDialogData(const QString& title, const QColor& initial);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for layer properties dialog exchange
 */
struct LayerPropertiesForDialog
{
    QString name;
    QColor color;
    float alpha;
    bool hydrostatics;
    bool symmetric;
    bool intersection_curves;
    bool developable;
    bool show_linesplan;
    float material_density;
    float thickness;
    const SubdivisionLayer* data;
    ShipCAD::LayerProperties layer_properties;
};
    
struct LayerDialogData
{
    const SubdivisionLayer* active;
    std::vector<LayerPropertiesForDialog> layers;
    explicit LayerDialogData(std::vector<ShipCAD::SubdivisionLayer*> list_of_layers,
        SubdivisionLayer* active_layer);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for layer chooser dialog exchange
 */
enum LayerSelectMode {
    fsFaces = 0,
    fsPoints
};
    
struct ChooseLayerDialogData
{
    bool accepted;
    bool include_points;
    ShipCAD::LayerSelectMode mode;
    // collection of layers and whether they are selected
    std::vector<std::pair<ShipCAD::SubdivisionLayer*, bool> > layers;
    explicit ChooseLayerDialogData(std::vector<ShipCAD::SubdivisionLayer*> list_of_layers,
                                   ShipCAD::LayerSelectMode mode);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for mirror dialog exchange
 */

struct MirrorDialogData
{
    bool accepted;
    bool connect_points;
    plane_selected_t which_plane;
    float distance;
    
    explicit MirrorDialogData(bool connect, plane_selected_t init, float d);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for rotate dialog exchange
 */

struct RotateDialogData
{
    bool accepted;
    QVector3D rotation_vector;
    QString dialog_title;
    QString units;

    explicit RotateDialogData(const QString& title, const QString& units);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for intersections dialog exchange
 */

struct IntersectionsDialogData
{
    intersection_type_t intersection_type;
    std::vector<float> intersection_offsets;
    bool add_range;
    bool delete_all_intersections;
    bool delete_intersections;
    bool changed;

    IntersectionVector stations;
    IntersectionVector waterlines;
    IntersectionVector buttocks;
    IntersectionVector diagonals;
    
    explicit IntersectionsDialogData(ShipCAD::ShipCADModel* model);
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for new model dialog exchange
 */

struct NewModelDialogData
{
    bool accepted;
    float length;
    float breadth;
    float depth;
    size_t rows;
    size_t cols;
    unit_type_t units;

    explicit NewModelDialogData();
};

//////////////////////////////////////////////////////////////////////////////////////
/*! \brief data structure for preferences dialog exchange
 */

struct ColorChanger 
{
    QColor orig;
    QColor* setColor;

    explicit ColorChanger(QColor* addr);
    
};

struct PreferencesDialogData
{
    bool accepted;
    size_t undo_memory;
    size_t control_point_size;
    std::map<int, ColorChanger> colors;

    explicit PreferencesDialogData(Preferences& p);
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

