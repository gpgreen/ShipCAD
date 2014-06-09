#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "subdivface.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivcontrolcurve.h"
#include "subdivlayer.h"
#include "filebuffer.h"
#include "utility.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

const QVector3D ZERO = QVector3D(0,0,0);

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionFace::SubdivisionFace(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionFace::~SubdivisionFace()
{
    // does nothing
}

size_t SubdivisionFace::indexOfPoint(SubdivisionPoint* pt)
{
    return _owner->indexOfPoint(pt);
}

static float triangle_area(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    float ax = 0.5 * ((p1.y() - p2.y()) * (p1.z() + p2.z())
                      + (p2.y() - p3.y()) * (p2.z() + p3.z())
                      + (p3.y() - p1.y()) * (p3.z() + p1.z()));
    float ay = 0.5 * ((p1.z() - p2.z()) * (p1.x() + p2.x())
                      + (p2.z() - p3.z()) * (p2.x() + p3.x())
                      + (p3.z() - p1.z()) * (p3.x() + p1.x()));
    float az = 0.5 * ((p1.x() - p2.x()) * (p1.y() + p2.y())
                      + (p2.x() - p3.x()) * (p2.y() + p3.y())
                      + (p3.x() - p1.x()) * (p3.y() + p1.y()));
    return sqrt(ax*ax + ay*ay + az*az);
}

float SubdivisionFace::getArea()
{
    float result = 0;
    for (size_t i=3; i<=_points.size(); ++i)
        result += triangle_area(_points[0]->getCoordinate(),
                _points[i-2]->getCoordinate(),
                _points[i-1]->getCoordinate());
    return result;
}

QVector3D SubdivisionFace::getFaceNormal()
{
    QVector3D result = ZERO;
    QVector3D c = ZERO;
    // calculate center of the face
    for (size_t i=0; i<_points.size(); ++i) {
        QVector3D p = _points[i]->getCoordinate();
        c += p;
    }
    c /= _points.size();
    // calculate normal
    SubdivisionPoint* p1 = _points[_points.size() - 1];
    for (size_t i=0; i<_points.size(); ++i) {
        SubdivisionPoint* p2 = _points[i];
        QVector3D n = UnifiedNormal(c, p1->getCoordinate(), p2->getCoordinate());
        result += n;
        p1 = p2;
    }
    result.normalize();
    return result;
}

bool SubdivisionFace::hasPoint(SubdivisionPoint *pt)
{
    return (find(_points.begin(), _points.end(), pt) != _points.end());
}

SubdivisionPoint* SubdivisionFace::getPoint(size_t index)
{
    if (index < _points.size())
        return _points[index];
    throw range_error("index not in range SubdivisionFace::getPoint");
}

SubdivisionPoint* SubdivisionFace::calculateFacePoint()
{
    SubdivisionPoint* result = 0;
    QVector3D centre = ZERO;
    if (_points.size() > 3 || _owner->getSubdivisionMode() == SubdivisionSurface::fmCatmullClark) {
        for (size_t i=0; i<_points.size(); ++i) {
            QVector3D p = _points[i]->getCoordinate();
            centre += p;
        }
        centre /= _points.size();
        result = new SubdivisionPoint(_owner);
        result->setCoordinate(centre);
    }
    return result;
}

void SubdivisionFace::clear()
{
    _points.clear();
}

void SubdivisionFace::flipNormal()
{
    size_t mid = _points.size() / 2 - 1;
    for (size_t i=0; i<=mid; ++i)
        swap(_points[i], _points[_points.size() - i - 1]);
}

void SubdivisionFace::edgeCheck(SubdivisionSurface *owner,
                                SubdivisionPoint* p1,
                                SubdivisionPoint* p2,
                                bool crease,
                                bool controledge,
                                SubdivisionControlCurve* curve,
                                SubdivisionFace* newface,
                                vector<SubdivisionEdge*> &interioredges,
                                vector<SubdivisionEdge*> &controledges
                                )
{
    SubdivisionEdge* newedge = 0;
    if (p1 == 0 || p2 == 0)
        throw runtime_error("bad points in SubdivisionFace::edgeCheck");
    newedge = owner->edgeExists(p1, p2);
    if (newedge == 0) {
        newedge = new SubdivisionEdge(owner);
        newedge->setPoints(p1, p2);
        newedge->startPoint()->addEdge(newedge);
        newedge->endPoint()->addEdge(newedge);
        newedge->setControlEdge(controledge);
        newedge->setCrease(crease);
        if (newedge->isControlEdge())
            controledges.push_back(newedge);
        else
            interioredges.push_back(newedge);
    }
    else if (newedge->isControlEdge())
        controledges.push_back(newedge);
    if (newedge->isControlEdge())
        newedge->setCurve(curve);
    newedge->addFace(newface);
}

// predicate class to find an element with given point
struct PointPred {
    ShipCADGeometry::SubdivisionPoint* _querypt;
    bool operator()(const pair<ShipCADGeometry::SubdivisionPoint*, ShipCADGeometry::SubdivisionPoint*>& val)
    {
        return val.first == _querypt;
    }
    PointPred (ShipCADGeometry::SubdivisionPoint* querypt) : _querypt(querypt) {}
};

// predicate class to find an element with given face
struct FacePred {
    ShipCADGeometry::SubdivisionFace* _queryface;
    bool operator()(const pair<ShipCADGeometry::SubdivisionFace*, ShipCADGeometry::SubdivisionPoint*>& val)
    {
        return val.first == _queryface;
    }
    FacePred (ShipCADGeometry::SubdivisionFace* queryface) : _queryface(queryface) {}
};

// predicate class to find an element with given edge
struct EdgePred {
    ShipCADGeometry::SubdivisionEdge* _queryedge;
    bool operator()(const pair<ShipCADGeometry::SubdivisionEdge*, ShipCADGeometry::SubdivisionPoint*>& val)
    {
        return val.first == _queryedge;
    }
    EdgePred (ShipCADGeometry::SubdivisionEdge* queryedge) : _queryedge(queryedge) {}
};

void SubdivisionFace::subdivide(SubdivisionSurface *owner,
                                bool controlface,
                                vector<pair<SubdivisionPoint*,SubdivisionPoint*> > &vertexpoints,
                                vector<pair<SubdivisionEdge*,SubdivisionPoint*> > &edgepoints,
                                vector<pair<SubdivisionFace*,SubdivisionPoint*> > &facepoints,
                                vector<SubdivisionEdge*> &interioredges,
                                vector<SubdivisionEdge*> &controledges,
                                vector<SubdivisionFace*> &dest)
{
    SubdivisionPoint* pts[4];
    SubdivisionFace* newface;
    SubdivisionPoint* p2;
    SubdivisionEdge* prevedge;
    SubdivisionEdge* curedge;
    vector<pair<SubdivisionPoint*,SubdivisionPoint*> >::iterator ptmpindex;
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> >::iterator etmpindex;
    vector<pair<SubdivisionFace*,SubdivisionPoint*> >::iterator ftmpindex;

    if (_points.size() != 3 || owner->getSubdivisionMode() == SubdivisionSurface::fmCatmullClark) {
        for (size_t i=1; i<=_points.size(); ++i) {
            p2 = _points[i-1];
            size_t index = (i + _points.size() - 2) % _points.size();
            prevedge = owner->edgeExists(p2, _points[index]);
            index = (i + _points.size()) % _points.size();
            curedge = owner->edgeExists(p2, _points[index]);
            index = (i - 1) % 4;
            ptmpindex = find_if(vertexpoints.begin(), vertexpoints.end(), PointPred(p2));
            pts[index] = (*ptmpindex).second; // p2.newlocation
            SubdivisionPoint* p2pt = pts[index];
            index = (index + 1) % 4;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(curedge));
            pts[index] = (*etmpindex).second; // curedge.newlocation
            SubdivisionPoint* curredgept = pts[index];
            index = (index + 1) % 4;
            ftmpindex = find_if(facepoints.begin(), facepoints.end(), FacePred(this));
            pts[index] = (*ftmpindex).second; // this.newlocation
            SubdivisionPoint* newlocation = pts[index];
            index = (index + 1) % 4;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(prevedge));
            pts[index] = (*etmpindex).second; // prevedge.newlocation
            SubdivisionPoint* prevedgept = pts[index];
            // add the new face
            newface = new SubdivisionFace(owner);
            dest.push_back(newface);
            // check the edges of the new face
            edgeCheck(owner, prevedgept, p2pt, prevedge->isCrease(), prevedge->isControlEdge() || controlface, prevedge->getCurve(), newface, interioredges, controledges);
            edgeCheck(owner, p2pt, curredgept, curedge->isCrease(), curedge->isControlEdge() || controlface, curedge->getCurve(), newface, interioredges, controledges);
            edgeCheck(owner, curredgept, newlocation, false, false, 0, newface, interioredges, controledges);
            edgeCheck(owner, prevedgept, newlocation, false, false, 0, newface, interioredges, controledges);
            for (size_t j=0; j<4; ++j) {
                // add new face to points
                pts[j]->addFace(newface);
                newface->addPoint(pts[j]);
            }
        }
    }
    else if (numberOfPoints() == 3) {
        // special case, quadrisect triangle by connecting all three
        // edge points
        // first the three surrounding triangles
        for (size_t i=1; i<=_points.size(); ++i) {
            p2 = _points[i-1];
            size_t index = (i-2+numberOfPoints()) % numberOfPoints();
            prevedge = owner->edgeExists(p2, _points[index]);
            index = (i+numberOfPoints()) % numberOfPoints();
            curedge = owner->edgeExists(p2, _points[index]);

            index = 0;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(prevedge));
            pts[index] = (*etmpindex).second;
            index = 1;
            ptmpindex = find_if(vertexpoints.begin(), vertexpoints.end(), PointPred(p2));
            pts[index] = (*ptmpindex).second;
            index = 2;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(curedge));
            pts[index] = (*etmpindex).second;
            // add the new face
            newface = new SubdivisionFace(owner);
            dest.push_back(newface);
            // check the edges of the new face
            edgeCheck(owner, pts[0], pts[1], prevedge->isCrease(), prevedge->isControlEdge() || controlface, prevedge->getCurve(), newface, interioredges, controledges);
            edgeCheck(owner, pts[1], pts[2], curedge->isCrease(), curedge->isControlEdge() || controlface, curedge->getCurve(), newface, interioredges, controledges);
            edgeCheck(owner, pts[2], pts[0], false, false, 0, newface, interioredges, controledges);
            for (size_t j=0; j<3; ++j) {
                // add new face to points
                pts[j]->addFace(newface);
                newface->addPoint(pts[j]);
            }
        }

        // then the center triangle
        for (size_t i=1; i<=numberOfPoints(); ++i) {
            p2 = _points[i-1];
            size_t index = (i - 2 + numberOfPoints()) % numberOfPoints();
            prevedge = owner->edgeExists(p2, _points[index]);
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(prevedge));
            pts[i-1] = (*etmpindex).second;
        }
        // add the new face
        newface = new SubdivisionFace(owner);
        dest.push_back(newface);
        edgeCheck(owner, pts[0], pts[1], false, false, 0, newface, interioredges, controledges);
        edgeCheck(owner, pts[1], pts[2], false, false, 0, newface, interioredges, controledges);
        edgeCheck(owner, pts[2], pts[0], false, false, 0, newface, interioredges, controledges);
        for (size_t j=0; j<3; ++j) {
            // add new face to points
            pts[j]->addFace(newface);
            newface->addPoint(pts[j]);
        }
    }
}

