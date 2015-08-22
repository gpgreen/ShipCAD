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

#include <iostream>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewport.h"
#include "shipcadmodel.h"
#include "controller.h"

using namespace ShipCAD;
using namespace std;

MainWindow::MainWindow(Controller* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _controller(c), _menu_recent_files(0)
{
    ui->setupUi(this);
    addDefaultViewports();
    createRecentFiles();
    createToolBars();
    createStatusBar();

    // connect controller signals
    connect(_controller, SIGNAL(updateUndoData()), SLOT(updateUndoData()));
    connect(_controller, SIGNAL(changeRecentFiles()), SLOT(changeRecentFiles()));
    connect(_controller, SIGNAL(changedLayerData()), SLOT(changedLayerData()));
    connect(_controller, SIGNAL(changeActiveLayer()), SLOT(changeActiveLayer()));
    connect(_controller, SIGNAL(changedModel()), SLOT(modelChanged()));
    connect(_controller, SIGNAL(onUpdateVisibilityInfo()), SLOT(updateVisibilityActions()));

    // connect file actions
    connect(ui->actionFileNew, SIGNAL(triggered()), _controller, SLOT(newModel()));
    connect(ui->actionOpen, SIGNAL(triggered()), _controller, SLOT(loadFile()));
    connect(ui->actionSave, SIGNAL(triggered()), _controller, SLOT(saveFile()));
    connect(ui->actionSave_As, SIGNAL(triggered()), _controller, SLOT(saveAsFile()));
    connect(ui->actionImportCarene, SIGNAL(triggered()), _controller, SLOT(importCarene()));
    connect(ui->actionImportChines, SIGNAL(triggered()), _controller, SLOT(importChines()));
    connect(ui->actionImportFEF, SIGNAL(triggered()), _controller, SLOT(importFEF()));
    connect(ui->actionImportPart, SIGNAL(triggered()), _controller, SLOT(importPart()));
    connect(ui->actionImportPolyCad, SIGNAL(triggered()), _controller, SLOT(importPolycad()));
    connect(ui->actionImportSurface, SIGNAL(triggered()), _controller, SLOT(importSurface()));
    connect(ui->actionImportVRML, SIGNAL(triggered()), _controller, SLOT(importVRML()));
    connect(ui->actionImportMichlet_waves, SIGNAL(triggered()), _controller, SLOT(importMichletWaves()));
    connect(ui->actionImportCarlson, SIGNAL(triggered()), _controller, SLOT(importHull()));
    connect(ui->actionExportArchimedes, SIGNAL(triggered()), _controller, SLOT(exportFileArchimedes()));
    connect(ui->actionExportCoordinates, SIGNAL(triggered()), _controller, SLOT(exportCoordinates()));
    connect(ui->actionExportDXF_2D_Polylines, SIGNAL(triggered()), _controller, SLOT(export2DPolylinesDXF()));
    connect(ui->actionExportDXF_3D_mesh, SIGNAL(triggered()), _controller, SLOT(exportFacesDXF()));
    connect(ui->actionExportDXF_3D_Polylines, SIGNAL(triggered()), _controller, SLOT(export3DPolylinesDXF()));
    connect(ui->actionExportFEF, SIGNAL(triggered()), _controller, SLOT(exportFEF()));
    connect(ui->actionExportGHS, SIGNAL(triggered()), _controller, SLOT(exportGHS()));
    connect(ui->actionExportPart, SIGNAL(triggered()), _controller, SLOT(exportPart()));
    connect(ui->actionExportMichlet, SIGNAL(triggered()), _controller, SLOT(exportMichlet()));
    connect(ui->actionExportWavefront, SIGNAL(triggered()), _controller, SLOT(exportObj()));
    connect(ui->actionExportOffsets, SIGNAL(triggered()), _controller, SLOT(exportOffsets()));
    connect(ui->actionExportSTL, SIGNAL(triggered()), _controller, SLOT(exportSTL()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(showPreferences()));

    // connect project actions

    // connect edit actions

    // connect point actions

    // connect edge actions

    // connect face actions

    // connect layer actions

    // connect visibility actions
    connect(ui->actionShowControl_net, SIGNAL(triggered(bool)), SLOT(showControlNet(bool)));
    connect(ui->actionShowInterior_edges, SIGNAL(triggered(bool)), SLOT(showInteriorEdges(bool)));
    connect(ui->actionShowControl_curves, SIGNAL(triggered(bool)), SLOT(showControlCurves(bool)));
    connect(ui->actionShowCurvature, SIGNAL(triggered(bool)), SLOT(showCurvature(bool)));
    connect(ui->actionShowNormals, SIGNAL(triggered(bool)), SLOT(showNormals(bool)));
    connect(ui->actionShow_both_sides, SIGNAL(triggered(bool)), SLOT(showBothSides(bool)));

    // connect selection actions

    // connect tools actions

    // connect transform actions

    // connect calculations actions

    // connect window actions

    // connect about actions

    // set action status
    updateVisibilityActions();

    ui->statusBar->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        delete _viewports[i].second;
    }
    delete ui;
}

void MainWindow::addDefaultViewports()
{
    ShipCADModel* model = _controller->getModel();
    int row = 0;
    int col = 0;
    for (int i=0; i<4; i++) {
        // make the viewport
        Viewport* vp = new Viewport();
        // connect the render signal to the viewport
        connect(this, SIGNAL(viewportRender()), vp, SLOT(renderLater()));
        vp->setSurface(model->getSurface());
        // put it in window container
        QWidget* container = QWidget::createWindowContainer(vp);
        container->setMinimumSize(320,200);
        // put it in display area
        ui->displayLayout->addWidget(container, row, col++);
        if (col == 2) {
            col = 0;
            row++;
        }
        _viewports.push_back(make_pair(container, vp));
    }
}

void MainWindow::updateVisibilityActions()
{
    Visibility& vis = _controller->getModel()->getVisibility();

    ui->actionShowControl_net->setChecked(vis.isShowControlNet());
    //ui->actionShow_both_sides->setChecked(vis.drawMirror());
    ui->actionShowControl_curves->setChecked(vis.isShowControlCurves());
    ui->actionShowInterior_edges->setChecked(vis.isShowInteriorEdges());
    //ui->actionShowGrid->setChecked(vis.isShowControlCurves());
    //ui->actionShowStations->setChecked(vis.isShowStations());
    //ui->actionShowButtocks->setChecked(vis.isShowButtocks());
    //ui->actionShowWaterlines->setChecked(vis.isShowWaterlines());
    //ui->actionShowDiagonals->setChecked(vis.isShowDiagonals());
    //ui->actionShowHydrostatic_features->setChecked(vis.isShowHydrostaticFeatures());
    //ui->actionShowFlowlines->setChecked(vis.isShowFlowlines());
    //ui->actionShowNormals->setChecked(vis.isShowNormals());
    ui->actionShowCurvature->setChecked(vis.isShowCurvature());
    //ui->actionShowMarkers->setChecked(vis.isShowMarkers());

//    ui->actionShade_Underwater->setChecked(s->shadeUnderWater());
    emit viewportRender();
    cout << "updateVisibilityActions" << endl;
}


// add the menu entry to the file menu, and create all the actions...
void MainWindow::createRecentFiles()
{
    _menu_recent_files = new QMenu(tr("Recent files"), this);
    ui->menuFile->addMenu(_menu_recent_files);
    for (int i=0; i<10; i++) {
        QAction* action = new QAction(this);
        _recent_file_actions.push_back(action);
        action->setVisible(false);
        _menu_recent_files->addAction(action);
        connect(action, SIGNAL(triggered()), SLOT(openRecentFile()));
    }
    _menu_recent_files->setEnabled(false);
}

// update the recent files menu
void MainWindow::changeRecentFiles()
{
    cout << "changeRecentFiles" << endl;
    // the recent files menu
    const QStringList& filelist = _controller->getRecentFiles();
    if (filelist.size())
        _menu_recent_files->setEnabled(true);
    for (int i=0; i<filelist.size(); i++) {
        QString text = tr("&%1 %2").arg(i+1).arg(filelist[i]);
        _recent_file_actions[i]->setText(text);
        _recent_file_actions[i]->setData(filelist[i]);
        _recent_file_actions[i]->setVisible(true);
    }
    for (int i=filelist.size(); i<10; i++)
        _recent_file_actions[i]->setVisible(false);
}

void MainWindow::openRecentFile()
{
    cout << "openRecentFile" << endl;
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        _controller->loadFile(action->data().toString());
    }
}

