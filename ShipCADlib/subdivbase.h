/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#ifndef SUBDIVBASE_H_
#define SUBDIVBASE_H_

#include <iosfwd>
#include <QObject>

namespace ShipCADGeometry {

//////////////////////////////////////////////////////////////////////////////////////

class SubdivisionSurface;
class Viewport;

/*! \brief the base class for all subdivision points, edges and faces
 *
 */
class SubdivisionBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SubdivisionSurface* Owner READ getOwner)

public:

	/*! \brief Constructor
	 *
	 * \param owner which surface this element belongs to
	 */
    explicit SubdivisionBase(SubdivisionSurface* owner);
    virtual ~SubdivisionBase();

	/*! \brief get the owning surface
	 *
	 * \return the owning surface
	 */
    SubdivisionSurface* getOwner() { return _owner; }
	/*! \brief set the owing surface
	 *
	 * \param newowner the owning surface
	 */
    virtual void setOwner(SubdivisionSurface* newowner)
        { _owner = newowner; }

	/*! \brief reset this element to default values
	 */
    virtual void clear() = 0;

    /*! \brief print out the element to a stream
	 *
	 * \param os the output stream
	 * \param prefix string to prefix on each line output
	 */
    virtual void dump(std::ostream& os, const char* prefix = "") const;

protected:

	/*! \brief dump the element to a stream
	 *
	 * \param os the output stream
	 * \param prefix string to prefix on each line output
	 */
    void priv_dump(std::ostream& os, const char* prefix) const;

protected:

    SubdivisionSurface* _owner;	/**< the owning surface */
};

//////////////////////////////////////////////////////////////////////////////////////

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCADGeometry::SubdivisionBase& base);

#endif

