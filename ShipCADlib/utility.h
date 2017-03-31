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

#ifndef UTILITY_H_
#define UTILITY_H_

#include <vector>
#include <QVector3D>
#include <QColor>
#include <QString>
#include "spline.h"
#include "plane.h"
#include "projsettings.h"

namespace ShipCAD {

    /*! \brief find the min and max coordinates from a point
     *
     * given a point, see if that point coordinates are smaller or larger
     * in each axis compared to the min and max parameters
     *
     * \param p the point to check for new min/max
     * \param min the current minimum vector
     * \param max the current maximum vector
     */
    void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max);
    /*! \brief find the distance from a point to the line
     *
     * \param p the point
     * \param l1 the first point on the line
     * \param l2 a second point on the line
     * \return the nearest distance from the point to the line
     */
    float DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2);

    /*! \brief linearly interpolate a point between 2 points
     *
     * \param p1 first point
     * \param p2 second point
     * \param the ratio of the returned point along p1->p2 from p1
     * \return the interpolated point
     */
    QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, float param);

    /*! \brief get the midpoint between 2 points (interpolate 50%)
     *
     * \param p1 the first point
     * \param p2 the second point
     * \return point halfway between p1 and p2
     */
    QVector3D MidPoint(const QVector3D& p1, const QVector3D& p2);

    /*! \brief project a point to a line
     *
     * \param p1 the first point on line
     * \param p2 the second point on line
     * \param p the point to project
     * \return point projected onto line
     */
    QVector3D PointProjectToLine(const QVector3D& p1, const QVector3D& p2, const QVector3D& p);

    /*! \brief convert a QColor to a DXF color index
     *
     * \param color the color to convert
     * \return the DXF index of the color
     */
    int FindDXFColorIndex(QColor color);

    /*! \brief get a QColor from a DXF color index
     *
     * \param index the DXF index (0-255)
     * \return the QColor corresponding to the DXF color index
     * \throws std::range_error if index is not a DXF color index
     */
    QColor QColorFromDXFIndex(int index);

    /*! \brief calculate color on basis of wavelength (0.0 - 1.0)
     *
     * \param parameter
     * \return the color
     */
    QColor FillColor(float parameter);

    // calculate the normal of a plane defined by points p1,p2,p3 and scale to unit-length
    /*! \brief find the unit normal of a plane defined by 3 points
     *
     * \param p1 first point on plane
     * \param p2 second point on plane
     * \param p3 third point on plane
     * \return unit normal of plane
     */
    QVector3D UnifiedNormal(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3);

    /*! \brief convert degrees to radians
     *
     * \param deg degree value to convert
     * \return value in radians
     */
    float DegToRad(float deg);

    /*! \brief convert radians to degrees
     *
     * \param rad radian value to convert
     * \return value in degrees
     */
    float RadToDeg(float rad);

    /*! \brief find if point lies inside a triangle and on the plane of the triangle
     *
     * \param intercept the point to check
     * \param p0 first vertex point on plane
     * \param p1 second vertex point on plane
     * \param p2 third vertex point on plane
     * \return true if intercept is on triangular face
     */
    bool PointInTriangle(const QVector3D& intercept,
                                    const QVector3D& p0,
                                    const QVector3D& p1,
                                    const QVector3D& p2);

    /*! \brief clip a triangle given vertex point distances from a plane
     *
     * 3 points each with a distance to a plane are converted into 2 lists of points
     * that are in front of the plane, or those in back of the plane. If the plane cuts
     * through the triangle, then an intersection point on each side of the plane is added
     * to the lists
     *
     * \param p1 first vertex point
     * \param p2 second vertex point
     * \param p3 third vertex point
     * \param s1 first vertex point distance to plane
     * \param s2 second vertex point distance to plane
     * \param s3 third vertex point distance to plane
     * \param front result list of points in front of plane
     * \param back result list of points in back of plane
     */
    void ClipTriangle(const QVector3D& p1,
                      const QVector3D& p2,
                      const QVector3D& p3,
                     float s1,
                     float s2,
                     float s3,
                     std::vector<QVector3D>& front,
                     std::vector<QVector3D>& back);

    /*! \brief clip a triangle given a plane
     *
     * 3 points and a plane are converted into 2 lists of points
     * that are in front of the plane, or those in back of the plane. If the plane cuts
     * through the triangle, then an intersection point on each side of the plane is added
     * to the lists
     *
     * \param p1 first vertex point
     * \param p2 second vertex point
     * \param p3 third vertex point
     * \param plane the plane to check against
     * \param front result list of points in front of plane
     * \param back result list of points in back of plane
     */
    void ClipTriangle(const QVector3D& p1,
							 const QVector3D& p2,
							 const QVector3D& p3,
							 const Plane& plane,
							 std::vector<QVector3D>& front,
							 std::vector<QVector3D>& back);

    /*! \brief squared distance between 2 3D points
	 *
	 * \param p1 first point
	 * \param p2 second point
	 * \return square of distance between points
	 */
    float SquaredDistPP(const QVector3D& p1, const QVector3D& p2);

    /*! \brief distance between 2 3D points
	 *
	 * \param p1 first point
	 * \param p2 second point
	 * \return distance between points
	 */
    float DistPP3D(const QVector3D& p1, const QVector3D& p2);

    /*! \brief convert a bool to a string for serialization
     *
     * \param val boolean value
     * \return QString containing "0" or "1" depending on value
     */
    QString BoolToStr(bool val);

    /*! \brief join a set of linesegments and connect them into as few as possible splines
     *
     * \param join_error points closer than this are joined together
     * \param force_to_one_segment true if splines should be joined even if end points are farther apart
     * then join_error
     * \param list of splines to join together
     */
    void JoinSplineSegments(float join_error, bool force_to_one_segment,
								   SplineVector& list);

    /*! \brief convert a string to an integer value
     *
     * \param lineno indicates which line of the file we are on for errors
     * \param str QString to extract integer
     * \param start where to start extracting integer
     * \return integer value extracted from string, start will be incremented to after integer
     */
    int ReadIntFromStr(size_t lineno, const QString& str, size_t& start);

    /*! \brief convert a string to an boolean value
     *
     * \param lineno indicates which line of the file we are on for errors
     * \param str QString to extract boolean
     * \param start where to start extracting boolean
     * \return boolean value extracted from string, start will be incremented to after integer
     */
    bool ReadBoolFromStr(size_t lineno, const QString& str, size_t& start);

    /*! \brief convert a string to an float value
     *
     * \param lineno indicates which line of the file we are on for errors
     * \param str QString to extract float
     * \param start where to start extracting float
     * \return float value extracted from string, start will be incremented to after float
     */
    float ReadFloatFromStr(size_t lineno, const QString& str, size_t& start);

    /*! \brief find water viscosity based on density
	 *
	 * \param density the water density
     * \param units (imperial or metric)
	 * \returns the water viscosity
	 */
    float FindWaterViscosity(float density, unit_type_t units);

    /*! \brief Append or change the file extension to that given
	 *
	 * The extension is the last . (dot) in the filename and all trailing
	 * characters. This function replaces these characters with the specified
	 * extension, or if there is no dot in the name, appends the extension
	 *
	 * \param name name of file
	 * \param ext extension to add to filename (including starting .)
	 * \return the filename with extension
	 */
    QString ChangeFileExt(const QString& name, const QString& ext);

    /*! \brief convert volume to displacement given density, appendage coefficient and unit type
     *
     * \param volume to convert
     * \param density the density of water
     * \param appcoeff the appendage coefficient
     * \param units (imperial or metric)
     * \return displacement
     */
    float VolumeToDisplacement(float volume, float density, float appcoeff, unit_type_t units);

	/*! \brief Create a formatted string from a float value
	 *
	 * \param value the floating point value
	 * \param decimals
	 * \param des_length the finished length of the string, add leading spaces to fill out
	 * \return the fixed length string
	 */
    QString MakeLength(float value, int decimals, int des_length);

	/*! \brief Create a string of a given length
	 *
	 * \param value the string to start with
	 * \param des_length the finished length of the string, add leading spaces to fill out
	 * \return the fixed length string
	 */
    QString MakeLength(const QString& value, int des_length);

	/*! \brief convert a float value to a string with max number of decimals
	 *
	 * \param val the floating point value
	 * \param max_length the maximum number of decimals to show in the string
	 * \return the string with the converted floating point number
	 */
    QString truncate(float val, int max_length);

    /*! \brief compare 2 floats, if they are sufficiently close, return true
     *
     * \param val1 first value to compare
     * \param val2 second value to compare
     * \param error if absolute difference between the 2 values is less than this, then equal
     * \return true if values are sufficiently equal
     */
    bool FuzzyCompare(float val1, float val2, float error);

    /*! \brief get the sign of a floating point number
     *
     * \param val value to compute sign of
     * \return -1, 0, 1
     */
    double Sign(double val);

    /*! \brief convert an int to a size_t
     * * \param val the integer
     * * \return corresponding size_t
     */
    size_t to_size_t(int val);
};

#endif
