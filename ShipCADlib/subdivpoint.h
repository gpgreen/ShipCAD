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

#ifndef SUBDIVPOINT_H_
#define SUBDIVPOINT_H_

#include <vector>
#include <iosfwd>
#include <QObject>
#include <QVector3D>
#include <QColor>
#include "shipcadlib.h"
#include "subdivbase.h"

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionFace;
class SubdivisionEdge;
class FileBuffer;
class Viewport;

extern bool g_point_verbose;

	/*!
	 * \brief 3D Point
	 *
	 * Used on boundaries of Faces. 
	 */
class SubdivisionPoint : public SubdivisionBase
{
    Q_OBJECT
    Q_PROPERTY(QVector3D Coordinate READ getCoordinate WRITE setCoordinate)
    Q_PROPERTY(float Curvature READ getCurvature)
    Q_PROPERTY(bool IsBoundaryVertex READ isBoundaryVertex)
    Q_PROPERTY(QVector3D Normal READ getNormal)
    Q_PROPERTY(size_t VertexIndex READ getIndex)
    Q_PROPERTY(vertex_type_t VertexType READ getVertexType WRITE setVertexType)

public:

	/*! \brief Constructor
	 *
	 * Use the static construct method to create points
	 */
    explicit SubdivisionPoint(SubdivisionSurface* owner);
    virtual ~SubdivisionPoint();
    
    // altering

	/*! \brief Reset contents of the Point to default values
	 */
    virtual void clear();
	/*! \brief add an Edge
	 *
	 * \param edge the edge to add to this point
	 */
    void addEdge(SubdivisionEdge* edge);
	/*! \brief add a Face
	 *
	 * \param face the face to add to this point
	 */
    void addFace(SubdivisionFace* face);
	/*! \brief delete an Edge
	 *
	 * \param edge the edge to delete from this point
	 */
    void deleteEdge(SubdivisionEdge* edge);
	/*! \brief delete a Face
	 *
	 * \param face the face to delete from this point
	 */
    void deleteFace(SubdivisionFace* face);
    void destroy();
    
    // geometry ops

	/*! \brief find the curvature of the surface at this point
	 *
	 * For each pair of points attached to faces attached to this point, take the dot
	 * product of the triangular face created from that pair and this point. Add these
	 * dot products together and convert from radians to degrees
	 *
	 * \return the curvature of the surface at this point
	 */
    float getCurvature();
	/*! \brief find the 3D coordinates of a point
	 *
	 * Get coordinates for this point from faces and edges attached to this point
	 * If this is a crease point, then take all opposite end points of edges
	 * If this is not a crease point, but all other kinds, then take all points on faces
	 * attached to this point, weight them and calculate an average
	 *
	 * \return coordinates of the calculated point
	 */
    QVector3D averaging();
	/*! \brief Create a vertex point
	 *
	 * During the subdivision process, new points are created at the
	 * same position as existing points.  This method creates a new
	 * point at the same position as the old one, also replaces the
	 * original point in any curves that contain it with the newly
	 * constructed point
	 *
	 * \return new point that is a copy of this one
	 */
    SubdivisionPoint* calculateVertexPoint();

    // getters/setters

	/*! \brief gets a vertex_type_t from integer
	 *
	 * Returns the vertex_type_t from an integer, used in serialization
	 *
	 * \param val an integer representing vertex type
	 * \return vertex type
	 */
    vertex_type_t fromInt(int val);
    QVector3D getCoordinate();
    virtual void setCoordinate(const QVector3D& val);
	/*! \brief Calculate normal vector
	 *
	 * Find the surface normal at this point by finding the normal of each face
	 * attached to this point, and adding them together.
	 *
	 * \return coordinates of the normal vector
	 */
    QVector3D getNormal();
    SubdivisionFace* getFace(size_t index);
    SubdivisionEdge* getEdge(size_t index);
    bool isBoundaryVertex();
	/*! \brief index of this point in parent surface 
	 *
	 * \return the index of this point in parent surface
	 */
    virtual size_t getIndex();
    size_t numberOfEdges() { return _edges.size(); }
    size_t numberOfFaces() { return _faces.size(); }
    size_t numberOfCurves();
    bool isRegularPoint();
    QVector3D getLimitPoint();
    bool isRegularNURBSPoint(std::vector<SubdivisionFace*>& faces);
    size_t indexOfFace(SubdivisionFace* face);
    bool hasEdge(SubdivisionEdge* edge);
    bool hasFace(SubdivisionFace* face);
    vertex_type_t getVertexType() { return _vtype; }
    void setVertexType(vertex_type_t nt) { _vtype = nt; }

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    /*! \brief make a point
	 *
	 * This doesn't add the point to the parent surface
	 *
	 * \param owner surface this point belongs to
	 */
    static SubdivisionPoint* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;
    // used in getCurvature
    float Angle_VV_3D(const QVector3D& p1, const QVector3D& p2,
		      const QVector3D& p3);
 
protected:

    std::vector<SubdivisionFace*> _faces;	/**< list of faces attached to this point */
    std::vector<SubdivisionEdge*> _edges;	/**< list of edges attached to this point */
    QVector3D _coordinate;					/**< 3D coordinates of this point */
    vertex_type_t _vtype;					/**< vertex type of this point */
};

//////////////////////////////////////////////////////////////////////////////////////

	/*! \brief Control point
	 *
	 * Subdivision Surfaces consist of control points which define the surface contour
	 *
	 */
class SubdivisionControlPoint : public SubdivisionPoint
{
    Q_OBJECT
    Q_PROPERTY(QColor Color READ getColor)
    Q_PROPERTY(bool Locked READ isLocked WRITE setLocked)
    Q_PROPERTY(bool Selected READ isSelected WRITE setSelected)
    Q_PROPERTY(bool Visible READ isVisible)
    Q_PROPERTY(bool IsLeak READ isLeak)
public:

	/*! \brief Constructor
	 *
	 * New points should be created with the static construct method
	 *
	 * \param owner the surface this control point belongs to
	 */
    explicit SubdivisionControlPoint(SubdivisionSurface* owner);
    virtual ~SubdivisionControlPoint();

    // modifiers
    void collapse();

    // getters/setters
    QColor getColor();
    bool isSelected();
    bool isLeak();
    bool isVisible();
    void setSelected(bool val);
    bool isLocked() { return _locked; }
    void setLocked(bool val);
	/*! \brief index of this point in parent surface 
	 *
	 * \return the index of this point in parent surface
	 */
    virtual size_t getIndex();
    virtual void setCoordinate(const QVector3D& val);

    // persistence
    void load_binary(FileBuffer& source);
    void save_binary(FileBuffer& destination);
    void loadFromStream(size_t& lineno, QStringList& strings);
    void saveToStream(QStringList& strings);

    // drawing
    static void drawControlPoints(Viewport& vp, SubdivisionSurface* surface);

    // output
    virtual void dump(std::ostream& os, const char* prefix = "") const;

    // makers
    static SubdivisionControlPoint* construct(SubdivisionSurface* owner);

protected:

    void priv_dump(std::ostream& os, const char* prefix) const;

protected:

    bool _locked;				/**< whether point is locked */
};
    
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionPoint& point);
std::ostream& operator << (std::ostream& os, const ShipCAD::SubdivisionControlPoint& point);

//////////////////////////////////////////////////////////////////////////////////////

#endif

