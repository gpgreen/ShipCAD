#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "plane.h"
#include "spline.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "version.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

static QVector3D ZERO = QVector3D(0,0,0);
static QVector3D ONE = QVector3D(1,1,1);

static int sDecimals = 4;

bool ShipCADGeometry::g_surface_verbose = true;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionSurface::SubdivisionSurface()
    : _build(false), _show_curvature(true), _show_control_curves(true),
      _subdivision_mode(fmQuadTriangle), _control_point_size(4),
      _curvature_scale(0.25),
      _crease_color(Qt::green), _crease_edge_color(Qt::red),
      _underwater_color(Qt::gray),
      _edge_color(Qt::darkGray), _selected_color(Qt::yellow),
      _crease_point_color(Qt::green), _regular_point_color(QColor(0xc0, 0xc0, 0xc0)),
      _corner_point_color(QColor(0, 255, 255)), _dart_point_color(QColor(0xc0, 0x80, 0x00)),
      _layer_color(QColor(0, 255, 255)), _normal_color(QColor(0xc0, 0xc0, 0xc0)), _leak_color(Qt::red),
      _curvature_color(Qt::white), _control_curve_color(Qt::red),
      _zebra_color(Qt::black),
      _cpoint_pool(sizeof(SubdivisionControlPoint)),
      _cedge_pool(sizeof(SubdivisionControlEdge)),
      _cface_pool(sizeof(SubdivisionControlFace)),
      _ccurve_pool(sizeof(SubdivisionControlCurve)),
      _layer_pool(sizeof(SubdivisionLayer)),
      _point_pool(sizeof(SubdivisionPoint)),
      _edge_pool(sizeof(SubdivisionEdge)),
      _face_pool(sizeof(SubdivisionFace))
{
    clear();
}

SubdivisionSurface::~SubdivisionSurface()
{
    clear();
}

