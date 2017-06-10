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

#include "subdivface.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivcontrolcurve.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "shader.h"
#include "controlfacegrid.h"
#include "pointgrid.h"
#include "viewportview.h"
#include "predicate.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionFace* SubdivisionFace::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getFacePool().add();
    return new (mem) SubdivisionFace(owner);
}

SubdivisionFace::SubdivisionFace(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
	// does nothing
}

size_t SubdivisionFace::indexOfPoint(const SubdivisionPoint* pt) const
{
    vector<SubdivisionPoint*>::const_iterator i = find(_points.begin(), _points.end(), pt);
    return i - _points.begin();
}

void SubdivisionFace::insertPoint(size_t index, SubdivisionPoint *point)
{
    if (index >= _points.size())
        throw out_of_range("bad index in SubdivisionFace::insertPoint");
    _points.insert(_points.begin()+index, point);
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

float SubdivisionFace::getArea() const
{
    float result = 0;
    for (size_t i=3; i<=_points.size(); ++i)
        result += triangle_area(_points[0]->getCoordinate(),
                _points[i-2]->getCoordinate(),
                _points[i-1]->getCoordinate());
    return result;
}

QVector3D SubdivisionFace::getFaceCenter() const
{
    QVector3D result = ZERO;
    if (_points.size() > 1) {
        for (size_t i=1; i<=_points.size(); ++i) {
            SubdivisionPoint* p = _points[i-1];
            result = result + p->getCoordinate();
        }
        result /= _points.size();
    }
    return result;
}

QVector3D SubdivisionFace::getFaceNormal() const
{
    QVector3D result = ZERO;
    QVector3D c = ZERO;

	if (_points.size() == 0)
		return result;
	
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

bool SubdivisionFace::hasPoint(const SubdivisionPoint *pt) const
{
    return (find(_points.begin(), _points.end(), pt) != _points.end());
}

SubdivisionPoint* SubdivisionFace::getPoint(size_t index) const
{
    if (index < _points.size())
        return _points[index];
    throw out_of_range("index not in range SubdivisionFace::getPoint");
}

SubdivisionPoint* SubdivisionFace::getLastPoint() const
{
    if (_points.size() == 0)
        throw out_of_range("no last point for SubdivisionFace::getLastPoint");
    return _points.back();
}

void SubdivisionFace::addPoint(SubdivisionPoint *point)
{
    _points.push_back(point);
    point->addFace(this);
}

// algorithm from http://geomalgorithms.com/a06-_intersect.html
static bool privIntersectWithRay(const PickRay& ray, const QVector3D& p1, const QVector3D& p2,
                          const QVector3D& p3)
{
    // get triangle edge vectors and plane normal
    QVector3D u = p2 - p1;
    QVector3D v = p3 - p1;
    QVector3D n = QVector3D::crossProduct(u, v);
    // check for degenerate triangle
    if (n.x() == 0 && n.y() == 0 && n.z() == 0)
        return false;
    QVector3D w0 = ray.pt - p1;
    float a = -QVector3D::dotProduct(n, w0);
    float b = QVector3D::dotProduct(n, ray.dir);
    if (abs(b) < 1E-5) {
        if (a == 0)
            return true;        // ray lies in triangle plane
        return false;           // ray disjoint from plane
    }

    // get intersect point of ray with triangle plane
    float r = a / b;
    if (r < 0.0)                // ray goes away from triangle
        return false;

    QVector3D pt = ray.pt + r * ray.dir; // intersect point of ray and plane

    // is point in triangle?
    return PointInTriangle(pt, p1, p2, p3);
}

bool SubdivisionFace::intersectWithRay(const PickRay& ray) const
{
    if (_points.size() < 3)
        return false;
    if (_points.size() == 3)
        return privIntersectWithRay(ray, _points[0]->getCoordinate(), _points[1]->getCoordinate(),
                                    _points[2]->getCoordinate());
    for (size_t i=2; i<_points.size(); i++) {
        if (privIntersectWithRay(ray, _points[0]->getCoordinate(), _points[i-1]->getCoordinate(),
                                 _points[i]->getCoordinate()))
            return true;
    }
    return false;
}

SubdivisionPoint* SubdivisionFace::calculateFacePoint()
{
    SubdivisionPoint* result = 0;
    QVector3D centre = ZERO;
    if (_points.size() < 3)
        throw invalid_argument("trying to calculate face point with less than 3 points");
    if (_points.size() > 3 || _owner->getSubdivisionMode() == fmCatmullClark) {
        for (size_t i=0; i<_points.size(); ++i) {
            QVector3D p = _points[i]->getCoordinate();
            centre += p;
        }
        centre /= _points.size();
        result = SubdivisionPoint::construct(_owner);
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
	if (_points.size() < 2)
		return;
    size_t mid = _points.size() / 2 - 1;
    for (size_t i=0; i<=mid; ++i)
        swap(_points[i], _points[_points.size() - i - 1]);
}

void SubdivisionFace::edgeCheck(SubdivisionPoint* p1,
                                SubdivisionPoint* p2,
                                bool crease,
                                bool controledge,
                                SubdivisionControlCurve* curve,
                                vector<SubdivisionEdge*> &interioredges,
                                vector<SubdivisionEdge*> &controledges
                                )
{
    if (p1 == nullptr || p2 == nullptr)
        throw invalid_argument("null end points in SubdivisionFace::edgeCheck");
    SubdivisionEdge* newedge = _owner->edgeExists(p1, p2);
    if (newedge == nullptr) {
        newedge = SubdivisionEdge::construct(_owner);
        newedge->setControlEdge(controledge);
        newedge->setPoints(p1, p2);
        newedge->startPoint()->addEdge(newedge);
        newedge->endPoint()->addEdge(newedge);
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
    newedge->addFace(this);
}

// TODO: facepoints should be common to this face, and then we don't
// have to search the list each time to find the facepoint for this
// face
void SubdivisionFace::subdivide(bool controlface,
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

    if (_points.size() != 3 || _owner->getSubdivisionMode() == fmCatmullClark) {
        for (size_t i=1; i<=_points.size(); ++i) {
            p2 = _points[i-1];
            size_t index = (i + _points.size() - 2) % _points.size();
            prevedge = _owner->edgeExists(p2, _points[index]);
            index = (i + _points.size()) % _points.size();
            curedge = _owner->edgeExists(p2, _points[index]);
            index = (i - 1) % 4;
            ptmpindex = find_if(vertexpoints.begin(), vertexpoints.end(),
                                FirstPointPairPredicate(p2));
            pts[index] = (*ptmpindex).second; // p2.newlocation
            SubdivisionPoint* p2pt = pts[index];
            index = (index + 1) % 4;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(),
                                FirstEdgePointPredicate(curedge));
            pts[index] = (*etmpindex).second; // curedge.newlocation
            SubdivisionPoint* curredgept = pts[index];
            index = (index + 1) % 4;
            ftmpindex = find_if(facepoints.begin(), facepoints.end(),
                                FirstFacePointPredicate(this));
            pts[index] = (*ftmpindex).second; // this.newlocation
            SubdivisionPoint* newlocation = pts[index];
            index = (index + 1) % 4;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(),
                                FirstEdgePointPredicate(prevedge));
            pts[index] = (*etmpindex).second; // prevedge.newlocation
            SubdivisionPoint* prevedgept = pts[index];
            // add the new face
            newface = SubdivisionFace::construct(_owner);
            dest.push_back(newface);
            // check the edges of the new face
            newface->edgeCheck(prevedgept, p2pt, prevedge->isCrease(),
							   prevedge->isControlEdge() || controlface,
							   prevedge->getCurve(), interioredges, controledges);
            newface->edgeCheck(p2pt, curredgept, curedge->isCrease(),
							   curedge->isControlEdge() || controlface,
							   curedge->getCurve(), interioredges, controledges);
            newface->edgeCheck(curredgept, newlocation, false, false, 0,
							   interioredges, controledges);
            newface->edgeCheck(prevedgept, newlocation, false, false, 0,
							   interioredges, controledges);
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
            prevedge = _owner->edgeExists(p2, _points[index]);
            index = (i+numberOfPoints()) % numberOfPoints();
            curedge = _owner->edgeExists(p2, _points[index]);

            index = 0;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(),
                                FirstEdgePointPredicate(prevedge));
            pts[index] = (*etmpindex).second;
            index = 1;
            ptmpindex = find_if(vertexpoints.begin(), vertexpoints.end(),
                                FirstPointPairPredicate(p2));
            pts[index] = (*ptmpindex).second;
            index = 2;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(),
                                FirstEdgePointPredicate(curedge));
            pts[index] = (*etmpindex).second;
            // add the new face
            newface = SubdivisionFace::construct(_owner);
            dest.push_back(newface);
            // check the edges of the new face
            newface->edgeCheck(pts[0], pts[1], prevedge->isCrease(),
							   prevedge->isControlEdge() || controlface,
							   prevedge->getCurve(), interioredges, controledges);
            newface->edgeCheck(pts[1], pts[2], curedge->isCrease(),
                               curedge->isControlEdge() || controlface,
							   curedge->getCurve(), interioredges, controledges);
            newface->edgeCheck(pts[2], pts[0], false, false, 0,
							   interioredges, controledges);
            for (size_t j=0; j<3; ++j) {
                // add new face to points
                pts[j]->addFace(newface);
                newface->addPoint(pts[j]);
            }
        }

        // then the center triangle
        for (size_t i=0; i<numberOfPoints(); ++i) {
            p2 = _points[i];
            size_t index = (i - 1 + numberOfPoints()) % numberOfPoints();
            prevedge = _owner->edgeExists(p2, _points[index]);
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(),
                                FirstEdgePointPredicate(prevedge));
            pts[i] = (*etmpindex).second;
        }
        // add the new face
        newface = SubdivisionFace::construct(_owner);
        dest.push_back(newface);
        newface->edgeCheck(pts[0], pts[1], false, false, 0,
						   interioredges, controledges);
        newface->edgeCheck(pts[1], pts[2], false, false, 0,
						   interioredges, controledges);
        newface->edgeCheck(pts[2], pts[0], false, false, 0,
						   interioredges, controledges);
        for (size_t j=0; j<3; ++j) {
            // add new face to points
            pts[j]->addFace(newface);
            newface->addPoint(pts[j]);
        }
    }
}

