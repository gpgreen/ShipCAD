#include <iostream>

#include "subdivlayer.h"

using namespace std;
using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionLayer::SubdivisionLayer(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    // does nothing
}

SubdivisionLayer::~SubdivisionLayer()
{
    // does nothing
}

void SubdivisionLayer::dump(ostream& os) const
{
    os << " Owner:" << hex << _owner;
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionLayer& layer)
{
    layer.dump(os);
    return os;
}