void MainWindow::updateUndoData()
{

}

void MainWindow::changedLayerData()
{

}

void MainWindow::changeActiveLayer()
{

}

void MainWindow::showPreferences()
{

}

void
MainWindow::wireFrame()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        Viewport* vp = _viewports[i].second;
        if (vp->getViewportMode() != vmWireFrame)
            vp->setViewportMode(vmWireFrame);
    }
    cout << "Viewport mode Wire Frame" << endl;
}

void
MainWindow::shade()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        Viewport* vp = _viewports[i].second;
        if (vp->getViewportMode() != vmShade)
            vp->setViewportMode(vmShade);
    }
    cout << "Viewport mode shade" << endl;
}

void
MainWindow::shadeCurvature()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        Viewport* vp = _viewports[i].second;
        if (vp->getViewportMode() != vmShadeGauss)
            vp->setViewportMode(vmShadeGauss);
    }
    cout << "Viewport mode shade gauss" << endl;
}

void
MainWindow::shadeDevelopable()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        Viewport* vp = _viewports[i].second;
        if (vp->getViewportMode() != vmShadeDevelopable)
            vp->setViewportMode(vmShadeDevelopable);
    }
    cout << "Viewport mode shade developable" << endl;
}

void
MainWindow::shadeZebra()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        Viewport* vp = _viewports[i].second;
        if (vp->getViewportMode() != vmShadeZebra)
            vp->setViewportMode(vmShadeZebra);
    }
    cout << "Viewport mode shade zebra" << endl;
}

