#include <cmath>
#include <algorithm>
#include <iostream>

#include "spline.h"
#include "plane.h"
#include "viewport.h"
#include "filebuffer.h"
#include "utility.h"
#include "shader.h"
#include "exception.h"

using namespace ShipCADGeometry;
using namespace ShipCADUtility;
using namespace ShipCADException;
using namespace std;

static QVector3D ZERO = QVector3D();
static QVector3D ONE = QVector3D(1,1,1);

//////////////////////////////////////////////////////////////////////////////////////

Spline::Spline()
    : Entity()
{
    clear();
}

Spline::~Spline()
{
    // does nothing
}

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

void Spline::setFragments(size_t val)
{
    if (val != _fragments) {
        _fragments = val;
        setBuild(false);
    }
}

int Spline::getFragments()
{
    return _fragments;
}

bool Spline::isKnuckle(size_t index)
{
    if (index < _nopoints)
        return _knuckles[index];
    throw ListIndexOutOfBounds(__FILE__);
}

void Spline::setKnuckle(size_t index, bool val)
{
    if (index < _nopoints) {
        _knuckles[index] = val;
        setBuild(false);
    } 
    else {
        throw ListIndexOutOfBounds(__FILE__);
    }
}

void Spline::setPoint(size_t index, const QVector3D& p)
{
    if (index < _nopoints) {
        _points[index] = p;
        setBuild(false);
    } 
    else {
        throw PointIndexOutOfBounds(__FILE__);
    }
}

float Spline::getParameter(size_t index)
{
    if (index < _nopoints) {
        return _parameters[index];
    } 
    else {
        throw ListIndexOutOfBounds(__FILE__);
    }
}

QVector3D Spline::getPoint(size_t index)
{
    if (index < _nopoints) {
        return _points[index];
    } 
    else {
        throw PointIndexOutOfBounds(__FILE__);
    }
}

void Spline::rebuild()
{
    _build = false;

    // attempt to eliminate double points
    _total_length = 0;
    for (size_t i=1; i<_nopoints; ++i) {
        _total_length += sqrt(_points[i].distanceToPoint(_points[i-1]));
    }
    vector<QVector3D> u;
    QVector3D un, qn;
    float sig, p;

    if (_nopoints > 1) {
        _derivatives.clear();
        _derivatives.reserve(_nopoints);
        _parameters.clear();
        _parameters.reserve(_nopoints);
        u.reserve(_nopoints);

        if (fabs(_total_length) <  1E-5) {
            for (size_t i=0; i<_nopoints; ++i) {
                _parameters.push_back(i / static_cast<float>(_nopoints));
            }
        } 
        else {
            _parameters.push_back(0);
            float length = 0;
            for (size_t i=1; i<_nopoints-1; ++i) {
                length += sqrt(_points[i-1].distanceToPoint(_points[i]));
                _parameters.push_back(length / _total_length);
            }
            _parameters.push_back(1.0);
        }

        _derivatives.push_back(ZERO);
        u.push_back(_derivatives[0]);

        for (size_t i=1; i<_nopoints-1; ++i) {
            if (_knuckles[i]) {
                u[i-1] = ZERO;
                _derivatives.push_back(ZERO);
            } 
            else {
                if (fabs(_parameters[i+1] - _parameters[i-1]) < 1e-5
                        || fabs(_parameters[i] - _parameters[i-1]) < 1e-5
                        || fabs(_parameters[i+1] - _parameters[i]) < 1e-5) {
                    _derivatives.push_back(ZERO);
                } 
                else {
                    sig = (_parameters[i] - _parameters[i-1])
                            / (_parameters[i+1] - _parameters[i-1]);
                    // first x-value
                    p = sig * _derivatives[i-1].x() + 2.0;
                    QVector3D p1;
                    QVector3D p2;
                    p1.setX((sig - 1.0) / p);
                    p2.setX( (6.0 * ((_points[i+1].x() - _points[i].x())
                              / (_parameters[i+1] - _parameters[i])
                             - (_points[i].x() - _points[i-1].x())
                            / (_parameters[i] - _parameters[i-1]))
                            / (_parameters[i+1] - _parameters[i-1])
                            - sig * u[i-1].x()) / p);
                    // then y-value
                    p = sig * _derivatives[i-1].y() + 2.0;
                    p1.setY((sig - 1.0) / p);
                    p2.setY( (6.0 * ((_points[i+1].y() - _points[i].y())
                              / (_parameters[i+1] - _parameters[i])
                             - (_points[i].y() - _points[i-1].y())
                            / (_parameters[i] - _parameters[i-1]))
                            / (_parameters[i+1] - _parameters[i-1])
                            - sig * u[i-1].y()) / p);
                    // then z-value
                    p = sig * _derivatives[i-1].z() + 2.0;
                    p1.setZ((sig - 1.0) / p);
                    p2.setZ( (6.0 * ((_points[i+1].z() - _points[i].z())
                              / (_parameters[i+1] - _parameters[i])
                             - (_points[i].z() - _points[i-1].z())
                            / (_parameters[i] - _parameters[i-1]))
                            / (_parameters[i+1] - _parameters[i-1])
                            - sig * u[i-1].z()) / p);
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
            _derivatives[k-1] = _derivatives[k-1] * _derivatives[k] + u[k-1];
        }
    } // end _nopoints > 1
    _build = true;
    // determine min/max values
    if (_nopoints > 0) {
        for (size_t i=0; i<_nopoints; ++i) {
            if (i == 0) {
                _min = _points[i];
                _max = _min;
            }
            else {
                MinMax(_points[i], _min, _max);
            }
        }
    }
}

QVector3D Spline::second_derive(float parameter)
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
        } while (hi - lo > 1);
    }
    float frac;
    if (_parameters[hi] - _parameters[lo] <= 0.0)
        frac = 0.5;
    else
        frac = (parameter - _parameters[lo]) / (_parameters[hi] - _parameters[lo]);
    //result.setX(_derivatives[lo].x() + frac * (_derivatives[hi].x() - _derivatives[lo].x()));
    //result.setY(_derivatives[lo].y() + frac * (_derivatives[hi].y() - _derivatives[lo].y()));
    //result.setZ(_derivatives[lo].z() + frac * (_derivatives[hi].z() - _derivatives[lo].z()));
    result = _derivatives[lo] + (frac * (_derivatives[hi] - _derivatives[lo]));
    return result;
}

