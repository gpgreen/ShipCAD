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

#include "intersectionsdialog.h"
#include "ui_intersectionsdialog.h"

IntersectionsDialog::IntersectionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntersectionsDialog), _showing_intersection(stations),
    _stationsListModel(nullptr), _waterlinesListModel(nullptr),
    _diagonalsListModel(nullptr), _buttocksListModel(nullptr),
    _data(nullptr), _stationsToolButton(nullptr), _stationsAction(nullptr),
    _buttocksToolButton(nullptr), _buttocksAction(nullptr),
    _waterlinesToolButton(nullptr), _waterlinesAction(nullptr),
    _diagonalsToolButton(nullptr), _diagonalsAction(nullptr),
    _addOneToolButton(nullptr), _addOneAction(nullptr),
    _addNToolButton(nullptr), _addNAction(nullptr),
    _deleteAllToolButton(nullptr), _deleteAllAction(nullptr)
{
    ui->setupUi(this);
    createToolButtons();

    // create list view item model
    _waterlinesListModel = new QStandardItemModel(ui->intersectionsListView);
    _stationsListModel = new QStandardItemModel(ui->intersectionsListView);
    _diagonalsListModel = new QStandardItemModel(ui->intersectionsListView);
    _buttocksListModel = new QStandardItemModel(ui->intersectionsListView);
    ui->intersectionsListView->setModel(_stationsListModel);

    // register model item changed signal
    connect(_stationsListModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(listItemChanged(QStandardItem*)));
    connect(_buttocksListModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(listItemChanged(QStandardItem*)));
    connect(_waterlinesListModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(listItemChanged(QStandardItem*)));
    connect(_diagonalsListModel, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(listItemChanged(QStandardItem*)));

    readSettings();
}

IntersectionsDialog::~IntersectionsDialog()
{
    saveSettings();
    delete ui;
}

void IntersectionsDialog::createToolButtons()
{
    // separator
    // +1
    // +N
    // trash

    _stationsToolButton = new QToolButton(this);
    _stationsAction = new QAction(QIcon(":/Themes/Default/icons/32/ShowStations.png"),
                                  tr("Switch to view or add stations."), this);
    _stationsAction->setCheckable(true);
    _stationsToolButton->setDefaultAction(_stationsAction);
    ui->toolBarHorizontalLayout->addWidget(_stationsToolButton);
    connect(_stationsAction, SIGNAL(triggered()), this, SLOT(stationsToggled()));

    _buttocksToolButton = new QToolButton(this);
    _buttocksAction = new QAction(QIcon(":/Themes/Default/icons/32/ShowButtocks.png"),
                                  tr("Switch to view or add buttocks."), this);
    _buttocksAction->setCheckable(true);
    _buttocksToolButton->setDefaultAction(_buttocksAction);
    ui->toolBarHorizontalLayout->addWidget(_buttocksToolButton);
    connect(_buttocksAction, SIGNAL(triggered()), this, SLOT(buttocksToggled()));

    _waterlinesToolButton = new QToolButton(this);
    _waterlinesAction = new QAction(QIcon(":/Themes/Default/icons/32/ShowWaterlines.png"),
                                    tr("Switch to view or add waterlines."), this);
    _waterlinesAction->setCheckable(true);
    _waterlinesToolButton->setDefaultAction(_waterlinesAction);
    ui->toolBarHorizontalLayout->addWidget(_waterlinesToolButton);
    connect(_waterlinesAction, SIGNAL(triggered()), this, SLOT(waterlinesToggled()));

    _diagonalsToolButton = new QToolButton(this);
    _diagonalsAction = new QAction(QIcon(":/Themes/Default/icons/32/ShowDiagonals.png"),
                                   tr("Switch to view or add diagonals."), this);
    _diagonalsAction->setCheckable(true);
    _diagonalsToolButton->setDefaultAction(_diagonalsAction);
    ui->toolBarHorizontalLayout->addWidget(_diagonalsToolButton);
    connect(_diagonalsAction, SIGNAL(triggered()), this, SLOT(diagonalsToggled()));

    _addOneToolButton = new QToolButton(this);
    _addOneAction = new QAction(QIcon(":/Themes/Default/icons/32/AddOne.png"),
                                tr("Add one"), this);
    _addOneAction->setCheckable(true);
    _addOneToolButton->setDefaultAction(_addOneAction);
    ui->toolBarHorizontalLayout->addWidget(_addOneToolButton);
    connect(_addOneAction, SIGNAL(triggered()), this, SLOT(addOneTriggered()));

    _addNToolButton = new QToolButton(this);
    _addNAction = new QAction(QIcon(":/Themes/Default/icons/32/AddRange.png"),
                              tr("Add multiple"), this);
    _addNAction->setCheckable(true);
    _addNToolButton->setDefaultAction(_addNAction);
    ui->toolBarHorizontalLayout->addWidget(_addNToolButton);
    connect(_addNAction, SIGNAL(triggered()), this, SLOT(addNTriggered()));

    _deleteAllToolButton = new QToolButton(this);
    _deleteAllAction = new QAction(QIcon(":/Themes/Default/icons/32/DeleteAll.png"),
                              tr("Delete all"), this);
    _deleteAllAction->setCheckable(true);
    _deleteAllToolButton->setDefaultAction(_deleteAllAction);
    ui->toolBarHorizontalLayout->addWidget(_deleteAllToolButton);
    connect(_addNAction, SIGNAL(triggered()), this, SLOT(deleteAllTriggered()));

    ui->toolBarHorizontalLayout->addStretch(1);
}

