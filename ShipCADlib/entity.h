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

#ifndef ENTITY_H_
#define ENTITY_H_

#include <QObject>
#include <QColor>
#include <QPen>
#include <QVector3D>
#include <vector>
#include <iosfwd>

namespace ShipCAD {

class Viewport;
class LineShader;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Structure to record geometry intersections
 */
class IntersectionData
{
public:

    explicit IntersectionData() : number_of_intersections(0) {}
    ~IntersectionData() {}

    size_t number_of_intersections;
    std::vector<QVector3D> points;
    std::vector<float> parameters;
};

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief base class for all non-surface drawable elements
 */
class Entity : public QObject
{
    Q_OBJECT

public:

    explicit Entity();
    virtual ~Entity() {}

    virtual void clear();
    virtual void extents(QVector3D& min, QVector3D& max);
    virtual void draw(Viewport& vp, LineShader* lineshader) = 0;
    virtual void rebuild() = 0;

    // getters/setters
    virtual QColor getColor() const
        {return _color;}
    void setColor(const QColor& col)
        {_color = col;}

    int getPenWidth() const
        {return _pen_width;}
    void setPenWidth(int w)
        {_pen_width=w;}

    Qt::PenStyle getPenStyle() const
        {return _pen_style;}
    void setPenStyle(Qt::PenStyle style)
        {_pen_style = style;}

    QVector3D getMin() const;
    QVector3D getMax() const;
    bool isBuild() const
        {return _build;}
    virtual void setBuild(bool val);

    // output
    void dump(std::ostream& os) const;

protected:

protected:

    bool _build;
    QVector3D _min;
    QVector3D _max;
    int _pen_width;
    QColor _color;
    Qt::PenStyle _pen_style;

};

};				/* end namespace */

std::ostream& operator << (std::ostream& os, const ShipCAD::Entity& entity);

//////////////////////////////////////////////////////////////////////////////////////

#endif

