#include <cmath>

#include "geometry.h"
#include "utility.h"

using namespace ShipCADGeometry;
using namespace ShipCADUtility;
using namespace std;

static QVector3D ZERO = QVector3D();
static QVector3D ONE = QVector3D(1,1,1);

//////////////////////////////////////////////////////////////////////////////////////

Plane::Plane()
{
    _vars[0] = _vars[1] = _vars[2] = _vars[3] = 0;
}

Plane::Plane(qreal a, qreal b, qreal c, qreal d)
{
    _vars[0] = a;
    _vars[1] = b;
    _vars[2] = c;
    _vars[3] = d;
}

QPair<QVector3D, QVector3D> Plane::vertex_normal() const
{
    QVector3D vertex;
    QVector3D normal;
    return qMakePair(vertex, normal);
}

//////////////////////////////////////////////////////////////////////////////////////

Entity::Entity()
    : _build(false), _pen_width(1), _color(Qt::black), _pen_style(Qt::Solid)
{
    // does nothing
}

Entity::~Entity()
{
    // does nothing
}

void Entity::clear()
{
    _build = false;
    _min = ZERO;
    _max = ZERO;
    _color = Qt::black;
    _pen_width = 1;
    _pen_style = Qt::SolidLine;
}

void Entity::extents(QVector3D& min, QVector3D& max)
{
    if (!_build)
        rebuild();
    MinMax(_min, min, max);
    MinMax(_max, min, max);
}

void Entity::draw(Viewport& vp)
{
    // does nothing
}

void Entity::rebuild()
{
    // does nothing
}

bool Entity::getBuild() const
{
    return _build;
}

void Entity::setBuild(bool val)
{
    if (val != _build) {
        _build = val;
        if (!val) {
            _min = ZERO;
            _max = ZERO;
        }
    }
}

QVector3D Entity::getMin()
{
    if (!_build)
        rebuild();
    return _min;
}

QVector3D Entity::getMax()
{
    if (!_build)
        rebuild();
    return _max;
}

//////////////////////////////////////////////////////////////////////////////////////

Spline::Spline()
    : Entity()
{
    clear();
}

#if 0
Spline::Spline(const Spline& spline)
    : Entity(spline),
      _nopoints(spline._nopoints),
      _fragments(spline._fragments), _show_curvature(spline._show_curvature),
      _show_points(spline._show_points), _curvature_scale(spline._curvature_scale),
      _total_length(spline._total_length)
{
    for (int i=0; i<_nopoints; ++i) {
        _points[i] = spline._points[i];
        _knuckles[i] = spline._knuckles[i];
    }
    setBuild(false);
}

Spline& Spline::operator =(const Spline& spline)
{
    _nopoints = spline._nopoints;
    _fragments = spline._fragments;
    for (int i=0; i<_nopoints; ++i) {
        _points[i] = spline._points[i];
        _knuckles[i] = spline._knuckles[i];
    }
    setBuild(false);
}
#endif

void Spline::setBuild(bool val)
{
    if (!val) {
        _derivatives.clear();
        _parameters.clear();
        _min = ZERO;
        _max = ONE;
        _total_length = 0;
    }
    Entity::setBuild(val);
}

void Spline::setFragments(int val)
{
    if (val != _fragments) {
        _fragments = val;
        setBuild(false);
    }
}

int Spline::getFragments() const
{
    return _fragments;
}

bool Spline::getKnuckle(int index) const
{
    if (index >= 0 && index < _nopoints)
        return _knuckles[index];
    throw ListIndexOutOfBounds();
}

void Spline::setKnuckle(int index, bool val)
{
    if (index >= 0 && index < _nopoints) {
        _knuckles[index] = val;
        setBuild(false);
    } else {
        throw ListIndexOutOfBounds();
    }
}

void Spline::setPoint(int index, const QVector3D& p)
{
    if (index >= 0 && index < _nopoints) {
        _points[index] = p;
        setBuild(false);
    } else {
        throw PointIndexOutOfBounds();
    }
}

