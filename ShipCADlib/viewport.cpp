#include <QtGui/QOpenGLShaderProgram>

#include "viewport.h"
#include "entity.h"
#include "subdivsurface.h"

using namespace std;
using namespace ShipCADGeometry;

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
  _program->link();

  addUniform("matrix");

  for (size_t i=0; i<uniforms.size(); ++i)
    addUniform(uniforms[i]);
  for (size_t i=0; i<attributes.size(); ++i)
    addAttribute(attributes[i]);
}

void Shader::addUniform(const string& name)
{
  _uniforms[name] = _program->uniformLocation(name.c_str());
  if (_uniforms[name] == -1)
    throw runtime_error("bad uniform");
}

void Shader::addAttribute(const string& name)
{
  _attributes[name] = _program->attributeLocation(name.c_str());
  if (_attributes[name] == -1)
    throw runtime_error("bad attribute");
}

void Shader::setMatrix(const QMatrix4x4& matrix)
{
  _program->setUniformValue(_uniforms["matrix"], matrix);
}
  
//////////////////////////////////////////////////////////////////////////////////////

static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "uniform vec4 sourceColor;\n"
    "varying out vec4 outColor;\n"
    "void main() {\n"
    "   outColor = sourceColor;\n"
    "}\n";


LineShader::LineShader(Viewport* vp)
  : Shader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("sourceColor");
    attrs.push_back("posAttr");
    initialize(vertexShaderSource, fragmentShaderSource, unis, attrs);
}

void LineShader::renderPoints(QVector<QVector3D>& points, QColor color)
{
    _program->setUniformValue(_uniforms["sourceColor"],
                  color.redF(),
                  color.greenF(),
                  color.blueF(),
                  1.0f);

    GLuint posAttr = _attributes["posAttr"];

    _program->enableAttributeArray(posAttr);
    _program->setAttributeArray(posAttr, points.constData());
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

    _program->enableAttributeArray(posAttr);
    _program->setAttributeArray(posAttr, vertices.constData());
    glDrawArrays(GL_LINES, 0, vertices.size());
    _program->disableAttributeArray(posAttr);
}

//////////////////////////////////////////////////////////////////////////////////////

static const char* VertexShadervmShade =
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

static const char* FragmentShadervmShade =
        "varying mediump vec4 color;"
        "void main(void)"
        "{"
        "    gl_FragColor = color;"
        "}";

MonoFaceShader::MonoFaceShader(Viewport* vp)
  : Shader(vp)
{
    vector<string> attrs;
    vector<string> unis;
    unis.push_back("sourceColor");
    attrs.push_back("vertex");
    attrs.push_back("normal");
    initialize(VertexShadervmShade, FragmentShadervmShade, unis, attrs);
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

    _program->enableAttributeArray(normalAttr);
    _program->enableAttributeArray(vertexAttr);
    _program->setAttributeArray(vertexAttr, vertices.constData());
    _program->setAttributeArray(normalAttr, normals.constData());
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    _program->disableAttributeArray(normalAttr);
    _program->disableAttributeArray(vertexAttr);
}

//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport()
    : _mode(vmWireFrame), _viewtype(fvPerspective), m_frame(0), _current_shader(0)
{
    // does nothing else
}

Viewport::~Viewport()
{
    map<string, Shader*>::iterator i = _shaders.begin();
    while (i != _shaders.end()) {
        delete (*i).second;
        ++i;
    }
}

void Viewport::initialize()
{
    LineShader* lineshader = new LineShader(this);
    addShader("lineshader", lineshader);

    MonoFaceShader* monofaceshader = new MonoFaceShader(this);
    addShader("monofaceshader", monofaceshader);
}

void Viewport::setViewportMode(viewport_mode_t mode)
{
    _mode = mode;
    // BUGBUG: need to trigger redraw
}

void Viewport::setViewportType(viewport_type_t ty)
{
    if (ty != _viewtype) {
        _viewtype = ty;
        //_zoom = 1.0;
        // BUGBUG: need to do rest of viewangle
    }
}

void Viewport::add(Entity* entity)
{
    _entities.push_back(entity);
}

void Viewport::add(SubdivisionSurface* surface)
{
    _surfaces.push_back(surface);
}

void Viewport::addShader(const string &name, Shader *shader)
{
    _shaders[name] = shader;
}

void Viewport::render()
{
    glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    _matrix = QMatrix4x4();
    _matrix.perspective(90, 4.0f/3.0f, 0.1f, 100.0f);
    _matrix.translate(0, 0, -2);
    _matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    LineShader* lineshader = setLineShader();

    for (size_t i=0; i<_entities.size(); ++i)
        _entities[i]->draw(*this, lineshader);

    // now the shader can change...
    for (size_t i=0; i<_surfaces.size(); ++i)
        _surfaces[i]->draw(*this);

    if (_current_shader != 0)
        _current_shader->release();

    ++m_frame;
}

LineShader* Viewport::setLineShader()
{
    Shader* shader = _shaders["lineshader"];
    if (shader == _current_shader)
        return dynamic_cast<LineShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_matrix);
    _current_shader = shader;
    return dynamic_cast<LineShader*>(_current_shader);
}

MonoFaceShader* Viewport::setMonoFaceShader()
{
    Shader* shader = _shaders["monofaceshader"];
    if (shader == _current_shader)
        return dynamic_cast<MonoFaceShader*>(_current_shader);
    if (_current_shader != 0)
        _current_shader->release();
    shader->bind();
    shader->setMatrix(_matrix);
    _current_shader = shader;
    return dynamic_cast<MonoFaceShader*>(_current_shader);
}