void SubdivisionFace::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionFace ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionFace::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionBase::priv_dump(os, prefix);
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionFace& face)
{
    face.dump(os);
    return os;
}

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionControlFace* SubdivisionControlFace::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getControlFacePool().add();
    return new (mem) SubdivisionControlFace(owner);
}

SubdivisionControlFace::SubdivisionControlFace(SubdivisionSurface *owner)
    : SubdivisionFace(owner), _layer(0), _min(ZERO), _max(ZERO),
      _vertices1(0), _vertices2(0)
{
	// does nothing
}

// FreeGeometry.pas:11953
void SubdivisionControlFace::removeFace()
{
    // remove from layer
    setLayer(0);
    SubdivisionPoint* p1 = getPoint(numberOfPoints() - 1);
    for (size_t i=0; i<numberOfPoints(); ++i) {
        SubdivisionPoint* p2 = getPoint(i);
        p2->deleteFace(this);
        SubdivisionControlEdge* edge = _owner->controlEdgeExists(p1, p2);
        if (edge != 0) {
            edge->deleteFace(this);
            if (edge->numberOfFaces() == 0)
                _owner->deleteControlEdge(edge);
        }
        p1 = p2;
    }
    clear();
}

// add face to display list, face is constant color
static void addFaceToDL(QVector<QVector3D>& vertices, QVector<QVector3D>& normals,
                        const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    QVector3D n = UnifiedNormal(p1, p2, p3);
    vertices << p1 << p2 << p3;
    normals << n << n << n;
}

