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

#include <stdexcept>
#include <iostream>
#include <QtGui/QOpenGLShaderProgram>

#include "shader.h"
#include "viewport.h"

using namespace std;
using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

Shader::Shader(Viewport* vp)
  : _viewport(vp), _program(0)
{
    // does nothing
}

Shader::~Shader()
{
    delete _program;
}

void Shader::initialize(const char* vertexShaderSource,
            const char* fragmentShaderSource,
            vector<string> uniforms,
            vector<string> attributes)
{
    _program = new QOpenGLShaderProgram(_viewport);
    _program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    _program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    if (!_program->link())
        cerr << "OpenGL Link failed - Log:\n" << _program->log().toStdString() << endl;

    for (size_t i=0; i<uniforms.size(); ++i)
        addUniform(uniforms[i]);
    for (size_t i=0; i<attributes.size(); ++i)
        addAttribute(attributes[i]);
}

void Shader::addUniform(const string& name)
{
    int ul = _program->uniformLocation(name.c_str());
    if (ul == -1) {
        string err("bad uniform:");
        err += name;
        throw runtime_error(err);
    }
    _uniforms[name] = ul;
}

void Shader::addAttribute(const string& name)
{
    int al = _program->attributeLocation(name.c_str());
    if (al == -1) {
        string err("bad attribute:");
        err += name;
        throw runtime_error(err);
    }
    _attributes[name] = al;
}

//////////////////////////////////////////////////////////////////////////////////////

static const char *LineShaderVertexSource =
    "attribute highp vec4 posAttr;\n"
    "uniform highp mat4 matrix;\n"
    "uniform lowp vec4 sourceColor;\n"
    "varying mediump vec4 color;\n"
    "void main() {\n"
    "   color = sourceColor;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *LineShaderFragmentSource =
    "varying mediump vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = color;\n"
    "}\n";


LineShader::LineShader(Viewport* vp)
  : Shader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("sourceColor");
    attrs.push_back("posAttr");
    initialize(LineShaderVertexSource, LineShaderFragmentSource, unis, attrs);
    addUniform("matrix");
}

void LineShader::setMatrix(const QMatrix4x4& matrix)
{
    _program->setUniformValue(_uniforms["matrix"], matrix);
}

void LineShader::renderPoints(QVector<QVector3D>& points, QColor color)
{
    _program->setUniformValue(_uniforms["sourceColor"],
                  color.redF(),
                  color.greenF(),
                  color.blueF(),
                  1.0f);

    GLuint posAttr = _attributes["posAttr"];

    _program->setAttributeArray(posAttr, points.constData());
    _program->enableAttributeArray(posAttr);
    glDrawArrays(GL_POINTS, 0, points.size());
    _program->disableAttributeArray(posAttr);
}

void LineShader::renderLines(QVector<QVector3D>& vertices, QColor lineColor)
{
    _program->setUniformValue(_uniforms["sourceColor"],
                  lineColor.redF(),
                  lineColor.greenF(),
                  lineColor.blueF(),
                  1.0f);

    GLuint posAttr = _attributes["posAttr"];

    _program->setAttributeArray(posAttr, vertices.constData());
    _program->enableAttributeArray(posAttr);
    glDrawArrays(GL_LINES, 0, vertices.size());
    _program->disableAttributeArray(posAttr);
}

QVector<QVector3D>& LineShader::getVertexBuffer()
{
    _vertices.clear();
    return _vertices;
}

//////////////////////////////////////////////////////////////////////////////////////

static const char* MonoShaderVertexSource =
	"attribute highp vec4 vertex;"
	"attribute mediump vec3 normal;"
	"uniform mediump mat4 matrix;"
	"uniform lowp vec4 sourceColor;"
	"varying mediump vec4 color;"
	"void main(void)"
	"{"
	"    vec3 toLight = normalize(vec3(0.0, 0.3, 1.0));"
	"    float angle = max(dot(normal, toLight), 0.0);"
	"    vec3 col = sourceColor.rgb;"
	"    color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);"
	"    color = clamp(color, 0.0, 1.0);"
	"    gl_Position = matrix * vertex;"
	"}";

static const char* MonoShaderFragmentSource =
	"varying mediump vec4 color;"
	"void main(void)"
	"{"
	"    gl_FragColor = color;"
	"}";

MonoFaceShader::MonoFaceShader(Viewport* vp)
  : FaceShader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("sourceColor");
    attrs.push_back("vertex");
    attrs.push_back("normal");
    initialize(MonoShaderVertexSource, MonoShaderFragmentSource, unis, attrs);
    addUniform("matrix");
}

void MonoFaceShader::setMatrix(const QMatrix4x4& matrix)
{
    _program->setUniformValue(_uniforms["matrix"], matrix);
}

void MonoFaceShader::renderMesh(QColor meshColor,
                                QVector<QVector3D>& vertices,
                                QVector<QVector3D>& normals)
{
    if (vertices.size() != normals.size())
        throw runtime_error("vertex and normal array not same size MonoFaceShader::renderMesh");

    _program->setUniformValue(_uniforms["sourceColor"],
                  meshColor.redF(),
                  meshColor.greenF(),
                  meshColor.blueF(),
                  1.0f);

    GLuint normalAttr = _attributes["normal"];
    GLuint vertexAttr = _attributes["vertex"];

    _program->setAttributeArray(vertexAttr, vertices.constData());
    _program->setAttributeArray(normalAttr, normals.constData());
    _program->enableAttributeArray(normalAttr);
    _program->enableAttributeArray(vertexAttr);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    _program->disableAttributeArray(normalAttr);
    _program->disableAttributeArray(vertexAttr);
}

