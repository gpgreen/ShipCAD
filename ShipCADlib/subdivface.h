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

#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionLayer;
class SubdivisionEdge;
class SubdivisionControlCurve;
class Viewport;

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
    void subdivide(SubdivisionSurface* owner, bool controlface,
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

public:

    explicit SubdivisionControlFace(SubdivisionSurface* owner);
    virtual ~SubdivisionControlFace();

    // getters/setters
    SubdivisionLayer* getLayer();
    void setLayer(SubdivisionLayer* layer);

    // drawing
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

protected:
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionFace& face);
std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionControlFace& face);

#endif

