/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>								   *
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

#ifndef INTERSECTIONSDIALOG_H
#define INTERSECTIONSDIALOG_H

#include <QDialog>
#include <QToolButton>
#include "dialogdata.h"

namespace Ui {
class IntersectionsDialog;
}

class IntersectionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntersectionsDialog(QWidget *parent = 0);
    ~IntersectionsDialog();

    void initialize(ShipCAD::IntersectionsDialogData* data);
    ShipCAD::IntersectionsDialogData* retrieve();

    virtual void keyPressEvent(QKeyEvent *event);
    
signals:

    void showCurvatureChange();
    void addOrDeleteIntersections();

public slots:

    void listItemChanged(QStandardItem* item);
    void updateState();

    /*! \brief stations toggled
     */
    void stationsToggled();

    /*! \brief buttocks toggled
     */
    void buttocksToggled();

    /*! \brief waterlines toggled
     */
    void waterlinesToggled();

    /*! \brief diagonals toggled
     */
    void diagonalsToggled();

    /*! \brief add one triggered
     */
    void addOneTriggered();

    /*! \brief add range triggered
     */
    void addNTriggered();

    /*! \brief delete all intersections
     */
    void deleteAllTriggered();

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    /*! \brief create tool buttons in dialog
     */
    void createToolButtons();
    /*! \brief called before dialog is closed
     */
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::IntersectionsDialog *ui;
    ShipCAD::intersection_type_t _showing_intersection;
    QStandardItemModel* _stationsListModel;
    QStandardItemModel* _waterlinesListModel;
    QStandardItemModel* _diagonalsListModel;
    QStandardItemModel* _buttocksListModel;
    ShipCAD::IntersectionsDialogData* _data;
    QToolButton* _stationsToolButton;
    QAction* _stationsAction;
    QToolButton* _buttocksToolButton;
    QAction* _buttocksAction;
    QToolButton* _waterlinesToolButton;
    QAction* _waterlinesAction;
    QToolButton* _diagonalsToolButton;
    QAction* _diagonalsAction;
    QToolButton* _addOneToolButton;
    QAction* _addOneAction;
    QToolButton* _addNToolButton;
    QAction* _addNAction;
    QToolButton* _deleteAllToolButton;
    QAction* _deleteAllAction;
};

#endif // INTERSECTIONSDIALOG_H
