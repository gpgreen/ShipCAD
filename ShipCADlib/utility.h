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

extern int FindDXFColorIndex(QColor color);

};

#endif
