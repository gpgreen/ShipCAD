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

#include <QObject>
#include <stdexcept>

#include "version.h"

using namespace ShipCAD;

//////////////////////////////////////////////////////////////////////////////////////

QString ShipCAD::versionString(version_t v)
{
    if (v == fv100)
        return QString("1.0");
    else if (v == fv110)
        return QString("1.1");
    else if (v == fv120)
        return QString("1.2");
    else if (v == fv130)
        return QString("1.3");
    else if (v == fv140)
        return QString("1.4");
    else if (v == fv150)
        return QString("1.5");
    else if (v == fv160)
        return QString("1.6");
    else if (v == fv165)
        return QString("1.65");
    else if (v == fv170)
        return QString("1.7");
    else if (v == fv180)
        return QString("1.8");
    else if (v == fv190)
        return QString("1.9");
    else if (v == fv191)
        return QString("1.91");
    else if (v == fv195)
        return QString("1.95");
    else if (v == fv198)
        return QString("1.98");
    else if (v == fv200)
        return QString("2.0");
    else if (v == fv201)
        return QString("2.01");
    else if (v == fv210)
        return QString("2.1");
    else if (v == fv220)
        return QString("2.2");
    else if (v == fv230)
        return QString("2.3");
    else if (v == fv240)
        return QString("2.4");
    else if (v == fv250)
        return QString("2.5");
    else if (v == fv260)
        return QString("2.6");
    else
        throw std::runtime_error(QObject::tr("Unknown fileversion").toStdString());
}
