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
#include <QMessageBox>
#include <QDesktopWidget>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pointdialog.h"
#include "insertplanepointsdialog.h"
#include "intersectlayersdialog.h"
#include "extrudeedgedialog.h"
#include "chooselayerdialog.h"
#include "mirrordialog.h"
#include "rotatedialog.h"
#include "layerdialog.h"
#include "viewport.h"
#include "shipcadmodel.h"
#include "subdivlayer.h"
#include "controller.h"
#include "viewportcontainer.h"
#include "utility.h"
#include "colorview.h"

using namespace ShipCAD;
using namespace std;

MainWindow::MainWindow(Controller* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), _pointdialog(nullptr), _planepointsdialog(nullptr),
    _intersectlayersdialog(nullptr), _extrudeedgedialog(nullptr), _chooselayerdialog(nullptr),
    _mirrordialog(nullptr), _rotatedialog(nullptr), _layerdialog(nullptr),
    _controller(c), _currentViewportContext(nullptr),
    _menu_recent_files(nullptr), _contextMenu(nullptr), _cameraMenu(nullptr),
    _viewportModeGroup(nullptr),
    _wireframeAction(nullptr), _shadeAction(nullptr), _gaussCurvAction(nullptr), _zebraAction(nullptr),
    _developCheckAction(nullptr),
    _viewGroup(nullptr),
    _perspectiveAction(nullptr), _bodyPlanAction(nullptr),
    _profileAction(nullptr), _planViewAction(nullptr), _zoomInAction(nullptr), _zoomOutAction(nullptr),
    _zoomAllAction(nullptr), _printAction(nullptr), _saveImageAction(nullptr),
    _cameraGroup(nullptr),
    _wideLensAction(nullptr), _stdLensAction(nullptr), _shortLensAction(nullptr), _medLensAction(nullptr),
    _longLensAction(nullptr),
    _visibleBgImgAction(nullptr), _clearBgImgAction(nullptr), _loadBgImgAction(nullptr),
    _saveBgImgAction(nullptr), _originBgImgAction(nullptr), _scaleBgImgAction(nullptr),
    _alphaBgImgAction(nullptr), _tolBgImgAction(nullptr), _blendBgImgAction(nullptr),
    _fileToolBar(nullptr), _visToolBar(nullptr), _layerToolBar(nullptr), _pointToolBar(nullptr),
    _modToolBar(nullptr), _precisionComboBox(nullptr), _activeLayerComboBox(nullptr),
    _colorView(nullptr)
{
    ui->setupUi(this);
    setIcons();
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
    connect(_controller, SIGNAL(changeActiveLayer()), SLOT(enableActions()));
    connect(_controller, SIGNAL(modifiedModel()), SLOT(modelChanged()));
    connect(_controller, SIGNAL(modifiedModel()), SLOT(enableActions()));
    connect(_controller, SIGNAL(modifiedModel()), SIGNAL(viewportRender()));
    connect(_controller, SIGNAL(modifiedModel()), SLOT(updateVisibilityActions()));
    connect(_controller, SIGNAL(showControlPointDialog(bool)),
            SLOT(showControlPointDialog(bool)));
    connect(_controller, SIGNAL(modelLoaded()), SLOT(modelLoaded()));
    connect(_controller, SIGNAL(modelLoaded()), SLOT(enableActions()));
    connect(_controller, SIGNAL(modelLoaded()), SLOT(updateVisibilityActions()));
    connect(_controller, SIGNAL(modelLoaded()), SLOT(modelChanged()));
    connect(_controller, SIGNAL(changeSelectedItems()), SLOT(enableActions()));
    connect(_controller, SIGNAL(changeSelectedItems()), SLOT(changeSelectedItems()));
    connect(_controller,
            SIGNAL(exeInsertPlanePointsDialog(ShipCAD::InsertPlaneDialogData&)),
            SLOT(executeInsertPlanePointsDialog(ShipCAD::InsertPlaneDialogData&)));
    connect(_controller,
            SIGNAL(exeIntersectLayersDialog(ShipCAD::IntersectLayersDialogData&)),
            SLOT(executeIntersectLayersDialog(ShipCAD::IntersectLayersDialogData&)));
    connect(_controller,
            SIGNAL(exeExtrudeEdgeDialog(ShipCAD::ExtrudeEdgeDialogData&)),
            SLOT(executeExtrudeEdgeDialog(ShipCAD::ExtrudeEdgeDialogData&)));
    connect(_controller,
            SIGNAL(exeChooseColorDialog(ShipCAD::ChooseColorDialogData&)),
            SLOT(executeChooseColorDialog(ShipCAD::ChooseColorDialogData&)));
    connect(_controller,
            SIGNAL(exeChooseLayerDialog(ShipCAD::ChooseLayerDialogData&)),
            SLOT(executeChooseLayerDialog(ShipCAD::ChooseLayerDialogData&)));
    connect(_controller,
            SIGNAL(exeMirrorDialog(ShipCAD::MirrorDialogData&)),
            SLOT(executeMirrorDialog(ShipCAD::MirrorDialogData&)));
    connect(_controller,
            SIGNAL(exeRotateDialog(ShipCAD::RotateDialogData&)),
            SLOT(executeRotateDialog(ShipCAD::RotateDialogData&)));
    connect(_controller,
            SIGNAL(displayInfoDialog(const QString&)),
            SLOT(showInfoDialog(const QString&)));
    connect(_controller, SIGNAL(displayWarningDialog(const QString&)),
            SLOT(showWarnDialog(const QString&)));
    connect(_controller, SIGNAL(displayErrorDialog(const QString&)),
            SLOT(showErrDialog(const QString&)));
    connect(_controller, SIGNAL(displayQuestionDialog(const QString&, bool&)),
            SLOT(showQuestionDialog(const QString&, bool&)));

    // connect file actions
    connect(ui->actionImportCarene, SIGNAL(triggered()), _controller, SLOT(importCarene()));
    connect(ui->actionImportChines, SIGNAL(triggered()), _controller, SLOT(importChines()));
    connect(ui->actionImportFEF, SIGNAL(triggered()), _controller, SLOT(importFEF()));
    connect(ui->actionImportPart, SIGNAL(triggered()), _controller, SLOT(importPart()));
    connect(ui->actionImportPolyCad, SIGNAL(triggered()), _controller, SLOT(importPolycad()));
    connect(ui->actionImportSurface, SIGNAL(triggered()), _controller, SLOT(importSurface()));
    connect(ui->actionImportVRML, SIGNAL(triggered()), _controller, SLOT(importVRML()));
    connect(ui->actionImportMichlet_waves, SIGNAL(triggered()),
            _controller, SLOT(importMichletWaves()));
    connect(ui->actionImportCarlson, SIGNAL(triggered()), _controller, SLOT(importHull()));
    connect(ui->actionExportArchimedes, SIGNAL(triggered()),
            _controller, SLOT(exportFileArchimedes()));
    connect(ui->actionExportCoordinates, SIGNAL(triggered()),
            _controller, SLOT(exportCoordinates()));
    connect(ui->actionExportDXF_2D_Polylines, SIGNAL(triggered()),
            _controller, SLOT(export2DPolylinesDXF()));
    connect(ui->actionExportDXF_3D_mesh, SIGNAL(triggered()),
            _controller, SLOT(exportFacesDXF()));
    connect(ui->actionExportDXF_3D_Polylines, SIGNAL(triggered()),
            _controller, SLOT(export3DPolylinesDXF()));
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
    connect(ui->actionIntersect_layers, SIGNAL(triggered()),
            _controller, SLOT(intersectLayerPoint()));
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
    connect(ui->actionActive_layer_color, SIGNAL(triggered()),
            _controller, SLOT(setActiveLayerColor()));
    connect(ui->actionLayerAuto_group, SIGNAL(triggered()), _controller, SLOT(autoGroupLayer()));
    connect(ui->actionLayerNew, SIGNAL(triggered()), _controller, SLOT(newLayer()));
    connect(ui->actionLayerDelete_empty, SIGNAL(triggered()),
            _controller, SLOT(deleteEmptyLayers()));
    connect(ui->actionLayerDialog, SIGNAL(triggered()), SLOT(displayLayerDialog()));
    
    // connect visibility actions
    connect(ui->actionShowControl_net, SIGNAL(triggered(bool)),
            _controller, SLOT(showControlNet(bool)));
    connect(ui->actionShow_both_sides, SIGNAL(triggered(bool)),
            _controller, SLOT(showBothSides(bool)));
    connect(ui->actionShowControl_curves, SIGNAL(triggered(bool)),
            _controller, SLOT(showControlCurves(bool)));
    connect(ui->actionShowInterior_edges, SIGNAL(triggered(bool)),
            _controller, SLOT(showInteriorEdges(bool)));
    connect(ui->actionShowGrid, SIGNAL(triggered(bool)),
            _controller, SLOT(showGrid(bool)));
    connect(ui->actionShowStations, SIGNAL(triggered(bool)),
            _controller, SLOT(showStations(bool)));
    connect(ui->actionShowButtocks, SIGNAL(triggered(bool)),
            _controller, SLOT(showButtocks(bool)));
    connect(ui->actionShowWaterlines, SIGNAL(triggered(bool)),
            _controller, SLOT(showWaterlines(bool)));
    connect(ui->actionShowDiagonals, SIGNAL(triggered(bool)),
            _controller, SLOT(showDiagonals(bool)));
    connect(ui->actionShowHydrostatic_features, SIGNAL(triggered(bool)),
            _controller, SLOT(showHydroData(bool)));
    connect(ui->actionShowFlowlines, SIGNAL(triggered(bool)),
            _controller, SLOT(showFlowlines(bool)));
    connect(ui->actionShowNormals, SIGNAL(triggered(bool)),
            _controller, SLOT(showNormals(bool)));
    connect(ui->actionShowCurvature, SIGNAL(triggered(bool)),
            _controller, SLOT(showCurvature(bool)));
    connect(ui->actionShowMarkers, SIGNAL(triggered(bool)),
            _controller, SLOT(showMarkers(bool)));
    // inc/dec curvature

    // connect selection actions
    connect(ui->actionSelect_all, SIGNAL(triggered()), _controller, SLOT(selectAll()));
    connect(ui->actionDeselect_all, SIGNAL(triggered()), _controller, SLOT(clearSelections()));

    // connect tools actions
    connect(ui->actionImportMarkers, SIGNAL(triggered()), _controller, SLOT(importMarkers()));
    connect(ui->actionDelete_all_markers, SIGNAL(triggered()), _controller, SLOT(deleteMarkers()));
    connect(ui->actionCheck_model, SIGNAL(triggered()), _controller, SLOT(checkModel()));
    connect(ui->actionRemove_negative, SIGNAL(triggered()),
            _controller, SLOT(deleteNegativeFaces()));
    connect(ui->actionRemove_unused_points, SIGNAL(triggered()),
            _controller, SLOT(removeUnusedPoint()));
    connect(ui->actionDevelop_plates, SIGNAL(triggered()),
            _controller, SLOT(developLayers()));
    connect(ui->actionKeel_and_rudder_wizard, SIGNAL(triggered()),
            _controller, SLOT(keelAndRudderWizard()));
    connect(ui->actionAdd_cylinder, SIGNAL(triggered()), _controller, SLOT(addCylinder()));

    // connect transform actions
    connect(ui->actionScale, SIGNAL(triggered()), _controller, SLOT(scaleFaces()));
    connect(ui->actionMove, SIGNAL(triggered()), _controller, SLOT(moveFaces()));
    connect(ui->actionRotate, SIGNAL(triggered()), _controller, SLOT(rotateFaces()));
    connect(ui->actionMirror, SIGNAL(triggered()), _controller, SLOT(mirrorPlaneFace()));
    connect(ui->actionLackenby, SIGNAL(triggered()),
            _controller, SLOT(lackenbyModelTransformation()));

    // connect calculations actions
    connect(ui->actionDelft_yacht_series, SIGNAL(triggered()), _controller, SLOT(delftResistance()));
    connect(ui->actionKAPER, SIGNAL(triggered()), _controller, SLOT(kaperResistance()));
    connect(ui->actionIntersections, SIGNAL(triggered()), _controller, SLOT(intersectionDialog()));
    connect(ui->actionDesign_Hydrostatics, SIGNAL(triggered()),
            _controller, SLOT(calculateHydrostatics()));
    connect(ui->actionHydrostatics, SIGNAL(triggered()), _controller, SLOT(hydrostaticsDialog()));
    connect(ui->actionCross_curves, SIGNAL(triggered()),
            _controller, SLOT(crossCurvesHydrostatics()));

    // connect window actions

    // connect about actions

    // set action status
    enableActions();
    updateVisibilityActions();
    
    // restore settings
    readSettings();
    
    ui->statusBar->showMessage(tr("Ready"));
}

