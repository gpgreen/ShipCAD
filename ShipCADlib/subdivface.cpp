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

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionFace* SubdivisionFace::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getFacePool().add();
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionFace::construct");
    return new (mem) SubdivisionFace(owner);
}

SubdivisionFace::SubdivisionFace(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
	// does nothing
}

size_t SubdivisionFace::indexOfPoint(SubdivisionPoint* pt)
{
    vector<SubdivisionPoint*>::iterator i = find(_points.begin(), _points.end(), pt);
    return i - _points.begin();
}

void SubdivisionFace::insertPoint(size_t index, SubdivisionPoint *point)
{
    if (index >= _points.size())
        throw range_error("bad index in SubdivisionFace::insertPoint");
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

float SubdivisionFace::getArea()
{
    float result = 0;
    for (size_t i=3; i<=_points.size(); ++i)
        result += triangle_area(_points[0]->getCoordinate(),
                _points[i-2]->getCoordinate(),
                _points[i-1]->getCoordinate());
    return result;
}

QVector3D SubdivisionFace::getFaceCenter()
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

QVector3D SubdivisionFace::getFaceNormal()
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

void SubdivisionFace::addPoint(SubdivisionPoint *point)
{
    _points.push_back(point);
    point->addFace(this);
}

SubdivisionPoint* SubdivisionFace::calculateFacePoint()
{
    SubdivisionPoint* result = 0;
    QVector3D centre = ZERO;
    if (_points.size() < 3)
        throw range_error("trying to calculate face point with less than 3 points");
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
    SubdivisionEdge* newedge = 0;
    if (p1 == 0 || p2 == 0)
        throw runtime_error("bad points in SubdivisionFace::edgeCheck");
    newedge = _owner->edgeExists(p1, p2);
    if (newedge == 0) {
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

// predicate class to find an element with given point
struct PointPred {
    ShipCAD::SubdivisionPoint* _querypt;
    bool operator()(const pair<ShipCAD::SubdivisionPoint*, ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _querypt;
    }
    PointPred (ShipCAD::SubdivisionPoint* querypt) : _querypt(querypt) {}
};

// predicate class to find an element with given face
struct FacePred {
    ShipCAD::SubdivisionFace* _queryface;
    bool operator()(const pair<ShipCAD::SubdivisionFace*, ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _queryface;
    }
    FacePred (ShipCAD::SubdivisionFace* queryface) : _queryface(queryface) {}
};

// predicate class to find an element with given edge
struct EdgePred {
    ShipCAD::SubdivisionEdge* _queryedge;
    bool operator()(const pair<ShipCAD::SubdivisionEdge*, ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _queryedge;
    }
    EdgePred (ShipCAD::SubdivisionEdge* queryedge) : _queryedge(queryedge) {}
};

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
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(prevedge));
            pts[index] = (*etmpindex).second;
            index = 1;
            ptmpindex = find_if(vertexpoints.begin(), vertexpoints.end(), PointPred(p2));
            pts[index] = (*ptmpindex).second;
            index = 2;
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(curedge));
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
        for (size_t i=1; i<=numberOfPoints(); ++i) {
            p2 = _points[i-1];
            size_t index = (i - 2 + numberOfPoints()) % numberOfPoints();
            prevedge = _owner->edgeExists(p2, _points[index]);
            etmpindex = find_if(edgepoints.begin(), edgepoints.end(), EdgePred(prevedge));
            pts[i-1] = (*etmpindex).second;
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
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionControlFace::construct");
    return new (mem) SubdivisionControlFace(owner);
}

SubdivisionControlFace::SubdivisionControlFace(SubdivisionSurface *owner)
    : SubdivisionFace(owner), _layer(0), _min(ZERO), _max(ZERO)
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

void SubdivisionControlFace::addFaceToDL(QVector<QVector3D>& vertices, QVector<QVector3D>& normals, QVector3D& p1, QVector3D& p2, QVector3D& p3)
{
    QVector3D n = QVector3D::normal(p2 - p1, p3 - p1);
    vertices << p1;
    vertices << p2;
    vertices << p3;
    normals << n;
    normals << n;
    normals << n;
}

void SubdivisionControlFace::drawFaces(Viewport &vp, FaceShader* monoshader)
{
    // make the vertex and color buffers
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> vertices_underwater;
    QVector<QVector3D> normals_underwater;

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
                        addFaceToDL(vertices_underwater, normals_underwater, p1, p2, p3);
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
    }
    // render above the waterline
    monoshader->renderMesh(getColor(), vertices, normals);
    // render below the waterline
    monoshader->renderMesh(_owner->getUnderWaterColor(), vertices_underwater, normals_underwater);
}