// add face to display list, face has a different color at each vertex, colors are provided as r,g,b triples
static void addCurveFaceToDL(QVector<QVector3D>& vertices,
                             QVector<QVector3D>& colors,
                             QVector<QVector3D>& normals,
                             const QVector3D& p1, const QVector3D& p2, const QVector3D& p3,
                             const QVector3D& c1, const QVector3D& c2, const QVector3D& c3)
{
    QVector3D n = UnifiedNormal(p1, p2, p3);
    vertices << p1 << p2 << p3;
    colors << c1 << c2 << c3;
    normals << n << n << n;
}

// add face to display list, face has different color at each vertex, colors are QColors
static void addCurveFaceToDL(QVector<QVector3D>& vertices,
                             QVector<QVector3D>& colors,
                             QVector<QVector3D>& normals,
                             const QVector3D& p1, const QVector3D& p2, const QVector3D& p3,
                             const QColor& c1, const QColor& c2, const QColor& c3,
                             const QVector3D& n1, const QVector3D& n2, const QVector3D& n3)
{
    vertices << p1 << p2 << p3;
    colors << QVector3D(c1.redF(), c1.blueF(), c1.greenF())
           << QVector3D(c2.redF(), c2.blueF(), c2.greenF())
           << QVector3D(c3.redF(), c3.blueF(), c3.greenF());
    normals << n1 << n2 << n3;
}

// constant used in zebra striping
const float Width = 0.02;

// structure used in zebra striping
struct ZebraIntersection 
{
    QVector3D p;
    float dotprod;

    ZebraIntersection(const QVector3D& pt, float dp)
        : p(pt), dotprod(dp) {}
};

// function used during zebra shading, splits triangle up based on coloring
static void zebraProcess(const QVector3D& p, float dp,
                         vector<ZebraIntersection>& intersections)
{
    float prevdp = intersections.back().dotprod;
    float lf;
    modf(prevdp/Width, &lf);
    float hf;
    modf(dp/Width, &hf);
    int lo = static_cast<int>(lf);
    int hi = static_cast<int>(hf);
    if ((hi - lo) > 0) {
        const QVector3D& prevp = intersections.back().p;
        for (int i=lo; i<=hi; i++) {
            float val = i * Width;
            if (val > prevdp && val < dp) {
                float t = (val - prevdp) / (dp - prevdp);
                QVector3D p3d = prevp + t * (p - prevp);
                intersections.push_back(ZebraIntersection(p3d, val));
            }
        }
        intersections.push_back(ZebraIntersection(p, dp));
    } else if ((lo - hi) > 0) {
        const QVector3D& prevp = intersections.back().p;
        for (int i=lo; i>=hi; i--) {
            float val = i * Width;
            if (val < prevdp && val > dp) {
                float t = (val - prevdp) / (dp - prevdp);
                QVector3D p3d = prevp + t * (p - prevp);
                intersections.push_back(ZebraIntersection(p3d, val));
            }
        }
        intersections.push_back(ZebraIntersection(p, dp));
    } else
        intersections.push_back(ZebraIntersection(p, dp));
}

// returns true if points are outside of previous extents
bool checkExtents(const QVector3D& fmin, const QVector3D& fmax, const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    QVector3D newmin;
    QVector3D newmax;
    MinMax(p1, newmin, newmax);
    MinMax(p2, newmin, newmax);
    MinMax(p3, newmin, newmax);
    if (newmin.x() < fmin.x() || newmin.y() < fmin.y() || newmin.z() < fmin.z())
        return true;
    if (newmax.x() > fmax.x() || newmax.y() > fmax.y() || newmax.z() > fmax.z())
        return true;
    if (abs(newmax.x() - newmax.x()) > 2.0)
        return true;
    return false;
}

// draw a face with zebra shading
static void zebraDrawFace(bool check, const QVector3D& facemin, const QVector3D& facemax,
                          Viewport& vp,
                          QVector<QVector3D>& vertices,
                          QVector<QVector3D>& colors,
                          QVector<QVector3D>& normals,
                          const QColor& color, const QColor& zebraColor,
                          const QVector3D& p1, const QVector3D& p2, const QVector3D& p3,
                          const QVector3D& n1, const QVector3D& n2, const QVector3D& n3)
{
    vector<ZebraIntersection> intersections;
    QVector3D eye = (vp.getCamera() - p1).normalized();
    float d1 = QVector3D::dotProduct(eye, n1);
    if (d1 < 0)
        d1 = -d1;
    eye = (vp.getCamera() - p2).normalized();
    float d2 = QVector3D::dotProduct(eye, n2);
    if (d2 < 0)
        d2 = -d2;
    eye = (vp.getCamera() - p3).normalized();
    float d3 = QVector3D::dotProduct(eye, n3);
    if (d3 < 0)
        d3 = -d3;
    float mindp = d1;
    float maxdp = d1;
    mindp = min(d2, mindp);
    maxdp = max(d2, maxdp);
    mindp = min(d3, mindp);
    maxdp = max(d3, maxdp);
    float lof;
    modf(mindp/Width, &lof);
    if (lof < 0)
        lof = 0;
    float hif;
    modf(maxdp/Width, &hif);
    if (hif > roundf(1/Width))
        hif = roundf(1/Width);
    int lo = static_cast<int>(lof);
    int hi = static_cast<int>(hif);
    if (lo == hi) {
        if ((lo % 2) == 0) {      // lo is even
            if (check && checkExtents(facemin, facemax, p1, p2, p3))
                cout << "bad stuff" << endl;
            addCurveFaceToDL(vertices, colors, normals, p1, p2, p3, color, color, color,
                             n1, n2, n3);
        }
        else {
            if (check && checkExtents(facemin, facemax, p1, p2, p3))
                cout << "bad stuff" << endl;
            addCurveFaceToDL(vertices, colors, normals, p1, p2, p3,
                             zebraColor, zebraColor, zebraColor,
                             n1, n2, n3);
        }
    } else {
        intersections.push_back(ZebraIntersection(p1, d1));
        zebraProcess(p2, d2, intersections);
        zebraProcess(p3, d3, intersections);
        zebraProcess(p1, d1, intersections);
        for (int i=lo; i<=hi+1; i++) {
            vector<QVector3D*> pts;
            for (size_t j=0; j<intersections.size()-1; j++) {
                if (((intersections[j].dotprod >= ((i-1)*Width))
                     || (fabs(intersections[j].dotprod-((i-1)*Width)) < 1E-6))
                    && ((intersections[j].dotprod <= (i*Width))
                        || (fabs(intersections[j].dotprod-(i*Width)) < 1E-6))) {
                    pts.push_back(&(intersections[j].p));
                }
            }
            for (size_t j=2; j<pts.size(); j++) {
                if (check && checkExtents(facemin, facemax, *pts[0], *pts[j-1], *pts[j]))
                    cout << "bad stuff" << endl;
                if ((i % 2)) {    // i is odd
                    addCurveFaceToDL(vertices, colors, normals, *pts[0], *pts[j-1], *pts[j],
                                     color, color, color,
                                     n1, n2, n3);
                }
                else {
                    addCurveFaceToDL(vertices, colors, normals, *pts[0], *pts[j-1], *pts[j],
                                     zebraColor, zebraColor, zebraColor,
                                     n1, n2, n3);
                }
            }
        }
    }
}

