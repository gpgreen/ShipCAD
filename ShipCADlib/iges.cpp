/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2018, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                               *
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

#include "iges.h"
#include "nurbsurface.h"
#include "shipcadmodel.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivlayer.h"

using namespace ShipCAD;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////

static QString ConvertString(const QString& input)
{
    if (input.size() == 0)
        return input;
    return QString("%1H%2").arg(input.size()).arg(input);
}

static QString CheckString(const QString& str, size_t max_len, char sectionchar, size_t index)
{
    size_t l = str.size();
    if (l > max_len)
    {
        size_t c = max_len - l;
    }
    QString result = QString("%1%2").arg(str).arg(sectionchar);
    QString lenstr = QString("%1").arg(index);
    
}

//////////////////////////////////////////////////////////////////////////////////////

IGES::IGES(ShipCADModel* model, bool minimize_faces, bool send_triangles)
    : _num_surfaces(0), _max_coordinate(0.0), _model(model), _minimize_faces(minimize_faces),
      _send_triangles(send_triangles)
{
}

void IGES::addEntity128(NURBSurface& nurb, size_t color_index)
{
}

size_t IGES::addEntity314(QColor col)
{
}

// FreeShipUnit.pas:6262
static SubdivisionPoint* FindFourthPoint(SubdivisionPoint* p1,
                                         SubdivisionPoint* p2,
                                         SubdivisionPoint* p3)
{
    SubdivisionPoint* result = nullptr;
    if (p1 == p2 || p2 == p3 || p1 == p3)
        return result;
    for (size_t i=0; i<p2->numberOfFaces(); ++i)
    {
        SubdivisionFace* face = p2->getFace(i);
        size_t numpts = face->numberOfPoints();
        if (face->indexOfPoint(p1) != numpts
            && face->indexOfPoint(p3) != numpts)
        {
            vector<SubdivisionPoint*> pts;
            for (size_t j=0; j<face->numberOfPoints(); ++j)
                pts.push_back(face->getPoint(j));
            vector<SubdivisionPoint*>::iterator k = find(pts.begin(),
                                                         pts.end(),
                                                         p1);
            if (k != pts.end())
                pts.erase(k);
            k = find(pts.begin(), pts.end(), p2);
            if (k != pts.end())
                pts.erase(k);
            k = find(pts.begin(), pts.end(), p3);
            if (k != pts.end())
                pts.erase(k);
            if (pts.size() > 0)
                result = pts.front();
        }
    }
    return result;
}

// FreeShipUnit.pas:6292
static std::pair<SubdivisionPoint*, SubdivisionPoint*>
FindOpposingPoints(SubdivisionSurface* surface,
                   SubdivisionPoint* p1,
                   SubdivisionPoint* p2,
                   SubdivisionPoint* p3,
                   SubdivisionPoint* p4)
{
    pair<SubdivisionPoint*, SubdivisionPoint*> result =
        make_pair(nullptr, nullptr);
    SubdivisionEdge* edge = surface->edgeExists(p2, p3);
    if (edge == nullptr || edge->isCrease())
        return result;
    for (size_t i=0; i<edge->numberOfFaces(); ++i)
    {
        SubdivisionFace* face = edge->getFace(i);
        size_t numpts = face->numberOfPoints();
        if (face->indexOfPoint(p1) == numpts
            || face->indexOfPoint(p2) == numpts
            || face->indexOfPoint(p3) == numpts
            || face->indexOfPoint(p4) == numpts)
        {
            size_t index = face->indexOfPoint(p3);
            if (index != numpts)
            {
                index = (index + 1) % numpts;
                if (face->getPoint(index) == p2)
                {
                    // face is oriented CCW
                    index = (index + 1) % numpts;
                    result.first = face->getPoint(index);
                    index = (index + 1) % numpts;
                    result.second = face->getPoint(index);
                } else {
                    index = face->indexOfPoint(p2);
                    if (index != numpts)
                    {
                        index = (index + 1) % numpts;
                        if (face->getPoint(index) == p3)
                        {
                            // face is oriented CW
                            index = (index + 1) % numpts;
                            result.second = face->getPoint(index);
                            index = (index + 1) % numpts;
                            result.first = face->getPoint(index);
                        }
                    }
                }
            }
            break;
        }
    }
    return result;
}

