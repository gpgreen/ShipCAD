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
#include "shipcadmodel.h"
#include "viewport.h"
#include "shader.h"

using namespace std;
using namespace ShipCAD;

Marker::Marker(ShipCADModel* owner)
    : _owner(owner), _visible(true)
{
	// does nothing
}

Marker* Marker::construct(ShipCADModel* owner)
{
    Marker* m = new Marker(owner);
    return m;
}

void Marker::clear()
{
    _visible = true;
    Spline::clear();
}

void Marker::draw(Viewport& vp, LineShader* lineshader)
{
    if (!isVisible())
        return;
    if (isSelected())
        setColor(_owner->getPreferences().getSelectColor());
    else
        setColor(_owner->getPreferences().getMarkerColor());
    setFragments(250);

    QVector<QVector3D>& vertices = lineshader->getVertexBuffer();

    QVector3D p3d;
    if (vp.getViewportType() == fvBodyplan && _owner->getVisibility().getModelView() != mvBoth) {

        vector<float> params;
        vector<QVector3D> parray1;
        vector<QVector3D> parray2;

        Plane pln(1,0,0,-_owner->getProjectSettings().getMainframeLocation());
        params.push_back(0);
        params.push_back(1);

        IntersectionData output;
        if (intersect_plane(pln, output)) {
            params.insert(params.end(), output.parameters.begin(), output.parameters.end());
            sort(params.begin(), params.end());
        }

        for (size_t i=1; i<params.size(); i++) {
            p3d = value(0.5 * params[i-1] + params[i]);
            float scale = 1;
            if (p3d.x() < _owner->getProjectSettings().getMainframeLocation())
                scale = -1;
            size_t fragm = floor((params[i] - params[i-1])*getFragments());
            if (fragm < 10)
                fragm = 10;
            for (size_t j=0; j<fragm; j++) {
                float t = params[i-1] + (params[i] - params[i-1]) * j / (fragm - 1);
                QVector3D p = value(t);
                p.setY(p.y() * scale);
                parray1.push_back(p);
            }
            if (showCurvature()) {
                for (size_t j=0; j<fragm; j++) {
                    float t = params[i-1] + (params[i] - params[i-1]) * j / (fragm - 1);
                    QVector3D normal;
                    float c = curvature(t, normal);
                    normal.setY(normal.y() * scale);
                    QVector3D p2 = parray1[j] - (c * 2 * getCurvatureScale() * normal);
                    parray2.push_back(p2);
                }
                for (size_t j=1; j<=fragm; j++) {
                    if ((j % 4) == 0 || j == 1 || j == fragm) {
                        vertices << parray1[j-1];
                        vertices << parray2[j-1];
                    }
                }
                for (size_t j=1; j<parray2.size(); j++) {
                    vertices << parray2[j-1];
                    vertices << parray2[j];
                }
                glLineWidth(1);
                lineshader->renderLines(vertices, getCurvatureColor());
            } else { // end show curvature
                for (size_t j=1; j<parray1.size(); j++) {
                    vertices << parray1[j-1];
                    vertices << parray1[j];
                }
                glLineWidth(1);
                lineshader->renderLines(vertices, getColor());
            }
        }
        vertices.clear();
        for (size_t i=0; i<numberOfPoints(); i++) {
            p3d = getPoint(i);
            if (p3d.x() < _owner->getProjectSettings().getMainframeLocation())
                p3d.setY(-p3d.y());
            vertices << p3d;
        }
        glPointSize(3);
        lineshader->renderPoints(vertices, getColor());
    } else { // end bodyplan
        Spline::draw(vp, lineshader);
        for (size_t i=0; i<numberOfPoints(); i++) {
            vertices << getPoint(i);
        }
        glPointSize(3);
        lineshader->renderPoints(vertices, getColor());
    }
}

bool Marker::isSelected() const
{
    return _owner->isSelectedMarker(const_cast<Marker*>(this));
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
            _owner->setSelectedMarker(this);
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

void Marker::loadFromText(ShipCADModel* model, QTextStream& file, MarkerVector& markers)
{
    size_t lineno = 0;
    bool unexpected = false;
    Marker* marker = nullptr;
    while (!unexpected && !file.atEnd()) {
        QString line = file.readLine();
        lineno++;
        // if an empty line, complete the marker
        // and get ready for more
        if (line.size() == 0 && marker != nullptr) {
            if (marker->numberOfPoints() > 1) {
                markers.add(marker);
            } else {
                delete marker;
            }
            marker = nullptr;
            continue;
        }
        if (line == "EOF")
            break;
        // split the line
        QStringList fields = line.split(" ");
        if (fields.size() != 3) {
            // try tabs
            fields = line.split("\t");
            if (fields.size() != 3) {
                unexpected = true;
                continue;
            }
        }
        // we have 3 pieces, turn it into a point
        bool ok;
        float x = fields[0].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        float y = fields[1].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        float z = fields[2].toFloat(&ok);
        if (!ok) {
            unexpected = true;
            continue;
        }
        // add the point to the marker
        QVector3D pt(x, y, z);
        if (marker == nullptr) {
            marker = Marker::construct(model);
        }
        marker->add(pt);
    }
    // complete the last marker
    if (!unexpected && marker != nullptr) {
        if (marker->numberOfPoints() > 1) {
            markers.add(marker);
        } else
            delete marker;
    }
    // got something wrong, bail
    if (unexpected) {
        throw runtime_error((tr("bad input at line %1").arg(lineno)).toStdString());
    }
}