// FreeGeometry.pas:12438
void SubdivisionControlFace::drawZebraFaces(Viewport& vp, CurveFaceShader* shader)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> colors;
    QVector<QVector3D> normals;
    // reserve the size of the lists to save memory allocations
    if (_vertices1 != 0) {
        vertices.reserve(_vertices1);
        colors.reserve(_vertices1);
        normals.reserve(_vertices1);
    }
    for (size_t i=0; i<_children.size(); ++i) {
        for (size_t j=2; j<_children[i]->numberOfPoints(); ++j) {
            SubdivisionPoint* sp1 = _children[i]->getPoint(0);
            SubdivisionPoint* sp2 = _children[i]->getPoint(j-1);
            SubdivisionPoint* sp3 = _children[i]->getPoint(j);
            QVector3D p1 = sp1->getCoordinate();
            QVector3D p2 = sp2->getCoordinate();
            QVector3D p3 = sp3->getCoordinate();

            // get min max of this face
            QVector3D facemin;
            QVector3D facemax;
            MinMax(p1, facemin, facemax);
            MinMax(p2, facemin, facemax);
            MinMax(p3, facemin, facemax);
            
            QVector3D vec = UnifiedNormal(p1, p2, p3);
            QVector3D n1 = (sp1->getVertexType() == svRegular || sp1->getVertexType() == svDart) ?
                sp1->getNormal() : vec;
            QVector3D n2 = (sp2->getVertexType() == svRegular || sp2->getVertexType() == svDart) ?
                sp2->getNormal() : vec;
            QVector3D n3 = (sp3->getVertexType() == svRegular || sp3->getVertexType() == svDart) ?
                sp3->getNormal() : vec;
            zebraDrawFace(true, facemin, facemax, vp, vertices, colors, normals, getColor(),
                          _owner->getZebraColor(), p1, p2, p3, n1, n2, n3);
            if (_owner->drawMirror() && getLayer()->isSymmetric()) {
                p1.setY(-p1.y());
                p2.setY(-p2.y());
                p3.setY(-p3.y());
                n1.setY(-n1.y());
                n2.setY(-n2.y());
                n3.setY(-n3.y());
                zebraDrawFace(false, facemin, facemax, vp, vertices, colors, normals, getColor(),
                              _owner->getZebraColor(), p1, p2, p3, n1, n2, n3);
            }
        }
    }
    if (vertices.size() > 0) {
        shader->renderMesh(vertices, colors, normals);
        _vertices1 = vertices.size();
    }
}

