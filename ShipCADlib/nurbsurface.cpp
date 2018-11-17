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

#include "nurbsurface.h"
#include "viewport.h"

using namespace ShipCAD;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////

NURBSurface::NURBSurface()
    : _build(false)
{
	// does nothing
}

void NURBSurface::setBuild(bool val)
{
    if (!val) {
        _build = val;
    }
}

// FreeShipUnit.pas:8012
void NURBSurface::setColDegree(size_t val)
{
    if (val > 5)
        val = 5;
    if (val != _col_degree)
    {
        _col_degree = val;
        _build = false;
    }
}

// FreeShipUnit.pas:8022
void NURBSurface::setRowDegree(size_t val)
{
    if (val > 5)
        val = 5;
    if (val != _row_degree)
    {
        _row_degree = val;
        _build = false;
    }
}

// FreeShipUnit.pas:8314
void NURBSurface::rebuild()
{
    _build = false;
    if (_col_degree > cols() - 1)
        _col_degree = cols() - 1;
    if (_row_degree > rows() - 1)
        _row_degree = rows() - 1;
    setDefaultColKnotVector();
    setDefaultRowKnotVector();
    _build = true;
}

// FreeShipUnit.pas:8032
void NURBSurface::setDefaultColKnotVector()
{
    if (_col_degree > cols() - 1)
        _col_degree = cols() - 1;
    size_t l = cols() + _col_degree + 1;
    size_t no = (cols() + _col_degree + 1) - 2 * _col_degree;
    _col_knots.resize(l, 0.0);
    for (size_t i=0; i<_col_degree; ++i)
        _col_knots[i] = 0.0;
    for (size_t i=0; i<no; ++i)
    {
        _col_knots[i + _col_degree] = i / (no-1);
        if (_col_knots[i] < 0.0)
            _col_knots[i] = 0.0;
        else if (_col_knots[i] > 1.0)
            _col_knots[i] = 1.0;
    }
    for (size_t i=0; i<_col_degree; ++i)
        _col_knots[_col_degree + no + i] = 1.0;
}

// FreeShipUnit.pas:8050
void NURBSurface::setDefaultRowKnotVector()
{
    if (_row_degree > rows() - 1)
        _row_degree = rows() - 1;
    size_t l = rows() + _row_degree + 1;
    size_t no = (rows() + _row_degree + 1) - 2 * _row_degree;
    _row_knots.resize(l, 0.0);
    for (size_t i=0; i<_row_degree; ++i)
        _row_knots[i] = 0.0;
    for (size_t i=0; i<no; ++i)
    {
        _row_knots[i + _row_degree] = i / (no-1);
        if (_row_knots[i] < 0.0)
            _row_knots[i] = 0.0;
        else if (_row_knots[i] > 1.0)
            _row_knots[i] = 1.0;
    }
    for (size_t i=0; i<_row_degree; ++i)
        _row_knots[_row_degree + no + i] = 1.0;
}

// FreeShipUnit.pas:8068
void NURBSurface::setUniformColKnotVector()
{
    if (_col_degree > cols() - 1)
        _col_degree = cols() - 1;
    size_t l = cols() + _col_degree + 1;
    _col_knots.resize(l, 0.0);
    for (size_t i=0; i<l; ++i)
    {
        _col_knots[i] = i / (l-1);
        if (_col_knots[i] < 0.0)
            _col_knots[i] = 0.0;
        else if (_col_knots[i] > 1.0)
            _col_knots[i] = 1.0;
    }
}

// FreeShipUnit.pas:8082
void NURBSurface::setUniformRowKnotVector()
{
    if (_row_degree > rows() - 1)
        _row_degree = rows() - 1;
    size_t l = rows() + _row_degree + 1;
    _row_knots.resize(l, 0.0);
    for (size_t i=0; i<l; ++i)
    {
        _row_knots[i] = i / (l-1);
        if (_row_knots[i] < 0.0)
            _row_knots[i] = 0.0;
        else if (_row_knots[i] > 1.0)
            _row_knots[i] = 1.0;
    }
}

