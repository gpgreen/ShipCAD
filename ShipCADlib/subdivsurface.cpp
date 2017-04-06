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

#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "plane.h"
#include "spline.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "viewportview.h"
#include "filebuffer.h"
#include "utility.h"
#include "version.h"
#include "controlfacegrid.h"
#include "pointgrid.h"

using namespace std;
using namespace ShipCAD;

static int sDecimals = 4;

bool ShipCAD::g_surface_verbose = true;

//////////////////////////////////////////////////////////////////////////////////////

DeleteElementsCollection::DeleteElementsCollection()
    : _suppress(false)
{
    // does nothing
}

void DeleteElementsCollection::clear()
{
    points.clear();
    edges.clear();
    faces.clear();
    curves.clear();
    splines.clear();
}

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionSurface::SubdivisionSurface()
    : Entity(),
      _show_control_net(true), _initialized(false), _show_interior_edges(false),
      _draw_mirror(false), _shade_under_water(false), _show_normals(true),
      _show_curvature(true), _show_control_curves(true),
      _subdivision_mode(fmQuadTriangle), _desired_subdiv_level(1),
      _current_subdiv_level(-1), _control_point_size(2),
      _curvature_scale(0.25), _min_gaus_curvature(0), _max_gaus_curvature(0),
      _main_frame_location(1E10),
      _crease_color(Qt::green), _crease_edge_color(Qt::red),
      _underwater_color(Qt::gray),
      _edge_color(Qt::darkGray), _selected_color(Qt::yellow),
      _crease_point_color(Qt::green), _regular_point_color(QColor(0xc0, 0xc0, 0xc0)),
      _corner_point_color(QColor(0, 255, 255)), _dart_point_color(QColor(0xc0, 0x80, 0x00)),
      _layer_color(QColor(0, 255, 255)), _normal_color(QColor(0xc0, 0xc0, 0xc0)), _leak_color(Qt::red),
      _curvature_color(Qt::white), _control_curve_color(Qt::red),
      _zebra_color(Qt::black), _last_used_layerID(0), _active_layer(0),
      _cpoint_pool(sizeof(SubdivisionControlPoint)),
      _cedge_pool(sizeof(SubdivisionControlEdge)),
      _cface_pool(sizeof(SubdivisionControlFace)),
      _ccurve_pool(sizeof(SubdivisionControlCurve)),
      _layer_pool(sizeof(SubdivisionLayer)),
      _point_pool(sizeof(SubdivisionPoint)),
      _edge_pool(sizeof(SubdivisionEdge)),
      _face_pool(sizeof(SubdivisionFace)),
      _spline_pool(sizeof(Spline))
{
    // construct default layer
	addNewLayer();
}

SubdivisionSurface::~SubdivisionSurface()
{
    // clear the pools
    _ccurve_pool.clear();
    _cface_pool.clear();
    _cedge_pool.clear();
    _cpoint_pool.clear();
    _layer_pool.clear();
    _edge_pool.clear();
    _point_pool.clear();
    _spline_pool.clear();
}

void SubdivisionSurface::deleteElementsCollection()
{
    if (_deleted.isSuppressed()) {
        cout << "Deleting elements suppressed" << endl;
        return;
    }
    
    // order of deletion doesn't matter, this is only to remove from pool, not remove
    // from structure, should have already been done
    size_t pt,edge,fc,crv,spl;
    pt = 0;
    for (set<SubdivisionControlPoint*>::iterator i=_deleted.points.begin(); i!=_deleted.points.end(); i++) {
        _cpoint_pool.del(*i);
        pt++;
    }
    edge = 0;
    for (set<SubdivisionControlEdge*>::iterator i=_deleted.edges.begin(); i!=_deleted.edges.end(); i++) {
        _cedge_pool.del(*i);
        edge++;
    }
    fc = 0;
    for (set<SubdivisionControlFace*>::iterator i=_deleted.faces.begin(); i!=_deleted.faces.end(); i++) {
        _cface_pool.del(*i);
        fc++;
    }
    crv = 0;
    for (set<SubdivisionControlCurve*>::iterator i=_deleted.curves.begin(); i!=_deleted.curves.end(); i++) {
        _ccurve_pool.del(*i);
        crv++;
    }
    spl = 0;
    for (set<Spline*>::iterator i=_deleted.splines.begin(); i!=_deleted.splines.end(); i++) {
        _spline_pool.del(*i);
        spl++;
    }
    _deleted.clear();
    cout << "Del pts:" << pt << " edges:" << edge << " faces:" << fc << " curves:"
         << crv << " splines:" << spl << endl;
    cout << "Ttl pts:" << _control_points.size() << " edges:" << _control_edges.size()
         << " faces:" << _control_faces.size() << " curves:" << _control_curves.size()
         << endl;
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
    for (size_t i=0; i<numberOfControlEdges(); ++i) {
        SubdivisionControlEdge* edge = getControlEdge(i);
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
        for (size_t i=0; i<numberOfControlPoints(); ++i) {
            SubdivisionControlPoint* point = getControlPoint(i);
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
        removeControlPoint(point);
        point->removePoint();
        _deleted.points.insert(point);
    }
    setBuild(false);
}

void SubdivisionSurface::collapseControlPoint(SubdivisionControlPoint* point)
{
    _deleted.suppressDelete(true);
    point->collapse();
    deleteControlPoint(point);
    setBuild(false);
    _deleted.suppressDelete(false);
    deleteElementsCollection();
}

SubdivisionLayer* SubdivisionSurface::addNewLayer()
{
    SubdivisionLayer* result = SubdivisionLayer::construct(this);
    _layers.push_back(result);
    result->setLayerID(requestNewLayerID());
    setActiveLayer(result);
    return result;
}

static SubdivisionControlFace* privGetControlFace(SubdivisionPoint* p1,
                                                  SubdivisionPoint* p2,
                                                  SubdivisionPoint* p3,
                                                  SubdivisionPoint* p4)
{
    SubdivisionFace* face;
    SubdivisionControlFace* cface = 0;
    for (size_t i=0; i<p1->numberOfFaces(); ++i) {
        face = p1->getFace(i);
        if (p2->hasFace(face) && p3->hasFace(face) && p4->hasFace(face)) {
            cface = dynamic_cast<SubdivisionControlFace*>(face);
            break;
        }
    }
    return cface;
}

static void privFindAttachedFaces(vector<SubdivisionControlFace*>& found_list,
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
                        privFindAttachedFaces(found_list, todo_list, f);
                    }
                }
            }
        }
        p1 = p2;
    }
}

static SubdivisionControlFace* privFindCornerFace(vector<SubdivisionControlFace*>& ctrlfaces)
{
    SubdivisionControlFace* result = 0;
    for (size_t i=0; i<ctrlfaces.size(); i++) {
        SubdivisionControlFace* face = ctrlfaces[i];
        size_t j = 0;
        while (j < face->numberOfPoints() && result == 0) {
            SubdivisionPoint* p = face->getPoint(j++);
            int nfaces = 1;
            for (size_t k=0; k<p->numberOfFaces(); ++k) {
                SubdivisionControlFace* face2 = dynamic_cast<SubdivisionControlFace*>(p->getFace(k));
                if (face2 != face) {
                    vector<SubdivisionControlFace*>::iterator index = find(
                        ctrlfaces.begin(), ctrlfaces.end(), face2);
                    if (index != ctrlfaces.end()) {
                        nfaces++;
                        break;
                    }
                }
            }
            if (nfaces == 1)
                result = face;
        }
    }
    if (result == 0)
        result = ctrlfaces.back();
    return result;
}

static bool privValidFace(SubdivisionFace* face,
                          vector<SubdivisionFace*>& faces,
                          vector<SubdivisionFace*>& tmpfaces)
{
    bool result = false;
    if (face->numberOfPoints() == 4) {
        vector<SubdivisionFace*>::iterator index = find(faces.begin(), faces.end(), face);
        if (index != faces.end()) {
            result = true;
            vector<SubdivisionFace*>::iterator tindex = find(
                tmpfaces.begin(), tmpfaces.end(), face);
            if (tindex != tmpfaces.end())
                return false;
            if (tmpfaces.size() > 0) {
                SubdivisionFace* tmp = tmpfaces.back();
                int n = 0;
                for (size_t j=0; j<face->numberOfPoints(); ++j) {
                    if (tmp->indexOfPoint(face->getPoint(j)) != tmp->numberOfPoints())
                        n++;
                }
                result = n > 1;
            }
        }
    }
    return result;
}

static bool privValidFace(SubdivisionControlFace* face,
                          vector<SubdivisionControlFace*>& faces,
                          vector<SubdivisionControlFace*>& tmpfaces)
{
    bool result = false;
    if (face->numberOfPoints() == 4) {
        vector<SubdivisionControlFace*>::iterator index = find(
            faces.begin(), faces.end(), face);
        if (index != faces.end()) {
            result = true;
            vector<SubdivisionControlFace*>::iterator tindex = find(
                tmpfaces.begin(), tmpfaces.end(), face);
            if (tindex != tmpfaces.end())
                return false;
            if (tmpfaces.size() > 0) {
                SubdivisionControlFace* tmp = tmpfaces.back();
                int n = 0;
                for (size_t j=0; j<face->numberOfPoints(); ++j) {
                    if (tmp->indexOfPoint(face->getPoint(j)) != tmp->numberOfPoints())
                        n++;
                }
                result = n > 1;
            }
        }
    }
    return result;
}