// FreeShipUnit.pas:6345
static void AssembleTriangle(SubdivisionSurface* surface,
                             Grid<SubdivisionPoint*>& grid,
                             SubdivisionControlFace* face)
{
    // initialize grid
    grid = Grid<SubdivisionPoint*>(0, 0);
    vector<SubdivisionPoint*> points;
    vector<SubdivisionFace*> faces;
    for (size_t i=0; i<face->numberOfChildren(); ++i)
    {
        SubdivisionFace* child = face->getChild(i);
        faces.push_back(child);
        for (size_t j=0; j<child->numberOfPoints(); ++j)
        {
            SubdivisionPoint* p = child->getPoint(j);
            vector<SubdivisionPoint*>::iterator k = find(
                points.begin(), points.end(), p);
            if (k == points.end())
                points.push_back(p);
        }
    }
    // TODO: faces sort ? how does this work
    // try to isolate the interior point
    SubdivisionPoint* interior_point = nullptr;
    for (size_t i=0; i<points.size(); ++i)
    {
        SubdivisionPoint* p = points[i];
        // check how many of the childfaces are connected
        // (interior point must have 3)
        size_t n = 0;
        for (size_t j=0; j<p->numberOfFaces(); ++j)
        {
            SubdivisionFace* f = p->getFace(j);
            vector<SubdivisionFace*>::iterator k = find(
                faces.begin(), faces.end(), f);
            if (k != faces.end())
                ++n;
        }
        if (n == 3)
        {
            interior_point = p;
            break;
        }
    }
    if (interior_point == nullptr)
    {
        // message  99
        return;
    }
    grid.setRows(3);
    grid.setCols(3);
    grid.set(1, 1, interior_point);
    SubdivisionFace* child = face->getChild(2);
    size_t index = child->indexOfPoint(interior_point);
    grid.set(1, 0, child->getPoint((index+1) % 4));
    grid.set(0, 0, child->getPoint((index+2) % 4));
    grid.set(0, 1, child->getPoint((index+3) % 4));
    pair<SubdivisionPoint*, SubdivisionPoint*> op =
        FindOpposingPoints(surface,
                           grid.get(0, 0), grid.get(0, 1),
                           grid.get(1, 1), grid.get(1, 0));
    grid.set(0, 2, op.first);
    grid.set(1, 2, op.second);
    op = FindOpposingPoints(surface,
                            grid.get(0, 1), grid.get(1, 1),
                            grid.get(1, 0), grid.get(0, 0));
    grid.set(2, 1, op.first);
    grid.set(2, 0, op.second);
    grid.set(2, 2, grid.get(2, 1));
    for (size_t i=0; i<3; ++i)
        for (size_t j=0; j<3; ++j)
            if (grid.get(i,j) == nullptr)
            {
                //raise exception userstring 98
            }
}

// FreeShipUnit.pas:6434
static QVector3D PhantomPoint(const QVector3D& borderpoint,
                              const QVector3D& innerpoint)
{
    return 2 * borderpoint - innerpoint;
}

// FreeShipUnit.pas:6441
static QVector3D CornerPoint(
    const QVector3D& p5, const QVector3D& p6,
    const QVector3D& p8, const QVector3D& p9)
{
    QVector3D p2 = PhantomPoint(p5, p8);
    QVector3D p3 = PhantomPoint(p6, p9);
    QVector3D p4 = PhantomPoint(p5, p6);
    QVector3D p7 = PhantomPoint(p8, p9);
    return 20*p5 - 4*p2 - p3 - 4*p4 - 4*p6 - p7 - 4*p8 - p9;
}

