/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                                *
 *    e-mail                  : Info@FREEship.org                                              *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                      *
 *    FREE!ship homepage      : www.FREEship.org                                               *
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

#ifndef SUBDIVSURFACE_H_
#define SUBDIVSURFACE_H_

#include <iosfwd>
#include <vector>
#include <map>
#include <QObject>
#include <QColor>
#include <QVector3D>
#include <boost/pool/pool.hpp>
#include "shipcadlib.h"
#include "plane.h"
#include "spline.h"
#include "entity.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionBase;
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
class PickRay;
    
extern bool g_surface_verbose;

// This is the subdivision surface used for modelling the hull.
// This is actually a quad-triangle subdivision surface as publisehed in the articles:
//
//   "Quad/triangle subdivision" by J. Stam & C. Loop 
//       http://research.microsoft.com/~cloop/qtEG.pdf
//   "On C2 triangle/quad subdivision" by Scott Schaeffer & Joe Warren
class SubdivisionSurface : public Entity
{
    Q_OBJECT

public:

    typedef std::vector<std::vector<SubdivisionControlFace*> > face_grid_t;
    typedef std::vector<std::vector<SubdivisionPoint*> > grid_t;
    typedef std::vector<std::vector<SubdivisionControlPoint*> > control_grid_t;
    typedef std::vector<std::vector<QVector3D> > coordinate_grid_t;

    explicit SubdivisionSurface();
    virtual ~SubdivisionSurface();

    virtual void clear();
    void initialize(size_t point_start, size_t edge_start);
    virtual void rebuild();
    virtual void setBuild(bool val);

    // selecting
    std::multimap<float, SubdivisionBase*> shootPickRay(Viewport& vp, const PickRay& ray);
    
    // modifiers

    // tries to assemble quads into as few as possible rectangular patches
    void assembleFacesToPatches(std::vector<SubdivisionLayer*>& layers,
                                assemble_mode_t mode,
                                std::vector<SubdivisionFace*>& assembledPatches,
                                size_t& nAssembled);
    void calculateGaussCurvature();
    void clearSelection();
    void convertToGrid(face_grid_t& input, grid_t& grid);
    void edgeConnect();
    virtual void extents(QVector3D& min, QVector3D& max);
    void extrudeEdges(std::vector<SubdivisionControlEdge*>& edges,
		      const QVector3D& direction);
    void calculateIntersections(const Plane& plane,
                                std::vector<SubdivisionControlFace*>& faces,
                                SplineVector& destination);
    void extractAllEdgeLoops(std::vector<std::vector<SubdivisionPoint*> >& destination);
    void extractPointsFromFaces(std::vector<SubdivisionFace*>& selectedfaces,
                                std::vector<SubdivisionControlPoint*>& points,
                                size_t& lockedpoints);
    void extractPointsFromSelection(std::vector<SubdivisionControlPoint*>& selectedpoints,
                                    size_t& lockedpoints);
    void importGrid(coordinate_grid_t& points, SubdivisionLayer* layer);
    bool intersectPlane(const Plane& plane, bool hydrostatics_layers_only, SplineVector& destination);
    void insertPlane(const Plane& plane, bool add_curves);
    void subdivide();
    void selectionDelete();

    size_t numberOfLockedPoints();
    size_t numberOfSelectedLockedPoints();

    // SubdivisionPoint
    size_t numberOfPoints() {return _points.size();}
    size_t indexOfPoint(SubdivisionPoint* pt);
    SubdivisionPoint* getPoint(size_t index);
    void deletePoint(SubdivisionPoint* pt);

    // SubdivisionControlPoint
    size_t numberOfControlPoints() {return _control_points.size();}
    size_t indexOfControlPoint(SubdivisionControlPoint* pt);
    bool hasControlPoint(SubdivisionControlPoint* pt);
    void removeControlPoint(SubdivisionControlPoint* pt);
    SubdivisionControlPoint* getControlPoint(size_t index);
    SubdivisionControlPoint* addControlPoint(const QVector3D& pt);
    void addControlPoint(SubdivisionControlPoint* pt);
    // adds a new controlpoint at 0,0,0 without checking other points
    SubdivisionControlPoint* addControlPoint();
    // delete a controlpoint singly, not by dumping the pool
    void deleteControlPoint(SubdivisionControlPoint* point);
	
    // selected SubdivisionControlPoint
    size_t numberOfSelectedControlPoints() {return _sel_control_points.size();}
    bool hasSelectedControlPoint(SubdivisionControlPoint* pt);
    void setSelectedControlPoint(SubdivisionControlPoint* pt);
    void removeSelectedControlPoint(SubdivisionControlPoint* pt);

