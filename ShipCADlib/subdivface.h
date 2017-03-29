/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#ifndef SUBDIVFACE_H_
#define SUBDIVFACE_H_

#include <iosfwd>
#include <vector>
#include <QObject>
#include <QVector3D>
#include <QColor>

#include "subdivbase.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionPoint;
class SubdivisionLayer;
class SubdivisionEdge;
class SubdivisionControlCurve;
class SubdivisionControlPoint;
class SubdivisionControlEdge;
class Viewport;
class LineShader;
class FileBuffer;
class FaceShader;

class SubdivisionFace : public SubdivisionBase
{
public:

	/*! \brief Constructor
	 *
	 * \param owner parent surface
	 */
    explicit SubdivisionFace(SubdivisionSurface* owner);
    virtual ~SubdivisionFace() {}
    
    // modifiers
	/*! \brief swap normal vector to other face
	 */
    void flipNormal();
	/*! \brief add a point to the face
	 *
	 * \param point point to add
	 */
    void addPoint(SubdivisionPoint* point);
	/*! \brief insert a new point into the face
	 *
	 * \param index insert the new point at this index
	 * \param point point to add to face
	 */
    void insertPoint(size_t index, SubdivisionPoint* point);
	/*! \brief reset point attributes to default values
	 */
    virtual void clear();
	/*! \brief subdivide the face
	 *
	 * \param vertexpoints list of Point <-> Point pairs. The first
	 * point is a vertex point on this face, the second point is a
	 * copy of this point to use in the new subdivided faces
	 *
	 * \param edgepoints list of Edge <-> Point pairs. The edge is an
	 * edge on this face. The point is the midpoint of that edge to
	 * use in the new subdivided faces
	 *
	 * \param facepoints list of Face <-> Point
	 * pairs.  
	 * 
	 * \param interioredges after subdivision, the new interior edges
	 * of the newly created faces will be added to this list
	 * 
	 * \param controledges after subdivision, the new edges descended
	 * from control edges will be added to this list
	 *
	 * \param dest the new subdivided faces will be added to this list
	 */
    virtual void subdivide(
		bool controlface,
		std::vector<std::pair<SubdivisionPoint*,SubdivisionPoint*> > &vertexpoints,
		std::vector<std::pair<SubdivisionEdge*,SubdivisionPoint*> > &edgepoints,
		std::vector<std::pair<SubdivisionFace*,SubdivisionPoint*> > &facepoints,
		std::vector<SubdivisionEdge*>& interioredges,
		std::vector<SubdivisionEdge*>& controledges,
		std::vector<SubdivisionFace*>& dest);

    // getters/setters
	/*! \brief number of points for this face
	 *
	 * \return number of points for this face
	 */
    size_t numberOfPoints() { return _points.size(); }
	/*! \brief does the face have this point
	 *
	 * \param pt the point to check
	 * \return true if the point is part of the face
	 */
    bool hasPoint(SubdivisionPoint* pt);
	/*! \brief get face point
	 *
	 * \param index index of point to get
	 * \return the point at index
	 */
    SubdivisionPoint* getPoint(size_t index);
	/*! \brief Get point on center of face for subdivision
	 *
	 * When subdividing a face, each edge is split, and a point is put in
	 * the center of the face, then all are connected and new faces created.
	 * If the face is a triangle and we are not using fvCatmullClark, then a
	 * center point is not created. The face is then divided into triangles,
	 * instead of quadrilateral faces. In that case this returns a null point
	 *
	 * \return the point at center of face, or 0 if triangle and not using fvCatmullClark,
	 * or the number of face points is less than 3
	 */
    SubdivisionPoint* calculateFacePoint();
	/*! \brief get index of point for this face
	 *
	 * \param pt point to find in face
	 * \return index of that point in face, same as numberOfPoints if point not in face
	 */
    size_t indexOfPoint(SubdivisionPoint* pt);
	/*! \brief calculate the area of this face
	 */
    float getArea();
	/*! \brief get coordinates of the center of the face
	 *
	 * \return coordinates of the face center
	 */
    QVector3D getFaceCenter();
	/*! \brief get coordinates of the face normal
	 *
	 * \return coordinates of the face normal
	 */
    QVector3D getFaceNormal();

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    // makers
    static SubdivisionFace* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    // used in subdivide
	/*! \brief check for edge between points
	 *
	 * This method will create edges between 2 points if it doesn't
	 * exist.
	 *
	 * \param p1 start point of edge
	 * \param p2 end point of edge
	 * \param crease whether this edge is a crease
	 * \param controledge whether this edge is a control edge
	 * \param curve if edge is a controledge, then pass the curve to be attached
	 * \param interioredges if this edge is an interior edge, add it to this list
	 * \param controledges if this edge is a control, add it to this list
	 */
    void edgeCheck(SubdivisionPoint* p1,
                   SubdivisionPoint* p2,
                   bool crease,
                   bool controledge,
                   SubdivisionControlCurve* curve,
                   std::vector<SubdivisionEdge*> &interioredges,
                   std::vector<SubdivisionEdge*> &controledges);

protected:

    std::vector<SubdivisionPoint*> _points; /**< points belonging to this face */
};

typedef std::vector<SubdivisionFace*>::iterator subdivface_iter;
typedef std::vector<SubdivisionFace*>::const_iterator const_subdivface_iter;
    
