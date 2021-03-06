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

#include "subdivbase.h"
#include "subdivsurface.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionBase::SubdivisionBase(SubdivisionSurface* owner)
    : _owner(owner)
{
    // does nothing
}

SubdivisionBase::~SubdivisionBase()
{
    // does nothing
}

void SubdivisionBase::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionBase [" << hex << this << "]";
    priv_dump(os, prefix);
}

void SubdivisionBase::priv_dump(ostream& os, const char* prefix) const
{
    os << prefix << " Owner [" << hex << _owner << "]";
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionBase& base)
{
    base.dump(os);
    return os;
}
