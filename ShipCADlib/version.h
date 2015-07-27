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

#ifndef VERSION_H_
#define VERSION_H_

#include <QString>

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

  enum version_t {
    fv100 = 1,
    fv110,
    fv120,
    fv130,
    fv140,
    fv150,
    fv160,
    fv165,
    fv170,
    fv180,
    fv190,
    fv191,
    fv195,
    fv198,
    fv200,
    fv201,
    fv210,
    fv220,
    fv230,
    fv240,
    fv250,
    fv260,
  };

  const version_t k_current_version = fv260;

  const QString k_released_date = QString("April 21, 2006");

  extern QString versionString(version_t v);

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

#endif

