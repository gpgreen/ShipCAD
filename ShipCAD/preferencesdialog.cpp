/*##############################################################################################
 *    ShipCAD																				   *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>								   *
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

#include <stdexcept>
#include <QIntValidator>
#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "preferences.h"

using namespace ShipCAD;
using namespace std;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog), _data(nullptr)
{
    ui->setupUi(this);

    _mapper = new QSignalMapper(this);
    connect(_mapper, SIGNAL(mapped(int)), SLOT(colorClicked(int)));
    initMembers();

    ui->undoMemoryLineEdit->setValidator( new QIntValidator(0, 10000) );
    connect(ui->resetPushButton, SIGNAL(clicked()), SIGNAL(reset()));

    readSettings();
}

PreferencesDialog::~PreferencesDialog()
{
    saveSettings();
    delete ui;
}

void PreferencesDialog::initMembers()
{
    QFrame* qframes [] = {
        ui->viewportBgColorFrame,
        ui->viewportGridColorFrame,
        ui->gridFontColorFrame,
        ui->newSurColFrame,
        ui->subSurfColFrame,
        ui->surfNormColFrame,
        ui->conCurvColFrame,
        ui->regConEdgeColFrame,
        ui->creaseEdgeConColFrame,
        ui->creaseEdgeIntColFrame,
        ui->regConPtsColFrame,
        ui->creasePtsColFrame,
        ui->cornerPtsColFrame,
        ui->dartPtsColFrame,
        ui->leakPtsColFrame,
        ui->selItemColFrame,
        ui->curvePlotColFrame,
        ui->markerColFrame,
        ui->stationColFrame,
        ui->buttockColFrame,
        ui->waterlineColFrame,
        ui->diagonalColFrame,
        ui->hydroFontColFrame,
        ui->zebraColFrame
    };

    // create the color view objs
    // connect the color view click to the signal mapper

    for (int i=0; i<24; ++i) {
        _color_views.push_back(make_pair(qframes[i], new ColorView(Qt::black, qframes[i])));
        connect(_color_views[i].second, SIGNAL(clicked()), _mapper, SLOT(map()));
        _mapper->setMapping(_color_views[i].second, i);
    }
}

void PreferencesDialog::initialize(ShipCAD::PreferencesDialogData* data)
{
    
    _data = data;
    ui->undoMemoryLineEdit->setText(QString("%1").arg(_data->undo_memory));
    ui->controlPointSizeSlider->setValue(_data->control_point_size);
    map<int, ColorChanger>::iterator i = _data->colors.begin();
    for (size_t j=0; i!=_data->colors.end(); ++i,++j)
        _color_views[j].second->setColor((*i).second.orig);
}

void PreferencesDialog::colorClicked(int id)
{
    map<int, ColorChanger>::iterator i = _data->colors.find(id);
    if (i == _data->colors.end())
        throw range_error("id not found in color map");
    ColorChanger& changer = (*i).second;
    ChooseColorDialogData cdata(tr("Choose color"), *(changer.setColor));
    emit exeChooseColorDialog(cdata);
    if (cdata.accepted) {
        *(changer.setColor) = cdata.chosen;
    }
}

void PreferencesDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void PreferencesDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("preferencesdialog-geometry",
                                               QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void PreferencesDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("preferencesdialog-geometry", saveGeometry());
}

