#include <iostream>

#include "subdivcontrolcurve.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "spline.h"
#include "viewport.h"
#include "filebuffer.h"

using namespace std;
using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionControlCurve::SubdivisionControlCurve(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionControlCurve::~SubdivisionControlCurve()
{
    // does nothing
}

void SubdivisionControlCurve::dump(ostream& os) const
{
    os << "SubdivisionControlCurve ["
       << hex << this << "]\n";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionControlCurve& curve)
{
    curve.dump(os);
    return os;
}
