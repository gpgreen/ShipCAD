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

#include "extrudeedgedialog.h"
#include "ui_extrudeedgedialog.h"
#include "shipcadlib.h"

using namespace ShipCAD;

ExtrudeEdgeDialog::ExtrudeEdgeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtrudeEdgeDialog)
{
    ui->setupUi(this);
    readSettings();
}

ExtrudeEdgeDialog::~ExtrudeEdgeDialog()
{
    saveSettings();
    delete ui;
}

void ExtrudeEdgeDialog::initialize(ShipCAD::ExtrudeEdgeDialogData& /*data*/)
{
    // does nothing
}

void ExtrudeEdgeDialog::retrieve(ShipCAD::ExtrudeEdgeDialogData& data)
{
    QString xtext(ui->xLineEdit->text());
    QString ytext(ui->yLineEdit->text());
    QString ztext(ui->zLineEdit->text());
    bool ok;
    data.vector = ZERO;
    if (xtext.size() > 0) {
        float x = xtext.toFloat(&ok);
        if (ok)
            data.vector.setX(x);
    }
    if (ytext.size() > 0) {
        float y = ytext.toFloat(&ok);
        if (ok)
            data.vector.setY(y);
    }
    if (ztext.size() > 0) {
        float z = ztext.toFloat(&ok);
        if (ok)
            data.vector.setZ(z);
    }
}

void ExtrudeEdgeDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void ExtrudeEdgeDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("extrudeedgedialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void ExtrudeEdgeDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("extrudeedgedialog-geometry", saveGeometry());
}

