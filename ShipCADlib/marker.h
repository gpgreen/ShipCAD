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

namespace ShipCADGeometry {

class ShipCAD;
class Viewport;
class LineShader;
class FileBuffer;
	
//////////////////////////////////////////////////////////////////////////////////////

class Marker : public Entity
{
    Q_OBJECT

public:

    explicit Marker(ShipCAD* owner);
    virtual ~Marker();

	static Marker* construct(ShipCAD* owner);
	
    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader);
    virtual void rebuild();

	bool isVisible();
	void setVisible(bool set);
	bool isSelected();
	void setSelected(bool set);

	void loadBinary(FileBuffer& buf);
	void saveBinary(FileBuffer& buf);
									
public slots:

protected:

private:

	ShipCAD* _owner;
	bool _visible;
	bool _selected;
};

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Vector class to contain Intersection pointers
 *
 */
class MarkerVector
{
public:

    typedef std::vector<Marker*>::iterator mvec_iterator ;

    MarkerVector();
    ~MarkerVector();

    void clear();

    size_t size() {return _vec.size();}

    void add(Marker* i) {_vec.push_back(i);}
    void del(Marker* i);

    typedef void apply_fn(Marker* elem);
    void apply(apply_fn* fn)
      {std::for_each(_vec.begin(), _vec.end(), fn);}

    mvec_iterator begin() {return _vec.begin();}
    mvec_iterator end() {return _vec.end();}

private:
    std::vector<Marker*> _vec;
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

#endif

