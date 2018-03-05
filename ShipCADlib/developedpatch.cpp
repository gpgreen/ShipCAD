/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>                                   *
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
#include <cmath>

#include "developedpatch.h"
#include "plane.h"
#include "subdivlayer.h"
#include "subdivface.h"
#include "subdivpoint.h"
#include "subdivsurface.h"
#include "subdivedge.h"
#include "viewport.h"
#include "shader.h"
#include "utility.h"
#include "shipcadlib.h"
#include "drawfaces.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

DevelopedPatch::DevelopedPatch(SubdivisionLayer* layer)
	: _owner(layer), _stations(true), _waterlines(true), _buttocks(true),
      _diagonals(true)
{
	clear();
}

void DevelopedPatch::clear()
{
    _connectedMirror = 0;
    _xgrid = 1.0;
    _ygrid = 1.0;
    _numIterations = 0;
    _name = "";
    _points.clear();
    _edges.clear();
    _donelist.clear();
    _corners.clear();
    _boundaryEdges.clear();
    _edgeErrors.clear();
    setRotation(0.0);
    _mirrorPlane = Plane();
    _min2D = ZERO2;
    _max2D = ZERO2;
    _translation = ZERO2;
    _showSolid = true;
    _showBoundingBox = false;
    _showInteriorEdges = true;
    _showStations = true;
    _showButtocks = true;
    _showWaterlines = true;
    _showDiagonals = true;
    _showErrorEdges = true;
    _showDimensions = true;
    _showPartName = true;
    _shadeSubmerged = true;
    _visible = true;
    _mirrorOnScreen = false;
    _mirror = false;

    // these are 'owned' vectors, don't need to delete individual splines
    _stations.clear();
    _buttocks.clear();
    _waterlines.clear();
    _diagonals.clear();
    
    _maxAreaError = 0.0;
    _totalAreaError = 0.0;
    // these are set in setRotation
    //_cos = 0.0;
    //_sin = 0.0;
    _vertices1 = 0;
    _vertices2 = 0;
}

void DevelopedPatch::extents(QVector3D& min, QVector3D& max)
{
    if (_points.size() == 0) {
        min.setX(-1.0);
        min.setY(-1.0);
        min.setZ(0.0);
        max.setX(1.0);
        max.setY(1.0);
        max.setZ(0.0);
    } else {
        for (size_t i=0; i<_points.size(); i++) {
            QVector3D p = getPoint(i);
            if (i == 0) {
                min = p;
                max = p;
            }
            MinMax(p, min, max);
            if (_mirror) {
                p = getMirrorPoint(i);
                MinMax(p, min, max);
            }
        }
    }
}

double DevelopedPatch::maxError()
{
    double err = 0.0;
    for (size_t i=0; i<_edgeErrors.size(); i++)
        if (_edgeErrors[i] > err)
            err = _edgeErrors[i];
    return err;
}

double DevelopedPatch::minError()
{
    double err = 0.0;
    for (size_t i=0; i<_edgeErrors.size(); i++)
        if (_edgeErrors[i] < err)
            err = _edgeErrors[i];
    return err;
}

void DevelopedPatch::setRotation(float val)
{
    _rotation = val;
    _cos = cos(DegToRad(_rotation));
    _sin = sin(DegToRad(_rotation));
}

void DevelopedPatch::setMirrorOnScreen(bool set)
{
    if (set != _mirrorOnScreen && !_mirror) {
        _mirrorOnScreen = set;
        float val = _min2D.y();
        _min2D.setY(-_max2D.y());
        _max2D.setY(-val);
        setRotation(-_rotation);
    }
}

QVector2D DevelopedPatch::midPoint()
{
    QVector2D p = 0.5 * _min2D + _max2D;
    return p;
}

QVector3D DevelopedPatch::getPoint(size_t index)
{
    QVector2D p = _points[index].pt2D;
    if (_mirrorOnScreen && !_mirror)
        p.setY(-p.y());
    return convertTo3D(p);
}

QVector3D DevelopedPatch::getMirrorPoint(size_t index)
{
    QVector2D p = _points[index].pt2D;
    QVector3D tmp(p.x(), p.y(), 0.0);
    tmp = _mirrorPlane.mirror(tmp);
    p.setX(tmp.x());
    p.setY(tmp.y());
    if (_mirrorOnScreen && !_mirror)
        p.setY(-p.y());
    return convertTo3D(p);
}

// FreeGeometry.pas:5568
void DevelopedPatch::drawSpline(LineShader* lineshader, Spline& spline)
{
    QVector<QVector3D>& vertices = lineshader->getVertexBuffer();
    for (size_t i=0; i<spline.getFragments(); ++i) {
        QVector3D p3d = spline.value(i/static_cast<float>(spline.getFragments()));
        QVector2D p2d(p3d.x(), p3d.y());
        if (mirrorOnScreen() && !isMirror())
            p2d.setY(-p2d.y());
        p3d = convertTo3D(p2d);
        vertices << p3d;
    }
    glLineWidth(1);
    lineshader->renderPoints(vertices, spline.getColor());
}

// FreeGeometry.pas:5589
void DevelopedPatch::setFontHeight(Viewport& vp, float desired_height)
{
    // set the fontheight to a height in modelspace
    //float height = desired_height * vp.scale() * vp.zoom();
    // TODO: set font height size of 8, then get height of that
}

// FreeGeometry.pas:5613
void DevelopedPatch::drawDimension(Viewport& /*vp*/, LineShader* /*lineshader*/,
                                   QVector3D p1, QVector3D p2)
{
    // TODO
}

