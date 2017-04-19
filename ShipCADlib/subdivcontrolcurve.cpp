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

#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "subdivcontrolcurve.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "spline.h"
#include "viewport.h"
#include "filebuffer.h"
#include "shader.h"
#include "entity.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionControlCurve* SubdivisionControlCurve::construct(SubdivisionSurface* owner)
{
    void * cmem = owner->getControlCurvePool().add();
    if (cmem == 0)
        throw runtime_error("out of memory in SubdivisionControlCurve::construct");
    void * smem = owner->getSplinePool().add();
    if (smem == 0)
        throw runtime_error("out of memory in SubdivisionControlCurve::construct:spline");
    Spline* spline = new (smem) Spline();
    return new (cmem) SubdivisionControlCurve(owner, spline);
}

SubdivisionControlCurve::SubdivisionControlCurve(SubdivisionSurface* owner, Spline* spline)
    : SubdivisionBase(owner), _build(false), _curve(spline)
{
	// does nothing
}

void SubdivisionControlCurve::removeCurve()
{
    // spline will have already been removed in surface remove
    // remove references from control edges
    for (size_t i=1; i<_points.size(); ++i) {
        SubdivisionControlPoint* p1 = _points[i-1];
        SubdivisionControlPoint* p2 = _points[i];
        SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
        if (edge != 0)
            edge->setCurve(0);
    }
    // remove references from subdivided edges
    for (size_t i=1; i<_div_points.size(); ++i) {
        SubdivisionPoint* p1 = _div_points[i-1];
        SubdivisionPoint* p2 = _div_points[i];
        SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
        if (edge != 0)
            edge->setCurve(0);
    }
}

void SubdivisionControlCurve::setSelected(bool val)
{
    if (val)
        _owner->setSelectedControlCurve(this);
    else
        _owner->removeSelectedControlCurve(this);
}

void SubdivisionControlCurve::addPoint(SubdivisionControlPoint* p)
{
    _points.push_back(p);
    setBuild(false);
}

void SubdivisionControlCurve::resetDivPoints()
{
    _div_points.clear();
    for (size_t i=0; i<_points.size(); ++i)
        _div_points.push_back(_points[i]);
    setBuild(false);
}

QColor SubdivisionControlCurve::getColor()
{
    if (isSelected())
        return _owner->getSelectedColor();
    return _owner->getControlCurveColor();
}

bool SubdivisionControlCurve::isSelected()
{
    return _owner->hasSelectedControlCurve(this);
}

bool SubdivisionControlCurve::isVisible()
{
    return _owner->showControlCurves();
}

SubdivisionControlPoint* SubdivisionControlCurve::getControlPoint(size_t index)
{
    if (index < _points.size())
        return _points[index];
    throw range_error("index for SubdivisionControlCurve::getControlPoint");
}

SubdivisionPoint* SubdivisionControlCurve::getSubdivPoint(size_t index)
{
    if (index < _div_points.size())
        return _div_points[index];
    throw range_error("index for SubdivisionControlCurve::getSubdivPoint");
}

void SubdivisionControlCurve::clear()
{
    _points.clear();
    _curve->clear();
    _div_points.clear();
    setBuild(false);
}