MainWindow::~MainWindow()
{
    deleteViewports();
    delete ui;
}

void MainWindow::setIcons()
{
    // file actions
    ui->actionFileNew->setIcon(QIcon(":/Themes/Default/icons/32/NewModel.png"));
    ui->actionOpen->setIcon(QIcon(":/Themes/Default/icons/32/LoadFile.png"));
    ui->actionSave->setIcon(QIcon(":/Themes/Default/icons/32/FileSave.png"));
    ui->actionSave_As->setIcon(QIcon(":/Themes/Default/icons/32/FileSaveas.png"));
    ui->actionExit->setIcon(QIcon(":/Themes/Default/icons/32/ExitProgram.png"));

    // edit
    ui->actionUndo->setIcon(QIcon(":/Themes/Default/icons/32/Undo.png"));
    ui->actionRedo->setIcon(QIcon(":/Themes/Default/icons/32/Redo.png"));
    ui->actionDelete->setIcon(QIcon(":/Themes/Default/icons/32/DeleteAll.png"));

    // points
    ui->actionAlign->setIcon(QIcon(":/Themes/Default/icons/32/PointAlign.png"));
    ui->actionPointCollapse->setIcon(QIcon(":/Themes/Default/icons/32/PointCollapse.png"));
    ui->actionInsert_plane->setIcon(QIcon(":/Themes/Default/icons/32/InsertPlane.png"));
    ui->actionIntersect_layers->setIcon(QIcon(":/Themes/Default/icons/32/LayerIntersection.png"));
    ui->actionLock_points->setIcon(QIcon(":/Themes/Default/icons/32/PointsLock.png"));
    ui->actionUnlock_points->setIcon(QIcon(":/Themes/Default/icons/32/PointsUnlock.png"));
    ui->actionUnlock_all_points->setIcon(QIcon(":/Themes/Default/icons/32/PointsUnlockAll.png"));

    // edge
    ui->actionEdgeExtrude->setIcon(QIcon(":/Themes/Default/icons/32/EdgeExtrude.png"));
    ui->actionEdgeSplit->setIcon(QIcon(":/Themes/Default/icons/32/EdgeSplit.png"));
    ui->actionEdgeCollapse->setIcon(QIcon(":/Themes/Default/icons/32/EdgeCollapse.png"));
    ui->actionEdgeInsert->setIcon(QIcon(":/Themes/Default/icons/32/NewEdge.png"));
    ui->actionEdgeCrease->setIcon(QIcon(":/Themes/Default/icons/32/EdgeCrease.png"));

    // curve
    ui->actionCurveNew->setIcon(QIcon(":/Themes/Default/icons/32/NewCurve.png"));

    // face
    ui->actionFaceNew->setIcon(QIcon(":/Themes/Default/icons/32/NewFace.png"));
    ui->actionFaceInvert->setIcon(QIcon(":/Themes/Default/icons/32/InvertFace.png"));

    // layer
    ui->actionActive_layer_color->setIcon(QIcon(":/Themes/Default/icons/32/ActiveLayerColor.png"));
    ui->actionLayerAuto_group->setIcon(QIcon(":/Themes/Default/icons/32/LayerAutoGroup.png"));
    ui->actionLayerNew->setIcon(QIcon(":/Themes/Default/icons/32/NewLayer.png"));
    ui->actionLayerDelete_empty->setIcon(QIcon(":/Themes/Default/icons/32/DeleteEmptyLayers.png"));
    ui->actionLayerDialog->setIcon(QIcon(":/Themes/Default/icons/32/LayerDialog.png"));

    // visibility
    ui->actionShowControl_net->setIcon(QIcon(":/Themes/Default/icons/32/ShowControlNet.png"));
    ui->actionShow_both_sides->setIcon(QIcon(":/Themes/Default/icons/32/BothSides.png"));
    ui->actionShowControl_curves->setIcon(QIcon(":/Themes/Default/icons/32/ShowControlCurves.png"));
    ui->actionShowInterior_edges->setIcon(QIcon(":/Themes/Default/icons/32/ShowInteriorEdges.png"));
    ui->actionShowGrid->setIcon(QIcon(":/Themes/Default/icons/32/ShowGrid.png"));
    ui->actionShowStations->setIcon(QIcon(":/Themes/Default/icons/32/ShowStations.png"));
    ui->actionShowButtocks->setIcon(QIcon(":/Themes/Default/icons/32/ShowButtocks.png"));
    ui->actionShowWaterlines->setIcon(QIcon(":/Themes/Default/icons/32/ShowWaterlines.png"));
    ui->actionShowDiagonals->setIcon(QIcon(":/Themes/Default/icons/32/ShowDiagonals.png"));
    ui->actionShowHydrostatic_features->setIcon(QIcon(":/Themes/Default/icons/32/ShowHydrostatics.png"));
    ui->actionShowFlowlines->setIcon(QIcon(":/Themes/Default/icons/32/ShowFlowlines.png"));
    ui->actionShowNormals->setIcon(QIcon(":/Themes/Default/icons/32/ShowNormals.png"));
    ui->actionShowCurvature->setIcon(QIcon(":/Themes/Default/icons/32/Showcurvature.png"));
    ui->actionShowMarkers->setIcon(QIcon(":/Themes/Default/icons/32/ShowMarkers.png"));

    // selection
    ui->actionDeselect_all->setIcon(QIcon(":/Themes/Default/icons/32/DeselectAll.png"));
    
    // tools
    ui->actionDelete_all_markers->setIcon(QIcon(":/Themes/Default/icons/32/DeleteMarkers.png"));
    ui->actionKeel_and_rudder_wizard->setIcon(QIcon(":/Themes/Default/icons/32/KeelRudderWizard.png"));

    // transform

    // calculations
    ui->actionIntersections->setIcon(QIcon(":/Themes/Default/icons/32/IntersectionDialog.png"));
}