SubdivisionControlPoint* SubdivisionSurface::newControlPoint(const QVector3D& p)
{
    SubdivisionControlPoint* pt = SubdivisionControlPoint::construct(this);
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

bool SubdivisionSurface::hasControlPoint(SubdivisionControlPoint* pt)
{
    return find(_control_points.begin(), _control_points.end(), pt)
            != _control_points.end();
}

void SubdivisionSurface::removeControlPoint(SubdivisionControlPoint* pt)
{
    vector<SubdivisionControlPoint*>::iterator i = find(_control_points.begin(), _control_points.end(), pt);
    if (i != _control_points.end())
        _control_points.erase(i);
}

SubdivisionControlPoint* SubdivisionSurface::addControlPoint()
{
    return newControlPoint(ZERO);
}

// delete a controlpoint singly, not by dumping the pool
void SubdivisionSurface::deleteControlPoint(SubdivisionControlPoint* point)
{
    if (hasSelectedControlPoint(point))
        removeSelectedControlPoint(point);
    if (hasControlPoint(point)) {
        point->~SubdivisionControlPoint();
        _cpoint_pool.free(point);
    }
}

SubdivisionLayer* SubdivisionSurface::addNewLayer()
{
    SubdivisionLayer* result = SubdivisionLayer::construct(this);
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
    // BUGBUG: not implemented
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

size_t SubdivisionSurface::indexOfControlPoint(SubdivisionControlPoint *pt)
{
    vector<SubdivisionControlPoint*>::iterator i = find(_control_points.begin(),
                                                        _control_points.end(),
                                                        pt);
    if (i != _control_points.end())
        return i - _control_points.begin();
    throw range_error("point not found in SubdivisionSurface::indexOfControlPoint");
}

SubdivisionControlPoint* SubdivisionSurface::getControlPoint(size_t index)
{
    if (index < _control_points.size())
        return _control_points[index];
    throw range_error("bad index in SubdivisionSurface::getControlPoint");
}

bool SubdivisionSurface::hasControlEdge(SubdivisionControlEdge *edge)
{
    return find(_control_edges.begin(), _control_edges.end(),
                edge) != _control_edges.end();
}

size_t SubdivisionSurface::indexOfControlEdge(SubdivisionControlEdge *edge)
{
    vector<SubdivisionControlEdge*>::iterator i = find(_control_edges.begin(),
                                                       _control_edges.end(),
                                                       edge);
    if (i != _control_edges.end())
        return i - _control_edges.begin();
    throw range_error("edge not found in SubdivisionSurface::indexOfControlEdge");
}

SubdivisionControlEdge* SubdivisionSurface::getControlEdge(size_t index)
{
    if (index < _control_edges.size())
        return _control_edges[index];
    throw range_error("bad index in SubdivisionSurface::getControlEdge");
}

void SubdivisionSurface::removeControlEdge(SubdivisionControlEdge *edge)
{
    vector<SubdivisionControlEdge*>::iterator i = find(_control_edges.begin(),
                                                       _control_edges.end(),
                                                       edge);
    if (i != _control_edges.end())
        _control_edges.erase(i);
    throw range_error("edge not found in SubdivisionSurface::removeControlEdge");
}

// delete a controledge singly, not by dumping the pool
void SubdivisionSurface::deleteControlEdge(SubdivisionControlEdge* edge)
{
    if (hasSelectedControlEdge(edge))
        removeSelectedControlEdge(edge);
    if (hasControlEdge(edge)) {
        edge->~SubdivisionControlEdge();
        _cedge_pool.free(edge);
    }
}

SubdivisionControlCurve* SubdivisionSurface::getControlCurve(size_t index)
{
    if (index < _control_curves.size())
        return _control_curves[index];
    throw range_error("bad index in SubdivisionSurface::getControlCurve");
}

void SubdivisionSurface::removeControlCurve(SubdivisionControlCurve *curve)
{
    vector<SubdivisionControlCurve*>::iterator i = find(_control_curves.begin(),
                                                        _control_curves.end(),
                                                        curve);
    if (i != _control_curves.end())
        _control_curves.erase(i);
    throw range_error("curve not found in SubdivisionSurface::deleteControlCurve");
}

bool SubdivisionSurface::hasControlCurve(SubdivisionControlCurve* curve)
{
    return find(_control_curves.begin(), _control_curves.end(),
                curve) != _control_curves.end();
}

size_t SubdivisionSurface::indexOfControlFace(SubdivisionControlFace *face)
{
    vector<SubdivisionControlFace*>::iterator i = find(_control_faces.begin(),
                                                       _control_faces.end(),
                                                       face);
    if (i != _control_faces.end())
        return i - _control_faces.begin();
    throw range_error("face not found in SubdivisionSurface::indexOfControlFace");
}

bool SubdivisionSurface::hasControlFace(SubdivisionControlFace *face)
{
    return find(_control_faces.begin(),
                _control_faces.end(),
                face) != _control_faces.end();
}

void SubdivisionSurface::removeControlFace(SubdivisionControlFace *face)
{
    vector<SubdivisionControlFace*>::iterator i = find(_control_faces.begin(),
                                                       _control_faces.end(),
                                                       face);
    if (i != _control_faces.end())
        _control_faces.erase(i);
    throw range_error("face not found in SubdivisionSurface::removeControlFace");
}

void SubdivisionSurface::deleteControlFace(SubdivisionControlFace *face)
{
    if (hasSelectedControlFace(face))
        removeSelectedControlFace(face);
    if (hasControlFace(face)) {
        face->~SubdivisionControlFace();
        _cface_pool.free(face);
    }
}

SubdivisionControlFace* SubdivisionSurface::getControlFace(size_t index)
{
    if (index < _control_faces.size())
        return _control_faces[index];
    throw range_error("bad index in SubdivisionSurface::getControlFace");
}

void SubdivisionSurface::addControlFace(SubdivisionControlFace* face)
{
    if (!hasControlFace(face))
        _control_faces.push_back(face);
}

bool SubdivisionSurface::isGaussCurvatureCalculated()
{
    return (isBuild() && _gaus_curvature.size() == _points.size());
}

size_t SubdivisionSurface::indexOfLayer(SubdivisionLayer *layer)
{
    vector<SubdivisionLayer*>::iterator i = find(_layers.begin(),
                                                 _layers.end(),
                                                 layer);
    if (i != _layers.end())
        return i - _layers.begin();
    throw range_error("bad layer in SubdivisionSurface::indexOfLayer");
}

SubdivisionLayer* SubdivisionSurface::getLayer(size_t index)
{
    if (index < _layers.size())
        return _layers[index];
    throw range_error("bad index in SubdivisionSurface::getLayer");
}

bool SubdivisionSurface::hasLayer(SubdivisionLayer *layer)
{
    return find(_layers.begin(), _layers.end(), layer)
            != _layers.end();
}

void SubdivisionSurface::deleteLayer(SubdivisionLayer *layer)
{
    vector<SubdivisionLayer*>::iterator i = find(_layers.begin(),
                                                 _layers.end(),
                                                 layer);
    if (i != _layers.end())
        _layers.erase(i);
    else
        throw range_error("bad layer in SubdivisionSurface::deleteLayer");
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

size_t SubdivisionSurface::indexOfPoint(SubdivisionPoint *pt)
{
    vector<SubdivisionPoint*>::iterator i = find(_points.begin(),
                                                 _points.end(),
                                                 pt);
    if (i != _points.end())
        return i - _points.begin();
    throw range_error("point is not in SubdivisionSurface");
}

SubdivisionPoint* SubdivisionSurface::getPoint(size_t index)
{
    if (index < _points.size())
        return _points[index];
    throw range_error("bad index in SubdivisionSurface::getPoint");
}

SubdivisionEdge* SubdivisionSurface::getEdge(size_t index)
{
    if (index < _edges.size())
        return _edges[index];
    throw range_error("bad index in SubdivisionSurface::getEdge");
}

size_t SubdivisionSurface::indexOfEdge(SubdivisionEdge *edge)
{
    vector<SubdivisionEdge*>::iterator i = find(_edges.begin(),
                                                _edges.end(),
                                                edge);
    if (i != _edges.end())
        return i - _edges.begin();
    throw range_error("edge is not in SubdivisionSurface");
}

size_t SubdivisionSurface::numberOfSelectedLockedPoints()
{
    size_t result = 0;
    for (size_t i=1; i<=numberOfSelectedControlPoints(); ++i)
        if (_sel_control_points[i-1]->isLocked())
            result++;
    return result;
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
        result = SubdivisionControlFace::construct(this);
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

SubdivisionControlEdge* SubdivisionSurface::addControlEdge(SubdivisionPoint *sp, SubdivisionPoint *ep)
{
    SubdivisionControlEdge* edge = controlEdgeExists(sp, ep);
    if (edge == 0) {
        edge = SubdivisionControlEdge::construct(this);
        edge->setPoints(sp, ep);
        edge->setControlEdge(true);
        sp->addEdge(edge);
        ep->addEdge(edge);
        _control_edges.push_back(edge);
    }
    return edge;
}

void SubdivisionSurface::addControlCurve(SubdivisionControlCurve *curve)
{
    _control_curves.push_back(curve);
    curve->setOwner(this);
    setBuild(false);
}

SubdivisionControlFace* SubdivisionSurface::addControlFace(std::vector<SubdivisionControlPoint *> &points, bool check_edges, SubdivisionLayer *layer)
{
    SubdivisionControlFace* result = 0;
    SubdivisionControlPoint* p1, *p2;
    SubdivisionControlEdge* edge;

    if (points.size() > 2 && points.back() == points.front())
        points.pop_back();
    if (points.size() > 2) {
        if (points.back() == points.front())
            points.pop_back();
        // check if another patch with the same vertices exists
        bool face_exists = false;
        size_t i = 1;
        while (i <= points.size()) {
            p1 = points[i-1];
            size_t j = 1;
            while (j <= p1->numberOfFaces()) {
                SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(p1->getFace(j-1));
                if (face->numberOfPoints() == points.size()) {
                    face_exists = true;
                    size_t n = 1;
                    while (n <= points.size() && face_exists) {
                        if (face->hasPoint(points[n-1])) {
                            n = points.size();
                            face_exists = false;
                        }
                        else
                            ++n;
                    }
                    if (face_exists) {
                        result = 0;
                        return 0;
                    }
                }
                ++j;
            }
            ++i;
        }
        if (face_exists) {
            result = 0;
            return 0;
        }
        result = SubdivisionControlFace::construct(this);
        if (layer == 0)
            result->setLayer(getLayer(0));
        else
            result->setLayer(layer);
        _control_faces.push_back(result);
        p1 = points.back();
        for (size_t i=1; i<=points.size(); ++i) {
            p2 = points[i-1];
            p2->addFace(result);
            result->addPoint(p2);
            edge = controlEdgeExists(p1, p2);
            if (edge == 0) {
                edge = SubdivisionControlEdge::construct(this);
                edge->setPoints(p1, p2);
                edge->setControlEdge(true);
                p1->addEdge(edge);
                p2->addEdge(edge);
                _control_edges.push_back(edge);
                edge->addFace(result);
                edge->setCrease(true);
            }
            else {
                edge->addFace(result);
                if (check_edges)
                    edge->setCrease(edge->numberOfFaces() < 2);
            }
            p1 = p2;
        }
        if (result->numberOfPoints() < 3) {
            delete result;
            result = 0;
        }
        else
            setBuild(false);
    }
    return result;
}

SubdivisionControlFace* SubdivisionSurface::addControlFace(std::vector<SubdivisionControlPoint *> &points, bool check_edges)
{
    return addControlFace(points, check_edges, 0);
}

void SubdivisionSurface::clear()
{
    _control_curves.clear();
    _control_faces.clear();
    _control_edges.clear();
    _control_points.clear();
    _layers.clear();
    _edges.clear();
    _points.clear();

    // clear the pools
    _ccurve_pool.release_memory();
    _cface_pool.release_memory();
    _cedge_pool.release_memory();
    _cpoint_pool.release_memory();
    _layer_pool.release_memory();
    _edge_pool.release_memory();
    _point_pool.release_memory();

    _last_used_layerID = 0;
    _sel_control_curves.clear();
    _sel_control_edges.clear();
    _sel_control_faces.clear();
    _sel_control_points.clear();
    // add one default layer and set it to active
    SubdivisionLayer* layer = addNewLayer();
    _active_layer = layer;
    _draw_mirror = false;
    _show_control_net = true;
    _show_interior_edges = false;
    _initialized = false;
    _desired_subdiv_level = -1;
    _show_normals = true;
    _shade_under_water = false;
    _main_frame_location = 1E10;
    setBuild(false);
    emit changedLayerData();
}

void SubdivisionSurface::clearFaces()
{
    for (size_t i=1; i<=numberOfControlFaces(); ++i) {
        getControlFace(i-1)->clearChildren();       // deletes children and rendermesh
    }
    // dump all edges
    _edges.clear();
    _edge_pool.release_memory();
    // dump all points
    _points.clear();
    _point_pool.release_memory();
    // clear edges in control faces
    for (size_t i=1; i<=numberOfControlFaces(); ++i)
        getControlFace(i-1)->clearControlEdges();
}

void SubdivisionSurface::clearSelection()
{
    _sel_control_curves.clear();
    _sel_control_edges.clear();
    _sel_control_faces.clear();
    _sel_control_points.clear();
    emit selectItem(0);
}

bool SubdivisionSurface::validFace(SubdivisionFace* face,
                                   vector<SubdivisionFace*>& faces,
                                   vector<SubdivisionFace*>& tmpfaces)
{
    bool result = false;
    if (face->numberOfPoints() == 4) {
        vector<SubdivisionFace*>::iterator i = find(faces.begin(), faces.end(), face);
        if (i != faces.end()) {
            result = true;
            for (size_t i=1; i<=tmpfaces.size(); ++i) {
                if (tmpfaces[i-1] == face) {
                    return false;
                }
            }
            if (tmpfaces.size() > 0) {
                // must also be connected to previous face
                SubdivisionFace* tmp = tmpfaces.back();
                size_t n = 0;
                for (size_t j=1; j<=face->numberOfPoints(); ++j) {
                    if (tmp->indexOfPoint(face->getPoint(j-1)))
                        ++n;
                }
                result = n > 1;
            }
        }
    }
    return result;
}

void SubdivisionSurface::doAssemble(grid_t& grid, 
                                    size_t& cols,
                                    size_t& rows,
                                    vector<SubdivisionFace*>& faces)
{
    bool search_bottom, search_top, search_left, search_right;
    size_t counter, index;
    SubdivisionEdge* edge;
    SubdivisionFace* face;
    vector<SubdivisionFace*> tmpfaces;
    vector<SubdivisionFace*>::iterator fidx;

    search_bottom = search_top = search_left = search_right = true;
    counter = 0;
    while ((search_bottom || search_top || search_right || search_left)
           && faces.size() > 0) {
        ++counter;
        if (counter > 4)
            counter = 1;
        if (counter == 1 && search_bottom) {
            tmpfaces.clear();
            for (size_t i=2; i<=grid[0].size(); ++i) {
                edge = edgeExists(grid[rows-1][i-2], grid[rows-1][i-1]);
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (validFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i - 1)
                    break;
            }
            if (tmpfaces.size() == cols - 1) {
                // search was successfull
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    vector<SubdivisionPoint*> newrow(cols);
                    for (size_t j=0; j<cols; ++j)
                        newrow.push_back(static_cast<SubdivisionPoint*>(0));
                    grid.push_back(newrow);
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid[rows-1][i]);
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid[rows-1][i-1]) {
                        index = (index + 1) % face->numberOfPoints();
                        grid[rows][i-1] = face->getPoint(index);
                        index = (index + 1) % face->numberOfPoints();
                        grid[rows][i] = face->getPoint(index);
                    }
                    else {
                        index = face->indexOfPoint(grid[rows-1][i-1]);
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid[rows-1][i]) {
                            index = (index + 1) % face->numberOfPoints();
                            grid[rows][i] = face->getPoint(index);
                            index = (index + 1) % face->numberOfPoints();
                            grid[rows][i-1] = face->getPoint(index);
                        }
                    }
                }
                ++rows;
            }
            else
                search_bottom = false;
        }
        else if (counter == 2 && search_right) {
            tmpfaces.clear();
            for (size_t i=2; i<=rows; ++i) {
                edge = edgeExists(grid[i-1][cols-1], grid[i-2][cols-1]);
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (validFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i - 1)
                    break;
            }
            if (tmpfaces.size() == rows - 1) {
                // search was successfull
                for (size_t i=1; i<=rows; ++i) {
                    grid[i-1].push_back(static_cast<SubdivisionPoint*>(0));
                }
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid[i-1][cols-1]);
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid[i][cols-1]) {
                        index = (index + 1) % face->numberOfPoints();
                        grid[i][cols] = face->getPoint(index);
                        index = (index + 1) % face->numberOfPoints();
                        grid[i-1][cols] = face->getPoint(index);
                    }
                    else {
                        index = face->indexOfPoint(grid[i][cols-1]);
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid[i-1][cols-1]) {
                            index = (index + 1) % face->numberOfPoints();
                            grid[i-1][cols] = face->getPoint(index);
                            index = (index + 1) % face->numberOfPoints();
                            grid[i][cols] = face->getPoint(index);
                        }
                    }
                }
                ++cols;
            }
            else
                search_right = false;
        }
        else if (counter == 3 && search_top) {
            tmpfaces.clear();
            for (size_t i=2; i<=cols; ++i) {
                edge = edgeExists(grid[0][i-2], grid[0][i-1]);
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (validFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i - 1)
                    break;
            }
            if (tmpfaces.size() == cols - 1) {
                // search was successfull
                vector<SubdivisionPoint*> newrow(cols);
                for (size_t j=0; j<cols; ++j)
                    newrow.push_back(static_cast<SubdivisionPoint*>(0));
                grid.insert(grid.begin(), newrow);
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid[1][i-1]);
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid[1][i]) {
                        index = (index + 1) % face->numberOfPoints();
                        grid[0][i] = face->getPoint(index);
                        index = (index + 1) % face->numberOfPoints();
                        grid[0][i-1] = face->getPoint(index);
                    }
                    else {
                        index = face->indexOfPoint(grid[1][i]);
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid[1][i-1]) {
                            index = (index + 1) % face->numberOfPoints();
                            grid[0][i-1] = face->getPoint(index);
                            index = (index + 1) % face->numberOfPoints();
                            grid[0][i] = face->getPoint(index);
                        }
                    }
                }
                ++rows;
            }
            else
                search_top = false;
        }
        else if (counter == 4 && search_left) {
            tmpfaces.clear();
            for (size_t i=2; i<=rows; ++i) {
                edge = edgeExists(grid[i-2][0], grid[i-1][0]);
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (validFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i - 1)
                    break;
            }
            if (tmpfaces.size() == rows - 1) {
                // search was successfull
                for (size_t i=1; i<=rows; ++i) {
                    grid[i-1].insert(grid[i-1].begin(), static_cast<SubdivisionPoint*>(0));
                }
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid[i][1]);
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid[i-1][1]) {
                        index = (index + 1) % face->numberOfPoints();
                        grid[i-1][0] = face->getPoint(index);
                        index = (index + 1) % face->numberOfPoints();
                        grid[i][0] = face->getPoint(index);
                    }
                    else {
                        index = face->indexOfPoint(grid[i-1][1]);
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid[i][1]) {
                            index = (index + 1) % face->numberOfPoints();
                            grid[i][0] = face->getPoint(index);
                            index = (index + 1) % face->numberOfPoints();
                            grid[i-1][0] = face->getPoint(index);
                        }
                    }
                }
                ++cols;
            }
            else
                search_left = false;
        }
    }
}