void SubdivisionControlCurve::deleteEdge(SubdivisionControlEdge* edge)
{
    bool delcurve = false;
    SubdivisionControlPoint* p1, *p2;
    SubdivisionEdge* anedge;

    size_t i = 2;
    while (i <= _points.size()) {
        if ((_points[i-2] == edge->startPoint() && _points[i-1] == edge->endPoint())
             || (_points[i-2] == edge->endPoint() && _points[i-1] == edge->startPoint())) {
            // remove references to this curve from control edges
            for (size_t j=2; j<=_points.size(); ++j) {
                anedge = _owner->edgeExists(_points[j-2], _points[j-1]);
                if (anedge != 0)
                    anedge->setCurve(0);
            }
            // remove references from subdivided edges
            for (size_t j=2; j<=_div_points.size(); ++j) {
                anedge = _owner->edgeExists(_div_points[j-2], _div_points[j-1]);
                if (anedge != 0)
                    anedge->setCurve(0);
            }
            _div_points.clear();

            if (i - 2 > 0) {
                // build first new curve
                SubdivisionControlCurve* newcurve = SubdivisionControlCurve::construct(_owner);
                _owner->addControlCurve(newcurve);
                p1 = 0;
                for (size_t j=0; j<=i-2; ++j) {
                    p2 = _points[j];
                    newcurve->addPoint(p2);
                    if (j > 0) {
                        anedge = _owner->edgeExists(p1, p2);
                        if (anedge != 0)
                            anedge->setCurve(newcurve);
                    }
                    p1 = p2;
                }
                newcurve->setSelected(isSelected());
            }
            if (i - 1 < _points.size() - 1) {
                // build second new curve
                SubdivisionControlCurve* newcurve = SubdivisionControlCurve::construct(_owner);
                _owner->addControlCurve(newcurve);
                p1 = 0;
                for (size_t j=i-1; j<=_points.size()-1; ++j) {
                    p2 = _points[j];
                    newcurve->addPoint(p2);
                    if (j > i - 1) {
                        anedge = _owner->edgeExists(p1, p2);
                        if (anedge != 0)
                            anedge->setCurve(newcurve);
                    }
                    p1 = p2;
                }
                newcurve->setSelected(isSelected());
            }
            delcurve = true;
            break;
        }
        ++i;
    }   // end while
    if (delcurve) {
        _owner->deleteControlCurve(this);
    }
}

void SubdivisionControlCurve::draw(Viewport &vp, LineShader* lineshader)
{
    if (numberOfControlPoints() <= 1)
        return;
    bool sel = isSelected();
    Spline* curve = getSpline();
    curve->setColor(_owner->getControlCurveColor());
    curve->setShowCurvature(sel && _owner->showCurvature());
    if (curve->showCurvature())
        curve->setFragments(600);
    else
        curve->setFragments(250);
    curve->setCurvatureColor(_owner->getCurvatureColor());
    curve->setCurvatureScale(_owner->getCurvatureScale());

    QVector<QVector3D>& vertices = lineshader->getVertexBuffer();

    if (!_owner->showControlNet() && sel) {
        // draw controlpoints and edges
        for (size_t i=2; i<=_points.size(); ++i) {
            SubdivisionPoint* p1 = _points[i-2];
            SubdivisionPoint* p2 = _points[i-1];
            SubdivisionControlEdge* edge = _owner->controlEdgeExists(p1, p2);
            if (edge != 0)
                edge->draw(vp, lineshader);
            if (i == 2)
                vertices << p1->getCoordinate();
            vertices << p2->getCoordinate();
        }
        lineshader->renderPoints(vertices, getColor());
    }
    vertices.clear();
    if (vp.getViewportType() == fvBodyplan && !_owner->drawMirror()) {

        vector<QVector3D> parray1;
        vector<QVector3D> parray2;

        // draw mainframe location
        Plane mfl(1, 0, 0, -_owner->getMainframeLocation());
        IntersectionData output;
        vector<float> parameters;
        parameters.push_back(0);
        parameters.push_back(1.0);
        if (curve->intersect_plane(mfl, output)) {
            parameters.insert(parameters.end(), output.parameters.begin(), output.parameters.end());
            sort(parameters.begin(), parameters.end());
        }
        QVector<QVector3D> curvelines;
        for (size_t i=2; i<=parameters.size(); ++i) {
            QVector3D p3d = curve->value(0.5*(parameters[i-2] + parameters[i-1]));
            int scale;
            if (p3d.x() < _owner->getMainframeLocation())
                scale = -1;
            else
                scale = 1;
            size_t fragm = (parameters[i-1] - parameters[i-2]) * curve->getFragments();
            if (fragm < 10)
                fragm = 10;
            for (size_t j=0; j<fragm; j++) {
                float t = parameters[i-1] + (parameters[i] - parameters[i-1]) * j / (fragm - 1);
                QVector3D p = curve->value(t);
                p.setY(p.y() * scale);
                parray1.push_back(p);
            }
            if (curve->showCurvature()) {
                for (size_t j=0; j<fragm; ++j) {
                    float t = parameters[i-2] + (parameters[i-1] - parameters[i-2]) * j / (fragm - 1);
                    QVector3D normal;
                    float c = curve->curvature(t, normal);
                    normal.setY(normal.y() * scale);
                    QVector3D p3d2 = parray1[j] - (c * 2 * curve->getCurvatureScale() * normal);
                    parray2.push_back(p3d2);
                }
                for (size_t j=1; j<=fragm; j++) {
                    if ((j % 4) == 0 || j == 1 || j == fragm) {
                        // draw normal lines
                        vertices << parray1[j-1];
                        vertices << parray2[j-1];
                    }
                }
                for (size_t j=1; j<parray2.size(); j++) {
                    vertices << parray2[j-1];
                    vertices << parray2[j];
                }
                glLineWidth(1);
                lineshader->renderLines(vertices, curve->getCurvatureColor());
            }
            else {
                for (size_t j=1; j<=parray1.size(); ++j) {
                    vertices << parray1[j-1];
                    vertices << parray1[j];
                }
                glLineWidth(1);
                lineshader->renderLines(vertices, getColor());
            }
        }
    }
    else {
        curve->draw(vp, lineshader);
    }
    if (_owner->drawMirror()) {
        // draw reversed curve
        for (size_t i=1; i<=curve->numberOfPoints(); ++i) {
            QVector3D p3d = curve->getPoint(i-1);
            p3d.setY(-p3d.y());
            curve->setPoint(i-1, p3d);
        }
        curve->draw(vp, lineshader);
        for (size_t i=1; i<=curve->numberOfPoints(); ++i) {
            QVector3D p3d = curve->getPoint(i-1);
            p3d.setY(-p3d.y());
            curve->setPoint(i-1, p3d);
        }
    }
}

