#include "Geometry.h"

using namespace Geometry;
using namespace std;

Entity::Entity()
  : _build(false)
{
  // does nothing
}

Entity::~Entity()
{
  // does nothing
}

void Entity::create()
{
  clear();
  _build = false;
}

void Entity::clear()
{
  _build = false;
  _min = ZERO;
  _max = ZERO;
  _color = Qt::Black;
  _pen_width = 1;
  _pen_style = SOLID;
}

static void MinMax(const QVector3D& p, QVector3D& min, QVector3D& max)
{
  if (p.X<min.X) min.X=p.X;
  if (p.Y<min.Y) min.Y=p.Y;
  if (p.Z<min.Z) min.Z=p.Z;
  if (p.X>max.X) max.X=p.X;
  if (p.Y>max.Y) max.Y=p.Y;
  if (p.Z>max.Z) max.Z=p.Z;
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

bool Entity::getBuild()
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

void Spline::setCapacity(int val)
{
  if (val != _capacity) {
    _capacity = val;
    _points.reserve(val);
    if (_nopoints > _capacity) {
      _nopoints = _capacity;
      _build = false;
    }
    _knuckles.reserve(val);
  }
}

void Spline::setFragments(int val)
{
  if (val != _fragments) {
    _fragments = val;
    _build = false;
  }
}

int Spline::getFragments()
{
  return _fragments;
}

bool Spline::getKnuckle(int index)
{
  if (index >= 0 && index < _nopoints)
    return _knuckle[index];
  throw ListIndexOutOfBounds();
}

void Spline::setKnuckle(int index, bool val)
{
  if (index >= 0 && index < _nopoints) {
    _knuckles[index] = val;
    _build = false;
  } else {
    throw ListIndexOutOfBounds();
  }
}

void Spline::setPoint(int index, const QVector3D& p)
{
  if (index >= 0 && index < _nopoints) {
    _points[index] = p;
    _build = false;
  } else {
    throw PointIndexOutOfBounds();
  }
}

qreal Spline::getParameter(int index)
{
  if (index >= 0 && index < _no_points) {
    return _parameters[index];
  } else {
    throw ListIndexOutOfBounds();
  }
}

QVector3D Spline::getPoint(int index)
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
  int i = 2;
  _total_length = 0;
  qreal length;
  while (i <= _nopoints) {
    length = sqrt(DistPP3D(_points[i-2],_points[i-1]));
    _total_length += length;
    i++;
  }

  vector<QVector3D> u;
  QVector3D un, qn;
  qreal sig, p;

  if (_nopoints > 1) {
    _derivatives.reserve(_nopoints);
    _parameters.reserve(_nopoints);
    u.reserve(_nopoints);

    length = 0;
    if (fabs(_total_length) <  1e-5) {
      for (int i=1; i<_nopoints; ++i) {
	_parameters[i-1] = (i - 1) / (_nopoints - 1);
      }
    } else {
      _parameters[0] = 0;
      for (int i=2; i<_nopoints-1; ++i) {
	length += sqrt(DistPP3D(_points[i-2], _points[i-1]));
	_parameters[i-1] = length / _total_length;
      }
      _parameters[_nopoints-1] = 1.0;
    }

    _derivatives[0] = ZERO;
    u[0] = _derivatives[0];

    for (int i=2; i<_nopoints-1; ++i) {
      if (_knuckle[i-1]) {
	u[i-1] = ZERO;
	_derivatives[i-1] = ZERO;
      } else {
	if (fabs(_parameters[i] - _parameters[i-2]) < 1e-5
	    || fabs(_parameters[i-1] - _parameters[i-2]) < 1e-5
	    || fabs(_parameters[i] - _parameters[i-1]) < 1e-5) {
	  _derivatives[i-1] = ZERO;
	} else {
	  sig = (_parameters[i-1] - _parameters[i-2]) 
	    / (_parameters[i] - _parameters[i-2]);
	  // first x-value
	  p = sig * _derivatives[i-2].X + 2.0;
	  _derivatives[i-1].X = (sig - 1.0) / p;
	  u[i-1].X = (6.0 * ((_points[i].X - _points[i-1].X) 
			     / (_parameters[i] - _parameters[i-1]) 
			     - (_points[i-1].X - _points[i-2].X) 
			     / (_parameters[i-1] - _parameters[i-2])) 
		      / (_parameters[i] - _parameters[i-2]) 
		      - sig * u[i-2].X) / p;
	  // then y-value
	}
      }
    }
    
    qn = ZERO;
    un = qn;
  } // end _nopoints > 1
  _build = true;
  // determine min/max values
}
