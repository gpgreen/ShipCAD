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

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include <vector>
#include <map>
#include <QtCore>
#include <QtGui>
#include "openglwindow.h"
#include "shipcadlib.h"

namespace ShipCAD {

class Entity;
class SubdivisionSurface;
class Shader;
class LineShader;
class MonoFaceShader;

//////////////////////////////////////////////////////////////////////////////////////

class Viewport : public OpenGLWindow
{
    Q_OBJECT
public:

    explicit Viewport();
    ~Viewport();

    void initialize();
    void render();

    viewport_mode_t getViewportMode() const {return _mode;}

    viewport_type_t getViewportType() const {return _view_type;}

    camera_type_t getCameraType() const {return _camera;}

    SubdivisionSurface* getSurface() {return _surface;}

    void add(Entity* entity);

    void addShader(const std::string& name, Shader* shader);

    LineShader* setLineShader();
    MonoFaceShader* setMonoFaceShader();

public slots:

    void setViewportMode(viewport_mode_t mode);
    void setViewportType(viewport_type_t ty);
    void setCameraType(camera_type_t val);
    void setSurface(SubdivisionSurface* surface);
    void setAngle(float val);
    void setElevation(float val);
    virtual void resizeEvent(QResizeEvent *event);

protected:

    void initializeViewport(const QVector3D& min, const QVector3D& max);
    void convertMouseCoordToWorld(int mx, int my);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

private:

    viewport_mode_t _mode;
    viewport_type_t _view_type;
    camera_type_t _camera;
    QMatrix4x4 _matrix;     // the final view matrix = proj * view
    QMatrix4x4 _view;       // the view matrix
    QMatrix4x4 _proj;       // the projection matrix
    QMatrix4x4 _matrixInv;  // the inverted view matrix
    float _field_of_view;	/* vertical field of view in radians */
    float _angle;           /* view angle */
    float _elevation;		/* view angle */
    float _zoom;            /* view magnification */
    float _panX;
    float _panY;
    float _distance;
    float _scale;
    float _margin;
    QVector3D _min3d;
    QVector3D _max3d;
    QVector3D _midpoint;    // middle of model, used as rotation point
    QVector3D _camera_location;
    QPoint _prev_pos;		// last position of mouse
    Qt::MouseButtons _prev_buttons; // last capture of button state
    std::map<std::string, Shader*> _shaders;
    Shader* _current_shader;
    std::vector<Entity*> _entities;
    SubdivisionSurface* _surface;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

