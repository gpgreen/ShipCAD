/*##############################################################################################
 *    ShipCAD
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>
 *    Original Copyright header below
 *
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

#include <stdexcept>
#include "shipcadlib.h"

using namespace ShipCAD;

const char* ShipCAD::kFileExtension = ".fbm";

const QVector3D ShipCAD::ZERO(0,0,0);

const QVector3D ShipCAD::ONE(1,1,1);

const QVector2D ShipCAD::ZERO2(0,0);

QString ShipCAD::AreaStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[ft2]");
    return QObject::tr("[m2]");
}

QString ShipCAD::LengthStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[ft]");
    return QObject::tr("[m]");
}

QString ShipCAD::InertiaStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[ft4]");
    return QObject::tr("[m4]");
}

QString ShipCAD::VolStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[ft3]");
    return QObject::tr("[m3]");
}

QString ShipCAD::DensityStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[lbs/ft3]");
    return QObject::tr("[t/m3]");
}

QString ShipCAD::WeightStr(unit_type_t units)
{
    if (units == fuImperial)
        return QObject::tr("[tons]");
    return QObject::tr("[tonnes]");
}

QImage ShipCAD::CreateFromJPEG(const JPEGImage* image)
{
    uchar* data = new uchar[image->data.size()];
    for (size_t i=0; i<image->data.size(); ++i)
        data[i] = image->data[i];
    QImage result = QImage::fromData(data, image->data.size());
    if (result.isNull())
        throw std::runtime_error("Unable to create QImage from image data");
    delete [] data;
    return result;
}

