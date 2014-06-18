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

//////////////////////////////////////////////////////////////////////////////////////

class Viewport;

class Shader : public QObject
{
    Q_OBJECT

public:

    explicit Shader(Viewport* vp);
    virtual ~Shader();

    virtual void initialize(const char* vertexShaderSource,
		    const char* fragmentShaderSource,
		    std::vector<std::string> uniforms,
		    std::vector<std::string> attributes);

    void addUniform(const std::string& name);
    void addAttribute(const std::string& name);

    void setColor(QColor newcolor);
    void setColorRGBA(QColor newcolor, float alpha);

    void setMatrix(const QMatrix4x4& matrix);

    void bind() {_program->bind();}
    void release() {_program->release();}

private:

    Viewport* _viewport;
    QOpenGLShaderProgram *_program;
    std::map<std::string, GLuint> _uniforms;
    std::map<std::string, GLuint> _attributes;
};

//////////////////////////////////////////////////////////////////////////////////////

class Viewport : public OpenGLWindow
{
    Q_OBJECT
public:

    explicit Viewport();
    ~Viewport();

    enum ViewportMode {
      vmWireFrame, vmShade, vmShadeGauss, vmShadeDevelopable, vmShadeZebra
    };

    void initialize();
    void render();
    void renderMesh(size_t nvertices, QVector3D* vertices, QVector3D* normals);

    enum ViewportMode getViewportMode() const;
    void setViewportMode(enum ViewportMode mode);

    void add(Entity* entity);
    void add(SubdivisionSurface* surface);

    void setColor(QColor newcolor);
    void setColorRGBA(QColor newcolor, float alpha);

    void addShader(const std::string& name, Shader* shader);

private:

    enum ViewportMode _mode;

//    GLuint m_posAttr;
//    GLuint m_matrixUniform;
//    GLuint m_sourceColorUniform;
//    GLuint _normalAttr;
//    GLuint _vertexAttr;

//    QOpenGLShaderProgram *m_program;
//    QOpenGLShaderProgram *m_faceProgram;

    int m_frame;

    std::map<std::string, Shader*> _shaders;
    Shader* _current_shader;
    std::vector<Entity*> _entities;
    std::vector<SubdivisionSurface*> _surfaces;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