void DevelopedPatch::draw(Viewport& /*vp*/, FaceShader* faceshader)
{
    if (showSolid() || showInteriorEdges()) {
        DrawFaces(faceshader, _owner->getOwner()->getWaterlinePlane(),
                  _donelist, _vertices1, _vertices2,
                  isShadeSubmerged(), isMirror(),
                  _owner->getColor(), _owner->getOwner()->getUnderWaterColor());
                  
    }
}

// FreeGeometry.pas:5548
void DevelopedPatch::draw(Viewport& /*vp*/, LineShader* lineshader)
{
    // TODO
    QVector<QVector3D>& redvertices = lineshader->getVertexBuffer();
    
    if (showSolid() || showInteriorEdges()) {
        // draw edges of faces
    } else if (!showInteriorEdges()) {
        // draw only boundary edges
        for (size_t i=0; i<_boundary_edges.size(); ++i) {
            SubdivisionEdge* edge = _boundaryEdges[i];
            SubdivisionPoint* s = edge->startPoint();
            SubdivisionPoint* e = edge->endPoint();
            patchpt_iter index1 = find_point(_points.begin(),
                                             _points.end(), s);
            patchpt_iter index2 = find_point(_points.begin(),
                                             _points.end(), e);
            if (index1 != _points.end() && index2 != _points.end()) {
                vertices << getPoint(index1 - _points.begin());
                vertices << getPoint(index2 - _points.begin());
                if (isMirror()) {
                    vertices << getMirrorPoint(index1 - _points.begin());
                    vertices << getMirrorPoint(index2 - _points.begin());
                }
            }
        }
        lineshader->renderLines(vertices, edgeColor);
    }
    if (showDimensions()) {
    }
    // show edges with errors
    if (showErrorEdges()) {
        vertices.clear();
        glLineWidth(2);
        // blue lines
        QVector<QVector3D> bluevertices;
        QVector<QVector3D>& which = redvertices;
        // draw blue lines
        for (size_t i=0; i<_edges.size(); ++i) {
            if (fabs(_edgeErrors[i]) > 1E-4) {
                if (_edgeErrors[i] > 0)
                    which = redvertices;
                else
                    which = bluevertices;
                SubdivisionEdge* edge = _edges[i];
                SubdivisionPoint* s = edge->startPoint();
                SubdivisionPoint* e = edge->endPoint();
                patchpt_iter index1 = find_point(_points.begin(),
                                                 _points.end(), s);
                patchpt_iter index2 = find_point(_points.begin(),
                                                 _points.end(), e);
                if (index1 != _points.end() && index2 != _points.end()) {
                    which << getPoint(index1 - _points.begin());
                    which << getPoint(index2 - _points.begin());
                    if (isMirror()) {
                        which << getMirrorPoint(index1 - _points.begin());
                        which << getMirrorPoint(index2 - _points.begin());
                    }
                }
            }
        }
        lineshader->renderLines(redvertices, Qt::red);
        lineshader->renderLines(bluevertices, Qt::blue);
    }
    if (showStations()) {
    }
    if (showButtocks()) {
    }
    if (showWaterlines()) {
    }
    if (showDiagonals()) {
    }
    if (showPartName()) {
    }
    if (showBoundingBox()) {
    }
}

struct PatchIntersection 
{
    QVector3D point;
    bool knuckle;
};

// predicate class to find SubdivisionPoint in PatchPoints vector
struct PatchPointPred
{
    ShipCAD::SubdivisionPoint* querypt;
    bool operator()(const ShipCAD::PatchPoints& val) {
        return (val.pt == querypt);
    }
    explicit PatchPointPred (ShipCAD::SubdivisionPoint* pt) : querypt(pt) {}
};

// function to find a point in a vector
patchpt_iter find_point(patchpt_iter start, patchpt_iter end, SubdivisionPoint* pt)
{
    PatchPointPred pred1(pt);
    return find_if(start, end, pred1);
}

