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
#include <QApplication>

#include "viewport.h"
#include "viewportview.h"
#include "shader.h"
#include "controller.h"
#include "entity.h"
#include "subdivsurface.h"
#include "utility.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

ViewportContextEvent::ViewportContextEvent(Viewport* vp, QMouseEvent* event)
    : _vp(vp), _event(event)
{
    // does nothing
}

//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport(Controller* ctl, viewport_type_t vtype)
    : _ctl(ctl), _mode(vmWireFrame), _view_type(vtype),
      _view(0), _drag_state(0), _current_shader(0)
{
    _view = ViewportView::construct(vtype, this);
}

Viewport::~Viewport()
{
    delete _view;
    
    map<string, Shader*>::iterator i = _shaders.begin();
    while (i != _shaders.end()) {
        delete (*i).second;
        ++i;
    }
}

Controller* Viewport::getController()
{
    return _ctl;
}

void Viewport::initialize()
{
    LineShader* lineshader = new LineShader(this);
    addShader("lineshader", lineshader);

    MonoFaceShader* monofaceshader = new MonoFaceShader(this);
    addShader("monofaceshader", monofaceshader);

    LightedFaceShader* lightedfaceshader = new LightedFaceShader(this);
    addShader("lightedfaceshader", lightedfaceshader);
}

void Viewport::setViewportMode(viewport_mode_t mode)
{
    if (mode != _mode) {
        _mode = mode;
        renderLater();
    }
}

camera_type_t Viewport::getCameraType() const
{
    if (_view_type != fvPerspective)
        return ftStandard;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    return view->getCameraType();
}

void Viewport::setCameraType(camera_type_t val)
{
    if (_view_type != fvPerspective)
        return;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    view->setCameraType(val);
    update();
}

void Viewport::setViewportType(viewport_type_t ty)
{
    if (ty != _view_type) {
        delete _view;
        _view = ViewportView::construct(ty, this);
        _view_type = ty;
        update();
    }
}

float Viewport::getAngle() const
{
    if (_view_type != fvPerspective)
        return 0.0;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    return view->getAngle();
}

void Viewport::setAngle(float val)
{
    if (_view_type != fvPerspective)
        return;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    view->setAngle(val);
    update();
}

float Viewport::getElevation() const
{
    if (_view_type != fvPerspective)
        return 0.0;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    return view->getElevation();
}

void Viewport::setElevation(float val)
{
    if (_view_type != fvPerspective)
        return;
    ViewportViewPerspective* view = dynamic_cast<ViewportViewPerspective*>(_view);
    view->setElevation(val);
    update();
}

void Viewport::resizeEvent(QResizeEvent *event)
{
    cout << "vp resize event: " << event->size().width()
         << "," << event->size().height() << endl;
    update();
}

void Viewport::update()
{
    QVector3D min, max;
    getController()->getModel()->getSurface()->extents(min, max);
    _view->initializeViewport(min, max, width(), height());
    renderLater();
}

void Viewport::addShader(const string &name, Shader *shader)
{
    _shaders[name] = shader;
}

void Viewport::render()
{
    
    glViewport(0, 0, width(), height());
    
 	// setup the lighting
    float ltcolor[] = {0, .3, 1.0};
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glLightfv(GL_LIGHT0, GL_POSITION, ltcolor);

    // set the color of the background
    QColor vpcolor = getController()->getModel()->getPreferences().getViewportColor();
    glClearColor(vpcolor.redF(), vpcolor.greenF(), vpcolor.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw the model
    getController()->getModel()->draw(*this);

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
    shader->setMatrix(_view->getWorld());
    _current_shader = shader;
    //cerr << "set line shader\n";
    return dynamic_cast<LineShader*>(_current_shader);
}

FaceShader* Viewport::setMonoFaceShader()
{
    Shader* shader = _shaders["monofaceshader"];
    if (shader == _current_shader)
        return dynamic_cast<MonoFaceShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_view->getWorld());
    _current_shader = shader;
    //cerr << "set mono face shader\n";
    return dynamic_cast<FaceShader*>(_current_shader);
}

FaceShader* Viewport::setLightedFaceShader()
{
    Shader* shader = _shaders["lightedfaceshader"];
    if (shader == _current_shader)
        return dynamic_cast<LightedFaceShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_view->getWorld());
    _current_shader = shader;
    //cerr << "set lighted face shader\n";
    return dynamic_cast<FaceShader*>(_current_shader);
}

bool Viewport::contextMenu(QMouseEvent *event)
{
    ViewportContextEvent cevent(this, event);
    emit contextMenuEvent(&cevent);
    return false;
}