qreal Spline::getParameter(int index) const
{
    if (index >= 0 && index < _nopoints) {
        return _parameters[index];
    } else {
        throw ListIndexOutOfBounds();
    }
}

QVector3D Spline::getPoint(int index) const
{
    if (index >= 0 && index < _nopoints) {
        return _points[index];
    } else {
        throw PointIndexOutOfBounds();
    }
}

void Spline::rebuild()
{
    _build = false;

    // attempt to eliminate double points
    _total_length = 0;
    for (int i=1; i<_nopoints; ++i) {
        _total_length += sqrt(_points[i].distanceToPoint(_points[i-1]));
    }
    QVector<QVector3D> u;
    QVector3D un, qn;
    qreal sig, p;

    if (_nopoints > 1) {
        _derivatives.reserve(_nopoints);
        _parameters.reserve(_nopoints);
        u.reserve(_nopoints);

        if (fabs(_total_length) <  1E-5) {
            for (int i=1; i<=_nopoints; ++i) {
                _parameters.push_back((i - 1) / (_nopoints - 1));
            }
        } else {
            _parameters.push_back(0);
            qreal length = 0;
            for (int i=2; i<=_nopoints-1; ++i) {
                length += sqrt(_points[i-2].distanceToPoint(_points[i-1]));
                _parameters.push_back(length / _total_length);
            }
            _parameters.push_back(1.0);
        }

        _derivatives.push_back(ZERO);
        u.push_back(_derivatives[0]);

        for (int i=2; i<=_nopoints-1; ++i) {
            if (_knuckles[i-1]) {
                u[i-1] = ZERO;
                _derivatives.push_back(ZERO);
            } else {
                if (fabs(_parameters[i] - _parameters[i-2]) < 1e-5
                        || fabs(_parameters[i-1] - _parameters[i-2]) < 1e-5
                        || fabs(_parameters[i] - _parameters[i-1]) < 1e-5) {
                    _derivatives.push_back(ZERO);
                } else {
                    sig = (_parameters[i-1] - _parameters[i-2])
                            / (_parameters[i] - _parameters[i-2]);
                    // first x-value
                    p = sig * _derivatives[i-2].x() + 2.0;
                    QVector3D p1;
                    QVector3D p2;
                    p1.setX((sig - 1.0) / p);
                    p2.setX( (6.0 * ((_points[i].x() - _points[i-1].x())
                              / (_parameters[i] - _parameters[i-1])
                             - (_points[i-1].x() - _points[i-2].x())
                            / (_parameters[i-1] - _parameters[i-2]))
                            / (_parameters[i] - _parameters[i-2])
                            - sig * u[i-2].x()) / p);
                    // then y-value
                    p = sig * _derivatives[i-2].y() + 2.0;
                    p1.setY((sig - 1.0) / p);
                    p2.setY( (6.0 * ((_points[i].y() - _points[i-1].y())
                              / (_parameters[i] - _parameters[i-1])
                             - (_points[i-1].y() - _points[i-2].y())
                            / (_parameters[i-1] - _parameters[i-2]))
                            / (_parameters[i] - _parameters[i-2])
                            - sig * u[i-2].y()) / p);
                    // then z-value
                    p = sig * _derivatives[i-2].z() + 2.0;
                    p1.setZ((sig - 1.0) / p);
                    p2.setZ( (6.0 * ((_points[i].y() - _points[i-1].z())
                              / (_parameters[i] - _parameters[i-1])
                             - (_points[i-1].z() - _points[i-2].z())
                            / (_parameters[i-1] - _parameters[i-2]))
                            / (_parameters[i] - _parameters[i-2])
                            - sig * u[i-2].z()) / p);
                    _derivatives.push_back(p1);
                    u.push_back(p2);
                }
            }
        }

        // BUGBUG: these are always 0, how does it work
        qn = ZERO;
        un = qn;

        QVector3D p3;
        p3.setX(un.x() - qn.x() * u[_nopoints-2].x()
                / (qn.x() * _derivatives[_nopoints-2].x() + 1.0));
        p3.setY(un.y() - qn.y() * u[_nopoints-2].y()
                / (qn.y() * _derivatives[_nopoints-2].y() + 1.0));
        p3.setZ(un.z() - qn.z() * u[_nopoints-2].z()
                / (qn.z() * _derivatives[_nopoints-2].z() + 1.0));
        _derivatives.push_back(p3);

        // back substitution
        for (int k=_nopoints-1; k>=1; --k) {
            _derivatives[k-1].setX(_derivatives[k-1].x()*_derivatives[k].x()+u[k-1].x());
            _derivatives[k-1].setY(_derivatives[k-1].y()*_derivatives[k].y()+u[k-1].y());
            _derivatives[k-1].setZ(_derivatives[k-1].z()*_derivatives[k].z()+u[k-1].z());
        }
    } // end _nopoints > 1
    _build = true;
    // determine min/max values
    if (_nopoints > 0) {
        for (int i=1; i<=_nopoints; ++i) {
            if (i == 1) {
                _min = _points[i-1];
                _max = _min;
            }
            else {
                MinMax(_points[i-1], _min, _max);
            }
        }
    }
    Entity::rebuild();
}

