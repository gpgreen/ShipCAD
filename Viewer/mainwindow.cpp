#include <vector>
#include <iostream>
#include <fstream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewport.h"

// stuff to create/show surface/spline
#include "spline.h"
#include "subdivsurface.h"
#include "subdivpoint.h"

using namespace std;
using namespace ShipCADGeometry;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _vp(0)
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
    connect(ui->actionWire_Frame, SIGNAL(triggered()), SLOT(wireFrame()));
    connect(ui->actionShade, SIGNAL(triggered()), SLOT(shade()));
    connect(ui->actionShade_Curvature, SIGNAL(triggered()), SLOT(shadeCurvature()));
    connect(ui->actionShade_Developable, SIGNAL(triggered()), SLOT(shadeDevelopable()));
    connect(ui->actionShade_Zebra, SIGNAL(triggered()), SLOT(shadeZebra()));

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

    // write it to dxf..
    vector<QString> dxfstrings;
    QString layer("splinelayer");
    spline->save_to_dxf(dxfstrings, layer, false);
    ofstream os("spline.dxf");
    for (size_t i=0; i<dxfstrings.size(); ++i)
        os << dxfstrings[i].toStdString() << "\r\n";
    os.close();
    cerr << *spline << endl;

    // make a surface
    SubdivisionSurface* surface = new SubdivisionSurface;
    surface->setDesiredSubdivisionLevel(3);
    surface->setShowControlNet(true);
    surface->setShowInteriorEdges(true);
    vector<SubdivisionControlPoint*> points;
    SubdivisionControlPoint* pt = surface->addControlPoint(QVector3D(1,1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(1,-1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(-1,-1,0));
    points.push_back(pt);
    pt = surface->addControlPoint(QVector3D(-1,1,0));
    points.push_back(pt);
    surface->addControlFace(points, true);
    cerr << *surface << endl;
    surface->rebuild();
//    cerr << *surface << endl;

    _vp->add(spline);
    _vp->add(surface);
}

MainWindow::~MainWindow()
{
    delete _vp;
    delete ui;
}

void
MainWindow::setAnimating(bool animating)
{
  _vp->setAnimating(animating);
}

void
MainWindow::wireFrame()
{
  if (_vp->getViewportMode() != Viewport::vmWireFrame)
    _vp->setViewportMode(Viewport::vmWireFrame);
  cout << "Viewport mode Wire Frame" << endl;
}

void
MainWindow::shade()
{
  if (_vp->getViewportMode() != Viewport::vmShade)
    _vp->setViewportMode(Viewport::vmShade);
  cout << "Viewport mode shade" << endl;
}

void
MainWindow::shadeCurvature()
{
  if (_vp->getViewportMode() != Viewport::vmShadeGauss)
    _vp->setViewportMode(Viewport::vmShadeGauss);
  cout << "Viewport mode shade gauss" << endl;
}

void
MainWindow::shadeDevelopable()
{
  if (_vp->getViewportMode() != Viewport::vmShadeDevelopable)
    _vp->setViewportMode(Viewport::vmShadeDevelopable);
  cout << "Viewport mode shade developable" << endl;
}

void
MainWindow::shadeZebra()
{
  if (_vp->getViewportMode() != Viewport::vmShadeZebra)
    _vp->setViewportMode(Viewport::vmShadeZebra);
  cout << "Viewport mode shad zebra" << endl;
}
