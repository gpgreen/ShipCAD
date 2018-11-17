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

#ifndef NURBSURFACE_H_
#define NURBSURFACE_H_

#include <vector>
#include <iosfwd>
#include <QObject>
#include <QVector3D>
#include "grid.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class NURBSurface : public QObject
{
    Q_OBJECT

public:

    explicit NURBSurface();
    virtual ~NURBSurface() {}
    
    // altering
    virtual void clear();
    virtual void rebuild();
    void deleteColumn(size_t col) 
        {_points.deleteColumn(col);}
    void deleteRow(size_t row)
        {_points.deleteRow(row);}
    void insertColKnot(float u);
    void insertRowKnot(float v);
    void normalizeKnotVectors();
    
    // getters/setters
    size_t rows() const
        { return _points.rows(); }
    size_t cols() const
        { return _points.cols(); }
    const QVector3D& getPoint(size_t row, size_t col) const
        {return _points.get(row, col);}
    void setColDegree(size_t val);
    void setRowDegree(size_t val);
    void setPoint(size_t row, size_t col, const QVector3D& val)
        {_points.set(row, col, val);}
    virtual void setBuild(bool val);
    void setDefaultColKnotVector();
    void setDefaultRowKnotVector();
    void setUniformColKnotVector();
    void setUniformRowKnotVector();
    
    // output
    void dump(std::ostream& os) const;

protected:

    bool _build;
    int _col_degree;
    int _row_degree;

    Grid<QVector3D> _points;
    std::vector<float> _col_knots;
    std::vector<float> _row_knots;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::NURBSurface& surface);

#endif

