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
#include <QFileDialog>
#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pointdialog.h"
#include "viewport.h"
#include "shipcadlib.h"
#include "shipcadmodel.h"
#include "controller.h"
#include "viewportcontainer.h"
#include "utility.h"

using namespace ShipCAD;
using namespace std;

MainWindow::MainWindow(Controller* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _pointdialog(0),
    _controller(c), _menu_recent_files(0), _contextMenu(0),
    _viewportModeGroup(0),
    _wireframeAct(0), _shadeAct(0), _gaussCurvAct(0), _zebraAct(0),
    _developCheckAct(0), _currentViewportContext(0)
{
    ui->setupUi(this);
    createToolBars();
    createStatusBar();
    createRecentFilesMenu();
    createActions();
    createMenus();
    
    // connect mainwindow actions
    connect(ui->actionFileNew, SIGNAL(triggered()), SLOT(newModel()));
    connect(ui->actionOpen, SIGNAL(triggered()), SLOT(openModelFile()));
    connect(ui->actionSave, SIGNAL(triggered()), SLOT(saveModelFile()));
    connect(ui->actionSave_As, SIGNAL(triggered()), SLOT(saveModelAsFile()));

    // connect controller signals
    connect(_controller, SIGNAL(updateUndoData()), SLOT(updateUndoData()));
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

void MainWindow::createActions()
{
    _wireframeAct = new QAction(tr("Wireframe"), this);
    _wireframeAct->setCheckable(true);
    _shadeAct = new QAction(tr("Shade"), this);
    _shadeAct->setCheckable(true);
    _gaussCurvAct = new QAction(tr("Gaussian Curvature"), this);
    _gaussCurvAct->setCheckable(true);
    _zebraAct = new QAction(tr("Zebra Shading"), this);
    _zebraAct->setCheckable(true);
    _developCheckAct = new QAction(tr("Developability Check"), this);
    _developCheckAct->setCheckable(true);

    _viewportModeGroup = new QActionGroup(this);
    _viewportModeGroup->addAction(_wireframeAct);
    _viewportModeGroup->addAction(_shadeAct);
    _viewportModeGroup->addAction(_gaussCurvAct);
    _viewportModeGroup->addAction(_zebraAct);
    _viewportModeGroup->addAction(_developCheckAct);
    _wireframeAct->setChecked(true);
}

void MainWindow::createMenus()
{
    // the context menu
    _contextMenu = new QMenu(this);
    QMenu* modeMenu = _contextMenu->addMenu(tr("Mode"));
    modeMenu->addAction(_wireframeAct);
    modeMenu->addAction(_shadeAct);
    modeMenu->addAction(_gaussCurvAct);
    modeMenu->addAction(_zebraAct);
    modeMenu->addAction(_developCheckAct);
    connect(_wireframeAct, SIGNAL(triggered()), this, SLOT(wireFrame()));
    connect(_shadeAct, SIGNAL(triggered()), this, SLOT(shade()));
    connect(_gaussCurvAct, SIGNAL(triggered()), this, SLOT(shadeCurvature()));
    connect(_zebraAct, SIGNAL(triggered()), this, SLOT(shadeZebra()));
    connect(_developCheckAct, SIGNAL(triggered()), this, SLOT(shadeDevelopable()));
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
        connect(vp, SIGNAL(contextMenuEvent(ShipCAD::ViewportContextEvent*)),
                this, SLOT(vpContextMenuEvent(ShipCAD::ViewportContextEvent*)));
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
    ui->actionShowHydrostatic_features->setChecked(vis.isShowHydrostaticData());
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

void MainWindow::openRecentFile()
{
    cout << "openRecentFile" << endl;
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        _controller->loadFile(action->data().toString());
    }
}

void MainWindow::newModel()
{
    ShipCADModel* model = _controller->getModel();
    if (model != 0 && model->isFileChanged()) {
        saveModelFile();
    }
    _controller->newModel();
}

void MainWindow::openModelFile()
{
    // get last directory
    QSettings settings;
    QString lastdir;
    if (settings.contains("file/opendir")) {
        lastdir = settings.value("file/opendir").toString();
    }
    // get the filename
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"), lastdir);
    if (filename.length() == 0)
        return;
    QFileInfo fi(filename);
    QString filepath = fi.filePath();
    settings.setValue("file/opendir", filepath);
    addRecentFiles(filename);
    _controller->loadFile(filename);
}

void MainWindow::saveModelFile()
{
    QFileInfo f(_controller->getModel()->getFilename());
    cout << "saveModelFile:" << f.absoluteFilePath().toStdString() << endl;
    _controller->saveFile();
    addRecentFiles(f.absoluteFilePath());
    QSettings settings;
    settings.setValue("file/savedir", f.absolutePath());
}

void MainWindow::saveModelAsFile()
{
    cout << "saveModelAsFile\n";
    // get last directory
    QSettings settings;
    QString lastdir;
    if (settings.contains("file/savedir")) {
        lastdir = settings.value("file/savedir").toString();
    }
    // get the filename
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                    lastdir,
                                                    tr("freeship (*.fbm)"));
    if (filename.length() == 0)
        return;
    QFileInfo fi(filename);
    QString filepath = fi.filePath();
    _controller->saveFileAs(filename);
    addRecentFiles(fi.absoluteFilePath());
    settings.setValue("file/savedir", fi.absolutePath());
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
    if (_currentViewportContext == 0)
        return;
    if (_currentViewportContext->getViewportMode() != vmWireFrame)
        _currentViewportContext->setViewportMode(vmWireFrame);
    cout << "Viewport mode Wire Frame" << endl;
}

void
MainWindow::shade()
{
    if (_currentViewportContext == 0)
        return;
    if (_currentViewportContext->getViewportMode() != vmShade)
        _currentViewportContext->setViewportMode(vmShade);
    cout << "Viewport mode shade" << endl;
}

void
MainWindow::shadeCurvature()
{
    if (_currentViewportContext == 0)
        return;
    if (_currentViewportContext->getViewportMode() != vmShadeGauss)
        _currentViewportContext->setViewportMode(vmShadeGauss);
    cout << "Viewport mode shade gauss" << endl;
}

void
MainWindow::shadeDevelopable()
{
    if (_currentViewportContext == 0)
        return;
    if (_currentViewportContext->getViewportMode() != vmShadeDevelopable)
        _currentViewportContext->setViewportMode(vmShadeDevelopable);
    cout << "Viewport mode shade developable" << endl;
}

void
MainWindow::shadeZebra()
{
    if (_currentViewportContext == 0)
        return;
    if (_currentViewportContext->getViewportMode() != vmShadeZebra)
        _currentViewportContext->setViewportMode(vmShadeZebra);
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
    const QStringList& filelist = getRecentFiles();
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

const QStringList& MainWindow::getRecentFiles() const
{
	return _recent_files;
}

void MainWindow::addRecentFiles(const QString& filename)
{
	bool already_present = false;
	QString basename(ChangeFileExt(filename, ""));
    for (int i=0; i<_recent_files.size(); i++) {
		if (QString::compare(basename, _recent_files[i], Qt::CaseInsensitive) == 0) {
			already_present = true;
			_recent_files.removeAt(i);
			_recent_files.push_front(filename);
			break;
		}
	}
	if (!already_present) {
		if (_recent_files.size() == 10) {
			_recent_files.pop_back();
		}
		_recent_files.push_front(filename);
	}
    changeRecentFiles();
}

void MainWindow::vpContextMenuEvent(ViewportContextEvent* event)
{
    cout << "MainWindow::contextMenuEvent" << endl;
    if (_contextMenu == 0)
        return;
    _currentViewportContext = event->getViewport();
    _contextMenu->exec(event->getMouseEvent()->globalPos());
    cout << "MainWindow::contextMenuEvent finished" << endl;
}
