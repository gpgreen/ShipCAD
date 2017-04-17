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

#include <QSettings>

#include "intersectlayersdialog.h"
#include "ui_intersectlayersdialog.h"
#include "subdivlayer.h"

IntersectLayersDialog::IntersectLayersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntersectLayersDialog)
{
    ui->setupUi(this);
    connect(ui->layer1ComboBox, SIGNAL(clicked()), SLOT(layer1Changed()));
    readSettings();
}

IntersectLayersDialog::~IntersectLayersDialog()
{
    saveSettings();
    delete ui;
}

void IntersectLayersDialog::initialize(ShipCAD::IntersectLayersDialogData& data)
{
    ui->layer1ComboBox->clear();
    ui->layer2ComboBox->clear();
    for (size_t i=0; i<data.layers.size(); i++) {
        ui->layer1ComboBox->addItem(data.layers[i]->getName());
        if (i == data.layer1)
            continue;
        ui->layer2ComboBox->addItem(data.layers[i]->getName());
    }
}

void IntersectLayersDialog::retrieve(ShipCAD::IntersectLayersDialogData& data)
{
    QString ln1 = ui->layer1ComboBox->currentText();
    QString ln2 = ui->layer2ComboBox->currentText();
    for (size_t i=0; i<data.layers.size(); i++) {
        if (ln1 == data.layers[i]->getName())
            data.layer1 = i;
        else if (ln2 == data.layers[i]->getName())
            data.layer2 = i;
    }
}

void IntersectLayersDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void IntersectLayersDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("intersectlayersdialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void IntersectLayersDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("intersectlayersdialog-geometry", saveGeometry());
}

void IntersectLayersDialog::layer1Changed()
{
    // alter layer2 combo box to exclude layer picked in combo box1
    int idx2 = ui->layer2ComboBox->currentIndex();
    QString item2(ui->layer2ComboBox->itemText(idx2));
    ui->layer2ComboBox->clear();
    int idx1 = ui->layer1ComboBox->currentIndex();
    for (int i=0; i<ui->layer1ComboBox->count(); i++) {
        if (i == idx1)
            continue;
        ui->layer2ComboBox->addItem(ui->layer1ComboBox->itemText(i));
    }
    // select the first item
    ui->layer2ComboBox->setCurrentIndex(0);
}
