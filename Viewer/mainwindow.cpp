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
\
    // write it to text file
    vector<QString> strings;
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
    delete _vp;
    delete ui;
}

void
MainWindow::setAnimating(bool animating)
{
  _vp->setAnimating(animating);
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
        vector<QString> lines;
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

void
MainWindow::showControlNet(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showControlNet() != val) {
        s->setShowControlNet(val);
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showInteriorEdges(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showInteriorEdges() != val) {
        s->setShowInteriorEdges(val);
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showControlCurves(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showControlCurves() != val) {
        s->setShowControlCurves(val);
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showCurvature(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showCurvature() != val) {
        s->setShowCurvature(val);
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::showNormals(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->showNormals() != val) {
        s->setShowNormals(val);
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::drawMirror(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->drawMirror() != val) {
        s->setDrawMirror(val);
        cout << "surface draw mirror: " << (val ? 'y' : 'n') << endl;
    }
}

void
MainWindow::shadeUnderwater(bool val)
{
    SubdivisionSurface* s = _vp->getSurface();
    if (s && s->shadeUnderWater() != val) {
        s->setShadeUnderWater(val);
        cout << "surface shade underwater: " << (val ? 'y' : 'n') << endl;
    }
}