// FreeShipUnit.pas:8148
void NURBSurface::insertColKnot(float u)
{
    if (_col_knots.size() == 0)
        setDefaultColKnotVector();
    _build = true;
    size_t index = _col_knots.size();
    for (size_t i=0; i<_col_knots.size() - 1; ++i)
    {
        if (_col_knots[i] < u && _col_knots[i+1] >= u)
        {
            index = i;
            break;
        }
    }
    if (index != _col_knots.size())
    {
        size_t i = index;
        size_t n = cols();
        size_t k = _col_degree + 1;
        vector<float> alpha(n+1, 0.0);
        Grid<QVector3D> newpoints(rows(), n+1);
        int l = i - k + 1;
        if (l < 0)
            l = 0;
        for (size_t j=0; j<=n; ++j)
        {
            if (j <= l)
            {
                alpha[j] = 1.0;
            }
            else if (l + 1 <= j && j <= i)
            {
                if ((_col_knots[j+k-1] - _col_knots[j]) == 0.0)
                {
                    alpha[j] = 0;
                } else {
                    alpha[j] = (u - _col_knots[j]) / (_col_knots[j+k-1]-_col_knots[j]);
                }
            }
            else
                alpha[j] = 0;
        }
        for (size_t j=0; j<rows(); ++j)
        {
            for (size_t x=0; x <= n; ++x)
            {
                if (alpha[x] == 0.0)
                    newpoints.set(j, x, _points.get(j, x-1));
                else if (alpha[x] == 1.0)
                    newpoints.set(j, x, _points.get(j, x));
                else
                {
                    newpoints.set(j, x, ((1 - alpha[x]) * _points.get(j, x-1) + alpha[x] * _points.get(j, x)));
                }
            }
        }
        // replace the current controlpoints with the new ones
        _points.setRows(rows());
        _points.setCols(cols() + 1);
        for (size_t i=0; i<rows(); ++i)
            _points.grid[i].assign(newpoints.grid[i].begin(), newpoints.grid[i].end());
        // create new knotvector
        _col_knots.insert(_col_knots.begin() + index + 1, u);
    }
}

// FreeShipUnit.pas:8214
void NURBSurface::insertRowKnot(float v)
{
    if (_row_knots.size() == 0)
        setDefaultRowKnotVector();
    _build = true;
    size_t index = _row_knots.size();
    for (size_t i=0; i<_row_knots.size() - 1; ++i)
    {
        if (_row_knots[i] < v && _row_knots[i+1] >= v)
        {
            index = i;
            break;
        }
    }
    if (index != _row_knots.size())
    {
        size_t i = index;
        size_t n = rows();
        size_t k = _row_degree + 1;
        vector<float> alpha(n+1, 0.0);
        Grid<QVector3D> newpoints(rows()+1, n);
        int l = i - k + 1;
        if (l < 0)
            l = 0;
        for (size_t j=0; j<=n; ++j)
        {
            if (j <= l)
            {
                alpha[j] = 1.0;
            }
            else if (l + 1 <= j && j <= i)
            {
                if ((_row_knots[j+k-1] - _row_knots[j]) == 0.0)
                {
                    alpha[j] = 0;
                } else {
                    alpha[j] = (v - _row_knots[j]) / (_row_knots[j+k-1]-_row_knots[j]);
                }
            }
            else
                alpha[j] = 0;
        }
        for (size_t j=0; j<cols(); ++j)
        {
            for (size_t x=0; x <= n; ++x)
            {
                if (alpha[x] == 0.0)
                    newpoints.set(x, j, _points.get(x - 1, j));
                else if (alpha[x] == 1.0)
                    newpoints.set(x, j, _points.get(x, j));
                else
                {
                    newpoints.set(x, j, ((1 - alpha[x]) * _points.get(x - 1, j) + alpha[x] * _points.get(x, j)));
                }
            }
        }
        // replace the current controlpoints with the new ones
        _points.setRows(rows() + 1);
        _points.setCols(cols());
        for (size_t i=0; i<rows(); ++i)
            _points.grid[i].assign(newpoints.grid[i].begin(), newpoints.grid[i].end());
        // create new knotvector
        _row_knots.insert(_row_knots.begin() + index + 1, v);
    }
}

// FreeShipUnit.pas:8282
void NURBSurface::normalizeKnotVectors()
{
    size_t n = _row_knots.size();
    float min, max;
    if (n > 0)
    {
        min = _row_knots.front();
        max = _row_knots.back();
        if (min != max)
        {
            for (size_t i=0; i<n; ++i)
            {
                float newf = (_row_knots[i] - min) / (max - min);
                if (newf > 1.0)
                    newf = 1.0;
                _row_knots[i] = newf;
            }
        }
        n = _col_knots.size();
        if (n > 0)
        {
            min = _col_knots.front();
            max = _col_knots.back();
            if (min != max)
            {
                for (size_t i=0; i<n; ++i)
                {
                    float newf = (_col_knots[i] - min) / (max - min);
                    if (newf > 1.0)
                        newf = 1.0;
                    _col_knots[i] = newf;
                }
            }
        }
    }
}

// FreeShipUnit.pas:8115
void NURBSurface::clear()
{
    _build = false;
    _points.clear();
    _col_degree = 3;
    _row_degree = 3;
}

void NURBSurface::dump(ostream& os) const
{
    os << "NURBSurface";
}

ostream& operator << (ostream& os, const ShipCAD::NURBSurface& surface)
{
    surface.dump(os);
    return os;
}
