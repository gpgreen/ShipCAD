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

#ifndef FLOWLINE_H_
#define FLOWLINE_H_

#include <QtCore>
#include <QtGui>
#include "entity.h"
#include "shipcadlib.h"
#include "pointervec.h"

namespace ShipCAD {

class ShipCADModel;
class Viewport;
class LineShader;
class FileBuffer;
class Spline;

//////////////////////////////////////////////////////////////////////////////////////

class Flowline : public Entity
{
    Q_OBJECT

public:

    explicit Flowline(ShipCADModel* owner);
    virtual ~Flowline();

    static Flowline* construct(ShipCADModel* owner);
	
    virtual void clear();
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void rebuild();
    virtual void setBuild(bool val);

    bool isVisible() const;
    bool isSelected() const;
	void setSelected(bool set);

    QColor getColor() const;
    
    void loadBinary(FileBuffer& source);
    void saveBinary(FileBuffer& dest);
									
private:

    ShipCADModel* _owner;
    QVector2D _projection_point;
    viewport_type_t _projection_vw;
    bool _method_new;
    Spline* _flowline;
};

typedef PointerVector<Flowline> FlowlineVector;

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