void SubdivisionSurface::convertToGrid(face_grid_t& input, grid_t& grid)
{
    size_t cols = 0;
    size_t rows = 0;
    if (input.size() == 0 || input[0].size() == 0)
        return;
    // assemble all childfaces in one temp sorted list
    size_t n = input[0][0]->numberOfChildren();
    vector<SubdivisionFace*> backup(input.size() * input[0].size() * n);
    for (size_t i=1; i<=input.size(); ++i) {
        for (size_t j=1; j<=input[i-1].size(); ++j) {
            SubdivisionControlFace* ctrlface = input[i-1][j-1];
            backup.insert(backup.end(), ctrlface->childrenBegin(), ctrlface->childrenEnd());
        }
    }
    // BUGBUG: how does this sort them? the comparator
    sort(backup.begin(), backup.end());
    if (backup.size() == 0)
        return;
    vector<SubdivisionFace*> faces(backup.size());
    size_t ind = 0;
    do {
        faces.insert(faces.end(), backup.begin(), backup.end());
        ++ind;
        SubdivisionFace* face = faces[ind-1];
        faces.erase(faces.begin()+ind-1);
        rows = 2;
        cols = 2;
        vector<SubdivisionPoint*> row0(2);
        row0.push_back(face->getPoint(0));
        row0.push_back(face->getPoint(1));
        grid.push_back(row0);
        vector<SubdivisionPoint*> row1(2);
        row1.push_back(face->getPoint(2));
        row1.push_back(face->getPoint(3));
        grid.push_back(row1);
        doAssemble(grid, cols, rows, faces);
    }
    while (faces.size() > 0 && ind < backup.size());
    if (faces.size() != 0)
        throw runtime_error("Could not establish the entire grid!");
}

void SubdivisionSurface::edgeConnect()
{
    if (numberOfSelectedControlPoints() <= 1)
        return;
    for (size_t i=numberOfSelectedControlPoints()-1; i>=1; --i) {
        SubdivisionControlPoint* v1 = _sel_control_points[numberOfSelectedControlPoints()-2];
        SubdivisionControlPoint* v2 = _sel_control_points[numberOfSelectedControlPoints()-1];
        if (controlEdgeExists(v1, v2) == 0) {
            if (v1->numberOfFaces() == 0 && v2->numberOfFaces() == 0) {
                SubdivisionControlEdge* edge = addControlEdge(v1, v2);
                if (edge != 0)
                    edge->setCrease(true);
            }
            else {
                for (size_t j=1; j<=v1->numberOfFaces(); ++j) {
                    SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(v1->getFace(j-1));
                    if (v2->hasFace(face)) {
                        face->insertControlEdge(v1, v2);
                        v2->setSelected(false);
                        setBuild(false);
                        break;
                    }
                }
            }
        }
        else if (numberOfSelectedControlPoints() == 2) {
            // BUGBUG: userstring 202
            throw runtime_error("");
        }
    }
    for (size_t i=numberOfSelectedControlPoints(); i>=1; --i) {
        _sel_control_points[i-1]->setSelected(false);
    }
}

void SubdivisionSurface::exportFeFFile(vector<QString>& strings)
{
    // add layer information
    strings.push_back(QString("%1").arg(numberOfLayers()));
    for (size_t i=1; i<=numberOfLayers(); ++i) {
        SubdivisionLayer* layer = getLayer(i-1);
        strings.push_back(layer->getName());
        strings.push_back(QString("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10")
                          .arg(layer->getLayerID())
                          .arg(FindDXFColorIndex(layer->getColor()))
                          .arg(BoolToStr(layer->isVisible()))
                          .arg(BoolToStr(layer->isDevelopable()))
                          .arg(BoolToStr(layer->isSymmetric()))
                          .arg(BoolToStr(layer->useForIntersections()))
                          .arg(BoolToStr(layer->useInHydrostatics()))
                          .arg(BoolToStr(layer->showInLinesplan()))
                          // BUGBUG: FloatToStrF ffFixed, 10, 8
                          .arg(layer->getMaterialDensity())
                          .arg(layer->getThickness()));
    }
    // BUGBUG: save to stream not implemented for points/edges/faces
}

void SubdivisionSurface::exportObjFile(bool export_control_net, vector<QString>& strings)
{
    if (!isBuild())
        rebuild();
    strings.push_back("# FREE!ship model");
    // first sort controlpoints for faster access of function indexOf()
    sort(_points.begin(), _points.end());

    if (!export_control_net) {
        // export subdivided surface
        // create points for portside
        vector<SubdivisionPoint*> tmp(numberOfPoints());
        for (size_t i=1; i<=numberOfPoints(); ++i) {
            // BUGBUG: FloatToStrF ffFixed, 7, 4
            strings.push_back(QString("v %1 %2 %3").arg(getPoint(i-1)->getCoordinate().y())
                              .arg(getPoint(i-1)->getCoordinate().z())
                              .arg(getPoint(i-1)->getCoordinate().x()));
            if (getPoint(i-1)->getCoordinate().y() > 0)
                tmp.push_back(getPoint(i-1));
        }
        if (isDrawMirror()) {
            // create points for starboard side
            sort(tmp.begin(), tmp.end());
            for (size_t i=1; i<=tmp.size(); ++i) {
                SubdivisionPoint* p = tmp[i-1];
                strings.push_back(QString("v %1 %2 %3").arg(-p->getCoordinate().y())
                                  .arg(p->getCoordinate().z())
                                  .arg(p->getCoordinate().x()));
            }
        }
        for (size_t i=1; i<=numberOfControlFaces(); ++i) {
            SubdivisionControlFace* cface = _control_faces[i-1];
            if (cface->getLayer()->isVisible()) {
                for (size_t j=1; j<=cface->numberOfChildren(); ++j) {
                    // portside
                    QString str("f");
                    SubdivisionFace* child = cface->getChild(j-1);
                    for (size_t k=1; k<=child->numberOfPoints(); ++k) {
                        size_t index = indexOfPoint(child->getPoint(k-1));
                        str.append(QString(" %1").arg(index+1));
                    }
                    strings.push_back(str);
                    if (cface->getLayer()->isSymmetric() && isDrawMirror()) {
                        // starboard side
                        QString str("f");
                        for (size_t k=child->numberOfPoints(); k>=1; --k) {
                            size_t index;
                            if (child->getPoint(k-1)->getCoordinate().y() > 0) {
                                vector<SubdivisionPoint*>::iterator idx =
                                        find(tmp.begin(), tmp.end(), child->getPoint(k-1));
                                index = (idx - tmp.begin()) + numberOfPoints();
                            }
                            else
                                index = indexOfPoint(child->getPoint(k-1));
                            str.append(QString(" %1").arg(index+1));
                        }
                        strings.push_back(str);
                    }
                }
            }
        }
    }
    else {
        // export the control net only
        // first sort controlpoints for fac
        // create points for portside
        vector<SubdivisionPoint*> tmp(numberOfPoints());
        for (size_t i=1; i<=numberOfControlPoints(); ++i) {
            // BUGBUG: FloatToStrF ffFixed, 7, 4
            strings.push_back(QString("v %1 %2 %3").arg(getControlPoint(i-1)->getCoordinate().y())
                              .arg(getControlPoint(i-1)->getCoordinate().z())
                              .arg(getControlPoint(i-1)->getCoordinate().x()));
            if (getControlPoint(i-1)->getCoordinate().y() > 0)
                tmp.push_back(getControlPoint(i-1));
        }
        if (isDrawMirror()) {
            // create points for starboard side
            sort(tmp.begin(), tmp.end());
            for (size_t i=1; i<=tmp.size(); ++i) {
                SubdivisionPoint* p = tmp[i-1];
                strings.push_back(QString("v %1 %2 %3").arg(-p->getCoordinate().y())
                                  .arg(p->getCoordinate().z())
                                  .arg(p->getCoordinate().x()));
            }
        }
        for (size_t i=1; i<=numberOfControlFaces(); ++i) {
            SubdivisionControlFace* cface = _control_faces[i-1];
            if (cface->getLayer()->isVisible()) {
                for (size_t j=1; j<=cface->numberOfChildren(); ++j) {
                    // portside
                    QString str("f");
                    for (size_t k=1; k<=cface->numberOfPoints(); ++k) {
                        size_t index = indexOfPoint(cface->getPoint(k-1));
                        str.append(QString(" %1").arg(index+1));
                    }
                    strings.push_back(str);
                    if (cface->getLayer()->isSymmetric() && isDrawMirror()) {
                        // starboard side
                        QString str("f");
                        for (size_t k=cface->numberOfPoints(); k>=1; --k) {
                            size_t index;
                            if (cface->getPoint(k-1)->getCoordinate().y() > 0) {
                                vector<SubdivisionPoint*>::iterator idx =
                                        find(tmp.begin(), tmp.end(), cface->getPoint(k-1));
                                index = (idx - tmp.begin()) + numberOfPoints();
                            }
                            else
                                index = indexOfPoint(cface->getPoint(k-1));
                            str.append(QString(" %1").arg(index+1));
                        }
                        strings.push_back(str);
                    }
                }
            }
        }
    }
}

