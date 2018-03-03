/*##############################################################################################
 *    ShipCAD										       *
 *    Copyright 2018, by Greg Green <ggreen@bit-builder.com>				       *
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

#ifndef DRAWFACES_H_
#define DRAWFACES_H_

#include <QtCore>
#include <vector>

namespace ShipCAD {

class SubdivisionFace;
class FaceShader;
class Plane;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief draw SubdivisionFaces
 */
void DrawFaces(ShipCAD::FaceShader* faceshader,
               const ShipCAD::Plane& waterlinePlane,
               std::vector<ShipCAD::SubdivisionFace*>& faces,
               size_t& vertices1, size_t& vertices2,
               bool shadeUnderwater, bool drawMirror,
               QColor color, QColor underWaterColor);

};				/* end namespace */

#endif

