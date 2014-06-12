#include <iostream>
#include <algorithm>

#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "plane.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "version.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

static QVector3D ZERO = QVector3D(0,0,0);

static int sDecimals = 4;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionSurface::SubdivisionSurface()
{
    clear();
}

SubdivisionSurface::~SubdivisionSurface()
{
    // does nothing
}

SubdivisionControlPoint* SubdivisionSurface::newControlPoint(const QVector3D& p)
{
    SubdivisionControlPoint* pt = new SubdivisionControlPoint(this);
    pt->setCoordinate(p);
    _control_points.push_back(pt);
    return pt;
}

SubdivisionControlPoint* SubdivisionSurface::addControlPoint(const QVector3D& pt)
{
    SubdivisionControlPoint* result = 0;
    double max_error = 1E-5;
    for (size_t i=1; i<=numberOfControlEdges(); ++i) {
        SubdivisionControlEdge* edge = getControlEdge(i-1);
        if (edge->numberOfFaces() <= 1) { // boundary edge
            if (SquaredDistPP(pt, edge->startPoint()->getCoordinate()) <= max_error) {
                result = dynamic_cast<SubdivisionControlPoint*>(edge->startPoint());
                break;
            }
            else if (SquaredDistPP(pt, edge->endPoint()->getCoordinate()) <= max_error) {
                result = dynamic_cast<SubdivisionControlPoint*>(edge->endPoint());
                break;
            }
        }
    }
    if (result == 0) {
        // search controlpoints without edges
        for (size_t i=1; i<=numberOfControlPoints(); ++i) {
            SubdivisionControlPoint* point = getControlPoint(i-1);
            if (point->numberOfEdges() == 0) {
                if (SquaredDistPP(pt, point->getCoordinate()) <= max_error) {
                    result = point;
                    break;
                }
            }
        }
    }
    if (result == 0) {
        result = newControlPoint(pt);
    }
    return result;
}

void SubdivisionSurface::addControlPoint(SubdivisionControlPoint* pt)
{
    if (!hasControlPoint(pt)) {
        _control_points.push_back(pt);
        pt->setOwner(this);
    }
    setBuild(false);
}

SubdivisionControlPoint* SubdivisionSurface::addControlPoint()
{
    SubdivisionControlPoint* result = new SubdivisionControlPoint(this);
    result->setCoordinate(ZERO);
    _control_points.push_back(result);
    return result;
}

SubdivisionLayer* SubdivisionSurface::addNewLayer()
{
    SubdivisionLayer* result = new SubdivisionLayer(this);
    _layers.push_back(result);
    result->setLayerID(requestNewLayerID());
    setActiveLayer(result);
    emit changedLayerData();
    return result;
}

// used in assembleFacesToPatches
SubdivisionControlFace* SubdivisionSurface::getControlFace(SubdivisionPoint* p1,
                                                           SubdivisionPoint* p2,
                                                           SubdivisionPoint* p3,
                                                           SubdivisionPoint* p4)
{
    SubdivisionFace* face;
    SubdivisionControlFace* cface = 0;
    for (size_t i=1; i<=p1->numberOfFaces(); ++i) {
        face = p1->getFace(i-1);
        if (p2->hasFace(face) && p3->hasFace(face) && p4->hasFace(face)) {
            cface = dynamic_cast<SubdivisionControlFace*>(face);
            break;
        }
    }
    return cface;
}

void SubdivisionSurface::findAttachedFaces(vector<SubdivisionControlFace*>& found_list,
                                           vector<SubdivisionControlFace*>& todo_list,
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
                        found_list.push_back(f);
                        todo_list.erase(idx);
                        findAttachedFaces(found_list, todo_list, f);
                    }
                }
            }
        }
        p1 = p2;
    }
}

// tries to assemble quads into as few as possible rectangular patches
void SubdivisionSurface::assembleFacesToPatches(vector<SubdivisionLayer*>& layers,
                                                assemble_mode_t mode,
                                                vector<SubdivisionFace*>& assembledPatches,
                                                size_t& nAssembled)
{
}

void SubdivisionSurface::calculateGaussCurvature()
{
    if (!isBuild())
        rebuild();
    _min_gaus_curvature = 0;
    _max_gaus_curvature = 0;
    for (size_t i=1; i<=numberOfPoints(); ++i) {
        SubdivisionPoint* point = getPoint(i-1);
        _gaus_curvature.push_back(point->getCurvature());
        if (i == 1) {
            _min_gaus_curvature = _gaus_curvature[0];
            _max_gaus_curvature = _min_gaus_curvature;
        }
        if (_gaus_curvature[i-1] < _min_gaus_curvature)
            _min_gaus_curvature = _gaus_curvature[i-1];
        if (_gaus_curvature[i-1] > _max_gaus_curvature)
            _max_gaus_curvature = _gaus_curvature[i-1];
    }
}