void SubdivisionControlFace::drawCurvatureFaces(Viewport &/*vp*/, float /*MinCurvature*/, float /*MaxCurvature*/)
{
    // TODO
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
        // TODO showing normals not implemented
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

void SubdivisionControlFace::saveToDXF(QStringList& strings)
{
    QString layername = getLayer()->getName();
    int colorindex = FindDXFColorIndex(getLayer()->getColor());
    if (numberOfPoints() == 4) {
        // create one polymesh for all childfaces
        ControlFaceGrid facedata;
        facedata.setRows(1);
        facedata.setCols(1);
        facedata.setFace(0, 0, this);
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
                    strings.push_back(QString("10\r\n%1").arg(truncate(p.x(), 4)));
                    strings.push_back(QString("20\r\n%1").arg(truncate(p.y(), 4)));
                    strings.push_back(QString("30\r\n%1").arg(truncate(p.z(), 4)));
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
                        strings.push_back(QString("10\r\n%1").arg(truncate(p.x(), 4)));
                        strings.push_back(QString("20\r\n%1").arg(truncate(-p.y(), 4)));
                        strings.push_back(QString("30\r\n%1").arg(truncate(p.z(), 4)));
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
                strings.push_back(QString("%1\r\n%2").arg(10+k).arg(truncate(p.x(), 4)));
                strings.push_back(QString("%1\r\n%2").arg(20+k).arg(truncate(p.y(), 4)));
                strings.push_back(QString("%1\r\n%2").arg(30+k).arg(truncate(p.z(), 4)));
            }
            if (face->numberOfPoints() == 3) {
                QVector3D p = face->getPoint(2)->getCoordinate();
                // 4th point is same as third
                strings.push_back(QString("13\r\n%1").arg(truncate(p.x(), 4)));
                strings.push_back(QString("23\r\n%1").arg(truncate(p.y(), 4)));
                strings.push_back(QString("33\r\n%1").arg(truncate(p.z(), 4)));
            }
            if (getLayer()->isSymmetric() && getOwner()->drawMirror()) {
                // send starboard side also
                strings.push_back(QString("0\r\n3DFACE"));
                strings.push_back(QString("8\r\n%1").arg(layername));
                strings.push_back(QString("62\r\n%1").arg(colorindex));
                for (size_t k=face->numberOfPoints(); k!=0; --k) {
                    QVector3D p = face->getPoint(k-1)->getCoordinate();
                    strings.push_back(QString("%1\r\n%2").arg(10+k).arg(truncate(p.x(), 4)));
                    strings.push_back(QString("%1\r\n%2").arg(20+k).arg(truncate(-p.y(), 4)));
                    strings.push_back(QString("%1\r\n%2").arg(30+k).arg(truncate(p.z(), 4)));
                }
                if (face->numberOfPoints() == 3) {
                    QVector3D p = face->getPoint(0)->getCoordinate();
                    // 4th point is same as third
                    strings.push_back(QString("13\r\n%1").arg(truncate(p.x(), 4)));
                    strings.push_back(QString("23\r\n%1").arg(truncate(-p.y(), 4)));
                    strings.push_back(QString("33\r\n%1").arg(truncate(p.z(), 4)));
                }
            }
        }
    }
}

void SubdivisionControlFace::saveToStream(QStringList &strings)
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
    if (_layer != 0)
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
        if (edge != 0)
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
