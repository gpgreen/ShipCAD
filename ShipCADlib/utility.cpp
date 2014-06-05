#include <cmath>
#include <boost/math/constants/constants.hpp>
#include "utility.h"

using namespace std;
using namespace boost::math::float_constants;

void ShipCADUtility::MinMax(const QVector3D& p, QVector3D& min, QVector3D& max)
{
    if (p.x()<min.x()) min.setX(p.x());
    if (p.y()<min.y()) min.setY(p.y());
    if (p.z()<min.z()) min.setZ(p.z());
    if (p.x()>max.x()) max.setX(p.x());
    if (p.y()>max.y()) max.setY(p.y());
    if (p.z()>max.z()) max.setZ(p.z());
}

float ShipCADUtility::DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2)
{
    QVector3D vec = l2 - l1;
    vec.normalize();
    return p.distanceToLine(l1, vec);
}

#if 0
float DistanceToLine(const QVector2D& p1, const QVector2D& p2,
                     int x, int y, qreal& parameter)
{
    qreal result = 0;
    x = y  = 0;
    parameter = 0;
    result = p1.x();
    result = p2.x();
    return result;
}
#endif

QVector3D ShipCADUtility::Interpolate(const QVector3D& p1, const QVector3D& p2, float param)
{
    QVector3D result;
    result.setX(p1.x() + param * (p2.x() - p1.x()));
    result.setY(p1.y() + param * (p2.y() - p1.y()));
    result.setX(p1.z() + param * (p2.z() - p1.z()));
    return result;
}

