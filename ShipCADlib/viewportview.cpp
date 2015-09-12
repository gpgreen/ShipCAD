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

using namespace std;
using namespace ShipCAD;

ViewportView::ViewportView(Viewport* vp)
    : _vp(vp), _zoom(1.0), _panX(0.0), _panY(0.0),
      _scale(1.0), _margin(5)
{
    // does nothing
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

ViewportViewPerspective::ViewportViewPerspective(Viewport* vp)
    : ViewportView(vp), _camera(ftStandard), _field_of_view(atan(35.0/50.0)),
      _angle(20), _elevation(20), _distance(0)
{
    // does nothing
}

void ViewportViewPerspective::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);
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

    float aspect_ratio = width / static_cast<float>(height);
    float hi, margin;
    _proj = QMatrix4x4();

    QMatrix4x4 model;

    // create projection
    _proj.perspective(RadToDeg(_field_of_view), aspect_ratio, 0.1f, 100.0f);
    
    // find the camera location
    model.rotate(_elevation, 1, 0, 0);
    model.rotate(_angle, 0, 0, 1);
    _camera_location = model.map(QVector3D(max.x() + _distance, 0, 0));
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, _midpoint, QVector3D(0,0,1));
    
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
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera


    float aspect_ratio = width / static_cast<float>(height);
    float hi, margin;

    _proj = QMatrix4x4();

    // find view extents using aspect ratio
    if (_midpoint.x() / aspect_ratio >= _midpoint.y() * 2) {
        margin = _midpoint.x() * .01 * _margin;
        hi = _midpoint.x() / aspect_ratio + margin;
        _proj.ortho(-_midpoint.x() - margin, _midpoint.x() + margin, -hi, hi, 0.1f, 100.0f);
    }
    else {
        margin = _midpoint.y() * 2 * .01 * _margin;
        hi = _midpoint.y() * 2 * aspect_ratio + margin;
        _proj.ortho(-hi, hi, -_midpoint.y() * 2 - margin, _midpoint.y() * 2 + margin, 0.1f, 100.0f);
    }
    
    // find the camera location
    _camera_location = QVector3D(_midpoint.x(), _midpoint.y(), max.z() + 40);
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(_midpoint.x(), _midpoint.y(), 0), QVector3D(0,1,0));
    
    _view = view;

    finishSetup();
}

ViewportViewProfile::ViewportViewProfile(Viewport* vp)
    : ViewportView(vp)
{
    // does nothing
}

void ViewportViewProfile::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera


    float aspect_ratio = width / static_cast<float>(height);
    float hi, margin;

    _proj = QMatrix4x4();

    if (_midpoint.x() / aspect_ratio >= _midpoint.z()) {
        margin = _midpoint.x() * .01 * _margin;
        hi = _midpoint.x() / aspect_ratio + margin;
        _proj.ortho(-_midpoint.x() - margin, _midpoint.x() + margin, -hi, hi, 0.1f, 100.0f);
    }
    else {
        margin = _midpoint.z() * .01 * _margin;
        hi = _midpoint.z() + margin;
        _proj.ortho(-hi, hi, -_midpoint.z() - margin, _midpoint.z() + margin, 0.1f, 100.0f);
    }
    
    // find the camera location
    _camera_location = QVector3D(_midpoint.x(), min.y() - 40, _midpoint.z());

    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(_midpoint.x(), 0, _midpoint.z()), QVector3D(0,0,1));
    
    _view = view;

    finishSetup();
}

ViewportViewBodyplan::ViewportViewBodyplan(Viewport* vp)
    : ViewportView(vp)
{
    // does nothing
}

void ViewportViewBodyplan::initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height)
{
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (min + max);

    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera


    float aspect_ratio = width / static_cast<float>(height);
    float hi, margin;

    _proj = QMatrix4x4();

    if (_midpoint.y() * 2 / aspect_ratio >= _midpoint.z()) {
        margin = _midpoint.y() * 2 * .01 * _margin;
        hi = _midpoint.y() * 2 / aspect_ratio + margin;
        _proj.ortho(-_midpoint.y() * 2 - margin, _midpoint.y() * 2 + margin, -hi, hi, 0.1f, 100.0f);
    }
    else {
        margin = _midpoint.z() * .01 * _margin;
        hi = _midpoint.z() * aspect_ratio + margin;
        _proj.ortho(-hi, hi, -_midpoint.z() - margin, _midpoint.z() + margin, 0.1f, 100.0f);
    }
    
    // find the camera location
    _camera_location = QVector3D(max.x() + 40, 0, _midpoint.z());
    
    // view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, QVector3D(0, 0, _midpoint.z()), QVector3D(0,0,1));
    
    _view = view;

    finishSetup();
}


