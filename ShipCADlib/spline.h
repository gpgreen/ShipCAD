/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#ifndef SPLINE_H_
#define SPLINE_H_

#include <vector>
#include <iosfwd>
#include <QObject>
#include <QVector3D>
#include <QColor>
#include <QString>
#include "entity.h"
#include "pointervec.h"

namespace ShipCAD {

class FileBuffer;
class Plane;
class IntersectionData;
class LineShader;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief spline entity used in intersections etc
 */
class Spline : public Entity
{
    Q_OBJECT

public:

    /*! \brief constructor
     */
    explicit Spline();
    /*! \brief copy constructor
     */
    explicit Spline(const Spline& copied);
    /*! \brief destructor
     */
    virtual ~Spline() {}
    
    // altering
    /*! \brief add a point to the spline
     *
     * \param p coordinates of point to add
     */
    void add(const QVector3D& p);
    /*! \brief delete a point from the spline
     *
     * \param index delete point at index
     */
    void delete_point(size_t index);
    /*! \brief insert a point at the index
     *
     * \param index where to add the new point
     * \param p coordinates of new point
     */
    void insert(size_t index, const QVector3D& p);
    /*! \brief insert a copy of a spline at the index
     *
     * \param index where to insert a copy of the spline
     * \param invert if true, invert the direction of the copied spline
     * \param duplicate_point if true, then the last point of the copied
	 * spline is not inserted into this spline
     * \param source the spline to copy into this spline at the index
     */
    void insert_spline(size_t index, bool invert, bool duplicate_point, 
		       const Spline& source);
    /*! \brief invert the direction of this spline
     */
    void invert_direction();
	/*! \brief simplify the spline by removing points
	 *
	 * \param criterium remove each point that is weighted less than this
	 * \return true if spline is simplified or already simplified
	 */
    bool simplify(float criterium);
    virtual void clear();
    virtual void rebuild();
    void setBuild(bool val);

    // geometry ops
    float coord_length(float t1, float t2) const;
    float chord_length_approximation(float percentage) const;
    float curvature(float parameter, QVector3D& normal) const;
    QVector3D first_derive(float parameter) const;
    QVector3D second_derive(float parameter) const;
    bool intersect_plane(const Plane& plane, IntersectionData& output) const;
    QVector3D value(float parameter) const;

    // persistence
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& destination) const;
    void saveToDXF(QStringList& strings, QString layername,
             bool sendmirror) const;

    // drawing
    //int distance_to_cursor(int x, int y, Viewport& vp) const;
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void drawStarboard(Viewport& vp, LineShader* lineshader);

    // getters/setters

    bool showCurvature() const
        {return _show_curvature;}
    void setShowCurvature(bool val)
        {_show_curvature = val;}
    QColor getCurvatureColor() const
        {return _curvature_color;}
    void setCurvatureColor(const QColor& val)
        {_curvature_color=val;}
    float getCurvatureScale() const
        {return _curvature_scale;}
    void setCurvatureScale(float val)
        {_curvature_scale=val;}
    float getParameter(size_t index) const;
    QVector3D getPoint(size_t index) const
        {return _points[index];}
    QVector3D getFirstPoint() const
        {return _points.front();}
    QVector3D getLastPoint() const
        {return _points.back();}
    void setPoint(size_t index, const QVector3D& p);
    size_t getFragments() const
        {return _fragments;}
    void setFragments(size_t val);
    bool isKnuckle(size_t index) const
        {return _knuckles[index];}
    void setKnuckle(size_t index, bool val);
    size_t numberOfPoints() const
        { return _nopoints; }

    // output
    void dump(std::ostream& os) const;

private:

    // methods used in simplify
    float weight(size_t index, float total_length);
    std::vector<float>::iterator find_next_point(std::vector<float>& weights);

private:

    size_t _nopoints;
    size_t _fragments;
    bool _show_curvature;
    bool _show_points;
    float _curvature_scale;
    QColor _curvature_color;
    std::vector<QVector3D> _points;
    std::vector<bool> _knuckles;

    // cached elements
    float _total_length;
    std::vector<float> _parameters;
    std::vector<QVector3D> _derivatives;
};

typedef PointerVector<Spline> SplineVector;

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::Spline& spline);

#endif

