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

#ifndef SUBDIVFACE_H_
#define SUBDIVFACE_H_

#include <iosfwd>
#include <vector>
#include <QObject>
#include <QVector3D>
#include <QColor>

#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionLayer;
class SubdivisionEdge;
class SubdivisionControlCurve;
class SubdivisionControlPoint;
class SubdivisionControlEdge;
class Viewport;
class FileBuffer;

class SubdivisionFace : public SubdivisionBase
{
    Q_OBJECT
    Q_PROPERTY(float Area READ getArea)
    Q_PROPERTY(QVector3D FaceCenter READ getFaceCenter)
    Q_PROPERTY(QVector3D FaceNormal READ getFaceNormal)

public:

    explicit SubdivisionFace(SubdivisionSurface* owner);
    virtual ~SubdivisionFace();
    
    // modifiers
    void flipNormal();
    void addPoint(SubdivisionPoint* point);
    void insertPoint(size_t index, SubdivisionPoint* point);
    void clear();
    virtual void subdivide(SubdivisionSurface* owner, bool controlface,
	std::vector<std::pair<SubdivisionPoint*,SubdivisionPoint*> > &vertexpoints,
	std::vector<std::pair<SubdivisionEdge*,SubdivisionPoint*> > &edgepoints,
	std::vector<std::pair<SubdivisionFace*,SubdivisionPoint*> > &facepoints,
	std::vector<SubdivisionEdge*>& interioredges,
	std::vector<SubdivisionEdge*>& controledges,
	std::vector<SubdivisionFace*>& dest);

    // getters/setters
    size_t numberOfPoints() { return _points.size(); }
    QVector3D faceCenter();
    bool hasPoint(SubdivisionPoint* pt);
    SubdivisionPoint* getPoint(size_t index);
    SubdivisionPoint* calculateFacePoint();
    size_t indexOfPoint(SubdivisionPoint* pt);
    float getArea();
    QVector3D getFaceCenter();
    QVector3D getFaceNormal();


    // drawing
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

protected:

    // used in subdivide
    void edgeCheck(SubdivisionSurface *owner,
                   SubdivisionPoint* p1,
                   SubdivisionPoint* p2,
                   bool crease,
                   bool controledge,
                   SubdivisionControlCurve* curve,
                   SubdivisionFace* newface,
                   std::vector<SubdivisionEdge*> &interioredges,
                   std::vector<SubdivisionEdge*> &controledges);

protected:

    std::vector<SubdivisionPoint*> _points;
};

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionControlFace : public SubdivisionFace
{
    Q_OBJECT
    Q_PROPERTY(QColor Color READ getColor)
    Q_PROPERTY(size_t FaceIndex READ getIndex)
    Q_PROPERTY(SubdivisionLayer* Layer READ getLayer WRITE setLayer)
    Q_PROPERTY(QVector3D Max READ getMax)
    Q_PROPERTY(QVector3D Min READ getMin)
    Q_PROPERTY(bool Selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool Visible READ isVisible)

public:

    explicit SubdivisionControlFace(SubdivisionSurface* owner);
    virtual ~SubdivisionControlFace();

    // modifiers
    void calcExtents();
    virtual void clear();
    void clearChildren();
    SubdivisionControlEdge* insertEdge(SubdivisionControlPoint* p1,
				       SubdivisionControlPoint* p2);
    void removeReferences();
    virtual void subdivide(SubdivisionSurface* owner,
        std::vector<std::pair<SubdivisionPoint*,SubdivisionPoint*> > &vertexpoints,
        std::vector<std::pair<SubdivisionEdge*,SubdivisionPoint*> > &edgepoints,
        std::vector<std::pair<SubdivisionFace*,SubdivisionPoint*> > &facepoints,
        std::vector<SubdivisionEdge*>& interioredges,
        std::vector<SubdivisionEdge*>& controledges,
        std::vector<SubdivisionFace*>& dest);
    void trace();

    // getters/setters
    SubdivisionLayer* getLayer();
    void setLayer(SubdivisionLayer* layer);
    SubdivisionFace* getChild(size_t index);
    size_t numberOfChildren() { return _children.size(); }
    QColor getColor();
    SubdivisionEdge* getControlEdge(size_t index);
    size_t getNumberOfControlEdge() { return _control_edges.size(); }
    SubdivisionEdge* getEdge(size_t index);
    size_t getNumberOfEdge() { return _edges.size(); }
    size_t getIndex();
    bool isSelected();
    bool isVisible();
    void setSelected(bool val);
    QVector3D getMin() { return _min; }
    QVector3D getMax() { return _max; }

    // persistence
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& destination);
    void saveToDXF(std::vector<QString>& strings);

    // drawing
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

protected:

    // used in trace
    void findAttachedFaces(std::vector<SubdivisionControlFace*>& todo_list,
                           SubdivisionControlFace* face);

protected:

    SubdivisionLayer* _layer;
    QVector3D _min;
    QVector3D _max;
    std::vector<SubdivisionFace*> _children;
    std::vector<SubdivisionEdge*> _edges;
    std::vector<SubdivisionEdge*> _control_edges;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionFace& face);
std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionControlFace& face);

#endif

