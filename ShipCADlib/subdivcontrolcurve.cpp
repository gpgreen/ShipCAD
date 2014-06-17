#include <iostream>
#include <stdexcept>

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

SubdivisionControlCurve* SubdivisionControlCurve::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getControlCurvePool().malloc();
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionControlCurve::construct");
    return new (mem) SubdivisionControlCurve(owner);
}

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
        _owner->removeControlCurve(this);
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

void SubdivisionControlCurve::addPoint(SubdivisionControlPoint* p)
{
    _points.push_back(p);
    setBuild(false);
}

void SubdivisionControlCurve::resetDivPoints()
{
    _div_points.clear();
    for (size_t i=0; i<_points.size(); ++i)
        _div_points.push_back(_points[i]);
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

SubdivisionPoint* SubdivisionControlCurve::getSubdivPoint(size_t index)
{
    if (index < _div_points.size())
        return _div_points[index];
    throw range_error("index for SubdivisionControlCurve::getSubdivPoint");
}

void SubdivisionControlCurve::clear()
{
    _points.clear();
    _curve->clear();
    _div_points.clear();
    setBuild(false);
}

void SubdivisionControlCurve::deleteEdge(SubdivisionControlEdge* edge)
{
    bool delcurve = false;
    SubdivisionControlPoint* p1, *p2;
    SubdivisionEdge* anedge;

    size_t i = 2;
    while (i <= _points.size()) {
        if ((_points[i-2] == edge->startPoint() && _points[i-1] == edge->endPoint())
             || (_points[i-2] == edge->endPoint() && _points[i-1] == edge->startPoint())) {
            // remove references to this curve from control edges
            for (size_t j=2; j<=_points.size(); ++j) {
                anedge = _owner->edgeExists(_points[j-2], _points[j-1]);
                if (anedge != 0)
                    anedge->setCurve(0);
            }
            // remove references from subdivided edges
            for (size_t j=2; j<=_div_points.size(); ++j) {
                anedge = _owner->edgeExists(_div_points[j-2], _div_points[j-1]);
                if (anedge != 0)
                    anedge->setCurve(0);
            }
            _div_points.clear();

            if (i - 2 > 0) {
                // build first new curve
                SubdivisionControlCurve* newcurve = SubdivisionControlCurve::construct(_owner);
                _owner->addControlCurve(newcurve);
                p1 = 0;
                for (size_t j=0; j<=i-2; ++j) {
                    p2 = _points[j];
                    newcurve->addPoint(p2);
                    if (j > 0) {
                        anedge = _owner->edgeExists(p1, p2);
                        if (anedge != 0)
                            anedge->setCurve(newcurve);
                    }
                    p1 = p2;
                }
                newcurve->setSelected(isSelected());
            }
            if (i - 1 < _points.size() - 1) {
                // build second new curve
                SubdivisionControlCurve* newcurve = SubdivisionControlCurve::construct(_owner);
                _owner->addControlCurve(newcurve);
                p1 = 0;
                for (size_t j=i-1; j<=_points.size()-1; ++j) {
                    p2 = _points[j];
                    newcurve->addPoint(p2);
                    if (j > i - 1) {
                        anedge = _owner->edgeExists(p1, p2);
                        if (anedge != 0)
                            anedge->setCurve(newcurve);
                    }
                    p1 = p2;
                }
                newcurve->setSelected(isSelected());
            }
            delcurve = true;
            break;
        }
        ++i;
    }   // end while
    if (delcurve) {
        _points.clear();
        if (_owner->hasSelectedControlCurve(this))
            _owner->removeSelectedControlCurve(this);
        if (_owner->hasControlCurve(this))
            _owner->removeControlCurve(this);
        // BUGBUG: we are supposed to delete this curve now, but can't
    }
}

void SubdivisionControlCurve::draw(Viewport &vp)
{

}

void SubdivisionControlCurve::insertControlPoint(SubdivisionControlPoint *p1,
                                                 SubdivisionControlPoint *p2,
                                                 SubdivisionControlPoint *newpt)
{
    size_t i = 2;
    while (i <= _points.size()) {
        if ((_points[i-2] == p1 && _points[i-1] == p2) || (_points[i-1] == p1 && _points[i-2] == p2))
            _points.insert(_points.begin()+i-1, newpt);
        i++;
    }
}

void SubdivisionControlCurve::insertEdgePoint(SubdivisionPoint *p1,
                                              SubdivisionPoint *p2,
                                              SubdivisionPoint *newpt)
{
    size_t i = 2;
    while (i <= _div_points.size()) {
        if ((_div_points[i-2] == p1 && _div_points[i-1] == p2) || (_div_points[i-1] == p1 && _div_points[i-2] == p2))
            _div_points.insert(_div_points.begin()+i-1, newpt);
        i++;
    }
}

void SubdivisionControlCurve::loadBinary(FileBuffer &source)
{
    size_t n, ind;
    source.load(n);
    SubdivisionControlPoint* p1 = 0;
    for (size_t i=1; i<=n; ++i) {
        source.load(ind);
        SubdivisionControlPoint* p2 = _owner->getControlPoint(ind);
        _points.push_back(p2);
        if (i > 1) {
            SubdivisionEdge* edge = _owner->edgeExists(p1, p2);
            if (edge != 0)
                edge->setCurve(this);
        }
        p1 = p2;
    }
    for (size_t i=0; i<_points.size(); ++i)
        _div_points.push_back(_points[i]);
    bool sel;
    source.load(sel);
    if (sel)
        setSelected(true);
}

void SubdivisionControlCurve::replaceVertexPoint(SubdivisionPoint *oldpt, SubdivisionPoint *newpt)
{
    for (size_t i=1; i<=_div_points.size(); ++i) {
        if (_div_points[i-1] == oldpt) {
            _div_points[i-1] = newpt;
            _curve->clear();
        }
    }
}

void SubdivisionControlCurve::saveBinary(FileBuffer &destination)
{
    destination.add(numberOfControlPoints());
    for (size_t i=1; i<=numberOfControlPoints(); ++i) {
        SubdivisionControlPoint* p = _points[i-1];
        size_t ind = _owner->indexOfControlPoint(p);
        destination.add(ind);
    }
    destination.add(isSelected());
}

void SubdivisionControlCurve::saveToDXF(vector<QString> &strings)
{
    QString layer("Control_curves");
    _curve->setFragments(_curve->numberOfPoints());
    _curve->save_to_dxf(strings, layer, _owner->isDrawMirror());
}

void SubdivisionControlCurve::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionControlCurve ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionControlCurve::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionBase::priv_dump(os, prefix);
    //os << "SubdivisionControlCurve ["
    //   << hex << this << "]\n";
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionControlCurve& curve)
{
    curve.dump(os);
    return os;
}
