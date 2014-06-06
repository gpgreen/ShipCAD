#include <iostream>
#include <algorithm>

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

void SubdivisionSurface::setSelectedControlEdge(SubdivisionControlEdge* edge)
{
  if (!hasSelectedControlEdge(edge))
    _sel_control_edges.push_back(edge);
}

bool SubdivisionSurface::hasSelectedControlEdge(SubdivisionControlEdge* edge)
{
  return (find(_sel_control_edges.begin(), _sel_control_edges.end(), edge) 
	  != _sel_control_edges.end());
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
