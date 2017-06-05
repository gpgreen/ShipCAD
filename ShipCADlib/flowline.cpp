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

#include "flowline.h"
#include "shipcadmodel.h"
#include "filebuffer.h"
#include "viewport.h"
#include "shader.h"
#include "plane.h"
#include "subdivlayer.h"
#include "subdivface.h"
#include "subdivpoint.h"
#include "utility.h"

using namespace ShipCAD;
using namespace std;

Flowline::Flowline(ShipCADModel* owner)
    : _owner(owner), _projection_point(ZERO2), _projection_vw(fvProfile), _method_new(false)
{
    // does nothing
}

Flowline* Flowline::construct(ShipCADModel* owner)
{
    Flowline* f = new Flowline(owner);
    return f;
}

void Flowline::clear()
{
    _projection_point = ZERO2;
    _projection_vw = fvProfile;
    _method_new = false;
    Spline::clear();
}

void Flowline::initialize(QVector2D pt, viewport_type_t ty)
{
    _projection_point = pt;
    _projection_vw = ty;
    setBuild(false);
}

// FreeShipUnit.pas:3573
void Flowline::draw(Viewport& vp, LineShader* lineshader)
{
    if (!isBuild())
        rebuild();
    setColor(getColor());
    setFragments(600);
    if (numberOfPoints() > 0 && vp.getViewportMode() == vmWireFrame)
    {
        // draw flowline source
        QVector3D p3d(getPoint(0));
        if (vp.getViewportType() == fvBodyplan
            && _owner->getVisibility().getModelView() != mvBoth
            && p3d.x() < _owner->getProjectSettings().getMainframeLocation())
            p3d.setY(-p3d.y());
        // draw entire circle in white
        // draw an ellipse
        if (_owner->getVisibility().getModelView() == mvBoth) {
            p3d.setY(-p3d.y());
            // draw entire circle in white
            // draw an ellipse
        }
    }

    QVector<QVector3D>& vertices = lineshader->getVertexBuffer();
    
    if (vp.getViewportType() == fvBodyplan && _owner->getVisibility().getModelView() != mvBoth) {
        Plane pln(1.0, 0.0, 0.0, -_owner->getProjectSettings().getMainframeLocation());
        vector<float> params;
        params.push_back(0.0);
        params.push_back(1.0);
        
        IntersectionData output;
        if (intersect_plane(pln, output)) {
            params.insert(params.end(), output.parameters.begin(), output.parameters.end());
            sort(params.begin(), params.end());
        }

        for (size_t i=1; i<params.size(); ++i) {
            QVector3D p3d = value(0.5 * params[i-1] + params[i]);
            float scale = 1;
            if (p3d.x() < _owner->getProjectSettings().getMainframeLocation())
                scale = -1;
            size_t fragm = floor((params[i] - params[i-1])*getFragments());
            if (fragm < 10)
                fragm = 10;
            p3d = value(0);
            p3d.setY(p3d.y() * scale);
            for (size_t j=1; j<fragm; ++j) {
                float t = params[i-1] + (params[i] - params[i-1]) * j / (fragm - 1);
                QVector3D p3d1 = value(t);
                p3d1.setY(p3d.y() * scale);
                vertices << p3d << p3d1;
                p3d = p3d1;
            }
            glLineWidth(1);
            lineshader->renderLines(vertices, getColor());
        }
    } else {
        Spline::draw(vp, lineshader);
        if (_owner->getVisibility().getModelView() == mvBoth)
            Spline::drawStarboard(vp, lineshader);
    }
}

void Flowline::setBuild(bool val)
{
    Spline::setBuild(val);
}

//< used in Flowline::rebuild
static QVector3D CalculateFlowDirection2(QVector3D incoming, SubdivisionPoint* point)
{
    incoming.normalize();
    QVector3D normal = point->getNormal();
    QVector3D p = point->getCoordinate();
    Plane plane(p, normal);
    QVector3D direction = normal + incoming;
    direction.normalize();
    incoming = p + direction;
    QVector3D proj = plane.projectPointOnPlane(incoming);
    direction = proj - p;
    direction.normalize();
    return direction;
}

//< used in Flowline::rebuild
static QVector3D CalculateFlowDirection(SubdivisionPoint* point)
{
    QVector3D v(-1, 0, 0);
    return CalculateFlowDirection2(v, point);
}


//< used in Flowline::rebuild
struct PointData 
{
    QVector3D coord;
    QVector3D flowdir;
    vector<size_t> triangles;
};

//< used in Flowline::rebuild
struct Triangle 
{
    map<SubdivisionPoint*, PointData>::iterator p1;
    map<SubdivisionPoint*, PointData>::iterator p2;
    map<SubdivisionPoint*, PointData>::iterator p3;
    Plane plane;
    size_t index;
    bool processed;
};

