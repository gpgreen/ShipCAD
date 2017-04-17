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

#include "insertplanepointsdialog.h"
#include "ui_insertplanepointsdialog.h"

#include <QSettings>

#include "utility.h"

using namespace ShipCAD;
//using namespace std;

InsertPlanePointsDialog::InsertPlanePointsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertPlanePointsDialog)
{
    ui->setupUi(this);
    connect(ui->transversePlaneRadioButton, SIGNAL(clicked()), SLOT(planeChanged()));
    connect(ui->horizontalPlaneRadioButton, SIGNAL(clicked()), SLOT(planeChanged()));
    connect(ui->verticalPlaneRadioButton, SIGNAL(clicked()), SLOT(planeChanged()));
    readSettings();
}

InsertPlanePointsDialog::~InsertPlanePointsDialog()
{
    saveSettings();
    delete ui;
}

QString InsertPlanePointsDialog::distanceValue() const
{
    return ui->distanceLineEdit->text();
}

plane_selected_t InsertPlanePointsDialog::whichPlane() const
{
    if (ui->transversePlaneRadioButton->isChecked())
        return transverse;
    else if (ui->horizontalPlaneRadioButton->isChecked())
        return horizontal;
    else if (ui->verticalPlaneRadioButton->isChecked())
        return vertical;
    return transverse;
}

void InsertPlanePointsDialog::setPlaneSelected(plane_selected_t pln)
{
    switch(pln) {
    case transverse:
        ui->transversePlaneRadioButton->setChecked(true);
        break;
    case horizontal:
        ui->horizontalPlaneRadioButton->setChecked(true);
        break;
    case vertical:
        ui->verticalPlaneRadioButton->setChecked(true);
        break;
    default:
        ui->transversePlaneRadioButton->setChecked(true);
        break;
    }
}

bool InsertPlanePointsDialog::addControlCurveSelected() const
{
    return ui->addCurveCheckBox->isChecked();
}

void InsertPlanePointsDialog::setExtents(const QVector3D& min, const QVector3D& max)
{
    _min = min;
    _max = max;
    planeChanged();
}

void InsertPlanePointsDialog::planeChanged()
{
    float minval, maxval;
    if (ui->transversePlaneRadioButton->isChecked()) {
        minval = _min.x();
        maxval = _max.x();
    }
    else if (ui->horizontalPlaneRadioButton->isChecked()) {
        minval = _min.z();
        maxval = _max.z();
    }
    else if (ui->verticalPlaneRadioButton->isChecked()) {
        minval = _min.y();
        maxval = _max.y();
    }
    ui->minValueLabel->setText(MakeLength(minval+1E-4, 4, 10));
    ui->maxValueLabel->setText(MakeLength(maxval+1E-4, 4, 10));
}

void InsertPlanePointsDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("insertplanedialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void InsertPlanePointsDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("insertplanedialog-geometry", saveGeometry());
}

void InsertPlanePointsDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