// FreeGeometry.pas:5952
void DevelopedPatch::intersectPlane(Plane& plane, QColor color)
{
    SplineVector dest(false);
    for (size_t j=0; j<_donelist.size(); j++) {
        SubdivisionFace* face = _donelist[j];
        vector<PatchIntersection> intarray;
        SubdivisionPoint* p1 = face->getPoint(face->numberOfPoints()-1);
        patchpt_iter index1 = find_point(_points.begin(), _points.end(), p1);
        patchpt_iter index2 = _points.begin();
        float side1 = plane.distance(p1->getCoordinate());
        for (size_t k=0; k<face->numberOfPoints(); k++) {
            SubdivisionPoint* p2 = face->getPoint(k);
            float side2 = plane.distance(p2->getCoordinate());
            if ((side1 < -1e-5 && side2 > 1e-5) || (side1 > 1e-5 && side2 < -1e-5)) {
                // regular intersection of edge
                // add the edge to the list
                float parameter = -side1 / (side2 - side1);
                QVector2D pt1 = (*index1).pt2D;
                index2 = find_point(_points.begin(), _points.end(), p2);
                QVector2D tmp = pt1 + parameter * ((*index2).pt2D - pt1);
                QVector3D output(tmp.x(), tmp.y(), 0.0);
                PatchIntersection intsect;
                intsect.point = output;
                SubdivisionEdge* edge = getOwner()->getOwner()->edgeExists(p1, p2);
                if (edge != 0)
                    intsect.knuckle = edge->isCrease();
                else
                    intsect.knuckle = false;
                intarray.push_back(intsect);
            } else {
                // does the edge lie entirely with the plane?
                if (fabs(side1) <= 1e-5 && fabs(side2) <= 1e-5) {
                } else if (fabs(side2) < 1e-5) {
                    PatchIntersection intsect;
                    index2 = find_point(_points.begin(), _points.end(), p2);
                    QVector2D pt = (*index2).pt2D;
                    intsect.point.setX(pt.x());
                    intsect.point.setY(pt.y());
                    intsect.point.setZ(0.0);
                    intsect.knuckle = p2->getVertexType() != svRegular;
                    intarray.push_back(intsect);
                }
            }
            p1 = p2;
            side1 = side2;
            index1 = index2;
        }
        if (intarray.size() > 1) {
            if (DistPP3D(intarray[0].point, intarray.back().point) < 1e-4)
                intarray.pop_back();
            if (intarray.size() > 1) {
                Spline* spline = new Spline();
                spline->setColor(color);
                for (size_t k=0; k<intarray.size(); k++) {
                    spline->add(intarray[k].point);
                    spline->setKnuckle(spline->numberOfPoints()-1, intarray[k].knuckle);
                }
                dest.add(spline);
            }
        }
    }
    if (dest.size() > 1) {
        JoinSplineSegments(0.01, false, dest);
        for (size_t i=dest.size(); i>=1; i--) {
            Spline* spline = dest.get(i-1);
            if (spline->numberOfPoints() > 1) {
                float parameter = SquaredDistPP(spline->getMin(), spline->getMax());
                if (parameter < 1e-3) {
                    dest.del(spline);
                    delete spline;
                }
            }
        }
    }
    for (size_t i=0; i<dest.size(); i++) {
        Spline* spline = dest.get(i);
        if (fabs(plane.a()) > 0.9999)
            _stations.add(spline);
        else if (fabs(plane.b()) > 0.9999)
            _buttocks.add(spline);
        else if (fabs(plane.c()) > 0.9999)
            _waterlines.add(spline);
        else if (fabs(plane.b()) > 0.5 && fabs(plane.c()) > 0.5)
            _diagonals.add(spline);
        if (_mirror) {
            Spline* copy(spline);
            for (size_t j=0; j<copy->numberOfPoints(); j++) {
                copy->setPoint(j, _mirrorPlane.mirror(copy->getPoint(j)));
            }
            if (fabs(plane.a()) > 0.9999)
                _stations.add(copy);
            else if (fabs(plane.b()) > 0.9999)
                _buttocks.add(copy);
            else if (fabs(plane.c()) > 0.9999)
                _waterlines.add(copy);
            else if (fabs(plane.b()) > 0.5 && fabs(plane.c()) > 0.5)
                _diagonals.add(copy);
        }
    }
}

// FreeGeometry.pas:6099
QVector3D DevelopedPatch::convertTo3D(QVector2D p)
{
    QVector2D p1(p);
    QVector2D mid = midPoint();
    // translate to origin
    QVector2D p2 = p - mid;
    // rotate around origin
    p1.setX(p2.x() * _cos - p2.y() * _sin);
    p1.setY(p2.x() * _sin + p2.y() * _cos);
    // translate back again
    return QVector3D(p1.x() + mid.x() + _translation.x(),
                     p1.y() + mid.y() + _translation.y(),
                     0.0);
}

// FreeGeometry.pas:6116
void DevelopedPatch::saveToDXF(QStringList& strings)
{
    // extract edges as polylines
    vector<SubdivisionEdge*> source(_boundaryEdges.begin(), _boundaryEdges.end());
    vector<vector<SubdivisionPoint*> > dest;
    getOwner()->getOwner()->isolateEdges(source, dest);
    int col = FindDXFColorIndex(getOwner()->getColor());
    QString layername = getOwner()->getName();
    for (size_t i=0; i<dest.size(); i++) {
        vector<SubdivisionPoint*>& src = dest[i];
        // save data as 2D polyline
        strings.push_back(QString("0\r\nPOLYLINE"));
        strings.push_back(QString("8\r\n%1").arg(layername));
        strings.push_back(QString("62\r\n%1").arg(col));
        strings.push_back(QString("66\r\n1"));
        for (size_t j=0; j<src.size(); j++) {
            patchpt_iter index = find_point(_points.begin(), _points.end(), src[j]);
            QVector3D p3d = getPoint(index - _points.begin());
            strings.push_back(QString("0\r\nVERTEX"));
            strings.push_back(QString("8\r\n%1").arg(layername));
            strings.push_back(QString("10\r\n%1").arg(Truncate(p3d.x(), 4)));
            strings.push_back(QString("20\r\n%1").arg(Truncate(p3d.y(), 4)));
        }
        strings.push_back(QString("0\r\nSEQEND"));
        if (_mirror) {
            strings.push_back(QString("0\r\nPOLYLINE"));
            strings.push_back(QString("8\r\n%1").arg(layername));
            strings.push_back(QString("62\r\n%1").arg(col));
            strings.push_back(QString("66\r\n1"));
            for (size_t j=0; j<src.size(); j++) {
                patchpt_iter index = find_point(_points.begin(), _points.end(), src[j]);
                QVector3D p3d = getMirrorPoint(index - _points.begin());
                strings.push_back(QString("0\r\nVERTEX"));
                strings.push_back(QString("8\r\n%1").arg(layername));
                strings.push_back(QString("10\r\n%1").arg(Truncate(p3d.x(), 4)));
                strings.push_back(QString("20\r\n%1").arg(Truncate(p3d.y(), 4)));
            }
            strings.push_back(QString("0\r\nSEQEND"));
        }
    }
    
    if (_showStations) {
        for (size_t i=0; i<_stations.size(); i++)
            exportSpline(strings, _stations.get(i), "stations");
    }
    if (_showButtocks) {
        for (size_t i=0; i<_buttocks.size(); i++)
            exportSpline(strings, _buttocks.get(i), "buttocks");
    }
    if (_showWaterlines) {
        for (size_t i=0; i<_waterlines.size(); i++)
            exportSpline(strings, _waterlines.get(i), "waterlines");
    }
    if (_showDiagonals) {
        for (size_t i=0; i<_diagonals.size(); i++)
            exportSpline(strings, _diagonals.get(i), "diagonals");
    }
}