void MainWindow::deleteViewports()
{
    for (size_t i=0; i<_viewports.size(); i++) {
        delete _viewports[i].viewport();
    }
    _viewports.clear();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (_controller->getModel()->isFileChanged()) {
        QMessageBox msgBox;
        // msg 0103
        msgBox.setText(tr("The current model has been changed!"));
        // msg 0282
        msgBox.setInformativeText(tr("Are you sure you want to quit?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No) {
            saveModelFile();
        }
    }
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    saveViewports();
    QMainWindow::closeEvent(event);
}

void MainWindow::showInfoDialog(const QString& msg)
{
    QMessageBox::information(this, "ShipCAD",
                             msg,
                             QMessageBox::Ok);
}

void MainWindow::showWarnDialog(const QString& msg)
{
    QMessageBox::warning(this, "ShipCAD",
                         msg,
                         QMessageBox::Ok);
}

void MainWindow::showErrDialog(const QString& msg)
{
    QMessageBox::critical(this, "ShipCAD",
                          msg,
                          QMessageBox::Ok);
}

void MainWindow::showQuestionDialog(const QString& msg, bool& ok)
{
    QMessageBox::StandardButton btn = QMessageBox::question(this, "ShipCAD", msg);
    if (btn == QMessageBox::StandardButton::Yes)
        ok = true;
    else
        ok = false;
}

void MainWindow::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    const QByteArray windowState = settings.value("windowstate", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 2, availableGeometry.height() / 1.5);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }
}

