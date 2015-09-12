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

#ifndef VIEWPORTVIEW_H_
#define VIEWPORTVIEW_H_

#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"

namespace ShipCAD {

class Viewport;
    
//////////////////////////////////////////////////////////////////////////////////////

class ViewportView
{
public:
    
    explicit ViewportView(Viewport* vp);
    virtual ~ViewportView() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height) = 0;
    
protected:

    virtual void finishSetup();
    
protected:

    Viewport* _vp;
    float _zoom;            /* view magnification */
    float _panX;
    float _panY;
    float _scale;
    float _margin;
    QVector3D _midpoint;    // middle of model, used as rotation point
    QVector3D _camera_location;
    QMatrix4x4 _world;      // the final view matrix = proj * view
    QMatrix4x4 _worldInv;   // the inverted world matrix
    QMatrix4x4 _view;       // the view matrix
    QMatrix4x4 _proj;       // the projection matrix
};
        
//////////////////////////////////////////////////////////////////////////////////////

class ViewportViewPerspective : public ViewportView
{
public:
    
    explicit ViewportViewPerspective(Viewport* vp);
    virtual ~ViewportViewPerspective() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);

    void setAngle(float val)
        {_angle = val;}
    void setElevation(float val)
        {_elevation=val;}
    void setCameraType(camera_type_t val);

private:
        
    camera_type_t _camera;
    float _field_of_view;	/* vertical field of view in radians */
    float _angle;           /* view angle */
    float _elevation;		/* view angle */
    float _distance;
};
        
//////////////////////////////////////////////////////////////////////////////////////

class ViewportViewPlan : public ViewportView
{
public:
    
    explicit ViewportViewPlan(Viewport* vp);
    virtual ~ViewportViewPlan() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

class ViewportViewProfile : public ViewportView
{
public:
    
    explicit ViewportViewProfile(Viewport* vp);
    virtual ~ViewportViewProfile() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

class ViewportViewBodyplan : public ViewportView
{
public:
    
    explicit ViewportViewBodyplan(Viewport* vp);
    virtual ~ViewportViewBodyplan() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

