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

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class ShipCADModel;
class Viewport;
class LineShader;
class FileBuffer;
class Spline;

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
    virtual ~Intersection();

    static Intersection* construct(ShipCADModel* owner);

    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void rebuild();

    QColor getColor();
    Plane getPlane() {return _plane;}
    size_t getCount() {return _items.size();}
    Spline* getItem(size_t index);
    const QString& getDescription();
    void add(Spline* sp);
    void calculateArea(const Plane& plane, float* area, QVector3D* cog, QVector2D* moment_of_inertia);
    void createStarboardPart();
    void deleteItem(Spline* item);

    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);

    void saveToDXF(QStringList& strings);

public slots:

protected:

    virtual void setBuild(bool val);

private:

    ShipCADModel* _owner;
    std::vector<Spline*> _items;
    intersection_type_t _intersection_type;
    Plane _plane;
    bool _show_curvature;
    bool _use_hydrostatic_surfaces_only;
};

/*! \brief Vector class to contain Intersection pointers
 *
 */
class IntersectionVector
{
public:

    typedef std::vector<Intersection*>::iterator ivec_iterator ;

    IntersectionVector();
    ~IntersectionVector();

    void clear();

    size_t size() {return _vec.size();}

    void add(Intersection* i) {_vec.push_back(i);}
    void del(Intersection* i);

    typedef void apply_fn(Intersection* elem);
    void apply(apply_fn* fn)
      {std::for_each(_vec.begin(), _vec.end(), fn);}

    ivec_iterator begin() {return _vec.begin();}
    ivec_iterator end() {return _vec.end();}

private:
    std::vector<Intersection*> _vec;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

