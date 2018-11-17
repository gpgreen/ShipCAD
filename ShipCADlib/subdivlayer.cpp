/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                                *
 *    e-mail                  : Info@FREEship.org                                              *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                      *
 *    FREE!ship homepage      : www.FREEship.org                                               *
 *                                                                                             *
 *    This program is free software; you can redistribute it and/or modify it under            *
 *    the terms of the GNU General Public License as published by the                          *
 *    Free Software Foundation; either version 2 of the License, or (at your option)           *
 *    any later version.                                                                       *
 *                                                                                             *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY          *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 *
 *                                                                                             *
 *    You should have received a copy of the GNU General Public License along with             *
 *    this program; if not, write to the Free Software Foundation, Inc.,                       *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    *
 *                                                                                             *
 *#############################################################################################*/
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
#include "grid.h"
#include "developedpatch.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

SubdivisionLayer* SubdivisionLayer::construct(SubdivisionSurface* owner)
{
    void * mem = owner->getLayerPool().add();
    return new (mem) SubdivisionLayer(owner);
}

SubdivisionLayer::SubdivisionLayer(SubdivisionSurface* owner)
    : SubdivisionBase(owner), _layerid(0), _color(_owner->getLayerColor()),
	  _visible(true), _symmetric(true), _developable(false), _use_for_intersections(true),
	  _use_in_hydrostatics(true), _show_in_linesplan(true), _material_density(0),
	  _thickness(0), _alphablend(255)
{
    // does nothing
}

SubdivisionLayer::~SubdivisionLayer()
{
    if (_owner->getActiveLayer() == this)
        _owner->setActiveLayer(0);
    if (_owner->hasLayer(this))
        _owner->deleteLayer(this);
}

QString SubdivisionLayer::getName() const
{
    if (_desc == "") {
        return QString("%1_%2").arg(_owner->getDefaultLayerName()).arg(_layerid);
    }
    else
        return _desc;
}

SubdivisionControlFace* SubdivisionLayer::getFace(size_t index)
{
    if (index < _patches.size())
        return _patches[index];
    throw out_of_range("index out of range in SubdivisionLayer::getFace");
}

const SubdivisionControlFace* SubdivisionLayer::getFace(size_t index) const
{
    if (index < _patches.size())
        return _patches[index];
    throw out_of_range("index out of range in SubdivisionLayer::getFace");
}

void SubdivisionLayer::processTriangle(const QVector3D& p1, 
                                       const QVector3D& p2,
                                       const QVector3D& p3,
                                       LayerProperties& props) const
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

LayerProperties SubdivisionLayer::getSurfaceProperties() const
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

void SubdivisionLayer::setAlphaBlend(unsigned char val)
{
    if (val != _alphablend) {
        _alphablend = val;
    }
}

void SubdivisionLayer::setDevelopable(bool val)
{
    if (val != _developable) {
        _developable = val;
    }
}

void SubdivisionLayer::setName(const QString& val)
{
    if (QString::compare(val, _desc, Qt::CaseInsensitive) != 0) {
        _desc = val;
    }
}

void SubdivisionLayer::setDescription(const QString& val)
{
    if (QString::compare(val, _desc, Qt::CaseInsensitive) != 0) {
        _desc = val;
    }
}

void SubdivisionLayer::setSymmetric(bool val)
{
    if (val != _symmetric) {
        _symmetric = val;
    }
}

void SubdivisionLayer::setColor(QColor val)
{
    if (val != _color) {
        _color = val;
    }
}

void SubdivisionLayer::setThickness(float t)
{
    if (fabs(t - _thickness) > 1e-5) {
        _thickness = t;
    }
}

void SubdivisionLayer::setMaterialDensity(float d)
{
    if (fabs(d - _material_density) > 1e-5) {
        _material_density = d;
    }
}

