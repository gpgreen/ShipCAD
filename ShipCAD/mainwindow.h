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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <vector>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include "shipcadlib.h"
#include "dialogdata.h"

namespace Ui {
class MainWindow;
}

namespace ShipCAD {
class Viewport;
class ViewportContainer;
class Controller;
class ViewportContextEvent;
}

class PointDialog;
class InsertPlanePointsDialog;
class IntersectLayersDialog;
class ExtrudeEdgeDialog;

class ViewportState
{
private:
    ShipCAD::ViewportContainer* _container;
    ShipCAD::Viewport* _vp;
    int _row;
    int _col;
public:
    ViewportState(ShipCAD::ViewportContainer* container,
                  ShipCAD::Viewport* vp,
                  int row,
                  int col)
        : _container(container), _vp(vp), _row(row), _col(col)
        {}
    
    ShipCAD::Viewport* viewport() const {return _vp;}
    int row() const {return _row;}
    int col() const {return _col;}
    
};

/*! \brief GUI Main window for ShipCAD
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*! \brief constructor
     */
    explicit MainWindow(ShipCAD::Controller* c, QWidget *parent = 0);
    /*! \brief destructor
     */
    ~MainWindow();


signals:

    /*! \brief render the viewport(s)
     */
    void viewportRender();

protected:

    void closeEvent(QCloseEvent* event);

    /*! \brief delete the viewports
     */
    void deleteViewports();
    /*! \brief restore the viewports when model loaded
     */
    void restoreViewports();
    /*! \brief add the default viewports to the GUI
     */
    void addDefaultViewports();
    /*! \brief add a specified viewport to the GUI
     */
    void addViewport(int row, int col, ShipCAD::viewport_type_t ty,
                     ShipCAD::viewport_mode_t vm, ShipCAD::camera_type_t ct,
                     float angle, float elev);
    /*! \brief save the viewports to settings
     */
    void saveViewports();
    /*! \brief save a viewport to settings
     */
    void saveViewport(size_t idx, ViewportState& state);
    /*! \brief create the Recent files menu
     */
    void createRecentFilesMenu();
    /*! \brief create actions not in form
     */
    void createActions();
    /*! \brief create menus
     */
    void createMenus();
    /*! \brief read settings and action them
     */
    void readSettings();
                       
