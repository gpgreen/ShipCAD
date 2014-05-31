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
    
  QVector3D Interpolate(const QVector3D& p1, const QVector3D& p2, qreal param)
  {
      QVector3D result = p1;
      result = p2;
      param = 0;
      return result;
  }
    

};

#endif