void DevelopedPatch::exportSpline(QStringList& strings, Spline* spline,
                                  const QString& layername)
{
    strings.push_back(QString("0\r\nPOLYLINE"));
    strings.push_back(QString("8\r\n%1").arg(layername));
    strings.push_back(QString("62\r\n%1").arg(FindDXFColorIndex(spline->getColor())));
    strings.push_back(QString("66\r\n1"));
    for (size_t i=0; i<spline->getFragments(); i++) {
        QVector3D p3d = spline->value(i/static_cast<float>(spline->getFragments()));
        QVector2D p2d(p3d.x(), p3d.y());
        if (_mirrorOnScreen && !_mirror)
            p2d.setY(-p2d.y());
        p3d = convertTo3D(p2d);
        strings.push_back(QString("0\r\nVERTEX"));
        strings.push_back(QString("8\r\n%1").arg(layername));
        strings.push_back(QString("10\r\n%1").arg(Truncate(p3d.x(), 4)));
        strings.push_back(QString("20\r\n%1").arg(Truncate(p3d.y(), 4)));
    }
    strings.push_back(QString("0\r\nSEQEND"));
}

// FreeGeometry.pas:6208
void DevelopedPatch::saveToTextFile(QStringList& strings)
{
    // extract edges as polylines
    vector<SubdivisionEdge*> source(_boundaryEdges.begin(), _boundaryEdges.end());
    vector<vector<SubdivisionPoint*> > dest;
    getOwner()->getOwner()->isolateEdges(source, dest);

    // calculate min, max extents of boundary
    // all measurements are referred to the min coordinate
    bool first = true;
    strings.push_back("");
    strings.push_back(QString("Boundary coordinates for: %1").arg(name()));
    QVector3D min;
    QVector3D max;
    for (size_t i=0; i<dest.size(); i++) {
        vector<SubdivisionPoint*>& src = dest[i];
        for (size_t j=0; j<src.size(); j++) {
            patchpt_iter index = find_point(_points.begin(), _points.end(), src[j]);
            QVector3D p3d = getPoint(index - _points.begin());
            if (first) {
                min = p3d;
                max = min;
                first = false;
            } else
                MinMax(p3d, min, max);
            if (_mirror) {
                p3d = getMirrorPoint(index - _points.begin());
                MinMax(p3d, min, max);
            }
        }
    }

    for (size_t i=0; i<dest.size(); i++) {
        if (i > 0)
            strings.push_back("");
        vector<SubdivisionPoint*>& src = dest[i];
        for (size_t j=0; j<src.size(); j++) {
            patchpt_iter index = find_point(_points.begin(), _points.end(), src[j]);
            QVector3D p3d = getPoint(index - _points.begin());
            p3d = p3d - min;
            strings.push_back(QString("%1 %2 %3").arg(p3d.x(),7,'g',3).arg(p3d.y(),7,'g',3).arg(p3d.z(),7,'g',3));
        }
        if (_mirror) {
            strings.push_back("");
            for (size_t j=0; j<src.size(); j++) {
                patchpt_iter index = find_point(_points.begin(), _points.end(), src[j]);
                QVector3D p3d = getMirrorPoint(index - _points.begin());
                p3d = p3d - min;
                strings.push_back(QString("%1 %2 %3").arg(p3d.x(),7,'g',3).arg(p3d.y(),7,'g',3).arg(p3d.z(),7,'g',3));
            }
        }
    }
}

