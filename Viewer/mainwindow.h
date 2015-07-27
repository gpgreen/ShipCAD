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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QActionGroup>

namespace Ui {
class MainWindow;
}

namespace ShipCADGeometry {
class Viewport;
class SubdivisionSurface;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setAnimating(bool animating);

    void setSurface(ShipCADGeometry::SubdivisionSurface* surface);

public slots:

    void openFile();

protected slots:

    void wireFrame();
    void shade();
    void shadeCurvature();
    void shadeDevelopable();
    void shadeZebra();
    void showControlNet(bool val);
    void showInteriorEdges(bool val);
    void showControlCurves(bool val);
    void showCurvature(bool val);
    void showNormals(bool val);
    void drawMirror(bool val);
    void shadeUnderwater(bool val);

    void animationTimeout();

private:
    Ui::MainWindow *ui;
    ShipCADGeometry::Viewport* _vp;
    QActionGroup* _modeGroup;
    QTimer* _animation_timer;
};

#endif // MAINWINDOW_H
