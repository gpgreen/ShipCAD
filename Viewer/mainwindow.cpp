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

#include <vector>
#include <iostream>
#include <fstream>
#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewport.h"

// stuff to create/show surface/spline
#include "spline.h"
#include "subdivsurface.h"
#include "subdivpoint.h"

using namespace std;
using namespace ShipCAD;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _vp(0), _animation_timer(0)
{
    ui->setupUi(this);

    // make the viewport
    _vp = new Viewport();
    // put it in window container
    QWidget* container = QWidget::createWindowContainer(_vp);
    container->setMinimumSize(320,200);
    // put it in display area
    ui->displayLayout->addWidget(container);

    // make action groups
    _modeGroup = new QActionGroup(this);
    _modeGroup->addAction(ui->actionWire_Frame);
    _modeGroup->addAction(ui->actionShade);
    _modeGroup->addAction(ui->actionShade_Curvature);
    _modeGroup->addAction(ui->actionShade_Developable);
    _modeGroup->addAction(ui->actionShade_Zebra);

    // connect actions to slots
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(openFile()));
    connect(ui->actionWire_Frame, SIGNAL(triggered()), SLOT(wireFrame()));
    connect(ui->actionShade, SIGNAL(triggered()), SLOT(shade()));
    connect(ui->actionShade_Curvature, SIGNAL(triggered()), SLOT(shadeCurvature()));
    connect(ui->actionShade_Developable, SIGNAL(triggered()), SLOT(shadeDevelopable()));
    connect(ui->actionShade_Zebra, SIGNAL(triggered()), SLOT(shadeZebra()));
    connect(ui->actionShow_Control_Net, SIGNAL(triggered(bool)), SLOT(showControlNet(bool)));
    connect(ui->actionShow_Interior_Edges, SIGNAL(triggered(bool)), SLOT(showInteriorEdges(bool)));
    connect(ui->actionShow_Control_Curves, SIGNAL(triggered(bool)), SLOT(showControlCurves(bool)));
    connect(ui->actionShow_Curvature, SIGNAL(triggered(bool)), SLOT(showCurvature(bool)));
    connect(ui->actionShow_Normals, SIGNAL(triggered(bool)), SLOT(showNormals(bool)));
    connect(ui->actionDraw_Mirror, SIGNAL(triggered(bool)), SLOT(drawMirror(bool)));
    connect(ui->actionShade_Underwater, SIGNAL(triggered(bool)), SLOT(shadeUnderwater(bool)));

    // make a spline
    Spline* spline = new Spline;
    spline->add(QVector3D(0,0,0));
    spline->add(QVector3D(0.7f,1.0f,0));
    spline->add(QVector3D(1,1,0));
    spline->setProperty("Color", QColor(Qt::blue));
    //spline->setProperty("CurvatureColor", QColor(Qt::yellow));
    spline->setProperty("ShowCurvature", true);
    //spline->setProperty("ShowPoints", true);
    cerr << *spline << endl;

#if 0
    // write it to dxf..
    vector<QString> dxfstrings;
    QString layer("splinelayer");
    spline->saveToDXF(dxfstrings, layer, false);
    ofstream os("spline.dxf");
    for (size_t i=0; i<dxfstrings.size(); ++i)
        os << dxfstrings[i].toStdString() << "\r\n";
    os.close();
    cerr << *spline << endl;
