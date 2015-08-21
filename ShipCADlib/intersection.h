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
#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"
#include "entity.h"
#include "plane.h"
#include "pointervec.h"
#include "spline.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class ShipCADModel;
class Viewport;
class LineShader;
class FileBuffer;

/*! \brief List of curves intersecting hull
 *
 * A list of curves from the intersection of the ships hull (represented by a
 * subdivision surface) and a plane
 * This plane can be a orthogonal plane (eg stations, waterlines, buttocks) or
 * a freely oriented 3D plane
 */
class Intersection : public Entity
{
    Q_OBJECT

public:

    explicit Intersection(ShipCADModel* owner);
    explicit Intersection(ShipCADModel *owner, intersection_type_t ty, const Plane& pln, bool use_hydrostatics_only);
    virtual ~Intersection() {}

    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void rebuild();
    virtual void setBuild(bool val);

    QColor getColor();
    Plane getPlane()
        {return _plane;}
    void setPlane(const Plane& pln)
        {_plane=pln;}
    QString getDescription();
    intersection_type_t getIntersectionType()
        {return _intersection_type;}
    void setIntersectionType(intersection_type_t set)
        {_intersection_type=set;}
    bool useHydrostaticsSurfacesOnly()
        {return _use_hydrostatic_surfaces_only;}
    void setUseHydrostaticsSurfacesOnly(bool set)
        {_use_hydrostatic_surfaces_only=set;}

    SplineVector& getSplines() {return _items;}

    void calculateArea(const Plane& plane, float* area, QVector3D* cog, QVector2D* moment_of_inertia);
    void createStarboardPart();
    void deleteItem(Spline* item);

    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);

    void saveToDXF(QStringList& strings);

public slots:

protected:

private:

    ShipCADModel* _owner;
    SplineVector _items;
    intersection_type_t _intersection_type;
    Plane _plane;
    bool _show_curvature;
    bool _use_hydrostatic_surfaces_only;
};

typedef PointerVector<Intersection> IntersectionVector;

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

