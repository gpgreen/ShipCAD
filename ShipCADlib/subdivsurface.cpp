#include <iostream>

#include "subdivsurface.h"

using namespace std;
using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionSurface::SubdivisionSurface()
{
  // does nothing
}

SubdivisionSurface::~SubdivisionSurface()
{
    // does nothing
}

void SubdivisionSurface::dump(ostream& os) const
{
  //  os << " Owner:" << hex() << _owner;
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionSurface& surface)
{
    surface.dump(os);
    return os;
}