//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionControlFace : public SubdivisionFace
{
public:

	/*! \brief Constructor
	 *
	 * \param owner parent surface
	 */
    explicit SubdivisionControlFace(SubdivisionSurface* owner);
    virtual ~SubdivisionControlFace() {}

    // modifiers
    /*! \brief remove this face in preparation for deleting it
     */
    void removeFace();
	/*! \brief get min/max coordinate amongst all children
	 *
	 * For each child face, update the min and max value for
	 * this face.
	 */
    void calcExtents();
	/*! \brief reset attributes to default values
	 */
    virtual void clear();
	/*! \brief remove all child faces and edges
	 *
	 * Remove all child faces and edges and also remove
	 * them from the parent surface face and edge pools
	 */
    void clearChildren();
	/*! \brief remove all control edges of this face
	 */
    void clearControlEdges() {_control_edges.clear();}
	/*! \brief add a control edge
	 *
	 * TODO: this shouldn't exist here, but belongs to
	 * surface
	 *
	 * Using 2 control points, add a control edge to this face. The
	 * edge must already exist. If
	 * the edge already exists between these points, returns this
	 * edge.
	 *
	 * \param p1 start point of edge
	 * \param p2 end point of edge
     * \param deleteme true if this face should be deleted, it was replaced in method
	 * \return new control edge or 0 if no edge exists between
	 * these 2 points
	 */
    SubdivisionControlEdge* insertControlEdge(SubdivisionControlPoint* p1,
                                              SubdivisionControlPoint* p2,
                                              bool& deleteme);
	/*! \brief Removes this face and all edges from points
	 *
	 * For each point in the face, remove this face from the point.
	 * For each edge in the face, remove the face from that edge.
	 * When done, the points and edges no longer reference this face
	 */
    void removeReferences();
	/*! \brief subdivide the face
	 *
	 * \param vertexpoints list of Point <-> Point pairs. The first
	 * point is a vertex point on this face, the second point is a
	 * copy of this point to use in the new subdivided faces
	 *
	 * \param edgepoints list of Edge <-> Point pairs. The edge is an
	 * edge on this face. The point is the midpoint of that edge to
	 * use in the new subdivided faces
	 *
	 * \param facepoints list of Face <-> Point
	 * pairs.  
	 * 
	 * \param controledges after subdivision, the new edges descended
	 * from control edges will be added to this list
	 */
    virtual void subdivide(
		std::vector<std::pair<SubdivisionPoint*,SubdivisionPoint*> > &vertexpoints,
		std::vector<std::pair<SubdivisionEdge*,SubdivisionPoint*> > &edgepoints,
		std::vector<std::pair<SubdivisionFace*,SubdivisionPoint*> > &facepoints,
		std::vector<SubdivisionEdge*>& controledges);
	/*! \brief select all control faces connected to this one
	 *
	 * Select all control faces connected to this one on the same
	 * layer, that are not separated by a crease edge
	 */
    void trace();

    // getters/setters
    SubdivisionLayer* getLayer() {return _layer;}
    void setLayer(SubdivisionLayer* layer);
    SubdivisionFace* getChild(size_t index);
    size_t numberOfChildren() { return _children.size(); }
    QColor getColor();
    SubdivisionEdge* getControlEdge(size_t index);
    size_t numberOfControlEdges() { return _control_edges.size(); }
    SubdivisionEdge* getEdge(size_t index);
    size_t numberOfEdges() { return _edges.size(); }
    size_t getIndex();
    bool isSelected();
    bool isVisible();
    void setSelected(bool val);
    QVector3D getMin() { return _min; }
    QVector3D getMax() { return _max; }
    // iterators to beginning/ending of _children list
    std::vector<SubdivisionFace*>::iterator childrenBegin() {return _children.begin();}
    std::vector<SubdivisionFace*>::iterator childrenEnd() {return _children.end();}

    // persistence
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& destination);
    void saveToDXF(QStringList& strings);
    void saveToStream(QStringList& strings);
    void loadFromStream(size_t& lineno, QStringList& strings);

    // drawing
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void drawFaces(Viewport& vp, FaceShader* monoshader);
    virtual void drawCurvatureFaces(Viewport& vp, float MinCurvature, float MaxCurvature);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    // makers
    static SubdivisionControlFace* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    // used in trace
    void findAttachedFaces(std::vector<SubdivisionControlFace*>& todo_list,
                           SubdivisionControlFace* face);
    void addFaceToDL(QVector<QVector3D>& vertices, QVector<QVector3D>& normals,
                     QVector3D& p1, QVector3D& p2, QVector3D& p3);

protected:

    SubdivisionLayer* _layer;	/**< which layer this face belongs to */
    QVector3D _min;				/**< minimum coordinate of this face */
    QVector3D _max;				/**< maximum coordinate of this face */
    std::vector<SubdivisionFace*> _children;    /**< subdivided faces */
    std::vector<SubdivisionEdge*> _edges;       /**< subdivided internal edges */
    std::vector<SubdivisionEdge*> _control_edges;   /**< control edges (may be of SubdivisionEdge type if this face has been subdivided */
};

typedef std::vector<SubdivisionControlFace*>::iterator subdivctlface_iter;
typedef std::vector<SubdivisionControlFace*>::const_iterator const_subdivctlface_iter;
    
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionFace& face);
std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionControlFace& face);

#endif

