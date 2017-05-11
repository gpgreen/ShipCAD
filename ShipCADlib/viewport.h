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

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include <vector>
#include <map>
#include <QtCore>
#include <QtGui>
#include "openglwindow.h"
#include "shipcadlib.h"

namespace ShipCAD {

class SubdivisionSurface;
class Shader;
class LineShader;
class FaceShader;
class CurveFaceShader;
class ViewportView;
class Controller;
struct PickRay;
class Viewport;
    
//////////////////////////////////////////////////////////////////////////////////////

class ViewportContextEvent : public QObject
{
    Q_OBJECT
public:
    explicit ViewportContextEvent(Viewport* vp, QMouseEvent* event);

    Viewport* getViewport() {return _vp;}
    QMouseEvent* getMouseEvent() {return _event;}
    
private:
    Viewport* _vp;
    QMouseEvent* _event;
};
    
//////////////////////////////////////////////////////////////////////////////////////

class Viewport : public OpenGLWindow
{
    Q_OBJECT
public:

    explicit Viewport(ShipCAD::Controller* ctl, viewport_type_t vtype);
    virtual ~Viewport();

    virtual void initialize();

    /*! \brief clear background
     */
    virtual void clearBackground();

    /*! \brief render OpenGL
     */
    virtual void renderOpenGL();

    /*! \brief render qpainter
     */
    virtual void renderWithPainter(QPainter* painter);
    
    ShipCAD::Controller* getController();
    const ShipCAD::Controller* getController() const;

    viewport_mode_t getViewportMode() const {return _mode;}

    viewport_type_t getViewportType() const {return _view_type;}

    camera_type_t getCameraType() const;

    const QVector3D& getCamera() const;
    
    float getAngle() const;
    float getElevation() const;
    
    void addShader(const std::string& name, Shader* shader);

    LineShader* setLineShader();
    FaceShader* setMonoFaceShader();
    FaceShader* setLightedFaceShader();
    CurveFaceShader* setCurveFaceShader();

    bool shootPickRay(PickRay& ray);

    bool canPick() const;
    bool canPickFace() const;

signals:
    void contextMenuEvent(ShipCAD::ViewportContextEvent* event);
                                                 
public slots:

    void setViewportMode(viewport_mode_t mode);
    void setViewportType(viewport_type_t ty);
    void setCameraType(camera_type_t val);
    void setAngle(float val);
    void setElevation(float val);
    virtual void resizeEvent(QResizeEvent *event);
    bool contextMenu(QMouseEvent *event);
    
protected:

    virtual void update();
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void wheelEvent(QWheelEvent *);
    virtual void keyPressEvent(QKeyEvent *);
    
private:

    // define away copy constructor and assigment operator
    Viewport(const Viewport&);
    Viewport& operator=(const Viewport&);

    // members
    Controller* _ctl;
    viewport_mode_t _mode;
    viewport_type_t _view_type;
    ViewportView* _view;
    QPoint _prev_pos;		// last position of mouse
    QPoint _drag_start;
    int _drag_state;
    Qt::MouseButtons _prev_buttons; // last capture of button state
    std::map<std::string, Shader*> _shaders;
    Shader* _current_shader;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

