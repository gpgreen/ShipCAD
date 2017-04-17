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

#include "chooselayerdialog.h"
#include "ui_chooselayerdialog.h"
#include "subdivlayer.h"

ChooseLayerDialog::ChooseLayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseLayerDialog),
    _listModel(nullptr), _data(nullptr)
{
    ui->setupUi(this);

    // create list view item model
    _listModel = new QStandardItemModel(ui->layerListView);
    ui->layerListView->setModel(_listModel);

    // register model item changed signal
    connect(_listModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(listItemChanged(QStandardItem*)));

    // connect check box
    connect(ui->includePointsCheckBox, SIGNAL(clicked()), this, SLOT(includePointClicked()));

    readSettings();
}

ChooseLayerDialog::~ChooseLayerDialog()
{
    saveSettings();
    delete ui;
}

void ChooseLayerDialog::initialize(ShipCAD::ChooseLayerDialogData& data)
{
    _data = &data;
    _listModel->clear();
    for (size_t i=0; i<data.layers.size(); i++) {
        QStandardItem* listItem = new QStandardItem(data.layers[i].first->getName());
        listItem->setCheckable(true);
        if (data.layers[i].second)
            listItem->setCheckState(Qt::Checked);
        else
            listItem->setCheckState(Qt::Unchecked);
        _listModel->setItem(i, listItem);
    }
    if (data.mode == ShipCAD::fsPoints)
        ui->includePointsCheckBox->setHidden(false);
    else
        ui->includePointsCheckBox->setHidden(true);
    ui->includePointsCheckBox->setChecked(data.include_points);
}

void ChooseLayerDialog::retrieve(ShipCAD::ChooseLayerDialogData& data)
{
    data.include_points = ui->includePointsCheckBox->isChecked();
}

// handles the checkbox indicator state
void ChooseLayerDialog::listItemChanged(QStandardItem *item)
{
    // Get current index from item
    const QModelIndex currentIndex =
            item->model()->indexFromItem(item);

    // set in data
    if (_data != nullptr) {
        Qt::CheckState state = item->checkState();
        if (state == Qt::Checked) {
            _data->layers[currentIndex.row()].second = true;
            if (_data->mode == ShipCAD::fsFaces)
                emit layerSelected(_data->layers[currentIndex.row()].first);
            else
                emit layerUpdate(_data);
        } else {
            _data->layers[currentIndex.row()].second = false;
            if (_data->mode == ShipCAD::fsFaces)
                emit layerDeselected(_data->layers[currentIndex.row()].first);
            else
                emit layerUpdate(_data);
        }
    }
}

void ChooseLayerDialog::includePointClicked()
{
    if (_data != nullptr && _data->mode == ShipCAD::fsPoints)
        emit layerUpdate(_data);
}

void ChooseLayerDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void ChooseLayerDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("chooselayerdialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void ChooseLayerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("chooselayerdialog-geometry", saveGeometry());
}