void IntersectionsDialog::initialize(ShipCAD::IntersectionsDialogData* data, bool delete_data)
{
    if (delete_data)
        delete _data;
    _data = data;
    _stationsListModel->clear();
    for (size_t i=0; i<data->stations.size(); ++i) {
        QStandardItem* listItem = new QStandardItem(data->stations[i].first->getDescription());
        listItem->setCheckable(true);
        if (data->stations[i].second)
            listItem->setCheckState(Qt::Checked);
        else
            listItem->setCheckState(Qt::Unchecked);
        _stationsListModel->setItem(i, listItem);
    }
    _buttocksListModel->clear();
    for (size_t i=0; i<data->buttocks.size(); ++i) {
        QStandardItem* listItem = new QStandardItem(data->buttocks[i].first->getDescription());
        listItem->setCheckable(true);
        if (data->buttocks[i].second)
            listItem->setCheckState(Qt::Checked);
        else
            listItem->setCheckState(Qt::Unchecked);
        _buttocksListModel->setItem(i, listItem);
    }
    _waterlinesListModel->clear();
    for (size_t i=0; i<data->waterlines.size(); ++i) {
        QStandardItem* listItem = new QStandardItem(data->waterlines[i].first->getDescription());
        listItem->setCheckable(true);
        if (data->waterlines[i].second)
            listItem->setCheckState(Qt::Checked);
        else
            listItem->setCheckState(Qt::Unchecked);
        _waterlinesListModel->setItem(i, listItem);
    }
    _diagonalsListModel->clear();
    for (size_t i=0; i<data->diagonals.size(); ++i) {
        QStandardItem* listItem = new QStandardItem(data->diagonals[i].first->getDescription());
        listItem->setCheckable(true);
        if (data->diagonals[i].second)
            listItem->setCheckState(Qt::Checked);
        else
            listItem->setCheckState(Qt::Unchecked);
        _diagonalsListModel->setItem(i, listItem);
    }
    _showing_intersection = stations;
    _stationsAction->setChecked(true);
    updateState();
}

void IntersectionsDialog::updateState()
{
    switch(_showing_intersection) {
    case stations:
        ui->intersectionsListView->setModel(_stationsListModel);
        _buttocksAction->setChecked(false);
        _waterlinesAction->setChecked(false);
        _diagonalsAction->setChecked(false);
        _addOneAction->setToolTip(tr("Add one station."));
        _addNAction->setToolTip(tr("Add multiple stations."));
        _deleteAllAction->setToolTip(tr("Delete all stations."));
        _deleteAllAction->setEnabled(_data->stations.size() > 0);
        break;
    case buttocks:
        ui->intersectionsListView->setModel(_buttocksListModel);
        _stationsAction->setChecked(false);
        _waterlinesAction->setChecked(false);
        _diagonalsAction->setChecked(false);
        _addOneAction->setToolTip(tr("Add one buttock."));
        _addNAction->setToolTip(tr("Add multiple buttocks."));
        _deleteAllAction->setToolTip(tr("Delete all buttocks."));
        _deleteAllAction->setEnabled(_data->buttocks.size() > 0);
        break;
    case waterlines:
        ui->intersectionsListView->setModel(_waterlinesListModel);
        _stationsAction->setChecked(false);
        _buttocksAction->setChecked(false);
        _diagonalsAction->setChecked(false);
        _addOneAction->setToolTip(tr("Add one waterline."));
        _addNAction->setToolTip(tr("Add multiple waterlines."));
        _deleteAllAction->setToolTip(tr("Delete all waterlines."));
        _deleteAllAction->setEnabled(_data->waterlines.size() > 0);
        break;
    case diagonals:
        ui->intersectionsListView->setModel(_diagonalsListModel);
        _stationsAction->setChecked(false);
        _buttocksAction->setChecked(false);
        _waterlinesAction->setChecked(false);
        _addOneAction->setToolTip(tr("Add one diagonal."));
        _addNAction->setToolTip(tr("Add multiple diagonals."));
        _deleteAllAction->setToolTip(tr("Delete all diagonals."));
        _deleteAllAction->setEnabled(_data->diagonals.size() > 0);
        break;
    }
    ui->intersectionsListView->update();
}

void IntersectionsDialog::listItemChanged(QStandardItem* item)
{
    // Get current index from item
    const QModelIndex currentIndex =
            item->model()->indexFromItem(item);

#if 0
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
#endif
}

void IntersectionsDialog::stationsToggled()
{
    if (_stationsAction->isChecked()) {
        _showing_intersection = stations;
        updateState();
    }
}

void IntersectionsDialog::buttocksToggled()
{
    if (_buttocksAction->isChecked()) {
        _showing_intersection = buttocks;
        updateState();
    }
}

void IntersectionsDialog::waterlinesToggled()
{
    if (_waterlinesAction->isChecked()) {
        _showing_intersection = waterlines;
        updateState();
    }
}

void IntersectionsDialog::diagonalsToggled()
{
    if (_diagonalsAction->isChecked()) {
        _showing_intersection = diagonals;
        updateState();
    }
}

void IntersectionsDialog::addOneTriggered()
{

}

void IntersectionsDialog::addNTriggered()
{

}

void IntersectionsDialog::deleteAllTriggered()
{

}

void IntersectionsDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void IntersectionsDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("intersectionsdialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void IntersectionsDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("intersectionsdialog-geometry", saveGeometry());
}

