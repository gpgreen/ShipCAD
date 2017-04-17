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

#ifndef CHOOSELAYERDIALOG_H
#define CHOOSELAYERDIALOG_H

#include <QDialog>
#include "dialogdata.h"

namespace Ui {
class ChooseLayerDialog;
}

class ChooseLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseLayerDialog(QWidget *parent = 0);
    ~ChooseLayerDialog();

    void initialize(ShipCAD::ChooseLayerDialogData& data);
    void retrieve(ShipCAD::ChooseLayerDialogData& data);

signals:

    void layerSelected(ShipCAD::SubdivisionLayer*);
    void layerDeselected(ShipCAD::SubdivisionLayer*);
    void layerUpdate(ShipCAD::ChooseLayerDialogData*);

public slots:

    void listItemChanged(QStandardItem *item);
    void includePointClicked();

protected:
    /*! \brief read stored settings for dialog
     */
    void readSettings();
    /*! \brief save settings for dialog
     */
    void saveSettings();
    virtual void closeEvent(QCloseEvent* event);

private:
    Ui::ChooseLayerDialog *ui;
    QStandardItemModel* _listModel;
    ShipCAD::ChooseLayerDialogData* _data;
};

#endif // CHOOSELAYERDIALOG_H
