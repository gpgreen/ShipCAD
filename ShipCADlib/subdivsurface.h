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
#include <QVector>

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionControlPoint;
class SubdivisionEdge;
class SubdivisionFace;
class SubdivisionControlFace;
class SubdivisionControlEdge;
class SubdivisionControlCurve;
class SubdivisionLayer;
class Viewport;
class FileBuffer;

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
    enum assemble_mode_t {amRegular, amNURBS};

    explicit SubdivisionSurface();
    virtual ~SubdivisionSurface();

    virtual void clear();
    void initialize();
    void rebuild();

    // modifiers

    // tries to assemble quads into as few as possible rectangular patches
    void assembleFacesToPatches(std::vector<SubdivisionLayer*>& layers,
				assemble_mode_t mode,
				std::vector<SubdivisionFace*>& assembledPatches,
				size_t& nAssembled);
    void calculateGaussCurvature();
    void clearSelection();
    void convertToGrid();
    void edgeConnect();
    void extents(QVector3D& min, QVector3D& max);
    void extrudeEdges();
    void calculateIntersections();
    void extractAllEdgeLoops();
    void extractPointsFromFaces();
    viod extractPointsFromSelection();
    void importGrid();
    bool intersectPlane();
    void insertPlane();
    void subdivide();

    size_t numberOfLockedPoints();
    size_t numberOfSelectedLockedPoints();

    // SubdivisionPoint
    size_t numberOfPoints();
    size_t indexOfPoint(SubdivisionPoint* pt);
    SubdivisionPoint* getPoint(size_t index);

    // SubdivisionControlPoint
    size_t numberOfControlPoints();
    size_t indexOfControlPoint(SubdivisionControlPoint* pt);
    bool hasControlPoint(SubdivisionControlPoint* pt);
    SubdivisionControlPoint* getControlPoint(size_t index);
    SubdivisionControlPoint* addControlPoint(const QVector3D& pt);
    void addControlPoint(SubdivisionControlPoint* pt);
    // adds a new controlpoint at 0,0,0 without checking other points
    SubdivisionControlPoint* addControlPoint();

    // selected SubdivisionControlPoint
    size_t numberOfSelectedControlPoints();
    bool hasSelectedControlPoint(SubdivisionControlPoint* pt);
    void setSelectedControlPoint(SubdivisionControlPoint* pt);
    void removeSelectedControlPoint(SubdivisionControlPoint* pt);

    // SubdivisionEdge
    size_t numberOfEdges();
    size_t indexOfEdge(SubdivisionEdge* edge);
    SubdivisionEdge* getEdge(size_t index);
    void deleteEdge(SubdivisionEdge* edge);
    SubdivisionEdge* edgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);

    // SubdivisionControlEdge
    size_t numberOfControlEdges();
    size_t indexOfControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* getControlEdge(size_t index);
    bool hasControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* addControlEdge(SubdivisionPoint* sp, SubdivisionPoint* ep);
    void addControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* controlEdgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);
    void deleteControlEdge(SubdivisionControlEdge* edge);
    void isolateEdges(std::vector<SubdivisionControlEdge*>& input, 
		      std::vector<std::vector<SubdivisionControlPoint*> >& sorted);

    // selected SubdivisionControlEdge
    size_t numberOfSelectedControlEdges();
    void setSelectedControlEdge(SubdivisionControlEdge* edge);
    void removeSelectedControlEdge(SubdivisionControlEdge* edge);
    bool hasSelectedControlEdge(SubdivisionControlEdge* edge);

    // SubdivisionFace
    size_t numberOfFaces();
    size_t indexOfFace(SubdivisionFace* face);
    SubdivisionFace* getFace(size_t index);
    void deleteFace(SubdivisionFace* face);
    void clearFaces();

    // SubdivisionControlFace
    size_t numberOfControlFaces();
    size_t indexOfControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* getControlFace(size_t index);
    bool hasControlFace(SubdivisionControlFace* face);
    void addControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
					   bool check_edges);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
					   bool check_edges, SubdivisionLayer* layer);
    void deleteControlFace(SubdivisionControlFace* face);

    // selected SubdivisionControlFace
    size_t numberOfSelectedControlFaces();
    void setSelectedControlFace(SubdivisionControlFace* face);
    void removeSelectedControlFace(SubdivisionControlFace* face);
    bool hasSelectedControlFace(SubdivisionControlFace* face);

    // SubdivisionControlCurve
    size_t numberOfControlCurves();
    size_t indexOfControlCurve(SubdivisionControlCurve* curve);
    SubdivisionControlCurve* getControlCurve(size_t index);
    bool hasControlCurve(SubdivisionControlCurve* curve);
    void addControlCurve(SubdivisionControlCurve* curve);
    void deleteControlCurve(SubdivisionControlCurve* curve);

    // selected SubdivisionControlCurve
    size_t numberOfSelectedControlCurves();
    void setSelectedControlCurve(SubdivisionControlCurve* curve);
    void removeSelectedControlCurve(SubdivisionControlCurve* curve);
    bool hasSelectedControlCurve(SubdivisionControlCurve* curve);

    // SubdivisionLayer
    size_t numberOfLayers();
    size_t indexOfLayer(SubdivisionLayer* layer);
    SubdivisionLayer* getLayer(size_t index);
    SubdivisionLayer* getActiveLayer();
    void setActiveLayer(SubdivisionLayer* layer);
    bool hasLayer(SubdivisionLayer* layer);
    void deleteLayer(SubdivisionLayer* layer);
    size_t lastUsedLayerID();
    void setLastUsedLayerID(size_t newid);
    size_t requestNewLayerID();
    SubdivisionLayer* addNewLayer();

    // getters/setters
    bool isBuild() { return _build; }
    void setBuild(bool val);
    void setDesiredSubdivisionLevel(int val);
    void setShowControlNet(bool val);
    void setSubdivisionMode(subdiv_mode_t val);

    // options
    bool showControlNet();
    bool showControlCurves();
    bool isDrawMirror();
    subdiv_mode_t getSubdivisionMode();

    // colors
    QColor getSelectedColor();
    QColor getCreaseEdgeColor();
    QColor getEdgeColor();
    QColor getLeakColor();
    QColor getRegularPointColor();
    QColor getCornerPointColor();
    QColor getDartPointColor();
    QColor getCreasePointColor();
    QColor getControlCurveColor();
    QColor getLayerColor();

    // persistence
    void saveBinary(FileBuffer& destination);
    void loadBinary(FileBuffer& source);
    void loadVRMLFile(const QString& filename);
    void exportFeFFile(std::vector<QString>& strings);
    void importFeFFile(std::vector<QString>& strings, int lineno);
    void exportObjFile(bool export_control_net, std::vector<QString>& strings);

    // drawing
    virtual void draw(Viewport& vp);

    // output
    void dump(std::ostream& os) const;

