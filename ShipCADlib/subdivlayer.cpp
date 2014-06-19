#include <iostream>
#include <stdexcept>
#include <cmath>

#include "subdivlayer.h"
#include "subdivface.h"
#include "subdivsurface.h"
#include "subdivpoint.h"
#include "plane.h"
#include "subdivedge.h"
#include "utility.h"
#include "filebuffer.h"
#include "viewport.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

static QVector3D ZERO = QVector3D(0,0,0);

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionLayer* SubdivisionLayer::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getLayerPool().malloc();
    if (mem == 0)
        throw runtime_error("out of memory in SubdivisionLayer::construct");
    return new (mem) SubdivisionLayer(owner);
}

SubdivisionLayer::SubdivisionLayer(SubdivisionSurface* owner)
    : SubdivisionBase(owner)
{
    clear();
}

SubdivisionLayer::~SubdivisionLayer()
{
    for (size_t i=_patches.size(); i>0; --i)
        delete _patches[i-1];
    if (_owner->getActiveLayer() == this)
        _owner->setActiveLayer(0);
    if (_owner->hasLayer(this))
        _owner->deleteLayer(this);
}

QString SubdivisionLayer::getName()
{
    if (_desc == "") {
        // BUGBUG: resource string
        //QString userstring(33);
        return QString("default%1%2").arg(' ').arg(_layerid);
    }
    else
        return _desc;
}
SubdivisionControlFace* SubdivisionLayer::getFace(size_t index)
{
    if (index < _patches.size())
        return _patches[index];
    throw range_error("index out of range in SubdivisionLayer::getFace");
}

size_t SubdivisionLayer::getLayerIndex()
{
    return _owner->indexOfLayer(this);
}

void SubdivisionLayer::processTriangle(const QVector3D& p1, 
                                       const QVector3D& p2,
                                       const QVector3D& p3,
                                       LayerProperties& props)
{
    QVector3D center = (p1 + p2 + p3) / 3.0f;
    float ax = 0.5 * ((p1.y() - p2.y()) * (p1.z() + p2.z())
                      + (p2.y() - p3.y()) * (p2.z() + p3.z())
                      + (p3.y() - p1.y()) * (p3.z() + p1.z()));
    float ay = 0.5 * ((p1.z() - p2.z()) * (p1.x() + p2.x())
                      + (p2.z() - p3.z()) * (p2.x() + p3.x())
                      + (p3.z() - p1.z()) * (p3.x() + p1.x()));
    float az = 0.5 * ((p1.x() - p2.x()) * (p1.y() + p2.y())
                      + (p2.x() - p3.x()) * (p2.y() + p3.y())
                      + (p3.x() - p1.x()) * (p3.y() + p1.y()));
    float area = sqrt(ax * ax + ay * ay + az * az);
    props.surface_area += area;
    props.surface_center_of_gravity += (area * center);
}

LayerProperties SubdivisionLayer::getSurfaceProperties()
{
    LayerProperties result;
    result.surface_area = 0;
    result.weight = 0;
    result.surface_center_of_gravity = ZERO;

    for (size_t i=1; i<=_patches.size(); ++i) {
        for (size_t j=1; j<=_patches[i-1]->numberOfChildren(); ++j) {
            SubdivisionFace* child = _patches[i-1]->getChild(j-1);
            for (size_t k=3; k<=child->numberOfPoints(); ++k) {
                processTriangle(child->getPoint(0)->getCoordinate(),
                                child->getPoint(k-2)->getCoordinate(),
                                child->getPoint(k-1)->getCoordinate(),
                                result);
            }
        }
    }
    if (result.surface_area != 0) {
        result.surface_center_of_gravity /= result.surface_area;
        if (isSymmetric()) {
            result.surface_area *= 2.0f;
            result.surface_center_of_gravity.setY(0);
        }
        result.weight = result.surface_area * _thickness * _material_density;
    }
    return result;
}

