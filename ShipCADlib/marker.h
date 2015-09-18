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

#ifndef MARKER_H_
#define MARKER_H_

#include <QtCore>
#include <QtGui>
#include "entity.h"
#include "spline.h"
#include "pointervec.h"

namespace ShipCAD {

class ShipCADModel;
class Viewport;
class LineShader;
class FileBuffer;
	
//////////////////////////////////////////////////////////////////////////////////////

class Marker : public Spline
{
    Q_OBJECT

public:

    explicit Marker(ShipCADModel* owner);
    virtual ~Marker() {}

    static Marker* construct(ShipCADModel* owner);

    virtual void clear();
    virtual void draw(Viewport& vp, LineShader* lineshader);

    bool isVisible() const {return _visible;}
    void setVisible(bool set) {_visible=set;}
    bool isSelected();
	void setSelected(bool set);

    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);
									
public slots:

protected:

private:

    ShipCADModel* _owner;
	bool _visible;
};

typedef PointerVector<Marker> MarkerVector;

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

