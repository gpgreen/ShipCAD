#include <iostream>

#include "subdivbase.h"
#include "subdivsurface.h"

using namespace std;
using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionBase::SubdivisionBase(SubdivisionSurface* owner)
    : _owner(owner)
{
    // does nothing
}

SubdivisionBase::~SubdivisionBase()
{
    // does nothing
}

SubdivisionSurface* SubdivisionBase::getOwner() const
{
    return _owner;
}

void SubdivisionBase::dump(ostream& os) const
{
    os << " Owner:" << hex << _owner;
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionBase& base)
{
    base.dump(os);
    return os;
}