void SubdivisionSurface::extents(QVector3D& min, QVector3D& max)
{
    if (!isBuild())
        rebuild();
    if (numberOfControlFaces() > 0) {
        for (size_t i=1; i<=numberOfLayers(); ++i)
            getLayer(i-1)->extents(min, max);
        for (size_t i=1; i<=numberOfControlPoints(); ++i) {
            if (_control_points[i-1]->numberOfFaces() == 0)
                MinMax(_control_points[i-1]->getCoordinate(), min, max);
        }
    }
    else {
        MinMax(_min, min, max);
        MinMax(_max, min, max);
    }
}

// predicate class to find an element with given point
struct ExistPointPred {
    ShipCADGeometry::SubdivisionControlPoint* _querypt;
    bool operator()(const pair<ShipCADGeometry::SubdivisionControlPoint*, ShipCADGeometry::SubdivisionControlPoint*>& val)
    {
        return val.first == _querypt;
    }
    ExistPointPred (ShipCADGeometry::SubdivisionPoint* querypt) : _querypt(dynamic_cast<SubdivisionControlPoint*>(querypt)) {}
};

// predicate class to find an element with given point
struct ExtrudePointPred {
    ShipCADGeometry::SubdivisionControlPoint* _querypt;
    bool operator()(const pair<ShipCADGeometry::SubdivisionControlPoint*, ShipCADGeometry::SubdivisionControlPoint*>& val)
    {
        return val.second == _querypt;
    }
    ExtrudePointPred (ShipCADGeometry::SubdivisionPoint* querypt) : _querypt(dynamic_cast<SubdivisionControlPoint*>(querypt)) {}
};

void SubdivisionSurface::extrudeEdges(vector<SubdivisionControlEdge*>& edges,
                                      const QVector3D& direction)
{
    // first point is from edge, second is extruded point
    vector<pair<SubdivisionControlPoint*, SubdivisionControlPoint*> > vertices;
    vector<pair<SubdivisionControlPoint*, SubdivisionControlPoint*> >::iterator vidx;
    vector<SubdivisionControlPoint*> vertice_objs;
    vector<SubdivisionControlPoint*> points;
    vector<SubdivisionControlEdge*> newedges;
    SubdivisionControlEdge* edge, *tmp;
    SubdivisionControlPoint* point1, *point2;

    // assemble all points
    for (size_t i=1; i<=edges.size(); ++i) {
        edge = edges[i-1];
        point1 = dynamic_cast<SubdivisionControlPoint*>(edge->startPoint());
        point2 = dynamic_cast<SubdivisionControlPoint*>(edge->endPoint());
        if (find_if(vertices.begin(), vertices.end(), ExistPointPred(point1)) == vertices.end())
            vertices.push_back(make_pair(point1, static_cast<SubdivisionControlPoint*>(0)));
        if (find_if(vertices.begin(), vertices.end(), ExistPointPred(point2)) == vertices.end())
            vertices.push_back(make_pair(point2, static_cast<SubdivisionControlPoint*>(0)));
    }
    size_t noedges = numberOfControlEdges();
    size_t novertices = numberOfControlPoints();
    // create all new extruded points
    for (size_t i=1; i<=vertices.size(); ++i) {
        point1 = vertices[i-1].first;
        point2 = SubdivisionControlPoint::construct(this);
        point2->setCoordinate(point1->getCoordinate() + direction);
        vertices[i-1].second = point2;
    }
    for (size_t i=1; i<=edges.size(); ++i) {
        edge = edges[i-1];
        points.clear();
        points.push_back(dynamic_cast<SubdivisionControlPoint*>(edge->endPoint()));
        points.push_back(dynamic_cast<SubdivisionControlPoint*>(edge->startPoint()));
        point1 = 0;
        point2 = 0;
        vidx = find_if(vertices.begin(), vertices.end(), ExtrudePointPred(edge->startPoint()));
        if (vidx != vertices.end()) {
            point1 = (*vidx).second;
            points.push_back(point1);
        }
        vidx = find_if(vertices.begin(), vertices.end(), ExtrudePointPred(edge->endPoint()));
        if (vidx != vertices.end()) {
            point2 = (*vidx).second;
            points.push_back(point2);
        }
        SubdivisionControlFace* face = addControlFace(points, true);
        face->setLayer(getActiveLayer());
        if (point1 != 0 && point2 != 0) {
            tmp = controlEdgeExists(point1, point2);
            if (tmp != 0)
                newedges.push_back(tmp);
        }
        if (edge->startPoint()->getVertexType() == SubdivisionControlPoint::svCorner && point1 != 0) {
            tmp = controlEdgeExists(edge->startPoint(), point1);
            if (tmp != 0)
                tmp->setCrease(true);
        }
        if (edge->endPoint()->getVertexType() == SubdivisionControlPoint::svCorner && point2 != 0) {
            tmp = controlEdgeExists(edge->endPoint(), point2);
            if (tmp != 0)
                tmp->setCrease(true);
        }
        edge->setCrease(true);
    }
    // return the new edges
    for (size_t i=1; i<=newedges.size(); ++i)
        edges.push_back(newedges[i-1]);
    initialize(novertices+1, noedges+1);
}

struct SurfIntersectionData
{
    QVector3D point;
    bool knuckle;
    SubdivisionEdge* edge;
    SurfIntersectionData() : point(ZERO), knuckle(false), edge(0) {}
    SurfIntersectionData(const QVector3D& pt) : point(pt), knuckle(false), edge(0) {}
};

