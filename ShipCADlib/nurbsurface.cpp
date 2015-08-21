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

#include "nurbsurface.h"
#include "viewport.h"

using namespace ShipCAD;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////

NURBSurface::NURBSurface()
    : Entity()
{
	// does nothing
}

void NURBSurface::setBuild(bool val)
{
    if (!val) {
    }
    Entity::setBuild(val);
}

void NURBSurface::setPoint(size_t /*row*/, size_t /*col*/, const QVector3D& /*p*/)
{
}

QVector3D NURBSurface::getPoint(size_t /*row*/, size_t /*col*/)
{
    return QVector3D();
}

void NURBSurface::rebuild()
{
    _build = false;
    _build = true;
}

void NURBSurface::draw(Viewport& /*vp*/)
{
}

void NURBSurface::clear()
{
}

void NURBSurface::dump(ostream& os) const
{
    os << "NURBSurface";
    Entity::dump(os);
}

ostream& operator << (ostream& os, const ShipCAD::NURBSurface& surface)
{
    surface.dump(os);
    return os;
}