void SubdivisionControlCurve::insertControlPoint(SubdivisionControlPoint *p1,
                                                 SubdivisionControlPoint *p2,
                                                 SubdivisionControlPoint *newpt)
{
    size_t i = 2;
    while (i <= _points.size()) {
        if ((_points[i-2] == p1 && _points[i-1] == p2) || (_points[i-1] == p1 && _points[i-2] == p2))
            _points.insert(_points.begin()+i-1, newpt);
        i++;
    }
}

void SubdivisionControlCurve::insertEdgePoint(SubdivisionPoint *p1,
                                              SubdivisionPoint *p2,
                                              SubdivisionPoint *newpt)
{
    size_t i = 2;
    while (i <= _div_points.size()) {
        if ((_div_points[i-2] == p1 && _div_points[i-1] == p2) || (_div_points[i-1] == p1 && _div_points[i-2] == p2))
            _div_points.insert(_div_points.begin()+i-1, newpt);
        i++;
    }
}

void SubdivisionControlCurve::loadBinary(FileBuffer &source)
{
    quint32 n, ind;
    source.load(n);
    SubdivisionControlPoint* p1 = 0;
    for (size_t i=1; i<=n; ++i) {
        source.load(ind);
        SubdivisionControlPoint* p2 = _owner->getControlPoint(ind);
        _points.push_back(p2);
        if (i > 1) {
            SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
            if (edge != 0)
                edge->setCurve(this);
        }
        p1 = p2;
    }
    for (size_t i=0; i<_points.size(); ++i)
        _div_points.push_back(_points[i]);
    bool sel;
    source.load(sel);
    if (sel)
        setSelected(true);
}

void SubdivisionControlCurve::replaceVertexPoint(SubdivisionPoint *oldpt, SubdivisionPoint *newpt)
{
    for (size_t i=1; i<=_div_points.size(); ++i) {
        if (_div_points[i-1] == oldpt) {
            _div_points[i-1] = newpt;
            _curve->clear();
        }
    }
}

void SubdivisionControlCurve::saveBinary(FileBuffer &destination)
{
    destination.add(numberOfControlPoints());
    for (size_t i=1; i<=numberOfControlPoints(); ++i) {
        SubdivisionControlPoint* p = _points[i-1];
        size_t ind = _owner->indexOfControlPoint(p);
        destination.add(ind);
    }
    destination.add(isSelected());
}

void SubdivisionControlCurve::saveToDXF(QStringList& strings)
{
    QString layer("Control_curves");
    _curve->setFragments(_curve->numberOfPoints());
    _curve->saveToDXF(strings, layer, _owner->drawMirror());
}

void SubdivisionControlCurve::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionControlCurve ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionControlCurve::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionBase::priv_dump(os, prefix);
    //os << "SubdivisionControlCurve ["
    //   << hex << this << "]\n";
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionControlCurve& curve)
{
    curve.dump(os);
    return os;
}

