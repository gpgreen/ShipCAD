/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "marker.h"
#include "filebuffer.h"
#include "spline.h"
#include "shipcad.h"
#include "viewport.h"
#include "shader.h"

using namespace std;
using namespace ShipCADGeometry;

Marker::Marker(ShipCAD* owner)
    : _owner(owner)
{
    clear();
}

Marker::~Marker()
{
    _owner->deleteMarker(this);
}

void Marker::clear()
{
    _visible = true;
    Spline::clear();
}

void Marker::draw(Viewport& vp, LineShader* lineshader)
{
    // TODO
}

bool Marker::isSelected()
{
    return _owner->isSelectedMarker(this);
}

void Marker::setSelected(bool set)
{
    if (set)
        _owner->setSelectedMarker(this);
    else
        _owner->removeSelectedMarker(this);
}

void Marker::loadBinary(FileBuffer& source)
{
    source.load(_visible);
    if (_owner->getFileVersion() >= fv260) {
        bool sel;
        source.load(sel);
        if (sel)
            _owner->addSelectedMarker(this);
    }
    Spline::loadBinary(source);
}

void Marker::saveBinary(FileBuffer& dest)
{
    dest.add(_visible);
    if (_owner->getFileVersion() >= fv260) {
        dest.add(isSelected());
    }
    Spline::saveBinary(dest);
}