void MainWindow::createToolBars()
{
    _fileToolBar = addToolBar(tr("File"));
    _fileToolBar->setObjectName("filetb");

    _fileToolBar->insertAction(0, ui->actionFileNew);
    _fileToolBar->insertAction(0, ui->actionOpen);
    _fileToolBar->insertAction(0, ui->actionSave);
    _fileToolBar->insertAction(0, ui->actionExit);

    _visToolBar = addToolBar(tr("Visibility"));
    _visToolBar->setObjectName("vistb");

    // precision combo box
    _precisionComboBox = new QComboBox();
    _precisionComboBox->addItem(tr("Low"));
    _precisionComboBox->addItem(tr("Medium"));
    _precisionComboBox->addItem(tr("High"));
    _precisionComboBox->addItem(tr("Very High"));
    _precisionComboBox->setCurrentIndex(1);
    connect(_precisionComboBox, SIGNAL(activated(int)), SLOT(precisionChanged(int)));
    _visToolBar->addWidget(_precisionComboBox);

    _visToolBar->insertAction(0, ui->actionShowControl_net);
    _visToolBar->insertAction(0, ui->actionShowControl_curves);
    _visToolBar->insertAction(0, ui->actionShowInterior_edges);
    _visToolBar->insertAction(0, ui->actionShow_both_sides);
    _visToolBar->insertAction(0, ui->actionShowGrid);
    _visToolBar->insertAction(0, ui->actionShowStations);
    _visToolBar->insertAction(0, ui->actionShowButtocks);
    _visToolBar->insertAction(0, ui->actionShowWaterlines);
    _visToolBar->insertAction(0, ui->actionShowDiagonals);
    _visToolBar->insertAction(0, ui->actionShowHydrostatic_features);
    _visToolBar->insertAction(0, ui->actionShowFlowlines);
    _visToolBar->insertAction(0, ui->actionShowNormals);
    _visToolBar->insertAction(0, ui->actionShowCurvature);
    _visToolBar->insertAction(0, ui->actionShowMarkers);

    // layers
    _layerToolBar = addToolBar(tr("Layers"));
    _layerToolBar->setObjectName("layertb");
    
    _layerToolBar->insertAction(0, ui->actionLayerNew);
    _layerToolBar->insertAction(0, ui->actionLayerDelete_empty);
    _layerToolBar->insertAction(0, ui->actionLayerDialog);
    _layerToolBar->insertAction(0, ui->actionLayerAuto_group);

    _activeLayerComboBox = new QComboBox();
    _activeLayerComboBox->addItem(tr("0"));
    _activeLayerComboBox->setCurrentIndex(0);
    _activeLayerComboBox->setMinimumWidth(100);
    connect(_activeLayerComboBox, SIGNAL(activated(int)), _controller, SLOT(setActiveLayer(int)));
    _layerToolBar->addWidget(_activeLayerComboBox);

    _colorView = new ColorView(Qt::black);
    _colorView->setMinimumSize(32, 32);
    _colorView->setMaximumSize(32, 32);
    _layerToolBar->addWidget(_colorView);
    
    // pick
    _pointToolBar = addToolBar(tr("Points"));
    _pointToolBar->setObjectName("pointtb");

    _pointToolBar->insertAction(0, ui->actionDelete);
    _pointToolBar->insertAction(0, ui->actionAlign);
    _pointToolBar->insertAction(0, ui->actionPointCollapse);
    _pointToolBar->insertAction(0, ui->actionInsert_plane);
    _pointToolBar->insertAction(0, ui->actionIntersect_layers);
    _pointToolBar->insertAction(0, ui->actionLock_points);
    _pointToolBar->insertAction(0, ui->actionUnlock_points);
    _pointToolBar->insertAction(0, ui->actionUnlock_all_points);
    
    _modToolBar = addToolBar(tr("Modification"));
    _modToolBar->setObjectName("modtb");
    
    _modToolBar->insertAction(0, ui->actionEdgeExtrude);
    _modToolBar->insertAction(0, ui->actionEdgeSplit);
    _modToolBar->insertAction(0, ui->actionEdgeCollapse);
    _modToolBar->insertAction(0, ui->actionEdgeInsert);
    _modToolBar->insertAction(0, ui->actionEdgeCrease);
    _modToolBar->insertAction(0, ui->actionCurveNew);
    _modToolBar->insertAction(0, ui->actionFaceNew);
    _modToolBar->insertAction(0, ui->actionFaceInvert);
    _modToolBar->insertAction(0, ui->actionIntersections);
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
    _wireframeAction = new QAction(tr("Wireframe"), this);
    _wireframeAction->setCheckable(true);
    _shadeAction = new QAction(tr("Shade"), this);
    _shadeAction->setCheckable(true);
    _gaussCurvAction = new QAction(tr("Gaussian curvature"), this);
    _gaussCurvAction->setCheckable(true);
    _zebraAction = new QAction(tr("Zebra shading"), this);
    _zebraAction->setCheckable(true);
    _developCheckAction = new QAction(tr("Developability check"), this);
    _developCheckAction->setCheckable(true);

    _viewportModeGroup = new QActionGroup(this);
    _viewportModeGroup->addAction(_wireframeAction);
    _viewportModeGroup->addAction(_shadeAction);
    _viewportModeGroup->addAction(_gaussCurvAction);
    _viewportModeGroup->addAction(_zebraAction);
    _viewportModeGroup->addAction(_developCheckAction);
    _wireframeAction->setChecked(true);

    connect(_wireframeAction, SIGNAL(triggered()), this, SLOT(wireFrame()));
    connect(_shadeAction, SIGNAL(triggered()), this, SLOT(shade()));
    connect(_gaussCurvAction, SIGNAL(triggered()), this, SLOT(shadeCurvature()));
    connect(_zebraAction, SIGNAL(triggered()), this, SLOT(shadeZebra()));
    connect(_developCheckAction, SIGNAL(triggered()), this, SLOT(shadeDevelopable()));

    _perspectiveAction = new QAction(tr("Perspective"), this);
    _perspectiveAction->setCheckable(true);
    _bodyPlanAction = new QAction(tr("Body plan"), this);
    _bodyPlanAction->setCheckable(true);
    _profileAction = new QAction(tr("Profile"), this);
    _profileAction->setCheckable(true);
    _planViewAction = new QAction(tr("Plan view"), this);
    _planViewAction->setCheckable(true);
    
    _viewGroup = new QActionGroup(this);
    _viewGroup->addAction(_bodyPlanAction);
    _viewGroup->addAction(_profileAction);
    _viewGroup->addAction(_planViewAction);
    _viewGroup->addAction(_perspectiveAction);
    _perspectiveAction->setChecked(true);

    connect(_bodyPlanAction, SIGNAL(triggered()), this, SLOT(setBodyPlanView()));
    connect(_profileAction, SIGNAL(triggered()), this, SLOT(setProfileView()));
    connect(_planViewAction, SIGNAL(triggered()), this, SLOT(setPlanView()));
    connect(_perspectiveAction, SIGNAL(triggered()), this, SLOT(setPerspectiveView()));
    
    _zoomInAction = new QAction(tr("Zoom in"), this);
    _zoomOutAction = new QAction(tr("Zoom out"), this);
    _zoomAllAction = new QAction(tr("All"), this);

    _wideLensAction = new QAction(tr("Wide lens 28mm."), this);
    _wideLensAction->setCheckable(true);
    _stdLensAction = new QAction(tr("Standard lens 50mm."), this);
    _stdLensAction->setCheckable(true);
    _shortLensAction = new QAction(tr("Short telelens 90mm."), this);
    _shortLensAction->setCheckable(true);
    _medLensAction = new QAction(tr("Medium telelens 130mm."), this);
    _medLensAction->setCheckable(true);
    _longLensAction = new QAction(tr("Long telelens 200mm."), this);
    _longLensAction->setCheckable(true);

    _cameraGroup = new QActionGroup(this);
    _cameraGroup->addAction(_wideLensAction);
    _cameraGroup->addAction(_stdLensAction);
    _cameraGroup->addAction(_shortLensAction);
    _cameraGroup->addAction(_medLensAction);
    _cameraGroup->addAction(_longLensAction);
    _stdLensAction->setChecked(true);

    connect(_wideLensAction, SIGNAL(triggered()), this, SLOT(setWideLens()));
    connect(_stdLensAction, SIGNAL(triggered()), this, SLOT(setStdLens()));
    connect(_shortLensAction, SIGNAL(triggered()), this, SLOT(setShortLens()));
    connect(_medLensAction, SIGNAL(triggered()), this, SLOT(setMedLens()));
    connect(_longLensAction, SIGNAL(triggered()), this, SLOT(setFarLens()));

    _visibleBgImgAction = new QAction(tr("Visible"), this);
    _visibleBgImgAction->setEnabled(false);
    _clearBgImgAction = new QAction(tr("Clear"), this);
    _clearBgImgAction->setEnabled(false);
    _loadBgImgAction = new QAction(tr("Load"), this);
    _loadBgImgAction->setEnabled(false);
    _saveBgImgAction = new QAction(tr("Save"), this);
    _saveBgImgAction->setEnabled(false);
    _originBgImgAction = new QAction(tr("Origin"), this);
    _originBgImgAction->setEnabled(false);
    _scaleBgImgAction = new QAction(tr("Scale"), this);
    _scaleBgImgAction->setEnabled(false);
    _alphaBgImgAction = new QAction(tr("Transparent"), this);
    _alphaBgImgAction->setEnabled(false);
    _tolBgImgAction = new QAction(tr("Tolerance"), this);
    _tolBgImgAction->setEnabled(false);
    _blendBgImgAction = new QAction(tr("Blending"), this);
    _blendBgImgAction->setEnabled(false);

    _printAction = new QAction(tr("Print"), this);
    _printAction->setEnabled(false);
    _saveImageAction = new QAction(tr("Save image"), this);
    _saveImageAction->setEnabled(false);
}