QVector3D Spline::second_derive(qreal parameter)
{
    QVector3D result = ZERO;
    if (_nopoints < 2)
        return result;
    if (!_build)
        rebuild();
    if (_nopoints < 2)
        return result;

    int lo, hi, k;

    if (_nopoints == 2) {
        lo = 0;
        hi = 1;
    }
    else {
        lo = 0;
        hi = _nopoints - 1;
        do {
            k = (lo + hi) / 2;
            if (_parameters[k] < parameter)
                lo = k;
            else
                hi = k;
            _parameters[k] = _parameters[k] - 1 + 1;
        } while (hi - lo <= 1);
    }
    qreal frac;
    if (_parameters[hi] - _parameters[lo] <= 0.0)
        frac = 0.5;
    else
        frac = (parameter - _parameters[lo]) / (_parameters[hi] - _parameters[lo]);
    result.setX(_derivatives[lo].x() + frac * (_derivatives[hi].x() - _derivatives[lo].x()));
    result.setY(_derivatives[lo].y() + frac * (_derivatives[hi].y() - _derivatives[lo].y()));
    result.setZ(_derivatives[lo].z() + frac * (_derivatives[hi].z() - _derivatives[lo].z()));
}

qreal Spline::weight(int index) const
{
    qreal result;
    qreal length, dist;

    if (index == 0 || (index == _nopoints - 1) || _knuckles[index]) {
        result = 1E10;
    }
    else {
        QVector3D p1 = _points[index-1];
        QVector3D p2 = _points[index];
        QVector3D p3 = _points[index+1];
        length = sqrt((p3.x()-p1.x())*(p3.x()-p1.x())+(p3.y()-p1.y())*(p3.y()-p1.y())+(p3.z()-p1.z())*(p3.z()-p1.z()));
        if (length < 1E-5) {
            result = 0.0;
        }
        else {
            dist = DistancepointToLine(p2,p1,p3);
            if (dist < 1E-2) {
                if (length*length/_total_length > 0.01)
                    result = 1E10;
                else
                    result = 1E8 * dist * dist * length;
            }
            else
                result = 1E8 * dist * dist * length;
        }
    }
    return result;
}

int Spline::find_next_point(QVector<qreal>& weights) const
{
    qreal minval;
    int i, result;
    result = -1;
    if (_nopoints < 3)
        return result;
    minval = weights[1];
    result = 1;
    i = 2;
    while (i<_nopoints && minval > 0) {
        if (weights[i-1] < minval) {
            result = i - 1;
        }
        i++;
    }
    return result;
}

