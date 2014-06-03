#include <iostream>
#include <cmath>

#include "subdivpoint.h"
#include "subdivsurface.h"
#include "subdivedge.h"
#include "subdivface.h"
#include "exception.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"

using namespace ShipCADException;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;
using namespace std;

static QVector3D ZERO = QVector3D(0,0,0);

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionPoint::SubdivisionPoint(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionPoint::~SubdivisionPoint()
{
    // does nothing
}

void SubdivisionPoint::clear()
{
    _coordinate = ZERO;
    _faces.clear();
    _edges.clear();
    _vtype = svRegular;
}

SubdivisionEdge* SubdivisionPoint::getEdge(int index) const
{
    if (index >= 0 && index < _edges.size())
        return _edges[index];
    throw ListIndexOutOfBounds(__FILE__);
}

SubdivisionFace* SubdivisionPoint::getFace(int index) const
{
    if (index >= 0 && index < _faces.size())
        return _faces[index];
    throw ListIndexOutOfBounds(__FILE__);
}

int SubdivisionPoint::getIndex() const
{
    return _owner->getPointIndex(*this);
}

QVector3D SubdivisionPoint::getCoordinate() const
{
    return _coordinate;
}

float SubdivisionPoint::curvature() const
{
    return 0.0;
}

bool SubdivisionPoint::isBoundaryVertex() const
{
    bool result = false;
    if (fabs(_coordinate.y()) > 1E-4) {
        for (size_t i=0; i<_edges.size(); ++i)
            result = result || _edges[i]->isBoundaryEdge();
    }
    return result;
}

QVector3D SubdivisionPoint::getNormal() const
{
    QVector3D result;
    for (size_t i=0; i<_faces.size(); ++i) {
        SubdivisionFace* face = _faces[i];
        if (face->numberOfPoints() > 4) {
            // face possibly concave at this point
            // use the normal of all points from this face
            QVector3D c = face->faceCenter();
            QVector3D n = ZERO;
            for (size_t j=1; j<face->numberOfPoints(); ++j) {
                n = n + UnifiedNormal(c, face->getPoint(j-1).getCoordinate(),
                                      face->getPoint(j).getCoordinate());
            }
            n.normalize();
            result = result + n;
        }
        else {
            int index = face->getPointIndex(*this);
            int j = (index + face->numberOfPoints() - 1) % face->numberOfPoints();
            QVector3D p1 = face->getPoint(j).getCoordinate();
            j = (index + face->numberOfPoints() + 1) % face->numberOfPoints();
            QVector3D p3 = face->getPoint(j).getCoordinate();
            result = result + UnifiedNormal(p1, getCoordinate(), p3);
        }
    }
    result.normalize();
    return result;
}

void SubdivisionPoint::dump(ostream& os) const
{
    os << "SubdivisionPoint";
    SubdivisionBase::dump(os);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionPoint& point)
{
    point.dump(os);
    return os;
}