//< used in Flowline::rebuild
static void AddTriangle(vector<Triangle>& triangles,
                        map<SubdivisionPoint*, PointData>& pointdata,
                        SubdivisionPoint* p1,
                        SubdivisionPoint* p2, SubdivisionPoint* p3)
{
    Triangle t;
    t.index = triangles.size();
    t.processed = false;
    t.p1 = pointdata.find(p1);
    t.p1->second.triangles.push_back(t.index);
    t.p2 = pointdata.find(p2);
    t.p2->second.triangles.push_back(t.index);
    t.p3 = pointdata.find(p3);
    t.p3->second.triangles.push_back(t.index);
    t.plane = Plane(p1->getCoordinate(), p2->getCoordinate(), p3->getCoordinate());
    triangles.push_back(t);
}

//< used in Flowline::rebuild
static size_t FindInitialTriangle(bool method_new,
                                  vector<Triangle>& triangles,
                                  QVector3D startpoint, QVector3D endpoint,
                                  QVector3D& intersection, QVector3D& direction)
{
    size_t result = triangles.size();
    double distance = 1E8;
    intersection = ZERO;
    direction = ZERO;
    for (size_t i=0; i<triangles.size(); ++i) {
        Triangle& triangle = triangles[i];
        float s1 = triangle.plane.distance(startpoint);
        float s2 = triangle.plane.distance(endpoint);
        if ((s1 < 0 && s2 > 0) || (s1 > 0 && s2 < 0)) {
            // possible intersection
            float t;
            if (s1 == s2)
                t = 0.5;
            else
                t = -s1 / (s2 - s1);
            QVector3D p = startpoint + t * (endpoint - startpoint);
            if (PointInTriangle(p, triangle.p1->second.coord,
                                triangle.p2->second.coord,
                                triangle.p3->second.coord)) {
                t = startpoint.distanceToPoint(p);
                if (t < distance) {
                    distance = t;
                    result = i;
                    intersection = p;
                }
            }
        }
    }
    if (result != triangles.size()) {
        // Calculate baycentric coordinates to interpolate between the three flowdirections
        // http://softsurfer.com/Archive/algorithm_0104/algorithm_0104.htm
        Triangle& triangle = triangles[result];
        QVector3D p0 = triangle.p1->second.coord;
        QVector3D p1 = triangle.p2->second.coord;
        QVector3D p2 = triangle.p3->second.coord;
        QVector3D u = p1 - p0;
        QVector3D v = p2 - p0;
        QVector3D w = intersection - p0;
        double udotu = QVector3D::dotProduct(u, u);
        double udotv = QVector3D::dotProduct(u, v);
        double vdotv = QVector3D::dotProduct(v, v);
        double wdotu = QVector3D::dotProduct(w, u);
        double wdotv = QVector3D::dotProduct(w, v);
        double s = (udotv*wdotv-vdotv*wdotu)/(udotv*udotv-udotu*vdotv);
        double t = (udotv*wdotu-udotu*wdotv)/(udotv*udotv-udotu*vdotv);
        double b0 = 1 - s - t;
        double b1 = s;
        double b2 = t;
        // check
        t = b0 + b1 + b2;
        if (t == 1) {
            p0 = triangle.p1->second.flowdir;
            p1 = triangle.p2->second.flowdir;
            p2 = triangle.p3->second.flowdir;
            direction = b0 * p0 + b1 * p1 + b2 * p2;

            if (method_new) {
                direction = QVector3D(-1, 0.1, -0.1);
                direction.normalize();
            }
        }
    }
    return result;
}

static size_t NextTriangleIndex(map<SubdivisionPoint*, PointData>::iterator& ind1,
                                map<SubdivisionPoint*, PointData>::iterator& ind2,
                                size_t currindex, size_t maxindex)
{
    size_t result = maxindex;
    PointData& point1 = ind1->second;
    PointData& point2 = ind2->second;
    for (size_t i=0; i<point1.triangles.size(); ++i) {
        for (size_t j=0; j<point2.triangles.size(); ++j) {
            if (point1.triangles[i] == point2.triangles[j] &&
                point1.triangles[i] != currindex) {
                result = point1.triangles[i];
                return result;
            }
        }
    }
    return result;
}