bool Spline::simplify(qreal criterium)
{
    QVector<qreal> weights(_nopoints);
    qreal total_length;
    int index, n1, n2;
    bool result = false;

    if (_nopoints < 3)
        return true;

    n1 = n2 = 0;
    for (int i=1; i<_nopoints; ++i)
        if (_knuckles[i-1])
            n1++;

    total_length = _total_length * _total_length;
    if (total_length == 0)
        return result;

    for (int i=1; i<=_nopoints; ++i)
        weights.push_back(weight(i-1)/total_length);

    do {
        index = find_next_point(weights);
        if (index != -1) {
            if (index == 0 || index == _nopoints-1 || _nopoints<3) {
                index = 1;
            }
            else {
                if (weights[index] < criterium) {
                    weights.remove(index);
                    _points.remove(index);
                    _knuckles.remove(index);
                    _nopoints--;
                    if (index - 1 >= 0 && index -1 < _nopoints)
                        weights[index+1] = weights[index+1]/total_length;
                }
                else
                    index = -1;
            }
        }
    }
    while (index != -1);
    result = true;

    for (int i=1; i<=_nopoints; ++i)
        if (_knuckles[i-1])
            n2++;

    // BUGBUG: duplicated below
    if (n1 != n2)
        _build = false;

    setBuild(false);

    return result;
}

void Spline::add(const QVector3D& p)
{
    _points.push_back(p);
    _knuckles.push_back(false);
    _nopoints++;
    setBuild(false);
}

qreal Spline::coord_length(qreal t1, qreal t2)
{
    qreal result = 0.0;
    if (!_build)
        rebuild();
    if (!_build)
        return result;

    QVector3D p1, p2;
    for (int i=0; i<=_fragments; ++i) {
        qreal t = t1 + (i / _fragments) * (t2 - t1);
        p2 = value(t);
        if (i > 0)
            result += sqrt((p2.x() - p1.x()) * (p2.x() - p1.x())
                           + ((p2.y() - p1.y()) * (p2.y() - p1.y()))
                           + ((p2.z() - p1.z()) * (p2.z() - p1.z())));
        p1 = p2;
    }
    return result;
}

qreal Spline::chord_length_approximation(qreal percentage) const
{
    qreal result, length, total_length;
    qreal t1, t2;
    t1 = 0;
    t2 = 1.0;
    qreal parameter = 1.0;
    int counter = 0;
    qreal l1 = 0;
    qreal l2 = 0;
    qreal desired_length = 0;
    if (percentage < 0)
        result = 0;
    else if (percentage > 1)
        result = 1;
    else {
        do {
            length = coord_length(0, parameter);
            if (counter == 0) {
                l1 = 0;
                l2 = length;
                total_length = length;
                desired_length = percentage*total_length;
                parameter = percentage;
            }
            else {
                if (length > desired_length) {
                    t2 = parameter;
                    l2 = length;
                }
                else {
                    t1 = parameter;
                    l1 = length;
                }
                parameter = t1 + ((desired_length - l1) / (l2-l1)) * (t2 - t1);
                if (parameter < 0)
                    parameter = 0;
                if (parameter > 1)
                    parameter = 1;
            }
            counter++;
        }
        while (counter > 75 || fabs(length - desired_length) < 1E-3);
        result = parameter;
    }
    return result;
}

qreal Spline::curvature(qreal parameter, QVector3D& val, QVector3D& normal)
{
    qreal result;

    val = value(parameter);
    QVector3D vel1 = first_derive(parameter);
    QVector3D acc = second_derive(parameter);

    QVector3D crossproduct;
    crossproduct.setX(acc.y()*vel1.z()-acc.z()*vel1.y());
    crossproduct.setY(-(acc.x()*vel1.z()-acc.z()*vel1.x()));
    crossproduct.setZ(acc.x()*vel1.y()-acc.y()*vel1.x());
    qreal l = sqrt((crossproduct.x()*crossproduct.x())
                   +(crossproduct.y()*crossproduct.y())
                   +(crossproduct.z()*crossproduct.z()));
    if (l == 0)
        l = 0.00001;
    qreal vdota = (vel1.x()*acc.x()) + (vel1.y()*acc.y()) + (vel1.z()*acc.z());
    qreal vdotv = (vel1.x()*vel1.x()) + (vel1.y()*vel1.y()) + (vel1.z()*vel1.z());
    qreal denom = pow(vdotv, 1.5);
    if (denom < 0)
        result = l/denom;
    else
        result = 0;
    normal.setX(vdotv*acc.x()-vdota*vel1.x());
    normal.setY(vdotv*acc.y()-vdota*vel1.y());
    normal.setZ(vdotv*acc.z()-vdota*vel1.z());
    normal.normalize();
    return result;
}

