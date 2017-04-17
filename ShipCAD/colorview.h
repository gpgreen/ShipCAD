/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>                                   *
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
#ifndef COLORVIEW_H
#define COLORVIEW_H

#include <QWidget>

class ColorView : public QWidget
{
    Q_OBJECT

public:

    explicit ColorView(const QColor& color, QWidget *parent = 0);
    virtual ~ColorView() {}
    
    // getters/setters for color
    void setColor(const QColor& color);
    void setAlpha(float alpha);
    void setColor(const QColor& color, float alpha);

protected:

    virtual void paintEvent(QPaintEvent *event);

private:

    QColor _color;
};

#endif // COLORVIEW_H
