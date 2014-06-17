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
#include <QtCore>
#include <QtGui>
#include "openglwindow.h"

namespace ShipCADGeometry {

class Entity;
class SubdivisionSurface;

//////////////////////////////////////////////////////////////////////////////////////

class Viewport : public OpenGLWindow
{
    Q_OBJECT
public:

    explicit Viewport();
    ~Viewport();

    enum ViewportMode {vmWireFrame, vmShade, vmShadeGauss, vmShadeDevelopable, vmShadeZebra};

    void initialize();
    void render();

    enum ViewportMode getViewportMode() const;
    void setViewportMode(enum ViewportMode mode);

    void add(Entity* entity);
    void add(SubdivisionSurface* surface);

    void setColor(QColor newcolor);

private:

    GLuint loadShader(GLenum type, const char *source);

    enum ViewportMode _mode;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    GLuint m_fragColorUniform;

    QOpenGLShaderProgram *m_program;
    int m_frame;

    std::vector<Entity*> _entities;
    std::vector<SubdivisionSurface*> _surfaces;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

