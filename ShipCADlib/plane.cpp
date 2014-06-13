#include "plane.h"

using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

Plane::Plane()
{
    _vars[0] = _vars[1] = _vars[2] = _vars[3] = 0;
}

Plane::Plane(float a, float b, float c, float d)
{
    _vars[0] = a;
    _vars[1] = b;
    _vars[2] = c;
    _vars[3] = d;
}

Plane::Plane(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
  // calculate normal
  QVector3D n;
  n.setX((p2.y() - p1.y()) * (p3.x() - p1.x()) - (p2.x() - p1.x()) * (p3.y() - p1.y()));
  n.setY((p2.x() - p1.x()) * (p3.x() - p1.x()) - (p2.x() - p1.x()) * (p3.x() - p1.x()));
  n.setZ((p2.x() - p1.x()) * (p3.y() - p1.y()) - (p2.y() - p1.y()) * (p3.x() - p1.x()));
  n.normalize();
  _vars[0] = n.x() / n.length();
  _vars[1] = n.y() / n.length();
  _vars[2] = n.z() / n.length();
  _vars[3] = -p1.x() * _vars[0] - p1.y() * _vars[1] - p1.z() * _vars[2];
}

float Plane::distance(const QVector3D& point) const
{
    return _vars[0] * point.x() + _vars[1] * point.y()
            + _vars[2] * point.z() + _vars[3];
}