void SubdivisionSurface::calculateIntersections(const Plane& plane,
                                                vector<SubdivisionControlFace*>& faces,
                                                vector<Spline*>& destination)
{
    SubdivisionControlFace* ctrlface;
    SubdivisionFace* face, *f2;
    SubdivisionPoint* p1, *p2, *p3;
    SubdivisionEdge* edge;
    float side1, side2;
    bool addedge;

    // first assemble all edges belong to this set of faces
    vector<SubdivisionEdge*> edges(faces.size()+100);
    vector<SurfIntersectionData> intarray(10);

    // first is start point, second is end point
    vector<pair<SurfIntersectionData, SurfIntersectionData> > segments(50);

    for (size_t i=1; i<=faces.size(); ++i) {
        ctrlface = faces[i-1];
        for (size_t j=1; j<=ctrlface->numberOfChildren(); ++j) {
            face = ctrlface->getChild(j-1);
            intarray.clear();
            p1 = face->getPoint(face->numberOfPoints()-1);
            side1 = plane.distance(p1->getCoordinate());
            for (size_t k=1; k<=face->numberOfPoints(); ++k) {
                p2 = face->getPoint(k-1);
                side2 = plane.distance(p2->getCoordinate());
                bool addedge = false;
                if ((side1 < -1E-5 && side2 > 1E-5) || (side1 > 1E5 && side2 < -1E-5)) {
                    // regular intersection of edge
                    // add the edge to the list
                    float parameter = side1 / (side2 - side1);
                    QVector3D output = p1->getCoordinate()
                            + parameter * (p2->getCoordinate() - p1->getCoordinate());
                    intarray.push_back(SurfIntersectionData(output));
                    edge = edgeExists(p1, p2);
                    if (edge != 0) {
                        intarray.back().knuckle = edge->isCrease();
                        intarray.back().edge = edge;
                    }
                    else {
                        intarray.back().knuckle = false;
                        intarray.back().edge = 0;
                    }
                }
                else {
                    // does the edge lie entirely within the plane?
                    if ((fabs(side1) <= 1E-5) && (fabs(side2) <= 1E-5)) {
                        // if so, then add this edge ONLY if:
                        // 1. the edge is a boundary edge
                        // 2. at least ONE of the attached faces does NOT lie in the plane
                        edge = edgeExists(p1, p2);
                        if (edge != 0) {
                            if (edge->numberOfFaces() == 1)
                                addedge = true;
                            else {
                                for (size_t n=1; n<=edge->numberOfFaces(); ++n) {
                                    f2 = edge->getFace(n-1);
                                    for (size_t m=1; m<=f2->numberOfPoints(); ++m) {
                                        p3 = f2->getPoint(m-1);
                                        float parameter = plane.distance(p3->getCoordinate());
                                        if (fabs(parameter) > 1E-5) {
                                            addedge = true;
                                            break;
                                        }
                                    }
                                    if (addedge)
                                        break;
                                }
                            }
                            if (addedge) {
                                if (find(edges.begin(), edges.end(), edge) == edges.end()) {
                                    edges.push_back(edge);
                                    SurfIntersectionData sp;
                                    sp.point = p1->getCoordinate();
                                    sp.edge = edge;
                                    SurfIntersectionData ep;
                                    ep.point = p2->getCoordinate();
                                    ep.edge = edge;
                                    if (!edge->isCrease()) {
                                        sp.knuckle = p1->getVertexType() != SubdivisionPoint::svRegular;
                                        ep.knuckle = sp.knuckle;
                                    }
                                    else {
                                        sp.knuckle = p1->getVertexType() == SubdivisionPoint::svCorner;
                                        ep.knuckle = sp.knuckle;
                                    }
                                    segments.push_back(make_pair(sp, ep));
                                }
                            }
                        }
                    }
                    else if (fabs(side2) < 1E-5) {
                        SurfIntersectionData id(p2->getCoordinate());
                        id.knuckle = p2->getVertexType() != SubdivisionPoint::svRegular;
                        id.edge = edgeExists(p1, p2);
                        intarray.push_back(id);
                    }
                }
                p1 = p2;
                side1 = side2;
            }
            if (intarray.size() > 1) {
                if (intarray.front().edge == intarray.back().edge) {
                    if (intarray.front().point.distanceToPoint(intarray.back().point) < 1E-4) {
                        intarray.pop_back();
                    }
                    size_t k = 2;
                    while (k <= intarray.size()) {
                        if (intarray[k-1].edge == intarray[k-2].edge) {
                            if (intarray[k-1].point.distanceToPoint(intarray[k-2].point) < 1E-4) {
                                intarray.erase(intarray.begin()+k-1);
                            }
                            else
                                ++k;
                        }
                        else
                            ++k;
                    }
                    for (size_t l=2; l<=intarray.size(); ++l) {
                        segments.push_back(make_pair(intarray[l-2], intarray[l-1]));
                    }
                }
            }
        }
    }

    // convert segments into polylines
    Spline* spline = 0;
    SurfIntersectionData startp, endp;
    pair<SurfIntersectionData, SurfIntersectionData> segment;
    while (segments.size() > 0) {
        if (spline == 0) {
            spline = new Spline();
            destination.push_back(spline);
            segment = segments.back();
            spline->add(segment.first.point);
            spline->setKnuckle(0, segment.first.knuckle);
            spline->add(segment.second.point);
            spline->setKnuckle(1, segment.second.knuckle);
            startp = segment.first;
            endp = segment.second;
            segments.pop_back();
        }
        addedge = false;
        size_t j = 1;
        while (j <= segments.size()) {
            segment = segments[j-1];
            if (endp.edge == segment.first.edge) {
                addedge = endp.point.distanceToPoint(segment.first.point) < 1E-4;
                if (addedge) {
                    spline->setKnuckle(spline->numberOfPoints()-1, segment.first.knuckle
                                       || spline->isKnuckle(spline->numberOfPoints()-1));
                    spline->add(segment.second.point);
                    spline->setKnuckle(spline->numberOfPoints()-1, segment.second.knuckle);
                    endp = segment.second;
                }
            }
            else if (endp.edge == segment.second.edge) {
                addedge = endp.point.distanceToPoint(segment.second.point) < 1E-4;
                if (addedge) {
                    spline->setKnuckle(spline->numberOfPoints()-1, segment.second.knuckle
                                       || spline->isKnuckle(spline->numberOfPoints()-1));
                    spline->add(segment.first.point);
                    spline->setKnuckle(spline->numberOfPoints()-1, segment.first.knuckle);
                    endp = segment.first;
                }
            }
            else if (startp.edge == segment.first.edge) {
                addedge = startp.point.distanceToPoint(segment.first.point) < 1E-4;
                if (addedge) {
                    spline->setKnuckle(0, segment.first.knuckle || spline->isKnuckle(0));
                    spline->insert(0, segment.second.point);
                    spline->setKnuckle(0, segment.second.knuckle);
                    startp = segment.second;
                }
            }
            else if (startp.edge == segment.second.edge) {
                addedge = startp.point.distanceToPoint(segment.second.point) < 1E-4;
                if (addedge) {
                    spline->setKnuckle(0, segment.second.knuckle || spline->isKnuckle(0));
                    spline->insert(0, segment.first.point);
                    spline->setKnuckle(0, segment.first.knuckle);
                    startp = segment.first;
                }
            }
            else if (segment.first.edge == segment.second.edge) {
                // special case, edge lies entirely in plane
                // perform more extensive test to check whether the two edges
                // are possibly connected
                if (startp.edge->startPoint()->hasEdge(segment.first.edge)
                        || startp.edge->endPoint()->hasEdge(segment.first.edge)
                        || endp.edge->startPoint()->hasEdge(segment.first.edge)
                        || endp.edge->endPoint()->hasEdge(segment.first.edge)) {
                    addedge = endp.point.distanceToPoint(segment.first.point) < 1E-4;
                    if (addedge) {
                        spline->setKnuckle(spline->numberOfPoints()-1, segment.first.knuckle
                                           || spline->isKnuckle(spline->numberOfPoints()-1));
                        spline->add(segment.second.point);
                        spline->setKnuckle(spline->numberOfPoints()-1, segment.second.knuckle);
                        endp = segment.second;
                    }
                    else {
                        addedge = endp.point.distanceToPoint(segment.second.point) < 1E-4;
                        if (addedge) {
                            spline->setKnuckle(spline->numberOfPoints()-1, segment.second.knuckle
                                               || spline->isKnuckle(spline->numberOfPoints()-1));
                            spline->add(segment.first.point);
                            spline->setKnuckle(spline->numberOfPoints()-1, segment.first.knuckle);
                            endp = segment.first;
                        }
                        else {
                            addedge = startp.point.distanceToPoint(segment.first.point) < 1E-4;
                            if (addedge) {
                                spline->setKnuckle(0, segment.first.knuckle
                                                   || spline->isKnuckle(0));
                                spline->insert(0, segment.second.point);
                                spline->setKnuckle(0, segment.second.knuckle);
                                startp = segment.second;
                            } else {
                                addedge = startp.point.distanceToPoint(segment.second.point) < 1E-4;
                                if (addedge) {
                                    spline->setKnuckle(0, segment.second.knuckle
                                                       || spline->isKnuckle(0));
                                    spline->insert(0, segment.first.point);
                                    spline->setKnuckle(0, segment.first.knuckle);
                                    startp = segment.first;
                                }
                            }
                        }
                    }
                }
            }
            if (addedge) {
                segments.pop_back();
                j = 1;
                addedge = false;
            }
            else
                ++j;
        }
        if (!addedge)
            spline = 0;
    }
    if (destination.size() > 1) {
        JoinSplineSegments(0.01f, false, destination);
        for (size_t i=destination.size(); i>=1; --i) {
            // remove tiny fragments of very small length
            spline = destination[i-1];
            if (spline->numberOfPoints()>1) {
                float parameter = SquaredDistPP(spline->getMin(), spline->getMax());
                if (parameter < 1E-3) {
                    delete spline;
                    destination.erase(destination.begin()+i-1);
                }
            }
        }
    }
}

void SubdivisionSurface::draw(Viewport &vp)
{
    if (!isBuild())
        rebuild();
    if (vp.getViewportMode() != Viewport::vmWireFrame) {
        if (vp.getViewportMode() == Viewport::vmShadeGauss
                || vp.getViewportMode() == Viewport::vmShadeDevelopable) {
            if (!isGaussCurvatureCalculated())
                calculateGaussCurvature();
        }
    }
    for (size_t i=0; i<numberOfLayers(); ++i) {
        getLayer(i)->draw(vp);
    }
    if (showControlNet()) {
        for (size_t i=0; i<numberOfControlEdges(); ++i) {
            getControlEdge(i)->draw(false, vp);
        }
        for (size_t i=0; i<numberOfControlPoints(); ++i) {
            if (getControlPoint(i)->isVisible())
                getControlPoint(i)->draw(vp);
        }
    }
    for (size_t i=0; i<numberOfControlCurves(); ++i) {
        if (getControlCurve(i)->isVisible())
            getControlCurve(i)->draw(vp);
    }
}

SubdivisionEdge* SubdivisionSurface::edgeExists(SubdivisionPoint *p1, SubdivisionPoint *p2)
{
    SubdivisionEdge* result = 0;
    // if the edge exists then it must exist
    // in both the points, therefore only the point
    // with the smallest number of edges has to be checked
    if (p1->numberOfEdges() <= p2->numberOfEdges()) {
        for (size_t i=1; i<=p1->numberOfEdges(); ++i) {
            SubdivisionEdge* edge = p1->getEdge(i-1);
            if ((edge->startPoint() == p1 && edge->endPoint() == p2)
                    || (edge->startPoint() == p2 && edge->endPoint() == p1))
                return edge;
        }
    }
    else {
        for (size_t i=1; i<=p2->numberOfEdges(); ++i) {
            SubdivisionEdge* edge = p2->getEdge(i-1);
            if ((edge->startPoint() == p1 && edge->endPoint() == p2)
                    || (edge->startPoint() == p2 && edge->endPoint() == p1))
                return edge;
        }
    }
    return result;
}

SubdivisionControlEdge* SubdivisionSurface::controlEdgeExists(SubdivisionPoint *p1,
                                                              SubdivisionPoint *p2)
{
    SubdivisionEdge* result = edgeExists(p1, p2);
    if (result != 0)
        return dynamic_cast<SubdivisionControlEdge*>(result);
    return static_cast<SubdivisionControlEdge*>(0);
}