//////////////////////////////////////////////////////////////////////////////////////

#if 0
static const char *vertexShaderSourceCore =
    "#version 150\n"
    "in vec4 vertex;\n"
    "in vec3 normal;\n"
    "out vec3 vert;\n"
    "out vec3 vertNormal;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSourceCore =
    "#version 150\n"
    "in highp vec3 vert;\n"
    "in highp vec3 vertNormal;\n"
    "out highp vec4 fragColor;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   fragColor = vec4(col, 1.0);\n"
    "}\n";
#endif

static const char *vertexShaderSource =
    "attribute vec4 vertex;\n"
    "attribute vec3 normal;\n"
    "varying vec3 vert;\n"
    "varying vec3 vertNormal;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying highp vec3 vert;\n"
    "varying highp vec3 vertNormal;\n"
    "uniform highp vec3 lightPos;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 col = clamp(color.xyz * 0.2 + color.xyz * 0.8 * NL, 0.0, 1.0);\n"
    "   gl_FragColor = vec4(col, color.z);\n"
    "}\n";

LightedFaceShader::LightedFaceShader(Viewport* vp)
  : FaceShader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("lightPos");
    unis.push_back("projMatrix");
    unis.push_back("mvMatrix");
    unis.push_back("normalMatrix");
    unis.push_back("color");
    attrs.push_back("vertex");
    attrs.push_back("normal");
    initialize(vertexShaderSource, fragmentShaderSource, unis, attrs);
}

void LightedFaceShader::setMatrices(const QMatrix4x4& proj, const QMatrix4x4& view, const QMatrix4x4& world)
{
    _program->setUniformValue(_uniforms["projMatrix"], proj);
    _program->setUniformValue(_uniforms["mvMatrix"], view);
    _program->setUniformValue(_uniforms["normalMatrix"], world.normalMatrix());
    _program->setUniformValue(_uniforms["lightPos"], QVector3D(50, 20, 2));
}

void LightedFaceShader::renderMesh(QColor meshColor,
                                QVector<QVector3D>& vertices,
                                QVector<QVector3D>& normals)
{
    if (vertices.size() != normals.size())
        throw runtime_error("vertex and normal array not same size LightedFaceShader::renderMesh");

    GLuint normalAttr = _attributes["normal"];
    GLuint vertexAttr = _attributes["vertex"];

    _program->setUniformValue(_uniforms["color"], meshColor.redF(), meshColor.greenF(),
                meshColor.blueF(), meshColor.alphaF());
    _program->setAttributeArray(vertexAttr, vertices.constData());
    _program->setAttributeArray(normalAttr, normals.constData());
    _program->enableAttributeArray(normalAttr);
    _program->enableAttributeArray(vertexAttr);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    _program->disableAttributeArray(normalAttr);
    _program->disableAttributeArray(vertexAttr);
}

//////////////////////////////////////////////////////////////////////////////////////

static const char *colVertexShaderSource =
    "attribute vec4 vertex;\n"
    "attribute vec4 color;\n"
    "attribute vec3 normal;\n"
    "varying vec3 vert;\n"
    "varying vec3 vertNormal;\n"
    "varying vec4 col;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   col = color;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *colFragmentShaderSource =
    "varying highp vec3 vert;\n"
    "varying highp vec3 vertNormal;\n"
    "varying highp vec4 col;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 col = clamp(col.xyz * 0.2 + col.xyz * 0.8 * NL, 0.0, 1.0);\n"
    "   gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

CurveFaceShader::CurveFaceShader(Viewport* vp)
  : Shader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("lightPos");
    unis.push_back("projMatrix");
    unis.push_back("mvMatrix");
    unis.push_back("normalMatrix");
    attrs.push_back("vertex");
    attrs.push_back("color");
    attrs.push_back("normal");
    initialize(colVertexShaderSource, colFragmentShaderSource, unis, attrs);
}

void CurveFaceShader::setMatrices(const QMatrix4x4& proj, const QMatrix4x4& view,
                                  const QMatrix4x4& world)
{
    _program->setUniformValue(_uniforms["projMatrix"], proj);
    _program->setUniformValue(_uniforms["mvMatrix"], view);
    _program->setUniformValue(_uniforms["normalMatrix"], world.normalMatrix());
    _program->setUniformValue(_uniforms["lightPos"], QVector3D(50, 20, 2));
}

void CurveFaceShader::renderMesh(QVector<QVector3D>& vertices,
                                 QVector<QVector3D>& colors,
                                 QVector<QVector3D>& normals)
{
    if (vertices.size() != normals.size() || vertices.size() != colors.size())
        throw runtime_error("vertex, color, normal array not same size CurveFaceShader::renderMesh");

    GLuint normalAttr = _attributes["normal"];
    GLuint vertexAttr = _attributes["vertex"];
    GLuint colorAttr = _attributes["color"];

    _program->setAttributeArray(vertexAttr, vertices.constData());
    _program->setAttributeArray(colorAttr, colors.constData());
    _program->setAttributeArray(normalAttr, normals.constData());
    _program->enableAttributeArray(normalAttr);
    _program->enableAttributeArray(colorAttr);
    _program->enableAttributeArray(vertexAttr);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    _program->disableAttributeArray(normalAttr);
    _program->disableAttributeArray(colorAttr);
    _program->disableAttributeArray(vertexAttr);
}

//////////////////////////////////////////////////////////////////////////////////////

