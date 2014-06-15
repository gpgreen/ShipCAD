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

bool Plane::intersectsBox(const QVector3D& p1, const QVector3D& p2) const
{
    QVector3D p[8];
    p[0] = p1;

    p[1].setX(p2.x());
    p[1].setY(p1.y());
    p[1].setZ(p1.z());

    p[2].setX(p2.x());
    p[2].setY(p2.y());
    p[2].setZ(p1.z());

    p[3].setX(p1.x());
    p[3].setY(p2.y());
    p[3].setZ(p1.z());

    p[4].setX(p1.x());
    p[4].setY(p1.y());
    p[4].setZ(p2.z());

    p[5].setX(p2.x());
    p[5].setY(p1.y());
    p[5].setZ(p2.z());

    p[6] = p2;

    p[7].setX(p1.x());
    p[7].setY(p2.y());
    p[7].setZ(p2.z());

    float smin, smax;
    for (size_t i=0; i<8; ++i) {
        float s = _vars[0] * p[i].x() + _vars[1] * p[i].y() + _vars[2] * p[i].z() + _vars[3];
        if (i == 0) {
            smin = s;
            smax = s;
        }
        if (s < smin)
            smin = s;
        if (s > smax)
            smax = s;
    }
    return (smin <= 0 && smax >= 0) || (smax <= 0 && smin >= 0);
}