static QColor DXFLayerColors[255] = {
    QColor(0x00,0x00,0xff), QColor(0x00,0xff,0xff), QColor(0x00,0xFF,0x00), QColor(0xFF,0xFF,0x00), QColor(0xFF,0x00,0x00),
    QColor(0xFF,0x00,0xFF), QColor(0x00,0x00,0x00), QColor(0x80,0x80,0x80), QColor(0xC0,0xC0,0xC0), QColor(0x00,0x00,0xFF),
    QColor(0x7F,0x7F,0xFF), QColor(0x00,0x00,0xA5), QColor(0x52,0x52,0xA5), QColor(0x00,0x00,0x7F), QColor(0x3F,0x3F,0x7F),
    QColor(0x00,0x00,0x4C), QColor(0x26,0x26,0x4C), QColor(0x00,0x00,0x26), QColor(0x13,0x13,0x26), QColor(0x00,0x3F,0xFF),
    QColor(0x7F,0x9F,0xFF), QColor(0x00,0x29,0xA5), QColor(0x52,0x67,0xA5), QColor(0x00,0x1F,0x7F), QColor(0x3F,0x4F,0x7F),
    QColor(0x00,0x13,0x4C), QColor(0x26,0x2F,0x4C), QColor(0x00,0x09,0x26), QColor(0x13,0x17,0x26), QColor(0x00,0x7F,0xFF),
    QColor(0x7F,0xBF,0xFF), QColor(0x00,0x52,0xA5), QColor(0x52,0x7C,0xA5), QColor(0x00,0x3F,0x7F), QColor(0x3F,0x5F,0x7F),
    QColor(0x00,0x26,0x4C), QColor(0x26,0x39,0x4C), QColor(0x00,0x13,0x26), QColor(0x13,0x1C,0x26), QColor(0x00,0xBF,0xFF),
    QColor(0x7F,0xDF,0xFF), QColor(0x00,0x7C,0xA5), QColor(0x52,0x91,0xA5), QColor(0x00,0x5F,0x7F), QColor(0x3F,0x6F,0x7F),
    QColor(0x00,0x39,0x4C), QColor(0x26,0x42,0x4C), QColor(0x00,0x1C,0x26), QColor(0x13,0x21,0x26), QColor(0x00,0xFF,0xFF),
    QColor(0x7F,0xFF,0xFF), QColor(0x00,0xA5,0xA5), QColor(0x52,0xA5,0xA5), QColor(0x00,0x7F,0x7F), QColor(0x3F,0x7F,0x7F),
    QColor(0x00,0x4C,0x4C), QColor(0x26,0x4C,0x4C), QColor(0x00,0x26,0x26), QColor(0x13,0x26,0x26), QColor(0x00,0xFF,0xBF),
    QColor(0x7F,0xFF,0xDF), QColor(0x00,0xA5,0x7C), QColor(0x52,0xA5,0x91), QColor(0x00,0x7F,0x5F), QColor(0x3F,0x7F,0x6F),
    QColor(0x00,0x4C,0x39), QColor(0x26,0x4C,0x42), QColor(0x00,0x26,0x1C), QColor(0x13,0x26,0x21), QColor(0x00,0xFF,0x7F),
    QColor(0x7F,0xFF,0xBF), QColor(0x00,0xA5,0x52), QColor(0x52,0xA5,0x7C), QColor(0x00,0x7F,0x3F), QColor(0x3F,0x7F,0x5F),
    QColor(0x00,0x4C,0x26), QColor(0x26,0x4C,0x39), QColor(0x00,0x26,0x13), QColor(0x13,0x26,0x1C), QColor(0x00,0xFF,0x3F),
    QColor(0x7F,0xFF,0x9F), QColor(0x00,0xA5,0x29), QColor(0x52,0xA5,0x67), QColor(0x00,0x7F,0x1F), QColor(0x3F,0x7F,0x4F),
    QColor(0x00,0x4C,0x13), QColor(0x26,0x4C,0x2F), QColor(0x00,0x26,0x09), QColor(0x13,0x26,0x17), QColor(0x00,0xFF,0x00),
    QColor(0x7F,0xFF,0x7F), QColor(0x00,0xA5,0x00), QColor(0x52,0xA5,0x52), QColor(0x00,0x7F,0x00), QColor(0x3F,0x7F,0x3F),
    QColor(0x00,0x4C,0x00), QColor(0x26,0x4C,0x26), QColor(0x00,0x26,0x00), QColor(0x13,0x26,0x13), QColor(0x3F,0xFF,0x00),
    QColor(0x9F,0xFF,0x7F), QColor(0x29,0xA5,0x00), QColor(0x67,0xA5,0x52), QColor(0x1F,0x7F,0x00), QColor(0x4F,0x7F,0x3F),
    QColor(0x13,0x4C,0x00), QColor(0x2F,0x4C,0x26), QColor(0x09,0x26,0x00), QColor(0x17,0x26,0x13), QColor(0x7F,0xFF,0x00),
    QColor(0xBF,0xFF,0x7F), QColor(0x52,0xA5,0x00), QColor(0x7C,0xA5,0x52), QColor(0x3F,0x7F,0x00), QColor(0x5F,0x7F,0x3F),
    QColor(0x26,0x4C,0x00), QColor(0x39,0x4C,0x26), QColor(0x13,0x26,0x00), QColor(0x1C,0x26,0x13), QColor(0xBF,0xFF,0x00),
    QColor(0xDF,0xFF,0x7F), QColor(0x7C,0xA5,0x00), QColor(0x91,0xA5,0x52), QColor(0x5F,0x7F,0x00), QColor(0x6F,0x7F,0x3F),
    QColor(0x39,0x4C,0x00), QColor(0x42,0x4C,0x26), QColor(0x1C,0x26,0x00), QColor(0x21,0x26,0x13), QColor(0xFF,0xFF,0x00),
    QColor(0xFF,0xFF,0x7F), QColor(0xA5,0xA5,0x00), QColor(0xA5,0xA5,0x52), QColor(0x7F,0x7F,0x00), QColor(0x7F,0x7F,0x3F),
    QColor(0x4C,0x4C,0x00), QColor(0x4C,0x4C,0x26), QColor(0x26,0x26,0x00), QColor(0x26,0x26,0x13), QColor(0xFF,0xBF,0x00),
    QColor(0xFF,0xDF,0x7F), QColor(0xA5,0x7C,0x00), QColor(0xA5,0x91,0x52), QColor(0x7F,0x5F,0x00), QColor(0x7F,0x6F,0x3F),
    QColor(0x4C,0x39,0x00), QColor(0x4C,0x42,0x26), QColor(0x26,0x1C,0x00), QColor(0x26,0x21,0x13), QColor(0xFF,0x7F,0x00),
    QColor(0xFF,0xBF,0x7F), QColor(0xA5,0x52,0x00), QColor(0xA5,0x7C,0x52), QColor(0x7F,0x3F,0x00), QColor(0x7F,0x5F,0x3F),
    QColor(0x4C,0x26,0x00), QColor(0x4C,0x39,0x26), QColor(0x26,0x13,0x00), QColor(0x26,0x1C,0x13), QColor(0xFF,0x3F,0x00),
    QColor(0xFF,0x9F,0x7F), QColor(0xA5,0x29,0x00), QColor(0xA5,0x67,0x52), QColor(0x7F,0x1F,0x00), QColor(0x7F,0x4F,0x3F),
    QColor(0x4C,0x13,0x00), QColor(0x4C,0x2F,0x26), QColor(0x26,0x09,0x00), QColor(0x26,0x17,0x13), QColor(0xFF,0x00,0x00),
    QColor(0xFF,0x7F,0x7F), QColor(0xA5,0x00,0x00), QColor(0xA5,0x52,0x52), QColor(0x7F,0x00,0x00), QColor(0x7F,0x3F,0x3F),
    QColor(0x4C,0x00,0x00), QColor(0x4C,0x26,0x26), QColor(0x26,0x00,0x00), QColor(0x26,0x13,0x13), QColor(0xFF,0x00,0x3F),
    QColor(0xFF,0x7F,0x9F), QColor(0xA5,0x00,0x29), QColor(0xA5,0x52,0x67), QColor(0x7F,0x00,0x1F), QColor(0x7F,0x3F,0x4F),
    QColor(0x4C,0x00,0x13), QColor(0x4C,0x26,0x2F), QColor(0x26,0x00,0x09), QColor(0x26,0x13,0x17), QColor(0xFF,0x00,0x7F),
    QColor(0xFF,0x7F,0xBF), QColor(0xA5,0x00,0x52), QColor(0xA5,0x52,0x7C), QColor(0x7F,0x00,0x3F), QColor(0x7F,0x3F,0x5F),
    QColor(0x4C,0x00,0x26), QColor(0x4C,0x26,0x39), QColor(0x26,0x00,0x13), QColor(0x26,0x13,0x1C), QColor(0xFF,0x00,0xBF),
    QColor(0xFF,0x7F,0xDF), QColor(0xA5,0x00,0x7C), QColor(0xA5,0x52,0x91), QColor(0x7F,0x00,0x5F), QColor(0x7F,0x3F,0x6F),
    QColor(0x4C,0x00,0x39), QColor(0x4C,0x26,0x42), QColor(0x26,0x00,0x1C), QColor(0x26,0x13,0x21), QColor(0xFF,0x00,0xFF),
    QColor(0xFF,0x7F,0xFF), QColor(0xA5,0x00,0xA5), QColor(0xA5,0x52,0xA5), QColor(0x7F,0x00,0x7F), QColor(0x7F,0x3F,0x7F),
    QColor(0x4C,0x00,0x4C), QColor(0x4C,0x26,0x4C), QColor(0x26,0x00,0x26), QColor(0x26,0x13,0x26), QColor(0xBF,0x00,0xFF),
    QColor(0xDF,0x7F,0xFF), QColor(0x7C,0x00,0xA5), QColor(0x91,0x52,0xA5), QColor(0x5F,0x00,0x7F), QColor(0x6F,0x3F,0x7F),
    QColor(0x39,0x00,0x4C), QColor(0x42,0x26,0x4C), QColor(0x1C,0x00,0x26), QColor(0x21,0x13,0x26), QColor(0x7F,0x00,0xFF),
    QColor(0xBF,0x7F,0xFF), QColor(0x52,0x00,0xA5), QColor(0x7C,0x52,0xA5), QColor(0x3F,0x00,0x7F), QColor(0x5F,0x3F,0x7F),
    QColor(0x26,0x00,0x4C), QColor(0x39,0x26,0x4C), QColor(0x13,0x00,0x26), QColor(0x1C,0x13,0x26), QColor(0x3F,0x00,0xFF),
    QColor(0x9F,0x7F,0xFF), QColor(0x29,0x00,0xA5), QColor(0x67,0x52,0xA5), QColor(0x1F,0x00,0x7F), QColor(0x4F,0x3F,0x7F),
    QColor(0x13,0x00,0x4C), QColor(0x2F,0x26,0x4C), QColor(0x09,0x00,0x26), QColor(0x17,0x13,0x26), QColor(0x00,0x00,0x00),
    QColor(0x2D,0x2D,0x2D), QColor(0x5B,0x5B,0x5B), QColor(0x89,0x89,0x89), QColor(0xB7,0xB7,0xB7), QColor(0xB3,0xB3,0xB3)
};

