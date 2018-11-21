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
#include <set>
#include <QObject>
#include <QColor>
#include <QVector3D>
#include "shipcadlib.h"
#include "plane.h"
#include "spline.h"
#include "entity.h"
#include "mempool.h"
#include "orderedmap.h"
#include "tempvar.h"
#include "grid.h"

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
class Preferences;
struct PickRay;
    
extern bool g_surface_verbose;

//////////////////////////////////////////////////////////////////////////////////////

class DeleteElementsCollection
{
public:
    std::set<SubdivisionControlPoint*> points;
    std::set<SubdivisionControlEdge*> edges;
    std::set<SubdivisionControlFace*> faces;
    std::set<SubdivisionControlCurve*> curves;
    std::set<Spline*> splines;
    
    explicit DeleteElementsCollection();
    
    void clear();
    bool isSuppressed() const
        { return _suppress; }
    void suppressDelete(bool on)
        { _suppress = on;}

private:
    bool _suppress;
};

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Subdivision Surface
 *
 * This is the subdivision surface used for modelling the hull.
 * This is actually a quad-triangle subdivision surface as published in the articles:
 *
 *   "Quad/triangle subdivision" by J. Stam & C. Loop 
 *       http://research.microsoft.com/~cloop/qtEG.pdf
 *   "On C2 triangle/quad subdivision" by Scott Schaeffer & Joe Warren
 */
class SubdivisionSurface : public Entity
{
    Q_OBJECT

public:

    explicit SubdivisionSurface();
    virtual ~SubdivisionSurface();

    virtual void clear();
    void initialize(size_t point_start, size_t edge_start);
    virtual void rebuild();
    virtual void setBuild(bool val);

    // selecting
    SubdivisionBase* shootPickRay(Viewport& vp, const PickRay& ray);
    
    // modifiers

    /*! \brief check a surface for consistent normal direction
     *
     * \param checked structure to hold check results
     * \return true if any faces were inverted during check
     */
    bool check(ShipCAD::SurfaceCheckResult& checked, bool quiet);
    
    // tries to assemble quads into as few as possible rectangular patches
    void assembleFacesToPatches(assemble_mode_t mode,
                                std::vector<Grid<SubdivisionControlFace*> >& assembledPatches);
    /*! \brief group faces
     *
     * \return true if any faces were moved to new layers
     */
    bool autoGroupFaces();
    
    /*! \brief mirror selected faces around a plane
     *
     * \param connect_points use existing points if true, otherwise create new
     * \param pln the plane to mirror faces around
     * \param faces the list of faces to mirror
     */
    void mirrorFaces(bool connect_points,
                     const Plane& pln,
                     std::vector<SubdivisionControlFace*>& faces);
    
    void calculateGaussCurvature();
    void clearSelection();
    void convertToGrid(Grid<SubdivisionControlFace*>& input, Grid<SubdivisionPoint*>& grid);
    /*! \brief connect edges between selected points
     *
     * \return true if successful, false if edge already exists
     */
    bool edgeConnect();
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
    void importGrid(Grid<QVector3D>& points, SubdivisionLayer* layer);
    bool intersectPlane(const Plane& plane, bool hydrostatics_layers_only, SplineVector& destination);
    void insertPlane(const Plane& plane, bool add_curves);
    void subdivide();
    void deleteSelected();

    size_t numberOfLockedPoints() const;
    size_t numberOfSelectedLockedPoints() const;

    // SubdivisionPoint
    size_t numberOfPoints() const {return _points.size();}
    size_t indexOfPoint(const SubdivisionPoint* pt) const;
    SubdivisionPoint* getPoint(size_t index) const;
    void deletePoint(SubdivisionPoint* pt);

