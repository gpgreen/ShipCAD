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
#include <cmath>
#include <stdexcept>
#include <typeinfo>

#include "subdivedge.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "shader.h"

using namespace std;
using namespace ShipCAD;

bool ShipCAD::g_edge_verbose = true;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionEdge* SubdivisionEdge::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getEdgePool().malloc();
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionEdge::construct");
    return new (mem) SubdivisionEdge(owner);
}

SubdivisionEdge::SubdivisionEdge(SubdivisionSurface* owner)
    : SubdivisionBase(owner), _crease(false), _control_edge(false), _curve(0)
{
	// does nothing
}

SubdivisionEdge::~SubdivisionEdge()
{
    // does nothing
}

void SubdivisionEdge::clear()
{
    _points[0] = _points[1] = 0;
    _curve = 0;
    _faces.clear();
    _crease = false;
    _control_edge = false;
}

size_t SubdivisionEdge::getIndex()
{
    try {
        return _owner->indexOfEdge(this);
    }
    catch(range_error&) {
        return _owner->indexOfControlEdge(static_cast<SubdivisionControlEdge*>(this));
    }
}

bool SubdivisionEdge::isBoundaryEdge()
{
    bool result = false;
    if (_faces.size() == 1) {
        result = (fabs(_points[0]->getCoordinate().y()) > 1E-4f
                || fabs(_points[1]->getCoordinate().y()) > 1E-4f);
    }
    return result;
}

SubdivisionFace* SubdivisionEdge::getFace(size_t index)
{
    if (index < _faces.size())
        return _faces[index];
    throw range_error("SubdivisionEdge::getFace");
}

bool SubdivisionEdge::hasFace(SubdivisionFace* face)
{
    return (find(_faces.begin(), _faces.end(), face) != _faces.end());
}

void SubdivisionEdge::setCrease(bool val)
{
    if (_faces.size() == 1)
        val = true;			// boundary edges must always be crease edges
    if (val == _crease)
        return;
    _crease = val;
    int n = 0;
    SubdivisionPoint* startPoint = _points[0];
    SubdivisionPoint* endPoint = _points[1];
    for (size_t i = 0; i<startPoint->numberOfEdges(); ++i) {
        SubdivisionEdge* edge = startPoint->getEdge(i);
        if (edge->isCrease())
            n++;
    }
    if (startPoint->getVertexType() == svCorner) {
        if (startPoint->numberOfFaces() > 1 && n == 2)
            startPoint->setVertexType(svCrease);
    }
    else {
        if (n == 0)
            startPoint->setVertexType(svRegular);
        else if (n == 1)
            startPoint->setVertexType(svDart);
        else if (n == 2)
            startPoint->setVertexType(svCrease);
        else if (n > 2)
            startPoint->setVertexType(svCorner);
    }
    n = 0;
    for (size_t i=0; i<endPoint->numberOfEdges(); ++i) {
        SubdivisionEdge* edge = endPoint->getEdge(i);
        if (edge->isCrease())
            n++;
    }
    if (endPoint->getVertexType() == svCorner) {
        if (endPoint->numberOfFaces() > 1 && n == 2)
            endPoint->setVertexType(svCrease);
    }
    else {
        if (n == 0)
            endPoint->setVertexType(svRegular);
        else if (n == 1)
            endPoint->setVertexType(svDart);
        else if (n == 2)
            endPoint->setVertexType(svCrease);
        else if (n > 2)
            endPoint->setVertexType(svCorner);
    }
    // BUGBUG: this fails during subdivide
    //_owner->setBuild(false);
}

SubdivisionEdge* SubdivisionEdge::getPreviousEdge()
{
    SubdivisionPoint* p = _points[0];
    SubdivisionEdge* result = 0;
    if (p->isRegularPoint() && p->getVertexType() != svCorner) {
        // find previous edge
        for (size_t i=0; i<p->numberOfEdges(); ++i) {
            if (p->getEdge(i) == this)
                continue;
            SubdivisionEdge* edge = p->getEdge(i);
            if (edge->isCrease() == isCrease()) {
                bool shares_face = false;
                for (size_t j=0; j<_faces.size(); ++j) {
                    if (edge->hasFace(_faces[j])) {
                        shares_face = true;
                        break;
                    }
                }
                if (!shares_face) {
                    if (edge->startPoint() == startPoint())
                        edge->swapData();
                    result = edge;
                    break;
                }
            }
        }
    }
    return result;
}