void SubdivisionFace::dump(ostream& os) const
{
    os << "SubdivisionFace ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionFace& face)
{
    face.dump(os);
    return os;
}

SubdivisionControlFace::SubdivisionControlFace(SubdivisionSurface *owner)
    : SubdivisionFace(owner)
{
    clear();
}

SubdivisionControlFace::~SubdivisionControlFace()
{
    // delete from selection
    setSelected(false);
    // remove from layer
    setLayer(0);
    if (_owner->hasControlFace(this)) {
        _owner->deleteControlFace(this);
        SubdivisionPoint* p1 = getPoint(numberOfPoints() - 1);
        p1->deleteFace(this);
        for (size_t i=1; i<=numberOfPoints(); ++i) {
            SubdivisionPoint* p2 = getPoint(i - 1);
            SubdivisionControlEdge* edge = _owner->controlEdgeExists(p1, p2);
            if (edge != 0) {
                edge->deleteFace(this);
                if (edge->numberOfFaces() == 0)
                    delete edge;
            }
            p1 = p2;
        }
    }
    _owner->setBuild(false);
    clear();
}

void SubdivisionControlFace::draw(Viewport& /*vp*/)
{
    // does nothing
}

SubdivisionControlEdge* SubdivisionControlFace::insertEdge(
        SubdivisionControlPoint *p1, SubdivisionControlPoint *p2)
{
    SubdivisionControlEdge* result = 0;
    if (p1->hasFace(this) && p2->hasFace(this)) {
        if (_owner->edgeExists(p1, p2) != 0)
            return result;
        size_t tmp = indexOfPoint(p1);
        vector<SubdivisionControlPoint*> pts;
        pts.push_back(p1);
        for (size_t i=0; i<numberOfPoints(); ++i) {
            tmp = (tmp + 1) % numberOfPoints();
            pts.push_back(dynamic_cast<SubdivisionControlPoint*>(getPoint(tmp)));
            if (pts[pts.size()-1] == p2)
                break;
        }
        if (pts.size() > 2)
            _owner->addControlFace(pts, false, getLayer());
        tmp = indexOfPoint(p2);
        pts.clear();
        pts.push_back(p2);
        for (size_t i=0; i<numberOfPoints(); ++i) {
            tmp = (tmp + 1) % numberOfPoints();
            pts.push_back(dynamic_cast<SubdivisionControlPoint*>(getPoint(tmp)));
            if (pts[pts.size()-1] == p1)
                break;
        }
        if (pts.size() > 2)
            _owner->addControlFace(pts, false, getLayer());
        // BUGBUG: we need to delete 'this' face, can't do it here
    }
    result = _owner->controlEdgeExists(p1, p2);
    if (result != 0)
        result->setCrease(false);
    return result;
}

SubdivisionFace* SubdivisionControlFace::getChild(size_t index)
{
    if (index < _children.size())
        return _children[index];
    throw runtime_error("Bad index in SubdivisionControlFace::getChild");
}

QColor SubdivisionControlFace::getColor()
{
    if (isSelected())
        return _owner->getSelectedColor();
    else
        return _layer->getColor();
}

SubdivisionEdge* SubdivisionControlFace::getControlEdge(size_t index)
{
    if (index < _control_edges.size())
        return _control_edges[index];
    throw runtime_error("bad index in SubdivisionControlFace::getControlEdge");
}

void SubdivisionControlFace::setSelected(bool val)
{
    bool have = _owner->hasSelectedControlFace(this);
    if (val && !have)
        _owner->setSelectedControlFace(this);
    else if (!val && have)
        _owner->removeSelectedControlFace(this);
}

bool SubdivisionControlFace::isSelected()
{
    return _owner->hasSelectedControlFace(this);
}

SubdivisionEdge* SubdivisionControlFace::getEdge(size_t index)
{
    if (index < _edges.size())
        return _edges[index];
    throw runtime_error("bad index in SubdivisionControlFace::getEdge");
}

size_t SubdivisionControlFace::getIndex()
{
    return _owner->indexOfControlFace(this);
}

bool SubdivisionControlFace::isVisible()
{
    if (_layer != 0 && _layer->isVisible())
        return true;
    return isSelected();
}

void SubdivisionControlFace::setLayer(SubdivisionLayer *layer)
{
    if (layer == _layer)
        return;
    if (layer != 0) {
        // disconnect from current layer
        _layer->deleteControlFace(this);
        _layer = 0;
    }
    _layer = layer;
    if (layer != 0)
        // connect to the new layer
        _layer->addControlFace(this);
}

void SubdivisionControlFace::calcExtents()
{
    // calculate min/max coordinate of all children
    if (numberOfPoints() > 0)
        _min = getPoint(0)->getCoordinate();
    else
        _min = ZERO;
    _max = _min;
    if (_children.size() > 0) {
        for (size_t i=1; i<=_children.size(); ++i) {
            SubdivisionFace* face = _children[i-1];
            for (size_t j=1; j<=face->numberOfPoints(); ++j) {
                if (i == 1 && j == 1) {
                    _min = face->getPoint(j - 1)->getCoordinate();
                    _max = _min;
                }
                MinMax(face->getPoint(j - 1)->getCoordinate(), _min, _max);
            }
        }
    }
    else {
        for (size_t i=2; i<=numberOfPoints(); ++i) {
            MinMax(getPoint(i - 1)->getCoordinate(), _min, _max);
        }
    }
}

void SubdivisionControlFace::clear()
{
    clearChildren();
    SubdivisionFace::clear();
    _layer = 0;
}

void SubdivisionControlFace::clearChildren()
{
    for (size_t i=0; i<_children.size(); ++i)
        delete _children[i];
    _children.clear();
    for (size_t i=0; i<_edges.size(); ++i)
        delete _edges[i];
    _edges.clear();
}

void SubdivisionControlFace::loadBinary(FileBuffer &source)
{
    // read controlpoint data
    size_t n, index;
    SubdivisionControlPoint* p1;
    source.load(n);
    _points.clear();
    for (size_t i=0; i<n; ++i) {
        source.load(index);
        p1 = _owner->getControlPoints(index);
        _points.push_back(p1);
        p1->addFace(this);
    }
    // read layer index
    int ind;
    source.load(ind);
    if (ind >= 0 && ind < _owner->numberOfLayers())
        _layer = _owner->getLayer(ind);
    else
        _layer = _owner->getLayer(0); // reference to an invalid layer, assign to owners default layer
    if (_layer != 0)
        _layer->addControlFace(this);
    else
        throw runtime_error("Invalid layer reference in SubdivisionControlFace::loadBinary");
    bool sel;
    source.load(sel);
    if (sel)
        setSelected(true);
    p1 = dynamic_cast<SubdivisionControlPoint*>(_points[numberOfPoints()-1]);
    for (size_t i=0; i<numberOfPoints(); ++i) {
        SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(_points[i]);
        SubdivisionControlEdge* edge = _owner->controlEdgeExists(p1, p2);
        if (edge != 0) {
            edge->addFace(this);
        }
        else {
            edge = _owner->addControlEdge(p1, p2);
            edge->setCrease(true);
            cerr << "Could not find edge!" << endl;
        }
        p1 = p2;
    }
}

void SubdivisionControlFace::removeReferences()
{
    SubdivisionPoint* p1 = getPoint(_points.size()-1);
    for (size_t i=0; i<_points.size(); ++i) {
        SubdivisionPoint* p2 = getPoint(i);
        p2->deleteFace(this);
        SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
        if (edge != 0)
            edge->deleteFace(this);
        p1 = p2;
    }
}

void SubdivisionControlFace::saveBinary(FileBuffer &destination)
{
    destination.add(numberOfPoints());
    for (size_t i=0; i<numberOfPoints(); ++i) {
        destination.add(_owner->indexOfControlPoint(dynamic_cast<SubdivisionControlPoint*>(getPoint(i))));
    }
    // add layer index
    int index;
    if (getLayer() != 0)
        index = _owner->indexOfLayer(getLayer());
    else
        index = -1;
    destination.add(index);
    destination.add(isSelected());
}

void SubdivisionControlFace::saveToDXF(vector<QString>& /*strings*/)
{
    QString layername = getLayer()->getName();
    int colorindex = FindDXFColorIndex(getLayer()->getColor());
    if (numberOfPoints() == 4) {
        // create one polymesh for all childfaces
        //BUGBUG: we need a SubdivisionGrid and FaceGrid object
    }
}

void SubdivisionControlFace::subdivide(SubdivisionSurface *owner,
                                       vector<pair<SubdivisionPoint*,SubdivisionPoint*> >& vertexpoints,
                                       vector<pair<SubdivisionEdge*,SubdivisionPoint*> >& edgepoints,
                                       vector<pair<SubdivisionFace*,SubdivisionPoint*> >& facepoints,
                                       vector<SubdivisionEdge*>& /*interioredges*/,
                                       vector<SubdivisionEdge*>& controledges,
                                       vector<SubdivisionFace*>& /*dest*/)
{
    _control_edges.clear();
    if (_children.size() == 0) {
        SubdivisionFace::subdivide(owner, true, vertexpoints, edgepoints, facepoints, _edges, _control_edges,
                                   _children);
    }
    else {
        vector<SubdivisionFace*> tmplist;
        vector<SubdivisionEdge*> tmp;
        for (size_t i=0; i<_children.size(); ++i) {
            SubdivisionFace* face = _children[i];
            face->subdivide(owner, false, vertexpoints, edgepoints, facepoints, tmp, _control_edges, tmplist);
        }
        clearChildren();
        _edges = tmp;
        _children = tmplist;
    }
    for (size_t i=0; i<_control_edges.size(); ++i) {
        if (find(controledges.begin(), controledges.end(), _control_edges[i]) == controledges.end())
            controledges.push_back(_control_edges[i]);
    }
    calcExtents();
}

void SubdivisionControlFace::findAttachedFaces(vector<SubdivisionControlFace*>& todo_list,
                                               SubdivisionControlFace* face)
{
    SubdivisionPoint* p1 = face->getPoint(face->numberOfPoints() - 1);
    for (size_t i=0; i<face->numberOfPoints(); ++i) {
        SubdivisionPoint* p2 = face->getPoint(i);
        SubdivisionEdge* edge = face->getOwner()->edgeExists(p1, p2);
        if (edge != 0 && !edge->isCrease()) {
            for (size_t j=0; j<edge->numberOfFaces(); ++j) {
                SubdivisionControlFace* f = dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                if (f != 0 && f != face) {
                    vector<SubdivisionControlFace*>::iterator idx = find(todo_list.begin(),
                                                                  todo_list.end(),
                                                                  f);
                    if (idx != todo_list.end()) {
                        todo_list.erase(idx);
                        findAttachedFaces(todo_list, f);
                    }
                }
            }
        }
        p1 = p2;
    }
}

// select all controlfaces connected to the current one that belong to the same layer
// and are not separated by a crease edge
void SubdivisionControlFace::trace()
{
    vector<SubdivisionControlFace*> todo_list;
    for (size_t i=0; i<getLayer()->numberOfFaces(); ++i) {
        SubdivisionControlFace* face = getLayer()->getFace(i);
        if (face != this && face->isSelected() != isSelected())
            todo_list.push_back(face);
    }
    findAttachedFaces(todo_list, this);
}

void SubdivisionControlFace::dump(ostream& os) const
{
    os << "SubdivisionControlFace ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionControlFace& face)
{
    face.dump(os);
    return os;
}
