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

#ifndef SUBDIVSURFACE_H_
#define SUBDIVSURFACE_H_

#include <iosfwd>
#include <vector>
#include <QObject>
#include <QColor>

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionControlPoint;
class SubdivisionEdge;
class SubdivisionFace;
class SubdivisionControlFace;
class SubdivisionControlEdge;
class SubdivisionLayer;

// This is the subdivision surface used for modelling the hull.
// This is actually a quad-triangle subdivision surface as publisehed in the articles:
//
//   "Quad/triangle subdivision" by J. Stam & C. Loop http://research.microsoft.com/~cloop/qtEG.pdf
//   "On C2 triangle/quad subdivision" by Scott Schaeffer & Joe Warren
class SubdivisionSurface : public QObject
{
    Q_OBJECT

public:

    enum subdiv_mode_t {fmQuadTriangle, fmCatmullClark};

    explicit SubdivisionSurface();
    virtual ~SubdivisionSurface();

    // SubdivisionPoint
    size_t indexOfPoint(SubdivisionPoint* pt);\

    // SubdivisionControlPoint
    size_t indexOfControlPoint(SubdivisionControlPoint* pt);
    void addControlPoint(SubdivisionControlPoint* pt);
    SubdivisionControlPoint* getControlPoints(size_t index);

    // selected SubdivisionControlPoint
    bool hasSelectedControlPoint(SubdivisionControlPoint* pt);
    void setSelectedControlPoint(SubdivisionControlPoint* pt);
    void removeSelectedControlPoint(SubdivisionControlPoint* pt);

    // SubdivisionEdge
    size_t indexOfEdge(SubdivisionEdge* edge);
    void deleteEdge(SubdivisionEdge* edge);
    SubdivisionEdge* edgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);

    // SubdivisionControlEdge
    size_t indexOfControlEdge(SubdivisionControlEdge* edge);
    bool hasControlEdge(SubdivisionControlEdge* edge);
    void addControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* controlEdgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);
    void deleteControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* addControlEdge(SubdivisionPoint* sp, SubdivisionPoint* ep);
    void isolateEdges(std::vector<SubdivisionControlEdge*>& input, 
		      std::vector<std::vector<SubdivisionControlPoint*> >& sorted);

    // selected SubdivisionControlEdge
    void setSelectedControlEdge(SubdivisionControlEdge* edge);
    void removeSelectedControlEdge(SubdivisionControlEdge* edge);
    bool hasSelectedControlEdge(SubdivisionControlEdge* edge);

    // SubdivisionFace
    size_t indexOfFace(SubdivisionFace* face);
    void deleteFace(SubdivisionFace* face);

    // SubdivisionControlFace
    size_t indexOfControlFace(SubdivisionControlFace* face);
    bool hasControlFace(SubdivisionControlFace* face);
    void addControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
					   bool check_edges);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
					   bool check_edges, SubdivisionLayer* layer);
    void deleteControlFace(SubdivisionControlFace* face);

    // selected SubdivisionControlFace
    void setSelectedControlFace(SubdivisionControlFace* face);
    void removeSelectedControlFace(SubdivisionControlFace* face);
    bool hasSelectedControlFace(SubdivisionControlFace* face);

    // SubdivisionControlCurve
    size_t numberControlCurves();
    bool hasControlCurve(SubdivisionControlCurve* curve);
    void addControlCurve(SubdivisionControlCurve* curve);
    size_t indexOfControlCurve(SubdivisionControlCurve* curve);
    void deleteControlCurve(SubdivisionControlCurve* curve);

    // selected SubdivisionControlCurve
    void setSelectedControlCurve(SubdivisionControlCurve* curve);
    void removeSelectedControlCurve(SubdivisionControlCurve* curve);
    bool hasSelectedControlCurve(SubdivisionControlCurve* curve);

    // SubdivisionLayer
    size_t numberOfLayers();
    SubdivisionLayer* getLayer(size_t index);
    size_t indexOfLayer(SubdivisionLayer* layer);

    // getters/setters
    bool isBuild() { return _build; }
    void setBuild(bool val);

    // options
    bool showControlNet();
    subdiv_mode_t getSubdivisionMode();

    QColor getSelectedColor();
    QColor getCreaseEdgeColor();
    QColor getEdgeColor();
    QColor getLeakColor();
    QColor getRegularPointColor();
    QColor getCornerPointColor();
    QColor getDartPointColor();
    QColor getCreasePointColor();

    // output
    void dump(std::ostream& os) const;

protected:

    bool _build;

    std::vector<SubdivisionControlEdge*> _sel_control_edges;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionSurface& surface);

#endif