SubdivisionControlPoint* SubdivisionSurface::getControlPoint(size_t index)
{
    if (index < _control_points.size())
        return _control_points[index];
    throw range_error("bad index in SubdivisionSurface::getControlPoint");
}

SubdivisionControlEdge* SubdivisionSurface::getControlEdge(size_t index)
{
    if (index < _control_edges.size())
        return _control_edges[index];
    throw range_error("bad index in SubdivisionSurface::getControlEdge");
}

SubdivisionControlCurve* SubdivisionSurface::getControlCurve(size_t index)
{
    if (index < _control_curves.size())
        return _control_curves[index];
    throw range_error("bad index in SubdivisionSurface::getControlCurve");
}

SubdivisionControlFace* SubdivisionSurface::getControlFace(size_t index)
{
    if (index < _control_faces.size())
        return _control_faces[index];
    throw range_error("bad index in SubdivisionSurface::getControlFace");
}

bool SubdivisionSurface::isGaussCurvatureCalculated()
{
    return (isBuild() && _gaus_curvature.size() == _points.size());
}

SubdivisionLayer* SubdivisionSurface::getLayer(size_t index)
{
    if (index < _layers.size())
        return _layers[index];
    throw range_error("bad index in SubdivisionSurface::getLayer");
}

size_t SubdivisionSurface::numberOfFaces()
{
    size_t result = 0;
    for (size_t i=1; i<=numberOfControlFaces(); ++i)
        result += getControlFace(i-1)->numberOfChildren();
    return result;
}

size_t SubdivisionSurface::numberOfLockedPoints()
{
    size_t result = 0;
    for (size_t i=1; i<=numberOfControlPoints(); ++i)
        if (getControlPoint(i-1)->isLocked())
            result++;
    return result;
}

SubdivisionPoint* SubdivisionSurface::getPoint(size_t index)
{
    if (_points.size() > 0) {
        if (index < _points.size())
            return _points[index];
        throw range_error("bad index in SubdivisionSurface::getPoint-1");
    }
    else if (index < _control_points.size())
        return _control_points[index];
    throw range_error("bad index in SubdivisionSurface::getPoint-2");
}

SubdivisionEdge* SubdivisionSurface::getEdge(size_t index)
{
    if (_edges.size() > 0) {
        if (index < _edges.size())
            return _edges[index];
        throw range_error("bad index in SubdivisionSurface::getEdge-1");
    }
    else if (index < _control_edges.size())
        return _control_edges[index];
    throw range_error("bad index in SubdivisionSurface::getEdge-2");
}

size_t SubdivisionSurface::numberOfPoints()
{
    if (_points.size() > 0)
        return _points.size();
    return _control_points.size();
}

size_t SubdivisionSurface::numberOfSelectedLockedPoints()
{
    size_t result = 0;
    for (size_t i=1; i<=numberOfSelectedControlPoints(); ++i)
        if (_sel_control_points[i-1]->isLocked())
            result++;
    return result;
}

size_t SubdivisionSurface::numberOfEdges()
{
    if (_edges.size() > 0)
        return _edges.size();
    return _control_edges.size();
}

void SubdivisionSurface::setSelectedControlEdge(SubdivisionControlEdge* edge)
{
    if (!hasSelectedControlEdge(edge))
        _sel_control_edges.push_back(edge);
}

bool SubdivisionSurface::hasSelectedControlEdge(SubdivisionControlEdge* edge)
{
    return (find(_sel_control_edges.begin(), _sel_control_edges.end(), edge)
            != _sel_control_edges.end());
}

void SubdivisionSurface::removeSelectedControlEdge(SubdivisionControlEdge *edge)
{
    vector<SubdivisionControlEdge*>::iterator i = find(
                _sel_control_edges.begin(), _sel_control_edges.end(), edge);
    _sel_control_edges.erase(i);
}

void SubdivisionSurface::setSelectedControlCurve(SubdivisionControlCurve* curve)
{
    if (!hasSelectedControlCurve(curve))
        _sel_control_curves.push_back(curve);
}

bool SubdivisionSurface::hasSelectedControlCurve(SubdivisionControlCurve* curve)
{
    return (find(_sel_control_curves.begin(), _sel_control_curves.end(), curve)
            != _sel_control_curves.end());
}

void SubdivisionSurface::removeSelectedControlCurve(SubdivisionControlCurve *curve)
{
    vector<SubdivisionControlCurve*>::iterator i = find(
                _sel_control_curves.begin(), _sel_control_curves.end(), curve);
    _sel_control_curves.erase(i);
}

void SubdivisionSurface::setSelectedControlFace(SubdivisionControlFace* face)
{
    if (!hasSelectedControlFace(face))
        _sel_control_faces.push_back(face);
}

bool SubdivisionSurface::hasSelectedControlFace(SubdivisionControlFace* face)
{
    return (find(_sel_control_faces.begin(), _sel_control_faces.end(), face)
            != _sel_control_faces.end());
}