void SubdivisionSurface::sortEdges(vector<SubdivisionEdge*>& edges)
{
    if (edges.size() <= 1)
        return;
    SubdivisionEdge* edge1 = edges[0];
    for (size_t j=2; j<=edges.size(); ++j) {
        SubdivisionEdge* edge2 = edges[j-1];
        if (j == 2) {
            if (edge1->startPoint() == edge2->startPoint())
                edge1->swapData();
            else if (edge1->startPoint() == edge2->endPoint()) {
                edge1->swapData();
                edge2->swapData();
            }
            else if (edge1->endPoint() == edge2->endPoint()) {
                edge2->swapData();
            }
        }
        else {
            if (edge1->endPoint() == edge2->endPoint())
                edge2->swapData();
            if (edge1->endPoint() == edge2->startPoint()) {
                edge2->swapData();
                edge2->swapData();
            }
        }
        edge1 = edge2;
    }
}

// use the bool argument so that we don't get compile error, value passed in doesn't matter
vector<SubdivisionPoint*> SubdivisionSurface::sortEdges(bool /*always_true*/, vector<SubdivisionEdge*>& edges)
{
    vector<SubdivisionPoint*> points;
    if (edges.size() > 0) {
        sortEdges(edges);
        for (size_t i=1; i<=edges.size(); ++i) {
            if (i == 1)
                points.push_back(edges[i-1]->startPoint());
            points.push_back(edges[i-1]->endPoint());
        }
    }
    return points;
}

// use the bool argument so that we don't get compile error, value passed in doesn't matter
vector<SubdivisionControlPoint*> SubdivisionSurface::sortEdges(vector<SubdivisionControlEdge*>& edges)
{
    vector<SubdivisionControlPoint*> points;
    if (edges.size() > 0) {
        sortEdges(edges);
        for (size_t i=1; i<=edges.size(); ++i) {
            if (i == 1)
                points.push_back(dynamic_cast<SubdivisionControlPoint*>(edges[i-1]->startPoint()));
            points.push_back(dynamic_cast<SubdivisionControlPoint*>(edges[i-1]->endPoint()));
        }
    }
    return points;
}

void SubdivisionSurface::extractAllEdgeLoops(vector<vector<SubdivisionPoint*> >& destination)
{
    vector<SubdivisionEdge*> sourcelist;
    for (size_t i=1; i<=_edges.size(); ++i) {
        SubdivisionEdge* edge = _edges[i-1];
        if (edge->isCrease())
            sourcelist.push_back(edge);
    }
    sort(sourcelist.begin(), sourcelist.end());
    while (sourcelist.size() > 0) {
        SubdivisionEdge* edge = sourcelist.back();
        sourcelist.pop_back();
        vector<SubdivisionEdge*> loop;
        loop.push_back(edge);
        SubdivisionEdge* nextedge;
        // trace edge to back
        do {
            nextedge = edge->getPreviousEdge();
            if (nextedge != 0) {
                vector<SubdivisionEdge*>::iterator i = find(sourcelist.begin(), sourcelist.end(), nextedge);
                if (i != sourcelist.end()) {
                    loop.insert(loop.begin(), nextedge);
                    sourcelist.erase(i);
                    edge = nextedge;
                }
                else
                    nextedge = 0;
            }
        } while(nextedge != 0);

        vector<SubdivisionPoint*> points = sortEdges(true, loop);
        if (points.size() > 0)
            destination.push_back(points);
    }
}

void SubdivisionSurface::extractPointsFromFaces(vector<SubdivisionFace*>& selectedfaces,
                                                vector<SubdivisionControlPoint*>& points,
                                                size_t& lockedpoints)
{
    sort(selectedfaces.begin(), selectedfaces.end());
    lockedpoints = 0;
    bool ok;
    for (size_t i=1; i<=selectedfaces.size(); ++i) {
        SubdivisionFace* face = selectedfaces[i-1];
        for (size_t j=1; j<=face->numberOfPoints(); j++) {
            SubdivisionControlPoint* p = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j-1));
            if (find(points.begin(), points.end(), p) == points.end()) {
                ok = true;
                for (size_t k=1; k<=p->numberOfFaces(); ++k) {
                    if (find(selectedfaces.begin(), selectedfaces.end(), p->getFace(k-1)) == selectedfaces.end()) {
                        ok = false;
                        break;
                    }
                }
                if (ok) {
                    points.push_back(p);
                    if (p->isLocked())
                        ++lockedpoints;
                }
            }
        }
    }
}

void SubdivisionSurface::extractPointsFromSelection(vector<SubdivisionControlPoint*>& selectedpoints, size_t& lockedpoints)
{
    selectedpoints.reserve(4 * numberOfSelectedControlFaces()
                           + 2 * numberOfSelectedControlEdges()
                           + numberOfSelectedControlPoints() + 1);
    lockedpoints = 0;
    SubdivisionControlPoint* p;
    for (size_t i=1; i<=_sel_control_faces.size(); ++i) {
        SubdivisionControlFace* face = _sel_control_faces[i-1];
        for (size_t j=1; j<=face->numberOfPoints(); ++j) {
            p = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j-1));
            if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
                selectedpoints.push_back(p);
        }
    }
    for (size_t i=1; i<=_sel_control_edges.size(); ++i) {
        SubdivisionControlEdge* edge = _sel_control_edges[i-1];
        p = dynamic_cast<SubdivisionControlPoint*>(edge->startPoint());
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
        p = dynamic_cast<SubdivisionControlPoint*>(edge->endPoint());
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
    }
    for (size_t i=1; i<=_sel_control_points.size(); ++i) {
        p = _sel_control_points[i-1];
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
    }
    for (size_t i=1; i<=selectedpoints.size(); ++i) {
        p = _sel_control_points[i-1];
        if (p->isLocked())
            ++lockedpoints;
    }
}

void SubdivisionSurface::importFeFFile(vector<QString> &strings, size_t& lineno)
{
    size_t start;
    SubdivisionLayer* layer;

    // read layer information
    QString str = strings[++lineno].trimmed();
    start = 0;
    size_t n = ReadIntFromStr(lineno, str, start);
    for (size_t i=1; i<=n; ++i) {
        if (i > numberOfLayers())
            layer = addNewLayer();
        else
            layer = getLayer(i-1);
        layer->setDescription(strings[++lineno].trimmed());
        str = strings[++lineno].trimmed();
        start = 0;
        layer->setLayerID(ReadIntFromStr(lineno, str, start));  // layer id
        if (layer->getLayerID() > _last_used_layerID)
            _last_used_layerID = layer->getLayerID();
        int c = ReadIntFromStr(lineno, str, start); // layer color
        layer->setColor(QColorFromDXFIndex(c));
        layer->setVisible(ReadBoolFromStr(lineno, str, start));
        layer->setDevelopable(ReadBoolFromStr(lineno, str, start));
        layer->setSymmetric(ReadBoolFromStr(lineno, str, start));
        layer->setSymmetric(true);
        layer->setUseForIntersections(ReadBoolFromStr(lineno, str, start));
        layer->setUseInHydrostatics(ReadBoolFromStr(lineno, str, start));
        layer->setShowInLinesplan(ReadBoolFromStr(lineno, str, start));
        layer->setMaterialDensity(ReadFloatFromStr(lineno, str, start));
        layer->setThickness(ReadFloatFromStr(lineno, str, start));
    }
    emit changedLayerData();

    str = strings[++lineno].trimmed();
    start = 0;
    // read controlpoints
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlPoint* point = SubdivisionControlPoint::construct(this);
        _control_points.push_back(point);
        point->loadFromStream(lineno, strings);
    }

    str = strings[++lineno].trimmed();
    start = 0;
    // read controledges
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlEdge* edge = SubdivisionControlEdge::construct(this);
        _control_edges.push_back(edge);
        edge->loadFromStream(lineno, strings);
    }

    str = strings[++lineno].trimmed();
    start = 0;
    // read controlfaces
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlFace* face = SubdivisionControlFace::construct(this);
        _control_faces.push_back(face);
        str = strings[++lineno].trimmed();
        start = 0;
        size_t np = ReadIntFromStr(lineno, str, start);
        for (size_t j=1; j<=np; ++j) {
            size_t index = ReadIntFromStr(lineno, str, start);
            // attach controlfacet to controlpoints
            face->addPoint(_control_points[index]);
        }
        // attach control face to the already existing control edges
        SubdivisionControlPoint* p1 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(face->numberOfPoints()-1));
        for (size_t j=1; j<=face->numberOfPoints(); ++j) {
            SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j-1));
            SubdivisionControlEdge* edge = controlEdgeExists(p1, p2);
            if (edge != 0)
                edge->addFace(face);
            else
                // BUGBUG: usestring 201
                throw runtime_error("");
            p1 = p2;
        }
        // read layer index
        size_t index = ReadIntFromStr(lineno, str, start);
        layer = _layers[index];
        layer->addControlFace(face);
    }
    setBuild(false);
    _initialized = true;
    emit changedLayerData();
    setActiveLayer(0);
    emit changeActiveLayer();
}

