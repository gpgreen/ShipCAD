#include <QtGui/QOpenGLShaderProgram>

#include "viewport.h"
#include "entity.h"
#include "subdivsurface.h"

using namespace ShipCADGeometry;

//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport()
    : _mode(vmWireFrame), m_program(0), m_frame(0)
{
    // does nothing else
}

Viewport::~Viewport()
{
    delete m_program;
}

#if 0
static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";
#endif
static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "uniform vec3 fragmentColor;\n"
    "varying out vec4 outColor;\n"
    "void main() {\n"
    "   outColor = vec4(fragmentColor, 1.0);\n"
    "}\n";

void Viewport::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_program->link();
    m_posAttr = m_program->attributeLocation("posAttr");
//    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
    m_fragColorUniform = m_program->uniformLocation("fragmentColor");
}

GLuint Viewport::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

Viewport::ViewportMode Viewport::getViewportMode() const
{
    return _mode;
}

void Viewport::setViewportMode(enum ViewportMode mode)
{
    _mode = mode;
}

void Viewport::setColor(QColor newcolor)
{
    m_program->setUniformValue(m_fragColorUniform,
                               newcolor.redF(),
                               newcolor.greenF(),
                               newcolor.blueF());
}

void Viewport::add(Entity* entity)
{
    _entities.push_back(entity);
}

void Viewport::add(SubdivisionSurface* surface)
{
    _surfaces.push_back(surface);
}

void Viewport::render()
{
    glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(90, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    for (size_t i=0; i<_entities.size(); ++i)
        _entities[i]->draw(*this);
    for (size_t i=0; i<_surfaces.size(); ++i)
        _surfaces[i]->draw(*this);

#if 0
    GLfloat vertices[] = {
        0.0f, 0.707f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
#endif
    m_program->release();

    ++m_frame;
}

