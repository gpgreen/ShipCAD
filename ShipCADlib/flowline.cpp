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

#include "flowline.h"
#include "shipcadmodel.h"
#include "filebuffer.h"

using namespace ShipCAD;

Flowline::Flowline(ShipCADModel* owner)
    : _owner(owner), _projection_point(ZERO2), _projection_vw(fvProfile), _method_new(false),
      _flowline(0)
{
    _flowline = new Spline();
}

Flowline::~Flowline()
{
    delete _flowline;
}

Flowline* Flowline::construct(ShipCADModel* owner)
{
    Flowline* f = new Flowline(owner);
    return f;
}

void Flowline::clear()
{
    _projection_point = ZERO2;
    _projection_vw = fvProfile;
    _method_new = false;
    _flowline->clear();
}

void Flowline::draw(Viewport& vp, LineShader* lineshader)
{
    // TODO
}

void Flowline::setBuild(bool val)
{
    Entity::setBuild(val);
    if (!val)
        clear();
}

void Flowline::rebuild()
{
    // TODO
}

bool Flowline::isVisible() const
{
    return _owner->getVisibility().isShowFlowlines();
}

bool Flowline::isSelected() const
{
    return _owner->isSelectedFlowline(this);
}

void Flowline::setSelected(bool set)
{
    if (set)
        _owner->setSelectedFlowline(this);
    else
        _owner->removeSelectedFlowline(this);
}

QColor Flowline::getColor() const
{
    if (isSelected())
        return _owner->getPreferences().getSelectColor();
    else if (_method_new)
        return Qt::red;
    return Qt::blue;
}

void Flowline::loadBinary(FileBuffer& source)
{
    quint32 n;
    float f;
    bool b;
    QVector3D p;
    source.load(f);
    _projection_point.setX(f);
    source.load(f);
    _projection_point.setY(f);
    source.load(n);
    _projection_vw = static_cast<viewport_type_t>(n);
    source.load(_build);
    source.load(b);
    if (b)
        _owner->setSelectedFlowline(this);
    source.load(n);
    for (size_t i=0; i<n; i++) {
        source.load(p);
        _flowline->add(p);
        source.load(b);
        _flowline->setKnuckle(i, b);
    }
}

void Flowline::saveBinary(FileBuffer& dest)
{
    dest.add(_projection_point.x());
    dest.add(_projection_point.y());
    dest.add(static_cast<quint32>(_projection_vw));
    dest.add(_build);
    dest.add(isSelected());
    dest.add(_flowline->numberOfPoints());
    for (size_t i=0; i<_flowline->numberOfPoints(); i++) {
        dest.add(_flowline->getPoint(i));
        dest.add(_flowline->isKnuckle(i));
    }
}

