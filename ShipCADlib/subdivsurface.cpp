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
      getLayer(i-1).extents(min, max);
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

void SubdivisionSurface::extrudeEdges(vector<SubdivisionControlEdge*>& edges,
				      const QVector3D& direction)
{
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