void SubdivisionSurface::importGrid(coordinate_grid_t& points, SubdivisionLayer* layer)
{
    size_t rows = points.size();
    size_t cols = points[0].size();

    control_grid_t grid(rows);
    for (size_t i=1; i<=rows; ++i) {
        vector<SubdivisionControlPoint*> row(cols);
        for (size_t j=1; j<=cols; ++j)
            row.push_back(addControlPoint(points[i-1][j-1]));
        grid.push_back(row);
    }

    vector<SubdivisionControlPoint*> facepoints;
    for (size_t i=2; i<=rows; ++i) {
        for (size_t j=2; j<=cols; ++j) {
            facepoints.clear();
            if (find(facepoints.begin(), facepoints.end(), grid[i-1][j-1]) == facepoints.end())
                facepoints.push_back(grid[i-1][j-1]);
            if (find(facepoints.begin(), facepoints.end(), grid[i-1][j-2]) == facepoints.end())
                facepoints.push_back(grid[i-1][j-2]);
            if (find(facepoints.begin(), facepoints.end(), grid[i-2][j-2]) == facepoints.end())
                facepoints.push_back(grid[i-2][j-2]);
            if (find(facepoints.begin(), facepoints.end(), grid[i-2][j-1]) == facepoints.end())
                facepoints.push_back(grid[i-2][j-1]);
            if (facepoints.size() >= 3) {
                if (layer != 0)
                    addControlFace(facepoints, true, layer);
                else
                    addControlFace(facepoints, true);
            }
        }
    }
    // set crease edges
    for (size_t i=2; i<=cols; ++i) {
        SubdivisionEdge* edge = edgeExists(grid[0][i-2], grid[0][i-1]);
        if (edge != 0)
            edge->setCrease(true);
        edge = edgeExists(grid[rows-1][i-2], grid[rows-1][i-1]);
        if (edge != 0)
            edge->setCrease(true);
    }
    for (size_t i=2; i<=rows; ++i) {
        SubdivisionEdge* edge = edgeExists(grid[i-2][0], grid[i-1][0]);
        if (edge != 0)
            edge->setCrease(true);
        edge = edgeExists(grid[i-2][cols-1], grid[i-1][cols-1]);
        if (edge != 0)
            edge->setCrease(true);
    }
    // set cornerpoints
    for (size_t i=1; i<=rows; ++i) {
        for (size_t j=1; j<=cols; ++j) {
            if (grid[i-1][j-1]->numberOfFaces() < 2)
                grid[i-1][j-1]->setVertexType(SubdivisionControlPoint::svCorner);
        }
    }
}

void SubdivisionSurface::initialize(size_t point_start, size_t edge_start)
{
    // identify all border edges
    if (edge_start <= numberOfControlEdges()) {
        for (size_t i=edge_start; i<=numberOfControlEdges(); ++i) {
            SubdivisionControlEdge* edge = _control_edges[i-1];
            if (edge->numberOfFaces() == 0) {
                edge->startPoint()->deleteEdge(edge);
                edge->endPoint()->deleteEdge(edge);
                edge->setCrease(false);
            }
            else if (edge->numberOfFaces() != 2)
                edge->setCrease(true);
        }
    }
    for (size_t i=point_start; i<=numberOfControlPoints(); ++i) {
        if(_control_points[i-1]->numberOfFaces() < 2)
            _control_points[i-1]->setVertexType(SubdivisionControlPoint::svCorner);
    }
    _initialized = true;
}

bool SubdivisionSurface::intersectPlane(const Plane& plane, bool hydrostatics_layers_only, vector<Spline*>& destination)
{
    QVector3D min, max;
    bool result = false;
    if (!isBuild())
        rebuild();
    if (!plane.intersectsBox(_min, _max))
        return result;
    vector<SubdivisionControlFace*> intersectedfaces;
    for (size_t i=1; i<=numberOfLayers(); ++i) {
        SubdivisionLayer* layer = getLayer(i-1);
        bool use_layer;
        if (hydrostatics_layers_only)
            use_layer = layer->useInHydrostatics();
        else
            use_layer = layer->useForIntersections();
        if (use_layer) {
            for (size_t j=1; j<=layer->numberOfFaces(); ++j) {
                SubdivisionControlFace* ctrlface = layer->getFace(j-1);
                min = ctrlface->getMin();
                max = ctrlface->getMax();
                if (plane.intersectsBox(min, max))
                    intersectedfaces.push_back(ctrlface);
            }
        }
    }
    calculateIntersections(plane, intersectedfaces, destination);
    result = destination.size() > 0;
    return result;
}

void SubdivisionSurface::insertPlane(const Plane& plane, bool add_curves)
{
    vector<SubdivisionControlPoint*> points;
    size_t i = 1;
    while (i <= numberOfControlEdges()) {
        SubdivisionControlEdge* edge = _control_edges[i-1];
        if (edge->isVisible()) {
            float s1 = plane.distance(edge->startPoint()->getCoordinate());
            float s2 = plane.distance(edge->endPoint()->getCoordinate());
            if ((s1 <-1E-5 && s2 > 1E-5) || (s1>1E-5 && s2 < -1E-5)) {
                float t;
                if (s1 == s2)
                    t = 0.5;
                else
                    t = -s1 / (s2 - s1);
                QVector3D p = edge->startPoint()->getCoordinate()
                        + t * (edge->endPoint()->getCoordinate() - edge->startPoint()->getCoordinate());
                SubdivisionControlPoint* newp = edge->insertControlPoint(p);
                points.push_back(newp);
            }
        }
        ++i;
    }
    if (points.size() > 0) {
        // try to find multiple points belonging to the same face and insert an edge
        i = 1;
        sort(points.begin(), points.end());
        vector<SubdivisionControlEdge*> edges;
        while (i <= points.size()) {
            SubdivisionControlPoint* p1 = points[i-1];
            size_t j = 1;
            while (j <= p1->numberOfFaces()) {
                SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(p1->getFace(j-1));
                size_t k = 1;
                bool inserted = false;
                while (k <= face->numberOfPoints()) {
                    SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(k-1));
                    if (p1 != p2 && find(points.begin(), points.end(), p2) != points.end()) {
                        // this is also a new point, first check if an edge already exists between p1 and p2
                        if (edgeExists(p1, p2) == 0) {
                            inserted = true;
                            SubdivisionControlEdge* edge = face->insertControlEdge(p1, p2);
                            edges.push_back(edge);
                        }
                    }
                    ++k;
                }
                if (!inserted)
                    ++j;
            }
            ++i;
        }
        if (add_curves) {
            points.clear();
            vector<vector<SubdivisionControlPoint*> > sortededges;
            isolateEdges(edges, sortededges);
            for (size_t i=1; i<=sortededges.size(); ++i) {
                vector<SubdivisionControlPoint*>& points = sortededges[i-1];
                if (points.size() > 1) {
                    SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(this);
                    addControlCurve(curve);
                    for (size_t j=1; j<=points.size(); ++j) {
                        SubdivisionControlPoint* p1 = points[j-1];
                        curve->addPoint(p1);
                        if (j > 1) {
                            SubdivisionControlEdge* edge = controlEdgeExists(curve->getControlPoint(j-2), curve->getControlPoint(j-1));
                            if (edge != 0)
                                edge->setCurve(curve);
                        }
                    }
                }
            }
        }
    }
}

void SubdivisionSurface::isolateEdges(vector<SubdivisionControlEdge *> &source,
                                      vector<vector<SubdivisionControlPoint *> > &sorted)
{
    // try to isolate individual (closed) sets of edges
    vector<SubdivisionControlEdge*> tmpedges;
    while (source.size() > 0) {
        SubdivisionControlEdge* edge = source[0];
        source.erase(source.begin());
        bool findmore = true;
        tmpedges.clear();
        tmpedges.push_back(edge);
        while (source.size() > 0 && findmore) {
            findmore = false;
            for (size_t i=1; i<=source.size(); ++i) {
                SubdivisionControlEdge* edge2 = source[i-1];
                // compare at start
                edge = tmpedges.front();
                if ((edge2->startPoint() == edge->startPoint())
                        || (edge2->startPoint() == edge->endPoint())
                        || (edge2->endPoint() == edge->startPoint())
                        || (edge2->endPoint() == edge->endPoint())) {
                    tmpedges.insert(tmpedges.begin(), edge2);
                    source.erase(source.begin()+i-1);
                    findmore = true;
                    break;
                }
                else {
                    edge = tmpedges.back();
                    if ((edge2->startPoint() == edge->startPoint())
                            || (edge2->startPoint() == edge->endPoint())
                            || (edge2->endPoint() == edge->startPoint())
                            || (edge2->endPoint() == edge->endPoint())) {
                        tmpedges.push_back(edge2);
                        source.erase(source.begin()+i-1);
                        findmore = true;
                        break;
                    }
                }
            }
        }
        if (tmpedges.size() > 0) {
            //sort all found edges in correct order
            vector<SubdivisionControlPoint*> tmppoints = sortEdges(tmpedges);
            if (tmppoints.size() > 0)
                sorted.push_back(tmppoints);
        }
    }
}

void SubdivisionSurface::loadBinary(FileBuffer &source)
{
    // first load layerdata
    size_t n;
    source.load(n);
    if (n != 0) {
        // delete current layers and load new ones
        _layer_pool.release_memory();
        _layers.clear();
        _layers.reserve(n);
        for (size_t i=1; i<=n; ++i) {
            SubdivisionLayer* layer = addNewLayer();
            layer->loadBinary(source);
        }
    }
    else {
        // no layers in the file, so keep the current default one
    }
    emit changedLayerData();
    // read index of active layer
    source.load(n);
    setActiveLayer(_layers[n]);
    // read control points
    source.load(n);
    _control_points.reserve(n);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlPoint* point = SubdivisionControlPoint::construct(this);
        _control_points.push_back(point);
        point->load_binary(source);
    }
    // read control edges
    source.load(n);
    _control_edges.reserve(n);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlEdge* edge = SubdivisionControlEdge::construct(this);
        _control_edges.push_back(edge);
        edge->load_binary(source);
    }
    if (source.version() >= fv195) {
        // load control curves
        source.load(n);
        _control_curves.reserve(n);
        for (size_t i=1; i<=n; ++i) {
            SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(this);
            _control_curves.push_back(curve);
            curve->loadBinary(source);
        }
    }
    // read control faces
    source.load(n);
    _control_faces.reserve(n);
    for (size_t i=1; i<=n; ++i) {
        SubdivisionControlFace* face = SubdivisionControlFace::construct(this);
        _control_faces.push_back(face);
        face->loadBinary(source);
    }
    setBuild(false);
    _initialized = true;
    emit changedLayerData();
    emit changeActiveLayer();
}

void SubdivisionSurface::loadFromStream(size_t& lineno, vector<QString>& strings)
{
    // BUGBUG: not implemented
}

