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

namespace Ui {
class IntersectionsDialog;
}

class IntersectionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntersectionsDialog(QWidget *parent = 0);
    ~IntersectionsDialog();

    void initialize(ShipCAD::IntersectionsDialogData* data, bool delete_data,
                    ShipCAD::unit_type_t units);
    ShipCAD::LayerDialogData* retrieve() {return _data;}

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
};

#endif // INTERSECTIONSDIALOG_H