SubdivisionEdge* SubdivisionEdge::getNextEdge()
{
    SubdivisionPoint* p = _points[1];
    SubdivisionEdge* result = 0;
    if (p->isRegularPoint() && p->getVertexType() != svCorner) {
        // find next edge
        for (size_t i=0; i<p->numberOfEdges(); ++i) {
            if (p->getEdge(i) == this)
                continue;
            SubdivisionEdge* edge = p->getEdge(i);
            if (edge->isCrease() == isCrease()) {
                bool shares_face = false;
                for (size_t j=0; j<_faces.size(); ++j) {
                    if (edge->hasFace(_faces[j])) {
                        shares_face = true;
                        break;
                    }
                }
                if (!shares_face) {
                    if (edge->startPoint() == startPoint())
                        edge->swapData();
                    result = edge;
                    break;
                }
            }
        }
    }
    return result;
}

void SubdivisionEdge::addFace(SubdivisionFace* face)
{
    if (!hasFace(face))
        _faces.push_back(face);
}

SubdivisionPoint* SubdivisionEdge::calculateEdgePoint()
{
    QVector3D point = 0.5 * (startPoint()->getCoordinate() + endPoint()->getCoordinate());
    SubdivisionPoint* result = SubdivisionPoint::construct(_owner);
    if (_crease)
        result->setVertexType(svCrease);
    if (_curve)
        _curve->insertEdgePoint(startPoint(), endPoint(), result);
    result->setCoordinate(point);
    return result;
}

void SubdivisionEdge::swapData()
{
    swap(_points[0], _points[1]);
}

void SubdivisionEdge::deleteFace(SubdivisionFace* face)
{
    if (hasFace(face)) {
        vector<SubdivisionFace*>::iterator del = find(_faces.begin(), _faces.end(), face);
        _faces.erase(del);
        if (_faces.size() == 1)
            _crease = true;
        else if (_faces.size() == 0)
            _crease = false;
    }
}

void SubdivisionEdge::draw(bool draw_mirror, Viewport& vp, LineShader* lineshader, const QColor& edgeColor)
{
    QVector3D p1 = startPoint()->getCoordinate();
    QVector3D p2 = endPoint()->getCoordinate();
    QVector<QVector3D> vertices;
    if (!draw_mirror && vp.getViewportType() == fvBodyplan) {
        float mfl = _owner->getMainframeLocation();
        if ((p1.x() < mfl && p2.x() > mfl) || (p1.x() > mfl && p2.x() < mfl)) {
            // straddles mainframe
            QVector3D m;
            if (p2.x() - p1.x() != 0)
                m = Interpolate(p1, p2, (mfl - p1.x()) / (p2.x() - p1.x()));
            else
                m = MidPoint(p1, p2);
            if (p1.x() <= mfl) {
                // p2 lies on port
                vertices << p2;
                vertices << m;
                // p1 lies on stbd
                p1.setY(-p1.y());
                m.setY(-m.y());
                vertices << p1;
                vertices << m;
            }
            else {
                // p2 lies on port
                vertices << p1;
                vertices << m;
                // p2 lies on stbd
                p2.setY(-p2.y());
                m.setY(-m.y());
                vertices << p2;
                vertices << m;
            }
        }
        else {
            if (p1.x() <= mfl)
                p1.setY(-p1.y());
            if (p2.x() <= mfl)
                p2.setY(-p2.y());
            vertices << p1;
            vertices << p2;
        }
    }
    else {
        // draw_mirror or not a bodyplan
        vertices << p1;
        vertices << p2;
        if (draw_mirror) {
            p1.setY(p1.y());
            p2.setY(p2.y());
            vertices << p1;
            vertices << p2;
        }
    }
    lineshader->renderLines(vertices, edgeColor);
}