void MainWindow::createMenus()
{
    // the context menu
    _contextMenu = new QMenu(this);
    _contextMenu->addAction(ui->actionDeselect_all);
    QMenu* viewMenu = _contextMenu->addMenu(tr("View"));
    viewMenu->addAction(_bodyPlanAction);
    viewMenu->addAction(_profileAction);
    viewMenu->addAction(_planViewAction);
    viewMenu->addAction(_perspectiveAction);
    QMenu* zoomMenu = _contextMenu->addMenu(tr("Zoom"));
    zoomMenu->addAction(_zoomInAction);
    zoomMenu->addAction(_zoomOutAction);
    zoomMenu->addAction(_zoomAllAction);
    _cameraMenu = _contextMenu->addMenu(tr("Camera"));
    _cameraMenu->addAction(_wideLensAction);
    _cameraMenu->addAction(_stdLensAction);
    _cameraMenu->addAction(_shortLensAction);
    _cameraMenu->addAction(_medLensAction);
    _cameraMenu->addAction(_longLensAction);
    QMenu* modeMenu = _contextMenu->addMenu(tr("Mode"));
    modeMenu->addAction(_wireframeAction);
    modeMenu->addAction(_shadeAction);
    modeMenu->addAction(_gaussCurvAction);
    modeMenu->addAction(_zebraAction);
    modeMenu->addAction(_developCheckAction);
    QMenu* imgMenu = _contextMenu->addMenu(tr("Background image"));
    imgMenu->addAction(_visibleBgImgAction);
    imgMenu->addAction(_clearBgImgAction);
    imgMenu->addAction(_loadBgImgAction);
    imgMenu->addAction(_saveBgImgAction);
    imgMenu->addAction(_originBgImgAction);
    imgMenu->addAction(_scaleBgImgAction);
    imgMenu->addAction(_alphaBgImgAction);
    imgMenu->addAction(_tolBgImgAction);
    imgMenu->addAction(_blendBgImgAction);
    _contextMenu->addAction(_printAction);
    _contextMenu->addAction(_saveImageAction);
}

// array of keys mapping type to settings name
const char* vp_keys[] = {
    "i:row", "i:col", "vt:type", "vm:mode", "ct:camera", "f:angle", "f:elev", 0
};

struct ViewportSettings 
{
    int row;
    int col;
    viewport_mode_t vm;
    viewport_type_t ty;
    camera_type_t ct;
    float angle;
    float elev;
};

void MainWindow::restoreViewports()
{
    QSettings settings;
    int num;
    vector<ViewportSettings> vpsets;
    if (settings.contains("view/viewports")) {
        num = settings.value("view/viewports").toInt();
        vpsets.reserve(num);
        for (int i=0; i<num; i++) {
            QString basekey = QString("view/viewport%1").arg(i);
            const char** vp_key = vp_keys;
            ViewportSettings vpset;
            try {
                while (*vp_key != 0) {
                    QString key(*vp_key);
                    QStringList pcs = key.split(":");
                    QString viewkey = QString("%1/%2").arg(basekey).arg(pcs.at(1));
                    if (settings.contains(viewkey)) {
                        if (pcs.at(0) == "i") {
                            int val = settings.value(viewkey).toInt();
                            if (pcs.at(1) == "row")
                                vpset.row = val;
                            else
                                vpset.col = val;
                        } else if (pcs.at(0) == "vt") {
                            vpset.ty = static_cast<viewport_type_t>(settings.value(viewkey).toInt());
                        } else if (pcs.at(0) == "vm") {
                            vpset.vm = static_cast<viewport_mode_t>(settings.value(viewkey).toInt());
                        } else if (pcs.at(0) == "ct") {
                            vpset.ct = static_cast<camera_type_t>(settings.value(viewkey).toInt());
                        } else if (pcs.at(0) == "f") {
                            float val = settings.value(viewkey).toFloat();
                            if (pcs.at(1) == "angle")
                                vpset.angle = val;
                            else
                                vpset.elev = val;
                        }
                    } else {
                        goto settingserror;
                    }
                    vp_key++;
                }
            } catch(...) {
                goto settingserror;
            }
            vpsets.push_back(vpset);
        }
    } else {
        goto settingserror;
    }
    for (size_t i=0; i<vpsets.size(); i++)
        addViewport(vpsets[i].row, vpsets[i].col, vpsets[i].ty, vpsets[i].vm, vpsets[i].ct,
                    vpsets[i].angle, vpsets[i].elev);
    return;
settingserror:
    addDefaultViewports();
}

