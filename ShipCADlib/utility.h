#ifndef UTILITY_H_
#define UTILITY_H_

#include <QVector3D>
#include <QColor>

namespace ShipCADUtility {

extern void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max);

extern float DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2);

#if 0
extern float DistanceToLine(const QVector2D& p1, const QVector2D& p2,
                     int x, int y, qreal& parameter);
#endif

extern QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, float param);

// convert a color to a DXF color index
extern int FindDXFColorIndex(QColor color);

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
};

#endif
