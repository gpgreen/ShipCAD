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

#include "viewportview.h"
#include "viewport.h"
#include "utility.h"
#include "subdivsurface.h"
#include "controller.h"

using namespace std;
using namespace ShipCAD;

ViewportView::ViewportView(Viewport* vp)
    : _vp(vp), _zoom(1.0), _panX(0.0), _panY(0.0),
      _scale(1.0), _margin(5)
{
    // does nothing
}

ViewportView* ViewportView::construct(viewport_type_t ty, Viewport* vp)
{
    if (ty == fvPerspective)
        return new ViewportViewPerspective(vp);
    if (ty == fvProfile)
        return new ViewportViewProfile(vp);
    if (ty == fvPlan)
        return new ViewportViewPlan(vp);
    if (ty == fvBodyplan)
        return new ViewportViewBodyplan(vp);
    // should never be reached
    return 0;
}

void ViewportView::finishSetup()
{
    // final matrix
    _world = _proj * _view;

    // inverted matrix
    bool invertable;
    _worldInv = _world.inverted(&invertable);
    if (!invertable)
        throw runtime_error("world matrix not invertable");
}

void ViewportView::resetView()
{
    _zoom = 1.0;
    _panX = 0;
    _panY = 0;
    _scale = 1.0;
}

QPoint ViewportView::convert3D(const QVector3D& pt, int w, int h) const
{
    QVector3D result = _world * pt;
    return QPoint(w / 2.0f * (result.x() + 1.0f), h / 2.0f * (1.0f - result.y()));
}

bool ViewportView::leftMousePick(QPoint pos, int w, int h, PickRay& ray)
{
    bool scene_changed = false;
    PickRay tray = convertMouseCoordToWorld(pos, w, h);
    ray.dir = tray.dir;
    ray.pt = tray.pt;
    scene_changed = _vp->shootPickRay(ray);
    return scene_changed;
}

bool ViewportView::rightMousePick(QPoint /*pos*/, int /*w*/, int /*h*/)
{
    return false;
}

bool ViewportView::leftMouseRelease(QPoint pos, int w, int h)
{
    convertMouseCoordToWorld(pos, w, h);
    return false;
}

bool ViewportView::rightMouseRelease(QPoint pos, int w, int h)
{
    convertMouseCoordToWorld(pos, w, h);
    return false;
}

bool ViewportView::middleMouseMove(QPoint /*cur*/, QPoint /*prev*/, int /*w*/, int /*h*/)
{
    return false;
}

bool ViewportView::leftMouseMove(QPoint /*cur*/, QPoint /*prev*/, int /*w*/, int /*h*/)
{
    return false;
}

bool ViewportView::rightMouseMove(QPoint /*cur*/, QPoint /*prev*/, int /*w*/, int /*h*/)
{
    return false;
}

bool ViewportView::wheelWithDegrees(QPoint degrees, int /*w*/, int /*h*/)
{
    bool zoomed = false;
    
    if (degrees.y() != 0) {
        // zoom is in units of 15 degrees
        _zoom += (degrees.y() / 150.0);
        cout << "zoom:" << _zoom << endl;
        zoomed = true;
    }
    
    return zoomed;
}

bool ViewportView::pointDrag(QPoint /*pos*/, int /*w*/, int /*h*/, QVector3D& /*newcoord*/)
{
    return false;
}

ShipCAD::PickRay ViewportView::convertMouseCoordToWorld(QPoint pos, int w, int h) const
{
    float x = (2.0f * pos.x()) / w - 1.0f;
    float y = 1.0f - (2.0f * pos.y()) / h;

    QVector4D from = _worldInv * QVector4D(x, y, -1.0, 1.0);
    QVector4D to = _worldInv * QVector4D(x, y, 1.0, 1.0);

    from /= from.w();
    to /= to.w();

    PickRay ray(false, false, false, false);
    ray.pt = from.toVector3D();
    ray.dir = to.toVector3D() - from.toVector3D();
    ray.dir.normalize();

    return ray;
}