    // SubdivisionControlPoint
    size_t numberOfControlPoints() const {return _control_points.size();}
    size_t indexOfControlPoint(const SubdivisionControlPoint* pt) const;
    bool hasControlPoint(const SubdivisionControlPoint* pt) const;
    void removeControlPoint(SubdivisionControlPoint* pt);
    SubdivisionControlPoint* getControlPoint(size_t index) const;
    SubdivisionControlPoint* addControlPoint(const QVector3D& pt);
    void addControlPoint(SubdivisionControlPoint* pt);
    // adds a new controlpoint at 0,0,0 without checking other points
    SubdivisionControlPoint* addControlPoint();
    // delete a controlpoint singly, not by dumping the pool
    void deleteControlPoint(SubdivisionControlPoint* point);
    // collapse a point (remove it)
    void collapseControlPoint(SubdivisionControlPoint* point);
	
    // selected SubdivisionControlPoint
    size_t numberOfSelectedControlPoints() const {return _sel_control_points.size();}
    bool hasSelectedControlPoint(const SubdivisionControlPoint* pt) const;
    void setSelectedControlPoint(SubdivisionControlPoint* pt);
    void removeSelectedControlPoint(SubdivisionControlPoint* pt);
    OrderedPointMap& getSelControlPointCollection()
        {return _sel_control_points;}
    
    // SubdivisionEdge
    size_t numberOfEdges() const {return _edges.size();}
    SubdivisionEdge* getEdge(size_t index) const;
    SubdivisionEdge* edgeExists(SubdivisionPoint* p1, SubdivisionPoint* p2);
    void deleteEdge(SubdivisionEdge* edge);
    void isolateEdges(std::vector<SubdivisionEdge*>& input,
                      std::vector<std::vector<SubdivisionPoint*> >& sorted);

    // SubdivisionControlEdge
    size_t numberOfControlEdges() const {return _control_edges.size();}
    SubdivisionControlEdge* getControlEdge(size_t index) const;
    bool hasControlEdge(const SubdivisionControlEdge* edge) const;
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
    size_t numberOfSelectedControlEdges() const {return _sel_control_edges.size();}
    void setSelectedControlEdge(SubdivisionControlEdge* edge);
    void removeSelectedControlEdge(SubdivisionControlEdge* edge);
    bool hasSelectedControlEdge(const SubdivisionControlEdge* edge) const;
    std::set<SubdivisionControlEdge*>& getSelControlEdgeCollection() {return _sel_control_edges;}

    // SubdivisionFace
    size_t numberOfFaces();
    void clearFaces();

