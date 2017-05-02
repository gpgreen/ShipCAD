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

#include "newmodeldialog.h"
#include "ui_newmodeldialog.h"

using namespace ShipCAD;

NewModelDialog::NewModelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewModelDialog)
{
    ui->setupUi(this);
    ui->lengthLineEdit->setValidator(new QDoubleValidator(0, 10000, 2));
    ui->breadthLineEdit->setValidator(new QDoubleValidator(0, 10000, 2));
    ui->depthLineEdit->setValidator(new QDoubleValidator(0, 10000, 2));
    ui->rowsLineEdit->setValidator(new QIntValidator(0, 1000000));
    ui->columnsLineEdit->setValidator(new QIntValidator(0, 1000000));
    ui->unitsComboBox->addItem(tr("Metric"));
    ui->unitsComboBox->addItem(tr("Imperial"));
    readSettings();
}

NewModelDialog::~NewModelDialog()
{
    saveSettings();
    delete ui;
}

void NewModelDialog::initialize(NewModelDialogData& data)
{
    ui->lengthLineEdit->setText(QString("%1").arg(data.length));
    ui->breadthLineEdit->setText(QString("%1").arg(data.breadth));
    ui->depthLineEdit->setText(QString("%1").arg(data.depth));
    ui->rowsLineEdit->setText(QString("%1").arg(data.rows));
    ui->columnsLineEdit->setText(QString("%1").arg(data.cols));
    ui->unitsComboBox->setCurrentIndex(data.units == fuMetric ? 0 : 1);
}

void NewModelDialog::retrieve(NewModelDialogData& data)
{
    data.length = ui->lengthLineEdit->text().toFloat();
    data.breadth = ui->breadthLineEdit->text().toFloat();
    data.depth = ui->depthLineEdit->text().toFloat();
    data.rows = ui->rowsLineEdit->text().toInt();
    data.cols = ui->columnsLineEdit->text().toInt();
    data.units = ui->unitsComboBox->currentIndex() == 0 ? fuMetric : fuImperial;
}

void NewModelDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void NewModelDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("newmodeldialog-geometry",
                                               QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void NewModelDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("newmodeldialog-geometry", saveGeometry());
}