ViewportViewPerspective::ViewportViewPerspective(Viewport* vp)
    : ViewportView(vp), _panZ(0.0), _camera(ftStandard), _field_of_view(atan(35.0/50.0)),
      _angle(-30), _elevation(30), _distance(0)
{
    // does nothing
}

void ViewportViewPerspective::resetView()
{
    ViewportView::resetView();
    _panZ = 0;
    _angle = -30.0;
    _elevation = 30.0;
}

QVector2D ViewportViewPerspective::projectTo3D(QPoint pos, int w, int h)
{
    Q_UNUSED(pos);
    Q_UNUSED(w);
    Q_UNUSED(h);
    return QVector2D(0,0);
}

bool ViewportViewPerspective::middleMouseMove(QPoint cur, QPoint prev, int /*w*/, int /*h*/)
{
    QPoint rel = cur - prev;
    // dragging the perspective around with middle button
    _angle -= rel.x() / 2.0f;
    _elevation += rel.y() / 2.0f;
    while (_elevation > 90) {
        _elevation = 90 - _elevation;
        _angle += 180;
    }
    while (_elevation < -90) {
        _elevation += 180;
        _angle += 180;
    }
    while (_angle > 180)
        _angle -= 360;
    while (_angle < -180)
        _angle += 360;
    cout << "angle:" << _angle << " elev:" << _elevation << endl;
    return true;
}

bool ViewportViewPerspective::leftMouseMove(QPoint cur, QPoint prev, int w, int h)
{
    PickRay pr1 = convertMouseCoordToWorld(cur, w, h);
    PickRay pr2 = convertMouseCoordToWorld(prev, w, h);

    // make plane at midpoint, normal to pick ray
    Plane n(_midpoint, pr1.dir);

    // intersect pick rays and this plane
    QVector3D chg = n.projectPointOnPlane(pr1.pt) - n.projectPointOnPlane(pr2.pt);
    
    _panX -= chg.x()*100;
    _panY -= chg.y()*100;
    _panZ -= chg.z()*100;

    cout << "perspective pan:" << _panX << "," << _panY << "," << _panZ << endl;
    
    return true;
}

void ViewportViewPerspective::initializeViewport(const QVector3D& surfmin, const QVector3D& surfmax, int width, int height)
{
    // add margin to max/min
    QVector3D diff = 0.01 * _margin * (surfmax - surfmin);
    QVector3D min = surfmin - diff;
    QVector3D max = surfmax + diff;

    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    QVector3D panpoint = _midpoint;
    panpoint.setX(panpoint.x() + _panX);
    panpoint.setY(panpoint.y() + _panY);
    panpoint.setZ(panpoint.z() + _panZ);

    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera
    float dist = sqrt((max.y() - min.y()) * (max.y() - min.y())
                      + (max.z() - min.z()) * (max.z() - min.z()));
    if (dist == 0)
        dist = 1E-2f;
    if (atan(_field_of_view) != 0) {
        _distance = 1.5 * dist / atan(_field_of_view);
        if (_distance > 1E5)
            _distance = 1E5;
    }
    else
        _distance = 1E5;

    // build the vertex transformation matrix from the perspective
    // and the angle, elevation

    // find the camera location
    QMatrix4x4 model;
    model.translate(_panX, _panY, _panZ);
    model.rotate(-_elevation, 1, 0, 0);
    model.rotate(_angle, 0, 0, 1);
    _camera_location = model.map(QVector3D(max.x() + _distance, 0, 0));

    // find the max point and min point distance from the camera
    // use these plus some buffer for near and far clipping planes
    float maxdist = (model.map(surfmax) - _camera_location).length();
    float mindist = (model.map(surfmin) - _camera_location).length();
    float near = 0;
    if (abs(maxdist) > abs(mindist))
        near = abs(mindist) * 0.80;
    else
        near = abs(maxdist) * 0.80;
    float far = 0;
    if (abs(maxdist) > abs(mindist))
        far = abs(maxdist) * 1.20;
    else
        far = abs(mindist) * 1.20;

    float aspect_ratio = 1.0;
    if (height != 0)
        aspect_ratio = width / static_cast<float>(height);
    _proj = QMatrix4x4();

    // create projection
    _proj.perspective(RadToDeg(_field_of_view) / _zoom, aspect_ratio, near, far);
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, panpoint, QVector3D(0,0,1));
    
    _view = view;

    finishSetup();
}

