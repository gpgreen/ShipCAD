#include <iostream>

#include "nurbsurface.h"
#include "viewport.h"

using namespace ShipCADGeometry;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////

NURBSurface::NURBSurface()
    : Entity()
{
    clear();
}

NURBSurface::~NURBSurface()
{
    // does nothing
}

void NURBSurface::setBuild(bool val)
{
    if (!val) {
    }
    Entity::setBuild(val);
}

void NURBSurface::setPoint(int index, const QVector3D& p)
{
}

QVector3D NURBSurface::getPoint(int index) const
{
}

void NURBSurface::rebuild()
{
    _build = false;
    _build = true;
}

void NURBSurface::draw(Viewport& vp)
{
}

void NURBSurface::clear()
{
}

void NURBSurface::dump(ostream& os) const
{
    os << "NURBSurface";
    Entity::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::NURBSurface& surface)
{
    surface.dump(os);
    return os;
}
