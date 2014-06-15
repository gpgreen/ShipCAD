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

#ifndef SUBDIVCONTROLCURVE_H_
#define SUBDIVCONTROLCURVE_H_

#include <iosfwd>
#include <vector>
#include <QObject>
#include <QColor>
#include <QString>

#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionControlPoint;
class SubdivisionControlEdge;
class Spline;
class Viewport;
class FileBuffer;

// Controlcurves are curves that can be added to the controlnet an are subdivide with the surface.
// The resulting curve therefore lies on the surface, and can be used in the fairing process
class SubdivisionControlCurve : public SubdivisionBase
{
    Q_OBJECT
    //Q_PROPERTY(SubdivisionSurface* Owner READ getOwner)

public:

    explicit SubdivisionControlCurve(SubdivisionSurface* owner);
    virtual ~SubdivisionControlCurve();

    // modifications
    void replaceVertexPoint(SubdivisionPoint* oldpt, SubdivisionPoint* newpt);
    void insertEdgePoint(SubdivisionPoint* p1, SubdivisionPoint* p2, SubdivisionPoint* newpt);
    void deleteEdge(SubdivisionControlEdge* edge);
    void insertControlPoint(SubdivisionControlPoint* p1, SubdivisionControlPoint* p2, 
			    SubdivisionControlPoint* newpt);
    void addPoint(SubdivisionControlPoint* p);
    virtual void clear();
    void resetDivPoints();

    // getters/setters
    bool isSelected();
    bool isVisible();
    bool isBuild() { return _build; }
    QColor getColor();
    size_t numberOfControlPoints() { return _points.size(); }
    size_t numberOfSubdivPoints() { return _div_points.size(); }
    SubdivisionControlPoint* getControlPoint(size_t index);
    SubdivisionPoint* getSubdivPoint(size_t index);
    void setVisible(bool val);
    void setBuild(bool val) { _build = val; }
    void setSelected(bool val);
    Spline* getSpline() {return _curve;}

    // persistence
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& destiniation);
    void saveToDXF(std::vector<QString>& strings);

    // draw
    virtual void draw(Viewport& vp);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;

protected:

    bool _build;
    std::vector<SubdivisionControlPoint*> _points;
    std::vector<SubdivisionPoint*> _div_points;
    Spline* _curve;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionControlCurve& curve);

#endif