void MainWindow::addDefaultViewports()
{
    int row = 0;
    int col = 0;
    float elev = 0.0;
    float angle = 0.0;
    viewport_type_t ty;
    for (int i=0; i<4; i++) {
        switch (i) {
        case 0:
            row = 0; col = 0;
            elev = 30.0;
            angle = -30.0;
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

        addViewport(row, col, ty, vmShade, ftStandard, angle, elev);
    }
    cout << "addDefaultViewports" << endl;
}

void MainWindow::addViewport(int row, int col, viewport_type_t ty, viewport_mode_t vm,
                             camera_type_t ct, float angle, float elev)
{
    // make the viewport
    Viewport* vp = new Viewport(_controller, ty);
    // connect the render signal to the viewport
    connect(this, SIGNAL(viewportRender()), vp, SLOT(renderNow()));
    ViewportContainer* vpcontainer = new ViewportContainer(vp, this);
    // connect viewport context menu to main window
    connect(vp, SIGNAL(contextMenuEvent(ShipCAD::ViewportContextEvent*)),
            this, SLOT(vpContextMenuEvent(ShipCAD::ViewportContextEvent*)));
    // put it in display area
    ui->displayLayout->addWidget(vpcontainer, row, col);
    ViewportState state(vpcontainer, vp, row, col);
    _viewports.push_back(state);
    vp->setViewportMode(vm);
    vp->setCameraType(ct);
    vp->setAngle(angle);
    vp->setElevation(elev);
}

void MainWindow::saveViewports()
{
    QSettings settings;
    if (_viewports.size() > 0)
        settings.setValue("view/viewports", static_cast<uint>(_viewports.size()));
    for (size_t i=0; i<_viewports.size(); i++) {
        ViewportState& state = _viewports[i];
        saveViewport(i, state);
    }
}

void MainWindow::saveViewport(size_t idx, ViewportState& state)
{
    QSettings settings;
    const char** vp_key = vp_keys;
    QString basekey = QString("view/viewport%1").arg(idx);
    while (*vp_key != 0) {
        QString key(*vp_key);
        QStringList pcs = key.split(":");
        QString viewkey = QString("%1/%2").arg(basekey).arg(pcs.at(1));
        if (pcs.at(0) == "i") {
            if (pcs.at(1) == "row")
                settings.setValue(viewkey, state.row());
            else
                settings.setValue(viewkey, state.col());
        } else if (pcs.at(0) == "vt") {
            settings.setValue(viewkey, static_cast<int>(state.viewport()->getViewportType()));
        } else if (pcs.at(0) == "vm") {
            settings.setValue(viewkey, static_cast<int>(state.viewport()->getViewportMode()));
        } else if (pcs.at(0) == "ct") {
            settings.setValue(viewkey, static_cast<int>(state.viewport()->getCameraType()));
        } else if (pcs.at(0) == "f") {
            if (pcs.at(1) == "angle")
                settings.setValue(viewkey, state.viewport()->getAngle());
            else
                settings.setValue(viewkey, state.viewport()->getElevation());
        }
        vp_key++;
    }
}

// Main.pas:542
void MainWindow::enableActions()
{
    Visibility& vis = _controller->getModel()->getVisibility();
    ShipCADModel* model = _controller->getModel();
    SubdivisionSurface* surf = _controller->getModel()->getSurface();
    size_t nlayers = 0;
    bool delete_empty_layers = false;
    bool develop_layers = false;
    for (size_t i=0; i<surf->numberOfLayers(); i++) {
        if (surf->getLayer(i)->numberOfFaces())
            nlayers++;
        else if (!delete_empty_layers && surf->getLayer(i)->numberOfFaces() == 0)
            delete_empty_layers = true;
        if (!develop_layers && surf->getLayer(i)->isDevelopable() && surf->getLayer(i)->numberOfFaces() > 0)
            develop_layers = true;
    }
    bool remove_unused = false;
    for (size_t i=0; i<surf->numberOfControlPoints(); i++) {
        if (surf->getControlPoint(i)->numberOfFaces() == 0) {
            remove_unused = true;
            break;
        }
    }
    size_t ncpoints = surf->numberOfControlPoints();
    size_t nselcpoints = surf->numberOfSelectedControlPoints();
    size_t nselcedges = surf->numberOfSelectedControlEdges();
    size_t ncfaces = surf->numberOfControlFaces();
    size_t nselcfaces = surf->numberOfSelectedControlFaces();
    size_t ncurves = surf->numberOfControlCurves();
    size_t nselccurves = surf->numberOfSelectedControlCurves();
    size_t nstations = model->getStations().size();
    size_t nwaterlines = model->getWaterlines().size();
    size_t nbuttocks = model->getButtocks().size();
    size_t ndiagonals = model->getDiagonals().size();
    size_t nmarkers = model->numberOfMarkers();
    size_t nflowlines = model->numberOfFlowlines();
    
    // file actions
    ui->actionSave_As->setEnabled(ncpoints > 0 || model->isFileChanged() || model->isFilenameSet());

    ui->actionImportCarene->setEnabled(true);
    ui->actionImportChines->setEnabled(true);
    ui->actionImportFEF->setEnabled(true);
    ui->actionImportPart->setEnabled(_viewports.size() > 0 && ncfaces > 0);
    ui->actionImportPolyCad->setEnabled(true);
    ui->actionImportSurface->setEnabled(true);
    ui->actionImportVRML->setEnabled(true);
    ui->actionImportMichlet_waves->setEnabled(_viewports.size() > 0 && ncfaces > 1);
    ui->actionImportCarlson->setEnabled(true);

    ui->actionExportArchimedes->setEnabled(nstations > 0);
    ui->actionExportCoordinates->setEnabled(ncpoints > 0);
    ui->actionExportDXF_2D_Polylines->setEnabled((nstations > 0 && vis.isShowStations())
                                                 || (nbuttocks > 0 && vis.isShowButtocks())
                                                 || (nwaterlines > 0 && vis.isShowWaterlines()));
    ui->actionExportDXF_3D_mesh->setEnabled(ncfaces > 0);
    ui->actionExportDXF_3D_Polylines->setEnabled((nstations > 0 && vis.isShowStations())
                                                 || (nbuttocks > 0 && vis.isShowButtocks())
                                                 || (nwaterlines > 0 && vis.isShowWaterlines())
                                                 || (ndiagonals > 0 && vis.isShowDiagonals())
                                                 || (ncurves > 0 && vis.isShowControlCurves()));
    ui->actionExportFEF->setEnabled(ncpoints > 0);
    ui->actionExportGHS->setEnabled(nstations > 0);
    ui->actionExportPart->setEnabled(ncfaces > 0);
    ui->actionExportMichlet->setEnabled(ncfaces > 0 && model->getProjectSettings().isMainParticularsSet());
    ui->actionExportWavefront->setEnabled(ncfaces > 0);
    ui->actionExportOffsets->setEnabled(nstations+nbuttocks+nwaterlines+ndiagonals+ncurves > 0);
    ui->actionExportSTL->setEnabled(ncfaces > 0);
    ui->actionExportIGES->setEnabled(ncfaces > 0);

    // edit actions
    ui->actionUndo->setEnabled(model->canUndo());
    ui->actionRedo->setEnabled(model->canRedo());
    ui->actionDelete->setEnabled(nselcpoints
                                 + surf->numberOfSelectedControlFaces()
                                 + surf->numberOfSelectedControlEdges()
                                 + surf->numberOfSelectedControlCurves() > 0);// BUGBUG missing number of flowlines and markers
    ui->actionUndo_history->setEnabled(model->canUndo());
    
    // point actions
    ui->actionAdd->setEnabled(_viewports.size() && vis.isShowControlNet());
    ui->actionAlign->setEnabled(nselcpoints > 2);
    ui->actionPointCollapse->setEnabled(nselcpoints > 0);
    ui->actionInsert_plane->setEnabled(surf->numberOfControlEdges() > 0 && vis.isShowControlNet());
    ui->actionIntersect_layers->setEnabled(surf->numberOfLayers() > 1);
    ui->actionLock_points->setEnabled(nselcpoints > 0
                                      && surf->numberOfSelectedLockedPoints() < nselcpoints);
    ui->actionUnlock_points->setEnabled(nselcpoints > 0);
    ui->actionUnlock_all_points->setEnabled(surf->numberOfLockedPoints() > 0);

    // edge actions
    ui->actionEdgeExtrude->setEnabled(nselcedges > 0);
    ui->actionEdgeSplit->setEnabled(nselcedges > 0);
    ui->actionEdgeCollapse->setEnabled(nselcedges > 0);
    ui->actionEdgeInsert->setEnabled(nselcpoints > 1);
    ui->actionEdgeCrease->setEnabled(nselcedges > 0);

    // curve actions
    ui->actionCurveNew->setEnabled(nselcedges > 0);

    // face actions
    ui->actionFaceNew->setEnabled(nselcpoints > 2);
    ui->actionFaceInvert->setEnabled(nselcfaces > 0);
    
    // layer actions
    ui->actionLayerAuto_group->setEnabled(ncfaces > 1 && vis.isShowInteriorEdges());
    ui->actionLayerDelete_empty->setEnabled(delete_empty_layers);

    // visibility actions
    ui->actionShowControl_net->setEnabled(ncpoints > 0);
    ui->actionShow_both_sides->setEnabled(ncfaces > 0);
    ui->actionShowControl_curves->setChecked(ncurves > 0);
    ui->actionShowInterior_edges->setEnabled(ncfaces > 0);
    ui->actionShowGrid->setEnabled(vis.isShowGrid());
    ui->actionShowStations->setEnabled(nstations > 0);
    ui->actionShowButtocks->setEnabled(nbuttocks > 0);
    ui->actionShowWaterlines->setEnabled(nwaterlines > 0);
    ui->actionShowDiagonals->setEnabled(ndiagonals > 0);
    ui->actionShowHydrostatic_features->setEnabled(
        ncfaces > 2 && model->getProjectSettings().isMainParticularsSet());
    ui->actionShowFlowlines->setEnabled(nflowlines > 0);
    ui->actionShowNormals->setEnabled(surf->numberOfSelectedControlFaces() > 0);
    ui->actionShowCurvature->setEnabled(nstations+nbuttocks+nwaterlines+ndiagonals+ncurves > 0);
    ui->actionShowMarkers->setEnabled(nmarkers > 0);

    // selection actions
    ui->actionSelect_all->setEnabled(true);
    ui->actionDeselect_all->setEnabled(nselcpoints+nselcedges+nselcfaces+nselccurves > 0
                                       || model->getActiveControlPoint() != 0);
    
    // tools actions
    //ui->actionImportMarkers->setEnabled(
    ui->actionDelete_all_markers->setEnabled(nmarkers > 0);
    ui->actionCheck_model->setEnabled(ncfaces > 0);
    //ui->actionRemove_negative->setEnabled(
    ui->actionRemove_unused_points->setEnabled(remove_unused);
    ui->actionDevelop_plates->setEnabled(develop_layers);
    ui->actionKeel_and_rudder_wizard->setEnabled(_viewports.size());
    ui->actionAdd_cylinder->setEnabled(_viewports.size());

    // transform actions
    ui->actionScale->setEnabled(ncpoints > 0);
    ui->actionMove->setEnabled(ncpoints > 0);
    ui->actionRotate->setEnabled(ncpoints > 0);
    ui->actionMirror->setEnabled(ncfaces > 0);
    ui->actionLackenby->setEnabled(ncfaces > 0);

    // calculations actions

    // update the precision combo box
    if (ncfaces > 0) {
        _precisionComboBox->setEnabled(true);
        _precisionComboBox->setCurrentIndex(static_cast<int>(_controller->getModel()->getPrecision()));
    } else
        _precisionComboBox->setEnabled(false);

    // update the layer combo box
    if (ncfaces > 0) {
        _activeLayerComboBox->setEnabled(true);
        QSignalBlocker block(_activeLayerComboBox);
        _activeLayerComboBox->clear();
        for (size_t i=0; i<surf->numberOfLayers(); i++) {
            _activeLayerComboBox->addItem(surf->getLayer(i)->getName());
            if (surf->getActiveLayer() == surf->getLayer(i))
                _activeLayerComboBox->setCurrentIndex(i);
        }
    } else
        _activeLayerComboBox->setEnabled(false);

    // update the layer color box
    if (surf->getActiveLayer() != nullptr) {
        _colorView->setColor(surf->getActiveLayer()->getColor());
    } else {
        _colorView->setColor(surf->getLayerColor());
    }
    _colorView->update();

    cout << "enableActions" << endl;
}

void MainWindow::updateVisibilityActions()
{
    Visibility& vis = _controller->getModel()->getVisibility();

    ui->actionShowControl_net->setChecked(vis.isShowControlNet());
    ui->actionShow_both_sides->setChecked(vis.getModelView() == mvBoth);
    ui->actionShowControl_curves->setChecked(
        _controller->getModel()->getSurface()->numberOfControlCurves() > 0
        && vis.isShowControlCurves());
    ui->actionShowInterior_edges->setChecked(vis.isShowInteriorEdges());
    ui->actionShowGrid->setChecked(vis.isShowGrid());
    ui->actionShowStations->setChecked(_controller->getModel()->getStations().size() > 0
                                       && vis.isShowStations());
    ui->actionShowButtocks->setChecked(_controller->getModel()->getButtocks().size() > 0
                                       && vis.isShowButtocks());
    ui->actionShowWaterlines->setChecked(_controller->getModel()->getWaterlines().size() > 0
                                         && vis.isShowWaterlines());
    ui->actionShowDiagonals->setChecked(_controller->getModel()->getDiagonals().size() > 0
                                        && vis.isShowDiagonals());
    ui->actionShowHydrostatic_features->setChecked(vis.isShowHydrostaticData());
    ui->actionShowFlowlines->setChecked(_controller->getModel()->numberOfFlowlines() > 0
                                        && vis.isShowFlowlines());
    ui->actionShowNormals->setChecked(vis.isShowNormals());
    ui->actionShowCurvature->setChecked(vis.isShowCurvature());
    ui->actionShowMarkers->setChecked(_controller->getModel()->numberOfMarkers() > 0 &&
                                      vis.isShowMarkers());

//    ui->actionShade_Underwater->setChecked(s->shadeUnderWater());
    emit viewportRender();
    cout << "updateVisibilityActions" << endl;
}

void MainWindow::modelLoaded()
{
    cout << "modelLoaded" << endl;
    // if we already had a model loaded, save the viewports
    if (_viewports.size()) {
        saveViewports();
    }
    else
        restoreViewports();
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

void MainWindow::showPreferences()
{

}

void
MainWindow::wireFrame()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportMode(vmWireFrame);
    cout << "Viewport mode Wire Frame" << endl;
}

void
MainWindow::shade()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportMode(vmShade);
    cout << "Viewport mode shade" << endl;
}