// used in rebuild
// returns true if triangle is valid
// nextindex will be set to the next triangle to process, or to maxindex if no more to process
// skipind1 and skipind2 are set to the points to ignore for intersections, will be updated
// in this method with the new points to ignore if valid
static bool ProcessTriangle(bool method_new, Triangle& triangle,
                            map<SubdivisionPoint*, PointData>::iterator& skipind1,
                            map<SubdivisionPoint*, PointData>::iterator& skipind2,
                            QVector3D& intersection,
                            QVector3D& direction,
                            size_t& nextindex, size_t maxindex)
{
    bool result = false;
    nextindex = triangle.index;
    triangle.processed = true;
    QVector3D p1 = triangle.plane.projectPointOnPlane(intersection);
    p1 = p1 + 0.0005 * direction;
    if (!PointInTriangle(p1, triangle.p1->second.coord, triangle.p2->second.coord,
                         triangle.p3->second.coord)) {
        p1 = triangle.plane.projectPointOnPlane(intersection);
    }

    p1 = triangle.plane.projectPointOnPlane(p1);
    double distance = 50;
    map<SubdivisionPoint*, PointData>::iterator ind1;
    map<SubdivisionPoint*, PointData>::iterator ind2;
    QVector3D p2 = p1 + distance * direction;
    // test all three linesegments for intersection
    for (size_t i=0; i<3; ++i) {
        if (i == 0) {
            ind1 = triangle.p1;
            ind2 = triangle.p2;
        } else if (i == 1) {
            ind1 = triangle.p2;
            ind2 = triangle.p3;
        } else {
            ind1 = triangle.p3;
            ind2 = triangle.p1;
        }
        QVector3D int1;
        double param;
        if ((ind1 == skipind1 && ind2 == skipind2) || (ind1 == skipind2 && ind2 == skipind1)) {
            // does nothing
        } else if (Lines3DIntersect(p1, p2, ind1->second.coord, ind2->second.coord, param, int1)) {
            distance = triangle.plane.distance(int1);
            if (distance < 1E-1) {
                intersection = int1;
                QVector3D dir1;
                QVector3D dir2;
                // calculate direction
                if (method_new) {
                    dir1 = CalculateFlowDirection2(direction, ind1->first);
                    dir2 = CalculateFlowDirection2(direction, ind2->first);
                } else {
                    dir1 = ind1->second.flowdir;
                    dir2 = ind2->second.flowdir;
                }
                skipind1 = ind1;
                skipind2 = ind2;
                direction = dir1 + param * (dir2 - dir1);
                nextindex = NextTriangleIndex(ind1, ind2, triangle.index, maxindex);
                result = true;
                break;
            }
        }
    }
    
    return result;
}