int ShipCADUtility::FindDXFColorIndex(QColor color)
{
    int result = 1;
    int r = color.red();
    int g = color.green();
    int b = color.blue();
    float dist = 1E30f;
    for (int i=0; i<255; ++i) {
        int r1 = DXFLayerColors[i].red();
        int g1 = DXFLayerColors[i].green();
        int b1 = DXFLayerColors[i].blue();
        float tmp = sqrt(static_cast<float>((r1-r)*(r1-r) + (g1-g)*(g1-g) + (b1-b)*(b1-b)));
        if (tmp < dist) {
            dist = tmp;
            result = i+1;
        }
    }
    return result;
}

QString ShipCADUtility::truncate(float val, int max_length)
{
    QString num;
    num.setNum(val);
    int decimal = num.lastIndexOf('.');
    num.truncate(decimal + max_length + 1);
    return num;
}

QVector3D ShipCADUtility::UnifiedNormal(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    QVector3D result((p2.y()-p1.y())*(p3.z()-p1.z())-(p2.z()-p1.z())*(p3.y()-p1.y()),
                     (p2.z()-p1.z())*(p3.x()-p1.x())-(p2.x()-p1.x())*(p3.z()-p1.z()),
                     (p2.x()-p1.x())*(p3.y()-p1.y())-(p2.y()-p1.y())*(p3.x()-p1.x()));
    float l = result.length();
    if (l < 1E-6)
        l = 1E-6f;
    result.setX(result.x() / l);
    result.setY(result.y() / l);
    result.setZ(result.z() / l);
    return result;
}

extern float ShipCADUtility::RadToDeg(float rad)
{
  return rad * 180.0 / pi;
}