// FreeGeometry.pas:11995
void SubdivisionControlFace::drawFaces(Viewport &vp, FaceShader* faceshader)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> vertices_underwater;
    QVector<QVector3D> normals_underwater;
    // reserve the size of the lists to save memory allocations
    if (_vertices1 != 0) {
        vertices.reserve(_vertices1);
        normals.reserve(_vertices1);
    }
    if (_vertices2 != 0) {
        vertices_underwater.reserve(_vertices2);
        normals_underwater.reserve(_vertices2);
    }

    if (_owner->shadeUnderWater() && vp.getViewportMode() == vmShade
            && getLayer()->useInHydrostatics()) {
        for (size_t i=0; i<_children.size(); ++i) {
            // clip all triangles against the waterline plane
            for (size_t j=2; j<_children[i]->numberOfPoints(); ++j) {
                QVector3D p1 = _children[i]->getPoint(0)->getCoordinate();
                QVector3D p2 = _children[i]->getPoint(j-1)->getCoordinate();
                QVector3D p3 = _children[i]->getPoint(j)->getCoordinate();

                // check if clipping is required
                float min = _owner->getWaterlinePlane().distance(p1);
                float max = min;
                float tmp = _owner->getWaterlinePlane().distance(p2);
                if (tmp < min)
                    min = tmp;
                else if (tmp > max)
                    max = tmp;
                tmp = _owner->getWaterlinePlane().distance(p3);
                if (tmp < min)
                    min = tmp;
                else if (tmp > max)
                    max = tmp;
                if (max <= 0.0) {
                    // entirely below the plane
                    addFaceToDL(vertices_underwater, normals_underwater, p1, p2, p3);
                }
                else if (min >= 0.0) {
                    // entirely above the plane
                    addFaceToDL(vertices, normals, p1, p2, p3);
                }
                else {
                    // pierces water, clip triangle
                    std::vector<QVector3D> above;
                    above.reserve(6);
                    std::vector<QVector3D> below;
                    below.reserve(6);
                    ClipTriangle(p1, p2, p3, _owner->getWaterlinePlane(), above, below);
                    for (size_t k=2; k<above.size(); ++k)
                        // shade triangle above
                        addFaceToDL(vertices, normals, above[0], above[k-1], above[k]);
                    for (size_t k=2; k<below.size(); ++k)
                        // shade triangle below
                        addFaceToDL(vertices_underwater, normals_underwater, below[0], below[k-1], below[k]);
                }
                if (_owner->drawMirror() && getLayer()->isSymmetric()) {
                    p1.setY(-p1.y());
                    p2.setY(-p2.y());
                    p3.setY(-p3.y());

                    // check if clipping is required
                    float min = _owner->getWaterlinePlane().distance(p1);
                    float max = min;
                    float tmp = _owner->getWaterlinePlane().distance(p2);
                    if (tmp < min)
                        min = tmp;
                    else if (tmp > max)
                        max = tmp;
                    tmp = _owner->getWaterlinePlane().distance(p3);
                    if (tmp < min)
                        min = tmp;
                    else if (tmp > max)
                        max = tmp;
                    if (max <= 0.0) {
                        // entirely below the plane
                        addFaceToDL(vertices_underwater, normals_underwater, p1, p2, p3);
                    }
                    else if (min >= 0.0) {
                        // entirely above the plane
                        addFaceToDL(vertices, normals, p1, p2, p3);
                    }
                    else {
                        // pierces water, clip triangle
                        std::vector<QVector3D> above;
                        above.reserve(6);
                        std::vector<QVector3D> below;
                        below.reserve(6);
                        ClipTriangle(p1, p2, p3, _owner->getWaterlinePlane(), above, below);
                        for (size_t k=2; k<above.size(); ++k)
                            // shade triangle above
                            addFaceToDL(vertices, normals, above[0], above[k-1], above[k]);
                        for (size_t k=2; k<below.size(); ++k)
                            // shade triangle below
                            addFaceToDL(vertices_underwater, normals_underwater, below[0], below[k-1], below[k]);
                    }
                }
            }
        }
    } else {
        for (size_t i=0; i<_children.size(); ++i) {
            for (size_t j=2; j<_children[i]->numberOfPoints(); ++j) {
                QVector3D p1 = _children[i]->getPoint(0)->getCoordinate();
                QVector3D p2 = _children[i]->getPoint(j-1)->getCoordinate();
                QVector3D p3 = _children[i]->getPoint(j)->getCoordinate();
                addFaceToDL(vertices, normals, p1, p2, p3);
                if (_owner->drawMirror() && getLayer()->isSymmetric()) {
                    p1.setY(-p1.y());
                    p2.setY(-p2.y());
                    p3.setY(-p3.y());
                    addFaceToDL(vertices, normals, p1, p2, p3);
                }
            }
        }
    }
        
    // render above the waterline
    if (vertices.size() > 0) {
        faceshader->renderMesh(getColor(), vertices, normals);
        _vertices1 = vertices.size();
    }
    // render below the waterline
    if (vertices_underwater.size() > 0) {
        faceshader->renderMesh(_owner->getUnderWaterColor(), vertices_underwater, normals_underwater);
        _vertices2 = vertices_underwater.size();
    }
}

// developable surface shading
void SubdivisionControlFace::drawDevelopableFaces(CurveFaceShader* faceshader)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> colors;
    QVector<QVector3D> normals;
    // reserve the size of the lists to save memory allocations
    if (_vertices1 != 0) {
        vertices.reserve(_vertices1);
        colors.reserve(_vertices1);
        normals.reserve(_vertices1);
    }

    QVector3D low(0,1.0,0);
    QVector3D hi(1.0,0,0);
    for (size_t i=0; i<_children.size(); ++i) {
        for (size_t j=2; j<_children[i]->numberOfPoints(); ++j) {
            QVector3D p1 = _children[i]->getPoint(0)->getCoordinate();
            QVector3D p2 = _children[i]->getPoint(j-1)->getCoordinate();
            QVector3D p3 = _children[i]->getPoint(j)->getCoordinate();
            // do developable curvature
            size_t idx = _owner->indexOfPoint(_children[i]->getPoint(0));
            float c1 = _owner->getGaussCurvature(idx);
            idx = _owner->indexOfPoint(_children[i]->getPoint(j-1));
            float c2 = _owner->getGaussCurvature(idx);
            idx = _owner->indexOfPoint(_children[i]->getPoint(j));
            float c3 = _owner->getGaussCurvature(idx);
            QVector3D& cc1 = (fabs(c1) < 1e-4) ? low : hi;
            QVector3D& cc2 = (fabs(c2) < 1e-4) ? low : hi;
            QVector3D& cc3 = (fabs(c3) < 1e-4) ? low : hi;
            addCurveFaceToDL(vertices, colors, normals, p1, p2, p3, cc1, cc2, cc3);
            if (_owner->drawMirror() && getLayer()->isSymmetric()) {
                p1.setY(-p1.y());
                p2.setY(-p2.y());
                p3.setY(-p3.y());
                addCurveFaceToDL(vertices, colors, normals, p1, p2, p3, cc1, cc2, cc3);
            }
        }
    }
    if (vertices.size() > 0) {
        faceshader->renderMesh(vertices, colors, normals);
        _vertices1 = vertices.size();
    }
}

float Fragment(float curvature, float mincurvature, float maxcurvature)
{
    float frag;
    float contrast = 0.1;
    if (curvature > -1e-4 && curvature < 1e-4)
        frag = 0.5;
    else {
        if (curvature > 0)
            frag = 0.5 + 0.5 * pow(curvature/maxcurvature, contrast);
        else
            frag = 0.5 - 0.5 * pow(curvature/mincurvature, contrast);
        frag = 1 - frag;
    }
    return frag;
}

