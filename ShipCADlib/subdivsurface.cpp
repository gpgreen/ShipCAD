#include <iostream>
#include <algorithm>

#include "subdivsurface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "subdivcontrolcurve.h"
#include "plane.h"
#include "subdivlayer.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "version.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

static QVector3D ZERO = QVector3D(0,0,0);

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionSurface::SubdivisionSurface()
{
    clear();
}

SubdivisionSurface::~SubdivisionSurface()
{
    // does nothing
}

SubdivisionControlPoint* SubdivisionSurface::newControlPoint(const QVector3D& p)
{
    SubdivisionControlPoint* pt = new SubdivisionControlPoint(this);
    pt->setCoordinate(p);
    _control_points.push_back(pt);
    return pt;
}

SubdivisionControlPoint* SubdivisionSurface::addControlPoint(const QVector3D& pt)
{
    SubdivisionControlPoint* result = 0;
    double max_error = 1E-5;
    for (size_t i=1; i<=numberOfControlEdges(); ++i) {
        SubdivisionControlEdge* edge = getControlEdge(i-1);
        if (edge->numberOfFaces() <= 1) { // boundary edge
            if (SquaredDistPP(pt, edge->startPoint()->getCoordinate()) <= max_error) {
                result = dynamic_cast<SubdivisionControlPoint*>(edge->startPoint());
                break;
            }
            else if (SquaredDistPP(pt, edge->endPoint()->getCoordinate()) <= max_error) {
                result = dynamic_cast<SubdivisionControlPoint*>(edge->endPoint());
                break;
            }
        }
    }
    if (result == 0) {
        // search controlpoints without edges
        for (size_t i=1; i<=numberOfControlPoints(); ++i) {
            SubdivisionControlPoint* point = getControlPoint(i-1);
            if (point->numberOfEdges() == 0) {
                if (SquaredDistPP(pt, point->getCoordinate()) <= max_error) {
                    result = point;
                    break;
                }
            }
        }
    }
    if (result == 0) {
        result = newControlPoint(pt);
    }
    return result;
}

void SubdivisionSurface::addControlPoint(SubdivisionControlPoint* pt)
{
    if (!hasControlPoint(pt)) {
        _control_points.push_back(pt);
        pt->setOwner(this);
    }
    setBuild(false);
}

SubdivisionControlPoint* SubdivisionSurface::addControlPoint()
{
    SubdivisionControlPoint* result = new SubdivisionControlPoint(this);
    result->setCoordinate(ZERO);
    _control_points.push_back(result);
    return result;
}

SubdivisionLayer* SubdivisionSurface::addNewLayer()
{
    SubdivisionLayer* result = new SubdivisionLayer(this);
    _layers.push_back(result);
    result->setLayerID(requestNewLayerID());
    setActiveLayer(result);
    emit changedLayerData();
    return result;
}

// used in assembleFacesToPatches
SubdivisionControlFace* SubdivisionSurface::getControlFace(SubdivisionPoint* p1,
                                                           SubdivisionPoint* p2,
                                                           SubdivisionPoint* p3,
                                                           SubdivisionPoint* p4)
{
    SubdivisionFace* face;
    SubdivisionControlFace* cface = 0;
    for (size_t i=1; i<=p1->numberOfFaces(); ++i) {
        face = p1->getFace(i-1);
        if (p2->hasFace(face) && p3->hasFace(face) && p4->hasFace(face)) {
            cface = dynamic_cast<SubdivisionControlFace*>(face);
            break;
        }
    }
    return cface;
}

// tries to assemble quads into as few as possible rectangular patches
void SubdivisionSurface::assembleFacesToPatches(vector<SubdivisionLayer*>& layers,
                                                assemble_mode_t mode,
                                                vector<SubdivisionFace*>& assembledPatches,
                                                size_t& nAssembled)
{
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
    os << "SubdivisionSurface ["
       << hex << this << "]\n";
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionSurface& surface)
{
    surface.dump(os);
    return os;
}