void
MainWindow::shadeCurvature()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportMode(vmShadeGauss);
    cout << "Viewport mode shade gauss" << endl;
}

void
MainWindow::shadeDevelopable()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportMode(vmShadeDevelopable);
    cout << "Viewport mode shade developable" << endl;
}

void
MainWindow::shadeZebra()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportMode(vmShadeZebra);
    cout << "Viewport mode shade zebra" << endl;
}

void MainWindow::setWideLens()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setCameraType(ftWide);
    cout << "MainWindow::setWideLens()" << endl;
}

void MainWindow::setStdLens()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setCameraType(ftStandard);
    cout << "MainWindow::setStdLens()" << endl;
}

void MainWindow::setShortLens()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setCameraType(ftShortTele);
    cout << "MainWindow::setShortLens()" << endl;
}

void MainWindow::setMedLens()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setCameraType(ftMediumTele);
    cout << "MainWindow::setMedLens()" << endl;
}

void MainWindow::setFarLens()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setCameraType(ftFarTele);
    cout << "MainWindow::setFarLens()" << endl;
}

void MainWindow::modelChanged()
{
    bool changed = _controller->getModel()->isFileChanged();
    QString fname(_controller->getModel()->getFilename());
    // msg 0280
    QString mod(tr("modified"));
    // msg 281
    QString notmod(tr("not modified"));
    QString title = QString(tr("ShipCAD  :  %1 (%2)")).arg(fname).arg(changed ? mod : notmod);
    setWindowTitle(title);
    ui->actionSave->setEnabled(changed);
    cout << "model changed" << endl;
}

void MainWindow::setBodyPlanView()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportType(fvBodyplan);
    cout << "MainWindow::setBodyPlanView()" << endl;
}

void MainWindow::setProfileView()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportType(fvProfile);
    cout << "MainWindow::setProfileView()" << endl;
}

void MainWindow::setPlanView()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportType(fvPlan);
    cout << "MainWindow::setPlanView()" << endl;
}

void MainWindow::setPerspectiveView()
{
    if (_currentViewportContext == nullptr)
        return;
    _currentViewportContext->setViewportType(fvPerspective);
    cout << "MainWindow::setPerspectiveView()" << endl;
}

void MainWindow::precisionChanged(int newprecision)
{
    precision_t p = static_cast<precision_t>(newprecision);
    _controller->setPrecision(p);
}

void MainWindow::showControlPointDialog(bool show)
{
    if (_pointdialog == nullptr) {
        _pointdialog = new PointDialog(this);
        connect(_pointdialog, SIGNAL(cornerPointSelect(bool)),
                _controller, SLOT(cornerPointSelected(bool)));
        connect(_pointdialog, SIGNAL(pointCoordChange(float,float,float)),
                _controller, SLOT(dialogUpdatedPointCoord(float,float,float)));
        connect(_controller, SIGNAL(updateControlPointValue(ShipCAD::SubdivisionControlPoint*)),
                _pointdialog, SLOT(controllerUpdatedPoint(ShipCAD::SubdivisionControlPoint*)));
    }
    _pointdialog->setActive(show);
    cout << "show control point dialog:" << (show ? "t" : "f") << endl;
}

void MainWindow::displayLayerDialog()
{
    if (_layerdialog == nullptr) {
        _layerdialog = new LayerDialog(this);
        connect(this, SIGNAL(layerDialogComplete(ShipCAD::LayerDialogData*)),
                _controller, SLOT(layerDialogComplete(ShipCAD::LayerDialogData*)));
        connect(_layerdialog, SIGNAL(activeLayerChanged(int)),
                _controller, SLOT(setActiveLayer(int)));
        connect(_layerdialog, SIGNAL(exeChooseColorDialog(ShipCAD::ChooseColorDialogData&)),
                this, SLOT(executeChooseColorDialog(ShipCAD::ChooseColorDialogData&)));
        connect(_layerdialog, SIGNAL(layerColorChanged(const QColor&)),
                _controller, SLOT(setActiveLayerColor(const QColor&)));
        connect(_layerdialog, SIGNAL(newLayer()), SLOT(newLayerFromDialog()));
        connect(_layerdialog, SIGNAL(deleteEmptyLayer()), SLOT(removeEmptyLayerFromDialog()));
        connect(_layerdialog, SIGNAL(reorderLayerList(ShipCAD::LayerDialogData*)),
                _controller, SLOT(reorderLayerList(ShipCAD::LayerDialogData*)));
    }
    vector<SubdivisionLayer*> layers(_controller->getSurface()->getLayers().begin(),
                                     _controller->getSurface()->getLayers().end());
    LayerDialogData* data = new LayerDialogData(layers, _controller->getSurface()->getActiveLayer());
    _layerdialog->initialize(data, false);
    _layerdialog->exec();
    data = _layerdialog->retrieve();
    emit layerDialogComplete(data);
    delete data;
}