float Spline::weight(size_t index)
{
    float result;
    float length, dist;

    if (index == 0 || (index == _nopoints - 1) || _knuckles[index]) {
        result = 1E10;
    }
    else {
        QVector3D p1 = _points[index-1];
        QVector3D p2 = _points[index];
        QVector3D p3 = _points[index+1];
        //length = sqrt((p3.x()-p1.x())*(p3.x()-p1.x())+(p3.y()-p1.y())*(p3.y()-p1.y())+(p3.z()-p1.z())*(p3.z()-p1.z()));
        length = (p3 - p1).length();
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

// get the next point index with the minimum weight
// will not return the first point index, only in the range from second to end
vector<float>::iterator Spline::find_next_point(vector<float>& weights)
{
    float minval;
    vector<float>::iterator result = weights.end();
    vector<float>::iterator i = weights.begin();
    if (_nopoints < 3)
        return result;
    minval = weights[1];
    i++;
    result = i;
    while (i<weights.end() && minval > 0) {
        if (*i < minval) {
	    minval = *i;
            result = i;
        }
        i++;
    }
    return result;
}

bool Spline::simplify(float criterium)
{
  vector<float> weights;
  weights.reserve(_nopoints);
    bool result = false;

    if (_nopoints < 3)
        return true;

    float total_length = _total_length * _total_length;
    if (total_length == 0)
        return result;

    for (size_t i=0; i<_nopoints; ++i)
        weights.push_back(weight(i)/total_length);

    vector<float>::iterator index;
    do {
        index = find_next_point(weights);
	vector<float>::iterator last = weights.end();
	last--;
        if (index == weights.end() || _nopoints < 3 || index == last)
	  break;
	if (*index < criterium) {
	  size_t j = index - weights.begin();
	  weights.erase(index);
	  _points.erase(_points.begin()+j);
	  _knuckles.erase(_knuckles.begin()+j);
	  _nopoints--;
	  if (j > 0 && j < _nopoints)
	    weights[j] = weights[j]/total_length;
	}
	else
	  break;
    }
    while (index != weights.end());
    result = true;

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

float Spline::coord_length(float t1, float t2)
{
    float result = 0.0;
    if (!_build)
        rebuild();
    if (!_build)
        return result;

    QVector3D p1, p2;
    p1 = ZERO;
    for (size_t i=0; i<=_fragments; ++i) {
        float t = t1 + (i / static_cast<float>(_fragments)) * (t2 - t1);
        p2 = value(t);
        if (i > 0)
            //result += sqrt((p2.x() - p1.x()) * (p2.x() - p1.x())
            //    + ((p2.y() - p1.y()) * (p2.y() - p1.y()))
            //    + ((p2.z() - p1.z()) * (p2.z() - p1.z())));
            result += (p2 - p1).length();
        p1 = p2;
    }
    return result;
}

float Spline::chord_length_approximation(float percentage)
{
    float result, length, total_length;
    float t1, t2;
    t1 = 0;
    t2 = 1.0;
    float parameter = 1.0;
    int counter = 0;
    float l1 = 0;
    float l2 = 0;
    float desired_length = 0;
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
        while (counter <= 75 && fabs(length - desired_length) >= 1E-3);
        result = parameter;
    }
    return result;
}

float Spline::curvature(float parameter, QVector3D& normal)
{
    float result;

    QVector3D vel1 = first_derive(parameter);
    QVector3D acc = second_derive(parameter);

    QVector3D crossproduct = QVector3D::crossProduct(acc, vel1);
    float l = crossproduct.length();
    if (l == 0)
        l = 0.00001f;
    float vdota = (vel1.x()*acc.x()) + (vel1.y()*acc.y()) + (vel1.z()*acc.z());
    float vdotv = (vel1.x()*vel1.x()) + (vel1.y()*vel1.y()) + (vel1.z()*vel1.z());
    if (vdotv < 0)
        result = 0;
    else
        result = l/pow(vdotv, static_cast<float>(1.5));
    //normal.setX(vdotv*acc.x()-vdota*vel1.x());
    //normal.setY(vdotv*acc.y()-vdota*vel1.y());
    //normal.setZ(vdotv*acc.z()-vdota*vel1.z());
    normal = (vdotv * acc) - (vdota * vel1);
    normal.normalize();
    return result;
}

void Spline::delete_point(size_t index)
{
    if (_nopoints > 0) {
        _nopoints--;
        _points.erase(_points.begin()+index);
        _knuckles.erase(_knuckles.begin()+index);
        setBuild(false);
    }
}

QVector3D Spline::first_derive(float parameter)
{

    float t1 = parameter - 1E-3;
    float t2 = parameter + 1E-3;
    if (t1 < 0)
        t1 = 0;
    if (t2 > 1)
        t2 = 1;
    QVector3D p1 = value(t1);
    QVector3D p2 = value(t2);

    //QVector3D result((p2.x() - p1.x()) / (t2 - t1),
    //                  (p2.y() - p1.y()) / (t2 - t1),
    //                  (p2.z() - p1.z()) / (t2 - t1));
    QVector3D result = (p2 - p1) / (t2 - t1);
    return result;
}

void Spline::insert(size_t index, const QVector3D& p)
{
    if (index < _nopoints) {
        _points.insert(_points.begin()+index, p);
        _knuckles.insert(_knuckles.begin()+index, false);
        _nopoints++;
        setBuild(false);
    }
    else
        throw IndexOutOfRange(__FILE__);
}

void Spline::draw(Viewport& vp, LineShader* lineshader)
{
    if (!_build)
        rebuild();

    QVector3D p1;
    QVector3D p2;
    QVector3D normal;
    vector<QVector3D> parray1;
    vector<QVector3D> parray2;
    QVector<QVector3D> vertices;

    for (size_t i=0; i<_fragments; ++i)
        parray1.push_back(value(i/static_cast<float>(_fragments)));
    if (vp.getViewportMode() == Viewport::vmWireFrame) {
        if (_show_curvature) {
            glLineWidth(1);
            for (size_t i=0; i<_fragments; ++i) {
                float c = curvature(i / static_cast<float>(_fragments), normal);
                //p2.setX(parray1[i].x() - c * 2 * _curvature_scale * normal.x());
                //p2.setY(parray1[i].y() - c * 2 * _curvature_scale * normal.y());
                //p2.setZ(parray1[i].z() - c * 2 * _curvature_scale * normal.z());
                p2 = parray1[i] - (c * 2 * _curvature_scale * normal);
                parray2.push_back(p2);
            }
            for (size_t i=1; i<=_fragments; ++i) {
                if (i % 4 == 0 || i == 1 || i == _fragments) {
                    vertices << parray1[i-1];
                    vertices << parray2[i-1];
                }
            }
            for (size_t i=1; i<parray2.size(); ++i) {
                vertices << parray2[i-1];
                vertices << parray2[i];
            }
            lineshader->renderLines(vertices, _curvature_color);
        }
        if (_show_points) {
            //vp.setFont("small fonts");
            //vp.setFontSize(7);
            //vp.setFontColor(Qt::black);
            //vp.setBrushStyle(Qt::clear);
            //vp.setColor(Qt::white);
            vertices.clear();
            for (size_t i=0; i<_nopoints; ++i) {
                vertices << _points[i];
                //vp.text(pt.x() + 2, pt.y(), IntToStr(i));
            }
            lineshader->renderPoints(vertices, QColor(Qt::white));
        }
    }
    vertices.clear();
    glLineWidth(1);
    for (size_t i=1; i<parray1.size(); ++i) {
        vertices << parray1[i-1];
        vertices << parray1[i];
    }
    lineshader->renderLines(vertices, _color);
}

void Spline::insert_spline(size_t index, bool invert, bool duplicate_point, 
			   const Spline& source)
{
    setBuild(false);
    int nonewpoints;
    if (duplicate_point)
        nonewpoints = source._nopoints-1;
    else
        nonewpoints = source._nopoints;
    if (_nopoints == 0)
        index = 0;
    _points.insert(_points.begin()+index, nonewpoints, ZERO);
    _knuckles.insert(_knuckles.begin()+index, nonewpoints, false);
    if (invert) {
        for (int i=0; i<nonewpoints; ++i) {
            _points[i+index] = source._points[nonewpoints-i-1];
            _knuckles[i+index] = source._knuckles[nonewpoints-i-1];
        }
    }
    else {
        for (int i=0; i<nonewpoints; ++i) {
            _points[i+index] = source._points[i];
            _knuckles[i+index] = source._knuckles[i];
        }
    }
    _nopoints += nonewpoints;
}

void Spline::add_to_output(const QVector3D& p, float parameter, IntersectionData& output)
{
    output._number_of_intersections++;
    output._points.push_back(p);
    output._parameters.push_back(parameter);
}

bool Spline::intersect_plane(const Plane& plane, IntersectionData& output)
{
    output._number_of_intersections = 0;
    float t1 = 0;
    QVector3D p1 = value(t1);
    float s1 = plane.distance(p1);
    if (fabs(s1) < 1E-6)
        add_to_output(p1, t1, output);
    for (size_t i=1; i<=_fragments; ++i) {
        float t2 = i / static_cast<float>(_fragments);
        QVector3D p2 = value(t2);
        float s2 = plane.distance(p2);
        if (fabs(s2) < 1E-6)
            add_to_output(p2, t2, output);
        if ((s1 < 0 && s2 > 0) || (s2 < 0 && s1 > 0)) {
            // intersection found
            float t = s1 / (s2 - s1);
            t = t1 + t * (t2 - t1);
            add_to_output(value(t), t, output);
        }
        p1 = p2;
        s1 = s2;
        t1 = t2;
    }
    return output._number_of_intersections > 0;
}

void Spline::invert_direction()
{
    int mid = _nopoints / 2;
    for (int i=0; i<mid; ++i) {
        swap(_points[i], _points[_nopoints - i - 1]);
        bool tmp = _knuckles[i];
        _knuckles[i] = _knuckles[_nopoints - i - 1];
        _knuckles[_nopoints - i - 1] = tmp;
    }
}

void Spline::load_binary(FileBuffer& source)
{
    source.load(_show_curvature);
    source.load(_curvature_scale);
    int n;
    source.load(n);
    for (int i=0; i<n; ++i) {
        QVector3D p;
        source.load(p);
        add(p);
        bool k;
        source.load(k);
        _knuckles[i] = k;
    }
}

void Spline::save_binary(FileBuffer& destination)
{
    destination.add(_show_curvature);
    destination.add(_curvature_scale);
    destination.add(_nopoints);
    for (size_t i=0; i<_nopoints; ++i) {
        destination.add(_points[i]);
        destination.add(_knuckles[i]);
    }
}

void Spline::save_to_dxf(vector<QString>& strings, QString layername, bool send_mirror)
{
    int ind = FindDXFColorIndex(_color);
    if (!_build)
        rebuild();
    vector<float> params;
    for (size_t i=2; i<_nopoints; ++i) {
        if (_knuckles[i-1]) {
            params.push_back(_parameters[i-1]);
        }
    }
    for (size_t i=1; i<=_fragments; ++i)
        params.push_back((i-1)/static_cast<float>(_fragments-1));
    sort(params.begin(), params.end());

    strings.push_back("0\r\nPOLYLINE");
    strings.push_back(QString("8\r\n%1").arg(layername));   // layername
    strings.push_back(QString("62\r\n%1").arg(ind));      // color by layer
    strings.push_back("70\r\n10");    // not closed
    strings.push_back("66\r\n1");     // vertices follow
    for (size_t i=0; i<params.size(); ++i) {
        QVector3D p = value(params[i]);
        strings.push_back("0\r\nVERTEX");
        strings.push_back(QString("8\r\n%1").arg(layername));
        strings.push_back(QString("10\r\n%1").arg(ShipCADUtility::truncate(p.x(), 4)));
        strings.push_back(QString("20\r\n%1").arg(ShipCADUtility::truncate(p.y(), 4)));
        strings.push_back(QString("30\r\n%1").arg(ShipCADUtility::truncate(p.z(), 4)));
        strings.push_back("70\r\n32");    // 3D polyline mesh vertex
    }
    strings.push_back("0\r\nSEQEND");
    if (send_mirror) {
      // send the starboard side too
      strings.push_back("0\r\nPOLYLINE");
      strings.push_back(QString("8\r\n%1").arg(layername));   // layername
      strings.push_back(QString("62\r\n%1").arg(ind));      // color by layer
      strings.push_back("70\r\n10");    // not closed
      strings.push_back("66\r\n1");     // vertices follow
      for (size_t i=0; i<params.size(); ++i) {
        QVector3D p = value(params[i]);
        strings.push_back("0\r\nVERTEX");
        strings.push_back(QString("8\r\n%1").arg(layername));
        strings.push_back(QString("10\r\n%1").arg(ShipCADUtility::truncate(p.x(), 4)));
        strings.push_back(QString("20\r\n%1").arg(ShipCADUtility::truncate(-p.y(), 4)));
        strings.push_back(QString("30\r\n%1").arg(ShipCADUtility::truncate(p.z(), 4)));
        strings.push_back("70\r\n32");    // 3D polyline mesh vertex
      }
      strings.push_back("0\r\nSEQEND");
    }
}

void Spline::clear()
{
    _nopoints = 0;
    _fragments = 100;
    _show_curvature = false;
    _curvature_scale = 0.10f;
    _curvature_color = Qt::magenta;
    _show_points = false;
    setBuild(false);
    Entity::clear();
    _points.clear();
    _knuckles.clear();
    _parameters.clear();
    _derivatives.clear();
}

QVector3D Spline::value(float parameter)
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
            int k = (lo + hi) / 2;
            if (_parameters[k] < parameter)
                lo = k;
            else
                hi = k;
        }
        while (hi - lo > 1);
    }
    float h, a, b;
    h = _parameters[hi] - _parameters[lo];
    if (fabs(h) < 1E-6) {
        result = _points[hi];
    }
    else {
        a = (_parameters[hi] - parameter) / h;
        //b = (parameter - _parameters[lo]) / h;
        b = 1 - a;
//        result.setX(a * _points[lo].x()
//                    + b * _points[hi].x()
//                    + ((a * a * a - a) * _derivatives[lo].x()
//                       +(b * b * b - b) * _derivatives[hi].x())
//                    * (h * h) / 6.0);
//        result.setY(a * _points[lo].y()
//                    + b * _points[hi].y()
//                    + ((a * a * a - a) * _derivatives[lo].y()
//                       +(b * b * b - b) * _derivatives[hi].y())
//                    * (h * h) / 6.0);
//        result.setZ(a * _points[lo].z()
//                    + b * _points[hi].z()
//                    + ((a * a * a - a) * _derivatives[lo].z()
//                       +(b * b * b - b) * _derivatives[hi].z())
//                    * (h * h) / 6.0);
        result = a * _points[lo] + b * _points[hi] + ((a * a * a - a) * _derivatives[lo]
                + (b * b * b - b) * _derivatives[hi])
                * (h * h) / 6.0;
    }
    return result;
}