void SubdivisionSurface::removeSelectedControlFace(SubdivisionControlFace *face)
{
    vector<SubdivisionControlFace*>::iterator i = find(
                _sel_control_faces.begin(), _sel_control_faces.end(), face);
    _sel_control_faces.erase(i);
}

void SubdivisionSurface::setSelectedControlPoint(SubdivisionControlPoint* pt)
{
    if (!hasSelectedControlPoint(pt))
        _sel_control_points.push_back(pt);
}

bool SubdivisionSurface::hasSelectedControlPoint(SubdivisionControlPoint* pt)
{
    return (find(_sel_control_points.begin(), _sel_control_points.end(), pt)
            != _sel_control_points.end());
}

void SubdivisionSurface::removeSelectedControlPoint(SubdivisionControlPoint *pt)
{
    vector<SubdivisionControlPoint*>::iterator i = find(
                _sel_control_points.begin(), _sel_control_points.end(), pt);
    _sel_control_points.erase(i);
}

size_t SubdivisionSurface::requestNewLayerID()
{
    _last_used_layerID++;
    return _last_used_layerID;
}

void SubdivisionSurface::setActiveLayer(SubdivisionLayer *layer)
{
    _active_layer = layer;
    emit changeActiveLayer();
}

void SubdivisionSurface::setBuild(bool val)
{
    if (!val) {
        clearFaces();
        for (size_t i=1; i<=numberOfControlCurves(); ++i)
            getControlCurve(i-1)->setBuild(false);
        _current_subdiv_level = 0;
        _gaus_curvature.clear();
        _min_gaus_curvature = 0;
        _max_gaus_curvature = 0;
    }
}

void SubdivisionSurface::setDesiredSubdivisionLevel(int val)
{
    if (val > 4)
        val = 4;
    if (val != _desired_subdiv_level) {
        _desired_subdiv_level = val;
        setBuild(false);
    }
}

void SubdivisionSurface::setSubdivisionMode(subdiv_mode_t val)
{
    if (val != _subdivision_mode) {
        _subdivision_mode = val;
        setBuild(false);
    }
}

SubdivisionControlFace* SubdivisionSurface::addControlFace(std::vector<QVector3D>& points)
{
    SubdivisionControlFace* result = 0;
    // remove double points
    size_t i = 0;
    double max_error = 1 / pow(10.0, sDecimals);
    while (i < points.size() - 1) {
        size_t j = i + 1;
        while (j < points.size()) {
            float dist = QVector3D(points[i] - points[j]).length();
            if (dist <= max_error) {
                points.erase(points.begin()+j);
            }
            else
                ++j;
        }
        ++i;
    }
    SubdivisionPoint* point;
    SubdivisionControlEdge* edge;
    SubdivisionPoint* prev = 0;
    if (points.size() > 2) {
        result = new SubdivisionControlFace(this);
        for (size_t i=1; i<=points.size(); ++i) {
            point = addControlPoint(points[i-1]);
            result->addPoint(point);
            if (i > 1) {
                edge = addControlEdge(prev, point);
                edge->addFace(result);
            }
            prev = point;
        }
        point = result->getPoint(0);
        edge = addControlEdge(prev, point);
        edge->addFace(result);

        // check if a point refers to the same face more than once ==> invalid face
        bool invalidface = false;
        for (size_t i=1; i<=result->numberOfPoints(); ++i) {
            size_t n = 0;
            point = result->getPoint(i-1);
            for (size_t j=1; j<=point->numberOfFaces(); ++j) {
                if (point->getFace(j-1) == result)
                    n++;
            }
            if (n > 1) {
                invalidface = true;
                break;
            }
        }
        if (result->numberOfPoints() < 3 || invalidface) {
            // delete invalid controlfaces
            for (size_t j=1; j<=result->numberOfPoints(); ++j) {
                result->getPoint(j-1)->deleteFace(result);
                if (j == 1)
                    edge = controlEdgeExists(result->getPoint(result->numberOfPoints()-1), result->getPoint(j-1));
                else
                    edge = controlEdgeExists(result->getPoint(j-2), result->getPoint(j-1));
                if (edge != 0) {
                    edge->deleteFace(result);
                    edge->startPoint()->deleteEdge(edge);
                    edge->startPoint()->deleteFace(result);
                    edge->endPoint()->deleteEdge(edge);
                    edge->endPoint()->deleteFace(result);
                }
            }
            delete result;
            result = 0;
        }
        else
            _control_faces.push_back(result);
    }
    else
        result = 0;
    setBuild(false);
    return result;
}

void SubdivisionSurface::dump(ostream& os) const
{
    os << "SubdivisionSurface ["
       << hex << this << "]\n";
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionSurface& surface)
{
    surface.dump(os);
    return os;
}
