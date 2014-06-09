#include <iostream>
#include <cmath>
#include <boost/math/constants/constants.hpp>
#include <stdexcept>

#include "subdivpoint.h"
#include "subdivsurface.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"

using namespace ShipCADGeometry;
using namespace ShipCADUtility;
using namespace std;
using namespace boost::math::float_constants;

static QVector3D ZERO = QVector3D(0,0,0);

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionPoint::SubdivisionPoint(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionPoint::~SubdivisionPoint()
{
    // does nothing
}

void SubdivisionPoint::clear()
{
    _coordinate = ZERO;
    _faces.clear();
    _edges.clear();
    _vtype = svRegular;
}

SubdivisionEdge* SubdivisionPoint::getEdge(size_t index)
{
    if (index < _edges.size())
        return _edges[index];
    throw range_error("SubdivisionPoint::getEdge");
}

SubdivisionFace* SubdivisionPoint::getFace(size_t index)
{
    if (index < _faces.size())
        return _faces[index];
    throw range_error("SubdivisionPoint::getFace");
}

size_t SubdivisionPoint::getIndex()
{
    return _owner->indexOfPoint(this);
}

QVector3D SubdivisionPoint::getCoordinate()
{
    return _coordinate;
}

float SubdivisionPoint::Angle_VV_3D(const QVector3D& p1, 
                                    const QVector3D& p2,
                                    const QVector3D& p3)
{
    QVector3D v1 = p1 - p2;
    QVector3D v2 = p3 - p2;
    v1.normalize();
    v2.normalize();
    float l = v1.x()*v2.x() + v1.y()*v2.y() + v1.z()*v2.z();
    if (l < -1)
        l = -1;
    else if (l > 1)
        l = 1;
    return acos(l);
}

float SubdivisionPoint::getCurvature()
{
    float result = 0.0;
    for (size_t i=0; i<_edges.size(); ++i)
        if (_edges[i]->numberOfFaces() < 2)
            return result;
    float sigma = 0;
    for (size_t j=0; j<_faces.size(); ++j) {
        SubdivisionFace* face = _faces[j];
        size_t index = face->indexOfPoint(this);
        size_t previndex = (index + face->numberOfPoints() - 1) % face->numberOfPoints();
        size_t nextindex = (previndex + 2) % face->numberOfPoints();
        SubdivisionPoint* prev = face->getPoint(previndex);
        SubdivisionPoint* next = face->getPoint(nextindex);
        float tmp = Angle_VV_3D(prev->getCoordinate(),
                                _coordinate,
                                next->getCoordinate());
        sigma += tmp;
    }
    result = (360 - RadToDeg(sigma));
    return result;
}

bool SubdivisionPoint::isBoundaryVertex()
{
    bool result = false;
    if (fabs(_coordinate.y()) > 1E-4) {
        for (size_t i=0; i<_edges.size(); ++i)
            result = result || _edges[i]->isBoundaryEdge();
    }
    return result;
}

size_t SubdivisionPoint::numberOfCurves()
{
    size_t result = 0;
    for (size_t i=0; i<_edges.size(); ++i)
        if (_edges[i]->getCurve())
            result++;
    return result;
}

bool SubdivisionPoint::isRegularPoint()
{
    bool result = false;
    // this procedure was only tested to TRACE regular quad edges
    // and regular/irregular CREASE edges for dxf export
    if (_faces.size() == 5 && _edges.size() == 5) {
        // boundary of quad and triangle
        int n = 0;
        for (size_t i=0; i<_faces.size(); ++i) {
            if (_faces[i]->numberOfPoints() == 3)
                n++;
        }
        if (n == 3)
            result = true;
    }
    else if (_faces.size() == 6 && _edges.size() == 6) {
        // regular point with all triangles
        int n = 0;
        for (size_t i=0; i<_faces.size(); ++i) {
            if (_faces[i]->numberOfPoints() == 3)
                n++;
        }
        if (n == 6)
            result = true;
    }
    else if (_faces.size() == 4 && _edges.size() == 4) {
        // regular point with all quads
        result = true;
    }
    else {
        // regular quad boundary edge
        int n = 0;
        for (size_t i=0; i<_edges.size(); ++i) {
            if (_edges[i]->numberOfFaces() == 1)
                n++;
        }
        if (n == 2 && _edges.size() == 3)
            result = true;
    }
    return result;
}

QVector3D SubdivisionPoint::getLimitPoint()
{
    QVector3D result = ZERO;
    SubdivisionPoint* p30 = 0;
    SubdivisionPoint* p33 = 0;
    SubdivisionPoint* p = 0;
    if (_vtype == svDart || _vtype == svRegular) {
        result = ZERO;
        int n = _faces.size();
        for (int i=0; i<n; ++i) {
            SubdivisionFace* face = _faces[i];
            size_t ind = face->indexOfPoint(this);
            p30 = face->getPoint((ind + 1) % face->numberOfPoints());
            p33 = face->getPoint((ind + 2) % face->numberOfPoints());
            result += (n * _coordinate + 4 * p30->getCoordinate() + p33->getCoordinate());
        }
        result /= (n * (n + 5));
    }
    else if (_vtype == svCrease) {
        for (size_t i=0; i<_edges.size(); ++i) {
            SubdivisionEdge* edge = _edges[i];
            if (edge->isCrease()) {
                if (edge->startPoint() == this)
                    p = edge->endPoint();
                else
                    p = edge->startPoint();
                if (p30 == 0)
                    p30 = p;
                else
                    p33 = p;
            }
        }
        if (p30 != 0 && p33 != 0) {
            result = (1/6.0f)*p30->getCoordinate()
                    + (2/3.0f)*result
                    + (1/6.0f)*p33->getCoordinate();
        }
        else {
            // this is an error
            result = getCoordinate();
        }
    }
    else
        result = getCoordinate();
    return result;
}

bool SubdivisionPoint::isRegularNURBSPoint(vector<SubdivisionFace*>& faces)
{
    bool result = false;
    if (_vtype == svRegular || _vtype == svDart)
        result = (_faces.size() == 4);
    else if (_vtype == svCrease) {
        if (_faces.size() != 0) {
            int n = 0;
            for (size_t i=0; i<_faces.size(); ++i) {
                if (find(faces.begin(), faces.end(), _faces[i]) != faces.end())
                    n++;
            }
            result = (n == 2);
            if (n == 2 && _faces.size() != 4)
                result = true;
        }
        else {
            // boundary edge ?
            bool boundary = false;
            for (size_t i=0; i<_edges.size(); ++i) {
                boundary = boundary || _edges[i]->numberOfFaces() == 1;
            }
            if (boundary)
                result = _faces.size() == 2;
            else
                result = _faces.size() == 4;
        }
    }
    return result;
}

void SubdivisionPoint::setCoordinate(const QVector3D& val)
{
    _coordinate = val;
}

void SubdivisionPoint::addEdge(SubdivisionEdge* edge)
{
    if (find(_edges.begin(), _edges.end(), edge) == _edges.end())
        _edges.push_back(edge);
}

void SubdivisionPoint::addFace(SubdivisionFace* face)
{
    if (find(_faces.begin(), _faces.end(), face) == _faces.end())
        _faces.push_back(face);
}

QVector3D SubdivisionPoint::averaging()
{
    QVector3D result;
    SubdivisionPoint* p;
    float totalweight = 0.0;
    size_t nt = 0;
    float weight;

    if (_edges.size() == 0 || _vtype == svCorner)
        result = getCoordinate();
    else {
        if (_vtype == svCrease) {
            result = getCoordinate() * 0.5;
            for (size_t i=0; i<_edges.size(); ++i) {
                SubdivisionEdge* edge = _edges[i];
                if (edge->numberOfFaces() == 1 || edge->isCrease()) {
                    if (edge->startPoint() == this)
                        p = edge->endPoint();
                    else
                        p = edge->startPoint();
                    result += (0.25 * p->getCoordinate());
                }
            }
        }
        else {
            result = ZERO;
            for (size_t i=0; i<_faces.size(); ++i) {
                SubdivisionFace* face = _faces[i];
                QVector3D center = ZERO;
                if (face->numberOfPoints() == 3) {
                    nt++;
                    // calculate centerpoint
                    for (size_t j=0; j<face->numberOfPoints(); ++j) {
                        p = face->getPoint(j);
                        if (p == this)
                            weight = .25;
                        else
                            weight = .375;
                        center += (weight * p->getCoordinate());
                    }
                    weight = third_pi;
                }
                else if (face->numberOfPoints() == 4) {
                    for (size_t j=0; j<face->numberOfPoints(); ++j) {
                        p = face->getPoint(j);
                        weight = .25;
                        center += (weight * p->getCoordinate());
                    }
                    weight = half_pi;
                }
                else
                    throw runtime_error("invalid number of points in SubdivisionPoint::averaging");
                result += (weight * center);
            }
            if (totalweight != 0) {
                result /= totalweight;
            }
            size_t nq = _faces.size() - nt;
            float a;
            if (nt == _faces.size()) {
                // apply averaging in case of vertex surrounded by triangles
                a = 5/3.0 - 8/3.0*(.375+.25*cos(two_pi/_faces.size()));
            }
            else if (nq == _faces.size()) {
                // apply averaging in case of vertex surrounded by quads
                a = 4 / static_cast<float>(_faces.size());
            }
            else {
                // apply averagin in case of vertex on boundary of quads and triangles
                if (nq == 0 && nt == 3)
                    a = 1.5;
                else
                    a = 12 / static_cast<float>(3 * nq + 2 * nt);
            }
            if (a != 1.0) {
                result = getCoordinate() + a * (result - getCoordinate());
            }
        }
    }
    return result;
}

SubdivisionPoint* SubdivisionPoint::calculateVertexPoint()
{
    SubdivisionPoint* result = new SubdivisionPoint(_owner);
    result->setProperty("VertexType", _vtype);
    result->setCoordinate(getCoordinate());
    for (size_t i=0; i<_edges.size(); ++i) {
        SubdivisionEdge* edge = _edges[i];
        if (edge->getCurve() != 0)
            edge->getCurve()->replaceVertexPoint(this, result);
    }
    return result;
}

void SubdivisionPoint::deleteEdge(SubdivisionEdge* edge)
{
    vector<SubdivisionEdge*>::iterator i = find(_edges.begin(),
                                                _edges.end(), edge);
    if (i != _edges.end())
        _edges.erase(i);
}

void SubdivisionPoint::deleteFace(SubdivisionFace* face)
{
    vector<SubdivisionFace*>::iterator i = find(_faces.begin(),
                                                _faces.end(), face);
    if (i != _faces.end())
        _faces.erase(i);
}

size_t SubdivisionPoint::indexOfFace(SubdivisionFace* face)
{
    for (size_t i=0; i<_faces.size(); ++i)
        if (_faces[i] == face)
            return i;
    throw range_error("SubdivisionPoint::indexOfFace");
}

bool SubdivisionPoint::hasFace(SubdivisionFace* face)
{
    return (find(_faces.begin(), _faces.end(), face) != _faces.end());
}

QVector3D SubdivisionPoint::getNormal()
{
    QVector3D result;
    for (size_t i=0; i<_faces.size(); ++i) {
        SubdivisionFace* face = _faces[i];
        if (face->numberOfPoints() > 4) {
            // face possibly concave at this point
            // use the normal of all points from this face
            QVector3D c = face->faceCenter();
            QVector3D n = ZERO;
            for (size_t j=1; j<face->numberOfPoints(); ++j) {
                n = n + UnifiedNormal(c, face->getPoint(j-1)->getCoordinate(),
                                      face->getPoint(j)->getCoordinate());
            }
            n.normalize();
            result = result + n;
        }
        else {
            size_t index = face->indexOfPoint(this);
            size_t j = (index + face->numberOfPoints() - 1) % face->numberOfPoints();
            QVector3D p1 = face->getPoint(j)->getCoordinate();
            j = (index + face->numberOfPoints() + 1) % face->numberOfPoints();
            QVector3D p3 = face->getPoint(j)->getCoordinate();
            result = result + UnifiedNormal(p1, getCoordinate(), p3);
        }
    }
    result.normalize();
    return result;
}

void SubdivisionPoint::draw(Viewport& /*vp*/)
{
  // does nothing
}

void SubdivisionPoint::dump(ostream& os) const
{
    os << "SubdivisionPoint ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
    os << "\n Coordinate ["
       << _coordinate.x()
       << "," << _coordinate.y()
       << "," << _coordinate.z()
       << "]\n VertexType:" << _vtype;
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionPoint& point)
{
    point.dump(os);
    return os;
}

SubdivisionControlPoint::SubdivisionControlPoint(SubdivisionSurface *owner)
    : SubdivisionPoint(owner)
{
    clear();
}

SubdivisionControlPoint::~SubdivisionControlPoint()
{
    // does nothing
}

QColor SubdivisionControlPoint::getColor()
{
    if (isSelected())
        return _owner->getSelectedColor();
    if (_locked)
        return Qt::darkGray;
    if (isLeak())
        return _owner->getLeakColor();
    switch (getVertexType()) {
    case svRegular:
        return _owner->getRegularPointColor();
    case svCorner:
        return _owner->getCornerPointColor();
    case svDart:
        return _owner->getDartPointColor();
    case svCrease:
        return _owner->getCreasePointColor();
    default:
        return Qt::red;
    }
}

size_t SubdivisionControlPoint::getIndex()
{
    return _owner->indexOfControlPoint(this);
}

bool SubdivisionControlPoint::isSelected()
{
    return _owner->hasSelectedControlPoint(this);
}

bool SubdivisionControlPoint::isVisible()
{
    // meant for controlpoints only
    // a controlpoint is visible if at least one of it's
    // neighbouring controlfaces belongs to a visible layer
    bool result = false;
    if (_owner->showControlNet()) {
        for (size_t i=0; i<_faces.size(); ++i) {
            SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(_faces[i]);
            if (face == 0)
                continue;
            if (face->getLayer() != 0) {
                if (face->getLayer()->isVisible())
                    return true;
            }
        }
    }
    // finally check if the point is selected
    // selected points must be visible at all times
    // points with no faces connected also!
    result = isSelected() || _faces.size() == 0;
    if (!result && _owner->numberControlCurves() > 0) {
        for (size_t i=0; i<numberOfEdges(); ++i) {
            if (_edges[i]->getCurve() != 0) {
                if (_edges[i]->getCurve()->isSelected()) {
                    result = true;
                    break;
                }
            }
        }
    }
    return result;
}

bool SubdivisionControlPoint::isLeak()
{
    return (fabs(getCoordinate().y()) > 1E-4f && isBoundaryVertex());
}

void SubdivisionControlPoint::setSelected(bool val)
{
    if (val) {
        if (!_owner->hasSelectedControlPoint(this))
            _owner->setSelectedControlPoint(this);
    }
    else {
        if (_owner->hasSelectedControlPoint(this))
            _owner->removeSelectedControlPoint(this);
    }
}

void SubdivisionControlPoint::setLocked(bool val)
{
    if (val != _locked)
        _locked = val;
}

void SubdivisionControlPoint::setCoordinate(const QVector3D &val)
{
    SubdivisionPoint::setCoordinate(val);
    _owner->setBuild(false);
}

void SubdivisionControlPoint::collapse()
{
    SubdivisionControlPoint* p1, *p2;
    SubdivisionControlEdge* edge1, *edge2;
    SubdivisionControlFace* face;
    if (_faces.size() <= 2) {
        setSelected(false);
        p1 = 0;
        p2 = 0;
        // this is possibly a point on a boundary edge
        // check for this special case
        edge1 = 0;
        edge2 = 0;
        for (size_t i=0; i<numberOfEdges(); ++i) {
            if (_edges[i]->numberOfFaces() == 1) {
                if (edge1 == 0) {
                    edge1 = dynamic_cast<SubdivisionControlEdge*>(_edges[i]);
                }
                else {
                    edge2 = dynamic_cast<SubdivisionControlEdge*>(_edges[i]);
                }
            }
        }
        if (edge1 != 0 && edge2 != 0) {
            for (size_t i=numberOfEdges(); i>0; --i) {
                edge1 = dynamic_cast<SubdivisionControlEdge*>(_edges[i-1]);
                edge1->collapse();
            }
        }
        bool edge_collapse = false;
        bool crease = false;
        if (_edges.size() == 2) {
            edge_collapse = true;
            edge1 = dynamic_cast<SubdivisionControlEdge*>(_edges[0]);
            if (edge1 == 0)
                throw runtime_error("edge is not a control edge SubdivisionControlPoint::collapse");
            if (edge1->startPoint() == this) {
                p1 = dynamic_cast<SubdivisionControlPoint*>(edge1->endPoint());
            }
            else {
                p1 = dynamic_cast<SubdivisionControlPoint*>(edge1->startPoint());
            }
            edge2 = dynamic_cast<SubdivisionControlEdge*>(_edges[1]);
            if (edge2 == 0)
                throw runtime_error("edge is not a control edge SubdivisionControlPoint::collapse");
            if (edge2->startPoint() == this) {
                p2 = dynamic_cast<SubdivisionControlPoint*>(edge2->endPoint());
            }
            else {
                p2 = dynamic_cast<SubdivisionControlPoint*>(edge2->startPoint());
            }
            crease = edge1->isCrease() || edge2->isCrease();
        }
        for (size_t i=numberOfFaces(); i>0; --i) {
            face = dynamic_cast<SubdivisionControlFace*>(_faces[i-1]);
            vector<SubdivisionControlPoint*> points;
            for (size_t j=0; j<face->numberOfPoints(); ++j) {
                SubdivisionControlPoint* pt = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
                if (pt && pt != this)
                    points.push_back(pt);
            }
            _owner->addControlFace(points, false, face->getLayer());
            delete face;
        }

        if (edge_collapse) {
            edge1 = _owner->controlEdgeExists(p1, p2);
            if (edge1 != 0)
                edge1->setCrease(crease);
        }
    }   // faces.size <= 2
    else {
        vector<SubdivisionControlPoint*> checklist;
        vector<SubdivisionControlEdge*> edges;
        for (size_t i=0; i<numberOfEdges(); ++i) {
            SubdivisionControlPoint* s = dynamic_cast<SubdivisionControlPoint*>(edges[i]->startPoint());
            SubdivisionControlPoint* e = dynamic_cast<SubdivisionControlPoint*>(edges[i]->endPoint());
            if (s == 0 || e == 0)
                throw runtime_error("point not control point SubdivisionControlPoint::collapse");
            if (s == this)
                checklist.push_back(e);
            else
                checklist.push_back(s);
        }
        for (size_t i=0; i<numberOfFaces(); ++i) {
            face = dynamic_cast<SubdivisionControlFace*>(_faces[i]);
            if (face == 0)
                throw runtime_error("face is not a control face SubdivisionControlPoint::collapse");
            p1 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(face->numberOfPoints()-1));
            if (p1 == 0)
                throw runtime_error("point not control point SubdivisionControlPoint::collapse");
            for (size_t j=0; j<face->numberOfPoints(); ++j) {
                p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
                if (p2 == 0)
                    throw runtime_error("point not control point SubdivisionControlPoint::collapse");
                if (p1 != this && p2 != this) {
                    edge1 = _owner->controlEdgeExists(p1, p2);
                    if (edge1 != 0
                            && find(edges.begin(), edges.end(), edge1) == edges.end())
                        _edges.push_back(edge1);
                }
                p1 = p2;
            }
        }
        // sort edges in correct order and add new face
        if (edges.size() > 2) {
            vector<vector<SubdivisionControlPoint*> > sorted;
            _owner->isolateEdges(edges, sorted);
            for (size_t i=0; i<sorted.size(); ++i) {
                vector<SubdivisionControlPoint*>& points = sorted[i];
                if (points.size() > 2) {
                    face = _owner->addControlFace(points, false);
                    if (face != 0) {
                        p1 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(face->numberOfPoints()-1));
                        if (p1 == 0)
                            throw runtime_error("point not control point SubdivisionControlPoint::collapse");
                        for (size_t j=0; j<face->numberOfPoints(); j++) {
                            p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
                            if (p2 == 0)
                                throw runtime_error("point not control point SubdivisionControlPoint::collapse");
                            // BUGBUG: doesn't do anything here
                            edge1 = _owner->controlEdgeExists(p1, p2);
                            p1 = p2;
                        }
                    }
                }
            }
        }
        // BUGBUG: supposed to delete point here
        for (size_t i=checklist.size(); i>0; --i) {
            p1 = checklist[i-1];
            if (p1->numberOfFaces() > 1 && p1->numberOfEdges() == 2)
                p1->collapse();
        }
    }
}

void SubdivisionControlPoint::dump(ostream& os) const
{
    os << "SubdivisionControlPoint ["
       << hex << this << "]\n";
    SubdivisionPoint::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionControlPoint& point)
{
    point.dump(os);
    return os;
}