// FreeShipUnit.pas:3700
void Flowline::rebuild()
{
    // clear spline data
    Spline::clear();
    
    // assemble all faces that are (partially) submerged and extract points
    float wlheight = _owner->findLowestHydrostaticsPoint() + _owner->getProjectSettings().getDraft();

    SubdivisionSurface* surf = _owner->getSurface();
    if (surf->numberOfPoints() == 0)
        return;
    vector<SubdivisionFace*> faces;
    for (size_t i=0; i<surf->numberOfLayers(); ++i) {
        if (!surf->getLayer(i)->useInHydrostatics())
            continue;
        SubdivisionLayer* layer = surf->getLayer(i);
        for (size_t j=0; j<layer->numberOfFaces(); ++j) {
            if (layer->getFace(j)->getMin().z() > wlheight)
                continue;
            SubdivisionControlFace* face = layer->getFace(j);
            for (size_t k=0; k<face->numberOfChildren(); ++k) {
                SubdivisionFace* child = face->getChild(k);
                for (size_t l=0; l<child->numberOfPoints(); ++l) {
                    if (child->getPoint(l)->getCoordinate().z() <= wlheight) {
                        // face is (partially) submerged
                        faces.push_back(child);
                        break;
                    }
                }
            }
        }
    }
    if (faces.size() == 0) {
        setBuild(true);
        return;
    }
    set<SubdivisionPoint*> points;
    for (size_t i=0; i<faces.size(); ++i) {
        SubdivisionFace* child = faces[i];
        for (size_t j=0; j<child->numberOfPoints(); ++j) {
            SubdivisionPoint* point = child->getPoint(j);
            points.insert(point);
        }
    }
    map<SubdivisionPoint*, PointData> pointdata;
    for (set<SubdivisionPoint*>::iterator i=points.begin(); i!=points.end(); i++) {
        PointData pd;
        pd.coord = (*i)->getCoordinate();
        pd.flowdir = CalculateFlowDirection(*i);
        pointdata.insert(make_pair(*i, pd));
    }
    vector<Triangle> triangles;
    for (size_t i=0; i<faces.size(); ++i) {
        SubdivisionFace* child = faces[i];
        for (size_t j=2; j<child->numberOfPoints(); ++j)
            AddTriangle(triangles, pointdata, child->getPoint(0), child->getPoint(j-1), child->getPoint(j));
    }
    QVector3D startpoint;
    QVector3D endpoint;
    switch(_projection_vw) {
    case fvProfile:
        startpoint.setX(_projection_point.x());
        startpoint.setY(_owner->getSurface()->getMax().y() + 10);
        startpoint.setZ(_projection_point.y());
        endpoint = QVector3D(startpoint.x(), 0, startpoint.z());
        break;
    case fvPlan:
        startpoint.setX(_projection_point.x());
        startpoint.setY(_projection_point.y());
        startpoint.setZ(_owner->getSurface()->getMin().z() - 10);
        endpoint = QVector3D(startpoint.x(), startpoint.z(),
                             _owner->getSurface()->getMax().z() + 100);
        break;
    case fvBodyplan:
        if (_projection_point.x() < 0) {
            startpoint.setX(_owner->getSurface()->getMin().x() - 10);
            startpoint.setY(-_projection_point.x());
            startpoint.setZ(_projection_point.y());
            endpoint = QVector3D(_owner->getSurface()->getMax().x() + 10,
                                 startpoint.y(), startpoint.z());
        } else {
            startpoint.setX(_owner->getSurface()->getMax().x() + 10);
            startpoint.setY(_projection_point.x());
            startpoint.setZ(_projection_point.y());
            endpoint = QVector3D(_owner->getSurface()->getMin().x() - 10,
                                 startpoint.y(), startpoint.z());
        }
        break;
    default:
        setBuild(true);
        return;
    };

    // find the initial triangle
    QVector3D intersection;
    QVector3D direction;
    size_t index = FindInitialTriangle(_method_new, triangles,
                                       startpoint, endpoint, intersection, direction);
    map<SubdivisionPoint*, PointData>::iterator skipind1 = pointdata.end();
    map<SubdivisionPoint*, PointData>::iterator skipind2 = pointdata.end();
    if (index != triangles.size()) {
        add(intersection);
        // trace triangles from here
        size_t iteration = 0;
        bool valid;
        do {
            if (triangles[index].processed)
                valid = false;
            else
                valid = ProcessTriangle(_method_new, triangles[index], skipind1, skipind2,
                                        intersection, direction, index, triangles.size());
            if (valid) {
                add(intersection);
            }
            
            else {
                valid = ProcessTriangle(_method_new, triangles[index], skipind1, skipind2,
                                        intersection, direction, index, triangles.size());
            }
            ++iteration;
        } while (valid && index != triangles.size() && iteration < 5000);
        while (numberOfPoints() > 1) {
            QVector3D last = getPoint(numberOfPoints() - 1);
            QVector3D nexttolast = getPoint(numberOfPoints() - 2);
            if (last.z() > wlheight && nexttolast.z() > wlheight) {
                delete_point(numberOfPoints() - 1);
            } else if (last.z() > wlheight && nexttolast.z() < wlheight) {
                QVector3D newendpoint(nexttolast.x() + ((last.x() - nexttolast.x())
                                   *(wlheight - nexttolast.z()) / (last.z() - nexttolast.z())),
                                   nexttolast.y() + ((last.y() - nexttolast.y())
                                   *(wlheight - nexttolast.z()) / (last.z() - nexttolast.z())),
                                   wlheight);
                setPoint(numberOfPoints() - 1, newendpoint);
            } else
                break;
        }
    }
    Spline::rebuild();
}

bool Flowline::isVisible() const
{
    return _owner->getVisibility().isShowFlowlines();
}

bool Flowline::isSelected() const
{
    return _owner->isSelectedFlowline(const_cast<Flowline*>(this));
}

void Flowline::setSelected(bool set)
{
    if (set)
        _owner->setSelectedFlowline(this);
    else
        _owner->removeSelectedFlowline(this);
}

QColor Flowline::getColor() const
{
    if (isSelected())
        return _owner->getPreferences().getSelectColor();
    else if (_method_new)
        return Qt::red;
    return Qt::blue;
}

void Flowline::loadBinary(FileBuffer& source)
{
    quint32 n;
    float f;
    bool b;
    QVector3D p;
    source.load(f);
    _projection_point.setX(f);
    source.load(f);
    _projection_point.setY(f);
    source.load(n);
    _projection_vw = static_cast<viewport_type_t>(n);
    source.load(_build);
    source.load(b);
    if (b)
        _owner->setSelectedFlowline(this);
    source.load(n);
    for (size_t i=0; i<n; i++) {
        source.load(p);
        add(p);
        source.load(b);
        setKnuckle(i, b);
    }
}

void Flowline::saveBinary(FileBuffer& dest)
{
    dest.add(_projection_point.x());
    dest.add(_projection_point.y());
    dest.add(static_cast<quint32>(_projection_vw));
    dest.add(_build);
    dest.add(isSelected());
    dest.add(numberOfPoints());
    for (size_t i=0; i<numberOfPoints(); i++) {
        dest.add(getPoint(i));
        dest.add(isKnuckle(i));
    }
}