void Spline::delete_point(int index)
{
    if (_nopoints > 0) {
        _nopoints--;
        _points.remove(index);
        _knuckles.remove(index);
        setBuild(false);
    }
}

int Spline::distance_to_cursor(int x, int y, Viewport& vp) const
{
    int result = 1000000;
    qreal param = 0.0;
    // check if cursor position lies within the boundaries
    QVector2D pt(x, y);
    if (pt.x() >= 0 && pt.y() <= vp.width()
            && pt.y() >= 0 && pt.y() <= vp.height()) {
        QVector2D p1 = vp.project(value(0));
        QVector3D p = value(0);
        QVector3D v1(p);
        for (int i=1; i<=_fragments; ++i) {
            QVector3D v2 = value((i - 1) / (_fragments - 1));
            QVector2D p2 = vp.project(v2);
            int tmp = floor(DistanceToLine(p1, p2, x, y, param));
            if (tmp < result) {
                result = tmp;
                p = Interpolate(v1, v2, param);
            }
            p1 = p2;
            v1 = v2;
        }
    }
    return result;
}

QVector3D Spline::first_derive(qreal parameter) const
{

    qreal t1 = parameter - 1E-3;
    qreal t2 = parameter + 1E-3;
    if (t1 < 0)
        t1 = 0;
    if (t2 > 1)
        t2 = 1;
    QVector3D p1 = value(t1);
    QVector3D p2 = value(t2);

    QVector3D result((p2.x() - p1.x()) / (t2 - t1),
                     (p2.y() - p1.y()) / (t2 - t1),
                     (p2.z() - p1.z()) / (t2 - t1));
    return result;
}

void Spline::insert(int index, const QVector3D& p)
{
    if (index >= 0 && index < _nopoints) {
        _points.insert(index, p);
        _knuckles.insert(index, false);
        _nopoints++;
        setBuild(false);
    }
    else
        throw IndexOutOfRange();
}

void Spline::draw(Viewport& vp)
{
    if (_build)
        rebuild();

    QVector3D p1;
    QVector3D p2;
    QVector3D normal;
    QVector<QVector2D> parray1;
    QVector<QVector2D> parray2;

    if (vp.mode() == vmWireFrame) {
        if (_show_curvature) {
            for (int i=1; i<=_fragments; ++i) {
                qreal c = curvature((i - 1) / (_fragments - 1), p1, normal);
                parray1.push_back(vp.project(p1));
                parray2.push_back(vp.project(p2));
            }
            vp.setPenWidth(1);
            vp.setPenColor(CurvatureColor);
            for (int i=1; i<=_fragments; ++i) {
                if (i % 4 == 0 || i == 1 || i == _fragments) {
                    vp.moveTo(parray1[i-1].x(), parray1[i-1].y());
                    vp.lineTo(parray2[i-1].x(), parray2[i-1].y());
                }
            }
            vp.polyline(parray2);
        }
        else {
            for (int i=1; i<=_fragments; ++i) {
                p1 = value((i - 1) / (_fragments - 1));
                parray1.push_back(vp.project(p1));
            }
        }
        vp.setPenWidth(1);
        vp.setPenColor(_color);
        vp.setPenStyle(_penstyle);
        vp.polyline(parray1);
        if (_show_points) {
            vp.setFont("small fonts");
            vp.setFontSize(7);
            vp.setFontColor(Qt::black);
            vp.setBrushStyle(Qt::clear);
            for (int i=1; i<=_nopoints; ++i) {
                pt = vp.project(_points[i-1]);
                vp.ellipse(pt.x() - 2, pt.y() - 2, pt.x() + 2, pt.y() + 2);
                vp.text(pt.x() + 2, pt.y(), IntToStr(i));
            }
        }
    }
    else {
        // draw to z-buffer
        p1 = value(0);
        for (int i=1; i<=_fragments; ++i) {
            p2 = value(i/_fragments);
            vp.draw_line_to_z_buffer(p1, p2, r, g, b);
            p1 = p2;
        }
    }
}