void SubdivisionLayer::setShowInLinesplan(bool val)
{
    if (val != _show_in_linesplan) {
        _show_in_linesplan = val;
    }
}

void SubdivisionLayer::setUseInHydrostatics(bool val)
{
    if (val != _use_in_hydrostatics) {
        _use_in_hydrostatics = val;
    }
}

void SubdivisionLayer::setUseForIntersections(bool val)
{
    if (val != _use_for_intersections) {
        _use_for_intersections = val;
    }
}

void SubdivisionLayer::setVisible(bool val)
{
    if (val != _visible) {
        _visible = val;
    }
}

void SubdivisionLayer::useControlFace(SubdivisionControlFace* face)
{
    // disconnect from current layer
    if (face->getLayer() != 0 && face->getLayer() != this)
        face->getLayer()->releaseControlFace(face);
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

LayerPropertiesForDialog SubdivisionLayer::getProperties() const
{
    LayerPropertiesForDialog props;
    props.data = this;
    props.name = getName();
    props.color = _color;
    props.alpha = _alphablend / 255.0;
    props.hydrostatics = _use_in_hydrostatics;
    props.symmetric = _symmetric;
    props.intersection_curves = _use_for_intersections;
    props.developable = _developable;
    props.show_linesplan = _show_in_linesplan;
    props.material_density = _material_density;
    props.thickness = _thickness;
    props.layer_properties = getSurfaceProperties();
    return props;
}

bool SubdivisionLayer::setProperties(LayerPropertiesForDialog& props)
{
    bool changed = false;
    
    props.data = this;
    if (props.name != getName()) {
        changed = true;
        _desc = props.name;
    }
    if (props.color != _color) {
        changed = true;
        _color = props.color;
    }
    double alpha_int;
    modf(props.alpha * 255, &alpha_int);
    if (alpha_int >= 0 && alpha_int <= 255) {
        unsigned char alpha = static_cast<unsigned int>(alpha_int);
        if (alpha != _alphablend) {
            changed = true;
            _alphablend = alpha;
        }
    }
    if (props.hydrostatics != _use_in_hydrostatics) {
        changed = true;
        _use_in_hydrostatics = props.hydrostatics;
    }
    if (props.symmetric != _symmetric) {
        changed = true;
        _symmetric = props.symmetric;
    }
    if (props.intersection_curves != _use_for_intersections) {
        _use_for_intersections = props.intersection_curves;
        changed = true;
    }
    if (props.developable != _developable) {
        _developable = props.developable;
        changed = true;
    }
    if (props.show_linesplan != _show_in_linesplan) {
        _show_in_linesplan = props.show_linesplan;
        changed = true;
    }
    if (abs(props.material_density - _material_density) >= 1E-5) {
        _material_density = props.material_density;
        changed = true;
    }
    if (abs(props.thickness - _thickness) >= 1E-5) {
        _thickness = props.thickness;
        changed = true;
    }
    return changed;
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
restart:
        size_t i = 0;
        edges.clear();
        bool inserted = false;
        while (i < newpoints.size()) {
            p1 = newpoints[i];
            size_t j = 0;
            while (j < p1->numberOfFaces()) {
                face = dynamic_cast<SubdivisionControlFace*>(p1->getFace(j));
                size_t k = 0;
                while (k < face->numberOfPoints() && !inserted) {
                    p2 = dynamic_cast<SubdivisionControlPoint*>(face->getPoint(k));
                    if (p1 != p2 && find(newpoints.begin(), newpoints.end(), p2) != newpoints.end()) {
                        // this is also a new point, first check if an edge already exists between p1 and p2
                        if (_owner->edgeExists(p1, p2) == 0) {
                            inserted = true;
                            bool deleteface;
                            edge = face->insertControlEdge(p1, p2, deleteface);
                            edge->setSelected(true);
                            edges.push_back(edge);
                            // if face has been deleted, restart the loop
                            if (deleteface) {
                                getOwner()->deleteControlFace(face);
                                goto restart;
                            }
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

void SubdivisionLayer::releaseControlFace(SubdivisionControlFace* face)
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
    if (vp.getViewportMode() == vmShadeGauss) {
        CurveFaceShader* shader = vp.setCurveFaceShader();
        for (size_t i=0; i<numberOfFaces(); ++i)
            getFace(i)->drawCurvatureFaces(shader,
                                           _owner->getMinGausCurvature(),
                                           _owner->getMaxGausCurvature());
    }
    else if (vp.getViewportMode() == vmShadeDevelopable) {
        CurveFaceShader* shader = vp.setCurveFaceShader();
        for (size_t i=0; i<numberOfFaces(); i++)
            getFace(i)->drawDevelopableFaces(shader);
    }
    else if (vp.getViewportMode() == vmShadeZebra) {
        CurveFaceShader* shader = vp.setCurveFaceShader();
        for (size_t i=0; i<numberOfFaces(); i++)
            getFace(i)->drawZebraFaces(vp, shader);
    }
    else if (vp.getViewportMode() == vmShade) {
        FaceShader* shader = vp.setLightedFaceShader();
        for (size_t i=0; i<numberOfFaces(); ++i)
            getFace(i)->drawFaces(vp, shader);
    }
    else if (vp.getViewportMode() == vmWireFrame) {
        LineShader* lineshader = vp.setLineShader();
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
                    edge->draw(_owner->drawMirror() && isSymmetric(), vp, lineshader,
                               _owner->getCreaseColor());
            }
        }
    }
}

void SubdivisionLayer::extents(QVector3D& min, QVector3D& max)
{
    if (isVisible()) {
        for (size_t i=0; i<numberOfFaces(); ++i) {
            SubdivisionControlFace* face = _patches[i];
            MinMax(face->getMin(), min, max);
            MinMax(face->getMax(), min, max);
            if (isSymmetric() && _owner->drawMirror()) {
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

void SubdivisionLayer::unroll(PointerVector<DevelopedPatch>& destination)
{
    vector<SubdivisionControlFace*> todo(_patches.begin(), _patches.end());
    vector<vector<SubdivisionControlFace*> > done;

    while (todo.size() > 0) {
        SubdivisionControlFace* face = todo.back();
        todo.pop_back();
        vector<SubdivisionControlFace*> current;
        current.push_back(face);
        findAttachedFaces(current, face, todo);
        done.push_back(current);
    }
    // unroll each separate surface area
    for (size_t i=0; i<done.size(); i++) {
        vector<SubdivisionControlFace*>& current = done[i];
        if (current.size() > 0) {
            DevelopedPatch* patch = new DevelopedPatch(this);
            patch->unroll(current);
            QString str;
            if (done.size() == 1)
                str = getName();
            else
                str = QString("%1 %2 %3").arg(getName()).arg("Part").arg(i+1);
            patch->setName(str);
            destination.add(patch);
            if (!patch->isMirror() && isSymmetric()) {
                QString nm(str);
                nm += " (SB)";
                patch->setName(nm);
                DevelopedPatch* copy = new DevelopedPatch(this);
                QString nm1(str);
                nm1 += " (P)";
                copy->setName(nm1);
                destination.add(copy);
            }
        }
    }
}

// FreeGeometry.pas:9456
void SubdivisionLayer::findAttachedFaces(vector<SubdivisionControlFace*>& list,
                                         SubdivisionControlFace* face,
                                         vector<SubdivisionControlFace*>& todo)
{
    SubdivisionPoint* p1 = face->getPoint(face->numberOfPoints() - 1);
    for (size_t i=0; i<face->numberOfPoints(); i++) {
        SubdivisionPoint* p2 = face->getPoint(i);
        SubdivisionEdge* edge = face->getOwner()->edgeExists(p1, p2);
        if (edge != 0) {
            for (size_t j=0; j<edge->numberOfFaces(); j++) {
                if (edge->getFace(j) != face) {
                    vector<SubdivisionControlFace*>::iterator index = find(
                        todo.begin(), todo.end(), edge->getFace(j));
                    if (index != todo.end()) {
                        SubdivisionControlFace* cface =
                            dynamic_cast<SubdivisionControlFace*>(edge->getFace(j));
                        list.push_back(cface);
                        todo.erase(index);
                        findAttachedFaces(list, cface, todo);
                    }
                }
            }
        }
        p1 = p2;
    }
}

void SubdivisionLayer::loadBinary(FileBuffer& source)
{
    source.load(_desc);
    quint32 n;
    source.load(n);
    _layerid = n;
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
    if (source.getVersion() >= fv180) {
        source.load(_use_for_intersections);
        source.load(_use_in_hydrostatics);
        if (source.getVersion() >= fv191) {
            source.load(_material_density);
            source.load(_thickness);
            if (source.getVersion() >= fv201) {
                source.load(_show_in_linesplan);
                if (source.getVersion() >= fv260) {
                    quint32 i;
                    source.load(i);
                    _alphablend = i;
                }
            }
        }
    }
}

void SubdivisionLayer::loadFromStream(size_t &lineno, QStringList &strings)
{
    // read description
    QString str = strings[lineno++].trimmed();
    size_t start = 0;
    _desc = str;
    // read layer identification
    str = strings[lineno++].trimmed();
    start = 0;
    _layerid = ReadIntFromStr(lineno, str, start);
    if (_layerid > _owner->lastUsedLayerID())
        _owner->setLastUsedLayerID(_layerid);
    // read color
    int col = ReadIntFromStr(lineno, str, start);
    _color = QColorFromDXFIndex(col);
    // read visible
    _visible = ReadBoolFromStr(lineno, str, start);
    _symmetric = ReadBoolFromStr(lineno, str, start);
    // read developability
    if (start != to_size_t(str.length()))
        _developable = ReadBoolFromStr(lineno, str, start);
    else
        _developable = false;
    // read calc intersections flag
    if (start != to_size_t(str.length()))
        _use_for_intersections = ReadBoolFromStr(lineno, str, start);
    else
        _use_for_intersections = true;
    // read use in hydrostatics flag
    if (start != to_size_t(str.length()))
        _use_in_hydrostatics = ReadBoolFromStr(lineno, str, start);
    else
        _use_in_hydrostatics = true;
}

void SubdivisionLayer::saveToStream(QStringList &strings) const
{
    strings.push_back(_desc);
    strings.push_back(QString("%1 %2 %3 %4 %5 %6 %7")
                      .arg(_layerid)
                      .arg(FindDXFColorIndex(_color))
                      .arg(BoolToStr(_visible))
                      .arg(BoolToStr(_symmetric))
                      .arg(BoolToStr(_developable))
                      .arg(BoolToStr(_use_for_intersections))
                      .arg(BoolToStr(_use_in_hydrostatics)));
}

void SubdivisionLayer::saveBinary(FileBuffer& destination) const
{
    destination.add(_desc);
    destination.add(_layerid);
    destination.add(_color);
    destination.add(_visible);
    destination.add(_symmetric);
    destination.add(_developable);
    if (destination.getVersion() >= fv180) {
        destination.add(_use_for_intersections);
        destination.add(_use_in_hydrostatics);
        if (destination.getVersion() >= fv191) {
            destination.add(_material_density);
            destination.add(_thickness);
            if (destination.getVersion() >= fv201) {
                destination.add(_show_in_linesplan);
                if (destination.getVersion() >= fv260) {
                    quint32 n = _alphablend;
                    destination.add(n);
                }
            }
        }
    }
}

void SubdivisionLayer::saveToDXF(QStringList& strings)
{
    if (!isVisible())
        return;
    vector<Grid<SubdivisionControlFace*> > assembled;
    getOwner()->assembleFacesToPatches(amRegular, assembled);
    if (assembled.size() == 0)
        return;
    for (size_t i=0; i<assembled.size(); ++i) {
        Grid<SubdivisionControlFace*> assface = assembled[i];
        if ((assface.cols() > 1 && assface.rows() >= 1) ||
            (assface.cols() >= 1 && assface.rows() > 1) ||
            (assface.cols() == 1 && assface.rows() == 1 && assface.get(0,0)->numberOfPoints() == 4)) {
            Grid<SubdivisionPoint*> grid;
            getOwner()->convertToGrid(assface, grid);
            if (grid.cols() > 0 && grid.rows() > 0) {
                strings.push_back(QString("0\r\nPOLYLINE"));
                strings.push_back(QString("8\r\n%1").arg(getName()));
                strings.push_back(QString("62\r\n%1").arg(FindDXFColorIndex(getColor())));
                strings.push_back(QString("66\r\n1"));
                strings.push_back(QString("70\r\n16"));
                strings.push_back(QString("71\r\n%1").arg(grid.rows()));
                strings.push_back(QString("72\r\n%1").arg(grid.cols()));
                for (size_t j=0; j<grid.rows(); j++) {
                    for (size_t k=0; k<grid.cols(); k++) {
                        strings.push_back(QString("0\r\nVERTEX"));
                        strings.push_back(QString("8\r\n%1").arg(getName()));
                        QVector3D p = grid.get(j, k)->getCoordinate();
                        strings.push_back(QString("10\r\n%1").arg(Truncate(p.x(), 4)));
                        strings.push_back(QString("20\r\n%1").arg(Truncate(p.y(), 4)));
                        strings.push_back(QString("30\r\n%1").arg(Truncate(p.z(), 4)));
                        strings.push_back(QString("70\r\n64")); // polygon mesh vertex
                    }
                }
                strings.push_back(QString("0\r\nSEQEND"));
                if (isSymmetric() && getOwner()->drawMirror()) {
                    strings.push_back(QString("0\r\nPOLYLINE"));
                    strings.push_back(QString("8\r\n%1").arg(getName()));
                    strings.push_back(QString("62\r\n%1").arg(FindDXFColorIndex(getColor())));
                    strings.push_back(QString("66\r\n1"));
                    strings.push_back(QString("70\r\n16"));
                    strings.push_back(QString("71\r\n%1").arg(grid.cols()));
                    strings.push_back(QString("72\r\n%1").arg(grid.rows()));
                    for (size_t j=0; j<grid.rows(); j++) {
                        for (size_t k=0; k<grid.cols(); k++) {
                            strings.push_back(QString("0\r\nVERTEX"));
                            strings.push_back(QString("8\r\n%1").arg(getName()));
                            QVector3D p = grid.get(j, k)->getCoordinate();
                            strings.push_back(QString("10\r\n%1").arg(Truncate(p.x(), 4)));
                            strings.push_back(QString("20\r\n%1").arg(Truncate(-p.y(), 4)));
                            strings.push_back(QString("30\r\n%1").arg(Truncate(p.z(), 4)));
                            strings.push_back(QString("70\r\n64")); // polygon mesh vertex
                        }
                    }
                    strings.push_back(QString("0\r\nSEQEND"));
                }
            } else {
                for (size_t j=0; j<assface.rows(); j++) {
                    for (size_t k=0; k<assface.cols(); k++) {
                        SubdivisionControlFace* face = assface.get(j, k);
                        if (face != 0)
                            face->saveToDXF(strings);
                    }
                }
            }
        } else if (assface.rows() == 1 && assface.cols() == 1) {
            assface.get(0, 0)->saveToDXF(strings);
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

ostream& operator << (ostream& os, const ShipCAD::SubdivisionLayer& layer)
{
    layer.dump(os);
    return os;
}