void SubdivisionEdge::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionEdge ["
       << hex << this << "]\n";
    if (g_edge_verbose)
        priv_dump(os, prefix);
}

void SubdivisionEdge::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionBase::priv_dump(os, prefix);
    QString nprefix(prefix);
    nprefix.append(" ");
    const char* snprefix = nprefix.toStdString().c_str();
    os << "\n" << snprefix << "points\n";
    bool tmp = g_point_verbose;
    g_point_verbose = false;
    os << snprefix;
    _points[0]->dump(os, snprefix);
    os << "\n";
    _points[1]->dump(os, snprefix);
    os << "\n";
    g_point_verbose = tmp;
    os << snprefix << "faces (" << _faces.size() << ")\n";
    for (size_t i=0; i<_faces.size(); ++i) {
        os << snprefix;
        _faces[i]->dump(os, snprefix);
        os << "\n";
    }
    os << snprefix << "curve ";
    if (_curve == 0)
        os << "null";
    else {
        os << snprefix << " " << *_curve;
    }
    os << "\n" << snprefix << "Crease "
       << (_crease ? 'y' : 'n')
       << "\n" << snprefix << "ControlEdge "
       << (_control_edge ? 'y' : 'n');
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionEdge& edge)
{
    edge.dump(os);
    return os;
}

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionControlEdge* SubdivisionControlEdge::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getControlEdgePool().malloc();
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionControlEdge::construct");
    return new (mem) SubdivisionControlEdge(owner);
}

SubdivisionControlEdge::SubdivisionControlEdge(SubdivisionSurface* owner)
    : SubdivisionEdge(owner), _selected(false), _visible(true)
{
    _control_edge = true;
}

SubdivisionControlEdge::~SubdivisionControlEdge()
{
    if (_owner->hasControlEdge(this)) {
        // remove from owner list so that destructor
        // won't do anything if called again during
        // rest of method
        _owner->removeControlEdge(this);
        if (getCurve() != 0)
            getCurve()->deleteEdge(this);
        for (size_t i=_faces.size(); i>0; --i)
            _owner->deleteControlFace(dynamic_cast<SubdivisionControlFace*>(_faces[i-1]));
        SubdivisionControlPoint* sp = dynamic_cast<SubdivisionControlPoint*>(startPoint());
        SubdivisionControlPoint* ep = dynamic_cast<SubdivisionControlPoint*>(endPoint());
        // remove endpoint from startpoint neighbours
        ep->deleteEdge(this);
        if (ep->numberOfEdges() == 0)
            _owner->deleteControlPoint(ep);
        // remove startpoint from endpoint neighbours
        sp->deleteEdge(this);
        if (sp->numberOfEdges() == 0)
            _owner->deleteControlPoint(sp);
    }
}

QColor SubdivisionControlEdge::getColor()
{
    QColor result;
    if (_selected)
        result = _owner->getSelectedColor();
    else if (_faces.size() > 2)
        result = Qt::green;
    else if (_crease)
        result = _owner->getCreaseEdgeColor();
    else
        result = _owner->getEdgeColor();
    return result;
}

size_t SubdivisionControlEdge::getIndex()
{
    return _owner->indexOfControlEdge(this);
}

bool SubdivisionControlEdge::isBoundaryEdge()
{
    bool result = false;
    int n = 0;
    for (size_t i=0; i<_faces.size(); ++i) {
        SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(_faces[i]);
        if (face && face->getLayer()->useInHydrostatics())
            n++;
    }
    if (n == 1)
        result = (fabs(startPoint()->getCoordinate().y()) > 1E-4f
                  || fabs(endPoint()->getCoordinate().y()) > 1E-4f);
    return result;
}

void SubdivisionControlEdge::setSelected(bool val)
{
    if (val)
        _owner->setSelectedControlEdge(this);
    else
        _owner->removeSelectedControlEdge(this);
}

bool SubdivisionControlEdge::isSelected()
{
    return _owner->hasSelectedControlEdge(this);
}

