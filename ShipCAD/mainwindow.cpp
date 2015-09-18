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
#include "pointdialog.h"
#include "viewport.h"
#include "shipcadlib.h"
#include "shipcadmodel.h"
#include "controller.h"
#include "viewportcontainer.h"

using namespace ShipCAD;
using namespace std;

MainWindow::MainWindow(Controller* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _pointdialog(0),
    _controller(c), _menu_recent_files(0)
{
    ui->setupUi(this);
    createToolBars();
    createStatusBar();
    createRecentFilesMenu();

    // connect controller signals
    connect(_controller, SIGNAL(updateUndoData()), SLOT(updateUndoData()));
    connect(_controller, SIGNAL(changeRecentFiles()), SLOT(changeRecentFiles()));
    connect(_controller, SIGNAL(changedLayerData()), SLOT(changedLayerData()));
    connect(_controller, SIGNAL(changeActiveLayer()), SLOT(changeActiveLayer()));
    connect(_controller, SIGNAL(changedModel()), SLOT(modelChanged()));
    connect(_controller, SIGNAL(onUpdateVisibilityInfo()), SLOT(updateVisibilityActions()));
    //emit updateControlPointValue();
    connect(_controller, SIGNAL(showControlPointDialog(bool)), SLOT(showControlPointDialog(bool)));
    connect(_controller, SIGNAL(modelLoaded()), SLOT(modelLoaded()));
    connect(_controller, SIGNAL(modelGeometryChanged()), SIGNAL(viewportRender()));
    connect(_controller, SIGNAL(changeSelectedItems()), SLOT(changeSelectedItems()));

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
    connect(ui->actionExportIGES, SIGNAL(triggered()), _controller, SLOT(exportIGES()));
    connect(ui->actionPreferences, SIGNAL(triggered()), SLOT(showPreferences()));

    // connect project actions

    // connect edit actions
    connect(ui->actionUndo, SIGNAL(triggered()), _controller, SLOT(undo()));
    connect(ui->actionRedo, SIGNAL(triggered()), _controller, SLOT(redo()));
    connect(ui->actionDelete, SIGNAL(triggered()), _controller, SLOT(deleteSelections()));
    connect(ui->actionUndo_history, SIGNAL(triggered()), _controller, SLOT(showHistoryUndo()));

    // connect point actions
    connect(ui->actionAdd, SIGNAL(triggered()), _controller, SLOT(newPoint()));
    connect(ui->actionAlign, SIGNAL(triggered()), _controller, SLOT(projectStraightLinePoint()));
    connect(ui->actionPointCollapse, SIGNAL(triggered()), _controller, SLOT(collapsePoint()));
    connect(ui->actionInsert_plane, SIGNAL(triggered()), _controller, SLOT(insertPlane()));
    connect(ui->actionIntersect_layers, SIGNAL(triggered()), _controller, SLOT(intersectLayerPoint()));
    connect(ui->actionLock_points, SIGNAL(triggered()), _controller, SLOT(lockPoints()));
    connect(ui->actionUnlock_points, SIGNAL(triggered()), _controller, SLOT(unlockPoints()));
    connect(ui->actionUnlock_all_points, SIGNAL(triggered()), _controller, SLOT(unlockAllPoints()));

    // connect edge actions
    connect(ui->actionEdgeExtrude, SIGNAL(triggered()), _controller, SLOT(extrudeEdges()));
    connect(ui->actionEdgeSplit, SIGNAL(triggered()), _controller, SLOT(splitEdges()));
    connect(ui->actionEdgeCollapse, SIGNAL(triggered()), _controller, SLOT(collapseEdges()));
    connect(ui->actionEdgeInsert, SIGNAL(triggered()), _controller, SLOT(connectEdges()));
    connect(ui->actionEdgeCrease, SIGNAL(triggered()), _controller, SLOT(creaseEdges()));

    // connect curve actions
    connect(ui->actionCurveNew, SIGNAL(triggered()), _controller, SLOT(addCurve()));

    // connect face actions
    connect(ui->actionFaceNew, SIGNAL(triggered()),  _controller, SLOT(newFace()));
    connect(ui->actionFaceInvert, SIGNAL(triggered()), _controller, SLOT(flipFaces()));

    // connect layer actions
    connect(ui->actionActive_layer_color, SIGNAL(triggered()), _controller, SLOT(setActiveLayerColor()));
    connect(ui->actionLayerAuto_group, SIGNAL(triggered()), _controller, SLOT(autoGroupLayer()));
    connect(ui->actionLayerNew, SIGNAL(triggered()), _controller, SLOT(newLayer()));
    connect(ui->actionLayerDelete_empty, SIGNAL(triggered()), _controller, SLOT(deleteEmptyLayers()));
    connect(ui->actionLayerDialog, SIGNAL(triggered()), _controller, SLOT(layerDialog()));

    // connect visibility actions
    connect(ui->actionShowControl_net, SIGNAL(triggered(bool)), _controller, SLOT(showControlNet(bool)));
    connect(ui->actionShow_both_sides, SIGNAL(triggered(bool)), _controller, SLOT(showBothSides(bool)));
    connect(ui->actionShowControl_curves, SIGNAL(triggered(bool)), _controller, SLOT(showControlCurves(bool)));
    connect(ui->actionShowInterior_edges, SIGNAL(triggered(bool)), _controller, SLOT(showInteriorEdges(bool)));
    connect(ui->actionShowGrid, SIGNAL(triggered(bool)), _controller, SLOT(showGrid(bool)));
    connect(ui->actionShowStations, SIGNAL(triggered(bool)), _controller, SLOT(showStations(bool)));
    connect(ui->actionShowButtocks, SIGNAL(triggered(bool)), _controller, SLOT(showButtocks(bool)));
    connect(ui->actionShowWaterlines, SIGNAL(triggered(bool)), _controller, SLOT(showWaterlines(bool)));
    connect(ui->actionShowDiagonals, SIGNAL(triggered(bool)), _controller, SLOT(showDiagonals(bool)));
    connect(ui->actionShowHydrostatic_features, SIGNAL(triggered(bool)), _controller, SLOT(showHydroData(bool)));
    connect(ui->actionShowFlowlines, SIGNAL(triggered(bool)), _controller, SLOT(showFlowlines(bool)));
    connect(ui->actionShowNormals, SIGNAL(triggered(bool)), _controller, SLOT(showNormals(bool)));
    connect(ui->actionShowCurvature, SIGNAL(triggered(bool)), _controller, SLOT(showCurvature(bool)));
    connect(ui->actionShowMarkers, SIGNAL(triggered(bool)), _controller, SLOT(showMarkers(bool)));
    // inc/dec curvature

    // connect selection actions
    connect(ui->actionSelect_all, SIGNAL(triggered()), _controller, SLOT(selectAll()));
    connect(ui->actionDeselect_all, SIGNAL(triggered()), _controller, SLOT(clearSelections()));

    // connect tools actions
    connect(ui->actionImportMarkers, SIGNAL(triggered()), _controller, SLOT(importMarkers()));
    connect(ui->actionDelete_all_markers, SIGNAL(triggered()), _controller, SLOT(deleteMarkers()));
    connect(ui->actionCheck_model, SIGNAL(triggered()), _controller, SLOT(checkModel()));
    connect(ui->actionRemove_negative, SIGNAL(triggered()), _controller, SLOT(deleteNegativeFaces()));
    connect(ui->actionRemove_unused_points, SIGNAL(triggered()), _controller, SLOT(removeUnusedPoint()));
    connect(ui->actionDevelop_plates, SIGNAL(triggered()), _controller, SLOT(developLayers()));
    connect(ui->actionKeel_and_rudder_wizard, SIGNAL(triggered()), _controller, SLOT(keelAndRudderWizard()));
    connect(ui->actionAdd_cylinder, SIGNAL(triggered()), _controller, SLOT(addCylinder()));

    // connect transform actions
    connect(ui->actionScale, SIGNAL(triggered()), _controller, SLOT(scaleModel()));
    connect(ui->actionMove, SIGNAL(triggered()), _controller, SLOT(moveFaces()));
    connect(ui->actionRotate, SIGNAL(triggered()), _controller, SLOT(rotateFaces()));
    connect(ui->actionMirror, SIGNAL(triggered()), _controller, SLOT(mirrorPlaneFace()));
    connect(ui->actionLackenby, SIGNAL(triggered()), _controller, SLOT(lackenbyModelTransformation()));

    // connect calculations actions
    connect(ui->actionDelft_yacht_series, SIGNAL(triggered()), _controller, SLOT(delftResistance()));
    connect(ui->actionKAPER, SIGNAL(triggered()), _controller, SLOT(kaperResistance()));
    connect(ui->actionIntersections, SIGNAL(triggered()), _controller, SLOT(intersectionDialog()));
    connect(ui->actionDesign_Hydrostatics, SIGNAL(triggered()), _controller, SLOT(calculateHydrostatics()));
    connect(ui->actionHydrostatics, SIGNAL(triggered()), _controller, SLOT(hydrostaticsDialog()));
    connect(ui->actionCross_curves, SIGNAL(triggered()), _controller, SLOT(crossCurvesHydrostatics()));

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

void MainWindow::addDefaultViewports()
{
    ShipCADModel* model = _controller->getModel();
    int row = 0;
    int col = 0;
    viewport_type_t ty;
    for (int i=0; i<4; i++) {
        switch (i) {
        case 0:
            row = 0; col = 0;
            ty = fvPerspective;
            break;
        case 1:
            row = 0; col = 1;
            ty = fvProfile;
            break;
        case 2:
            row = 1; col = 0;
            ty = fvPlan;
            break;
        case 3:
            row = 1; col = 1;
            ty = fvBodyplan;
            break;
        }

        // make the viewport
        Viewport* vp = new Viewport(_controller, ty);
        // connect the render signal to the viewport
        //connect(this, SIGNAL(viewportRender()), vp, SLOT(renderLater()));
        connect(this, SIGNAL(viewportRender()), vp, SLOT(renderNow()));
        ViewportContainer* vpcontainer = new ViewportContainer(vp, this);

        // put it in display area
        ui->displayLayout->addWidget(vpcontainer, row, col);
        _viewports.push_back(make_pair(vpcontainer, vp));
    }
    cout << "addDefaultViewports" << endl;
}

void MainWindow::updateVisibilityActions()
{
    Visibility& vis = _controller->getModel()->getVisibility();

    ui->actionShowControl_net->setChecked(vis.isShowControlNet());
    ui->actionShow_both_sides->setChecked(vis.getModelView() == mvBoth);
    ui->actionShowControl_curves->setChecked(vis.isShowControlCurves());
    ui->actionShowInterior_edges->setChecked(vis.isShowInteriorEdges());
    ui->actionShowGrid->setChecked(vis.isShowGrid());
    ui->actionShowStations->setChecked(vis.isShowStations());
    ui->actionShowButtocks->setChecked(vis.isShowButtocks());
    ui->actionShowWaterlines->setChecked(vis.isShowWaterlines());
    ui->actionShowDiagonals->setChecked(vis.isShowDiagonals());
    ui->actionShowHydrostatic_features->setChecked(vis.isShowHydroData());
    ui->actionShowFlowlines->setChecked(vis.isShowFlowlines());
    ui->actionShowNormals->setChecked(vis.isShowNormals());
    ui->actionShowCurvature->setChecked(vis.isShowCurvature());
    ui->actionShowMarkers->setChecked(vis.isShowMarkers());

//    ui->actionShade_Underwater->setChecked(s->shadeUnderWater());
    emit viewportRender();
    cout << "updateVisibilityActions" << endl;
}

void MainWindow::modelLoaded()
{
    cout << "modelLoaded" << endl;
    addDefaultViewports();
    enableActions();
    updateVisibilityActions();
}

void MainWindow::enableActions()
{
    // file actions
    ui->actionSave_As->setEnabled(true);
    ui->actionExportArchimedes->setEnabled(true);
    ui->actionExportCoordinates->setEnabled(true);
    ui->actionExportDXF_2D_Polylines->setEnabled(true);
    ui->actionExportDXF_3D_mesh->setEnabled(true);
    ui->actionExportDXF_3D_Polylines->setEnabled(true);
    ui->actionExportFEF->setEnabled(true);
    ui->actionExportGHS->setEnabled(true);
    ui->actionExportPart->setEnabled(true);
    ui->actionExportMichlet->setEnabled(true);
    ui->actionExportWavefront->setEnabled(true);
    ui->actionExportOffsets->setEnabled(true);
    ui->actionExportSTL->setEnabled(true);
    ui->actionExportIGES->setEnabled(true);

    // edit actions

    // point actions
    ui->actionAdd->setEnabled(true);
    ui->actionAlign->setEnabled(true);
    ui->actionPointCollapse->setEnabled(true);
    ui->actionInsert_plane->setEnabled(true);
    ui->actionIntersect_layers->setEnabled(true);
    ui->actionLock_points->setEnabled(true);
    ui->actionUnlock_points->setEnabled(true);
    ui->actionUnlock_all_points->setEnabled(true);

    // edge actions

    // face actions

    // layer actions

    // selection actions
    ui->actionSelect_all->setEnabled(true);

    // tools actions

    // transform actions

    // calculations actions

    cout << "enableActions" << endl;
}

// add the menu entry to the file menu, and create all the actions...
void MainWindow::createRecentFilesMenu()
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

void MainWindow::modelChanged()
{
    bool changed = _controller->getModel()->isFileChanged();
    if (changed)
        setWindowTitle(tr("ShipCAD") + tr(" (modified)"));
    else
        setWindowTitle(tr("ShipCAD"));
    cout << "model changed" << endl;
    ui->actionSave->setEnabled(changed);
}

void MainWindow::showControlPointDialog(bool show)
{
    if (_pointdialog == 0) {
        _pointdialog = new PointDialog(this);
        connect(_pointdialog, SIGNAL(cornerPointSelect(bool)), _controller, SLOT(cornerPointSelected(bool)));
        connect(_pointdialog, SIGNAL(pointCoordChange(float,float,float)),
                _controller, SLOT(dialogUpdatedPointCoord(float,float,float)));
        connect(_controller, SIGNAL(updateControlPointValue(ShipCAD::SubdivisionControlPoint*)),
                _pointdialog, SLOT(controllerUpdatedPoint(ShipCAD::SubdivisionControlPoint*)));
    }
    _pointdialog->setActive(show);
    cout << "show control point dialog:" << (show ? "t" : "f") << endl;
}

void MainWindow::changeSelectedItems()
{
    if (_controller->getModel()->countSelectedItems() > 0) {
        ui->actionDeselect_all->setEnabled(true);
    } else {
        ui->actionDeselect_all->setEnabled(false);
    }
    cout << "changeSelectedItems" << endl;
}