void SubdivisionLayer::setDevelopable(bool val)
{
    if (val != _developable) {
        _developable = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setName(const QString& val)
{
    if (val.toUpper() != _desc.toUpper()) {
        _desc = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setSymmetric(bool val)
{
    if (val != _symmetric) {
        _symmetric = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setColor(QColor val)
{
    if (val != _color) {
        _color = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setShowInLinesplan(bool val)
{
    if (val != _show_in_linesplan) {
        _show_in_linesplan = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setUseInHydrostatics(bool val)
{
    if (val != _use_in_hydrostatics) {
        _use_in_hydrostatics = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setUseForIntersections(bool val)
{
    if (val != _use_for_intersections) {
        _use_for_intersections = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::setVisible(bool val)
{
    if (val != _visible) {
        _visible = val;
        emit changedLayerData(_layerid);
    }
}

void SubdivisionLayer::addControlFace(SubdivisionControlFace* face)
{
    // disconnect from current layer
    if (face->getLayer() != 0)
        face->getLayer()->deleteControlFace(face);
    if (find(_patches.begin(), _patches.end(), face) == _patches.end())
        _patches.push_back(face);
    face->setLayer(this);
}

void SubdivisionLayer::assignProperties(SubdivisionLayer* source)
{
    _color = source->_color;
    _visible = source->_visible;
    _desc = source->_desc;
    _symmetric = source->_symmetric;
    _developable = source->_developable;
    _material_density = source->_material_density;
    _thickness = source->_thickness;
}

bool SubdivisionLayer::calculateIntersectionPoints(SubdivisionLayer* layer)
{
    bool result = false;
    vector<SubdivisionControlEdge*> edges;
    vector<SubdivisionControlPoint*> newpoints;

    SubdivisionControlPoint* p1, *p2;
    SubdivisionControlEdge* edge;
    SubdivisionControlFace* face;

    // assemble all controledges in a list
    for (size_t i=1; i<=_patches.size(); ++i) {
        face = _patches[i-1];
        p1 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(face->numberOfPoints()-1));
        for (size_t j=1; j<=face->numberOfPoints(); ++j) {
            p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(j-1));
            edge = _owner->controlEdgeExists(p1, p2);
            if (edge != 0 && find(edges.begin(), edges.end(), edge) == edges.end())
                edges.push_back(edge);
            p1 = p2;
        }
    }

    // now check all edges for intersection with layer 2
    size_t i = 1;
    while (i <= edges.size()) {
        edge = edges[i-1];
        bool intfound = false;
        size_t j = 1;
        while (j <= layer->numberOfFaces() && !intfound) {
            face = layer->getFace(j-1);
            size_t k = 1;
            while (k <= face->numberOfChildren() && !intfound) {
                SubdivisionFace* child = face->getChild(k-1);
                size_t l = 3;
                while (l <= child->numberOfPoints() && !intfound) {
                    Plane pln(child->getPoint(0)->getCoordinate(),
                              child->getPoint(l-2)->getCoordinate(),
                              child->getPoint(l-1)->getCoordinate());
                    float s1 = pln.a() * edge->startPoint()->getCoordinate().x()
                            + pln.b() * edge->startPoint()->getCoordinate().y()
                            + pln.c() * edge->startPoint()->getCoordinate().z()
                            + pln.d();
                    float s2 = pln.a() * edge->endPoint()->getCoordinate().x()
                            + pln.b() * edge->endPoint()->getCoordinate().y()
                            + pln.c() * edge->endPoint()->getCoordinate().z()
                            + pln.d();
                    if ((s1 < 0 && s2 > 0) || (s1 > 0 && s2 < 0)) {
                        // edge intersects the plane, does it lie in the triangle?
                        float t;
                        if (s1 == s2)
                            t = 0.5;
                        else
                            t = -s1 / (s2 - s1);
                        QVector3D p3d = edge->startPoint()->getCoordinate()
                                + t * (edge->endPoint()->getCoordinate()
                                       - edge->startPoint()->getCoordinate());
                        if (PointInTriangle(p3d, child->getPoint(0)->getCoordinate(),
                                            child->getPoint(l-2)->getCoordinate(),
                                            child->getPoint(l-1)->getCoordinate())) {
                            // yes, valid intersection
                            SubdivisionControlPoint* p = edge->insertControlPoint(p3d);
                            if (p != 0) {
                                intfound = true;
                                result = true;
                                newpoints.push_back(p);
                            }
                        }
                    }
                    l++;
                }
                k++;
            }
            j++;
        }
        i++;
    }
    if (newpoints.size() > 0) {
        // try to find multiple new points belonging to the same face and insert an edge
        size_t i = 1;
        edges.clear();
        bool inserted = false;
        while (i <= newpoints.size()) {
            p1 = newpoints[i-1];
            size_t j = 1;
            while (j <= p1->numberOfFaces()) {
                face = dynamic_cast<SubdivisionControlFace*>(p1->getFace(j-1));
                size_t k = 1;
                while (k <= face->numberOfPoints() && !inserted) {
                    p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(k-1));
                    if (p1 != p2 && find(newpoints.begin(), newpoints.end(), p2) != newpoints.end()) {
                        // this is also a new point, first check if an edge already exists between p1 and p2
                        if (_owner->edgeExists(p1, p2) == 0) {
                            inserted = true;
                            edge = face->insertControlEdge(p1, p2);
                            edge->setSelected(true);
                            edges.push_back(edge);
                        }
                    }
                    k++;
                }
                if (!inserted)
                    j++;
            }
            i++;
        }
    }
    return result;
}

void SubdivisionLayer::clear()
{
    _layerid = 0;
    _patches.clear();
    _color = _owner->getLayerColor();
    _visible = true;
    _desc = "";
    _symmetric = true;
    _developable = false;
    _use_for_intersections = true;
    _use_in_hydrostatics = true;
    _show_in_linesplan = true;
    _material_density = 0;
    _thickness = 0;
    _alphablend = 255;
}

void SubdivisionLayer::deleteControlFace(SubdivisionControlFace* face)
{
    vector<SubdivisionControlFace*>::iterator i = find(_patches.begin(), _patches.end(), face);
    if (i != _patches.end())
        _patches.erase(i);
}

void SubdivisionLayer::drawLayers(Viewport& vp, SubdivisionSurface* surface)
{
    for (size_t i=0; i<surface->numberOfLayers(); ++i) {
        SubdivisionLayer* layer = surface->getLayer(i);
        if (layer->isVisible())
            layer->draw(vp);
    }
}

void SubdivisionLayer::draw(Viewport& vp)
{
    if (!isVisible() || numberOfFaces() == 0)
        return;
    if (vp.getViewportMode() != Viewport::vmWireFrame) {
        // not vmWireFrame, but shaded
        if (vp.getViewportMode() == Viewport::vmShadeGauss) {
            for (size_t i=0; i<numberOfFaces(); ++i)
                getFace(i)->drawCurvatureFaces(vp,
                                               _owner->getMinGausCurvature(),
                                               _owner->getMaxGausCurvature());
        }
        else {
            MonoFaceShader* shader = vp.setMonoFaceShader();
            for (size_t i=0; i<numberOfFaces(); ++i)
                getFace(i)->drawFaces(vp, shader);
        }
    }
    else {
        LineShader* lineshader = vp.setLineShader();
        // vmWireFrame
        if (_owner->showInteriorEdges()) {
            for (size_t i=0; i<numberOfFaces(); ++i)
                getFace(i)->draw(vp, lineshader);
        }
        // draw all interior crease-edges
        for (size_t i=0; i<numberOfFaces(); ++i) {
            SubdivisionControlFace* face = getFace(i);
            for (size_t j=0; j<face->numberOfControlEdges(); ++j) {
                SubdivisionEdge* edge = face->getControlEdge(j);
                if (edge->isCrease())
                    edge->draw(_owner->isDrawMirror() && isSymmetric(), vp, lineshader);
            }
        }
    }
}

void SubdivisionLayer::extents(QVector3D& min, QVector3D& max)
{
    if (isVisible()) {
        for (size_t i=1; i<=numberOfFaces(); ++i) {
            SubdivisionControlFace* face = _patches[i-1];
            MinMax(face->getMin(), min, max);
            MinMax(face->getMax(), min, max);
            if (isSymmetric() && _owner->isDrawMirror()) {
                QVector3D p = face->getMin();
                p.setY(-p.y());
                MinMax(p, min, max);
                p = face->getMax();
                p.setY(-p.y());
                MinMax(p, min, max);
            }
        }
    }
}

void SubdivisionLayer::loadBinary(FileBuffer& source)
{
    source.load(_desc);
    source.load(_layerid);
    if (_layerid > _owner->lastUsedLayerID())
        _owner->setLastUsedLayerID(_layerid);
    source.load(_color);
    source.load(_visible);
    source.load(_symmetric);
    source.load(_developable);
    _material_density = 0;
    _thickness = 0;
    _use_for_intersections = true;
    _use_in_hydrostatics = true;
    _show_in_linesplan = true;
    if (source.version() >= fv180) {
        source.load(_use_for_intersections);
        source.load(_use_in_hydrostatics);
        if (source.version() >= fv191) {
            source.load(_material_density);
            source.load(_thickness);
            if (source.version() >= fv201) {
                source.load(_show_in_linesplan);
                if (source.version() >= fv260) {
                    int i;
                    source.load(i);
                    _alphablend = i;
                }
            }
        }
    }
}

void SubdivisionLayer::moveDown()
{
    // BUGBUG: not implemented
}

void SubdivisionLayer::moveUp()
{
    // BUGBUG: not implemented
}

void SubdivisionLayer::saveToDXF(vector<QString>& strings)
{
    // BUGBUG: not implemented
}

void SubdivisionLayer::saveBinary(FileBuffer& destination)
{
    destination.add(_desc);
    destination.add(_layerid);
    destination.add(_color);
    destination.add(_visible);
    destination.add(_symmetric);
    destination.add(_developable);
    if (destination.version() >= fv180) {
        destination.add(_use_for_intersections);
        destination.add(_use_in_hydrostatics);
        if (destination.version() >= fv191) {
            destination.add(_material_density);
            destination.add(_thickness);
            if (destination.version() >= fv201) {
                destination.add(_show_in_linesplan);
                if (destination.version() >= fv260) {
                    destination.add(_alphablend);
                }
            }
        }
    }
}

void SubdivisionLayer::dump(ostream& os, const char* prefix) const
{
    os << prefix << "SubdivisionLayer ["
       << hex << this << "]\n";
    priv_dump(os, prefix);
}

void SubdivisionLayer::priv_dump(ostream& os, const char* prefix) const
{
    SubdivisionBase::priv_dump(os, prefix);
}

ostream& operator << (ostream& os, const ShipCADGeometry::SubdivisionLayer& layer)
{
    layer.dump(os);
    return os;
}