// FreeShipUnit.pas:6416
void IGES::processGrid(Grid<SubdivisionPoint*>& grid, size_t color_index)
{
    SubdivisionSurface* surface = _model->getSurface();
    
    vector<SubdivisionPoint*> bottomrow;
    vector<SubdivisionPoint*> toprow;
    vector<SubdivisionPoint*> leftcolumn;
    vector<SubdivisionPoint*> rightcolumn;
    if (grid.cols() == 0 || grid.rows() == 0)
        return;
    bottomrow.resize(grid.cols());
    toprow.resize(grid.cols());
    leftcolumn.resize(grid.rows());
    rightcolumn.resize(grid.rows());

    // assemble bottom row to set tangency
    bool bottompresent = true;
    for (size_t i=1; i<grid.cols(); ++i)
    {
        pair<SubdivisionPoint*, SubdivisionPoint*> op =
            FindOpposingPoints(surface,
                               grid.get(grid.rows()-2, i-1),
                               grid.get(grid.rows()-1, i-1),
                               grid.get(grid.rows()-1, i),
                               grid.get(grid.rows()-2, i));
        bottomrow[i-1] = op.first;
        bottomrow[i] = op.second;
        if (bottomrow[i-1] == nullptr || bottomrow[i] == nullptr)
            bottompresent = false;
    }
    // assemble right column to set tangency
    bool rightpresent = true;
    for (size_t i=1; i<grid.rows(); ++i)
    {
        pair<SubdivisionPoint*, SubdivisionPoint*> op =
            FindOpposingPoints(surface,
                               grid.get(i, grid.cols()-2),
                               grid.get(i, grid.cols()-1),
                               grid.get(i-1, grid.cols()-1),
                               grid.get(i-1, grid.cols()-2));
        rightcolumn[i] = op.first;
        rightcolumn[i-1] = op.second;
        if (rightcolumn[i-1] == nullptr || rightcolumn[i] == nullptr)
            rightpresent = false;
    }
    // assemble top row
    bool toppresent = true;
    for (size_t i=1; i<grid.cols(); ++i)
    {
        pair<SubdivisionPoint*, SubdivisionPoint*> op =
            FindOpposingPoints(surface,
                               grid.get(1, i),
                               grid.get(0, i),
                               grid.get(0, i-1),
                               grid.get(1, i-1));
        toprow[i] = op.first;
        toprow[i-1] = op.second;
        if (toprow[i-1] == nullptr || toprow[i] == nullptr)
            toppresent = false;
    }
    // assemble left column
    bool leftpresent = true;
    for (size_t i=1; i<grid.rows(); ++i)
    {
        pair<SubdivisionPoint*, SubdivisionPoint*> op =
            FindOpposingPoints(surface,
                               grid.get(i-1, 1),
                               grid.get(i-1, 0),
                               grid.get(i, 0),
                               grid.get(i, 1));
        leftcolumn[i-1] = op.first;
        leftcolumn[i] = op.second;
        if (leftcolumn[i-1] == nullptr || leftcolumn[i] == nullptr)
            leftpresent = false;
    }
    SubdivisionPoint* bottomleft = nullptr;
    SubdivisionPoint* bottomright = nullptr;
    SubdivisionPoint* topleft = nullptr;
    SubdivisionPoint* topright = nullptr;

    if (bottompresent && leftpresent)
        bottomleft = FindFourthPoint(leftcolumn[grid.rows()-1],
                                     grid.get(grid.rows()-1, 0),
                                     bottomrow[0]);
    if (bottompresent && rightpresent)
        bottomright = FindFourthPoint(rightcolumn[grid.rows()-1],
                                      grid.get(grid.rows()-1, grid.cols()-1),
                                      bottomrow[grid.cols()-1]);
    if (toppresent && leftpresent)
        topleft = FindFourthPoint(leftcolumn[0],
                                  grid.get(0, 0),
                                  toprow[0]);
    if (toppresent && rightpresent)
        topright = FindFourthPoint(rightcolumn[0],
                                   grid.get(0, grid.cols()-1),
                                   toprow[grid.cols()-1]);
    NURBSurface nurb(grid.rows() + 2, grid.cols() + 2);
    nurb.set(grid);

    if (_send_triangles)
    {
        // check for special triangle case
        if ((grid.get(grid.rows()-1, grid.cols()-1) == grid.get(grid.rows()-1, grid.cols()-2))
            && (grid.get(grid.rows()-1, grid.cols()-1) == grid.get(grid.rows()-2, grid.cols()-1)))
        {
            nurb.setPoint(grid.cols(), grid.rows(),
                          grid.get(grid.rows()-1, grid.cols()-1)->getLimitPoint());
            nurb.setPoint(grid.cols()-1, grid.rows(),
                          grid.get(grid.rows()-1, grid.cols()-1)->getLimitPoint());
            nurb.setPoint(grid.cols(), grid.rows()-1,
                          grid.get(grid.rows()-1, grid.cols()-1)->getLimitPoint());
            // TODO: some funny code here (line 6520), clear the last row?
        }
    }
    if (toppresent)
    {
        for (size_t i=1; i<=grid.cols(); ++i)
            nurb.setPoint(i, 0, toprow[i-1]->getCoordinate());
    } else {
        for (size_t i=1; i<=grid.cols(); ++i)
            nurb.setPoint(i, 0, PhantomPoint(nurb.getPoint(i, 1),
                                             nurb.getPoint(i, 2)));
    }
    if (bottompresent)
    {
        for (size_t i=1; i<=grid.cols(); ++i)
            nurb.setPoint(i, grid.rows()+1, bottomrow[i-1]->getCoordinate());
    } else {
        for (size_t i=1; i<=grid.cols(); ++i)
            nurb.setPoint(i, grid.rows()+1,
                          PhantomPoint(nurb.getPoint(i, grid.rows()),
                                       nurb.getPoint(i, grid.rows()-1)));
    }
    if (leftpresent)
    {
        for (size_t i=1; i<=grid.rows(); ++i)
            nurb.setPoint(0, i, leftcolumn[i-1]->getCoordinate());
    } else {
        for (size_t i=1; i<=grid.rows(); ++i)
            nurb.setPoint(0, i,
                          PhantomPoint(nurb.getPoint(1, i),
                                       nurb.getPoint(2, i)));
    }
    if (rightpresent)
    {
        for (size_t i=1; i<=grid.rows(); ++i)
            nurb.setPoint(grid.cols()+1, i, rightcolumn[i-1]->getCoordinate());
    } else {
        for (size_t i=1; i<=grid.rows(); ++i)
            nurb.setPoint(grid.cols()+1, i,
                          PhantomPoint(nurb.getPoint(grid.cols(), i),
                                       nurb.getPoint(grid.cols()-1, i)));
    }
    if (topleft != nullptr)
    {
        nurb.setPoint(0, 0, topleft->getCoordinate());
    } else {
        nurb.setPoint(0, 0,
                      CornerPoint(nurb.getPoint(1, 1), nurb.getPoint(2, 1),
                                  nurb.getPoint(1, 2), nurb.getPoint(2, 2)));
    }
    if (topright != nullptr)
    {
        nurb.setPoint(nurb.cols()-1, 0, topright->getCoordinate());
    } else {
        nurb.setPoint(nurb.cols()-1, 0,
                      CornerPoint(nurb.getPoint(nurb.cols()-2, 1),
                                  nurb.getPoint(nurb.cols()-3, 1),
                                  nurb.getPoint(nurb.cols()-2, 2),
                                  nurb.getPoint(nurb.cols()-3, 2)));
    }
    if (bottomleft != nullptr)
    {
        nurb.setPoint(0, nurb.rows()-1, bottomleft->getCoordinate());
    } else {
        nurb.setPoint(0, nurb.rows()-1,
                      CornerPoint(nurb.getPoint(1, nurb.rows()-2),
                                  nurb.getPoint(2, nurb.rows()-2),
                                  nurb.getPoint(1, nurb.rows()-3),
                                  nurb.getPoint(2, nurb.rows()-3)));
    }
    if (bottomright != nullptr)
    {
        nurb.setPoint(nurb.cols()-1, nurb.rows()-1, bottomright->getCoordinate());
    } else {
        nurb.setPoint(nurb.cols()-1, nurb.rows()-1,
                      CornerPoint(nurb.getPoint(nurb.cols()-2, nurb.rows()-2),
                                  nurb.getPoint(nurb.cols()-3, nurb.rows()-2),
                                  nurb.getPoint(nurb.cols()-2, nurb.rows()-3),
                                  nurb.getPoint(nurb.cols()-3, nurb.rows()-3)));
    }
    
    nurb.setColDegree(3);
    nurb.setRowDegree(3);
    nurb.setUniformColKnotVector();
    nurb.setUniformRowKnotVector();

    // insert knots to force the patch to interpolate start and endknots
    for (size_t i=0; i<nurb.getColDegree(); ++i)
        nurb.insertColKnot(nurb.getColKnotVector(nurb.getColDegree()));
    for (size_t i=0; i<nurb.getColDegree(); ++i)
        nurb.insertColKnot(nurb.getColKnotVector(nurb.cols()));
    for (size_t i=0; i<nurb.getRowDegree(); ++i)
        nurb.insertRowKnot(nurb.getRowKnotVector(nurb.getRowDegree()));
    for (size_t i=0; i<nurb.getRowDegree(); ++i)
        nurb.insertRowKnot(nurb.getRowKnotVector(nurb.rows()));
    // delete old startpoints
    for (size_t i=0; i<nurb.getColDegree(); ++i)
        nurb.deleteColumn(0);
    for (size_t i=0; i<nurb.getColDegree(); ++i)
        nurb.deleteColumn(nurb.cols() - 1);
    for (size_t i=0; i<nurb.getRowDegree(); ++i)
        nurb.deleteRow(0);
    for (size_t i=0; i<nurb.getRowDegree(); ++i)
        nurb.deleteColumn(nurb.rows() - 1);
    // set knotvectors to open-knot vector type (standard interpolating form)
    nurb.setDefaultColKnotVector();
    nurb.setDefaultRowKnotVector();

    // check all cornerpoints for irregular vertices
    
    nurb.setPoint(0, 0, grid.get(0, 0)->getLimitPoint());
    nurb.setPoint(grid.cols()+1, 0, grid.get(0, grid.cols()-1)->getLimitPoint());
    nurb.setPoint(0, grid.rows()+1, grid.get(grid.rows()-1, 0)->getLimitPoint());
    nurb.setPoint(grid.cols()+1, grid.rows()+1, grid.get(grid.rows()-1, grid.cols()-1)->getLimitPoint());
    addEntity128(nurb, color_index);
    if (_model->getVisibility().getModelView() == mvBoth)
    {
        NURBSurface nurb2(nurb.rows(), nurb.cols());
        nurb2.setColDegree(nurb.getColDegree());
        nurb2.setRowDegree(nurb.getRowDegree());
        for (size_t i=nurb.cols(); i>=1; --i)
        {
            for (size_t j=nurb.rows(); j>=1; --j)
            {
                QVector3D p = nurb.getPoint(i-1, j-1);
                p.setY(-p.y());
                nurb2.setPoint(nurb2.cols()-i, nurb2.rows()-j, p);
            }
            nurb2.setDefaultColKnotVector();
            nurb2.setDefaultRowKnotVector();
            addEntity128(nurb2, color_index);
        }
    }
}

