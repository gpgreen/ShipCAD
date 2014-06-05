#include <iostream>

#include "entity.h"
#include "utility.h"

using namespace std;
using namespace ShipCADGeometry;
using namespace ShipCADUtility;

static QVector3D ZERO = QVector3D();
static QVector3D ONE = QVector3D(1,1,1);

//////////////////////////////////////////////////////////////////////////////////////

Entity::Entity()
{
    clear();
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

void Entity::dump(ostream& os) const
{
    os << " Build:" << (_build ? "true" : "false")
       << "\n Min:[" << _min.x() << "," << _min.y() << "," << _min.z()
       << "]\n Max:[" << _max.x() << "," << _max.y() << "," << _max.z()
       << "]\n PenWidth:" << _pen_width
       << "\n PenStyle:" << _pen_style
       << "\n Color:[" << _color.red() << "," << _color.green() << "," << _color.blue()
       << "]";
}

ostream& operator << (ostream& os, const ShipCADGeometry::Entity& entity)
{
    entity.dump(os);
    return os;
}
