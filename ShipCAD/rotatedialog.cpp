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

#include "rotatedialog.h"
#include "ui_rotatedialog.h"

RotateDialog::RotateDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RotateDialog)
{
    ui->setupUi(this);
    readSettings();
}

RotateDialog::~RotateDialog()
{
    saveSettings();
    delete ui;
}

void RotateDialog::initialize(ShipCAD::RotateDialogData& data)
{
    setWindowTitle(data.dialog_title);
    ui->longitudinalLineEdit->setText(QString("%1").arg(data.rotation_vector.x()));
    ui->transverseLineEdit->setText(QString("%1").arg(data.rotation_vector.y()));
    ui->verticalLineEdit->setText(QString("%1").arg(data.rotation_vector.z()));
    ui->longitudinalUnitsLabel->setText(data.units);
    ui->transverseUnitsLabel->setText(data.units);
    ui->verticalUnitsLabel->setText(data.units);
}

void RotateDialog::retrieve(ShipCAD::RotateDialogData& data)
{
    bool ok;
    float d = ui->transverseLineEdit->text().toFloat(&ok);
    if (ok)
        data.rotation_vector.setY(d);
    d = ui->longitudinalLineEdit->text().toFloat(&ok);
    if (ok)
        data.rotation_vector.setX(d);
    d = ui->verticalLineEdit->text().toFloat(&ok);
    if (ok)
        data.rotation_vector.setZ(d);
}

void RotateDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("rotatedialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void RotateDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("rotatedialog-geometry", saveGeometry());
}

void RotateDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