void MainWindow::newLayerFromDialog()
{
    // create the new layer
    _controller->newLayer();
    vector<SubdivisionLayer*> layers(_controller->getSurface()->getLayers().begin(),
                                     _controller->getSurface()->getLayers().end());
    LayerDialogData* data = new LayerDialogData(layers, _controller->getSurface()->getActiveLayer());
    _layerdialog->initialize(data, true);
}

void MainWindow::removeEmptyLayerFromDialog()
{
    // remove the empty layers
    _controller->deleteEmptyLayers();
    vector<SubdivisionLayer*> layers(_controller->getSurface()->getLayers().begin(),
                                     _controller->getSurface()->getLayers().end());
    LayerDialogData* data = new LayerDialogData(layers, _controller->getSurface()->getActiveLayer());
    _layerdialog->initialize(data, true);
}

void MainWindow::executeInsertPlanePointsDialog(InsertPlaneDialogData& data)
{
    if (_planepointsdialog == nullptr) {
        _planepointsdialog = new InsertPlanePointsDialog(this);
    }
    _planepointsdialog->setExtents(data.min, data.max);
    _planepointsdialog->setPlaneSelected(data.planeSelected);
    int result = _planepointsdialog->exec();
    data.accepted = (result == QDialog::Accepted);
    data.addControlCurveSelected = _planepointsdialog->addControlCurveSelected();
    data.planeSelected = _planepointsdialog->whichPlane();
    bool ok;
    data.distance = _planepointsdialog->distanceValue().toFloat(&ok);
    if (!ok)
        data.accepted = false;
    cout << "execute insert plane points dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::executeIntersectLayersDialog(IntersectLayersDialogData& data)
{
    if (_intersectlayersdialog == nullptr) {
        _intersectlayersdialog = new IntersectLayersDialog(this);
    }
    _intersectlayersdialog->initialize(data);
    int result = _intersectlayersdialog->exec();
    data.accepted = (result == QDialog::Accepted);
    _intersectlayersdialog->retrieve(data);
    cout << "execute intersect layers dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::executeExtrudeEdgeDialog(ExtrudeEdgeDialogData& data)
{
    if (_extrudeedgedialog == nullptr) {
        _extrudeedgedialog = new ExtrudeEdgeDialog(this);
    }
    _extrudeedgedialog->initialize(data);
    int result = _extrudeedgedialog->exec();
    data.accepted = (result == QDialog::Accepted);
    _extrudeedgedialog->retrieve(data);
    cout << "execute extrude edge dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::executeMirrorDialog(MirrorDialogData& data)
{
    if (_mirrordialog == nullptr) {
        _mirrordialog = new MirrorDialog(this);
    }
    _mirrordialog->initialize(data);
    int result = _mirrordialog->exec();
    data.accepted = (result == QDialog::Accepted);
    _mirrordialog->retrieve(data);
    cout << "execute mirror dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::executeRotateDialog(RotateDialogData& data)
{
    if (_rotatedialog == nullptr) {
        _rotatedialog = new RotateDialog(this);
    }
    _rotatedialog->initialize(data);
    int result = _rotatedialog->exec();
    data.accepted = (result == QDialog::Accepted);
    _rotatedialog->retrieve(data);
    cout << "execute rotate dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::executeChooseLayerDialog(ChooseLayerDialogData& data)
{
    if (_chooselayerdialog == nullptr) {
        _chooselayerdialog = new ChooseLayerDialog(this);
        connect(_chooselayerdialog, SIGNAL(layerSelected(ShipCAD::SubdivisionLayer*)),
                _controller, SLOT(layerFacesSelected(ShipCAD::SubdivisionLayer*)));
        connect(_chooselayerdialog, SIGNAL(layerDeselected(ShipCAD::SubdivisionLayer*)),
                _controller, SLOT(layerFacesDeselected(ShipCAD::SubdivisionLayer*)));
        connect(_chooselayerdialog, SIGNAL(layerUpdate(ShipCAD::ChooseLayerDialogData*)),
                _controller, SLOT(layerSelectionUpdate(ShipCAD::ChooseLayerDialogData*)));
    }
    _chooselayerdialog->initialize(data);

    // now that dialog is initialized, connect to display
    connect(_chooselayerdialog, SIGNAL(layerSelected(ShipCAD::SubdivisionLayer*)),
            this, SIGNAL(viewportRender()));
    connect(_chooselayerdialog, SIGNAL(layerDeselected(ShipCAD::SubdivisionLayer*)),
            this, SIGNAL(viewportRender()));
    connect(_chooselayerdialog, SIGNAL(layerUpdate(ShipCAD::ChooseLayerDialogData*)),
            this, SIGNAL(viewportRender()));
    emit viewportRender();

    // execute the dialog
    int result = _chooselayerdialog->exec();

    // dialog finished, disconnect from display
    disconnect(_chooselayerdialog, SIGNAL(layerSelected(ShipCAD::SubdivisionLayer*)),
            this, SIGNAL(viewportRender()));
    disconnect(_chooselayerdialog, SIGNAL(layerDeselected(ShipCAD::SubdivisionLayer*)),
            this, SIGNAL(viewportRender()));
    disconnect(_chooselayerdialog, SIGNAL(layerUpdate(ShipCAD::ChooseLayerDialogData*)),
               this, SIGNAL(viewportRender()));

    data.accepted = (result == QDialog::Accepted);
    _chooselayerdialog->retrieve(data);
    cout << "execute choose layer dialog:" << (data.accepted ? "t" : "f") << endl;
}

void MainWindow::changeSelectedItems()
{
    // TODO
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
    if (_contextMenu == nullptr)
        return;
    _currentViewportContext = event->getViewport();
    bool camera_menu_active = false;
    switch(_currentViewportContext->getViewportMode()) {
    case vmWireFrame:
        _wireframeAction->setChecked(true); break;
    case vmShade:
        _shadeAction->setChecked(true); break;
    case vmShadeGauss:
        _gaussCurvAction->setChecked(true); break;
    case vmShadeDevelopable:
        _developCheckAction->setChecked(true); break;
    case vmShadeZebra:
        _zebraAction->setChecked(true); break;
    }
    switch (_currentViewportContext->getViewportType()) {
    case fvBodyplan:
        _bodyPlanAction->setChecked(true); break;
    case fvProfile:
        _profileAction->setChecked(true); break;
    case fvPlan:
        _planViewAction->setChecked(true); break;
    case fvPerspective:
        _perspectiveAction->setChecked(true);
        camera_menu_active = true;
        break;
    }
    _cameraMenu->setEnabled(camera_menu_active);
    if (camera_menu_active) {
        switch (_currentViewportContext->getCameraType()) {
        case ftWide: _wideLensAction->setChecked(true); break;
        case ftStandard: _stdLensAction->setChecked(true); break;
        case ftShortTele: _shortLensAction->setChecked(true); break;
        case ftMediumTele: _medLensAction->setChecked(true); break;
        case ftFarTele: _longLensAction->setChecked(true); break;
        }
    }
    _contextMenu->exec(event->getMouseEvent()->globalPos());
    cout << "MainWindow::contextMenuEvent finished" << endl;
}

void MainWindow::executeChooseColorDialog(ChooseColorDialogData& data)
{
    const QColor color = QColorDialog::getColor(data.initial, this, data.title, data.options);

    if (color.isValid()) {
        data.chosen = color;
        data.accepted = true;
    } else
        data.accepted = false;
    
    cout << "MainWindow::executeColorDialog" << endl;
}