void SubdivisionSurface::loadVRMLFile(const QString &filename)
{
    // BUGBUG: not implemented
}

void SubdivisionSurface::rebuild()
{
    if (!_initialized)
        initialize(1,1);
    if (numberOfControlFaces() > 0) {
        for (size_t i=1; i<=numberOfControlCurves(); ++i) {
            SubdivisionControlCurve* curve = _control_curves[i-1];
            if (_current_subdiv_level == 0)
                curve->resetDivPoints();
        }
        _build = true;
        while (_current_subdiv_level < _desired_subdiv_level)
            subdivide();
        for (size_t i=1; i<=numberOfControlFaces(); ++i) {
            getControlFace(i-1)->calcExtents();
            if (i == 1) {
                _min = getControlFace(i-1)->getMin();
                _max = getControlFace(i-1)->getMax();
            }
            else {
                MinMax(getControlFace(i-1)->getMin(), _min, _max);
                MinMax(getControlFace(i-1)->getMax(), _min, _max);
            }
        }
        for (size_t i=1; i<=numberOfControlCurves(); ++i) {
            SubdivisionControlCurve* curve= getControlCurve(i-1);
            curve->getSpline()->clear();
            for (size_t j=1; j<=curve->numberOfSubdivPoints(); ++j) {
                SubdivisionPoint* point = curve->getSubdivPoint(j-1);
                curve->getSpline()->add(point->getCoordinate());
                if (j > 1 && j < curve->numberOfSubdivPoints()) {
                    if (point->getVertexType() == SubdivisionPoint::svCorner)
                        curve->getSpline()->setKnuckle(j-1, true);
                    else {
                        SubdivisionEdge* edge1 = edgeExists(curve->getSubdivPoint(j-2), curve->getSubdivPoint(j-1));
                        SubdivisionEdge* edge2 = edgeExists(curve->getSubdivPoint(j-1), curve->getSubdivPoint(j));
                        if (!edge1->isCrease() && !edge2->isCrease())
                            curve->getSpline()->setKnuckle(j-1, point->getVertexType() == SubdivisionPoint::svCrease);
                    }
                }
                curve->setBuild(true);
            }
        }
    }
    else if (numberOfControlPoints() > 0) {
        for (size_t i=1; i<=numberOfControlPoints(); ++i) {
            if (i == 1) {
                _min = getControlPoint(i-1)->getCoordinate();
                _max = _min;
            }
            else
                MinMax(getControlPoint(i-1)->getCoordinate(), _min, _max);
        }
    }
    else {
        _min = ZERO;
        _max = ONE;
    }
}

void SubdivisionSurface::saveBinary(FileBuffer &destination)
{
    // first save layerdata
    destination.add(numberOfLayers());
    for (size_t i=1; i<=numberOfLayers(); ++i)
        getLayer(i-1)->saveBinary(destination);
    // save index of active layer
    destination.add(getActiveLayer()->getLayerIndex());
    // first sort controlpoints for faster access of function
    sort(_control_points.begin(), _control_points.end());
    destination.add(numberOfControlPoints());
    for (size_t i=1; i<=numberOfControlPoints(); ++i)
        getControlPoint(i-1)->save_binary(destination);
    // save control edges
    destination.add(numberOfControlEdges());
    for (size_t i=1; i<=numberOfControlEdges(); ++i)
        getControlEdge(i-1)->save_binary(destination);
    if (destination.version() >= fv195) {
        destination.add(numberOfControlCurves());
        for (size_t i=1; i<=numberOfControlCurves(); ++i)
            getControlCurve(i-1)->saveBinary(destination);
    }
    // save control faces
    destination.add(numberOfControlFaces());
    for (size_t i=1; i<=numberOfControlFaces(); ++i)
        getControlFace(i-1)->saveBinary(destination);
}

void SubdivisionSurface::saveToStream(vector<QString>& strings)
{
    // BUGBUG: unimplemented
}

void SubdivisionSurface::selectionDelete()
{
    // first controlcurves, then faces, edges, and points
    size_t i = numberOfSelectedControlCurves();
    while (i >= 1) {
        delete getControlCurve(i-1);
        --i;
        if (i > numberOfSelectedControlCurves())
            i = numberOfSelectedControlCurves();
    }
    i = numberOfSelectedControlFaces();
    while (i >= 1) {
        delete getControlFace(i-1);
        --i;
        if (i > numberOfSelectedControlFaces())
            i = numberOfSelectedControlFaces();
    }
    i = _sel_control_edges.size();
    while (i >= 1) {
        deleteControlEdge(_sel_control_edges[i-1]);
        --i;
        if (i > _sel_control_edges.size())
            i = _sel_control_edges.size();
    }
    // delete selected points
    i = _sel_control_points.size();
    while (i >= 1) {
        deleteControlPoint(_sel_control_points[i-1]);
        --i;
        if (i > _sel_control_points.size())
            i = _sel_control_points.size();
    }
    setBuild(false);
}

void SubdivisionSurface::subdivide()
{
    if (numberOfControlFaces() < 1)
        return;
    ++_current_subdiv_level;
    vector<SubdivisionEdge*> newedgelist(static_cast<size_t>(pow(2.0f, _current_subdiv_level)));
    size_t number = numberOfFaces();

    // create the list with new facepoints and a reference to the original face
    vector<pair<SubdivisionFace*,SubdivisionPoint*> > facepoints;
    // create the list with new edgepoints and a reference to the original edge
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> > edgepoints;
    // create the list with new vertexpoints and a reference to the original vertex
    vector<pair<SubdivisionPoint*,SubdivisionPoint*> > vertexpoints;

    if (number == 0) {
        facepoints.reserve(numberOfControlFaces());
        for (size_t i=1; i<=numberOfControlFaces(); ++i) {
            facepoints.push_back(make_pair(getControlFace(i-1), getControlFace(i-1)->calculateFacePoint()));
        }
    }
    else {
        facepoints.reserve(4*numberOfControlFaces());
        for (size_t i=1; i<=numberOfControlFaces(); ++i) {
            SubdivisionControlFace* ctrlface = getControlFace(i-1);
            for (size_t j=1; j<=ctrlface->numberOfChildren(); ++j)
                facepoints.push_back(make_pair(ctrlface->getChild(j-1), ctrlface->getChild(j-1)->calculateFacePoint()));
            for (size_t j=1; j<=ctrlface->numberOfEdges(); ++j)
                edgepoints.push_back(make_pair(ctrlface->getEdge(j-1), ctrlface->getEdge(j-1)->calculateEdgePoint()));
        }
    }
    // calculate other edgepoints
    edgepoints.reserve(edgepoints.size() + numberOfEdges());
    for (size_t i=1; i<=numberOfEdges(); ++i) {
        edgepoints.push_back(make_pair(getEdge(i-1), getEdge(i-1)->calculateEdgePoint()));
    }
    // calculate vertex points
    vertexpoints.reserve(vertexpoints.size() + numberOfPoints());
    for (size_t i=1; i<=numberOfPoints(); ++i)
        vertexpoints.push_back(make_pair(getPoint(i-1), getPoint(i-1)->calculateVertexPoint()));
    // sort the points for faster access
    // vertexpoints.sort
    // edgepoints.sort
    // facepoints.sort

    // finally create the refined mesh over the newly created vertexpoints, edgepoints, and facepoints
    vector<SubdivisionEdge*> interioredges;
    vector<SubdivisionFace*> dest;
    for (size_t i=1; i<=numberOfControlFaces(); ++i) {
        interioredges.clear();
        dest.clear();
        getControlFace(i-1)->subdivide(this, vertexpoints, edgepoints, facepoints, interioredges, newedgelist, dest);
    }

    // BUGBUG: cleanup old mesh
    for (size_t i=1; i<=_edges.size(); ++i) {
        delete _edges[i-1];
    }
    _edges.clear();
    _edges = newedgelist;
    _points.clear();
//    _point_pool.release_memory();
    _points.reserve(vertexpoints.size() + edgepoints.size() + facepoints.size());
    for (size_t i=1; i<=vertexpoints.size(); ++i)
        if (vertexpoints[i-1].first != 0)
            _points.push_back(vertexpoints[i-1].second);
    for (size_t i=1; i<=edgepoints.size(); ++i)
        if (edgepoints[i-1].first != 0)
            _points.push_back(edgepoints[i-1].second);
    for (size_t i=1; i<=facepoints.size(); ++i)
        if (facepoints[i-1].first != 0)
            _points.push_back(facepoints[i-1].second);
    // perform averaging procedure to smooth the new mesh
    vector<QVector3D> tmppoints(_points.size());
    for (size_t i=1; i<=_points.size(); ++i) {
        tmppoints.push_back(_points[i-1]->averaging());
    }
    for (size_t i=1; i<=_points.size(); ++i) {
        _points[i-1]->setCoordinate(tmppoints[i-1]);
    }
}

void SubdivisionSurface::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionSurface ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionSurface::priv_dump(ostream& os, const char* prefix) const
{
    os << prefix << " Control Points (" << _control_points.size() << ")\n";
    char * np = new char[strlen(prefix) + 3];
    memcpy(np, prefix, strlen(prefix));
    memcpy(np+strlen(prefix), "  ", 2);
    np[strlen(prefix)+2] = 0;
    if (g_surface_verbose) {
        for (size_t i=0; i<_control_points.size(); ++i) {
            _control_points[i]->dump(os, np);
            os << "\n";
        }
    }
    os << prefix << " Control Edges (" << _control_edges.size() << ")\n";
    os << prefix << " Control Faces (" << _control_faces.size() << ")\n";
    os << prefix << " Control Curves (" << _control_curves.size() << ")\n";
    os << prefix << " Points (" << _points.size() << ")\n";
    os << prefix << " Edges (" << _edges.size() << ")\n";
    delete [] np;
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionSurface& surface)
{
    surface.dump(os);
    return os;
}
