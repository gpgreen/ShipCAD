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

namespace Ui {
class MainWindow;
}

namespace ShipCAD {
class Viewport;
class Controller;
}

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

private:

    /*! \brief add the default viewports to the GUI
     */
    void addDefaultViewports();
    /*! \brief create the Recent files menu
     */
    void createRecentFilesMenu();

private slots:

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
    /*! \brief model has changed
     */
    void modelChanged();
    void createToolBars();
    void createStatusBar();
private:
    Ui::MainWindow *ui; /**< the ui created by QtDesigner */
    QLabel* _undo_info;
    QLabel* _geom_info;
    ShipCAD::Controller* _controller; /**< controller of the ShipCADModel */
    std::vector<std::pair<QWidget*, ShipCAD::Viewport*> > _viewports; /**< collection of QWidget<->Viewport pairs */
    std::vector<QAction*> _recent_file_actions; /**< actions for each recent file */
    QMenu* _menu_recent_files; /**< the menu for recent files */
};

#endif // MAINWINDOW_H