    // SubdivisionControlFace
    size_t numberOfControlFaces() const {return _control_faces.size();}
    SubdivisionControlFace* getControlFace(size_t index) const;
    SubdivisionControlFace* getControlFace(SubdivisionPoint* p1,
                                           SubdivisionPoint* p2,
                                           SubdivisionPoint* p3,
                                           SubdivisionPoint* p4) const;
    bool hasControlFace(const SubdivisionControlFace* face) const;
    void addControlFace(SubdivisionControlFace* face);
    SubdivisionControlFace* addControlFace(std::vector<QVector3D>& points);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
                                           bool check_edges);
    SubdivisionControlFace* addControlFace(std::vector<SubdivisionControlPoint*>& points,
                                           bool check_edges, SubdivisionLayer* layer);
    void removeControlFace(SubdivisionControlFace* face);
    void deleteControlFace(SubdivisionControlFace* face);

    // selected SubdivisionControlFace
    size_t numberOfSelectedControlFaces() const {return _sel_control_faces.size();}
    void setSelectedControlFace(SubdivisionControlFace* face);
    void removeSelectedControlFace(SubdivisionControlFace* face);
    bool hasSelectedControlFace(const SubdivisionControlFace* face) const;
    std::set<SubdivisionControlFace*>& getSelControlFaceCollection() {return _sel_control_faces;}

    // SubdivisionControlCurve
    size_t numberOfControlCurves() const {return _control_curves.size();}
    SubdivisionControlCurve* getControlCurve(size_t index) const;
    bool hasControlCurve(const SubdivisionControlCurve* curve) const;
    void addControlCurve(SubdivisionControlCurve* curve);
    void addControlCurves(std::vector<SubdivisionControlEdge*>& edges);
    void removeControlCurve(SubdivisionControlCurve* curve);
    void deleteControlCurve(SubdivisionControlCurve* curve);
    
    // selected SubdivisionControlCurve
    size_t numberOfSelectedControlCurves() const {return _sel_control_curves.size();}
    void setSelectedControlCurve(SubdivisionControlCurve* curve);
    void removeSelectedControlCurve(SubdivisionControlCurve* curve);
    bool hasSelectedControlCurve(const SubdivisionControlCurve* curve) const;
    std::set<SubdivisionControlCurve*>& getSelControlCurveCollection() {return _sel_control_curves;}

    // SubdivisionLayer
    size_t numberOfLayers() const {return _layers.size();}
    size_t indexOfLayer(const SubdivisionLayer* layer) const;
    SubdivisionLayer* getLayer(size_t index);
    const SubdivisionLayer* getLayer(size_t index) const;
    SubdivisionLayer* getActiveLayer() const {return _active_layer;}
    void setActiveLayer(SubdivisionLayer* layer);
    bool hasLayer(const SubdivisionLayer* layer) const;
    void deleteLayer(SubdivisionLayer* layer);
    size_t lastUsedLayerID() const {return _last_used_layerID;}
    void setLastUsedLayerID(size_t newid) {_last_used_layerID = newid;}
    size_t requestNewLayerID();
    SubdivisionLayer* addNewLayer();
    QString getDefaultLayerName() const;
    size_t deleteEmptyLayers();
    std::vector<SubdivisionLayer*>& getLayers() {return _layers;}
    const std::vector<SubdivisionLayer*>& getLayers() const {return _layers;}
    
    // getters/setters
    subdiv_mode_t getSubdivisionMode() const {return _subdivision_mode;}
    void setSubdivisionMode(subdiv_mode_t val);
    int getDesiredSubdivisionLevel() const {return _desired_subdiv_level;}
    
    void setDesiredSubdivisionLevel(int val);

    bool isGaussCurvatureCalculated() const;
    float getCurvatureScale() const {return _curvature_scale;}
    void setCurvatureScale(float val) {_curvature_scale=val;}
    float getMinGausCurvature() const {return _min_gaus_curvature;}
    float getMaxGausCurvature() const {return _max_gaus_curvature;}
    float getGaussCurvature(size_t idx) const;
    
    const Plane& getWaterlinePlane() const {return _waterline_plane;}
    void setWaterlinePlane(const Plane& val) {_waterline_plane = val;}
    float getMainframeLocation() const {return _main_frame_location;}
    void setMainframeLocation(float val) {_main_frame_location=val;}

    int getControlPointSize() const {return _control_point_size;}
    void setControlPointSize(int sz) 
        {_control_point_size=sz;}

    // options
    bool showCurvature() const {return _show_curvature;}
    bool shadeUnderWater() const {return _shade_under_water;}
    bool showControlNet() const {return _show_control_net;}
    bool showControlCurves() const {return _show_control_curves;}
    bool showInteriorEdges() const {return _show_interior_edges;}
    bool drawMirror() const {return _draw_mirror;}
    bool showNormals() const {return _show_normals;}

    void setShowCurvature(bool val) {_show_curvature = val;}
    void setShadeUnderWater(bool val) {_shade_under_water = val;}
    void setShowControlNet(bool val) {_show_control_net = val;}
    void setShowControlCurves(bool val) {_show_control_curves = val;}
    void setShowInteriorEdges(bool val) {_show_interior_edges = val;}
    void setDrawMirror(bool val) {_draw_mirror = val;}
    void setShowNormals(bool val) {_show_normals = val;}

    // colors
    QColor getSelectedColor() const {return _selected_color;}
    QColor getCreaseEdgeColor() const {return _crease_color;}
    QColor getEdgeColor() const {return _edge_color;}
    QColor getLeakColor() const {return _leak_color;}
    QColor getRegularPointColor() const {return _regular_point_color;}
    QColor getCornerPointColor() const {return _corner_point_color;}
    QColor getDartPointColor() const {return _dart_point_color;}
    QColor getCreaseColor() const {return _crease_color;}
    QColor getCreasePointColor() const {return _crease_point_color;}
    QColor getControlCurveColor() const {return _control_curve_color;}
    QColor getLayerColor() const {return _layer_color;}
    QColor getNormalColor() const {return _normal_color;}
    QColor getCurvatureColor() const {return _curvature_color;}
    QColor getUnderWaterColor() const {return _underwater_color;}
    QColor getZebraColor() const {return _zebra_color;}

    void setUnderWaterColor(const QColor& c) 
        {_underwater_color = c;}

    // persistence
    void saveBinary(FileBuffer& destination);
    void loadBinary(FileBuffer& source);
    void loadFromStream(size_t& lineno, QStringList& strings);
    void loadVRMLFile(const QString& filename);
    void exportFeFFile(QStringList& strings) const;
    void importFeFFile(QStringList& strings, size_t& lineno);
    void exportObjFile(bool export_control_net, QStringList& strings);
    void saveToStream(QStringList& strings) const;

    // drawing
    virtual void draw(Viewport &vp);
    virtual void draw(Viewport& vp, LineShader* lineshader);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    Pool<SubdivisionControlPoint>& getControlPointPool() {return _cpoint_pool;}
    Pool<SubdivisionControlEdge>& getControlEdgePool() {return _cedge_pool;}
    Pool<SubdivisionControlFace>& getControlFacePool() {return _cface_pool;}
    Pool<SubdivisionControlCurve>& getControlCurvePool() {return _ccurve_pool;}
    Pool<SubdivisionLayer>& getLayerPool() {return _layer_pool;}
    Pool<SubdivisionPoint>& getPointPool() {return _point_pool;}
    Pool<SubdivisionEdge>& getEdgePool() {return _edge_pool;}
    Pool<SubdivisionFace>& getFacePool() {return _face_pool;}
    Pool<Spline>& getSplinePool() {return _spline_pool;}

    /*! \brief delete elements marked for permanent removal
     */
    void deleteElementsCollection();

    /*! \brief temporarily change draw mirror
     */
    TempVarChange<bool> tempChangeMirror(bool val)
        { return TempVarChange<bool>(val, &_draw_mirror); }
    
protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    SubdivisionControlPoint* newControlPoint(const QVector3D& p);
    // used in convertToGrid
    void doAssemble(Grid<SubdivisionPoint*>& grid, size_t& cols, size_t& rows,
                    std::vector<SubdivisionFace*>& faces);
    // used in assembleFaces
    void doAssembleSpecial(Grid<SubdivisionPoint*>& grid, size_t& cols, size_t& rows,
                           assemble_mode_t mode,
                           std::vector<SubdivisionControlFace*>& checkfaces,
                           std::vector<SubdivisionControlFace*>& faces);
    // used in assembleFacesToPatches
    void assembleFaces(assemble_mode_t mode,
                       std::vector<SubdivisionControlFace*>& ctrlfaces,
                       std::vector<Grid<SubdivisionControlFace*> >& assembled);
    void sortEdges(std::vector<SubdivisionEdge*>& edges);
    void sortEdges(std::vector<SubdivisionPoint*>& points, std::vector<SubdivisionEdge*>& edges);
    void sortControlEdges(std::vector<SubdivisionControlPoint*>& points, std::vector<SubdivisionControlEdge*>& edges);

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
    OrderedPointMap _sel_control_points;
    std::set<SubdivisionControlEdge*> _sel_control_edges;
    std::set<SubdivisionControlFace*> _sel_control_faces;
    std::set<SubdivisionControlCurve*> _sel_control_curves;
    
    // memory pools
    Pool<SubdivisionControlPoint> _cpoint_pool;
    Pool<SubdivisionControlEdge> _cedge_pool;
    Pool<SubdivisionControlFace> _cface_pool;
    Pool<SubdivisionControlCurve> _ccurve_pool;
    Pool<SubdivisionLayer> _layer_pool;
    Pool<SubdivisionPoint> _point_pool;
    Pool<SubdivisionEdge> _edge_pool;
    Pool<SubdivisionFace> _face_pool;
    Pool<Spline> _spline_pool;

    DeleteElementsCollection _deleted;
    
    friend class Preferences;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionSurface& surface);

#endif