// FreeGeometry.pas:12438
void SubdivisionControlFace::drawCurvatureFaces(CurveFaceShader* shader, float MinCurvature, float MaxCurvature)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> colors;
    QVector<QVector3D> normals;
    // reserve the size of the lists to save memory allocations
    if (_vertices1 != 0) {
        vertices.reserve(_vertices1);
        colors.reserve(_vertices1);
        normals.reserve(_vertices1);
    }

    for (size_t i=0; i<_children.size(); ++i) {
        for (size_t j=2; j<_children[i]->numberOfPoints(); ++j) {
            QVector3D p1 = _children[i]->getPoint(0)->getCoordinate();
            QVector3D p2 = _children[i]->getPoint(j-1)->getCoordinate();
            QVector3D p3 = _children[i]->getPoint(j)->getCoordinate();
            // do developable curvature
            size_t idx = _owner->indexOfPoint(_children[i]->getPoint(0));
            float c1 = _owner->getGaussCurvature(idx);
            QColor cc1 = FillColor(Fragment(c1, MinCurvature, MaxCurvature));
            idx = _owner->indexOfPoint(_children[i]->getPoint(j-1));
            float c2 = _owner->getGaussCurvature(idx);
            QColor cc2 = FillColor(Fragment(c2, MinCurvature, MaxCurvature));
            idx = _owner->indexOfPoint(_children[i]->getPoint(j));
            float c3 = _owner->getGaussCurvature(idx);
            QColor cc3 = FillColor(Fragment(c3, MinCurvature, MaxCurvature));
            QVector3D n = UnifiedNormal(p1, p2, p3);
            addCurveFaceToDL(vertices, colors, normals, p1, p2, p3,
                             cc1, cc2, cc3,
                             n, n, n);
            if (_owner->drawMirror() && getLayer()->isSymmetric()) {
                p1.setY(-p1.y());
                p2.setY(-p2.y());
                p3.setY(-p3.y());
                n = UnifiedNormal(p1, p2, p3);
                addCurveFaceToDL(vertices, colors, normals, p1, p2, p3,
                                 cc1, cc2, cc3,
                                 n, n, n);
            }
        }
    }
    if (vertices.size() > 0) {
        shader->renderMesh(vertices, colors, normals);
        _vertices1 = vertices.size();
    }
}

// function to draw a normal
static void DrawNormal(QVector3D p, QVector3D n, bool draw_mirror, float mainframe,
                       Viewport& vp, QVector<QVector3D>& vertices)
{
    if (vp.getViewportType() == fvBodyplan && !draw_mirror && p.x() <= mainframe) {
        p.setY(-p.y());
        n.setY(-n.y());
    }
    // TODO fix this so it is a fixed length, should be figured out by viewport
    // scale the length of the normal such that it is a fixed length relative to the screen resolution
    float ldes = 0.00075 * vp.width();
    if (ldes < 10)
        ldes = 10;
    n *= ldes;
    vertices << p << p + n;
}

void SubdivisionControlFace::draw(Viewport& vp, LineShader* lineshader)
{
    QVector<QVector3D> vertices;
    if (vp.getViewportMode() != vmWireFrame)
        return;
    // vmWireFrame
    // draw interior edges (not descending from controledges)
    QColor edgeColor;
    if (isSelected())
        edgeColor = _owner->getSelectedColor();
    else
        edgeColor = getLayer()->getColor();
    for (size_t i=0; i<_edges.size(); ++i)
        _edges[i]->draw(_owner->drawMirror() && getLayer()->isSymmetric(), vp, lineshader, edgeColor);
    // draw edges descending from controledges, but slightly darker than interior
    // edges
    if (!isSelected())
        edgeColor = QColor(getLayer()->getColor().redF()*0.6,
                           getLayer()->getColor().greenF()*0.6,
                           getLayer()->getColor().blueF()*0.6);
    for (size_t i=0; i<_control_edges.size(); ++i)
        _control_edges[i]->draw(_owner->drawMirror() && getLayer()->isSymmetric(), vp, lineshader, edgeColor);
    // show normals
    if (isSelected() && _owner->showNormals()) {
        QVector<QVector3D>& vertices = lineshader->getVertexBuffer();
        // first assemble all points with this controlface
        set<SubdivisionPoint*> points;
        for (size_t i=0; i<_children.size(); ++i)
            for (size_t j=0; j<_children[i]->numberOfPoints(); ++j)
                points.insert(_children[i]->getPoint(j));
        set<SubdivisionPoint*>::iterator i = points.begin();
        for ( ; i!=points.end(); ++i) {
            SubdivisionPoint* point = *i;
            QVector3D p1 = point->getCoordinate();
            QVector3D p2 = point->getNormal();
            DrawNormal(p1, p2, _owner->drawMirror(), _owner->getMainframeLocation(), vp, vertices);
            if (getLayer()->isSymmetric() && _owner->drawMirror()) {
                p1.setY(-p1.y());
                p2.setY(-p2.y());
                DrawNormal(p1, p2, _owner->drawMirror(), _owner->getMainframeLocation(), vp, vertices);
            }
        }
        lineshader->renderLines(vertices, _owner->getNormalColor());
    }
}

// FreeGeometry.pas:12496
SubdivisionControlEdge* SubdivisionControlFace::insertControlEdge(
        SubdivisionControlPoint *p1, SubdivisionControlPoint *p2, bool& deleteme)
{
    SubdivisionControlEdge* result = 0;
    deleteme = false;
    if (p1->hasFace(this) && p2->hasFace(this)) {
        if (_owner->edgeExists(p1, p2) != 0)
            return result;
        size_t tmp = indexOfPoint(p1);
        vector<SubdivisionControlPoint*> pts;
        pts.push_back(p1);
        for (size_t i=0; i<numberOfPoints(); ++i) {
            tmp = (tmp + 1) % numberOfPoints();
            pts.push_back(dynamic_cast<SubdivisionControlPoint*>(getPoint(tmp)));
            if (pts.back() == p2)
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
            if (pts.back() == p1)
                break;
        }
        if (pts.size() > 2)
            _owner->addControlFace(pts, false, getLayer());
        deleteme = true;
    }
    result = _owner->controlEdgeExists(p1, p2);
    if (result != 0)
        result->setCrease(false);
    return result;
}

SubdivisionFace* SubdivisionControlFace::getChild(size_t index) const
{
    if (index < _children.size())
        return _children[index];
    throw out_of_range("Bad index in SubdivisionControlFace::getChild");
}

QColor SubdivisionControlFace::getColor() const
{
    if (isSelected())
        return _owner->getSelectedColor();
    else
        return _layer->getColor();
}

SubdivisionEdge* SubdivisionControlFace::getControlEdge(size_t index) const
{
    if (index < _control_edges.size())
        return _control_edges[index];
    throw out_of_range("bad index in SubdivisionControlFace::getControlEdge");
}