private slots:

    /*! \brief action for new model
     */
    void newModel();

    /*! \brief action for open model
     */
    void openModelFile();

    /*! \brief action for saving model file
     */
    void saveModelFile();

    /*! \brief action for saving model file as new name
     */
    void saveModelAsFile();
    
    /*! \brief new model loaded
     */
    void modelLoaded();

    /*! \brief enable action items when a model has been loaded
     */
    void enableActions();

    void updateVisibilityActions();
    /*! \brief the list of recent files has been changed
     */
    void changeRecentFiles();
    /*! \brief open a "recent file"
     */
    void openRecentFile();
    /*! \brief undo data update
     */
    void updateUndoData();
    /*! \brief layer data updated
     */
    void changedLayerData();
    /*! \brief active layer has been changed
     */
    void changeActiveLayer();
    /*! \brief show preference dialog
     */
    void showPreferences();

    /*! \brief selected items has changed
     */
    void changeSelectedItems();

    /*! \brief use wireframe shading
     */
    void wireFrame();
    /*! \brief use shading
     */
    void shade();
    /*! \brief use curvature shading
     */
    void shadeCurvature();
    /*! \brief use developable surface shading
     */
    void shadeDevelopable();
    /*! \brief use zebra curvature shading
     */
    void shadeZebra();
    /*! \brief use wide camera lens
     */
    void setWideLens();
    /*! \brief use standard camera lens
     */
    void setStdLens();
    /*! \brief use short telephoto lens
     */
    void setShortLens();
    /*! \brief use medium telephoto lens
     */
    void setMedLens();
    /*! \brief use long telephoto lens
     */
    void setFarLens();
    /*! \brief change viewport to body plan view
     */
    void setBodyPlanView();
    /*! \brief change viewport to profile view
     */
    void setProfileView();
    /*! \brief change viewport to plan view
     */
    void setPlanView();
    /*! \brief change viewport to perspective view
     */
    void setPerspectiveView();
    /*! \brief model has changed
     */
    void modelChanged();

    void createToolBars();
    void createStatusBar();

    /*! \brief show or no show the control point dialog
     *
     * \param show true if dialog should be shown, false if hidden
     */
    void showControlPointDialog(bool show);

    /*! \brief execute the insert plane control points dialog
     *
     * \return true if dialog "ok" selected, false if "cancel" selected
     */
    void executeInsertPlanePointsDialog(ShipCAD::InsertPlaneDialogData& data);

    /*! \brief execute the intersect layers dialog
     *
     * \return true if dialog "ok" selected, false if "cancel" selected
     */
    void executeIntersectLayersDialog(ShipCAD::IntersectLayersDialogData& data);

    /*! \brief execute the extrude edge dialog
     *
     * \return true if dialog "ok" selected, false if "cancel" selected
     */
    void executeExtrudeEdgeDialog(ShipCAD::ExtrudeEdgeDialogData& data);

    /*! \brief get the list of recent files
     */
    const QStringList& getRecentFiles() const;

    /*! \brief add a filename to the list of recent files
     */
    void addRecentFiles(const QString& filename);

    /*! \brief context menu event in a viewport
     */
    void vpContextMenuEvent(ShipCAD::ViewportContextEvent* event);

    /*! \brief show an information dialog with given text
     */
    void showInfoDialog(const QString& msg);
    
    /*! \brief show warning dialog with given text
     */
    void showWarnDialog(const QString& msg);
    
    /*! \brief show error dialog with given text
     */
    void showErrDialog(const QString& msg);
    
private:
    Ui::MainWindow *ui; /**< the ui created by QtDesigner */
    PointDialog *_pointdialog; /**< the control point dialog created by QtDesigner */
    InsertPlanePointsDialog *_planepointsdialog; /**< the insert plane control points dialog created by QtDesigner */
    IntersectLayersDialog *_intersectlayersdialog; /**< the dialog to select 2 different layers */
    ExtrudeEdgeDialog *_extrudeedgedialog; /**< the dialog to set vector to extrude edges */
    QLabel* _undo_info;
    QLabel* _geom_info;
    ShipCAD::Controller* _controller; /**< controller of the ShipCADModel */
    ShipCAD::Viewport* _currentViewportContext; /**< viewport current during context menu */
    std::vector<ViewportState> _viewports; /**< collection of ViewportState */
    std::vector<QAction*> _recent_file_actions; /**< actions for each recent file */
    QMenu* _menu_recent_files; /**< the menu for recent files */
    QMenu* _contextMenu; /**< context menu */
    QMenu* _cameraMenu; /**< camera group in context menu */
    QStringList _recent_files; /**< the list of recent file names */
    QActionGroup* _viewportModeGroup;
    QAction* _wireframeAction;
    QAction* _shadeAction;
    QAction* _gaussCurvAction;
    QAction* _zebraAction;
    QAction* _developCheckAction;
    QActionGroup* _viewGroup;
    QAction* _perspectiveAction;
    QAction* _bodyPlanAction;
    QAction* _profileAction;
    QAction* _planViewAction;
    QAction* _zoomInAction;
    QAction* _zoomOutAction;
    QAction* _zoomAllAction;
    QAction* _printAction;
    QAction* _saveImageAction;
    QActionGroup* _cameraGroup;
    QAction* _wideLensAction;
    QAction* _stdLensAction;
    QAction* _shortLensAction;
    QAction* _medLensAction;
    QAction* _longLensAction;
};

#endif // MAINWINDOW_H
