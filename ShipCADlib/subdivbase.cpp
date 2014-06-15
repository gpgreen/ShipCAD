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

void SubdivisionBase::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionBase [" << hex << this << "]";
    priv_dump(os, prefix);
}

void SubdivisionBase::priv_dump(ostream& os, const char* prefix) const
{
    os << prefix << " Owner [" << hex << _owner << "]";
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionBase& base)
{
    base.dump(os);
    return os;
}
