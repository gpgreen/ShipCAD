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
  if (isSelected())
    setSelected(false);
  if (_owner->hasControlCurve(this))
    _owner->deleteControlCurve(this);
  // remove references from control edges
  for (size_t i=1; i<_points.size(); ++i) {
    SubdivisionControlPoint* p1 = _points[i-1];
    SubdivisionControlPoint* p2 = _points[i];
    SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
    if (edge != 0)
      edge->setCurve(0);
  }
  // remove references from subdivided edges
  for (size_t i=1; i<_div_points.size(); ++i) {
    SubdivisionPoint* p1 = _div_points[i-1];
    SubdivisionPoint* p2 = _div_points[i];
    SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
    if (edge != 0)
      edge->setCurve(0);
  }
}

void SubdivisionControlCurve::setSelected(bool val)
{
  bool have = _owner->hasSelectedControlCurve(this);
  if (val && !have)
    _owner->setSelectedControlCurve(this);
  else if (!val && have)
    _owner->removeSelectedControlCurve(this);
}

void SubdivisionControlCurve::addPoint(SubdivisionPoint* p)
{
  _points.push_back(p);
  setBuild(false);
}

QColor SubdivisionControlCurve::getColor()
{
  if (isSelected())
    return _owner->getSelectedColor();
  return _owner->getControlCurveColor();
}

bool SubdivisionControlCurve::isSelected()
{
  return _owner->hasSelectedControlCurve(this);
}

bool SubdivisionControlCurve::isVisible()
{
  return _owner->showControlCurves();
}

SubdivisionControlPoint* SubdivisionControlCurve::getControlPoint(size_t index)
{
  if (index < _points.size())
    return _points[index];
  throw range_error("index for SubdivisionControlCurve::getControlPoint");
}

void SubdivisionControlCurve::clear()
{
  _points.clear();
  _curve.clear();
  _div_points.clear();
  setBuild(false);
}

void SubdivisionControlCurve::deleteEdge(SubdivisionControlEdge* edge)
{
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
