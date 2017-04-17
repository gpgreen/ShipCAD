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

#ifndef POINTDIALOG_H
#define POINTDIALOG_H

#include <QDialog>

#include "subdivpoint.h"

namespace Ui {
class PointDialog;
}

/*! \brief Dialog to edit coordinates of a Control Point
 */
class PointDialog : public QDialog
{
    Q_OBJECT

public:

    explicit PointDialog(QWidget *parent = 0);
    ~PointDialog();

    /*! \brief show or hide the dialog
     *
     * \param active true if dialog is to be shown, false if hidden
     */
    void setActive(bool active);

signals:

    /*! \brief corner point selected or deselected
     */
    void cornerPointSelect(bool);

    /*! \brief dialog updated point coordinate
     */
    void pointCoordChange(float x, float y, float z);

public slots:

    /*! \brief update dialog when the point has changed
     */
    void controllerUpdatedPoint(ShipCAD::SubdivisionControlPoint* pt);

    /*! \brief called when spin box changes a value
     */
    void dialogUpdatePointCoord(double d);

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:

    Ui::PointDialog *ui;
};

#endif // POINTDIALOG_H
