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

    void initialize();
    void render();

    viewport_mode_t getViewportMode() const {return _mode;}
    void setViewportMode(viewport_mode_t mode);

    viewport_type_t getViewportType() const {return _viewtype;}
    void setViewportType(viewport_type_t ty);

    SubdivisionSurface* getSurface() {return _surface;}
    void setSurface(SubdivisionSurface* surface) {_surface = surface;}

    void add(Entity* entity);

    void addShader(const std::string& name, Shader* shader);

    LineShader* setLineShader();
    MonoFaceShader* setMonoFaceShader();

private:

    enum viewport_mode_t _mode;
    enum viewport_type_t _viewtype;

    int m_frame;
    QMatrix4x4 _matrix;

    std::map<std::string, Shader*> _shaders;
    Shader* _current_shader;
    std::vector<Entity*> _entities;
    SubdivisionSurface* _surface;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

