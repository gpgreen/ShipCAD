/*#############################################################################################}
  {    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
  {    open source surface-modelling program based on subdivision surfaces and intended for     }
  {    designing ships.                                                                         }
  {                                                                                             }
  {    Copyright © 2005, by Martijn van Engeland                                                }
  {    e-mail                  : Info@FREEship.org                                              }
  {    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
  {    FREE!ship homepage      : www.FREEship.org                                               }
  {                                                                                             }
  {    This program is free software; you can redistribute it and/or modify it under            }
  {    the terms of the GNU General Public License as published by the                          }
  {    Free Software Foundation; either version 2 of the License, or (at your option)           }
  {    any later version.                                                                       }
  {                                                                                             }
  {    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
  {    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
  {    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
  {                                                                                             }
  {    You should have received a copy of the GNU General Public License along with             }
  {    this program; if not, write to the Free Software Foundation, Inc.,                       }
  {    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
  {                                                                                             }
  {#############################################################################################*/

#ifndef SUBDIVPOINT_H_
#define SUBDIVPOINT_H_

#include <vector>
#include <iosfwd>
#include <QObject>
#include <QVector3D>

#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionFace;
class SubdivisionEdge;
class FileBuffer;
class Viewport;

class SubdivisionPoint : public SubdivisionBase
{
    Q_OBJECT
    //Q_PROPERTY(int Fragments READ getFragments WRITE setFragments)
    //Q_PROPERTY(bool ShowCurvature MEMBER _show_curvature)
    //Q_PROPERTY(bool ShowPoints MEMBER _show_points)
    //Q_PROPERTY(float CurvatureScale MEMBER _curvature_scale)
    //Q_PROPERTY(QColor CurvatureColor MEMBER _curvature_color)

public:

    enum vertex_type_t {svRegular, svCrease, svDart, svCorner};

    explicit SubdivisionPoint(SubdivisionSurface* owner);
    virtual ~SubdivisionPoint();
    
    // altering
    void clear();
    void add_edge(SubdivisionEdge* edge);
    void add_face(SubdivisionFace* face);
    void delete_edge(SubdivisionEdge* edge);
    void delete_face(SubdivisionFace* face);
    void destroy();
    
    // geometry ops
    float curvature() const;
    QVector3D averaging() const;

    // persistence
    void load_binary(FileBuffer& source);
    void save_binary(FileBuffer& destination);

    // drawing
    //int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp);

    // getters/setters
    QVector3D getCoordinate() const;
    QVector3D getNormal() const;
    void setCoordinate(const QVector3D val);
    SubdivisionFace* getFace(int index) const;
    SubdivisionEdge* getEdge(int index) const;
    bool isBoundaryVertex() const;
    int getIndex() const;

    // output
    void dump(std::ostream& os) const;

protected:

    std::vector<SubdivisionFace*> _faces;
    std::vector<SubdivisionEdge*> _edges;
    QVector3D _coordinate;
    vertex_type_t _vtype;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionPoint& point);

#endif