bool SubdivisionControlEdge::isVisible()
{
    // meant for control edges only
    // a control edge is visible if at least one of it's
    //  neighbouring control faces belongs to a visible layer
    bool result = false;

    if (_owner->showControlNet()) {
        for (size_t i=0; i<_faces.size(); ++i) {
            SubdivisionControlFace* cface = dynamic_cast<SubdivisionControlFace*>(_faces[i]);
            if (cface != 0 && cface->getLayer() != 0) {
                if (cface->getLayer()->isVisible()) {
                    result = true;
                    break;
                }
            }
        }
    }

    // finally check if the edge is selected
    // selected edges must be visible at all times
    if (!result)
        result = isSelected();
    if (!result && getCurve() != 0)
        result = getCurve()->isSelected();
    return result;
}

SubdivisionControlPoint* SubdivisionControlEdge::insertControlPoint(const QVector3D& p)
{
    SubdivisionControlPoint* result = SubdivisionControlPoint::construct(_owner);
    SubdivisionControlPoint* sp = dynamic_cast<SubdivisionControlPoint*>(startPoint());
    SubdivisionControlPoint* ep = dynamic_cast<SubdivisionControlPoint*>(endPoint());
    if (sp == 0 || ep == 0)
        throw runtime_error("start and/or end point are not control points");
    result->setCoordinate(p);
    if (getCurve() != 0) {
        // insert the new point in the controlcurve
        getCurve()->insertControlPoint(sp, ep, result);
    }
    _owner->addControlPoint(result);
    for (size_t i=0; i<_faces.size(); ++i) {
        SubdivisionFace* face = _faces[i];
        if (face->hasPoint(sp) && face->hasPoint(ep)) {
            size_t i1 = face->indexOfPoint(sp);
            size_t i2 = face->indexOfPoint(ep);
            if (i2 == i1+1)
                face->insertPoint(i2, result);
            else if (i1 == i2+1)
                face->insertPoint(i1, result);
            else if (i1 == 0 && i2 == face->numberOfPoints() - 1)
                face->insertPoint(0, result);
            else if (i2 == 0 && i1 == face->numberOfPoints() - 1)
                face->insertPoint(0, result);
            result->addFace(face);
        }
    }
    ep->deleteEdge(this);
    SubdivisionControlEdge* edge = _owner->addControlEdge(result, ep);
    edge->setCrease(isCrease());
    edge->setCurve(getCurve());
    if (getCurve())
        result->setVertexType(svCrease);
    for (size_t i=0; i<_faces.size(); ++i)
        edge->addFace(_faces[i]);
    _points[1] = result;
    result->addEdge(this);
    return result;
}

void SubdivisionControlEdge::loadBinary(FileBuffer& source)
{
    size_t index;
    // read startpoint
    source.load(index);
    _points[0] = _owner->getControlPoint(index);
    _points[0]->addEdge(this);
    // read endpoint
    source.load(index);
    _points[1] = _owner->getControlPoint(index);
    _points[1]->addEdge(this);
    source.load(_crease);
    source.load(_selected);
    if (_selected)
        _owner->setSelectedControlEdge(this);
}

void SubdivisionControlEdge::loadFromStream(size_t &lineno, QStringList &strings)
{
    QString str = strings[lineno++].trimmed();
    size_t start = 0;
    // startpoint
    size_t index = ReadIntFromStr(lineno, str, start);
    _points[0] = _owner->getControlPoint(index);
    _points[0]->addEdge(this);
    // endpoint
    index = ReadIntFromStr(lineno, str, start);
    _points[1] = _owner->getControlPoint(index);
    _points[1]->addEdge(this);
    // crease
    _crease = ReadBoolFromStr(lineno, str, start);
    if (start < static_cast<size_t>(str.length())) {
        // flag to indicate that this edge was selected when the model was saved (for undo-purposes)
        _selected = ReadBoolFromStr(lineno, str, start);
        if (_selected)
            _owner->setSelectedControlEdge(this);
    }
}

