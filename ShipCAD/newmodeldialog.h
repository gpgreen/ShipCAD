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

#ifndef NEWMODELDIALOG_H
#define NEWMODELDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class NewModelDialog;
}

class NewModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewModelDialog(QWidget *parent = 0);
    ~NewModelDialog();

    void initialize(ShipCAD::NewModelDialogData& data);
    void retrieve(ShipCAD::NewModelDialogData& data);

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    /*! \brief called before dialog is closed
     */
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::NewModelDialog *ui;
};

#endif // NEWMODELDIALOG_H
