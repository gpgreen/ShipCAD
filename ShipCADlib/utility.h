#ifndef UTILITY_H_
#define UTILITY_H_

#include <vector>
#include <QVector3D>
#include <QColor>
#include <QString>
#include "spline.h"
#include "plane.h"

namespace ShipCADUtility {

extern void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max);

extern float DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2);

extern QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, float param);

extern QVector3D MidPoint(const QVector3D& p1, const QVector3D& p2);

// convert a color to a DXF color index
extern int FindDXFColorIndex(QColor color);

// convert a DXF color index to a QColor
extern QColor QColorFromDXFIndex(int index);

// convert a float to a string with a max number of specified decimals
extern QString truncate(float val, int max_length);

// calculate the normal of a plane defined by points p1,p2,p3 and scale to unit-length
extern QVector3D UnifiedNormal(const QVector3D& p1, const QVector3D& p2, 
			       const QVector3D& p3);

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
                         const ShipCADGeometry::Plane& plane,
                         std::vector<QVector3D>& front,
                         std::vector<QVector3D>& back);

// calculates the squared distance between 2 points
extern float SquaredDistPP(const QVector3D& p1, const QVector3D& p2);

// returns "0" for false, and "1" for true
extern QString BoolToStr(bool val);

// This procedure takes a lot of linesegments and tries to connect them into as few as possible splines
extern void JoinSplineSegments(float join_error, bool force_to_one_segment,
                               std::vector<ShipCADGeometry::Spline*> list);

extern int ReadIntFromStr(size_t lineno, const QString& str, size_t& start);
extern bool ReadBoolFromStr(size_t lineno, const QString& str, size_t& start);
extern float ReadFloatFromStr(size_t lineno, const QString& str, size_t& start);

};

#endif
