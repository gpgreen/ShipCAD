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
#include <QColor>

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
    Q_PROPERTY(QVector3D Coordinate READ getCoordinate WRITE setCoordinate)
    Q_PROPERTY(float Curvature READ getCurvature)
    Q_PROPERTY(bool IsBoundaryVertex READ isBoundaryVertex)
    Q_PROPERTY(QVector3D Normal READ getNormal)
    Q_PROPERTY(size_t VertexIndex READ getIndex)
    Q_PROPERTY(vertex_type_t VertexType READ getVertexType WRITE setVertexType)

public:

    enum vertex_type_t {svRegular, svCrease, svDart, svCorner};

    explicit SubdivisionPoint(SubdivisionSurface* owner);
    virtual ~SubdivisionPoint();
    
    // altering
    void clear();
    void addEdge(SubdivisionEdge* edge);
    void addFace(SubdivisionFace* face);
    void deleteEdge(SubdivisionEdge* edge);
    void deleteFace(SubdivisionFace* face);
    void destroy();
    
    // geometry ops
    float getCurvature();
    QVector3D averaging();
    SubdivisionPoint* calculateVertexPoint();

    // drawing
    //int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp);

    // getters/setters
    QVector3D getCoordinate();
    virtual void setCoordinate(const QVector3D& val);
    QVector3D getNormal();
    SubdivisionFace* getFace(size_t index);
    SubdivisionEdge* getEdge(size_t index);
    bool isBoundaryVertex();
    virtual size_t getIndex();
    size_t numberOfEdges() { return _edges.size(); }
    size_t numberOfFaces() { return _faces.size(); }
    size_t numberOfCurves();
    bool getRegularPoint();
    QVector3D getLimitPoint();
    bool isRegularNURBSPoint(std::vector<SubdivisionFace*>& faces);
    size_t indexOfFace(SubdivisionFace* face);
    bool hasFace(SubdivisionFace* face);
    vertex_type_t getVertexType() { return _vtype; }
    void setVertexType(vertex_type_t nt) { _vtype = nt; }

    // output
    void dump(std::ostream& os) const;

 protected:

    // used in getCurvature
    float Angle_VV_3D(const QVector3D& p1, const QVector3D& p2,
		      const QVector3D& p3);
 
protected:

    std::vector<SubdivisionFace*> _faces;
    std::vector<SubdivisionEdge*> _edges;
    QVector3D _coordinate;
    vertex_type_t _vtype;
};

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionControlPoint : public SubdivisionPoint
{
    Q_OBJECT
    Q_PROPERTY(QColor Color READ getColor)
    Q_PROPERTY(bool Locked READ isLocked WRITE setLocked)
    Q_PROPERTY(bool Selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool Visible READ isVisible)
    Q_PROPERTY(bool IsLeak READ isLeak)
public:

    explicit SubdivisionControlPoint(SubdivisionSurface* owner);
    virtual ~SubdivisionControlPoint();

    // modifiers
    void collapse();
    
    // getters/setters
    QColor getColor();
    bool isSelected();
    bool isLeak();
    bool isVisible();
    void setSelected(bool val);
    bool isLocked() { return _locked; }
    void setLocked(bool val);
    virtual size_t getIndex();
    virtual void setCoordinate(const QVector3D& val);

    // persistence
    void load_binary(FileBuffer& source);
    void save_binary(FileBuffer& destination);

    // drawing
    //int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

 protected:

    bool _locked;
};
    
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionPoint& point);
std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionControlPoint& point);

//////////////////////////////////////////////////////////////////////////////////////

#endif