void ViewportViewPerspective::setCameraType(camera_type_t val)
{
    if (val != _camera) {
        float film = 35.0;
        float dist;
        if (val == ftWide)
            dist = 20;
        else if (val == ftStandard)
            dist = 50;
        else if (val == ftShortTele)
            dist = 90;
        else if (val == ftMediumTele)
            dist = 130;
        else if (val == ftFarTele)
            dist = 200;
        else
            dist = 50;
        _camera = val;
        _field_of_view = atan(film/dist);
    }
}

ViewportViewPlan::ViewportViewPlan(Viewport* vp)
    : ViewportView(vp)
{
    // does nothing
}

void ViewportViewPlan::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    //float breadth = max.y() - min.y();
    float hull_width = max.x() - min.x();

    float aspect_ratio = 1.0;
    if (width != 0 && height != 0)
        aspect_ratio = width / static_cast<float>(height);

    _proj = QMatrix4x4();

    // find view extents using aspect ratio
    float margin = hull_width / 2.0 * .01 * _margin;
    float hw = (hull_width / 2.0 + margin) / _zoom;
    float hh = hw / aspect_ratio;
    _proj.ortho(-hw, hw, -hh, hh, 0.1f, 1000.0f);

    // calculate the midpoint of the bounding box
    _midpoint = 0.5 * (min + max);

    // calculate the extents of the view
    _min = min - QVector3D(margin, margin, 0);
    _max = max + QVector3D(margin, margin, 0);

    QVector3D panpoint(_midpoint);
    panpoint.setX(panpoint.x() + _panX);
    panpoint.setY(panpoint.y() + _panY);

    // find the camera location
    _camera_location = QVector3D(panpoint.x(), panpoint.y(), max.z() + 20);
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(panpoint.x(), panpoint.y(), 0), QVector3D(0,1,0));
    
    _view = view;

    finishSetup();
}

QVector2D ViewportViewPlan::projectTo3D(QPoint pos, int w, int h)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    return QVector2D(pr.pt.x(),pr.pt.y());
}

bool ViewportViewPlan::rightMouseMove(QPoint cur, QPoint prev, int w, int h)
{
    PickRay pr1 = convertMouseCoordToWorld(cur, w, h);
    PickRay pr2 = convertMouseCoordToWorld(prev, w, h);

    QVector3D chg = pr1.pt - pr2.pt;
    
    _panX -= chg.x();
    _panY -= chg.y();

    cout << "plan pan:" << _panX << "," << _panY << endl;
    
    return true;
}

bool ViewportViewPlan::pointDrag(QPoint pos, int w, int h, QVector3D& newcoord)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    newcoord.setX(pr.pt.x());
    newcoord.setY(pr.pt.y());
    newcoord.setZ(0.0);
    cout << "plan drag:(" << newcoord.x() << "," << newcoord.y() << "," << newcoord.z() << ")" << endl;
    return true;
}

ViewportViewProfile::ViewportViewProfile(Viewport* vp)
    : ViewportView(vp)
{
    // does nothing
}

void ViewportViewProfile::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    //float hull_height = max.z() - min.z();
    float hull_width = max.x() - min.x();
    
    float aspect_ratio = 1.0;
    if (width != 0 && height != 0)
        aspect_ratio = width / static_cast<float>(height);

    _proj = QMatrix4x4();

    float margin = hull_width / 2.0 * .01 * _margin;
    float hw = (hull_width / 2.0 + margin) / _zoom;
    float hh = hw / aspect_ratio;
    _proj.ortho(-hw, hw, -hh, hh, 0.1f, 1000.0f);
    
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    // calculate the extents of the view
    _min = min - QVector3D(margin, 0, margin);
    _max = max + QVector3D(margin, 0, margin);

    QVector3D panpoint = _midpoint;
    panpoint.setX(panpoint.x() + _panX);
    panpoint.setZ(panpoint.z() + _panY);

    // find the camera location
    _camera_location = QVector3D(panpoint.x(), min.y() - 20, panpoint.z());

    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(panpoint.x(), 0, panpoint.z()), QVector3D(0,0,1));
    
    _view = view;

    finishSetup();
}

