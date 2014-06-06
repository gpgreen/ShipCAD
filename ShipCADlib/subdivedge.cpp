#include <iostream>
#include <cmath>
#include <stdexcept>

#include "subdivedge.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "viewport.h"

using namespace std;
using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionEdge::SubdivisionEdge(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
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
    catch( range_error& e ) {
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
    if (startPoint->getVertexType() == SubdivisionPoint::svCorner) {
        if (startPoint->numberOfFaces() > 1 && n == 2)
            startPoint->setVertexType(SubdivisionPoint::svCrease);
    }
    else {
        if (n == 0)
            startPoint->setVertexType(SubdivisionPoint::svRegular);
        else if (n == 1)
            startPoint->setVertexType(SubdivisionPoint::svDart);
        else if (n == 2)
            startPoint->setVertexType(SubdivisionPoint::svCrease);
        else if (n > 2)
            startPoint->setVertexType(SubdivisionPoint::svCorner);
    }
    n = 0;
    for (size_t i=0; i<endPoint->numberOfEdges(); ++i) {
        SubdivisionEdge* edge = endPoint->getEdge(i);
        if (edge->isCrease())
            n++;
    }
    if (endPoint->getVertexType() == SubdivisionPoint::svCorner) {
        if (endPoint->numberOfFaces() > 1 && n == 2)
            endPoint->setVertexType(SubdivisionPoint::svCrease);
    }
    else {
        if (n == 0)
            endPoint->setVertexType(SubdivisionPoint::svRegular);
        else if (n == 1)
            endPoint->setVertexType(SubdivisionPoint::svDart);
        else if (n == 2)
            endPoint->setVertexType(SubdivisionPoint::svCrease);
        else if (n > 2)
            endPoint->setVertexType(SubdivisionPoint::svCorner);
    }
    _owner->setBuild(false);
}

SubdivisionEdge* SubdivisionEdge::getPreviousEdge()
{
    SubdivisionPoint* p = _points[0];
    SubdivisionEdge* result = 0;
    if (p->getRegularPoint() && p->getVertexType() != SubdivisionPoint::svCorner) {
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
    if (p->getRegularPoint() && p->getVertexType() != SubdivisionPoint::svCorner) {
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
    QVector3D point = 0.5 * startPoint()->getCoordinate() * endPoint()->getCoordinate();
    SubdivisionPoint* result = new SubdivisionPoint(_owner);
    if (_crease)
        result->setVertexType(SubdivisionPoint::svCrease);
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

void SubdivisionEdge::draw(Viewport& vp)
{
}

void SubdivisionEdge::dump(ostream& os) const
{
    os << "SubdivisionEdge ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionEdge& edge)
{
    edge.dump(os);
    return os;
}

SubdivisionControlEdge::SubdivisionControlEdge(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionControlEdge::~SubdivisionControlEdge()
{
    // does nothing
}

void SubdivisionControlEdge::clear()
{

}

void SubdivisionControlEdge::collapse()
{

}

QColor SubdivisionControlEdge::getColor()
{
    QColor result;
    if (_selected)
        result = _owner->getSelectedColor();
    else if (_faces.size() > 2)
        result = cLime;
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
        if (_faces[i]->useInHydrostatics())
            n++;
    }
    if (n == 1)
        result = (fabs(startPoint()->getCoordinate().y()) > 1E-4f || fabs(endPoint()->getCoordinate().y()) > 1E-4f);
    return result;
}

void SubdivisionControlEdge::setSelected(bool val)
{
    _owner->setSelectedControlEdges(this);
}

bool SubdivisionControlEdge::getSelected()
{
    return _owner->hasSelectedControlEdge(this);
}

bool SubdivisionControlEdge::getVisible()
{
    // meant for control edges onlly
    // a control edge is visible if at least one of it's
    //  neighbouring control faces belongs to a visible layer
    bool result = false;

    // BUGBUG: bunch of stuff here to see what layer...

    // finally check if the edge is selected
    // selected edges must be visible at all times
    if (!result)
        result = getSelected();
    if (!result && getCurve() != 0)
        result = getCurve()->isSelected();
    return result;
}

SubdivisionControlPoint* SubdivisionControlEdge::insertControlPoint(QVector3D p)
{

}

void SubdivisionControlEdge::load_binary(FileBuffer& source)
{

}

void SubdivisionControlEdge::save_binary(FileBuffer& destination)
{

}

void SubdivisionControlEdge::trace()
{

}

void SubdivisionControlEdge::draw(Viewport &vp)
{

}

void SubdivisionControlEdge::dump(std::ostream& os) const
{

}

void SubdivisionControlEdge::dump(ostream& os) const
{
    os << "SubdivisionControlEdge ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionControlEdge& edge)
{
    edge.dump(os);
    return os;
}
