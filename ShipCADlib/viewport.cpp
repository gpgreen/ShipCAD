#include "viewport.h"
#include "shader.h"
#include "entity.h"
#include "subdivsurface.h"
#include "utility.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

//////////////////////////////////////////////////////////////////////////////////////

Viewport::Viewport()
    : _mode(vmWireFrame), _view_type(fvPerspective), m_frame(0),
      _camera(ftStandard), _field_of_view(atan(35.0/50.0)),
      _angle(20), _elevation(20),
      _zoom(1.0), _panX(0), _panY(0),
      _distance(0), _scale(1.0), _margin(0),
      _current_shader(0), _surface(0)
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

void Viewport::setCameraType(camera_type_t val)
{
    if (val != _camera) {
        float film = 35.0;
        float dist;
        if (val == ftWide)
            dist = 20;
        else if (val == ftStandard)
            dist = 50;
        else if (val == ftShortTele)
            dist = 90;
        else if (val == ftMediumTele)
            dist = 130;
        else if (val == ftFarTele)
            dist = 200;
        else
            dist = 50;
        _camera = val;
        _field_of_view = atan(film/dist);
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setViewportType(viewport_type_t ty)
{
    if (ty != _view_type) {
        _view_type = ty;
        _zoom = 1.0;
        _panX = 0;
        _panY = 0;
        switch (ty) {
        case fvBodyplan:
            _angle = 0;
            _elevation = 0;
            break;
        case fvProfile:
            _angle = 90;
            _elevation = 0;
            break;
        case fvPlan:
            _angle = 90;
            _elevation = 90;
            break;
        case fvPerspective:
            _angle = 20;
            _elevation = 20;
            break;
        }
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setAngle(float val)
{
    if (val != _angle) {
        _angle = val;
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setElevation(float val)
{
    if (val != _elevation) {
        _elevation = val;
        initializeViewport(_min3d, _max3d);
    }
}

void Viewport::setWindowSize(const QSize& /*sz*/)
{
    // at this point, our window has been resized, so
    // redo the transformations
    initializeViewport(_min3d, _max3d);
}

// this procedure initializes the viewport and sets the scale in such a way
// that the model completely fills the viewport
void Viewport::initializeViewport(const QVector3D& min, const QVector3D& max)
{
    // calculate the midpoint of the bounding box, which is used as the center of the
    // model for rotating the model
    _midpoint = 0.5 * (_min3d + _max3d);
    // calculate the distance of the camera to the center of the model, following from
    // the field of view from the camera
    float dist = sqrt((_max3d.y() - _min3d.y()) * (_max3d.y() - _min3d.y())
                      + (_max3d.z() - _min3d.z()) * (_max3d.z() - _min3d.z()));
    if (dist == 0)
        dist = 1E-2f;
    if (_view_type == fvPerspective) {
        if (atan(_field_of_view) != 0) {
            _distance = 1.5 * dist / atan(_field_of_view);
            if (_distance > 1E5)
                _distance = 1E5;
        }
        else
            _distance = 1E5;
    }
    else
        _distance = 1E8;

    // build the vertex transformation matrix from the perspective
    // and the angle, elevation

    _proj = QMatrix4x4();
    _proj.perspective(RadToDeg(_field_of_view), 4/3.0f, 0.1f, 100.0f);

    QMatrix4x4 model;
    // find the camera location
    model.rotate(_elevation, 1, 0, 0);
    model.rotate(_angle, 0, 0, 1);
    _camera_location = model.map(QVector3D(_max3d.x() + _distance, 0, 0));
    _model = model;

    // perspective view matrix
    QMatrix4x4 view;
    view.lookAt(_camera_location, _midpoint, QVector3D(0,0,1));
    _view = view;

    // final matrix
    _matrix = _proj * _view;

    cout << "angle: " << _angle << " elev: " << _elevation << endl;
}

void Viewport::add(Entity* entity)
{
    _entities.push_back(entity);
}

void Viewport::setSurface(SubdivisionSurface* surface)
{
    _surface = surface;
    if (_surface == 0)
        return;
    _surface->extents(_min3d, _max3d);

    // add margin to max/min
    QVector3D diff = 0.01 * _margin * (_max3d - _min3d);
    _min3d -= diff;
    _max3d += diff;

    initializeViewport(_min3d, _max3d);
}

void Viewport::addShader(const string &name, Shader *shader)
{
    _shaders[name] = shader;
}

void Viewport::render()
{
    glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    LineShader* lineshader = setLineShader();

    for (size_t i=0; i<_entities.size(); ++i)
        _entities[i]->draw(*this, lineshader);

    _surface->draw(*this);

    // need to release the shader, otherwise doesn't draw right
    if (_current_shader != 0) {
        _current_shader->release();
        _current_shader = 0;
    }

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
    //cerr << "set line shader\n";
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
    //cerr << "set mono face shader\n";
    return dynamic_cast<MonoFaceShader*>(_current_shader);
}
