/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
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

#include "backgroundimage.h"
#include "filebuffer.h"

using namespace ShipCAD;
using namespace std;

BackgroundImage::BackgroundImage(ShipCADModel* owner)
    : _owner(owner), _assigned_view(fvPerspective), _quality(100),
      _origin(ZERO2), _scale(10), _transparent(false), _blending_value(255),
      _transparent_color(Qt::black), _visible(true), _tolerance(3)
{
    // does nothing
}

BackgroundImage::~BackgroundImage()
{
    // does nothing
}

void BackgroundImage::loadBinary(FileBuffer& source)
{
    quint32 n;
    source.load(n);
    _assigned_view = static_cast<viewport_type_t>(n);
    source.load(_visible);
    source.load(_quality);
    float f;
    source.load(f);
    _origin.setX(f);
    source.load(f);
    _origin.setY(f);
    source.load(_scale);
    source.load(_blending_value);
    source.load(_transparent);
    source.load(_transparent_color);
    source.load(_tolerance);
    source.load(_image);
}

void BackgroundImage::saveBinary(FileBuffer& dest)
{
    dest.add(static_cast<quint32>(_assigned_view));
    dest.add(_visible);
    dest.add(_quality);
    dest.add(_origin.x());
    dest.add(_origin.y());
    dest.add(_scale);
    dest.add(_blending_value);
    dest.add(_transparent);
    dest.add(_transparent_color);
    dest.add(_tolerance);
    dest.add(_image);
}

void BackgroundImage::updateData(Viewport& /*vp*/)
{
    // TODO
}

void BackgroundImage::updateViews()
{
    // TODO
}


