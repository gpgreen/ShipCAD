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

#include <QPainter>
#include <QBrush>
#include "colorview.h"

ColorView::ColorView(const QColor& color, QWidget *parent) :
    QWidget(parent), _color(color)
{
    // does nothing
}

void ColorView::setColor(const QColor& color) {
    _color = color;
    setAlpha(1.0f);
}

void ColorView::setAlpha(float alpha) 
{
    _color.setAlphaF(alpha);
}

void ColorView::setColor(const QColor& color, float alpha) 
{
    _color = color;
    _color.setAlphaF(alpha);
}

void ColorView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent( event );

    QPainter painter;
    painter.begin(this);
    painter.fillRect(1, 1, 30, 30, QBrush(_color));
    painter.end();
}
