/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
 *    Original Copyright header below														   *
 *																							   *
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

#ifndef BACKGROUNDIMAGE_H
#define BACKGROUNDIMAGE_H

#include <QtCore>
#include <QtGui>
#include "shipcadlib.h"

namespace ShipCAD {

class ShipCAD;
class FileBuf;
class Viewport;

//////////////////////////////////////////////////////////////////////////////////////

class BackgroundImage : public QImage
{
public:

    explicit BackgroundImage(ShipCAD* owner);
    ~BackgroundImage();

    void loadBinary(FileBuf& source);
    void saveBinary(FileBuf& dest);

    void updateData(Viewport& vp);
    void updateViews();

private:

    ShipCAD* _owner;
    viewport_type_t _assigned_view;
    int _quality;
    QVector3D _origin;
    float _scale;
    bool _transparent;
    int _blending_value;
    QColor _transparent_color;
    bool _visible;
    int _tolerance;
};


//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */


#endif // BACKGROUNDIMAGE_H

