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

	extern void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max);

	extern float DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2);

	extern QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, float param);

	extern QVector3D MidPoint(const QVector3D& p1, const QVector3D& p2);

    // convert a color to a DXF color index
	extern int FindDXFColorIndex(QColor color);

    // convert a DXF color index to a QColor
	extern QColor QColorFromDXFIndex(int index);

    // calculate the normal of a plane defined by points p1,p2,p3 and scale to unit-length
	extern QVector3D UnifiedNormal(const QVector3D& p1, const QVector3D& p2, 
								   const QVector3D& p3);

	extern float DegToRad(float deg);

	extern float RadToDeg(float rad);

    // this function calculates if a point lies inside a triangle
    // assuming it lies on the plane determined by the triangle
	extern bool PointInTriangle(const QVector3D& intercept,
								const QVector3D& p0,
								const QVector3D& p1,
								const QVector3D& p2);

    // clip a triangle given the 3 distances from a plane, 
    // returns 2 sets of coordinates, front of the plane, and back of the plane
	extern void ClipTriangle(const QVector3D& p1,
							 const QVector3D& p2,
							 const QVector3D& p3,
							 float s1,
							 float s2,
							 float s3,
							 std::vector<QVector3D>& front,
							 std::vector<QVector3D>& back);

    // clip a triangle given a plane, returns 2 sets of coordinates, front of the plane, and
    // back of the plane
	extern void ClipTriangle(const QVector3D& p1,
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
	extern float SquaredDistPP(const QVector3D& p1, const QVector3D& p2);

    /*! \brief distance between 2 3D points
	 *
	 * \param p1 first point
	 * \param p2 second point
	 * \return distance between points
	 */
	extern float DistPP3D(const QVector3D& p1, const QVector3D& p2);

    // returns "0" for false, and "1" for true
	extern QString BoolToStr(bool val);

    // This procedure takes a lot of linesegments and tries to connect them into as few as possible splines
	extern void JoinSplineSegments(float join_error, bool force_to_one_segment,
								   SplineVector& list);

	extern int ReadIntFromStr(size_t lineno, const QString& str, size_t& start);
	extern bool ReadBoolFromStr(size_t lineno, const QString& str, size_t& start);
	extern float ReadFloatFromStr(size_t lineno, const QString& str, size_t& start);

    /*! \brief find water viscosity based on density
	 *
	 * \param density the water density
	 * \param units
	 * \returns the water viscosity
	 */
	extern float FindWaterViscosity(float density, unit_type_t units);

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
	extern QString ChangeFileExt(const QString& name, const QString& ext);

	extern float VolumeToDisplacement(float volume, float density, float appcoeff, unit_type_t units);

	/*! \brief Create a formatted string from a float value
	 *
	 * \param value the floating point value
	 * \param decimals
	 * \param des_length the finished length of the string, add leading spaces to fill out
	 * \return the fixed length string
	 */
	extern QString MakeLength(float value, int decimals, int des_length);

	/*! \brief Create a string of a given length
	 *
	 * \param value the string to start with
	 * \param des_length the finished length of the string, add leading spaces to fill out
	 * \return the fixed length string
	 */
	extern QString MakeLength(const QString& value, int des_length);

    // convert a float to a string with a max number of specified decimals
	/*! \brief convert a float value to a string with max number of decimals
	 *
	 * \param val the floating point value
	 * \param max_length the maximum number of decimals to show in the string
	 * \return the string with the converted floating point number
	 */
	extern QString truncate(float val, int max_length);

    /*! \brief compare 2 floats, if they are sufficiently close, return true
     *
     * \param val1 first value to compare
     * \param val2 second value to compare
     * \param error if absolute difference between the 2 values is less than this, then equal
     * \return true if values are sufficiently equal
     */
    extern bool FuzzyCompare(float val1, float val2, float error);
};

#endif
