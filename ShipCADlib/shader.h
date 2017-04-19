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

#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <map>
#include <string>
#include <QtCore>
#include <QtGui>

namespace ShipCAD {

class Viewport;

//////////////////////////////////////////////////////////////////////////////////////

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

    void bind() {_program->bind();}
    void release() {_program->release();}

protected:

    Viewport* _viewport;
    QOpenGLShaderProgram *_program;
    std::map<std::string, GLuint> _uniforms;
    std::map<std::string, GLuint> _attributes;
};

//////////////////////////////////////////////////////////////////////////////////////

class LineShader : public Shader
{
    Q_OBJECT

public:

    explicit LineShader(Viewport* vp);
    virtual ~LineShader() {}

    void setMatrix(const QMatrix4x4& matrix);

    void renderPoints(QVector<QVector3D>& points, QColor color);
    void renderLines(QVector<QVector3D>& vertices, QColor lineColor);

    QVector<QVector3D>& getVertexBuffer();

private:
    QVector<QVector3D> _vertices;
};

//////////////////////////////////////////////////////////////////////////////////////

class FaceShader : public Shader
{
    Q_OBJECT

public:

    explicit FaceShader(Viewport* vp)
        : Shader(vp) {}
    virtual ~FaceShader() {}

    virtual void renderMesh(QColor meshColor,
                            QVector<QVector3D>& vertices,
                            QVector<QVector3D>& normals) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////

class MonoFaceShader : public FaceShader
{
    Q_OBJECT

public:

    explicit MonoFaceShader(Viewport* vp);
    virtual ~MonoFaceShader() {}

    void setMatrix(const QMatrix4x4& matrix);

    virtual void renderMesh(QColor meshColor,
                            QVector<QVector3D>& vertices,
                            QVector<QVector3D>& normals);

};

//////////////////////////////////////////////////////////////////////////////////////

class LightedFaceShader : public FaceShader
{
    Q_OBJECT

public:

    explicit LightedFaceShader(Viewport* vp);
    virtual ~LightedFaceShader() {}

    void setMatrices(const QMatrix4x4& proj, const QMatrix4x4& view, const QMatrix4x4& world);

    virtual void renderMesh(QColor meshColor,
                            QVector<QVector3D>& vertices,
                            QVector<QVector3D>& normals);

};

//////////////////////////////////////////////////////////////////////////////////////

class CurveFaceShader : public Shader
{
    Q_OBJECT

public:

    explicit CurveFaceShader(Viewport* vp);
    virtual ~CurveFaceShader() {}

    void setMatrices(const QMatrix4x4& proj, const QMatrix4x4& view, const QMatrix4x4& world);

    virtual void renderMesh(QVector<QVector3D>& vertices,
                            QVector<QVector3D>& colors,
                            QVector<QVector3D>& normals);

};

//////////////////////////////////////////////////////////////////////////////////////

}   // end namespace

#endif // SHADER_H
