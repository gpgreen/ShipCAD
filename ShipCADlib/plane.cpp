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

QPair<QVector3D, QVector3D> Plane::vertex_normal() const
{
    QVector3D vertex;
    QVector3D normal(_vars[0], _vars[1], _vars[2]);
    return qMakePair(vertex, normal);
}