void SubdivisionControlEdge::saveToStream(QStringList &strings)
{
    SubdivisionControlPoint* sp = dynamic_cast<SubdivisionControlPoint*>(_points[0]);
    SubdivisionControlPoint* ep = dynamic_cast<SubdivisionControlPoint*>(_points[1]);
    strings.push_back(QString("%1 %2 %3 %4")
                      .arg(_owner->indexOfControlPoint(sp))
                        .arg(_owner->indexOfControlPoint(ep))
                        .arg(BoolToStr(_crease)).arg(BoolToStr(_selected)));
}

void SubdivisionControlEdge::saveBinary(FileBuffer& destination)
{
    destination.add(_owner->indexOfPoint(_points[0]));
    destination.add(_owner->indexOfPoint(_points[1]));
    destination.add(isCrease());
    destination.add(isSelected());
}

void SubdivisionControlEdge::priv_trace(SubdivisionControlPoint* p)
{
    SubdivisionControlEdge* edge;
    if (p->isRegularPoint() && p->getVertexType() != svCorner) {
        // find next edge
        for (size_t i=0; i<p->numberOfEdges(); ++i) {
            if (p->getEdge(i) != this) {
                edge = dynamic_cast<SubdivisionControlEdge*>(p->getEdge(i));
                if (edge->isSelected() != isSelected() && edge->isCrease() == isCrease()) {
                    bool shares_face = false;
                    for (size_t j=0; j<numberOfFaces(); ++j) {
                        if (edge->hasFace(_faces[j])) {
                            shares_face = true;
                            break;
                        }
                    }
                    if (!shares_face) {
                        if (edge->startPoint() == startPoint())
                            edge->swapData();
                        edge->setSelected(isSelected());
                        edge->trace();
                        break;
                    }
                }
            }
        }
    }
}

void SubdivisionControlEdge::trace()
{
    SubdivisionControlPoint* p;

    p = dynamic_cast<SubdivisionControlPoint*>(startPoint());
    priv_trace(p);
    p = dynamic_cast<SubdivisionControlPoint*>(endPoint());
    priv_trace(p);
}

void SubdivisionControlEdge::draw(Viewport& vp, LineShader* lineshader)
{
    if (!isVisible())
        return;
    QVector3D p1 = startPoint()->getCoordinate();
    QVector3D p2 = endPoint()->getCoordinate();
    QVector<QVector3D> vertices;
    if (vp.getViewportMode() == vmWireFrame) {
        if (isCrease()) {
            glLineWidth(2);
        }
        else {
            glLineWidth(1);
        }
        if (vp.getViewportType() == fvBodyplan) {
            float mfl = _owner->getMainframeLocation();
            if ((p1.x() < mfl && p2.x() > mfl) || (p1.x() > mfl && p2.x() < mfl)) {
                // straddles mainframe
                QVector3D m;
                if (p2.x() - p1.x() != 0)
                    m = Interpolate(p1, p2, (mfl - p1.x()) / (p2.x() - p1.x()));
                else
                    m = MidPoint(p1, p2);
                if (p1.x() <= mfl) {
                    // p2 lies on port
                    vertices << p2;
                    vertices << m;
                    // p1 lies on stbd
                    p1.setY(-p1.y());
                    m.setY(-m.y());
                    vertices << p1;
                    vertices << m;
                }
                else {
                    // p2 lies on port
                    vertices << p1;
                    vertices << m;
                    // p2 lies on stbd
                    p2.setY(-p2.y());
                    m.setY(-m.y());
                    vertices << p2;
                    vertices << m;
                }
            }
            else {
                if (p1.x() <= mfl)
                    p1.setY(-p1.y());
                if (p2.x() <= mfl)
                    p2.setY(-p2.y());
                vertices << p1;
                vertices << p2;
            }
        }
        else {
            vertices << p1;
            vertices << p2;
        }
    }
    else {
        // viewport is not vmWireFrame
        glLineWidth(1);
        vertices << p1;
        vertices << p2;
    }
    lineshader->renderLines(vertices, getColor());
}

void SubdivisionControlEdge::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionControlEdge ["
       << hex << this << "]\n";
    if (g_edge_verbose)
        priv_dump(os, prefix);
}

void SubdivisionControlEdge::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionEdge::priv_dump(os, prefix);
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionControlEdge& edge)
{
    edge.dump(os);
    return os;
}
