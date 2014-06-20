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

#ifndef SUBDIVEDGE_H_
#define SUBDIVEDGE_H_

#include <iosfwd>
#include <vector>
#include <QObject>
#include <QColor>

#include "subdivbase.h"

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionControlPoint;
class SubdivisionControlCurve;
class SubdivisionFace;
class Viewport;
class LineShader;
class FileBuffer;

extern bool g_edge_verbose;

class SubdivisionEdge : public SubdivisionBase
{
    Q_OBJECT
    Q_PROPERTY(SubdivisionPoint* StartPoint READ startPoint)
    Q_PROPERTY(SubdivisionPoint* EndPoint READ endPoint)
    Q_PROPERTY(bool Crease READ isCrease WRITE setCrease)
    Q_PROPERTY(size_t Index READ getIndex)
    Q_PROPERTY(SubdivisionControlCurve* Curve READ getCurve WRITE setCurve)
    Q_PROPERTY(bool ControlEdge READ isControlEdge WRITE setControlEdge)
public:

    explicit SubdivisionEdge(SubdivisionSurface* owner);
    virtual ~SubdivisionEdge();

    virtual void clear();
    virtual void draw(bool draw_mirror, Viewport& vp, LineShader* lineshader,
                      const QColor& edgeColor);

    // modifiers
    void addFace(SubdivisionFace* face);
    void assign(SubdivisionEdge* edge);
    SubdivisionPoint* calculateEdgePoint();
    void deleteFace(SubdivisionFace* face);
    void swapData();

    // getters/setters
    SubdivisionPoint* startPoint() { return _points[0]; }
    SubdivisionPoint* endPoint() { return _points[1]; }
    virtual bool isBoundaryEdge();
    bool isControlEdge() { return _control_edge; }
    void setControlEdge(bool val) { _control_edge = val; }
    size_t numberOfFaces() { return _faces.size(); }
    bool isCrease() { return _crease; }
    void setCrease(bool val);
    SubdivisionControlCurve* getCurve() { return _curve; }
    void setCurve(SubdivisionControlCurve* curve) { _curve = curve; }
    size_t getIndex();
    SubdivisionFace* getFace(size_t index);
    bool hasFace(SubdivisionFace* face);
    SubdivisionEdge* getPreviousEdge();
    SubdivisionEdge* getNextEdge();
    void setPoints(SubdivisionPoint* p1, SubdivisionPoint* p2)
        { _points[0] = p1; _points[1] = p2; }

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    // makers
    static SubdivisionEdge* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;

protected:

    SubdivisionPoint* _points[2];
    std::vector<SubdivisionFace*> _faces;
    bool _crease;
    bool _control_edge;
    SubdivisionControlCurve* _curve;
};

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionControlEdge : public SubdivisionEdge
{
    Q_OBJECT
    Q_PROPERTY(QColor Color READ getColor)
    Q_PROPERTY(bool Visible READ isVisible)
    Q_PROPERTY(bool Selected READ isSelected WRITE setSelected)

public:

    explicit SubdivisionControlEdge(SubdivisionSurface* owner);
    virtual ~SubdivisionControlEdge();

    // modifiers
    void collapse();
    SubdivisionControlPoint* insertControlPoint(const QVector3D& p);
    void load_binary(FileBuffer& source);
    void save_binary(FileBuffer& destination);
    void trace();

    // getters/setters
    size_t getIndex();
    QColor getColor();
    virtual bool isBoundaryEdge();
    bool isSelected();
    void setSelected(bool val);
    bool isVisible();

    // drawing
    virtual void draw(bool draw_mirror, Viewport &vp, LineShader* lineshader);

    // persistence
    void loadFromStream(size_t& lineno, std::vector<QString>& strings);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    // makers
    static SubdivisionControlEdge* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    void priv_trace(SubdivisionControlPoint* p);

protected:

    bool _selected;
    bool _visible;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionEdge& edge);
std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionControlEdge& edge);

//////////////////////////////////////////////////////////////////////////////////////

#endif