void SubdivisionSurface::doAssembleSpecial(PointGrid& grid,
                                           size_t& cols, size_t& rows,
                                           assemble_mode_t mode,
                                           vector<SubdivisionControlFace*>& checkfaces,
                                           vector<SubdivisionControlFace*>& faces)
{
    bool searchbottom = true;
    bool searchtop = true;
    bool searchleft = true;
    bool searchright = true;
    int counter = 0;
    if (mode == amNURBS) {
        if (!grid.getPoint(0,0)->isRegularNURBSPoint(checkfaces)) {
            searchleft = false;
            searchtop = false;
        }
        if (!grid.getPoint(rows-1,0)->isRegularNURBSPoint(checkfaces)) {
            searchleft = false;
            searchbottom = false;
        }
        if (!grid.getPoint(rows-1,cols-1)->isRegularNURBSPoint(checkfaces)) {
            searchright = false;
            searchbottom = false;
        }
        if (!grid.getPoint(0,cols-1)->isRegularNURBSPoint(checkfaces)) {
            searchright = false;
            searchtop = false;
        }
    }
    SubdivisionControlFace* face = 0;
    SubdivisionEdge* edge1, *edge2;
    while ((searchbottom || searchtop || searchright || searchleft) && faces.size() > 0) {
        if (++counter > 4)
            counter = 1;
        vector<SubdivisionControlFace*> tmpfaces;
        if (counter == 1 && searchbottom) {
            for (size_t i=2; i<=cols; i++) {
                SubdivisionEdge* edge = edgeExists(
                    grid.getPoint(rows-1,i-2),
                    grid.getPoint(rows-1,i-1));
                if (edge != 0 && !edge->isCrease()) {
                    for (size_t j=0; j<edge->numberOfFaces(); j++) {
                        face = dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                        if (privValidFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i-1)
                    break;
            }
            if (tmpfaces.size() == cols - 1) {
                // search was successful
                grid.setRows(rows+1);
                for (size_t i=1; i<=tmpfaces.size(); i++) {
                    face = tmpfaces[i-1];
                    vector<SubdivisionControlFace*>::iterator index = find(
                        faces.begin(), faces.end(), face);
                    if (index != faces.end())
                        faces.erase(index);
                    size_t idx = face->indexOfPoint(grid.getPoint(rows-1,i));
                    idx = (idx + 1) % face->numberOfPoints();
                    if (face->getPoint(idx) == grid.getPoint(rows-1,i-1)) {
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(rows, i-1, face->getPoint(idx));
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(rows, i, face->getPoint(idx));
                    } else {
                        idx = face->indexOfPoint(grid.getPoint(rows-1,i-1));
                        idx = (idx + 1) % face->numberOfPoints();
                        if (face->getPoint(idx) == grid.getPoint(rows-1,i)) {
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(rows, i, face->getPoint(idx));
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(rows, i-1, face->getPoint(idx));
                        }
                    }
                }
                // check if the boundary edges do not switch between crease or not crease
                bool dataOK = true;
                if (mode == amNURBS) {
                    edge1 = edgeExists(grid.getPoint(rows, 0),
                                       grid.getPoint(rows-1, 0));
                    edge2 = edgeExists(grid.getPoint(rows-1, 0),
                                       grid.getPoint(rows-2, 0));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    edge1 = edgeExists(grid.getPoint(rows,cols-1),
                                       grid.getPoint(rows-1, cols-1));
                    edge2 = edgeExists(grid.getPoint(rows-1,cols-1),
                                       grid.getPoint(rows-2, cols-1));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    if (!dataOK) {
                        // do not add the current row
                        for (size_t i=0; i<tmpfaces.size(); i++)
                            faces.push_back(tmpfaces[i]);
                        searchbottom = false;
                    } else {
                        rows++;
                    }
                } else {
                    searchbottom = false;
                }
            }
        } else if (counter == 2 && searchright) {
            for (size_t i=2; i<=rows; i++) {
                SubdivisionEdge* edge = edgeExists(
                    grid.getPoint(i-1,cols-1),
                    grid.getPoint(i-2,cols-1));
                if (edge != 0 && !edge->isCrease()) {
                    for (size_t j=0; j<edge->numberOfFaces(); j++) {
                        face = dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                        if (privValidFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i-1)
                    break;
            }
            if (tmpfaces.size() == rows - 1) {
                // search was successful
                grid.setCols(cols+1);
                for (size_t i=1; i<=tmpfaces.size(); i++) {
                    face = tmpfaces[i-1];
                    vector<SubdivisionControlFace*>::iterator index = find(
                        faces.begin(), faces.end(), face);
                    if (index != faces.end())
                        faces.erase(index);
                    size_t idx = face->indexOfPoint(grid.getPoint(i-1,cols-1));
                    idx = (idx + 1) % face->numberOfPoints();
                    if (face->getPoint(idx) == grid.getPoint(i,cols-1)) {
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(i, cols, face->getPoint(idx));
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(i-1, cols, face->getPoint(idx));
                    } else {
                        idx = face->indexOfPoint(grid.getPoint(i,cols-1));
                        idx = (idx + 1) % face->numberOfPoints();
                        if (face->getPoint(idx) == grid.getPoint(i-1,cols-1)) {
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(i-1, cols, face->getPoint(idx));
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(i, cols, face->getPoint(idx));
                        }
                    }
                }
                // check if the boundary edges do not switch between crease or not crease
                bool dataOK = true;
                if (mode == amNURBS) {
                    edge1 = edgeExists(grid.getPoint(0, cols),
                                       grid.getPoint(0, cols-1));
                    edge2 = edgeExists(grid.getPoint(0, cols-1),
                                       grid.getPoint(0, cols-2));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    edge1 = edgeExists(grid.getPoint(rows-1, cols),
                                       grid.getPoint(rows-1, cols-1));
                    edge2 = edgeExists(grid.getPoint(rows-1, cols-1),
                                       grid.getPoint(rows-1, cols-2));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    if (!dataOK) {
                        // do not add the current row
                        for (size_t i=0; i<tmpfaces.size(); i++)
                            faces.push_back(tmpfaces[i]);
                        searchright = false;
                    } else {
                        cols++;
                    }
                } else {
                    searchright = false;
                }
            }
        } else if (counter == 3 && searchtop) {
            for (size_t i=2; i<=cols; i++) {
                SubdivisionEdge* edge = edgeExists(
                    grid.getPoint(0, i-2),
                    grid.getPoint(0, i-1));
                if (edge != 0 && !edge->isCrease()) {
                    for (size_t j=0; j<edge->numberOfFaces(); j++) {
                        face = dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                        if (privValidFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i-1)
                    break;
            }
            if (tmpfaces.size() == cols - 1) {
                // search was successful
                grid.setRows(rows+1);
                for (size_t i=rows; i>=1; i--) {
                    for (size_t j=0; j<cols; j++) {
                        grid.setPoint(i,j,grid.getPoint(i-1,j));
                    }
                }
                for (size_t i=0; i<cols; i++)
                    grid.setPoint(0, i, 0);
                for (size_t i=1; i<=tmpfaces.size(); i++) {
                    face = tmpfaces[i-1];
                    vector<SubdivisionControlFace*>::iterator index = find(
                        faces.begin(), faces.end(), face);
                    if (index != faces.end())
                        faces.erase(index);
                    size_t idx = face->indexOfPoint(grid.getPoint(1,i-1));
                    idx = (idx + 1) % face->numberOfPoints();
                    if (face->getPoint(idx) == grid.getPoint(1,i)) {
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(0, i, face->getPoint(idx));
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(0, i-1, face->getPoint(idx));
                    } else {
                        idx = face->indexOfPoint(grid.getPoint(1,i));
                        idx = (idx + 1) % face->numberOfPoints();
                        if (face->getPoint(idx) == grid.getPoint(1,i-1)) {
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(0, i-1, face->getPoint(idx));
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(0, i, face->getPoint(idx));
                        }
                    }
                }
                // check if the boundary edges do not switch between crease or not crease
                bool dataOK = true;
                if (mode == amNURBS) {
                    edge1 = edgeExists(grid.getPoint(0, 0),
                                       grid.getPoint(1, 0));
                    edge2 = edgeExists(grid.getPoint(1, 0),
                                       grid.getPoint(2, 0));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    edge1 = edgeExists(grid.getPoint(0, cols-1),
                                       grid.getPoint(1, cols-1));
                    edge2 = edgeExists(grid.getPoint(1, cols-1),
                                       grid.getPoint(2, cols-1));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    for (size_t i=2; i<=cols-1; i++) {
                        if (!grid.getPoint(0,i-1)->isRegularNURBSPoint(checkfaces))
                            dataOK = false;
                    }
                    if (!dataOK) {
                        // do not add the current row
                        for (size_t i=0; i<tmpfaces.size(); i++)
                            faces.push_back(tmpfaces[i]);
                        searchtop = false;
                        for (size_t i=1; i<=rows; i++) {
                            for (size_t j=0; j<cols; j++) {
                                grid.setPoint(i-1,j,grid.getPoint(i,j));
                            }
                        }
                    } else {
                        rows++;
                    }
                } else {
                    searchtop = false;
                }
            }
        } else if (counter == 4 && searchleft) {
            for (size_t i=2; i<=rows; i++) {
                SubdivisionEdge* edge = edgeExists(
                    grid.getPoint(i-2, 0),
                    grid.getPoint(i-1, 0));
                if (edge != 0 && !edge->isCrease()) {
                    for (size_t j=0; j<edge->numberOfFaces(); j++) {
                        face = dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                        if (privValidFace(face, faces, tmpfaces)) {
                            tmpfaces.push_back(face);
                            break;
                        }
                    }
                }
                if (tmpfaces.size() != i-1)
                    break;
            }
            if (tmpfaces.size() == rows - 1) {
                // search was successful
                grid.setCols(cols+1);
                for (size_t i=0; i<rows; i++) {
                    for (size_t j=cols; j>=1; j--)
                        grid.setPoint(i,j,grid.getPoint(i,j-1));
                }
                for (size_t i=1; i<=tmpfaces.size(); i++) {
                    face = tmpfaces[i-1];
                    vector<SubdivisionControlFace*>::iterator index = find(faces.begin(), faces.end(), face);
                    if (index != faces.end())
                        faces.erase(index);
                    size_t idx = face->indexOfPoint(grid.getPoint(i,1));
                    idx = (idx + 1) % face->numberOfPoints();
                    if (face->getPoint(idx) == grid.getPoint(i-1,1)) {
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(i-1, 0, face->getPoint(idx));
                        idx = (idx + 1) % face->numberOfPoints();
                        grid.setPoint(i, 0, face->getPoint(idx));
                    } else {
                        idx = face->indexOfPoint(grid.getPoint(i-1,1));
                        idx = (idx + 1) % face->numberOfPoints();
                        if (face->getPoint(idx) == grid.getPoint(i,1)) {
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(i, 0, face->getPoint(idx));
                            idx = (idx + 1) % face->numberOfPoints();
                            grid.setPoint(i-1, 0, face->getPoint(idx));
                        }
                    }
                }
                // check if the boundary edges do not switch between crease or not crease
                bool dataOK = true;
                if (mode == amNURBS) {
                    edge1 = edgeExists(grid.getPoint(0, 0),
                                       grid.getPoint(0, 1));
                    edge2 = edgeExists(grid.getPoint(0, 1),
                                       grid.getPoint(0, 2));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    edge1 = edgeExists(grid.getPoint(rows-1, 0),
                                       grid.getPoint(rows-1, 1));
                    edge2 = edgeExists(grid.getPoint(rows-1, 1),
                                       grid.getPoint(rows-1, 2));
                    if (edge1 != 0 && edge2 != 0 && edge1->isCrease() != edge2->isCrease())
                        dataOK = false;
                    for (size_t i=2; i<=rows-1; i++) {
                        if (!grid.getPoint(i-2,0)->isRegularNURBSPoint(checkfaces))
                            dataOK = false;
                    }
                    if (!dataOK) {
                        // do not add the current row
                        for (size_t i=0; i<tmpfaces.size(); i++)
                            faces.push_back(tmpfaces[i]);
                        searchleft = false;
                        for (size_t i=0; i<rows; i++) {
                            for (size_t j=1; j<=cols; j++) {
                                grid.setPoint(i,j-1,grid.getPoint(i,j));
                            }
                        }
                    } else {
                        cols++;
                    }
                } else {
                    searchleft = false;
                }
            }
        }
        if (mode == amNURBS) {
            if (!grid.getPoint(0,0)->isRegularNURBSPoint(checkfaces)) {
                searchleft = false;
                searchtop = false;
            }
            if (!grid.getPoint(rows-1,0)->isRegularNURBSPoint(checkfaces)) {
                searchleft = false;
                searchbottom = false;
            }
            if (!grid.getPoint(rows-1,cols-1)->isRegularNURBSPoint(checkfaces)) {
                searchright = false;
                searchbottom = false;
            }
            if (!grid.getPoint(0,cols-1)->isRegularNURBSPoint(checkfaces)) {
                searchright = false;
                searchtop = false;
            }
        }
    }
}

void SubdivisionSurface::assembleFaces(assemble_mode_t mode,
                                       vector<SubdivisionControlFace*>& ctrlfaces,
                                       vector<ControlFaceGrid>& assembled)
{
    size_t rows, cols;
    int errind;
    
    vector<SubdivisionControlFace*> checkfaces(ctrlfaces.begin(), ctrlfaces.end());
    while (ctrlfaces.size() > 0) {
        // find a corner face
        SubdivisionControlFace* face = privFindCornerFace(ctrlfaces);
        errind++;
        if (face != 0) {

            ControlFaceGrid newfacegrid;
            PointGrid grid;
            
            vector<SubdivisionControlFace*>::iterator index = find(
                ctrlfaces.begin(), ctrlfaces.end(), face);
            if (index != ctrlfaces.end())
                ctrlfaces.erase(index);
            if (face->numberOfPoints() == 4) {
                cols = 2;
                rows = 2;
                grid.setRows(2);
                grid.setCols(2);
                grid.setPoint(0,1,face->getPoint(0));
                grid.setPoint(0,0,face->getPoint(1));
                grid.setPoint(1,0,face->getPoint(2));
                grid.setPoint(1,1,face->getPoint(3));
                doAssembleSpecial(grid, cols, rows, mode, checkfaces, ctrlfaces);

                newfacegrid.setRows(rows - 1);
                newfacegrid.setCols(cols - 1);
                for (size_t i=2; i<=rows; i++) {
                    for (size_t j=2; j<=cols; j++) {
                        SubdivisionControlFace* face = privGetControlFace(
                            grid.getPoint(i-2, j-1),
                            grid.getPoint(i-2, j-2),
                            grid.getPoint(i-1, j-2),
                            grid.getPoint(i-1, j-1));
                        if (face != 0)
                            newfacegrid.setFace(i-2, j-2, face);
                        // elseraise exception
                    }
                }
                assembled.push_back(newfacegrid);
            } else {
                newfacegrid.setRows(1);
                newfacegrid.setCols(1);
                newfacegrid.setFace(0, 0, face);
                assembled.push_back(newfacegrid);
            }
        }
    }
}

// tries to assemble quads into as few as possible rectangular patches
void SubdivisionSurface::assembleFacesToPatches(assemble_mode_t mode,
                                                vector<ControlFaceGrid>& assembledPatches)
{
    vector<SubdivisionControlFace*> todo;
    vector<vector<SubdivisionControlFace*>* > done;

    // use all visible faces
    for (size_t i=0; i<numberOfLayers(); i++) {
        SubdivisionLayer* layer = getLayer(i);
        if (layer->isVisible()) {
            for (size_t j=0; j<layer->numberOfFaces(); j++)
                todo.push_back(layer->getFace(j));
        }
    }
    if (todo.size() > 0) {
        while (todo.size() > 0) {
            SubdivisionControlFace* face = todo.back();
            todo.erase(todo.end());
            vector<SubdivisionControlFace*>* current = new vector<SubdivisionControlFace*>();
            current->push_back(face);
            privFindAttachedFaces(*current, todo, face);
            done.push_back(current);
        }
        // assign all groups to different layers
        for (size_t i=0; i<done.size(); i++) {
            vector<SubdivisionControlFace*>* current = done[i];
            if (current->size() > 0)
                assembleFaces(mode, *current, assembledPatches);
            delete current;
        }
    }
}

void SubdivisionSurface::calculateGaussCurvature()
{
    if (!isBuild())
        rebuild();
    _min_gaus_curvature = 0;
    _max_gaus_curvature = 0;
    for (size_t i=0; i<numberOfPoints(); ++i) {
        SubdivisionPoint* point = getPoint(i);
        _gaus_curvature.push_back(point->getCurvature());
        if (i == 0) {
            _min_gaus_curvature = _gaus_curvature[0];
            _max_gaus_curvature = _min_gaus_curvature;
        }
        if (_gaus_curvature[i] < _min_gaus_curvature)
            _min_gaus_curvature = _gaus_curvature[i];
        if (_gaus_curvature[i] > _max_gaus_curvature)
            _max_gaus_curvature = _gaus_curvature[i];
    }
}

float SubdivisionSurface::getGaussCurvature(size_t idx)
{
    if (idx >= _gaus_curvature.size())
        throw range_error("curvature index out of range");
    return _gaus_curvature[idx];
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
    else
        throw range_error("edge not found in SubdivisionSurface::removeControlEdge");
}

// delete a controledge singly, not by dumping the pool
void SubdivisionSurface::deleteControlEdge(SubdivisionControlEdge* edge)
{
    if (hasSelectedControlEdge(edge))
        removeSelectedControlEdge(edge);
    if (hasControlEdge(edge)) {
        removeControlEdge(edge);
        edge->removeEdge();
        _deleted.edges.insert(edge);
    }
    setBuild(false);
}

SubdivisionControlCurve* SubdivisionSurface::getControlCurve(size_t index)
{
    if (index < _control_curves.size())
        return _control_curves[index];
    throw range_error("bad index in SubdivisionSurface::getControlCurve");
}

bool SubdivisionSurface::hasControlCurve(SubdivisionControlCurve* curve)
{
    return find(_control_curves.begin(), _control_curves.end(),
                curve) != _control_curves.end();
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

void SubdivisionSurface::deleteControlCurve(SubdivisionControlCurve *curve)
{
    if (curve->isSelected())
        curve->setSelected(false);
    if (hasControlCurve(curve)) {
        removeControlCurve(curve);
        _deleted.splines.insert(curve->getSpline());
        curve->removeCurve();
        _deleted.curves.insert(curve);
    }
    setBuild(false);
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
    else
        throw range_error("face not found in SubdivisionSurface::removeControlFace");
}

void SubdivisionSurface::deleteControlFace(SubdivisionControlFace *face)
{
    if (hasSelectedControlFace(face))
        removeSelectedControlFace(face);
    if (hasControlFace(face)) {
        removeControlFace(face);
        face->removeFace();
        _deleted.faces.insert(face);
    }
    setBuild(false);
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
    for (size_t i=0; i<numberOfControlFaces(); ++i)
        result += getControlFace(i)->numberOfChildren();
    return result;
}

size_t SubdivisionSurface::numberOfLockedPoints()
{
    size_t result = 0;
    for (size_t i=0; i<numberOfControlPoints(); ++i)
        if (getControlPoint(i)->isLocked())
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

void SubdivisionSurface::deletePoint(SubdivisionPoint* point)
{
    vector<SubdivisionPoint*>::iterator i = find(_points.begin(), _points.end(), point);
    if (i != _points.end()) {
        _points.erase(i);
        point->~SubdivisionPoint();
        _point_pool.del(point);
    }
}

SubdivisionEdge* SubdivisionSurface::getEdge(size_t index)
{
    if (index < _edges.size())
        return _edges[index];
    throw range_error("bad index in SubdivisionSurface::getEdge");
}

void SubdivisionSurface::deleteEdge(SubdivisionEdge* edge)
{
    vector<SubdivisionEdge*>::iterator i = find(_edges.begin(), _edges.end(), edge);
    if (i != _edges.end()) {
        _edges.erase(i);
        edge->~SubdivisionEdge();
        _edge_pool.del(edge);
    }
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

void SubdivisionSurface::setSelectedControlEdge(SubdivisionControlEdge* edge)
{
    if (!hasSelectedControlEdge(edge))
        _sel_control_edges.insert(edge);
}

bool SubdivisionSurface::hasSelectedControlEdge(SubdivisionControlEdge* edge)
{
    set<SubdivisionControlEdge*>::iterator i = _sel_control_edges.find(edge);
    return (i != _sel_control_edges.end());
}

void SubdivisionSurface::removeSelectedControlEdge(SubdivisionControlEdge *edge)
{
    set<SubdivisionControlEdge*>::iterator i = _sel_control_edges.find(edge);
    if (i != _sel_control_edges.end())
        _sel_control_edges.erase(i);
}

void SubdivisionSurface::setSelectedControlCurve(SubdivisionControlCurve* curve)
{
    if (!hasSelectedControlCurve(curve))
        _sel_control_curves.insert(curve);
}

bool SubdivisionSurface::hasSelectedControlCurve(SubdivisionControlCurve* curve)
{
    return (find(_sel_control_curves.begin(), _sel_control_curves.end(), curve)
            != _sel_control_curves.end());
}

void SubdivisionSurface::removeSelectedControlCurve(SubdivisionControlCurve *curve)
{
    set<SubdivisionControlCurve*>::iterator i = _sel_control_curves.find(curve);
    if (i != _sel_control_curves.end())
        _sel_control_curves.erase(i);
}

void SubdivisionSurface::setSelectedControlFace(SubdivisionControlFace* face)
{
    if (!hasSelectedControlFace(face))
        _sel_control_faces.insert(face);
}

bool SubdivisionSurface::hasSelectedControlFace(SubdivisionControlFace* face)
{
    return (_sel_control_faces.find(face) != _sel_control_faces.end());
}

void SubdivisionSurface::removeSelectedControlFace(SubdivisionControlFace *face)
{
    set<SubdivisionControlFace*>::iterator i = _sel_control_faces.find(face);
    if (i != _sel_control_faces.end())
        _sel_control_faces.erase(i);
}

size_t SubdivisionSurface::numberOfSelectedLockedPoints()
{
    size_t result = 0;
    for (OrderedPointMap::iterator i=_sel_control_points.begin(); i!=_sel_control_points.end(); ++i)
        if ((*i)->isLocked())
            result++;
    return result;
}

void SubdivisionSurface::setSelectedControlPoint(SubdivisionControlPoint* pt)
{
    if (!hasSelectedControlPoint(pt))
        _sel_control_points.add(pt);
}

bool SubdivisionSurface::hasSelectedControlPoint(SubdivisionControlPoint* pt)
{
    return _sel_control_points.has(pt);
}

void SubdivisionSurface::removeSelectedControlPoint(SubdivisionControlPoint *pt)
{
    _sel_control_points.remove(pt);
}

size_t SubdivisionSurface::requestNewLayerID()
{
    _last_used_layerID++;
    return _last_used_layerID;
}

QString SubdivisionSurface::getDefaultLayerName()
{
    return tr("Layer");
}

void SubdivisionSurface::setActiveLayer(SubdivisionLayer *layer)
{
    _active_layer = layer;
}

void SubdivisionSurface::setBuild(bool val)
{
    cout << "set build" << endl;
    Entity::setBuild(val);
    if (!val) {
        clearFaces();
        for (size_t i=0; i<numberOfControlCurves(); ++i)
            getControlCurve(i)->setBuild(false);
        _current_subdiv_level = 0;
        _gaus_curvature.clear();
        _min_gaus_curvature = 0;
        _max_gaus_curvature = 0;
    }
}

std::multimap<float, SubdivisionBase*>
SubdivisionSurface::shootPickRay(Viewport& vp, const PickRay& ray)
{
    multimap<float,SubdivisionBase*> picks;
    // check control points
    if (ray.point) {
        for (size_t i=0; i<numberOfControlPoints(); i++) {
            SubdivisionControlPoint* cp = getControlPoint(i);
            if (cp->distanceFromPickRay(vp, ray) < 1E-2) {
                float s = ray.pt.distanceToPoint(cp->getCoordinate());
                picks.insert(make_pair(s,cp));
            }
        }
    }
    if (ray.edge && picks.size() == 0) {
        // check control edges
        for (size_t i=0; i<numberOfControlEdges(); i++) {
            SubdivisionControlEdge* edge = getControlEdge(i);
            if (!edge->isVisible())
                continue;
            float dist = edge->distanceToEdge(ray.pt, ray.dir);
            if (dist < 1E-2) {
                picks.insert(make_pair(dist, edge));
                break; // if we came close to an edge, pick it and stop looking
            }
        }
    }
    if (ray.face && picks.size() == 0) {
        // check face
        for (size_t i=0; i<numberOfControlFaces(); i++) {
            SubdivisionControlFace* face = getControlFace(i);
            if (face->intersectWithRay(ray)) {
                picks.insert(make_pair(0, face));
                break;
            }
        }
    }
    return picks;
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
        for (size_t i=0; i<points.size(); ++i) {
            point = addControlPoint(points[i]);
            result->addPoint(point);
            if (i > 0) {
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
        for (size_t i=0; i<result->numberOfPoints(); ++i) {
            size_t n = 0;
            point = result->getPoint(i);
            for (size_t j=0; j<point->numberOfFaces(); ++j) {
                if (point->getFace(j) == result)
                    n++;
            }
            if (n > 1) {
                invalidface = true;
                break;
            }
        }
        if (result->numberOfPoints() < 3 || invalidface) {
            // delete invalid controlfaces
            for (size_t j=0; j<result->numberOfPoints(); ++j) {
                result->getPoint(j)->deleteFace(result);
                if (j == 0)
                    edge = controlEdgeExists(result->getPoint(result->numberOfPoints()-1), result->getPoint(j));
                else
                    edge = controlEdgeExists(result->getPoint(j-1), result->getPoint(j));
                if (edge != 0) {
                    edge->deleteFace(result);
                    edge->startPoint()->deleteEdge(edge);
                    edge->startPoint()->deleteFace(result);
                    edge->endPoint()->deleteEdge(edge);
                    edge->endPoint()->deleteFace(result);
                }
            }
            deleteControlFace(result);
            result = 0;
        }
        else {
            _control_faces.push_back(result);
            if (getActiveLayer() == 0)
                result->setLayer(getLayer(0));
            else
                result->setLayer(getActiveLayer());
        }
    }
    else
        result = 0;
    deleteElementsCollection();
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

void SubdivisionSurface::addControlCurves(vector<SubdivisionControlEdge*>& edges)
{
    vector<vector<SubdivisionControlPoint*> > sortededges;
    isolateEdges(edges, sortededges);
    for (size_t i=0; i<sortededges.size(); ++i) {
        vector<SubdivisionControlPoint*>& points = sortededges[i];
        if (points.size() > 1) {
            SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(this);
            addControlCurve(curve);
            for (size_t j=0; j<points.size(); ++j) {
                SubdivisionControlPoint* p1 = points[j];
                curve->addPoint(p1);
                if (j > 0) {
                    SubdivisionControlEdge* edge =
                        controlEdgeExists(curve->getControlPoint(j-1), curve->getControlPoint(j));
                    if (edge != 0)
                        edge->setCurve(curve);
                }
            }
        }
    }
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
        for (size_t j=0; j<points.size(); ++j) {
            p2 = points[j];
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
            deleteControlFace(result);
            result = 0;
        }
        else 
            setBuild(false);
        deleteElementsCollection();
    }
    return result;
}

SubdivisionControlFace* SubdivisionSurface::addControlFace(std::vector<SubdivisionControlPoint *> &points, bool check_edges)
{
    return addControlFace(points, check_edges, 0);
}

void SubdivisionSurface::clear()
{
    Entity::clear();
    _control_curves.clear();
    _control_faces.clear();
    _control_edges.clear();
    _control_points.clear();
    _layers.clear();
    _edges.clear();
    _points.clear();

    // clear the pools
    _ccurve_pool.clear();
    _cface_pool.clear();
    _cedge_pool.clear();
    _cpoint_pool.clear();
    _layer_pool.clear();
    _edge_pool.clear();
    _point_pool.clear();
    _spline_pool.clear();

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
}

void SubdivisionSurface::clearFaces()
{
    for (size_t i=0; i<numberOfControlFaces(); ++i) {
        getControlFace(i)->clearChildren();       // deletes children and rendermesh
    }
    // dump all edges
    _edges.clear();
    _edge_pool.clear();
    // dump all points
    _points.clear();
    _point_pool.clear();
    // dump all faces
    _face_pool.clear();
    // clear edges in control faces
    for (size_t i=0; i<numberOfControlFaces(); ++i)
        getControlFace(i)->clearControlEdges();
}

void SubdivisionSurface::clearSelection()
{
    _sel_control_curves.clear();
    _sel_control_edges.clear();
    _sel_control_faces.clear();
    _sel_control_points.clear();
}

void SubdivisionSurface::doAssemble(PointGrid& grid, 
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
            for (size_t i=2; i<=grid.cols(); ++i) {
                edge = edgeExists(grid.getPoint(rows-1,i-2), grid.getPoint(rows-1,i-1));
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (privValidFace(face, faces, tmpfaces)) {
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
                    grid.setRows(rows+1);
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid.getPoint(rows-1,i));
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid.getPoint(rows-1,i-1)) {
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(rows, i-1, face->getPoint(index));
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(rows, i, face->getPoint(index));
                    }
                    else {
                        index = face->indexOfPoint(grid.getPoint(rows-1,i-1));
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid.getPoint(rows-1,i)) {
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(rows, i, face->getPoint(index));
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(rows, i-1, face->getPoint(index));
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
                edge = edgeExists(grid.getPoint(i-1, cols-1), grid.getPoint(i-2, cols-1));
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (privValidFace(face, faces, tmpfaces)) {
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
                grid.setCols(cols+1);
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid.getPoint(i-1,cols-1));
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid.getPoint(i, cols-1)) {
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(i, cols, face->getPoint(index));
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(i-1, cols, face->getPoint(index));
                    }
                    else {
                        index = face->indexOfPoint(grid.getPoint(i, cols-1));
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid.getPoint(i-1, cols-1)) {
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(i-1, cols, face->getPoint(index));
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(i, cols, face->getPoint(index));
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
                edge = edgeExists(grid.getPoint(0, i-2), grid.getPoint(0, i-1));
                if (edge != 0) {
                    for (size_t j=1; j<=edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j-1);
                        if (privValidFace(face, faces, tmpfaces)) {
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
                grid.setRows(rows+1);
                for (size_t i=rows; i>=1; i--) {
                    for (size_t j=0; j<cols; j++) {
                        grid.setPoint(i,j,grid.getPoint(i-1,j));
                    }
                }
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid.getPoint(1, i-1));
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid.getPoint(1, i)) {
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(0, i, face->getPoint(index));
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(0, i-1, face->getPoint(index));
                    }
                    else {
                        index = face->indexOfPoint(grid.getPoint(1, i));
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid.getPoint(1, i-1)) {
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(0, i-1, face->getPoint(index));
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(0, i, face->getPoint(index));
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
                edge = edgeExists(grid.getPoint(i-2, 0), grid.getPoint(i-1, 0));
                if (edge != 0) {
                    for (size_t j=0; j<edge->numberOfFaces(); ++j) {
                        face = edge->getFace(j);
                        if (privValidFace(face, faces, tmpfaces)) {
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
                grid.setCols(cols+1);
                for (size_t i=0; i<rows; ++i) {
                    for (size_t j=cols; j>=1; j--)
                        grid.setPoint(i, j, grid.getPoint(i, j-1));
                }
                for (size_t i=1; i<=tmpfaces.size(); ++i) {
                    face = tmpfaces[i-1];
                    fidx = find(faces.begin(), faces.end(), face);
                    if (fidx != faces.end())
                        faces.erase(fidx);
                    index = face->indexOfPoint(grid.getPoint(i, 1));
                    index = (index + 1) % face->numberOfPoints();
                    if (face->getPoint(index) == grid.getPoint(i-1, 1)) {
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(i-1, 0, face->getPoint(index));
                        index = (index + 1) % face->numberOfPoints();
                        grid.setPoint(i, 0, face->getPoint(index));
                    }
                    else {
                        index = face->indexOfPoint(grid.getPoint(i-1, 1));
                        index = (index + 1) % face->numberOfPoints();
                        if (face->getPoint(index) == grid.getPoint(i, 1)) {
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(i, 0, face->getPoint(index));
                            index = (index + 1) % face->numberOfPoints();
                            grid.setPoint(i-1, 0, face->getPoint(index));
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

void SubdivisionSurface::convertToGrid(ControlFaceGrid& input, PointGrid& grid)
{
    size_t cols = 0;
    size_t rows = 0;
    if (input.rows() == 0 || input.cols() == 0)
        return;
    // assemble all childfaces in one temp sorted list
    size_t n = input.getFace(0, 0)->numberOfChildren();
    vector<SubdivisionFace*> backup;
    backup.reserve(input.rows() * input.cols() * n);
    for (size_t i=0; i<input.rows(); ++i) {
        for (size_t j=0; j<input.cols(); ++j) {
            SubdivisionControlFace* ctrlface = input.getFace(i, j);
            backup.insert(backup.end(), ctrlface->childrenBegin(), ctrlface->childrenEnd());
        }
    }
    // BUGBUG: how does this sort them? the comparator
    sort(backup.begin(), backup.end());
    if (backup.size() == 0)
        return;
    vector<SubdivisionFace*> faces;
    faces.reserve(backup.size());
    size_t ind = 0;
    do {
        faces.insert(faces.end(), backup.begin(), backup.end());
        ++ind;
        SubdivisionFace* face = faces[ind-1];
        faces.erase(faces.begin()+ind-1);
        grid.setRows(2);
        grid.setCols(2);
        rows = 2;
        cols = 2;
        grid.setPoint(0, 0, face->getPoint(0));
        grid.setPoint(0, 1, face->getPoint(1));
        grid.setPoint(1, 0, face->getPoint(2));
        grid.setPoint(1, 1, face->getPoint(3));
        doAssemble(grid, cols, rows, faces);
    }
    while (faces.size() > 0 && ind < backup.size());
    if (faces.size() != 0)
        throw runtime_error("Could not establish the entire grid!");
}

bool SubdivisionSurface::edgeConnect()
{
    for (size_t i=0; i<_sel_control_points.size()-1; ++i) {
        SubdivisionControlPoint* v1 = _sel_control_points.get(i);
        SubdivisionControlPoint* v2 = _sel_control_points.get(++i);
        if (controlEdgeExists(v1, v2) == 0) {
            if (v1->numberOfFaces() == 0 && v2->numberOfFaces() == 0) {
                SubdivisionControlEdge* edge = addControlEdge(v1, v2);
                if (edge != 0)
                    edge->setCrease(true);
            }
            else {
                for (size_t j=0; j<v1->numberOfFaces(); ++j) {
                    SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(v1->getFace(j));
                    if (v2->hasFace(face)) {
                        bool deleteface;
                        face->insertControlEdge(v1, v2, deleteface);
                        if (deleteface)
                            deleteControlFace(face);
                        break;
                    }
                }
            }
        }
        else if (_sel_control_points.size() == 2) {
            return false;
        }
    }
    _sel_control_points.clear();
    deleteElementsCollection();
    setBuild(false);
    return true;
}

void SubdivisionSurface::exportFeFFile(QStringList& strings)
{
    // add layer information
    strings.push_back(QString("%1").arg(numberOfLayers()));
    for (size_t i=0; i<numberOfLayers(); ++i) {
        SubdivisionLayer* layer = getLayer(i);
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
    // BUGBUG: sort controlpoints for faster access of function (indexof())
    strings.push_back(QString("%1").arg(_control_points.size()));
    for (size_t i=0; i<_control_points.size(); ++i)
        _control_points[i]->saveToStream(strings);
    strings.push_back(QString("%1").arg(_control_edges.size()));
    for (size_t i=0; i<_control_edges.size(); ++i)
        _control_edges[i]->saveToStream(strings);
    strings.push_back(QString("%1").arg(_control_faces.size()));
    for (size_t i=0; i<_control_faces.size(); ++i)
        _control_faces[i]->saveToStream(strings);
}

void SubdivisionSurface::exportObjFile(bool export_control_net, QStringList& strings)
{
    if (!isBuild())
        rebuild();
    strings.push_back("# FREE!ship model");
    // first sort controlpoints for faster access of function indexOf()
    sort(_points.begin(), _points.end());

    if (!export_control_net) {
        // export subdivided surface
        // create points for portside
      vector<SubdivisionPoint*> tmp;
      tmp.reserve(numberOfPoints());
        for (size_t i=0; i<numberOfPoints(); ++i) {
            // BUGBUG: FloatToStrF ffFixed, 7, 4
            strings.push_back(QString("v %1 %2 %3").arg(getPoint(i)->getCoordinate().y())
                              .arg(getPoint(i)->getCoordinate().z())
                              .arg(getPoint(i)->getCoordinate().x()));
            if (getPoint(i-1)->getCoordinate().y() > 0)
                tmp.push_back(getPoint(i-1));
        }
        if (drawMirror()) {
            // create points for starboard side
            sort(tmp.begin(), tmp.end());
            for (size_t i=0; i<tmp.size(); ++i) {
                SubdivisionPoint* p = tmp[i];
                strings.push_back(QString("v %1 %2 %3").arg(-p->getCoordinate().y())
                                  .arg(p->getCoordinate().z())
                                  .arg(p->getCoordinate().x()));
            }
        }
        for (size_t i=0; i<numberOfControlFaces(); ++i) {
            SubdivisionControlFace* cface = _control_faces[i];
            if (cface->getLayer()->isVisible()) {
                for (size_t j=0; j<cface->numberOfChildren(); ++j) {
                    // portside
                    QString str("f");
                    SubdivisionFace* child = cface->getChild(j);
                    for (size_t k=0; k<child->numberOfPoints(); ++k) {
                        size_t index = indexOfPoint(child->getPoint(k));
                        str.append(QString(" %1").arg(index+1));
                    }
                    strings.push_back(str);
                    if (cface->getLayer()->isSymmetric() && drawMirror()) {
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
      vector<SubdivisionPoint*> tmp;
      tmp.reserve(numberOfPoints());
        for (size_t i=0; i<numberOfControlPoints(); ++i) {
            // BUGBUG: FloatToStrF ffFixed, 7, 4
            strings.push_back(QString("v %1 %2 %3").arg(getControlPoint(i)->getCoordinate().y())
                              .arg(getControlPoint(i)->getCoordinate().z())
                              .arg(getControlPoint(i)->getCoordinate().x()));
            if (getControlPoint(i)->getCoordinate().y() > 0)
                tmp.push_back(getControlPoint(i));
        }
        if (drawMirror()) {
            // create points for starboard side
            sort(tmp.begin(), tmp.end());
            for (size_t i=0; i<tmp.size(); ++i) {
                SubdivisionPoint* p = tmp[i];
                strings.push_back(QString("v %1 %2 %3").arg(-p->getCoordinate().y())
                                  .arg(p->getCoordinate().z())
                                  .arg(p->getCoordinate().x()));
            }
        }
        for (size_t i=0; i<numberOfControlFaces(); ++i) {
            SubdivisionControlFace* cface = _control_faces[i];
            if (cface->getLayer()->isVisible()) {
                for (size_t j=0; j<cface->numberOfChildren(); ++j) {
                    // BUGBUG, j isn't used
                    // portside
                    QString str("f");
                    for (size_t k=0; k<cface->numberOfPoints(); ++k) {
                        size_t index = indexOfPoint(cface->getPoint(k));
                        str.append(QString(" %1").arg(index+1));
                    }
                    strings.push_back(str);
                    if (cface->getLayer()->isSymmetric() && drawMirror()) {
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
        for (size_t i=0; i<numberOfLayers(); ++i)
            getLayer(i)->extents(min, max);
        for (size_t i=0; i<numberOfControlPoints(); ++i) {
            if (_control_points[i]->numberOfFaces() == 0)
                MinMax(_control_points[i]->getCoordinate(), min, max);
        }
    }
    else {
        MinMax(_min, min, max);
        MinMax(_max, min, max);
    }
}

// predicate class to find an element with given point
struct ExistPointPred {
    ShipCAD::SubdivisionControlPoint* _querypt;
    bool operator()(const pair<ShipCAD::SubdivisionControlPoint*, ShipCAD::SubdivisionControlPoint*>& val)
    {
        return val.first == _querypt;
    }
    ExistPointPred (ShipCAD::SubdivisionPoint* querypt) : _querypt(dynamic_cast<SubdivisionControlPoint*>(querypt)) {}
};

// predicate class to find an element with given point
struct ExtrudePointPred {
    ShipCAD::SubdivisionControlPoint* _querypt;
    bool operator()(const pair<ShipCAD::SubdivisionControlPoint*, ShipCAD::SubdivisionControlPoint*>& val)
    {
        return val.second == _querypt;
    }
    ExtrudePointPred (ShipCAD::SubdivisionPoint* querypt) : _querypt(dynamic_cast<SubdivisionControlPoint*>(querypt)) {}
};

// FreeGeometry.pas:14684
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
    for (size_t i=0; i<edges.size(); ++i) {
        edge = edges[i];
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
    for (size_t i=0; i<vertices.size(); ++i) {
        point1 = vertices[i].first;
        point2 = newControlPoint(point1->getCoordinate() + direction);
        vertices[i].second = point2;
    }
    for (size_t i=0; i<edges.size(); ++i) {
        edge = edges[i];
        points.clear();
        points.push_back(dynamic_cast<SubdivisionControlPoint*>(edge->endPoint()));
        points.push_back(dynamic_cast<SubdivisionControlPoint*>(edge->startPoint()));
        point1 = 0;
        point2 = 0;
        vidx = find_if(vertices.begin(), vertices.end(), ExistPointPred(edge->startPoint()));
        if (vidx != vertices.end()) {
            point1 = (*vidx).second;
            points.push_back(point1);
        }
        vidx = find_if(vertices.begin(), vertices.end(), ExistPointPred(edge->endPoint()));
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
        if (edge->startPoint()->getVertexType() == svCorner && point1 != 0) {
            tmp = controlEdgeExists(edge->startPoint(), point1);
            if (tmp != 0)
                tmp->setCrease(true);
        }
        if (edge->endPoint()->getVertexType() == svCorner && point2 != 0) {
            tmp = controlEdgeExists(edge->endPoint(), point2);
            if (tmp != 0)
                tmp->setCrease(true);
        }
        edge->setCrease(true);
    }
    // return the new edges
    for (size_t i=0; i<newedges.size(); ++i)
        edges.push_back(newedges[i]);
    initialize(novertices+1, noedges+1);
    setBuild(false);
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
                                                SplineVector& destination)
{
    SubdivisionControlFace* ctrlface;
    SubdivisionFace* face, *f2;
    SubdivisionPoint* p1, *p2, *p3;
    SubdivisionEdge* edge;
    float side1, side2;
    bool addedge;

    // first assemble all edges belong to this set of faces
    vector<SubdivisionEdge*> edges;
    edges.reserve(faces.size()+100);
    vector<SurfIntersectionData> intarray;

    // first is start point, second is end point
    vector<pair<SurfIntersectionData, SurfIntersectionData> > segments;

    for (size_t i=0; i<faces.size(); ++i) {
        ctrlface = faces[i];
        for (size_t j=0; j<ctrlface->numberOfChildren(); ++j) {
            face = ctrlface->getChild(j);
            intarray.clear();
            p1 = face->getPoint(face->numberOfPoints()-1);
            side1 = plane.distance(p1->getCoordinate());
            for (size_t k=0; k<face->numberOfPoints(); ++k) {
                p2 = face->getPoint(k);
                side2 = plane.distance(p2->getCoordinate());
                bool addedge = false;
                if ((side1 < -1E-5 && side2 > 1E-5) || (side1 > 1E-5 && side2 < -1E-5)) {
                    // regular intersection of edge
                    // add the edge to the list
                    float parameter = -side1 / (side2 - side1);
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
                                for (size_t n=0; n<edge->numberOfFaces(); ++n) {
                                    f2 = edge->getFace(n);
                                    for (size_t m=0; m<f2->numberOfPoints(); ++m) {
                                        p3 = f2->getPoint(m);
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
                                        sp.knuckle = p1->getVertexType() != svRegular;
                                        ep.knuckle = p2->getVertexType() != svRegular;
                                    }
                                    else {
                                        sp.knuckle = p1->getVertexType() == svCorner;
                                        ep.knuckle = p2->getVertexType() == svCorner;
                                    }
                                    segments.push_back(make_pair(sp, ep));
                                }
                            }
                        }
                    }
                    else if (fabs(side2) < 1E-5) {
                        SurfIntersectionData id(p2->getCoordinate());
                        id.knuckle = p2->getVertexType() != svRegular;
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
                }
                size_t k = 1;
                while (k < intarray.size()) {
                    if (intarray[k].edge == intarray[k-1].edge) {
                        if (intarray[k].point.distanceToPoint(intarray[k-1].point) < 1E-4) {
                            intarray.erase(intarray.begin()+k-1);
                        }
                        else
                            ++k;
                    }
                    else
                        ++k;
                }
                for (size_t l=1; l<intarray.size(); ++l) {
                    segments.push_back(make_pair(intarray[l-1], intarray[l]));
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
            destination.add(spline);
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
        size_t j = 0;
        while (j < segments.size()) {
            segment = segments[j];
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
                segments.erase(segments.begin()+j);
                j = 0;
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
            spline = destination.get(i-1);
            if (spline->numberOfPoints()>1) {
                float parameter = SquaredDistPP(spline->getMin(), spline->getMax());
                if (parameter < 1E-3) {
                    destination.del(spline); // destination should own the pointer to delete it
                }
            }
        }
    }
}

// FreeGeometry.pas:15169
void SubdivisionSurface::draw(Viewport &vp)
{
    if (!isBuild())
        rebuild();
    if ((vp.getViewportMode() == vmShadeGauss
            || vp.getViewportMode() == vmShadeDevelopable)
            && !isGaussCurvatureCalculated()) {
        calculateGaussCurvature();
    }
    SubdivisionLayer::drawLayers(vp, this);
    LineShader* lineshader = vp.setLineShader();
    draw(vp, lineshader);
}

void SubdivisionSurface::draw(Viewport &vp, LineShader *lineshader)
{
    if (showControlNet()) {
        for (size_t i=0; i<numberOfControlEdges(); ++i) {
            getControlEdge(i)->draw(vp, lineshader);
        }
        SubdivisionControlPoint::drawControlPoints(vp, this);
    }
    for (size_t i=0; i<numberOfControlCurves(); ++i) {
        if (getControlCurve(i)->isVisible())
            getControlCurve(i)->draw(vp, lineshader);
    }
}

SubdivisionEdge* SubdivisionSurface::edgeExists(SubdivisionPoint *p1, SubdivisionPoint *p2)
{
    SubdivisionEdge* result = 0;
    // if the edge exists then it must exist
    // in both the points, therefore only the point
    // with the smallest number of edges has to be checked
    if (p1->numberOfEdges() <= p2->numberOfEdges()) {
        for (size_t i=0; i<p1->numberOfEdges(); ++i) {
            SubdivisionEdge* edge = p1->getEdge(i);
            if ((edge->startPoint() == p1 && edge->endPoint() == p2)
                    || (edge->startPoint() == p2 && edge->endPoint() == p1))
                return edge;
        }
    }
    else {
        for (size_t i=0; i<p2->numberOfEdges(); ++i) {
            SubdivisionEdge* edge = p2->getEdge(i);
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
        for (size_t i=0; i<edges.size(); ++i) {
            if (i == 0)
                points.push_back(edges[i]->startPoint());
            points.push_back(edges[i]->endPoint());
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
        for (size_t i=0; i<edges.size(); ++i) {
            if (i == 0)
                points.push_back(dynamic_cast<SubdivisionControlPoint*>(edges[i]->startPoint()));
            points.push_back(dynamic_cast<SubdivisionControlPoint*>(edges[i]->endPoint()));
        }
    }
    return points;
}

void SubdivisionSurface::extractAllEdgeLoops(vector<vector<SubdivisionPoint*> >& destination)
{
    vector<SubdivisionEdge*> sourcelist;
    for (size_t i=0; i<_edges.size(); ++i) {
        SubdivisionEdge* edge = _edges[i];
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
    for (size_t i=0; i<selectedfaces.size(); ++i) {
        SubdivisionFace* face = selectedfaces[i];
        for (size_t j=0; j<face->numberOfPoints(); j++) {
            SubdivisionControlPoint* p = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
            if (find(points.begin(), points.end(), p) == points.end()) {
                ok = true;
                for (size_t k=0; k<p->numberOfFaces(); ++k) {
                    if (find(selectedfaces.begin(), selectedfaces.end(), p->getFace(k)) == selectedfaces.end()) {
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
    for (set<SubdivisionControlFace*>::iterator i=_sel_control_faces.begin(); i!=_sel_control_faces.end(); ++i) {
        SubdivisionControlFace* face = *i;
        for (size_t j=0; j<face->numberOfPoints(); ++j) {
            p = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
            if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
                selectedpoints.push_back(p);
        }
    }
    for (set<SubdivisionControlEdge*>::iterator j=_sel_control_edges.begin(); j!=_sel_control_edges.end(); ++j) {
        SubdivisionControlEdge* edge = *j;
        p = dynamic_cast<SubdivisionControlPoint*>(edge->startPoint());
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
        p = dynamic_cast<SubdivisionControlPoint*>(edge->endPoint());
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
    }
    for (OrderedPointMap::iterator k=_sel_control_points.begin(); k!=_sel_control_points.end(); ++k) {
        p = *k;
        if (find(selectedpoints.begin(), selectedpoints.end(), p) == selectedpoints.end())
            selectedpoints.push_back(p);
    }
    for (size_t m=0; m<selectedpoints.size(); ++m) {
        if (selectedpoints[m]->isLocked())
            ++lockedpoints;
    }
}

void SubdivisionSurface::importFeFFile(QStringList &strings, size_t& lineno)
{
    size_t start;
    SubdivisionLayer* layer;

    // read layer information
    QString str = strings[lineno++].trimmed();
    start = 0;
    size_t n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        if (i >= numberOfLayers())
            layer = addNewLayer();
        else
            layer = getLayer(i);
        layer->setDescription(strings[++lineno].trimmed());
        str = strings[lineno++].trimmed();
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

    str = strings[lineno++].trimmed();
    start = 0;
    // read controlpoints
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlPoint* point = SubdivisionControlPoint::construct(this);
        _control_points.push_back(point);
        point->loadFromStream(lineno, strings);
    }

    str = strings[lineno++].trimmed();
    start = 0;
    // read controledges
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlEdge* edge = SubdivisionControlEdge::construct(this);
        _control_edges.push_back(edge);
        edge->loadFromStream(lineno, strings);
    }

    str = strings[lineno++].trimmed();
    start = 0;
    // read controlfaces
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlFace* face = SubdivisionControlFace::construct(this);
        _control_faces.push_back(face);
        str = strings[lineno++].trimmed();
        start = 0;
        size_t np = ReadIntFromStr(lineno, str, start);
        for (size_t j=0; j<np; ++j) {
            size_t index = ReadIntFromStr(lineno, str, start);
            // attach controlfacet to controlpoints
            face->addPoint(_control_points[index]);
        }
        // attach control face to the already existing control edges
        SubdivisionControlPoint* p1 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(face->numberOfPoints()-1));
        for (size_t j=0; j<face->numberOfPoints(); ++j) {
            SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j));
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
        layer->useControlFace(face);
    }
    setBuild(false);
    _initialized = true;
    setActiveLayer(0);
}

void SubdivisionSurface::importGrid(coordinate_grid_t& points, SubdivisionLayer* layer)
{
    size_t rows = points.size();
    size_t cols = points[0].size();

    PointGrid grid;
    grid.setRows(rows);
    grid.setCols(cols);
    for (size_t i=0; i<rows; ++i) {
        for (size_t j=0; j<cols; ++j)
            grid.setPoint(i, j, addControlPoint(points[i-1][j-1]));
    }

    vector<SubdivisionControlPoint*> facepoints;
    for (size_t i=2; i<=rows; ++i) {
        for (size_t j=2; j<=cols; ++j) {
            facepoints.clear();
            if (find(facepoints.begin(), facepoints.end(), grid.getPoint(i-1,j-1)) == facepoints.end())
                facepoints.push_back(dynamic_cast<SubdivisionControlPoint*>(grid.getPoint(i-1,j-1)));
            if (find(facepoints.begin(), facepoints.end(), grid.getPoint(i-1,j-2)) == facepoints.end())
                facepoints.push_back(dynamic_cast<SubdivisionControlPoint*>(grid.getPoint(i-1,j-2)));
            if (find(facepoints.begin(), facepoints.end(), grid.getPoint(i-2,j-2)) == facepoints.end())
                facepoints.push_back(dynamic_cast<SubdivisionControlPoint*>(grid.getPoint(i-2,j-2)));
            if (find(facepoints.begin(), facepoints.end(), grid.getPoint(i-2,j-1)) == facepoints.end())
                facepoints.push_back(dynamic_cast<SubdivisionControlPoint*>(grid.getPoint(i-2,j-1)));
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
        SubdivisionEdge* edge = edgeExists(grid.getPoint(0,i-2), grid.getPoint(0,i-1));
        if (edge != 0)
            edge->setCrease(true);
        edge = edgeExists(grid.getPoint(rows-1,i-2), grid.getPoint(rows-1,i-1));
        if (edge != 0)
            edge->setCrease(true);
    }
    for (size_t i=2; i<=rows; ++i) {
        SubdivisionEdge* edge = edgeExists(grid.getPoint(i-2,0), grid.getPoint(i-1,0));
        if (edge != 0)
            edge->setCrease(true);
        edge = edgeExists(grid.getPoint(i-2,cols-1), grid.getPoint(i-1,cols-1));
        if (edge != 0)
            edge->setCrease(true);
    }
    // set cornerpoints
    for (size_t i=0; i<rows; ++i) {
        for (size_t j=0; j<cols; ++j) {
            if (grid.getPoint(i, j)->numberOfFaces() < 2)
                grid.getPoint(i, j)->setVertexType(svCorner);
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
            _control_points[i-1]->setVertexType(svCorner);
    }
    _initialized = true;
}

bool SubdivisionSurface::intersectPlane(const Plane& plane, bool hydrostatics_layers_only, SplineVector& destination)
{
    QVector3D min, max;
    bool result = false;
    if (!isBuild())
        rebuild();
    if (!plane.intersectsBox(_min, _max))
        return result;
    vector<SubdivisionControlFace*> intersectedfaces;
    for (size_t i=0; i<numberOfLayers(); ++i) {
        SubdivisionLayer* layer = getLayer(i);
        bool use_layer;
        if (hydrostatics_layers_only)
            use_layer = layer->useInHydrostatics();
        else
            use_layer = layer->useForIntersections();
        if (use_layer) {
            for (size_t j=0; j<layer->numberOfFaces(); ++j) {
                SubdivisionControlFace* ctrlface = layer->getFace(j);
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
    size_t i = 0;
    while (i < numberOfControlEdges()) {
        SubdivisionControlEdge* edge = _control_edges[i];
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
restart:
        // try to find multiple points belonging to the same face and insert an edge
        i = 0;
        sort(points.begin(), points.end());
        vector<SubdivisionControlEdge*> edges;
        while (i < points.size()) {
            SubdivisionControlPoint* p1 = points[i];
            size_t j = 0;
            while (j < p1->numberOfFaces()) {
                SubdivisionControlFace* face = dynamic_cast<SubdivisionControlFace*>(p1->getFace(j));
                size_t k = 0;
                bool inserted = false;
                while (k < face->numberOfPoints()) {
                    SubdivisionControlPoint* p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(k));
                    if (p1 != p2 && find(points.begin(), points.end(), p2) != points.end()) {
                        // this is also a new point, first check if an edge already exists between p1 and p2
                        if (edgeExists(p1, p2) == 0) {
                            inserted = true;
                            bool deleteface;
                            SubdivisionControlEdge* edge = face->insertControlEdge(p1, p2, deleteface);
                            edges.push_back(edge);
                            // if face has been deleted, restart loop
                            if (deleteface) {
                                deleteControlFace(face);
                                goto restart;
                            }
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
            addControlCurves(edges);
        }
    }
    deleteElementsCollection();
    setBuild(false);
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
            for (size_t i=0; i<source.size(); ++i) {
                SubdivisionControlEdge* edge2 = source[i];
                // compare at start
                edge = tmpedges.front();
                if ((edge2->startPoint() == edge->startPoint())
                        || (edge2->startPoint() == edge->endPoint())
                        || (edge2->endPoint() == edge->startPoint())
                        || (edge2->endPoint() == edge->endPoint())) {
                    tmpedges.insert(tmpedges.begin(), edge2);
                    source.erase(source.begin()+i);
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
                        source.erase(source.begin()+i);
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

void SubdivisionSurface::isolateEdges(vector<SubdivisionEdge *> &source,
                                      vector<vector<SubdivisionPoint *> > &sorted)
{
    // try to isolate individual (closed) sets of edges
    vector<SubdivisionEdge*> tmpedges;
    while (source.size() > 0) {
        SubdivisionEdge* edge = source[0];
        source.erase(source.begin());
        bool findmore = true;
        tmpedges.clear();
        tmpedges.push_back(edge);
        while (source.size() > 0 && findmore) {
            findmore = false;
            for (size_t i=0; i<source.size(); ++i) {
                SubdivisionEdge* edge2 = source[i];
                // compare at start
                edge = tmpedges.front();
                if ((edge2->startPoint() == edge->startPoint())
                        || (edge2->startPoint() == edge->endPoint())
                        || (edge2->endPoint() == edge->startPoint())
                        || (edge2->endPoint() == edge->endPoint())) {
                    tmpedges.insert(tmpedges.begin(), edge2);
                    source.erase(source.begin()+i);
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
                        source.erase(source.begin()+i);
                        findmore = true;
                        break;
                    }
                }
            }
        }
        if (tmpedges.size() > 0) {
            //sort all found edges in correct order
            vector<SubdivisionPoint*> tmppoints = sortEdges(true, tmpedges);
            if (tmppoints.size() > 0)
                sorted.push_back(tmppoints);
        }
    }
}

// FreeGeometry.pas:10840
void SubdivisionSurface::collapseEdge(SubdivisionControlEdge* collapseedge)
{
	// checking..
	if (collapseedge->numberOfFaces() != 2)
		return;
	SubdivisionControlFace* face1 = dynamic_cast<SubdivisionControlFace*>(collapseedge->getFace(0));
	SubdivisionControlFace* face2 = dynamic_cast<SubdivisionControlFace*>(collapseedge->getFace(1));
	if (face1 == 0 || face2 == 0)
		return;
	
    SubdivisionControlPoint* s = 0;
    SubdivisionControlPoint* e = 0;
	SubdivisionPoint* p1, *p2;
	
	if (collapseedge->startPoint()->numberOfEdges() > 2
		&& collapseedge->endPoint()->numberOfEdges() > 2) {
		if (collapseedge->getCurve() != 0)
			collapseedge->getCurve()->deleteEdge(collapseedge);
        collapseedge->setSelected(false);
		s = dynamic_cast<SubdivisionControlPoint*>(collapseedge->startPoint());
		e = dynamic_cast<SubdivisionControlPoint*>(collapseedge->endPoint());
		if (s == 0 || e == 0)
			return;
		
		// check faces for consistent ordering of the points (same normal direction)
		// because inconsistent ordering can lead to access violations
		p1 = face1->getPoint(face1->numberOfPoints()-1);
		for (size_t i=0; i<face1->numberOfPoints(); ++i) {
			p2 = face1->getPoint(i);
			if ((p1 == collapseedge->startPoint() && p2 == collapseedge->endPoint())
				|| ((p2 == collapseedge->startPoint() && p1 == collapseedge->endPoint()))) {
				size_t i1 = face2->indexOfPoint(p2);
				size_t i2 = (i1 + 1) % face2->numberOfPoints(); // select the next index
				if (face2->getPoint(i2) != p1) {
					face2->flipNormal();
				}
				break;
			}
			else
				p1 = p2;
		}
	}
	
	SubdivisionLayer* layer = face1->getLayer();
	// remove the control faces from the layers they belong to
    face1->getLayer()->releaseControlFace(face1);
    face2->getLayer()->releaseControlFace(face2);
	size_t ind1 = face1->indexOfPoint(collapseedge->startPoint());
	size_t ind2 = face1->indexOfPoint(collapseedge->endPoint());
	if (ind2 < ind1 && abs(ind2 - ind1) == 1)
		swap(ind1, ind2);
	size_t ind3 = face2->indexOfPoint(collapseedge->startPoint());
	size_t ind4 = face2->indexOfPoint(collapseedge->endPoint());
	if (ind4 < ind3 && abs(ind4 - ind3) == 1)
		swap(ind3, ind4);
	if (ind1 == 0 && ind2 == face1->numberOfPoints() - 1
		&& ind3 == 0 && ind4 == face2->numberOfPoints() - 1) {
		swap(ind1, ind2);
		swap(ind3, ind4);
	}
	if (ind1 == 0 && ind2 == face1->numberOfPoints() - 1)
		swap(ind1, ind2);
	if (ind3 == 0 && ind4 == face2->numberOfPoints() - 1)
		swap(ind3, ind4);
	// remove all references to face1
	for (size_t i=0; i<face1->numberOfPoints(); ++i)
		face1->getPoint(i)->deleteFace(face1);
	// remove all references to face2
	for (size_t i=0; i<face2->numberOfPoints(); ++i)
		face2->getPoint(i)->deleteFace(face2);
	// add the new face
	SubdivisionControlFace* newface = SubdivisionControlFace::construct(this);
	newface->setLayer(layer);
	addControlFace(newface);
	for (size_t i=0; i<=ind1; ++i) {
        if (!newface->hasPoint(face1->getPoint(i)))
            newface->addPoint(face1->getPoint(i));
    }
	for (size_t i=ind4; i<face2->numberOfPoints(); ++i) {
        if (!newface->hasPoint(face2->getPoint(i)))
            newface->addPoint(face2->getPoint(i));
    }
	for (size_t i=0; i<=ind3; ++i) {
        if (!newface->hasPoint(face2->getPoint(i)))
            newface->addPoint(face2->getPoint(i));
    }
	for (size_t i=ind2; i<face1->numberOfPoints(); ++i) {
        if (!newface->hasPoint(face1->getPoint(i)))
            newface->addPoint(face1->getPoint(i));
    }
	// check all appropriate points are added
	if (newface->numberOfPoints() != face1->numberOfPoints() + face2->numberOfPoints() - 2)
		throw runtime_error("wrong number of points in SubdivisionControlEdge::collapse");
	p1 = newface->getPoint(newface->numberOfPoints() - 1);
	for (size_t i=0; i<newface->numberOfPoints(); ++i) {
		p2 = newface->getPoint(i);
		SubdivisionEdge* edge = edgeExists(p1, p2);
		if (edge != 0) {
			if (edge->hasFace(face1))
				edge->deleteFace(face1);
			if (edge->hasFace(face2))
				edge->deleteFace(face2);
			edge->addFace(newface);
			if (edge->numberOfFaces() < 2)
				edge->setCrease(true);
		}
		p1 = p2;
	}
	// connect the new face to a layer
	layer->useControlFace(newface);
	if (collapseedge->isCrease())
		collapseedge->setCrease(false);
	collapseedge->startPoint()->deleteEdge(collapseedge);
	collapseedge->endPoint()->deleteEdge(collapseedge);

	deleteControlFace(face1);
	deleteControlFace(face2);

	// check if startpoint and endpoint can be collapsed as well
    if (s != 0 && s->numberOfFaces() > 1 && s->numberOfEdges() == 2)
		s->collapse();
    if (e != 0 && e->numberOfFaces() > 1 && e->numberOfEdges() == 2)
		e->collapse();
    
    deleteControlEdge(collapseedge);
    deleteElementsCollection();
	setBuild(false);
}

void SubdivisionSurface::loadBinary(FileBuffer &source)
{
    // first load layerdata
    quint32 n;
    source.load(n);
    if (n != 0) {
        // delete current layers and load new ones
        _layer_pool.clear();
        _layers.clear();
        _layers.reserve(n);
        for (size_t i=0; i<n; ++i) {
            SubdivisionLayer* layer = addNewLayer();
            layer->loadBinary(source);
        }
    }
    else {
        // no layers in the file, so keep the current default one
    }
    // read index of active layer
    source.load(n);
    setActiveLayer(_layers[n]);
    // read control points
    source.load(n);
    _control_points.reserve(n);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlPoint* point = SubdivisionControlPoint::construct(this);
        _control_points.push_back(point);
        point->load_binary(source);
    }
    // read control edges
    source.load(n);
    _control_edges.reserve(n);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlEdge* edge = SubdivisionControlEdge::construct(this);
        _control_edges.push_back(edge);
        edge->loadBinary(source);
    }
    if (source.getVersion() >= fv195) {
        // load control curves
        source.load(n);
        _control_curves.reserve(n);
        for (size_t i=0; i<n; ++i) {
            SubdivisionControlCurve* curve = SubdivisionControlCurve::construct(this);
            _control_curves.push_back(curve);
            curve->loadBinary(source);
        }
    }
    // read control faces
    source.load(n);
    _control_faces.reserve(n);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlFace* face = SubdivisionControlFace::construct(this);
        _control_faces.push_back(face);
        face->loadBinary(source);
    }
    setBuild(false);
    _initialized = true;
}

void SubdivisionSurface::loadFromStream(size_t& lineno, QStringList& strings)
{
    // first read layerdata
    QString str = strings[lineno++].trimmed();
    size_t start = 0;
    size_t n = ReadIntFromStr(lineno, str, start);
    if (n > 0) {
        // delete current layers and load new ones
        for (size_t i=0; i<_layers.size(); ++i) {
            deleteLayer(_layers[i]);
        }
        for (size_t i=0; i<n; ++i) {
            SubdivisionLayer* layer = addNewLayer();
            layer->loadFromStream(lineno, strings);
        }
    }
    // read index of active layer
    str = strings[lineno++].trimmed();
    start = 0;
    n = ReadIntFromStr(lineno, str, start);
    _active_layer = _layers[n];

    // read controlpoints
    str = strings[lineno++].trimmed();
    start = 0;
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlPoint* point = SubdivisionControlPoint::construct(this);
        _control_points.push_back(point);
        point->loadFromStream(lineno, strings);
    }
    // read control edges
    str = strings[lineno++].trimmed();
    start = 0;
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlEdge* edge = SubdivisionControlEdge::construct(this);
        _control_edges.push_back(edge);
        edge->loadFromStream(lineno, strings);
    }
    // read control faces
    str = strings[lineno++].trimmed();
    start = 0;
    n = ReadIntFromStr(lineno, str, start);
    for (size_t i=0; i<n; ++i) {
        SubdivisionControlFace* face = SubdivisionControlFace::construct(this);
        _control_faces.push_back(face);
        face->loadFromStream(lineno, strings);
    }
    setBuild(false);
    _initialized = true;
}

void SubdivisionSurface::rebuild()
{
    if (!_initialized)
        initialize(1,1);
    if (numberOfControlFaces() > 0) {
        for (size_t i=0; i<numberOfControlCurves(); ++i) {
            SubdivisionControlCurve* curve = _control_curves[i];
            if (_current_subdiv_level == 0)
                curve->resetDivPoints();
        }
        _build = true;
        while (_current_subdiv_level < _desired_subdiv_level)
            subdivide();
        for (size_t i=0; i<numberOfControlFaces(); ++i) {
            getControlFace(i)->calcExtents();
            if (i == 0) {
                _min = getControlFace(i)->getMin();
                _max = getControlFace(i)->getMax();
            }
            else {
                MinMax(getControlFace(i)->getMin(), _min, _max);
                MinMax(getControlFace(i)->getMax(), _min, _max);
            }
        }
        for (size_t i=0; i<numberOfControlCurves(); ++i) {
            SubdivisionControlCurve* curve = getControlCurve(i);
            curve->getSpline()->clear();
            for (size_t j=0; j<curve->numberOfSubdivPoints(); ++j) {
                SubdivisionPoint* point = curve->getSubdivPoint(j);
                curve->getSpline()->add(point->getCoordinate());
                if (j > 0 && j < curve->numberOfSubdivPoints()) {
                    if (point->getVertexType() == svCorner)
                        curve->getSpline()->setKnuckle(j, true);
                    else {
                        SubdivisionEdge* edge1 = edgeExists(curve->getSubdivPoint(j-1), curve->getSubdivPoint(j));
                        SubdivisionEdge* edge2 = edgeExists(curve->getSubdivPoint(j), curve->getSubdivPoint(j-1));
                        if (!edge1->isCrease() && !edge2->isCrease())
                            curve->getSpline()->setKnuckle(j, point->getVertexType() == svCrease);
                    }
                }
                curve->setBuild(true);
            }
        }
    }
    else if (numberOfControlPoints() > 0) {
        for (size_t i=0; i<numberOfControlPoints(); ++i) {
            if (i == 0) {
                _min = getControlPoint(i)->getCoordinate();
                _max = _min;
            }
            else
                MinMax(getControlPoint(i)->getCoordinate(), _min, _max);
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
    destination.add(static_cast<quint32>(numberOfLayers()));
    for (size_t i=0; i<numberOfLayers(); ++i)
        getLayer(i)->saveBinary(destination);
    // save index of active layer
    destination.add(static_cast<quint32>(getActiveLayer()->getLayerIndex()));
    // first sort controlpoints for faster access of function
    sort(_control_points.begin(), _control_points.end());
    destination.add(static_cast<quint32>(numberOfControlPoints()));
    for (size_t i=0; i<numberOfControlPoints(); ++i)
        getControlPoint(i)->save_binary(destination);
    // save control edges
    destination.add(numberOfControlEdges());
    for (size_t i=0; i<numberOfControlEdges(); ++i)
        getControlEdge(i)->saveBinary(destination);
    if (destination.getVersion() >= fv195) {
        destination.add(numberOfControlCurves());
        for (size_t i=0; i<numberOfControlCurves(); ++i)
            getControlCurve(i)->saveBinary(destination);
    }
    // save control faces
    destination.add(numberOfControlFaces());
    for (size_t i=0; i<numberOfControlFaces(); ++i)
        getControlFace(i)->saveBinary(destination);
}

void SubdivisionSurface::saveToStream(QStringList& strings)
{
    // first save layerdata
    strings.push_back(QString("%1").arg(numberOfLayers()));
    for (size_t i=0; i<numberOfLayers(); ++i)
        getLayer(i)->saveToStream(strings);
    // save index of active layer
    strings.push_back(QString("%1").arg(indexOfLayer(_active_layer)));
    // BUGBUG: sort controlpoints for faster access to function indexof
    strings.push_back(QString("%1").arg(numberOfControlPoints()));
    for (size_t i=0; i<numberOfControlPoints(); ++i)
        getControlPoint(i)->saveToStream(strings);
    strings.push_back(QString("%1").arg(numberOfControlEdges()));
    for (size_t i=0; i<numberOfControlEdges(); ++i)
        getControlEdge(i)->saveToStream(strings);
    strings.push_back(QString("%1").arg(numberOfControlFaces()));
    for (size_t i=0; i<numberOfControlFaces(); ++i)
        getControlFace(i)->saveToStream(strings);
}

void SubdivisionSurface::deleteSelected()
{
    // first controlcurves, then faces, edges, and points
    size_t i = numberOfSelectedControlCurves();
    while (i >= 1) {
        delete getControlCurve(i-1);
        --i;
        if (i > numberOfSelectedControlCurves())
            i = numberOfSelectedControlCurves();
    }
    for (set<SubdivisionControlFace*>::iterator j=_sel_control_faces.begin(); j!=_sel_control_faces.end(); ++j) {
        deleteControlFace(*j);
    }
    _sel_control_faces.clear();
    for (set<SubdivisionControlEdge*>::iterator k=_sel_control_edges.begin(); k!=_sel_control_edges.end(); ++k) {
        deleteControlEdge(*k);
    }
    _sel_control_edges.clear();
    for (OrderedPointMap::iterator l=_sel_control_points.begin(); l!=_sel_control_points.end(); ++l) {
        deleteControlPoint(*l);
    }
    _sel_control_points.clear();
    deleteElementsCollection();
    setBuild(false);
}

// edges seem to be usually looked up by points or the edge itself, it might make
// sense to use a map(s)
void SubdivisionSurface::subdivide()
{
    if (numberOfControlFaces() < 1)
        return;
    ++_current_subdiv_level;
    vector<SubdivisionEdge*> newedgelist;
    size_t number = numberOfFaces();

    // create the list with new facepoints and a reference to the original face
    vector<pair<SubdivisionFace*,SubdivisionPoint*> > facepoints;
    // create the list with new edgepoints and a reference to the original edge
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> > edgepoints;
    // create the list with new vertexpoints and a reference to the original vertex
    vector<pair<SubdivisionPoint*,SubdivisionPoint*> > vertexpoints;

    if (number == 0) {
        facepoints.reserve(numberOfControlFaces());
        for (size_t i=0; i<numberOfControlFaces(); ++i) {
            SubdivisionPoint *pt = getControlFace(i)->calculateFacePoint();
            if (pt)
                facepoints.push_back(make_pair(getControlFace(i), pt));
        }
    }
    else {
        facepoints.reserve(4*numberOfControlFaces());
        for (size_t i=0; i<numberOfControlFaces(); ++i) {
            SubdivisionControlFace* ctrlface = getControlFace(i);
            for (size_t j=0; j<ctrlface->numberOfChildren(); ++j) {
                SubdivisionPoint *pt = ctrlface->getChild(j)->calculateFacePoint();
                if (pt)
                    facepoints.push_back(make_pair(ctrlface->getChild(j), pt));
            }
            for (size_t j=0; j<ctrlface->numberOfEdges(); ++j)
                edgepoints.push_back(make_pair(ctrlface->getEdge(j), ctrlface->getEdge(j)->calculateEdgePoint()));
        }
    }
    // calculate other edgepoints
    if (numberOfEdges() == 0) {
        edgepoints.reserve(edgepoints.size() + numberOfControlEdges());
        for (size_t i=0; i<numberOfControlEdges(); ++i) {
            edgepoints.push_back(make_pair(getControlEdge(i), getControlEdge(i)->calculateEdgePoint()));
        }
    }
    else {
        edgepoints.reserve(edgepoints.size() + numberOfEdges());
        for (size_t i=0; i<numberOfEdges(); ++i) {
            edgepoints.push_back(make_pair(getEdge(i), getEdge(i)->calculateEdgePoint()));
        }
    }
    // calculate vertex points
    if (numberOfPoints() == 0) {
        vertexpoints.reserve(vertexpoints.size() + numberOfControlPoints());
        for (size_t i=0; i<numberOfControlPoints(); ++i)
            vertexpoints.push_back(make_pair(getControlPoint(i), getControlPoint(i)->calculateVertexPoint()));
    }
    else {
        vertexpoints.reserve(vertexpoints.size() + numberOfPoints());
        for (size_t i=0; i<numberOfPoints(); ++i)
            vertexpoints.push_back(make_pair(getPoint(i), getPoint(i)->calculateVertexPoint()));
    }
    // sort the points for faster access
    // vertexpoints.sort
    // edgepoints.sort
    // facepoints.sort

    // finally create the refined mesh over the newly created vertexpoints, edgepoints, and facepoints
    for (size_t i=0; i<numberOfControlFaces(); ++i) {
        getControlFace(i)->subdivide(vertexpoints, edgepoints, facepoints, newedgelist);
    }

    // delete the edges that are in the list, not dumping the pool, as we have new edges that
    // are in the pool that we want to keep
    for (size_t i=0; i<_edges.size(); ++i) {
      deleteEdge(_edges[i]);
    }
    _edges = newedgelist;
    // delete the points that are in the list, don't dump the pool
    for (size_t i=0; i<_points.size(); ++i) {
      deletePoint(_points[i]);
    }
    _points.reserve(vertexpoints.size() + edgepoints.size() + facepoints.size());
    for (size_t i=0; i<vertexpoints.size(); ++i)
        if (vertexpoints[i].first != 0)
            _points.push_back(vertexpoints[i].second);
    for (size_t i=0; i<edgepoints.size(); ++i)
        if (edgepoints[i].first != 0)
            _points.push_back(edgepoints[i].second);
    for (size_t i=0; i<facepoints.size(); ++i)
        if (facepoints[i].first != 0)
            _points.push_back(facepoints[i].second);
    // perform averaging procedure to smooth the new mesh
    vector<QVector3D> tmppoints;
    tmppoints.reserve(_points.size());
    // make a copy of all points, average them, put em back
    for (size_t i=0; i<_points.size(); ++i) {
        tmppoints.push_back(_points[i]->averaging());
    }
    for (size_t i=0; i<_points.size(); ++i) {
        _points[i]->setCoordinate(tmppoints[i]);
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
    QString s(prefix);
    s.append(" ");
    const char* np = s.toStdString().c_str();
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
    if (g_surface_verbose) {
        for (size_t i=0; i<_points.size(); ++i) {
            _points[i]->dump(os, np);
            os << "\n";
        }
    }
    os << prefix << " Edges (" << _edges.size() << ")\n";
}

ostream& operator << (ostream& os, const ShipCAD::SubdivisionSurface& surface)
{
    surface.dump(os);
    return os;
}
