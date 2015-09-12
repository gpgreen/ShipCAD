/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#include "viewport.h"
#include "shader.h"
#include "entity.h"
#include "subdivsurface.h"
#include "utility.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport()
    : _mode(vmWireFrame), _view_type(fvPerspective),
      _camera(ftStandard),
      _field_of_view(atan(35.0/50.0)),
      _angle(20), _elevation(20),
      _zoom(1.0), _panX(0), _panY(0),
      _distance(0), _scale(1.0), _margin(5),
      _current_shader(0), _surface(0)
{
    // does nothing else
}

Viewport::~Viewport()
{
    map<string, Shader*>::iterator i = _shaders.begin();
    while (i != _shaders.end()) {
        delete (*i).second;
        ++i;
    }
}

void Viewport::initialize()
{
    LineShader* lineshader = new LineShader(this);
    addShader("lineshader", lineshader);

    MonoFaceShader* monofaceshader = new MonoFaceShader(this);
    addShader("monofaceshader", monofaceshader);
}

void Viewport::setViewportMode(viewport_mode_t mode)
{
    _mode = mode;
    renderLater();
}

void Viewport::setCameraType(camera_type_t val)
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
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setViewportType(viewport_type_t ty)
{
    if (ty != _view_type) {
        _view_type = ty;
        _zoom = 1.0;
        _panX = 0;
        _panY = 0;
        switch (ty) {
        case fvBodyplan:
            _angle = 0;
            _elevation = 0;
            break;
        case fvProfile:
            _angle = 90;
            _elevation = 0;
            break;
        case fvPlan:
            _angle = 90;
            _elevation = 90;
            break;
        case fvPerspective:
            _angle = 20;
            _elevation = 20;
            break;
        }
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setAngle(float val)
{
    if (val != _angle) {
        _angle = val;
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setElevation(float val)
{
    if (val != _elevation) {
        _elevation = val;
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::resizeEvent(QResizeEvent *event)
{
    cout << "vp resize event: " << event->size().width()
         << "," << event->size().height() << endl;
    initializeViewport(_min3d, _max3d);
}

// this procedure initializes the viewport and sets the scale in such a way
// that the model completely fills the viewport
void Viewport::initializeViewport(const QVector3D& min, const QVector3D& max)
{
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (_min3d + _max3d);
    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera
    float dist = sqrt((_max3d.y() - _min3d.y()) * (_max3d.y() - _min3d.y())
                      + (_max3d.z() - _min3d.z()) * (_max3d.z() - _min3d.z()));
    if (dist == 0)
        dist = 1E-2f;
    if (_view_type == fvPerspective) {
        if (atan(_field_of_view) != 0) {
            _distance = 1.5 * dist / atan(_field_of_view);
            if (_distance > 1E5)
                _distance = 1E5;
        }
        else
            _distance = 1E5;
    }
    else
        _distance = 1E8;

    // build the vertex transformation matrix from the perspective
    // and the angle, elevation

    float aspect_ratio = width() / static_cast<float>(height());
    float hi;
    _proj = QMatrix4x4();
    QMatrix4x4 model;
    QMatrix4x4 view;
    switch (_view_type) {
    case fvPerspective:
        _proj.perspective(RadToDeg(_field_of_view), aspect_ratio, 0.1f, 100.0f);

        // find the camera location
        model.rotate(_elevation, 1, 0, 0);
        model.rotate(_angle, 0, 0, 1);
        _camera_location = model.map(QVector3D(_max3d.x() + _distance, 0, 0));

        // view matrix
        view.lookAt(_camera_location, _midpoint, QVector3D(0,0,1));
        break;
    case fvProfile:
        hi = _midpoint.x() / aspect_ratio;
        _proj.ortho(-_midpoint.x(), _midpoint.x(), -hi, hi, 0.1f, 100.0f);

        // find the camera location
        _camera_location = QVector3D(_midpoint.x(), _min3d.y() - 20, _midpoint.z());

        // view matrix
        view.lookAt(_camera_location, QVector3D(_midpoint.x(), 0, _midpoint.z()), QVector3D(0,0,1));
        break;
    case fvPlan:
        hi = _midpoint.x() / aspect_ratio;
        _proj.ortho(-_midpoint.x(), _midpoint.x(), -hi, hi, 0.1f, 100.0f);

        // find the camera location
        _camera_location = QVector3D(_midpoint.x(), _midpoint.y(), _max3d.z() + 20);
        
        // view matrix
        view.lookAt(_camera_location, QVector3D(_midpoint.x(), _midpoint.y(), 0), QVector3D(0,1,0));
        break;
    case fvBodyplan:
        hi = _midpoint.z() * aspect_ratio;
        _proj.ortho(-hi, hi, -_midpoint.z(), _midpoint.z(), 0.1f, 100.0f);

        // find the camera location
        _camera_location = QVector3D(_max3d.x() + 20, 0, _midpoint.z());

        // perspective view matrix
        view.lookAt(_camera_location, QVector3D(0, 0, _midpoint.z()), QVector3D(0,0,1));
        break;
    }
    
    _view = view;

    // final matrix
    _world = _proj * _view;

    // inverted matrix
    bool invertable;
    _worldInv = _world.inverted(&invertable);
    if (!invertable)
        throw runtime_error("world matrix not invertable");

    renderLater();
}

void Viewport::add(Entity* entity)
{
    _entities.push_back(entity);
}

void Viewport::setSurface(SubdivisionSurface* surface)
{
    _surface = surface;
    if (_surface == 0)
        return;
    _surface->extents(_min3d, _max3d);

    // add margin to max/min
    QVector3D diff = 0.01 * _margin * (_max3d - _min3d);
    _min3d -= diff;
    _max3d += diff;

    initializeViewport(_min3d, _max3d);
}

void Viewport::addShader(const string &name, Shader *shader)
{
    _shaders[name] = shader;
}

void Viewport::render()
{
    glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    LineShader* lineshader = setLineShader();

    for (size_t i=0; i<_entities.size(); ++i)
        _entities[i]->draw(*this, lineshader);

    _surface->draw(*this);

    // need to release the shader, otherwise doesn't draw right
    if (_current_shader != 0) {
        _current_shader->release();
        _current_shader = 0;
    }
}

LineShader* Viewport::setLineShader()
{
    Shader* shader = _shaders["lineshader"];
    if (shader == _current_shader)
        return dynamic_cast<LineShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_world);
    _current_shader = shader;
    //cerr << "set line shader\n";
    return dynamic_cast<LineShader*>(_current_shader);
}

MonoFaceShader* Viewport::setMonoFaceShader()
{
    Shader* shader = _shaders["monofaceshader"];
    if (shader == _current_shader)
        return dynamic_cast<MonoFaceShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_world);
    _current_shader = shader;
    //cerr << "set mono face shader\n";
    return dynamic_cast<MonoFaceShader*>(_current_shader);
}

void
Viewport::convertMouseCoordToWorld(int mx, int my)
{
    float x = (2.0f * mx) / width() - 1.0f;
    float y = 1.0f - (2.0f * my) / height();

    QVector4D from = _worldInv * QVector4D(x, y, -1.0, 1.0);
    QVector4D to = _worldInv * QVector4D(x, y, 1.0, 1.0);

    from /= from.w();
    to /= to.w();
    
    cout << "from:" << from.x() << "," << from.y() << "," << from.z() << endl;
    cout << "to:" << to.x() << "," << to.y() << "," << to.z() << endl;

    // find the intersection with the xz plane if possible
    Plane xz(0,1,0,0);
    bool coplanar;
    QVector3D dir = to.toVector3D() - from.toVector3D();
    QVector3D intpt;
    if (!xz.intersectLine(from.toVector3D(), dir, coplanar, intpt))
        cout << "xz plane intersect:" << intpt.x() << "," << intpt.y() << "," << intpt.z() << endl;
    else
        cout << "parallel to xz plane" << endl;
    // find the intersection with the yz plane if possible
    Plane yz(1,0,0,0);
    if (!yz.intersectLine(from.toVector3D(), dir, coplanar, intpt))
        cout << "yz plane intersect:" << intpt.x() << "," << intpt.y() << "," << intpt.z() << endl;
    else
        cout << "parallel to yz plane" << endl;
}

void
Viewport::mousePressEvent(QMouseEvent *event)
{
    _prev_pos = event->pos();
    _prev_buttons = event->buttons();
    // are we getting mouse clicks in the gl window?
    cout << "mouse press: " << event->pos().x() << "," << event->pos().y() << endl;
}

void
Viewport::mouseReleaseEvent(QMouseEvent *event)
{
    // for mouse release, the button released won't be in the set
    if (_prev_buttons.testFlag(Qt::LeftButton) && !event->buttons().testFlag(Qt::LeftButton)) {
        convertMouseCoordToWorld(event->pos().x(), event->pos().y());
    }
    _prev_buttons = event->buttons();
    // are we getting mouse clicks in the gl window?
    cout << "mouse release: " << event->pos().x() << "," << event->pos().y() << endl;
}

void
Viewport::mouseMoveEvent(QMouseEvent *event)
{
    if (_view_type == fvPerspective && event->buttons().testFlag(Qt::MidButton)) {
        // dragging the perspective around with middle button
        _angle -= (event->pos().x() - _prev_pos.x()) / 2.0f;
        while (_angle > 180)
            _angle -= 360;
        while (_angle < -180)
            _angle += 360;
        _elevation += -(event->pos().y() - _prev_pos.y()) / 2.0f;
        while (_elevation > 180)
            _elevation -= 360;
        while (_elevation < -180)
            _elevation += 360;
        initializeViewport(_min3d, _max3d);
    }
    _prev_pos = event->pos();
    // are we getting mouse clicks in the gl window?
    //cout << "mouse move: " << event->pos().x() << "," << event->pos().y() << endl;
}