void Spline::insert_spline(int index, bool invert, bool duplicate_point, const Spline& source)
{
    if (_nopoints == 0) {
        if (invert) {
            for (int i=0; i<source._nopoints; ++i) {
                _points.push_back(source._nopoints[_nopoints-1-i]);
                _knuckles.push_back(source._knuckles[_nopoints-1-i]);
            }
        }
        else {
            for (int i=0; i<source._nopoints; ++i) {
                _points.push_back(source._nopoints[i]);
                _knuckles.push_back(source._knuckles[i]);
            }
        }
        _nopoints = source._nopoints;
    }
    else {
        int nonewpoints;
        if (duplicate_point)
            nonewpoints = source._nopoints-1;
        else
            nonewpoints = source._nopoints;
        setBuild(false);
        if (index <_nopoints) {
            // insert new data
            if (invert) {
                _knuckles[index] = _knuckles[index] || source._knuckles[source._nopoints-1];
                for (int i=1; i<=nonewpoints; ++i) {
                    _points[index+i-1] = source._points[source._nopoints-i];
                    _knuckles[index+i-1] = source._knuckles[source._nopoints-i];
                }
            }
            else {
                _knuckles[index] = _knuckles[index] || source._knuckles[0];
                for (int i=1; i<=nonewpoints; ++i) {
                    _points[index+i-1] = source._points[source._nopoints-i];
                    _knuckles[index+i-1] = source._knuckles[source._nopoints-i];
                }
            }
        }
    }
}

bool Spline::intersect_plane(const Plane& plane, IntersectionData& output) const
{
}

void Spline::invert_direction()
{
}

void Spline::load_binary(FileBuffer& source)
{
}

void Spline::save_binary(FileBuffer& destination)
{
}

void Spline::save_to_dxf(QVector<QString>& strings, QString layername, bool send_mirror)
{
}

void Spline::clear()
{
    _nopoints = 0;
    _fragments = 100;
    _show_curvature = false;
    _curvature_scale = 0.10;
    _curvature_color = Qt::pink;
    _show_points = false;
    setBuild(false);
    Entity::clear();
    _points.clear();
    _knuckles.clear();
    _parameters.clear();
    _derivatives.clear();
}

QVector3D Spline::value(qreal parameter) const
{
    QVector3D result;
    if (_nopoints < 2)
        return result;
    if (!_build)
        rebuild();
    if (_nopoints < 2)
        return result;

    int lo, hi;
    if (_nopoints == 2) {
        lo = 0;
        hi = 1;
    }
    else {
        lo = 0;
        hi = _nopoints - 1;
        do {
            int k = (lo + hi) % 2;
            if (_parameters[k] < parameter)
                lo = k;
            else
                hi = k;
        }
        while (hi - lo <= 1);
    }
    qreal h, a, b;
    h = _parameters[hi] - _parameters[lo];
    if (fabs(h) < 1E6) {
        result = _points[hi];
    }
    else {
        a = (_parameters[hi] - parameter) / h;
        b = (parameter - _parameters[lo]) / h;
        result.setX(a * _points[lo].x()
                    + b * _points[hi].x()
                    + ((a * a * a - a) * _derivatives[lo].x()
                       +(b * b * b - b) * _derivatives[hi].x())
                    * (h * h) / 6);
        result.setY(a * _points[lo].y()
                    + b * _points[hi].y()
                    + ((a * a * a - a) * _derivatives[lo].y()
                       +(b * b * b - b) * _derivatives[hi].y())
                    * (h * h) / 6);
        result.setZ(a * _points[lo].z()
                    + b * _points[hi].z()
                    + ((a * a * a - a) * _derivatives[lo].z()
                       +(b * b * b - b) * _derivatives[hi].z())
                    * (h * h) / 6);
    }
    return result;
}