void
MainWindow::showControlNet(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    if (vis.isShowControlNet() != val) {
        vis.setShowControlNet(val);
        cout << "surface control net visible: " << (val ? 'y' : 'n') << endl;
    }
    ui->actionShowControl_net->setChecked(val);
}

void
MainWindow::showInteriorEdges(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    if (vis.isShowInteriorEdges() != val) {
        vis.setShowInteriorEdges(val);
        cout << "surface interior edges visible: " << (val ? 'y' : 'n') << endl;
    }
    ui->actionShowInterior_edges->setChecked(val);
}

void
MainWindow::showControlCurves(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    if (vis.isShowControlCurves() != val) {
        vis.setShowControlCurves(val);
        cout << "surface control curves visible: " << (val ? 'y' : 'n') << endl;
    }
    ui->actionShowControl_curves->setChecked(val);
}

void
MainWindow::showCurvature(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    if (vis.isShowCurvature() != val) {
        vis.setShowCurvature(val);
        cout << "surface curvature visible: " << (val ? 'y' : 'n') << endl;
    }
    ui->actionShowCurvature->setChecked(val);
}

void
MainWindow::showNormals(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    SubdivisionSurface* s = _controller->getModel()->getSurface();
    if (s->showNormals() != val) {
        s->setShowNormals(val);
        cout << "surface normals visible: " << (val ? 'y' : 'n') << endl;
    }
    ui->actionShowNormals->setChecked(val);
}

void
MainWindow::showBothSides(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    SubdivisionSurface* s = _controller->getModel()->getSurface();
    if (s->drawMirror() != val) {
        s->setDrawMirror(val);
        emit viewportRender();
        cout << "surface draw mirror: " << (val ? 'y' : 'n') << endl;
    }
//    ui->actionShowControl_net->setChecked(val);
}

void
MainWindow::shadeUnderwater(bool val)
{
    Visibility& vis = _controller->getModel()->getVisibility();
    SubdivisionSurface* s = _controller->getModel()->getSurface();
    if (s->shadeUnderWater() != val) {
        s->setShadeUnderWater(val);
        emit viewportRender();
        cout << "surface shade underwater: " << (val ? 'y' : 'n') << endl;
    }
}

void MainWindow::modelChanged()
{
    cout << "model changed" << endl;
}

void MainWindow::createToolBars()
{

}

void MainWindow::createStatusBar()
{
    QLabel* _undo_info = new QLabel(tr("undo memory:"));
    _undo_info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QLabel* _geom_info = new QLabel(tr("faces: 0"));
    _geom_info->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    ui->statusBar->addPermanentWidget(_undo_info);
    ui->statusBar->addPermanentWidget(_geom_info);
}
