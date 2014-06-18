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
  addUniform("sourceColor");

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

void Shader::setColor(QColor newcolor)
{
  _program->setUniformValue(_uniforms["sourceColor"],
			    newcolor.redF(),
			    newcolor.greenF(),
			    newcolor.blueF(),
			    1.0f);
}

void Shader::setColorRGBA(QColor newcolor, float alpha)
{
  _program->setUniformValue(_uniforms["sourceColor"],
			    newcolor.redF(),
			    newcolor.greenF(),
			    newcolor.blueF(),
			    alpha);
}

void Shader::setMatrix(const QMatrix4x4& matrix)
{
  _program->setUniformValue(_uniforms["matrix"], matrix);
}
  
//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport()
    : _mode(vmWireFrame), m_frame(0), _current_shader(0)
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

void Viewport::initialize()
{
    //    m_program = new QOpenGLShaderProgram(this);
    //    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    //    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    //    m_program->link();
    //    m_posAttr = m_program->attributeLocation("posAttr");
    //    m_matrixUniform = m_program->uniformLocation("matrix");
    //    m_sourceColorUniform = m_program->uniformLocation("sourceColor");
    //    _vertexAttr = m_program->attributeLocation("vertex");
    //    _normalAttr = m_program->attributeLocation("normal");

    //    m_faceProgram = new QOpenGLShaderProgram(this);
    //    m_faceProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, VertexShadervmShade);
    //    m_faceProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, FragmentShadervmShade);
    //    m_faceProgram->link();
    //    m_matrixUniform = m_faceProgram->uniformLocation("matrix");
    //    m_sourceColorUniform = m_faceProgram->uniformLocation("sourceColor");
    //    _vertexAttr = m_faceProgram->attributeLocation("vertex");
    //    _normalAttr = m_faceProgram->attributeLocation("normal");

    vector<string> attrs;
    vector<string> unis;

    Shader* lineshader = new Shader(this);
    attrs.push_back("posAttr");
    lineshader->initialize(vertexShaderSource, fragmentShaderSource, unis, attrs);
    addShader("lineshader", lineshader);

    attrs.clear();
    unis.clear();
    Shader* faceshader = new Shader(this);
    attrs.push_back("vertex");
    attrs.push_back("normal");
    faceshader->initialize(VertexShadervmShade, FragmentShadervmShade, unis, attrs);
    addShader("faceshader", faceshader);
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
    _current_shader->setColorRGBA(newcolor, 1.0f);
//    m_program->setUniformValue(m_sourceColorUniform,
//                               newcolor.redF(),
//                               newcolor.greenF(),
//                               newcolor.blueF(),
//			       1.0f);
}

void Viewport::setColorRGBA(QColor newcolor, float alpha)
{
    _current_shader->setColorRGBA(newcolor, alpha);
//    m_program->setUniformValue(m_sourceColorUniform,
//                               newcolor.redF(),
//                               newcolor.greenF(),
//                               newcolor.blueF(),
//			       alpha);
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

void Viewport::renderMesh(size_t nvertices, QVector3D* vertices, QVector3D* normals)
{
//    m_program->enableAttributeArray(_normalAttr);
//    m_program->enableAttributeArray(_vertexAttr);
//    m_program->setAttributeArray(_vertexAttr, vertices);
//    m_program->setAttributeArray(_normalAttr, normals);
//    glDrawArrays(GL_TRIANGLES, 0, nvertices);
//    m_program->disableAttributeArray(_normalAttr);
//    m_program->disableAttributeArray(_vertexAttr);
}

void Viewport::render()
{
    glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    Shader* shader = _shaders["lineshader"];
    _current_shader = shader;

    QMatrix4x4 matrix;
    matrix.perspective(90, 4.0f/3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
    matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    //m_program->bind();
    shader->bind();

    //m_program->setUniformValue(m_matrixUniform, matrix);
    shader->setMatrix(matrix);

    for (size_t i=0; i<_entities.size(); ++i)
        _entities[i]->draw(*this);
    for (size_t i=0; i<_surfaces.size(); ++i)
        _surfaces[i]->draw(*this);

    //m_program->release();
    shader->release();

    ++m_frame;
}