    // SubdivisionEdge
    size_t numberOfEdges() {return _edges.size();}
    size_t indexOfEdge(SubdivisionEdge* edge);
    SubdivisionEdge* getEdge(size_t index);
    SubdivisionEdge* edgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);
    void deleteEdge(SubdivisionEdge* edge);

    // SubdivisionControlEdge
    size_t numberOfControlEdges() {return _control_edges.size();}
    size_t indexOfControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* getControlEdge(size_t index);
    bool hasControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* addControlEdge(SubdivisionPoint* sp, SubdivisionPoint* ep);
    void addControlEdge(SubdivisionControlEdge* edge);
    SubdivisionControlEdge* controlEdgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);
    void removeControlEdge(SubdivisionControlEdge* edge);
    void deleteControlEdge(SubdivisionControlEdge* edge);
    void isolateEdges(std::vector<SubdivisionControlEdge*>& input,
                      std::vector<std::vector<SubdivisionControlPoint*> >& sorted);
	/*! \brief collapse an edge on the surface
	 *
	 * \param edge the edge to collapse
	 */
	void collapseEdge(SubdivisionControlEdge* edge);
	
    // selected SubdivisionControlEdge
    size_t numberOfSelectedControlEdges() {return _sel_control_edges.size();}
    void setSelectedControlEdge(SubdivisionControlEdge* edge);
    void removeSelectedControlEdge(SubdivisionControlEdge* edge);
    bool hasSelectedControlEdge(SubdivisionControlEdge* edge);

    // SubdivisionFace
    size_t numberOfFaces();
    void clearFaces();

    // SubdivisionControlFace
    size_t numberOfControlFaces() {return _control_faces.size();}
    size_t indexOfControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* getControlFace(size_t index);
    SubdivisionControlFace* getControlFace(SubdivisionPoint* p1,
                                           SubdivisionPoint* p2,
                                           SubdivisionPoint* p3,
                                           SubdivisionPoint* p4);
    bool hasControlFace(SubdivisionControlFace* face);
    void addControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* addControlFace(std::vector<QVector3D>& points);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
                                           bool check_edges);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
                                           bool check_edges, SubdivisionLayer* layer);
    void removeControlFace(SubdivisionControlFace* face);
    void deleteControlFace(SubdivisionControlFace* face);

    // selected SubdivisionControlFace
    size_t numberOfSelectedControlFaces() {return _sel_control_faces.size();}
    void setSelectedControlFace(SubdivisionControlFace* face);
    void removeSelectedControlFace(SubdivisionControlFace* face);
    bool hasSelectedControlFace(SubdivisionControlFace* face);

    // SubdivisionControlCurve
    size_t numberOfControlCurves() {return _control_curves.size();}
    size_t indexOfControlCurve(SubdivisionControlCurve* curve);
    SubdivisionControlCurve* getControlCurve(size_t index);
    bool hasControlCurve(SubdivisionControlCurve* curve);
    void addControlCurve(SubdivisionControlCurve* curve);
    void removeControlCurve(SubdivisionControlCurve* curve);

    // selected SubdivisionControlCurve
    size_t numberOfSelectedControlCurves() {return _sel_control_curves.size();}
    void setSelectedControlCurve(SubdivisionControlCurve* curve);
    void removeSelectedControlCurve(SubdivisionControlCurve* curve);
    bool hasSelectedControlCurve(SubdivisionControlCurve* curve);

    // SubdivisionLayer
    size_t numberOfLayers() {return _layers.size();}
    size_t indexOfLayer(SubdivisionLayer* layer);
    SubdivisionLayer* getLayer(size_t index);
    SubdivisionLayer* getActiveLayer() {return _active_layer;}
    void setActiveLayer(SubdivisionLayer* layer);
    bool hasLayer(SubdivisionLayer* layer);
    void deleteLayer(SubdivisionLayer* layer);
    size_t lastUsedLayerID() {return _last_used_layerID;}
    void setLastUsedLayerID(size_t newid) {_last_used_layerID = newid;}
    size_t requestNewLayerID();
    SubdivisionLayer* addNewLayer();
    QString getDefaultLayerName();

    // getters/setters
    subdiv_mode_t getSubdivisionMode() {return _subdivision_mode;}
    void setSubdivisionMode(subdiv_mode_t val);
    void setDesiredSubdivisionLevel(int val);

    bool isGaussCurvatureCalculated();
    float getCurvatureScale() {return _curvature_scale;}
    void setCurvatureScale(float val) {_curvature_scale=val;}
    float getMinGausCurvature() {return _min_gaus_curvature;}
    float getMaxGausCurvature() {return _max_gaus_curvature;}

    const Plane& getWaterlinePlane() {return _waterline_plane;}
    void setWaterlinePlane(const Plane& val) {_waterline_plane = val;}
    float getMainframeLocation() {return _main_frame_location;}
    void setMainframeLocation(float val) {_main_frame_location=val;}

    int getControlPointSize() {return _control_point_size;}

    // options
    bool showCurvature() {return _show_curvature;}
    bool shadeUnderWater() {return _shade_under_water;}
    bool showControlNet() {return _show_control_net;}
    bool showControlCurves() {return _show_control_curves;}
    bool showInteriorEdges() {return _show_interior_edges;}
    bool drawMirror() {return _draw_mirror;}
    bool showNormals() {return _show_normals;}

    void setShowCurvature(bool val) {_show_curvature = val;}
    void setShadeUnderWater(bool val) {_shade_under_water = val;}
    void setShowControlNet(bool val) {_show_control_net = val;}
    void setShowControlCurves(bool val) {_show_control_curves = val;}
    void setShowInteriorEdges(bool val) {_show_interior_edges = val;}
    void setDrawMirror(bool val) {_draw_mirror = val;}
    void setShowNormals(bool val) {_show_normals = val;}

    // colors
    QColor getSelectedColor() {return _selected_color;}
    QColor getCreaseEdgeColor() {return _crease_color;}
    QColor getEdgeColor() {return _edge_color;}
    QColor getLeakColor() {return _leak_color;}
    QColor getRegularPointColor() {return _regular_point_color;}
    QColor getCornerPointColor() {return _corner_point_color;}
    QColor getDartPointColor() {return _dart_point_color;}
    QColor getCreaseColor() {return _crease_color;}
    QColor getCreasePointColor() {return _crease_point_color;}
    QColor getControlCurveColor() {return _control_curve_color;}
    QColor getLayerColor() {return _layer_color;}
    QColor getCurvatureColor() {return _curvature_color;}

    void setUnderWaterColor(const QColor& c) 
        {_underwater_color = c;}
    
    // persistence
    void saveBinary(FileBuffer& destination);
    void loadBinary(FileBuffer& source);
    void loadFromStream(size_t& lineno, QStringList& strings);
    void loadVRMLFile(const QString& filename);
    void exportFeFFile(QStringList& strings);
    void importFeFFile(QStringList& strings, size_t& lineno);
    void exportObjFile(bool export_control_net, QStringList& strings);
    void saveToStream(QStringList& strings);

    // drawing
    virtual void draw(Viewport &vp);
    virtual void draw(Viewport& vp, LineShader* lineshader);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    boost::pool<>& getControlPointPool() {return _cpoint_pool;}
    boost::pool<>& getControlEdgePool() {return _cedge_pool;}
    boost::pool<>& getControlFacePool() {return _cface_pool;}
    boost::pool<>& getControlCurvePool() {return _ccurve_pool;}
    boost::pool<>& getLayerPool() {return _layer_pool;}
    boost::pool<>& getPointPool() {return _point_pool;}
    boost::pool<>& getEdgePool() {return _edge_pool;}
    boost::pool<>& getFacePool() {return _face_pool;}

