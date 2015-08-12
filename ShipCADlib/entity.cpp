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
#include <iostream>

#include "entity.h"
#include "utility.h"

using namespace std;
using namespace ShipCAD;

static QVector3D ZERO = QVector3D();
static QVector3D ONE = QVector3D(1,1,1);

//////////////////////////////////////////////////////////////////////////////////////

Entity::Entity()
{
    clear();
}

void Entity::clear()
{
    _build = false;
    _min = ZERO;
    _max = ZERO;
    _color = Qt::black;
    _pen_width = 1;
    _pen_style = Qt::SolidLine;
}

void Entity::extents(QVector3D& min, QVector3D& max)
{
    if (!_build)
        rebuild();
    MinMax(_min, min, max);
    MinMax(_max, min, max);
}

bool Entity::isBuild()
{
    return _build;
}

void Entity::setBuild(bool val)
{
    if (val != _build) {
        _build = val;
        if (!val) {
            _min = ZERO;
            _max = ZERO;
        }
    }
}

QVector3D Entity::getMin()
{
    if (!_build)
        rebuild();
    return _min;
}

QVector3D Entity::getMax()
{
    if (!_build)
        rebuild();
    return _max;
}

void Entity::dump(ostream& os) const
{
    os << " Build:" << (_build ? "true" : "false")
       << "\n Min:[" << _min.x() << "," << _min.y() << "," << _min.z()
       << "]\n Max:[" << _max.x() << "," << _max.y() << "," << _max.z()
       << "]\n PenWidth:" << _pen_width
       << "\n PenStyle:" << _pen_style
       << "\n Color:[" << _color.red() << "," << _color.green() << "," << _color.blue()
       << "]";
}

ostream& operator << (ostream& os, const ShipCAD::Entity& entity)
{
    entity.dump(os);
    return os;
}
