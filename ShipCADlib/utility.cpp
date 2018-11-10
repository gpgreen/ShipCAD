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

#include <cmath>
#include <climits>
#include <algorithm>
#include <boost/math/constants/constants.hpp>
#include "utility.h"
#include "shipcadlib.h"

using namespace std;
using namespace ShipCAD;
using namespace boost::math::float_constants;

void ShipCAD::MinMax(const QVector3D& p, QVector3D& min, QVector3D& max)
{
    if (p.x()<min.x()) min.setX(p.x());
    if (p.y()<min.y()) min.setY(p.y());
    if (p.z()<min.z()) min.setZ(p.z());
    if (p.x()>max.x()) max.setX(p.x());
    if (p.y()>max.y()) max.setY(p.y());
    if (p.z()>max.z()) max.setZ(p.z());
}

float ShipCAD::DistancepointToLine(const QVector3D& p, const QVector3D& l1, const QVector3D& l2)
{
    QVector3D vec = l2 - l1;
    vec.normalize();
    return p.distanceToLine(l1, vec);
}

QVector3D ShipCAD::Interpolate(const QVector3D& p1, const QVector3D& p2, float param)
{
    QVector3D result = p1 + (param * (p2 - p1));
    return result;
}

QVector3D ShipCAD::MidPoint(const QVector3D& p1, const QVector3D& p2)
{
    return 0.5 * (p1 + p2);
}

