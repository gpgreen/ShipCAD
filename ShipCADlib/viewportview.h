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

/*! \brief a pick ray (line)
 */
struct PickRay 
{
    QVector3D pt;
    QVector3D dir;
    bool multi_sel;             // true if we are multi-selecting elements
};
    
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Abstract class to calculate view, world matrices and operations using those for a viewport
 */
class ViewportView
{
public:
    
    explicit ViewportView(Viewport* vp);
    virtual ~ViewportView() {}

    static ViewportView* construct(viewport_type_t ty, Viewport* vp);

    virtual void resetView();
    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height) = 0;

    const QMatrix4x4& getWorld() const
        {return _world;}
    const QMatrix4x4& getWorldInv() const
        {return _worldInv;}

    virtual bool leftMousePick(QPoint pos, int w, int h, bool multi_sel);
    virtual bool rightMousePick(QPoint pos, int w, int h);
    virtual bool leftMouseRelease(QPoint pos, int w, int h);
    virtual bool rightMouseRelease(QPoint pos, int w, int h);
    virtual bool leftMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool middleMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool rightMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool wheelWithDegrees(QPoint degrees, int w, int h);
    /*! \brief drag a point in the viewport
     *
     * \param pos the current mouse coordinates
     * \param w width of viewport in pixels
     * \param h height of viewport in pixels
     * \param newcoord the new coordinates of the point in 3D space
     * \return true if the point can be dragged in this view
     */
    virtual bool pointDrag(QPoint pos, int w, int h, QVector3D& newcoord);
    
    /*! \brief given mouse coordinates, find a pick ray for this view
     *
     * \param pos the mouse coordinates
     * \param width the viewport width in pixels
     * \param height the viewport height in pixels
     * \return the world coordinate system PickRay corresponding to the mouse coordinates
     */
    virtual PickRay convertMouseCoordToWorld(QPoint pos, int width, int height) const;
    
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

/*! \brief class to calculate view, world matrices and operations using those for a Perspective viewport
 */
class ViewportViewPerspective : public ViewportView
{
public:
    
    explicit ViewportViewPerspective(Viewport* vp);
    virtual ~ViewportViewPerspective() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);
    virtual void resetView();
    
    float getAngle() const
        {return _angle;}
    void setAngle(float val)
        {_angle = val;}
    float getElevation() const
        {return _elevation;}
    void setElevation(float val)
        {_elevation=val;}
    camera_type_t getCameraType() const
        { return _camera; }
    void setCameraType(camera_type_t val);

    virtual bool middleMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool leftMouseMove(QPoint cur, QPoint prev, int w, int h);

private:

    float _panZ;
    camera_type_t _camera;
    float _field_of_view;	/* vertical field of view in radians */
    float _angle;           /* view angle */
    float _elevation;		/* view angle */
    float _distance;
};
        
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief class to calculate view, world matrices and operations using those for a Plan viewport
 */
class ViewportViewPlan : public ViewportView
{
public:
    
    explicit ViewportViewPlan(Viewport* vp);
    virtual ~ViewportViewPlan() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);

    virtual bool rightMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool pointDrag(QPoint pos, int w, int h, QVector3D& newcoord);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief class to calculate view, world matrices and operations using those for a Profile viewport
 */
class ViewportViewProfile : public ViewportView
{
public:
    
    explicit ViewportViewProfile(Viewport* vp);
    virtual ~ViewportViewProfile() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);
    virtual bool rightMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool pointDrag(QPoint pos, int w, int h, QVector3D& newcoord);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

/*! \brief class to calculate view, world matrices and operations using those for a Bodyplan viewport
 */
class ViewportViewBodyplan : public ViewportView
{
public:
    
    explicit ViewportViewBodyplan(Viewport* vp);
    virtual ~ViewportViewBodyplan() {}

    virtual void initializeViewport(const QVector3D& min, const QVector3D& max, int width, int height);
    virtual bool rightMouseMove(QPoint cur, QPoint prev, int w, int h);
    virtual bool pointDrag(QPoint pos, int w, int h, QVector3D& newcoord);

private:
        
};
        
//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

