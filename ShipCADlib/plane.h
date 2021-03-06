/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#ifndef PLANE_H_
#define PLANE_H_

#include <QVector3D>

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class Plane
{
public:
    explicit Plane();
    explicit Plane(float a, float b, float c, float d);
	explicit Plane(const QVector3D& p, const QVector3D& normal);
    explicit Plane(const QVector3D& p1,
		   const QVector3D& p2,
		   const QVector3D& p3);
    ~Plane() {}

    float a() const
        { return _vars[0]; }
    float b() const
        { return _vars[1]; }
    float c() const
        { return _vars[2]; }
    float d() const
        { return _vars[3]; }

    void setA(float val)
        { _vars[0] = val; }

    void setB(float val)
        { _vars[1] = val; }

    void setC(float val)
        { _vars[2] = val; }

    void setD(float val)
        { _vars[3] = val; }

    float distance(const QVector3D& point) const;

    // determine if plane intersects a bounding box (p1, p2)
    bool intersectsBox(const QVector3D& p1, const QVector3D& p2) const;

    /*! \brief project a point onto a plane
     *
     * \param p the point to project onto this plane
     * \return the projected point
     */
    QVector3D projectPointOnPlane(const QVector3D& p) const;

    /*! \brief intersect a line with this plane
     *
     * \param pt point on line
     * \param n direction of line
     * \param coplanar will be true if line lies in the plane
     * \param intpt intersection point
     * \return true if line and plane are parallel
     */
    bool intersectLine(const QVector3D& pt, const QVector3D& n, bool& coplanar, QVector3D& intpt) const;

    
    /*! \brief get the mirror of a point about the plane
     *
     * \param pt point to mirror
     * \return the mirrored point
     */
    QVector3D mirror(const QVector3D& pt) const;
    
private:

    float _vars[4];

};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

