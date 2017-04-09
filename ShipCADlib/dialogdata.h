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
#include <vector>
#include "shipcadlib.h"

namespace ShipCAD {

    class SubdivisionLayer;
    
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
struct LayerDialogData
{
    explicit LayerDialogData();
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
    
    explicit RotateDialogData();
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

