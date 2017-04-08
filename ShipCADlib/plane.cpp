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

#include "plane.h"
#include "shipcadlib.h"

using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

Plane::Plane()
{
    _vars[0] = _vars[1] = _vars[2] = _vars[3] = 0;
}

Plane::Plane(float a, float b, float c, float d)
{
    _vars[0] = a;
    _vars[1] = b;
    _vars[2] = c;
    _vars[3] = d;
}

Plane::Plane(const QVector3D& p, const QVector3D& normal)
{
	_vars[0] = normal.x();
	_vars[1] = normal.y();
	_vars[2] = normal.z();
	_vars[3] = -p.x() * _vars[0] - p.y() * _vars[1] - p.z() * _vars[2];
}

Plane::Plane(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
  // calculate normal
  QVector3D n;
  n.setX((p2.y() - p1.y()) * (p3.z() - p1.z()) - (p2.z() - p1.z()) * (p3.y() - p1.y()));
  n.setY((p2.z() - p1.z()) * (p3.x() - p1.x()) - (p2.x() - p1.x()) * (p3.z() - p1.z()));
  n.setZ((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x()));
  n.normalize();
  _vars[0] = n.x() / n.length();
  _vars[1] = n.y() / n.length();
  _vars[2] = n.z() / n.length();
  _vars[3] = -p1.x() * _vars[0] - p1.y() * _vars[1] - p1.z() * _vars[2];
}

float Plane::distance(const QVector3D& point) const
{
    return _vars[0] * point.x() + _vars[1] * point.y()
            + _vars[2] * point.z() + _vars[3];
}

bool Plane::intersectsBox(const QVector3D& p1, const QVector3D& p2) const
{
    QVector3D p[8];
    p[0] = p1;

    p[1].setX(p2.x());
    p[1].setY(p1.y());
    p[1].setZ(p1.z());

    p[2].setX(p2.x());
    p[2].setY(p2.y());
    p[2].setZ(p1.z());

    p[3].setX(p1.x());
    p[3].setY(p2.y());
    p[3].setZ(p1.z());

    p[4].setX(p1.x());
    p[4].setY(p1.y());
    p[4].setZ(p2.z());

    p[5].setX(p2.x());
    p[5].setY(p1.y());
    p[5].setZ(p2.z());

    p[6] = p2;

    p[7].setX(p1.x());
    p[7].setY(p2.y());
    p[7].setZ(p2.z());

    float smin, smax;
    for (size_t i=0; i<8; ++i) {
        float s = _vars[0] * p[i].x() + _vars[1] * p[i].y() + _vars[2] * p[i].z() + _vars[3];
        if (i == 0) {
            smin = s;
            smax = s;
        }
        if (s < smin)
            smin = s;
        if (s > smax)
            smax = s;
    }
    return (smin <= 0 && smax >= 0) || (smax <= 0 && smin >= 0);
}

QVector3D Plane::projectPointOnPlane(const QVector3D& p) const
{
    float q = _vars[0] * _vars[0] + _vars[1] * _vars[1] + _vars[2] * _vars[2];
    if (q != 0) {
        float r = (_vars[0] * p.x() + _vars[1] * p.y() + _vars[2] * p.z() + _vars[3]) / q;
        return QVector3D(p.x() - _vars[0] * r,
                p.y() - _vars[1] * r,
                p.z() - _vars[2] *r);
    }
    return ZERO;
}

bool Plane::intersectLine(const QVector3D& pt, const QVector3D& n, bool& coplanar, QVector3D& intpt) const
{
    bool parallel = false;
    coplanar = false;
    float dotp = QVector3D::dotProduct(n, QVector3D(_vars[0], _vars[1], _vars[2]));
    if (dotp == 0.0) {
        parallel = true;
        // check and see if coplanar
        if (distance(pt) <= 1E-5)
            coplanar = true;
    } else {
        float s1 = -(_vars[0] * pt.x() + _vars[1] * pt.y() + _vars[2] * pt.z() + _vars[3]) / dotp;
        intpt = pt + s1 * n;
    }
    return parallel;
}

QVector3D Plane::mirror(const QVector3D& pt) const
{
    QVector3D p2 = projectPointOnPlane(pt);
    QVector3D result = pt + 2*(p2 - pt);
    return result;
}

    