signals:

    void changedLayerData();

protected:

    SubdivisionControlPoint* newControlPoint(const QVector3D& p);

protected:

    bool _build;
    bool _show_control_net;
    bool _initialized;
    bool _show_interior_edges;
    bool _draw_mirror;
    bool _shade_under_water;
    bool _show_normals;
    bool _show_curvature;
    bool _show_control_curves;

    subdiv_mode_t _subdivision_mode;
    int _desired_subdiv_level;
    int _current_subdiv_level;

    float _curvature_scale;
    // float gaus_curvature;
    float _min_gaus_curvature;
    float _max_gaus_curvature;
    float _main_frame_location;

    QColor _crease_color;
    QColor _crease_edge_color;
    QColor _underwater_color;
    QColor _edge_color;
    QColor _selected_color;
    QColor _crease_point_color;
    QColor _regular_point_color;
    QColor _corner_point_color;
    QColor _dart_point_color;
    QColor _layer_color;
    QColor _normal_color;
    QColor _leak_color;
    QColor _curvature_color;
    QColor _control_curve_color;
    QColor _zebra_color;

    Plane3D _waterline_plane;
    
    size_t _last_used_layerID;

    // control points which can be changed by the user
    std::vector<SubdivisionControlPoint*> _control_points;
    // control edges which can be changed by the user
    std::vector<SubdivisionControlEdge*> _control_edges;
    // control faces which can be changed by the user
    std::vector<SubdivisionControlFace*> _control_faces;
    // mastercurves
    std::vector<SubdivisionControlCurve*> _control_curves;

    // entities obtained by subdividing the surface
    std::vector<SubdivisionPoint*> _points;
    std::vector<SubdivisionEdge*> _edges;
    std::vector<SubdivisionLayer*> _layers;

    // selected entities
    std::vector<SubdivisionControlPoint*> _sel_control_points;
    std::vector<SubdivisionControlEdge*> _sel_control_edges;
    std::vector<SubdivisionControlFace*> _sel_control_faces;
    std::vector<SubdivisionControlCurve*> _sel_control_curves;
    
    // currently active layer, may not be 0
    SubdivisionLayer* _active_layer;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionSurface& surface);

#endif

