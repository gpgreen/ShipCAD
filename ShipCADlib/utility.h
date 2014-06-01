#ifndef UTILITY_H_
#define UTILITY_H_

#include <QtCore>

namespace ShipCADUtility {

  void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max)
  {
    if (p.x()<min.x()) min.setX(p.x());
    if (p.y()<min.y()) min.setY(p.y());
    if (p.z()<min.z()) min.setZ(p.z());
    if (p.x()>max.x()) max.setX(p.x());
    if (p.y()>max.y()) max.setY(p.y());
    if (p.z()>max.z()) max.setZ(p.z());
  }

  qreal DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2)
  {
      QVector3D vec = l2 - l1;
      vec.normalize();
      return p.distanceToLine(l1, vec);
  }

#if 0
  qreal DistanceToLine(const QVector2D& p1, const QVector2D& p2, 
		       int x, int y, qreal& parameter)
  {
      qreal result = 0;
      x = y  = 0;
      parameter = 0;
      result = p1.x();
      result = p2.x();
      return result;
  }
#endif

  QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, float param)
  {
      QVector3D result;
      result.setX(p1.x() + param * (p2.x() - p1.x()));
      result.setY(p1.y() + param * (p2.y() - p1.y()));
      result.setX(p1.z() + param * (p2.z() - p1.z()));
      return result;
  }

};

#endif
