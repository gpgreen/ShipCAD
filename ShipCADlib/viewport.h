/*#############################################################################################}
  {    This code is distributed as part of the FREE!ship project. FREE!ship is an               }
  {    open source surface-modelling program based on subdivision surfaces and intended for     }
  {    designing ships.                                                                         }
  {                                                                                             }
  {    Copyright © 2005, by Martijn van Engeland                                                }
  {    e-mail                  : Info@FREEship.org                                              }
  {    FREE!ship project page  : https://sourceforge.net/projects/freeship                      }
  {    FREE!ship homepage      : www.FREEship.org                                               }
  {                                                                                             }
  {    This program is free software; you can redistribute it and/or modify it under            }
  {    the terms of the GNU General Public License as published by the                          }
  {    Free Software Foundation; either version 2 of the License, or (at your option)           }
  {    any later version.                                                                       }
  {                                                                                             }
  {    This program is distributed in the hope that it will be useful, but WITHOUT ANY          }
  {    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          }
  {    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 }
  {                                                                                             }
  {    You should have received a copy of the GNU General Public License along with             }
  {    this program; if not, write to the Free Software Foundation, Inc.,                       }
  {    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    }
  {                                                                                             }
  {#############################################################################################*/

#ifndef VIEWPORT_H_
#define VIEWPORT_H_

#include <vector>
#include <map>
#include <string>
#include <QtCore>
#include <QtGui>
#include "openglwindow.h"

namespace ShipCADGeometry {

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

    enum viewport_mode_t {
        vmWireFrame, vmShade, vmShadeGauss, vmShadeDevelopable, vmShadeZebra
    };
    enum viewport_type_t {
        fvBodyplan, fvProfile, fvPlan, fvPerspective
    };
    enum camera_type_t {
      ftWide, ftStandard, ftShortTele, ftMediumTele, ftFarTele
    };

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
    virtual void setWindowSize(const QSize& sz);

protected:

    void initializeViewport(const QVector3D& min, const QVector3D& max);

private:

    enum viewport_mode_t _mode;
    enum viewport_type_t _view_type;
    enum camera_type_t _camera;
    int m_frame;
    QMatrix4x4 _matrix;     // the final view matrix = proj * view * model
    QMatrix4x4 _model;      // the model matrix
    QMatrix4x4 _view;       // the view matrix
    QMatrix4x4 _proj;       // the projection matrix
    float _field_of_view;	/* vertical field of view in radians */
    float _angle;		/* view angle */
    float _elevation;		/* view angle */
    float _zoom;		/* view magnification */
    float _panX;
    float _panY;
    float _distance;
    float _scale;
    float _margin;
    QVector3D _min3d;
    QVector3D _max3d;
    QVector3D _midpoint;
    QVector3D _camera_location;

    std::map<std::string, Shader*> _shaders;
    Shader* _current_shader;
    std::vector<Entity*> _entities;
    SubdivisionSurface* _surface;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

