/*##############################################################################################
 *    ShipCAD
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>
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

#ifndef PREDICATE_H
#define PREDICATE_H

#include <vector>

namespace ShipCAD {

    class SubdivisionPoint;
    class SubdivisionEdge;
    class SubdivisionFace;
    class SubdivisionControlPoint;
    
//////////////////////////////////////////////////////////////////////////////////////

// predicate class to find an element with given point
struct FirstPointPairPredicate {
    ShipCAD::SubdivisionPoint* _querypt;
    bool operator()(const std::pair<ShipCAD::SubdivisionPoint*,
                    ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _querypt;
    }
    FirstPointPairPredicate (ShipCAD::SubdivisionPoint* querypt)
        : _querypt(querypt) {}
};

// predicate class to find an element with given face
struct FirstFacePointPredicate {
    ShipCAD::SubdivisionFace* _queryface;
    bool operator()(const std::pair<ShipCAD::SubdivisionFace*,
                    ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _queryface;
    }
    FirstFacePointPredicate (ShipCAD::SubdivisionFace* queryface)
        : _queryface(queryface) {}
};

// predicate class to find an element with given edge
struct FirstEdgePointPredicate {
    ShipCAD::SubdivisionEdge* _queryedge;
    bool operator()(const std::pair<ShipCAD::SubdivisionEdge*,
                    ShipCAD::SubdivisionPoint*>& val)
    {
        return val.first == _queryedge;
    }
    FirstEdgePointPredicate (ShipCAD::SubdivisionEdge* queryedge)
        : _queryedge(queryedge) {}
};

// predicate class to find an element with given point
struct FirstCPointPairPredicate {
    ShipCAD::SubdivisionControlPoint* _querypt;
    bool operator()(const std::pair<ShipCAD::SubdivisionControlPoint*,
                    ShipCAD::SubdivisionControlPoint*>& val)
    {
        return val.first == _querypt;
    }
    explicit FirstCPointPairPredicate (ShipCAD::SubdivisionControlPoint* querypt)
        : _querypt(querypt) {}
    explicit FirstCPointPairPredicate (ShipCAD::SubdivisionPoint* querypt)
        : _querypt(dynamic_cast<SubdivisionControlPoint*>(querypt)) {}
};

// predicate class to return true if pt1 is z ordered less than pt2    
struct CompareControlPointZOrder 
{
    bool operator()(ShipCAD::SubdivisionControlPoint* pt1,
                    ShipCAD::SubdivisionControlPoint* pt2) 
        {
            QVector3D p1 = pt1->getCoordinate();
            QVector3D p2 = pt2->getCoordinate();
            if (p1.z() < p2.z())
                return true;
            if (abs(p1.z() - p2.z()) < 1E-6) {
                if (p1.x() < p2.x())
                    return true;
                if (abs(p1.x() - p2.x()) < 1E-6) {
                    if (p1.y() < p2.y())
                        return true;
                }
            }
            return false;
        }
};
    
                
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif // PREDICATE_H