// FreeGeometry.pas:6286
void DevelopedPatch::unroll(std::vector<SubdivisionControlFace*> controlfaces)
{
    // first assemble all points, edges and faces used
    vector<SubdivisionFace*> faces;
    for (size_t k=0; k<controlfaces.size(); k++) {
        SubdivisionControlFace* ctrlface = controlfaces[k];
        // copy all the ctrlface child faces
        faces.insert(faces.end(), ctrlface->childrenBegin(), ctrlface->childrenEnd());
        for (size_t i=0; i<ctrlface->numberOfChildren(); i++) {
            SubdivisionFace* child = ctrlface->getChild(i);
            SubdivisionPoint* p1 = child->getPoint(child->numberOfPoints()-1);
            for (size_t j=0; j<child->numberOfPoints(); j++) {
                SubdivisionPoint* p2 = child->getPoint(j);
                // add this point
                patchpt_iter idx = find_point(_points.begin(), _points.end(), p2);
                if (idx == _points.end()) {
                    PatchPoints pp;
                    pp.pt = p2;
                    _points.push_back(pp);
                }
                SubdivisionEdge* edge = getOwner()->getOwner()->edgeExists(p1, p2);
                if (edge != 0) {
                    vector<SubdivisionEdge*>::iterator eindex = find(
                        _edges.begin(), _edges.end(), edge);
                    if (eindex == _edges.end())
                        _edges.push_back(edge);
                }
                p1 = p2;
            }
        }
    }

    // Find all seed faces, which is characterized by the face that it
    // has one cornerpoint with (possibly) multiple faces, but only 1
    // face is present in the list of faces to be unrolled.
    vector<SubdivisionFace*> seedfaces;
    for (size_t i=0; i<_points.size(); i++) {
        PatchPoints& pp = _points[i];
        int n = 0;
        for (size_t j=0; j<pp.pt->numberOfFaces(); j++) {
            subdivface_iter idx = find(faces.begin(), faces.end(), pp.pt->getFace(j));
            if (idx != faces.end())
                n++;
        }
        if (n == 1) {
            for (size_t k=0; k<pp.pt->numberOfFaces(); k++) {
                subdivface_iter idx = find(faces.begin(), faces.end(), pp.pt->getFace(k));
                if (idx != faces.end()) {
                    seedfaces.push_back(pp.pt->getFace(k));
                }
            }
        }
    }

    // if NO seedfaces could be found (which should not occur) then
    // pick a random one (the one with the largest area)
    if (seedfaces.size() == 0) {
        double area;
        double seedarea = 0.0;
        SubdivisionFace* seedface = 0;
        for (size_t i=0; i<faces.size(); i++) {
            SubdivisionFace* face = faces[i];
            area = face->getArea();
            if (i == 0 || area > seedarea) {
                seedface = face;
                seedarea = area;
            }
        }
        if (seedface != 0)
            seedfaces.push_back(seedface);
    }

    // sort seedfaces
    for (size_t i=0; i<seedfaces.size(); i++) {
        SubdivisionFace* seedface = seedfaces[i];
        double seedarea = seedface->getArea();
        for (size_t j=1; j<seedfaces.size(); j++) {
            SubdivisionFace* child = seedfaces[j];
            double area = child->getArea();
            if (area < seedarea) {
                swap(seedfaces[i], seedfaces[j]);
                seedarea = area;
            }
        }
    }

    double error;
    subdivface_iter error_index;
    if (seedfaces.size() > 0) {
        double maxerror = 1E10;
        size_t bestindex = seedfaces.size();
        size_t i = 0;
        _numIterations = 0;
        // Keep trying to develop the faces until no error has occured
        // and the max. error<1e-7 and the number of iterations<=25
        while (i != seedfaces.size()) {
            processFaces(seedfaces[i], error, error_index, faces);
            ++_numIterations;
            if (error_index != _donelist.end() && seedfaces.size() < 25) {
                // Add faces where an error occured as new seedfaces, these
                // are generally areas where gauss curvature<>0.0
                subdivface_iter idx = find(
                    seedfaces.begin(), seedfaces.end(), *error_index);
                if (idx == seedfaces.end())
                    seedfaces.push_back(*error_index);
            }
            if (error < maxerror) {
                maxerror = error;
                bestindex = i;
            }
            i++;
        }

        // restore the best development
        if (bestindex < seedfaces.size()) {
            processFaces(seedfaces[bestindex], error, error_index, faces);
        }

        // assemble all boundaryedges
        _boundaryEdges.clear();
        for (size_t i=0; i<_edges.size(); i++) {
            SubdivisionEdge* edge = _edges[i];
            // only edges with 1 attached face in the faces list are valid
            int n = 0;
            for (size_t j=0; j<edge->numberOfFaces(); j++) {
                subdivface_iter idx = find(faces.begin(), faces.end(), edge->getFace(j));
                if (idx != faces.end())
                    n++;
            }
            if (n == 1)
                _boundaryEdges.push_back(edge);
        }

        // calculate min/max coordinates in 2D
        for (size_t i=0; i<_points.size(); i++) {
            if (i == 0) {
                _min2D = _points[i].pt2D;
                _max2D = _min2D;
            } else {
                QVector2D& pt = _points[i].pt2D;
                if (pt.x() < _min2D.x())
                    _min2D.setX(pt.x());
                if (pt.y() < _min2D.y())
                    _min2D.setY(pt.y());
                if (pt.x() > _max2D.x())
                    _max2D.setX(pt.x());
                if (pt.y() > _max2D.y())
                    _max2D.setY(pt.y());
            }
        }

        // finally find optimal rotation angle such that the
        // area of the bounding box is minimal
        double optarea = 0;
        double optangle = 0;
        QVector3D min;
        QVector3D max;
        for (int i=0; i<=180; i++) {
            setRotation(i / 2.0);
            extents(min, max);
            double area = (max.x() - min.x()) * (max.y() - min.y());
            if (i == 0 || area < optarea) {
                optarea = area;
                optangle = _rotation;
            }
        }
        setRotation(optangle);
        extents(min, max);
        if ((max.x() - min.x()) < (max.y() - min.y()))
            setRotation(_rotation - 90);
        if (getOwner()->isSymmetric()) {
            // now check if the surface has one of its sides on the centerplane
            _mirror = false;
            vector<SubdivisionEdge*> tmpedges;
            for (size_t j=0; j<_edges.size(); j++) {
                SubdivisionEdge* edge = _edges[j];
                if (edge->numberOfFaces() == 1
                    && fabs(edge->startPoint()->getCoordinate().y()) <= 1E-4
                    && fabs(edge->endPoint()->getCoordinate().y()) <= 1E-4) {
                    _mirror = true;
                    tmpedges.push_back(edge);
                }
            }
            if (tmpedges.size() > 0) {
                if (_mirror) {
                    // Now if all facenormals have a Y-coordinate of approx. 0.0 then
                    // this is probably a bottom panel, a deck or a transom
                    for (size_t j=0; j<_donelist.size(); j++) {
                        SubdivisionFace* face = _donelist[j];
                        QVector3D normal = face->getFaceNormal();
                        if (fabs(normal.y()) > 1E-2) {
                            _mirror = false;
                            // y-coordinate of normal is too big, do not attach
                            break;
                        }
                    }
                    if (_mirror) {
                        // Check if all points on the centerline are
                        // developed onto a 2Dline if so, then this
                        // line is used to mirror the other half of
                        // the layer so it forms 1 whole panel and
                        // there is no need to unfold it.
                        vector<vector<SubdivisionPoint*> > sortededges;
                        getOwner()->getOwner()->isolateEdges(tmpedges, sortededges);
                        vector<SubdivisionPoint*>& centerline = sortededges.front();
                        if (centerline.size() > 0) {
                            SubdivisionPoint* p1 = centerline[0];
                            SubdivisionPoint* p2 = centerline.back();
                            if (p1 == p2) {
                                size_t j = centerline.size();
                                while (j > 1 && p1 == p2) {
                                    p2 = centerline[j-1];
                                    --j;
                                }
                            }
                            if (p1 != p2) {
                                patchpt_iter idx = find_point(_points.begin(), _points.end(),
                                                              p1);
                                QVector3D p3d1((*idx).pt2D.x(), (*idx).pt2D.y(), 0.0);
                                idx = find_point(_points.begin(), _points.end(), p2);
                                QVector3D p3d2((*idx).pt2D.x(), (*idx).pt2D.y(), 0.0);
                                for (size_t j=1; j<centerline.size(); j++) {
                                    SubdivisionPoint* p3 = centerline[j];
                                    idx = find_point(_points.begin(), _points.end(), p3);
                                    QVector3D p3d3((*idx).pt2D.x(), (*idx).pt2D.y(), 0.0);
                                    double dist = DistancepointToLine(p3d1, p3d2, p3d3);
                                    if (dist > 1E-3) {
                                        _mirror = false;
                                        break;
                                    }
                                }
                                if (_mirror) {
                                    idx = find_point(_points.begin(), _points.end(), p1);
                                    p3d1 = QVector3D((*idx).pt2D.x(), (*idx).pt2D.y(), 0.0);
                                    idx = find_point(_points.begin(), _points.end(), p2);
                                    p3d2 = QVector3D((*idx).pt2D.x(), (*idx).pt2D.y(), 0.0);
                                    QVector3D p3d3(p3d2);
                                    p3d3.setZ(1.0);
                                    _mirror = true;
                                    _mirrorPlane = Plane(p3d1, p3d2, p3d3);

                                    // calculate min/max coordinates in 2D of the mirror part
                                    for (size_t j=0; j<_points.size(); j++) {
                                        p3d2 = QVector3D(_points[j].pt->getCoordinate().x(),
                                                         _points[j].pt->getCoordinate().y(),
                                                         0);
                                        p3d1 = _mirrorPlane.mirror(p3d2);
                                        if (p3d1.x() < _min2D.x())
                                            _min2D.setX(p3d1.x());
                                        else if (p3d1.x() > _max2D.x())
                                            _max2D.setX(p3d1.x());
                                        if (p3d1.y() < _min2D.y())
                                            _min2D.setY(p3d1.y());
                                        else if (p3d1.y() > _max2D.y())
                                            _max2D.setY(p3d1.y());
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // assemble cornerpoints for dimensioning
        for (size_t i=0; i<_points.size(); i++) {
            SubdivisionPoint* p1 = _points[i].pt;
            int n = 0;
            for (size_t j=0; j<p1->numberOfFaces(); j++) {
                subdivface_iter idx = find(faces.begin(), faces.end(), p1->getFace(j));
                if (idx != faces.end())
                    n++;
             }
            if (n == 1 || p1->getVertexType() == svCorner)
                _corners.push_back(p1);
        }
    } else {
        // message dialog 'seed could not be found'
    }
}

// Computes the crossproduct of three points
// Returns whether their internal angle is clockwise or counter-clockwise.
PolygonOrientation DevelopedPatch::crossproduct(const QVector2D& p1, const QVector2D& p2,
                                                const QVector2D& p3)
{
    PolygonOrientation result = poCCW;
    double tmp = (p2.x() - p1.x()) * (p3.y() - p2.y()) - (p2.y() - p1.y()) * (p3.x() - p2.x());
    if (tmp >= 0)
        result = poCCW;
    else if (tmp < 0)
        result = poCW;
    return result;
}
    
// Calculates the third point of a triangle when the length of its
// three sides and two coordinates are known
QVector2D DevelopedPatch::calculateTriangle(double a, double b, double c,
                                            const QVector2D& p1, const QVector2D& p2)
{
    double fie1, fie2, fie3, p;
    
    if (fabs(p2.x() - p1.x()) <= 1E-6) {
        if (p2.x() == p1.x()) {
            if (p2.y() > p1.y())
                fie1 = 0.5 * M_PI;
            else
                fie1 = -0.5 * M_PI;
        } else {
            fie1 = atan((p2.y() - p1.y()) / (p2.x() - p1.x()));
        }
    } else {
        fie1 = atan((p2.y() - p1.y()) / (p2.x() - p1.x()));
    }
    if (fie1 < 0 && p2.y() > p1.y())
        fie1 += M_PI;
    if (fie1 > 0 && p2.y() < p1.y())
        fie1 += M_PI;
    if (b * c != 0)
        p = (b*b + c*c - a*a) / (2*b*c);
    else
        p = Sign(b*b + c*c - a*a);
    if (p > 1)
        p = 1;
    else if (p < -1)
        p = -1;
    if (p == 0) {
        fie2 = 0.5 * M_PI;
    } else {
        fie2 = atan(sqrt(1 - p*p)/p);
    }
    if (p < 0)
        fie2 = M_PI - fabs(fie2);
    fie3 = fie1 - fie2;
    return QVector2D(b*cos(fie3) + p1.x(), b*sin(fie3) + p1.y());
}

QVector2D DevelopedPatch::calculateTriangle2(double a, double b, double c,
                                             const QVector2D& p1, const QVector2D& p2)
{
    double fie1, fie2, beta, tmp;
    
    if (fabs(p2.x() - p1.x()) <= 1E-6) {
        if (p2.x() == p1.x()) {
            if (p2.y() > p1.y())
                fie1 = 0.5 * M_PI;
            else
                fie1 = -0.5 * M_PI;
        } else {
            fie1 = atan((p2.y() - p1.y()) / (p2.x() - p1.x()));
        }
    } else {
        fie1 = atan((p2.y() - p1.y()) / (p2.x() - p1.x()));
    }
    if (fie1 < 0 && p2.y() > p1.y())
        fie1 += M_PI;
    if (fie1 > 0 && p2.y() < p1.y())
        fie1 += M_PI;
    if (2 * a * c == 0) {
        if (a == 0)
            a = 1E-7;
        if (c == 0)
            c = 1E-7;
    }
    tmp = (a*a + c*c - b*b) / (2*a*c);
    if (tmp > 1)
        tmp = 1.0;
    else if (tmp < -1)
        tmp = -1.0;
    beta = acos(tmp);
    fie2 = fie1 + beta;
    return QVector2D(c*cos(fie2) + p1.x(), c*sin(fie2) + p1.y());
}

// FreeGeometry.pas:6408
void DevelopedPatch::unroll2D(SubdivisionFace* face, bool& firstface, bool& error,
                              PolygonOrientation& orientation)
{
    error = false;
    patchpt_iter index1;
    patchpt_iter index2;
    patchpt_iter index3;
    
    vector<bool> indices;
    indices.reserve(face->numberOfPoints());
    for (size_t i=0; i<face->numberOfPoints(); i++) {
        index1 = find_point(_points.begin(), _points.end(), face->getPoint(i));
        indices.push_back((*index1).processed);
    }
    // find two successive calculated points
    size_t s = face->numberOfPoints();
    size_t e = s;
    for (size_t i=0; i<face->numberOfPoints(); i++) {
        if (indices[i]) {
            size_t ind1 = (i + 1) % face->numberOfPoints();
            if (indices[ind1]) {
                s = i;
                e = ind1;
                break;
            }
        }
    }
    if (s != face->numberOfPoints() && e != face->numberOfPoints()) {
        for (size_t i=2; i<face->numberOfPoints(); i++) {
            SubdivisionPoint* p1 = face->getPoint(s);
            index1 = find_point(_points.begin(), _points.end(), p1);
            e = (s + i - 1) % face->numberOfPoints();
            SubdivisionPoint* p2 = face->getPoint(e);
            index2 = find_point(_points.begin(), _points.end(), p2);
            e = (s + i) % face->numberOfPoints();
            SubdivisionPoint* p3 = face->getPoint(e);
            index3 = find_point(_points.begin(), _points.end(), p3);
            processTriangle(index1, index2, index3, firstface, error, orientation);
        }
    } else {
        for (size_t i=2; i<face->numberOfPoints(); i++) {
            SubdivisionPoint* p1 = face->getPoint(0);
            index1 = find_point(_points.begin(), _points.end(), p1);
            SubdivisionPoint* p2 = face->getPoint(i-1);
            index2 = find_point(_points.begin(), _points.end(), p2);
            SubdivisionPoint* p3 = face->getPoint(i);
            index3 = find_point(_points.begin(), _points.end(), p3);
            processTriangle(index1, index2, index3, firstface, error, orientation);
        }
    }
}

// FreeGeometry.pas:6416
void DevelopedPatch::processTriangle(patchpt_iter ind1, patchpt_iter ind2, patchpt_iter ind3,
                                     bool& first, bool& error,
                                     PolygonOrientation& orientation)
{
    double a = (*ind1).pt->getCoordinate().distanceToPoint((*ind2).pt->getCoordinate());
    double b = (*ind2).pt->getCoordinate().distanceToPoint((*ind3).pt->getCoordinate());
    double c = (*ind3).pt->getCoordinate().distanceToPoint((*ind1).pt->getCoordinate());

    QVector2D p1_2d, p2_2d, p3_2d;

    if (!(*ind1).processed && !(*ind2).processed && !(*ind3).processed) {
        // first face, calculate p1, p2, and p3
        p1_2d.setX((*ind1).pt->getCoordinate().x());
        p1_2d.setY((*ind1).pt->getCoordinate().y());
        p2_2d = p1_2d;
        p2_2d.setY(p2_2d.y() + a);
        (*ind1).processed = true;
        (*ind1).pt2D = p1_2d;
        (*ind2).processed = true;
        (*ind2).pt2D = p2_2d;
    }
    p1_2d = (*ind1).pt2D;
    p2_2d = (*ind2).pt2D;
    p3_2d = (*ind3).pt2D;
    if ((*ind1).processed && (*ind2).processed && !(*ind3).processed) {
        // calculate position of p3
        a = p1_2d.distanceToPoint(p2_2d);
        // p3_2d = calculateTriangle(b,c,a,p1_2d, p2_2d)
        p3_2d = calculateTriangle2(a, b, c, p1_2d, p2_2d);
        if (first) {
            orientation = crossproduct(p1_2d, p2_2d, p3_2d);
            first = false;
        } else {
            PolygonOrientation winding = crossproduct(p1_2d, p2_2d, p3_2d);
            if (winding != orientation) {
                if (!error)
                    error = true;
            }
        }
        (*ind3).pt2D = p3_2d;
        (*ind3).processed = true;
    }
}

// FreeGeometry.pas:6520
double DevelopedPatch::triangleArea(const QVector2D& p1, const QVector2D& p2,
                                    const QVector2D& p3)
{
    return 0.5 * ((p1.x() - p2.x()) * (p1.y() + p2.y())
                  + (p2.x() - p3.x()) * (p2.y() + p3.y())
                  + (p3.x() - p1.x()) * (p3.y() + p1.y()));
    
}

// FreeGeometry.pas:6525
void DevelopedPatch::processFaces(SubdivisionFace* seedface, double& maxerror,
                                  subdivface_iter& error_index,
                                  vector<SubdivisionFace*>& faces)
{
    maxerror = 0.0;
    double totalerror = 0.0;
    _maxAreaError = 0.0;
    _totalAreaError = 0.0;
    bool founderror = false;
    //error_index = _donelist.end();
    _edgeErrors = vector<double>(_edges.size(), 0.0);
    for (size_t i=0; i<_points.size(); i++) {
        _points[i].processed = false;
        _points[i].pt2D = ZERO2;
    }
    vector<SubdivisionFace*> todo(faces.begin(), faces.end());
    _donelist.clear();
    bool temp = false;
    PolygonOrientation orientation = poCCW;
    while (todo.size() > 0) {
        if (seedface == 0)
            // find a new seedface, this layer has multiple areas
            seedface = todo.front();
        _donelist.push_back(seedface);
        subdivface_iter index = find(todo.begin(), todo.end(), seedface);
        if (index != todo.end())
            todo.erase(index);
        bool firstface = true;
        for (size_t i=0; i<_donelist.size(); i++) {
            SubdivisionFace* face = _donelist[i];
            unroll2D(face, firstface, temp, orientation);
            if (temp && founderror) {
                founderror = true;
                error_index = _donelist.begin() + i;
            }
            SubdivisionPoint* p1 = face->getPoint(face->numberOfPoints()-1);
            for (size_t j=0; j<face->numberOfPoints(); j++) {
                SubdivisionPoint* p2 = face->getPoint(j);
                SubdivisionEdge* edge = getOwner()->getOwner()->edgeExists(p1, p2);
                if (edge != 0) {
                    for (size_t k=0; k<edge->numberOfFaces(); k++) {
                        SubdivisionFace* child = edge->getFace(k);
                        index = find(todo.begin(), todo.end(), child);
                        if (index != todo.end()) {
                            _donelist.push_back(child);
                            todo.erase(index);
                        }
                    }
                }
                p1 = p2;
            }
        }
        seedface = 0;
    }

    // calculate diff in area of all faces
    _maxAreaError = 0.0;
    _totalAreaError = 0.0;
    for (size_t i=0; i<_donelist.size(); i++) {
        SubdivisionFace* face = _donelist[i];
        double _2Darea = 0.0;
        double _3Darea = face->getArea();
        // calculate 2D area
        patchpt_iter index1 = find_point(_points.begin(), _points.end(), face->getPoint(0));
        for (size_t j=2; j<face->numberOfPoints(); j++) {
            patchpt_iter index2 = find_point(_points.begin(), _points.end(),
                                             face->getPoint(j-1));
            patchpt_iter index3 = find_point(_points.begin(), _points.end(),
                                             face->getPoint(j));
            _2Darea += triangleArea((*index1).pt2D, (*index2).pt2D, (*index3).pt2D);
        }
        double error = _2Darea - _3Darea;
        _totalAreaError += error;
        error = fabs(error);
        if (error > _maxAreaError)
            _maxAreaError = error;
    }

    // calculate min/max errors of edges
    for (size_t i=0; i<_edges.size(); i++) {
        SubdivisionEdge* edge = _edges[i];
        patchpt_iter index1 = find_point(_points.begin(), _points.end(),
                                         edge->startPoint());
        patchpt_iter index2 = find_point(_points.begin(), _points.end(),
                                         edge->endPoint());
        if (index1 != _points.end() && index2 != _points.end()) {
            // original distance in 3D
            double L3D = edge->endPoint()->getCoordinate().distanceToPoint(
                edge->startPoint()->getCoordinate());
            double L2D = (*index2).pt2D.distanceToPoint((*index1).pt2D);
            double error = L2D - L3D;
            totalerror += fabs(error);
            if (fabs(error) > maxerror)
                maxerror = fabs(error);
            _edgeErrors[i] = error;
        }
    }
    maxerror += fabs(_totalAreaError);
    
    // set error index
    if (!founderror)
        error_index = _donelist.end();
}


void DevelopedPatch::dump(ostream& os) const
{
    os << " name:" << _name.toStdString();
}

ostream& operator << (ostream& os, const ShipCAD::DevelopedPatch& patch)
{
    patch.dump(os);
    return os;
}
