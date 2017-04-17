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

#include "mirrordialog.h"
#include "ui_mirrordialog.h"

using namespace ShipCAD;

MirrorDialog::MirrorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MirrorDialog)
{
    ui->setupUi(this);
    readSettings();
}

MirrorDialog::~MirrorDialog()
{
    saveSettings();
    delete ui;
}

void MirrorDialog::initialize(ShipCAD::MirrorDialogData& data)
{
    ui->transverseRadioButton->setChecked(false);
    ui->horizontalRadioButton->setChecked(false);
    ui->verticalRadioButton->setChecked(false);
    switch(data.which_plane) {
    case transverse:
        ui->transverseRadioButton->setChecked(true);
        break;
    case horizontal:
        ui->horizontalRadioButton->setChecked(true);
        break;
    case vertical:
        ui->verticalRadioButton->setChecked(true);
        break;
    };
    ui->connectPointsCheckBox->setChecked(data.connect_points);
    ui->distanceLineEdit->setText(QString("%1").arg(data.distance));
}

void MirrorDialog::retrieve(ShipCAD::MirrorDialogData& data)
{
    if (ui->transverseRadioButton->isChecked())
        data.which_plane = transverse;
    else if (ui->horizontalRadioButton->isChecked())
        data.which_plane = horizontal;
    else if (ui->verticalRadioButton->isChecked())
        data.which_plane = vertical;
    data.connect_points = ui->connectPointsCheckBox->isChecked();
    bool ok;
    float d = ui->distanceLineEdit->text().toFloat(&ok);
    if (!ok)
        data.distance = 0.0;
    else
        data.distance = d;
}

void MirrorDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("mirrordialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void MirrorDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("mirrordialog-geometry", saveGeometry());
}

void MirrorDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