QVector3D ShipCAD::PointProjectToLine(const QVector3D& p1, const QVector3D& p2, const QVector3D& p)
{
    QVector3D ab = p2 - p1;
    QVector3D ap = p - p1;
    float ab_dot = QVector3D::dotProduct(ab, ab);
    if (ab_dot == 0)
        return p;
    QVector3D result(p1);
    return result + ((QVector3D::dotProduct(ap, ab) / ab_dot) * ab);
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

int ShipCAD::FindDXFColorIndex(QColor color)
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

QColor ShipCAD::QColorFromDXFIndex(int index)
{
    if (index >= 0 && index <= 255)
        return DXFLayerColors[index];
    throw out_of_range("bad index in FindColorFromDXFIndex");
}

QColor ShipCAD::FillColor(float parameter)
{
    float red, green, blue;
    
    parameter = 1.0 - parameter;
    float wavelength = 420 + parameter * 280;
    float ftrunc_wavelength;
    modf(wavelength, &ftrunc_wavelength);
    int trunc_wavelength = static_cast<int>(ftrunc_wavelength);
    if (trunc_wavelength >= 380 && trunc_wavelength <= 439) {
        red = -(wavelength - 440.0) / (440.0 - 380.0);
        green = 0.0;
        blue = 1.0;
    }
    else if (trunc_wavelength >= 440 && trunc_wavelength <= 489) {
        red = 0.0;
        green = (wavelength - 440.0) / (490.0 - 440.0);
        blue = 1.0;
    }
    else if (trunc_wavelength >= 490 && trunc_wavelength <= 509) {
        red = 0.0;
        green = 1.0;
        blue = -(wavelength - 510.0) / (510.0 - 490.0);
    }
    else if (trunc_wavelength >= 510 && trunc_wavelength <= 579) {
        red = (wavelength - 510.0) / (580.0 - 510.0);
        green = 1.0;
        blue = 0.0;
    }
    else if (trunc_wavelength >= 580 && trunc_wavelength <= 644) {
        red = 1.0;
        green = -(wavelength - 645.0) / (645.0 - 580.0);
        blue = 0.0;
    }
    else if (trunc_wavelength >= 645 && trunc_wavelength <= 780) {
        red = 1.0;
        green = 0.0;
        blue = 0.0;
    }
    else {
        red = 0.0;
        green = 0.0;
        blue = 0.0;
    }
    
    if (red < 0.0)
        red = 0;
    else if (red > 1.0)
        red = 1.0;
    if (green < 0.0)
        green = 0;
    else if (green > 1.0)
        green = 1.0;
    if (blue < 0.0)
        blue = 0;
    else if (blue > 1.0)
        blue = 1.0;
    QColor col;
    col.setRedF(red);
    col.setGreenF(green);
    col.setBlueF(blue);
    return col;
}

QString ShipCAD::Truncate(float val, int max_length)
{
    QString num;
    num.setNum(val);
    int decimal = num.lastIndexOf('.');
    num.truncate(decimal + max_length + 1);
    return num;
}

QVector3D ShipCAD::UnifiedNormal(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3)
{
    QVector3D result((p2.y()-p1.y())*(p3.z()-p1.z())-(p2.z()-p1.z())*(p3.y()-p1.y()),
                     (p2.z()-p1.z())*(p3.x()-p1.x())-(p2.x()-p1.x())*(p3.z()-p1.z()),
                     (p2.x()-p1.x())*(p3.y()-p1.y())-(p2.y()-p1.y())*(p3.x()-p1.x()));
    result.normalize();
    return result;

}

extern float ShipCAD::DegToRad(float deg)
{
    return deg * pi / 180.0;
}

extern float ShipCAD::RadToDeg(float rad)
{
    return rad * 180.0 / pi;
}

static bool SameSide(const QVector3D& p1,
                     const QVector3D& p2,
                     const QVector3D& a,
                     const QVector3D& b)
{
    QVector3D ba = b - a;
    QVector3D p = p1 - a;
    QVector3D cp1 = QVector3D::crossProduct(p, ba);
    p = p2 - a;
    QVector3D cp2 = QVector3D::crossProduct(p, ba);
    float dp = QVector3D::dotProduct(cp1, cp2);
    if (fabs(dp) < 1E5f)
        dp = dp - 1 + 1;
    return dp >= 0;
}

bool ShipCAD::PointInTriangle(const QVector3D& intercept,
                                     const QVector3D& p0,
                                     const QVector3D& p1,
                                     const QVector3D& p2)
{
    return (SameSide(intercept, p0, p1, p2) && SameSide(intercept, p1, p0, p2)
            && SameSide(intercept, p2, p0, p1));
}

// clip a triangle given the 3 distances from a plane, 
// returns 2 sets of coordinates, front of the plane, and back of the plane
void ShipCAD::ClipTriangle(const QVector3D& p1,
                                  const QVector3D& p2,
                                  const QVector3D& p3,
                                  float s1,
                                  float s2,
                                  float s3,
                                  std::vector<QVector3D>& front,
                                  std::vector<QVector3D>& back)
{
    if (s1 <=0 && s2 <= 0 && s3 <= 0) {
        // all at the back of the plane
        back.push_back(p1);
        back.push_back(p2);
        back.push_back(p3);
    }
    else if (s1 > 0 && s2 > 0 && s3 > 0) {
        // all at the front of the plane
        front.push_back(p1);
        front.push_back(p2);
        front.push_back(p3);
    }
    else {
        // triangle spans the plane, calculate the intersections
        if (s1 <= 0)
            back.push_back(p1);
        if (s1 >= 0)
            front.push_back(p1);
        if ((s1 < 0 && s2 > 0) || (s1 > 0 && s2 < 0)) {
            float t;
            if (s1 == s2)
                t = 0.5;
            else
                t = -s1 / (s2 - s1);
            QVector3D p = p1 + (t * (p2 - p1));
            back.push_back(p);
            front.push_back(p);
        }
        if (s2 <= 0)
            back.push_back(p2);
        if (s2 >= 0)
            front.push_back(p2);
        if ((s2 < 0 && s3 > 0) || (s2 > 0 && s3 < 0)) {
            float t;
            if (s2 == s3)
                t = 0.5;
            else
                t = -s2 / (s3 - s2);
            QVector3D p = p2 + (t * (p3 - p2));
            back.push_back(p);
            front.push_back(p);
        }
        if (s3 <= 0)
            back.push_back(p3);
        if (s3 >= 0)
            front.push_back(p3);
        if ((s3 < 0 && s1 > 0) || (s3 > 0 && s1 < 0)) {
            float t;
            if (s2 == s3)
                t = 0.5;
            else
                t = -s3 / (s1 - s3);
            QVector3D p = p3 + (t * (p1 - p3));
            back.push_back(p);
            front.push_back(p);
        }
    }
}

// clip a triangle given a plane, returns 2 sets of coordinates, front of the plane, and
// back of the plane
void ShipCAD::ClipTriangle(const QVector3D& p1,
                                  const QVector3D& p2,
                                  const QVector3D& p3,
                                  const Plane& plane,
                                  std::vector<QVector3D>& front,
                                  std::vector<QVector3D>& back)
{
    float s1 = plane.distance(p1);
    float s2 = plane.distance(p2);
    float s3 = plane.distance(p3);
    ClipTriangle(p1, p2, p3, s1, s2, s3, front, back);
}

float ShipCAD::SquaredDistPP(const QVector3D& p1, const QVector3D& p2)
{
    QVector3D p21 = p2 - p1;
    return p21.lengthSquared();
}

float ShipCAD::DistPP3D(const QVector3D& p1, const QVector3D& p2)
{
    QVector3D p21 = p2 - p1;
    return p21.length();
}

bool ShipCAD::Lines3DIntersect(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3,
                               const QVector3D& p4, double& param, QVector3D& int1)
{
    bool result = false;
    param = 0;
    int1 = ZERO;
    QVector3D p13 = p1 - p3;
    QVector3D p43 = p4 - p3;
    if (abs(p43.x()) < 1E-6 && abs(p43.y()) < 1E-6 && abs(p43.z()) < 1E-6)
        return result;
    QVector3D p21 = p2 - p1;
    if (abs(p21.x()) < 1E-6 && abs(p21.y()) < 1E-6 && abs(p21.z()) < 1E-6)
        return result;
    double d1343 = p13.x() * p43.x() + p13.y() * p43.y() + p13.z() * p43.z();
    double d4321 = p43.x() * p21.x() + p43.y() * p21.y() + p43.z() * p21.z();
    double d1321 = p13.x() * p21.x() + p13.y() * p21.y() + p13.z() * p21.z();
    double d4343 = p43.x() * p43.x() + p43.y() * p43.y() + p43.z() * p43.z();
    double d2121 = p21.x() * p21.x() + p21.y() * p21.y() + p21.z() * p21.z();

    double denom = d2121 * d4343 - d4321 * d4321;
    if (abs(denom) < 1E-6)
        return result;
    double numer = d1343 * d4321 - d1321 * d4343;
    double mua = numer / denom;
    double mub = (d1343 + d4321 * mua) / d4343;
    QVector3D pa = p1 + mua * p21;
    QVector3D pb = p3 + mub * p43;
    if (mua > -1E-6 && mua < 1 && mub > -1E-6 && mub < 1) {
        result = true;
        int1 = pa;
        param = mua;
    }
    return result;
}

QString ShipCAD::BoolToStr(bool val)
{
    return QString( (val ? "1" : "0") );
}

static float Minimum(float d1, float d2, float d3, float d4)
{
    auto result = minmax({d1, d2, d3, d4});
    return result.first;
}

// This procedure takes a lot of linesegments and tries to connect them into as few as possible splines
void ShipCAD::JoinSplineSegments(float join_error,
                                        bool force_to_one_segment,
                                        SplineVector& list)
{
    bool fixedclosed;
    bool matchclosed;
    Spline* nearest, *match;
    // remove any single line segments
    size_t i = 1;
    while (i < list.size()) {
        Spline* fixed = list.get(i-1);
        if (fixed->numberOfPoints() > 1) {
            fixedclosed = fixed->getFirstPoint().distanceToPoint(fixed->getLastPoint()) < 1E-5;
        }
        else
            fixedclosed = false;
        if (fixed->numberOfPoints() > 1 && !fixedclosed) {
            float nearestdist = 1E30f;
            nearest = 0;
            for (size_t j=1; j<=list.size(); ++j) {
                if (i == j)
                    continue;
                match = list.get(j-1);
                if (match->numberOfPoints() > 1)
                    matchclosed = match->getFirstPoint().distanceToPoint(match->getLastPoint()) < 1E-5;
                else
                    matchclosed = false;
                if (match->numberOfPoints() > 1 && !matchclosed) {
                    float d1 = SquaredDistPP(fixed->getFirstPoint(), match->getFirstPoint());
                    float d2 = SquaredDistPP(fixed->getFirstPoint(), match->getLastPoint());
                    float d3 = SquaredDistPP(fixed->getLastPoint(), match->getFirstPoint());
                    float d4 = SquaredDistPP(fixed->getLastPoint(), match->getLastPoint());
                    float min = Minimum(d1, d2, d3, d4);
                    if (min < nearestdist) {
                        nearestdist = min;
                        nearest = match;
                        if (min < 1E-5)
                            break;
                    }
                }
            }
            if (nearest != 0) {
                match = nearest;
                float d1 = SquaredDistPP(fixed->getFirstPoint(), match->getFirstPoint());
                float d2 = SquaredDistPP(fixed->getFirstPoint(), match->getLastPoint());
                float d3 = SquaredDistPP(fixed->getLastPoint(), match->getFirstPoint());
                float d4 = SquaredDistPP(fixed->getLastPoint(), match->getLastPoint());
                float min = Minimum(d1, d2, d3, d4);
                if (min < join_error || force_to_one_segment) {
                    // the splines do touch each other on one of their ends
                    if (min == d1)
                        fixed->insert_spline(0, true, true, *match);
                    else if (min == d2)
                        fixed->insert_spline(0, false, true, *match);
                    else if (min == d3)
                        fixed->insert_spline(fixed->numberOfPoints(), false, true, *match);
                    else if (min == d4)
                        fixed->insert_spline(fixed->numberOfPoints(), true, true, *match);
                    else
                        throw runtime_error("Error in comparing minimum values JoinSplineSegments");
                    list.del(match);
                    i = 0;  // reset match index to start searching for a new matching spline
                }
            }
        }
        ++i;
    }
}

int ShipCAD::ReadIntFromStr(size_t lineno, const QString& str, size_t& start)
{
    int spc = str.indexOf(' ', start);
    QStringRef s;
    if (spc != -1)
        s = str.midRef(start, spc - start);
    else
        s = str.midRef(start);
    bool ok;
    int result = s.toInt(&ok);
    if (!ok) {
        QString msg = QString("Invalid integer value in lineno: %1").arg(lineno);
        throw runtime_error(msg.toStdString());
    }
    if (spc != -1)
        start = spc + 1;
    else
        start = str.length();
    return result;
}

bool ShipCAD::ReadBoolFromStr(size_t /*lineno*/, const QString& str, size_t& start)
{
    int spc = str.indexOf(' ', start);
    QStringRef s;
    if (spc != -1)
        s = str.midRef(start, spc - start);
    else
        s = str.midRef(start);
    bool result = (s == "1" || s == "TRUE" || s == "YES");
    if (spc != -1)
        start = spc + 1;
    else
        start = str.length();
    return result;
}

float ShipCAD::ReadFloatFromStr(size_t lineno, const QString& str, size_t& start)
{
    int spc = str.indexOf(' ', start);
    QStringRef s;
    if (spc != -1)
        s = str.midRef(start, spc - start);
    else
        s = str.midRef(start);
    bool ok;
    float result = s.toFloat(&ok);
    if (!ok) {
        QString msg = QString("Invalid floating point value in lineno: %1").arg(lineno);
        throw runtime_error(msg.toStdString());
    }
    if (spc != -1)
        start = spc + 1;
    else
        start = str.length();
    return result;
}

float ShipCAD::FindWaterViscosity(float density, unit_type_t units)
{
	float result;
	double tmp_density;
    if (units == fuMetric) {
		result = 1.13902 + ((density - 0.999) / (1.0259-0.999))*(1.18831-1.13902);
	} else {
		tmp_density = density / kWeightConversionFactor;
		result = 1.13902+((tmp_density-0.999)/(1.0259-0.999))*(1.18831-1.13902);
		result /= (kFoot * kFoot);
	}
	return result;
}

QString ShipCAD::ChangeFileExt(const QString& name, const QString& ext)
{
	QString fname = name;
	int lasti = name.lastIndexOf(".");
	if (lasti == -1) {
		fname.append(ext);
	} else {
		fname.replace(lasti, name.length() - lasti, ext);
	}
	return fname;
}

float ShipCAD::VolumeToDisplacement(float volume, float density, float appcoeff, unit_type_t units)
{
    if (units == fuImperial)
        return volume * appcoeff * density / 2240;
    return volume * appcoeff * density;
}

static int NumberOfDecimals(float value)
{
	value = fabs(value);
	if (value < 1e-6)
        value = 1e-6f;
	double i = log(value)/2.30258;
    i = i < 0 ? ceil(i) : floor(i);
    int result = 4 - static_cast<int>(i);
	if (result < 0)
		result = 0;
	if (result > 3)
		result = 3;
	return result;
}

QString ShipCAD::MakeLength(float value, int decimals, int des_length)
{
	if (decimals == -1)
		decimals = NumberOfDecimals(value);
    QString input = QString("%1").arg(value, 0, 'f', decimals);
    return input.rightJustified(des_length, ' ');
}

QString ShipCAD::MakeLength(const QString& value, int des_length)
{
    return QString(value).leftJustified(des_length, ' ', true);
}

bool ShipCAD::FuzzyCompare(float val1, float val2, float error)
{
    return fabs(val1 - val2) < error;
}

double ShipCAD::Sign(double val)
{
    if (val < 0.0)
        return -1.0;
    else if (val > 0.0)
        return 1.0;
    return 0.0;
}

size_t ShipCAD::to_size_t(int val)
{
    if (val >= 0)
        return static_cast<size_t>(val);
    throw range_error("int cannot be converted to unsigned int");
}

QColor ShipCAD::RandomColor()
{
    quint8 r = rand() % 256;
    quint8 g = rand() % 256;
    quint8 b = rand() % 256;
    return QColor(r, g, b);
}

QVector3D ShipCAD::RotateVector(const QVector3D& coord,
                                double sinx, double cosx,
                                double siny, double cosy,
                                double sinz, double cosz)
{
    double r11 = cosy * cosz;
    double r12 = cosy * sinz;
    double r13 = -siny;
    double r21 = sinx * siny * cosz - cosx * sinz;
    double r22 = sinx * siny * sinz + cosx * cosz;
    double r23 = sinx * cosy;
    double r31 = cosx * siny * cosz + sinx * sinz;
    double r32 = cosx * siny * sinz - sinx * cosz;
    double r33 = cosx * cosy;
    return QVector3D(coord.x() * r11 + coord.y() * r12 + coord.z() * r13,
                     coord.x() * r21 + coord.y() * r22 + coord.z() * r23,
                     coord.x() * r31 + coord.y() * r32 + coord.z() * r33);
}

QString ShipCAD::ConvertDimension(float value, unit_type_t units)
{
    if (units == fuImperial) {
        int feet;
        if (value < 0)
            feet = static_cast<int>(ceil(value));
        else
            feet = static_cast<int>(floor(value));
        float inches = abs(value - feet) * 12;
        if (feet == 0 && value < 0)
            return QString("-%1' %2\"").arg(feet).arg(Truncate(inches, 1));
        else
            return QString("%1' %2\"").arg(feet).arg(Truncate(inches, 1));
    } else
        return QString("%1").arg(round(1000*value));
}