void SubdivisionControlFace::setSelected(bool val)
{
    if (val)
        _owner->setSelectedControlFace(this);
    else
        _owner->removeSelectedControlFace(this);
}

bool SubdivisionControlFace::isSelected() const
{
    return _owner->hasSelectedControlFace(this);
}

SubdivisionEdge* SubdivisionControlFace::getEdge(size_t index) const
{
    if (index < _edges.size())
        return _edges[index];
    throw out_of_range("bad index in SubdivisionControlFace::getEdge");
}

bool SubdivisionControlFace::isVisible() const
{
    if (_layer != 0 && _layer->isVisible())
        return true;
    return isSelected();
}

void SubdivisionControlFace::setLayer(SubdivisionLayer *layer)
{
    if (layer == _layer)
        return;
    if (_layer != 0) {
        // disconnect from current layer
        _layer->releaseControlFace(this);
        _layer = 0;
    }
    _layer = layer;
    if (layer != 0)
        layer->useControlFace(this);
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
        for (size_t i=0; i<_children.size(); ++i) {
            SubdivisionFace* face = _children[i];
            for (size_t j=0; j<face->numberOfPoints(); ++j) {
                if (i == 0 && j == 0) {
                    _min = face->getPoint(j)->getCoordinate();
                    _max = _min;
                }
                MinMax(face->getPoint(j)->getCoordinate(), _min, _max);
            }
        }
    }
    else {
        for (size_t i=1; i<numberOfPoints(); ++i) {
            MinMax(getPoint(i)->getCoordinate(), _min, _max);
        }
    }
}

void SubdivisionControlFace::clear()
{
    clearChildren();
	clearControlEdges();
    _layer = 0;
	_min = _max = ZERO;
}

// used to clear all subdivided edges and faces, but not the subdivided points
void SubdivisionControlFace::clearChildren()
{
    for (size_t i=0; i<_children.size(); ++i)
        _owner->getFacePool().del(_children[i]);
    for (size_t i=0; i<_edges.size(); ++i)
        _owner->getEdgePool().del(_edges[i]);
    _children.clear();
    _edges.clear();
}