#endif

    // make a surface
    SubdivisionSurface* surface = new SubdivisionSurface;
    surface->setDesiredSubdivisionLevel(3);
    vector<SubdivisionControlPoint*> points;
    SubdivisionControlPoint* pt = surface->addControlPoint(QVector3D(1,1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(1,-1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(-1,-1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(-1,1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(0,0,-.5));
    points.push_back(pt);
    vector<SubdivisionControlPoint*> fc1;
    fc1.push_back(points[0]);
    fc1.push_back(points[1]);
    fc1.push_back(points[4]);
    surface->addControlFace(fc1, true);
    vector<SubdivisionControlPoint*> fc2;
    fc2.push_back(points[1]);
    fc2.push_back(points[2]);
    fc2.push_back(points[4]);
    surface->addControlFace(fc2, true);
    vector<SubdivisionControlPoint*> fc3;
    fc3.push_back(points[2]);
    fc3.push_back(points[3]);
    fc3.push_back(points[4]);
    surface->addControlFace(fc3, true);
    vector<SubdivisionControlPoint*> fc4;
    fc4.push_back(points[3]);
    fc4.push_back(points[0]);
    fc4.push_back(points[4]);
    surface->addControlFace(fc4, true);
    cerr << *surface << endl;
    surface->rebuild();
//    cerr << *surface << endl;
\
    // write it to text file
    QStringList strings;
    surface->saveToStream(strings);
    ofstream sos("surface.txt");
    for (size_t i=0; i<strings.size(); ++i)
        sos << strings[i].toStdString() << "\r\n";
    sos.close();

    _vp->add(spline);
    setSurface(surface);
}

MainWindow::~MainWindow()
{
    delete _animation_timer;
    delete _vp;
    delete ui;
}

void
MainWindow::setAnimating(bool animating)
{
    if (animating && _animation_timer != 0)
        return;
    if (!animating && _animation_timer != 0) {
        _animation_timer->stop();
        delete _animation_timer;
        _animation_timer = 0;
        _vp->setAnimating(false);
        return;
    }
    if (!animating)
        return;
    _animation_timer = new QTimer(this);
    connect(_animation_timer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
    _animation_timer->start(100);
    _vp->setAnimating(true);
}

void MainWindow::animationTimeout()
{
    static float angle = 0;
    static float elevation = 0;
    static bool up = true;
    angle += 5;
    if (angle >= 180)
        angle -= 360;
    if (up)
        elevation += 1;
    else
        elevation -= 1;
    if (elevation >= 90) {
        up = false;
        elevation = 89;
    }
    if (elevation <= 0) {
        up = true;
        elevation = 0;
    }
    _vp->setAngle(angle);
    _vp->setElevation(elevation);
}

void
MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
            tr("All Files (*.*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        QTextStream in(&file);
        // read in file line by line
        QStringList lines;
        do {
            QString line = in.readLine();
            if (line.isNull())
                break;
            lines.push_back(line);
        }
        while (!in.atEnd());
        file.close();
        // now we have the file as lines, make a surface
        SubdivisionSurface* newsurface = new SubdivisionSurface;
        size_t lineno = 0;
        newsurface->loadFromStream(lineno, lines);
        // now set the gui to this new surface
        SubdivisionSurface* old = _vp->getSurface();
        if (old != 0) {
            delete old;
        }
        setSurface(newsurface);
        newsurface->setDesiredSubdivisionLevel(3);
        newsurface->rebuild();
        _vp->renderLater();
    }
}

void
MainWindow::setSurface(SubdivisionSurface *surface)
{
    _vp->setSurface(surface);
    if (surface != 0) {
        ui->actionShow_Control_Net->setChecked(surface->showControlNet());
        ui->actionShow_Interior_Edges->setChecked(surface->showInteriorEdges());
        ui->actionShow_Control_Curves->setChecked(surface->showControlCurves());
        ui->actionShow_Curvature->setChecked(surface->showCurvature());
        ui->actionShow_Normals->setChecked(surface->showNormals());
        ui->actionDraw_Mirror->setChecked(surface->drawMirror());
        ui->actionShade_Underwater->setChecked(surface->shadeUnderWater());
    }
}

void
MainWindow::wireFrame()
{
  if (_vp->getViewportMode() != vmWireFrame)
    _vp->setViewportMode(vmWireFrame);
  cout << "Viewport mode Wire Frame" << endl;
}

void
MainWindow::shade()
{
  if (_vp->getViewportMode() != vmShade)
    _vp->setViewportMode(vmShade);
  cout << "Viewport mode shade" << endl;
}

void
MainWindow::shadeCurvature()
{
  if (_vp->getViewportMode() != vmShadeGauss)
    _vp->setViewportMode(vmShadeGauss);
  cout << "Viewport mode shade gauss" << endl;
}

void
MainWindow::shadeDevelopable()
{
  if (_vp->getViewportMode() != vmShadeDevelopable)
    _vp->setViewportMode(vmShadeDevelopable);
  cout << "Viewport mode shade developable" << endl;
}

void
MainWindow::shadeZebra()
{
  if (_vp->getViewportMode() != vmShadeZebra)
    _vp->setViewportMode(vmShadeZebra);
  cout << "Viewport mode shad zebra" << endl;
}

void
MainWindow::showControlNet(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showControlNet() != val) {
        s->setShowControlNet(val);
        _vp->renderLater();
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showInteriorEdges(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showInteriorEdges() != val) {
        s->setShowInteriorEdges(val);
        _vp->renderLater();
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showControlCurves(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showControlCurves() != val) {
        s->setShowControlCurves(val);
        _vp->renderLater();
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showCurvature(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showCurvature() != val) {
        s->setShowCurvature(val);
        _vp->renderLater();
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showNormals(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showNormals() != val) {
        s->setShowNormals(val);
        _vp->renderLater();
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::drawMirror(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->drawMirror() != val) {
        s->setDrawMirror(val);
        _vp->renderLater();
        cout << "surface draw mirror: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::shadeUnderwater(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->shadeUnderWater() != val) {
        s->setShadeUnderWater(val);
        _vp->renderLater();
        cout << "surface shade underwater: " << (val ? 'y' : 'n') << endl;
    }
}