QVector2D ViewportViewProfile::projectTo3D(QPoint pos, int w, int h)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    return QVector2D(pr.pt.x(),pr.pt.z());
}

bool ViewportViewProfile::rightMouseMove(QPoint cur, QPoint prev, int w, int h)
{
    PickRay pr1 = convertMouseCoordToWorld(cur, w, h);
    PickRay pr2 = convertMouseCoordToWorld(prev, w, h);

    QVector3D chg = pr1.pt - pr2.pt;
    
    _panX -= chg.x();
    _panY -= chg.z();

    cout << "profile pan:" << _panX << "," << _panY << endl;
    
    return true;
}

bool ViewportViewProfile::pointDrag(QPoint pos, int w, int h, QVector3D& newcoord)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    newcoord.setX(pr.pt.x());
    newcoord.setY(0.0);
    newcoord.setZ(pr.pt.z());
    cout << "profile drag:(" << newcoord.x() << "," << newcoord.y() << "," << newcoord.z() << ")" << endl;
    return true;
}

ViewportViewBodyplan::ViewportViewBodyplan(Viewport* vp)
    : ViewportView(vp)
{
    // does nothing
}

void ViewportViewBodyplan::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    float breadth = max.y() - min.y();
    float hull_height = max.z() - min.z();
    if (min.y() == 0)
        breadth *= 2.0;

    float aspect_ratio = 1.0;
    if (height != 0)
        aspect_ratio = width / static_cast<float>(height);

    _proj = QMatrix4x4();

    float margin, hb, hh;
    if (breadth / aspect_ratio >= hull_height) {
        margin = breadth * .01 * _margin;
        hb = (breadth / 2.0 + margin) / _zoom;
        hh = hb / aspect_ratio;
    }
    else {
        margin = hull_height * .01 * _margin;
        hh = (hull_height / 2.0 + margin) / _zoom;
        hb = hh * aspect_ratio;
    }
    _proj.ortho(-hb, hb, -hh, hh, 0.1f, 1000.0f);
    
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    // calculate the extents of the view
    _min = min - QVector3D(0, margin, margin);
    _max = max + QVector3D(0, margin, margin);

    // find the camera location
    _camera_location = QVector3D(max.x() + 20, _panX, _midpoint.z() + _panY);
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(0, _panX, _midpoint.z() + _panY), QVector3D(0,0,1));
    
    _view = view;

    finishSetup();
}

QVector2D ViewportViewBodyplan::projectTo3D(QPoint pos, int w, int h)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    return QVector2D(pr.pt.y(),pr.pt.z());
}

bool ViewportViewBodyplan::rightMouseMove(QPoint cur, QPoint prev, int w, int h)
{
    PickRay pr1 = convertMouseCoordToWorld(cur, w, h);
    PickRay pr2 = convertMouseCoordToWorld(prev, w, h);

    QVector3D chg = pr1.pt - pr2.pt;
    
    _panX -= chg.y();
    _panY -= chg.z();

    cout << "bodyplan pan:" << _panX << "," << _panY << endl;
    
    return true;
}

bool ViewportViewBodyplan::pointDrag(QPoint pos, int w, int h, QVector3D& newcoord)
{
    PickRay pr = convertMouseCoordToWorld(pos, w, h);
    cout << "bodyplan drag:(" << pr.pt.y() << "," << pr.pt.z() << ")" << endl;
    newcoord.setX(0.0);
    newcoord.setY(-pr.pt.y());
    newcoord.setZ(pr.pt.z());
    cout << "bodyplan drag:(" << newcoord.x() << "," << newcoord.y() << "," << newcoord.z() << ")" << endl;
    return true;
}
