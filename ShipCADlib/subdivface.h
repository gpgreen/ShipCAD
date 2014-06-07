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
#include <QObject>
#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionLayer;
class Viewport;

class SubdivisionFace : public SubdivisionBase
{
    Q_OBJECT
    //Q_PROPERTY(SubdivisionSurface* Owner READ getOwner)

public:

    explicit SubdivisionFace(SubdivisionSurface* owner);
    virtual ~SubdivisionFace();
    
    // modifiers
    void flipNormal();
    void addPoint(SubdivisionPoint* point);
    void insertPoint(size_t index, SubdivisionPoint* point);

    // getters/setters
    size_t numberOfPoints();
    QVector3D faceCenter();
    bool hasPoint(SubdivisionPoint* pt);
    SubdivisionPoint* getPoint(int index);
    size_t indexOfPoint(SubdivisionPoint* pt);

    // drawing
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

protected:
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

