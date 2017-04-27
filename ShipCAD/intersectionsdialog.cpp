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
    ui(new Ui::IntersectionsDialog)
{
    ui->setupUi(this);
    createToolButtons();
}

IntersectionsDialog::~IntersectionsDialog()
{
    saveSettings();
    delete ui;
}

void IntersectionsDialog::createToolButtons()
{
    // stations
    // buttocks
    // waterlines
    // diagonals
    // separator
    // +1
    // +N
    // trash
#if 0
    _newToolButton = new QToolButton(this);
    _newLayerAction = new QAction(tr("Create new layer"), this);
    _newToolButton->setDefaultAction(_newLayerAction);
    _newLayerAction->setIcon(QIcon(":/Themes/Default/icons/32/NewLayer.png"));
    ui->toolBarHorizontalLayout->addWidget(_newToolButton);
    connect(_newLayerAction, SIGNAL(triggered()), SIGNAL(newLayer()));

    _removeEmptyToolButton = new QToolButton(this);
    _deleteEmptyAction = new QAction(tr("Delete empty layers"), this);
    _removeEmptyToolButton->setDefaultAction(_deleteEmptyAction);
    _deleteEmptyAction->setIcon(QIcon(":/Themes/Default/icons/32/DeleteEmptyLayers.png"));
    ui->toolBarHorizontalLayout->addWidget(_removeEmptyToolButton);
    connect(_deleteEmptyAction, SIGNAL(triggered()), SIGNAL(deleteEmptyLayer()));

    _moveUpToolButton = new QToolButton(this);
    _moveUpAction = new QAction(tr("Move layer up in list"), this);
    _moveUpToolButton->setDefaultAction(_moveUpAction);
    _moveUpToolButton->setArrowType(Qt::UpArrow);
    ui->toolBarHorizontalLayout->addWidget(_moveUpToolButton);
    connect(_moveUpAction, SIGNAL(triggered()), SLOT(moveUp()));

    _moveDownToolButton = new QToolButton(this);
    _moveDownAction = new QAction(tr("Move layer down in list"), this);
    _moveDownToolButton->setDefaultAction(_moveDownAction);
    _moveDownToolButton->setArrowType(Qt::DownArrow);
    ui->toolBarHorizontalLayout->addWidget(_moveDownToolButton);
    connect(_moveDownAction, SIGNAL(triggered()), SLOT(moveDown()));

    ui->toolBarHorizontalLayout->addStretch(1);
#endif
}

void IntersectionsDialog::initialize(ShipCAD::IntersectionsDialogData* data, bool delete_data,
                             ShipCAD::unit_type_t units)
{
#if 0
    QSignalBlocker block(ui->layerListWidget);
    if (delete_data)
        delete _data;
    _data = data;
    _current = 0;
    ui->layerListWidget->clear();
    for (size_t i=0; i<data->layers.size(); i++) {
        ui->layerListWidget->addItem(data->layers[i].name);
        if (data->active == data->layers[i].data)
            _current = i;
    }
    ui->layerListWidget->setCurrentRow(_current);
    _areastr = AreaStr(units);
    _weightstr = WeightStr(units);
    _lengthstr = LengthStr(units);
    ui->thicknessUnitLabel->setText(_lengthstr);
    ui->specificWeightUnitLabel->setText(DensityStr(units));
    updateState();
#endif
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