// keep track of drag state as fallows:
// _drag_state = 0      no dragging, move or pick in progress
// _drag_state = 1      button has been pressed, not moved far enough for a drag
// _drag_state = 2      button has been pressed, and dragging
// _drag_state = 3      left button has been pressed, dragging point. not moved far enough for a drag
// _drag_state = 4      left button has been pressed, dragging point
void
Viewport::mousePressEvent(QMouseEvent *event)
{
    cout << "mouse press enter:" << _drag_state << endl;
    _drag_state = 1;
    // on a left mouse press, check for selection
    if (event->buttons().testFlag(Qt::LeftButton)) {
        cout << "left mouse" << endl;
        bool multi = event->modifiers() == Qt::ControlModifier;
        if (_view->leftMousePick(event->pos(), width(), height(), multi)) {
            cout << "pick state changed - multisel:" << multi << endl;
            update();
            if (!multi)
                _drag_state = 3;
        }
    }
    _prev_pos = event->pos();
    _prev_buttons = event->buttons();
    _drag_start = event->pos();
    // are we getting mouse clicks in the gl window?
    cout << "mouse press exit: " << _drag_state << " pos:" << event->pos().x() << "," << event->pos().y() << endl;
}

void
Viewport::mouseReleaseEvent(QMouseEvent *event)
{
    cout << "mouse release enter:" << _drag_state << endl;
    bool view_changed = false;
    // for mouse release, the button released won't be in the set
    if (_prev_buttons.testFlag(Qt::LeftButton) && !event->buttons().testFlag(Qt::LeftButton)) {
        if (_drag_state != 1 || _drag_state != 3)
            view_changed = _view->leftMouseRelease(event->pos(), width(), height());
    }
    else if (_prev_buttons.testFlag(Qt::RightButton) && !event->buttons().testFlag(Qt::RightButton)) {
        if (_drag_state == 1)
            view_changed = contextMenu(event);
        else
            view_changed = _view->rightMouseRelease(event->pos(), width(), height());
    }

    if (view_changed)
        update();
    _prev_buttons = event->buttons();
    // are we getting mouse clicks in the gl window?
    cout << "mouse release exit: " << _drag_state << " pos:" << event->pos().x() << "," << event->pos().y() << endl;
    _drag_state = 0;
}

void
Viewport::mouseMoveEvent(QMouseEvent *event)
{
    cout << "mouse move enter:" << _drag_state << endl;

    bool view_changed = false;

    // check for actual dragging
    if (_drag_state == 0
        || (_drag_state == 1 && (event->pos() - _drag_start).manhattanLength() < QApplication::startDragDistance())
        || (_drag_state == 3 && (event->pos() - _drag_start).manhattanLength() < QApplication::startDragDistance()))
        return;

    if (_drag_state != 2 && _drag_state != 4)
        _drag_state = _drag_state == 1 ? 2 : 4;

    if (event->buttons().testFlag(Qt::LeftButton)) {
        // are we dragging a point?
        if (_drag_state == 4) {
            QVector3D coordchg;
            if (_view->pointDrag(event->pos(), width(), height(), coordchg)) {
                // the point has been dragged
                getController()->movePoint(coordchg);
            }
        }
        else {
            view_changed = _view->leftMouseMove(event->pos(), _prev_pos, width(), height());
        }
    }
    else if (event->buttons().testFlag(Qt::MidButton)) {
        view_changed = _view->middleMouseMove(event->pos(), _prev_pos, width(), height());
    }
    else if (event->buttons().testFlag(Qt::RightButton)) {
        view_changed = _view->rightMouseMove(event->pos(), _prev_pos, width(), height());
    }

    if (view_changed)
        update();

    _prev_pos = event->pos();
    // are we getting mouse clicks in the gl window?
    cout << "mouse move exit:" << _drag_state << " pos:" << event->pos().x() << "," << event->pos().y() << endl;
}

void Viewport::wheelEvent(QWheelEvent *event)
{
    //QPoint num_pixels = event->pixelDelta();
    QPoint num_degrees = event->angleDelta() / 8;

    bool view_changed = false;
    
    //if (!num_pixels.isNull())
    //view_changed = _view->wheelWithPixels(num_pixels, width(), height());
    if (!num_degrees.isNull())
        view_changed = _view->wheelWithDegrees(num_degrees, width(), height());

    if (view_changed)
        update();
        
}

void Viewport::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier) && event->key() == Qt::Key_A) {
        _view->resetView();
        update();
    }
}

bool Viewport::shootPickRay(PickRay& ray)
{
    return getController()->shootPickRay(*this, ray);
}