signals:

    void changedLayerData();
    void changeActiveLayer();

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    SubdivisionControlPoint* newControlPoint(const QVector3D& p);
    void findAttachedFaces(std::vector<SubdivisionControlFace*>& found_list,
                           std::vector<SubdivisionControlFace*>& todo_list,
                           SubdivisionControlFace* face);
    bool validFace(SubdivisionFace* face,
                   std::vector<SubdivisionFace*>& faces,
                   std::vector<SubdivisionFace*>& tmpfaces);
    void doAssemble(grid_t& grid, size_t& cols, size_t& rows,
                    std::vector<SubdivisionFace*>& faces);
    void sortEdges(std::vector<SubdivisionEdge*>& edges);
    std::vector<SubdivisionPoint*> sortEdges(bool always_true, std::vector<SubdivisionEdge*>& edges);
    std::vector<SubdivisionControlPoint*> sortEdges(std::vector<SubdivisionControlEdge*>& edges);

protected:

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
    int _control_point_size;

    float _curvature_scale;
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

    Plane _waterline_plane;
    
    size_t _last_used_layerID;
    // currently active layer, may not be 0
    SubdivisionLayer* _active_layer;

    // control points which can be changed by the user
    std::vector<SubdivisionControlPoint*> _control_points;
    // control edges which can be changed by the user
    std::vector<SubdivisionControlEdge*> _control_edges;
    // control faces which can be changed by the user
    std::vector<SubdivisionControlFace*> _control_faces;
    // mastercurves
    std::vector<SubdivisionControlCurve*> _control_curves;
    // our layers
    std::vector<SubdivisionLayer*> _layers;
    // curvature at points
    std::vector<float> _gaus_curvature;

    // entities obtained by subdividing the surface
    std::vector<SubdivisionPoint*> _points;     // all subdivided points, corners of the SubdivisionFace
    std::vector<SubdivisionEdge*> _edges;       // interior edges of subdivided faces descended from control
                                                // edges. They are not of SubdivisionControlEdge type, the
                                                // interior edges of the subdivided faces are not in this
                                                // list, but are only retrievable through the face itself
                                                // in their _edges list.

    // selected entities
    std::vector<SubdivisionControlPoint*> _sel_control_points;
    std::vector<SubdivisionControlEdge*> _sel_control_edges;
    std::vector<SubdivisionControlFace*> _sel_control_faces;
    std::vector<SubdivisionControlCurve*> _sel_control_curves;
    
    // memory pools
    boost::pool<> _cpoint_pool;
    boost::pool<> _cedge_pool;
    boost::pool<> _cface_pool;
    boost::pool<> _ccurve_pool;
    boost::pool<> _layer_pool;
    boost::pool<> _point_pool;
    boost::pool<> _edge_pool;
    boost::pool<> _face_pool;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionSurface& surface);

#endif

