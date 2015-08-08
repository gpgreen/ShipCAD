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

#ifndef INTERSECTION_H_
#define INTERSECTION_H_

#include <vector>
#include <string>
#include <QtCore>
#include <QtGui>
#include "entity.h"
#include "plane.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class ShipCAD;
class Viewport;
class LineShader;
class FileBuffer;
class Spline;

class Intersection : public Entity
{
    Q_OBJECT

public:

    enum intersection_type_t {
        fiFree,
        fiStation,
        fiButtock,
        fiWaterline,
        fiDiagonal,
    };

    explicit Intersection(ShipCAD* owner);
    virtual ~Intersection();

    static Intersection* construct(ShipCAD* owner);

    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader) = 0;
    virtual void rebuild();

    void add(Spline* sp);
    void calculateArea(const Plane& plane, float* area, QVector3D* cog, QVector2D* moment_of_inertia);
    void createStarboardPart();
    void deleteItem(Spline* item);

    void loadBinary(FileBuffer& buf);
    void saveBinary(FileBuffer& buf);

    void saveToDXF(QStringList& strings);

public slots:

protected:

private:

    ShipCAD* _owner;
    std::vector<Spline*> _items;
    intersection_type_t _intersection_type;
    Plane _plane;
    bool _show_curvature;
    bool _use_hydrostatic_surfaces_only;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