void IGES::saveToFile(const QString& filename)
{
    SubdivisionSurface* surface = _model->getSurface();
    
    int prev_level = surface->getDesiredSubdivisionLevel();
    size_t nsurfaces = 0;
    surface->setDesiredSubdivisionLevel(1);
    surface->setSubdivisionMode(fmCatmullClark);
    if (!surface->isBuild())
        surface->rebuild();
    // build color table
    vector<SubdivisionLayer*> layers;
    for (size_t i=0; i<surface->numberOfLayers(); ++i)
    {
        SubdivisionLayer* layer = surface->getLayer(i);
        if (layer->isVisible() && layer->numberOfFaces() > 0)
            layers.push_back(layer);
    }
    if (_minimize_faces)
    {
    } else {
    }
    if (nsurfaces > 0)
    {
        // save file
    }
    surface->setDesiredSubdivisionLevel(prev_level);
    surface->setSubdivisionMode(fmQuadTriangle);
    if (!surface->isBuild())
        surface->rebuild();
}

void IGES::processParameterData(const QString& str, QStringList& param_data)
{
}

void IGES::clear()
{
}

void IGES::dump(ostream& os) const
{
    os << "IGES";
}

ostream& operator << (ostream& os, const ShipCAD::IGES& iges)
{
    iges.dump(os);
    return os;
}
