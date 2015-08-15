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

#include "intersection.h"
#include "filebuffer.h"
#include "spline.h"
#include "shipcadmodel.h"
#include "viewport.h"
#include "shader.h"
#include "projsettings.h"
#include "preferences.h"
#include "subdivsurface.h"

using namespace std;
using namespace ShipCAD;

Intersection::Intersection(ShipCADModel* owner)
    : _owner(owner)
{
    clear();
}

Intersection::~Intersection()
{
    clear();
}

Intersection* Intersection::construct(ShipCADModel *owner)
{
    Intersection* i = new Intersection(owner);
    return i;
}

void Intersection::clear()
{
    for (size_t i=0; i<_items.size(); i++)
        delete _items[i];
    _items.clear();
    setBuild(false);
    _show_curvature = false;
    _use_hydrostatic_surfaces_only = false;
}

void Intersection::setBuild(bool val)
{
    if (val)
        clear();
	Entity::setBuild(val);
}

QColor Intersection::getColor()
{
    switch(_intersection_type) {
    case fiStation:
        return _owner->getPreferences().getStationColor();
    case fiButtock:
        return _owner->getPreferences().getButtockColor();
    case fiWaterline:
        return _owner->getPreferences().getWaterlineColor();
    case fiDiagonal:
        return _owner->getPreferences().getDiagonalColor();
    }
    return Qt::white;
}

void Intersection::extents(QVector3D& min, QVector3D& max)
{
    if (!_build)
        rebuild();
    for (size_t i=0; i<_items.size(); i++)
        _items[i]->extents(min, max);
}

QString Intersection::getDescription()
{
    return "";
}

void Intersection::draw(Viewport& vp, LineShader* lineshader)
{
    // TODO
}

void Intersection::rebuild()
{
    setBuild(false);
    _owner->getSurface()->intersectPlane(_plane, _use_hydrostatic_surfaces_only, _items);
    // use a low simplification factor to remove only points that are (nearly) on a line
    if (_owner->getProjectSettings().getSimplifyIntersections()) {
        for (size_t i=0; i<_items.size(); i++)
            _items[i]->simplify(2.0);
    }
    _build = true;
}

void Intersection::add(Spline* sp)
{
    _items.push_back(sp);
}

void Intersection::calculateArea(const Plane& plane, float* area, QVector3D* cog, QVector2D* moment_of_inertia)
{
    // TODO
}

void Intersection::createStarboardPart()
{
    // TODO
}

void Intersection::deleteItem(Spline* item)
{
    vector<Spline*>::iterator i = find(_items.begin(), _items.end(), item);
    if (i == _items.end())
        return;
    delete *i;
    _items.erase(i);
}

void Intersection::loadBinary(FileBuffer& source)
{
    size_t i;
    QVector3D p;
    source.load(i);
    _intersection_type = static_cast<intersection_type_t>(i);
    if (_owner->getFileVersion() >= fv191)
        source.load(_show_curvature);
    else
        _show_curvature = false;
    source.load(_plane);
    source.load(_build);
    size_t n;
    source.load(n);
    for (i=0; i<n; i++) {
        Spline* sp = new Spline();
        _items.push_back(sp);
        // read number of points for this spline
        size_t m;
        source.load(m);
        for (size_t j=0; j<m; j++) {
            float x, y, z;
            if (_owner->getFileVersion() >= fv160) {
                if (_intersection_type == fiStation) {
                    p.setX(-_plane.d());
                    source.load(y);
                    p.setY(y);
                    source.load(z);
                    p.setZ(z);
                } else if (_intersection_type == fiButtock) {
                    source.load(x);
                    p.setX(x);
                    p.setY(-_plane.d());
                    source.load(z);
                    p.setZ(z);
                } else if (_intersection_type == fiWaterline) {
                    source.load(x);
                    p.setX(x);
                    source.load(y);
                    p.setY(y);
                    p.setZ(-_plane.d());
                } else {
                    source.load(p);
                }
            } else {
                source.load(p);
            }
            sp->setPoint(j, p);
            bool b;
            source.load(b);
            sp->setKnuckle(j, b);
        }
    }
}

void Intersection::saveBinary(FileBuffer& dest)
{
    dest.add(static_cast<int>(_intersection_type));
    if (_owner->getFileVersion() >= fv191)
        dest.add(_show_curvature);
    dest.add(_plane);
    dest.add(_build);
    dest.add(_items.size());
    for (size_t i=0; i<_items.size(); i++) {
        Spline* sp = _items[i];
        dest.add(sp->numberOfPoints());
        for (size_t j=0; j<sp->numberOfPoints(); j++) {
            QVector3D p = sp->getPoint(j);
            if (_owner->getFileVersion() >= fv160) {
                switch (_intersection_type) {
                case fiStation:
                    dest.add(p.y());
                    dest.add(p.z());
                    dest.add(sp->isKnuckle(j));
                    break;
                case fiButtock:
                    dest.add(p.x());
                    dest.add(p.z());
                    dest.add(sp->isKnuckle(j));
                    break;
                case fiWaterline:
                    dest.add(p.x());
                    dest.add(p.y());
                    dest.add(sp->isKnuckle(j));
                    break;
                case fiDiagonal:
                    dest.add(p);
                    dest.add(sp->isKnuckle(j));
                    break;
                }
            } else {
                dest.add(p);
                dest.add(sp->isKnuckle(j));
            }
        }
    }
}

void Intersection::saveToDXF(QStringList& strings)
{
    // TODO
}