void Spline::dump(ostream& os) const
{
    os << "Spline nopoints=" << _nopoints << "\n";
    Entity::dump(os);
    os << "\n Fragments:" << _fragments
       << "\n ShowCurvature:" << (_show_curvature ? "true" : "false")
       << "\n ShowPoints:" << (_show_points ? "true" : "false")
       << "\n CurvatureScale:" << _curvature_scale
       << "\n CurvatureColor:[" << _curvature_color.red()
       << "," << _curvature_color.green()
       << "," << _curvature_color.blue()
       << "]\n TotalLength:" << _total_length
       << "\n Data (p, k, param, deriv)\n =========================\n";
    for (size_t i=0; i<_points.size(); ++i) {
        os << " [" << _points[i].x() << "," << _points[i].y() << "," << _points[i].z()
           << "]\t" << (_knuckles[i] ? 't' : 'f');
        if (_parameters.size() > i && _derivatives.size() > i) {
            os << "\t" << _parameters[i]
                  << "\t[" << _derivatives[i].x()
                  << "," << _derivatives[i].y()
                  << "," << _derivatives[i].z()
                  << "]";
        }
        os << endl;
    }
}

ostream& operator << (ostream& os, const ShipCADGeometry::Spline& spline)
{
    spline.dump(os);
    return os;
}