void SubdivisionControlFace::loadBinary(FileBuffer &source)
{
    // read controlpoint data
    quint32 n, index;
    SubdivisionControlPoint* p1;
    source.load(n);
    _points.clear();
    for (size_t i=0; i<n; ++i) {
        source.load(index);
        p1 = _owner->getControlPoint(index);
        _points.push_back(p1);
        p1->addFace(this);
    }
    // read layer index
    quint32 ind;
    source.load(ind);
    if (ind < _owner->numberOfLayers())
        _layer = _owner->getLayer(ind);
    else
        _layer = _owner->getLayer(0); // reference to an invalid layer, assign to owners default layer
    _layer->useControlFace(this);
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
            cerr << "SubdivisionControlFace::loadBinary - Could not find edge!" << endl;
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

void SubdivisionControlFace::saveBinary(FileBuffer &destination) const
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

void SubdivisionControlFace::saveToDXF(QStringList& strings) const
{
    QString layername = getLayer()->getName();
    int colorindex = FindDXFColorIndex(getLayer()->getColor());
    if (numberOfPoints() == 4) {
        // create one polymesh for all childfaces
        ControlFaceGrid facedata;
        facedata.setRows(1);
        facedata.setCols(1);
        facedata.setFace(0, 0, const_cast<SubdivisionControlFace*>(this));
        PointGrid grid;
        _owner->convertToGrid(facedata, grid);
        if (grid.rows() > 0 && grid.cols() > 0) {
            strings.push_back(QString("0\r\nPOLYLINE"));
            strings.push_back(QString("8\r\n%1").arg(layername));
            strings.push_back(QString("62\r\n%1").arg(colorindex));
            strings.push_back(QString("66\r\n1"));
            strings.push_back(QString("70\r\n16"));
            strings.push_back(QString("71\r\n%1").arg(grid.cols()));
            strings.push_back(QString("72\r\n%1").arg(grid.rows()));
            for (size_t i=0; i<grid.rows(); i++) {
                for (size_t j=0; j<grid.cols(); j++) {
                    strings.push_back(QString("0\r\nVERTEX"));
                    strings.push_back(QString("8\r\n%1").arg(layername));
                    QVector3D p = grid.getPoint(i, j)->getCoordinate();
                    strings.push_back(QString("10\r\n%1").arg(Truncate(p.x(), 4)));
                    strings.push_back(QString("20\r\n%1").arg(Truncate(p.y(), 4)));
                    strings.push_back(QString("30\r\n%1").arg(Truncate(p.z(), 4)));
                    strings.push_back(QString("70\r\n64")); // polygon mesh vertex
                }
            }
            strings.push_back(QString("0\r\nSEQEND"));
            if (getLayer()->isSymmetric() && getOwner()->drawMirror()) {
                strings.push_back(QString("0\r\nPOLYLINE"));
                strings.push_back(QString("8\r\n%1").arg(layername));
                strings.push_back(QString("62\r\n%1").arg(colorindex));
                strings.push_back(QString("66\r\n1"));
                strings.push_back(QString("70\r\n16"));
                strings.push_back(QString("71\r\n%1").arg(grid.cols()));
                strings.push_back(QString("72\r\n%1").arg(grid.rows()));
                for (size_t i=0; i<grid.rows(); i++) {
                    for (size_t j=0; j<grid.cols(); j++) {
                        strings.push_back(QString("0\r\nVERTEX"));
                        strings.push_back(QString("8\r\n%1").arg(layername));
                        QVector3D p = grid.getPoint(i, j)->getCoordinate();
                        strings.push_back(QString("10\r\n%1").arg(Truncate(p.x(), 4)));
                        strings.push_back(QString("20\r\n%1").arg(Truncate(-p.y(), 4)));
                        strings.push_back(QString("30\r\n%1").arg(Truncate(p.z(), 4)));
                        strings.push_back(QString("70\r\n64")); // polygon mesh vertex
                    }
                }
                strings.push_back(QString("0\r\nSEQEND"));
            }
        }
    } else {
        // send all child faces as 3D faces
        for (size_t j=0; j<_children.size(); j++) {
            SubdivisionFace* face = _children[j];
            strings.push_back(QString("0\r\n3DFACE"));
            strings.push_back(QString("8\r\n%1").arg(layername));
            strings.push_back(QString("62\r\n%1").arg(colorindex));
            for (size_t k=0; k<face->numberOfPoints(); ++k) {
                QVector3D p = face->getPoint(k)->getCoordinate();
                strings.push_back(QString("%1\r\n%2").arg(10+k).arg(Truncate(p.x(), 4)));
                strings.push_back(QString("%1\r\n%2").arg(20+k).arg(Truncate(p.y(), 4)));
                strings.push_back(QString("%1\r\n%2").arg(30+k).arg(Truncate(p.z(), 4)));
            }
            if (face->numberOfPoints() == 3) {
                QVector3D p = face->getPoint(2)->getCoordinate();
                // 4th point is same as third
                strings.push_back(QString("13\r\n%1").arg(Truncate(p.x(), 4)));
                strings.push_back(QString("23\r\n%1").arg(Truncate(p.y(), 4)));
                strings.push_back(QString("33\r\n%1").arg(Truncate(p.z(), 4)));
            }
            if (getLayer()->isSymmetric() && getOwner()->drawMirror()) {
                // send starboard side also
                strings.push_back(QString("0\r\n3DFACE"));
                strings.push_back(QString("8\r\n%1").arg(layername));
                strings.push_back(QString("62\r\n%1").arg(colorindex));
                for (size_t k=face->numberOfPoints(); k!=0; --k) {
                    QVector3D p = face->getPoint(k-1)->getCoordinate();
                    strings.push_back(QString("%1\r\n%2").arg(10+k).arg(Truncate(p.x(), 4)));
                    strings.push_back(QString("%1\r\n%2").arg(20+k).arg(Truncate(-p.y(), 4)));
                    strings.push_back(QString("%1\r\n%2").arg(30+k).arg(Truncate(p.z(), 4)));
                }
                if (face->numberOfPoints() == 3) {
                    QVector3D p = face->getPoint(0)->getCoordinate();
                    // 4th point is same as third
                    strings.push_back(QString("13\r\n%1").arg(Truncate(p.x(), 4)));
                    strings.push_back(QString("23\r\n%1").arg(Truncate(-p.y(), 4)));
                    strings.push_back(QString("33\r\n%1").arg(Truncate(p.z(), 4)));
                }
            }
        }
    }
}

void SubdivisionControlFace::saveToStream(QStringList &strings) const
{
    QString facestr;
    facestr.setNum(_points.size());
    for (size_t i=0; i<_points.size(); ++i) {
        SubdivisionControlPoint* point = dynamic_cast<SubdivisionControlPoint*>(_points[i]);
        facestr.append(QString(" %1").arg(_owner->indexOfControlPoint(point)));
    }
    // add layer index
    size_t index;
    if (_layer != 0)
        index = _owner->indexOfLayer(_layer);
    else
        index = 0;
    facestr.append(QString(" %1 %2").arg(index).arg(BoolToStr(isSelected())));
    strings.push_back(facestr);
}

void SubdivisionControlFace::loadFromStream(size_t &lineno, QStringList &strings)
{
    QString str = strings[lineno++].trimmed();
    size_t start = 0;
    // read control point data
    size_t n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        size_t index = ReadIntFromStr(lineno, str, start);
        SubdivisionControlPoint* p1 = _owner->getControlPoint(index);
        _points.push_back(p1);
        p1->addFace(this);
    }
    // read layer index
    size_t index = ReadIntFromStr(lineno, str, start);
    if (index < _owner->numberOfLayers()) {
        _layer = _owner->getLayer(index);
    }
    else {
        _layer = _owner->getLayer(0);   // reference to an invalid layer, assign to owners default layer
    }
    if (_layer != nullptr)
        _layer->useControlFace(this);
    else
        throw runtime_error("Invalid layer reference in SubdivisionControlFace::loadFromStream");
    if (start != to_size_t(str.length())) {
        bool sel = ReadBoolFromStr(lineno, str, start);
        if (sel)
            setSelected(true);
    }
    SubdivisionPoint* p1 = _points.back();
    for (size_t i=0; i<_points.size(); ++i) {
        SubdivisionPoint* p2 = _points[i];
        SubdivisionControlEdge* edge = _owner->controlEdgeExists(p1, p2);
        if (edge != nullptr)
            edge->addFace(this);
        else
            throw runtime_error("missing control edge in SubdivisionControlFace::loadFromStream");
        p1 = p2;
    }
}

// TODO: controledges should be reworked in subdivsurf::subdivide, we have them here in the face
// list, so we shouldn't have to collect them in the param list also
void SubdivisionControlFace::subdivide(
	vector<pair<SubdivisionPoint*,SubdivisionPoint*> >& vertexpoints,
	vector<pair<SubdivisionEdge*,SubdivisionPoint*> >& edgepoints,
	vector<pair<SubdivisionFace*,SubdivisionPoint*> >& facepoints,
	vector<SubdivisionEdge*>& controledges)
{
    _control_edges.clear();
    if (_children.size() == 0) {
        // not subdivided yet
        SubdivisionFace::subdivide(true,
                                   vertexpoints,
                                   edgepoints,
                                   facepoints,
                                   _edges,          // interior edges
                                   _control_edges,  // control edges
                                   _children);
    }
    else {
        // has been subdivided
        vector<SubdivisionFace*> tmpfaces;
        vector<SubdivisionEdge*> tmpedges;
        for (size_t i=0; i<_children.size(); ++i) {
            SubdivisionFace* face = _children[i];
            face->subdivide(false,
                            vertexpoints,
                            edgepoints,
                            facepoints,
                            tmpedges,               // interior edges
                            _control_edges,         // control edges
                            tmpfaces);
        }
        clearChildren();
        _edges = tmpedges;
        _children = tmpfaces;
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

void SubdivisionControlFace::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionControlFace ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionControlFace::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionFace::priv_dump(os, prefix);
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionControlFace& face)
{
    face.dump(os);
    return os;
}
