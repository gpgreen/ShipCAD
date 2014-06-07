#include <iostream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "subdivface.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivcontrolcurve.h"
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

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionFace& base)
{
    base.dump(os);
    return os;
}
