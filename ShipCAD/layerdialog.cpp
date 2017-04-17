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

#include <iostream>

#include "layerdialog.h"
#include "ui_layerdialog.h"
#include "shipcadlib.h"
#include "colorview.h"

using namespace ShipCAD;
using namespace std;

LayerDialog::LayerDialog(QWidget *parent) :
    QDialog(parent), _current(0),
    ui(new Ui::LayerDialog), _data(nullptr),
    _newToolButton(nullptr), _removeEmptyToolButton(nullptr),
    _moveUpToolButton(nullptr), _moveDownToolButton(nullptr),
    _colorAction(nullptr), _colorView(nullptr)
{
    ui->setupUi(this);
    createToolButtons();

    _colorAction = new QAction(tr("Layer Color"), this);
    _colorAction->setIcon(QIcon(":/Themes/Default/icons/32/ActiveLayerColor.png"));
    ui->colorToolButton->setDefaultAction(_colorAction);
    connect(_colorAction, SIGNAL(triggered()), this, SLOT(selectColor()));
    connect(_colorAction, SIGNAL(triggered()), this, SLOT(updateState()));
    
    // make the color view
    _colorView = new ColorView(Qt::black, ui->colorFrame);
    _colorView->setMinimumSize(32, 32);
    _colorView->setMaximumSize(32, 32);
    _colorView->show();

    connect(ui->layerListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(listRowChanged(int)));
    connect(ui->layerListWidget, SIGNAL(currentRowChanged(int)),
            this, SIGNAL(activeLayerChanged(int)));
    
    // connect line edit to slots
    connect(ui->nameLineEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(nameChanged(const QString&)));
    ui->specificWeightLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    connect(ui->specificWeightLineEdit, SIGNAL(editingFinished()),
            this, SLOT(weightChanged()));
    ui->thicknessLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    connect(ui->thicknessLineEdit, SIGNAL(editingFinished()),
            this, SLOT(thicknessChanged()));
    ui->alphaLineEdit->setValidator( new QDoubleValidator(0, 1.0, 2) );
    connect(ui->alphaLineEdit, SIGNAL(editingFinished()),
            this, SLOT(alphaChanged()));
    
    // connect check boxes to slots
    connect(ui->hydrostaticsCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(hydroBoxChanged(int)));
    connect(ui->intersectionCurvesCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(curveBoxChanged(int)));
    connect(ui->linesplanCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(showLinesBoxChanged(int)));
    connect(ui->symmetricCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(symmBoxChanged(int)));
    connect(ui->developableCheckBox, SIGNAL(stateChanged(int)),
            this, SLOT(devBoxChanged(int)));
    
    readSettings();
}

LayerDialog::~LayerDialog()
{
    saveSettings();
    delete ui;
}

void LayerDialog::createToolButtons()
{
    _newToolButton = new QToolButton(this);
    QAction* newAction = new QAction(tr("Create new layer"), this);
    _newToolButton->setDefaultAction(newAction);
    newAction->setIcon(QIcon(":/Themes/Default/icons/32/NewLayer.png"));
    ui->toolBarHorizontalLayout->addWidget(_newToolButton);

    _removeEmptyToolButton = new QToolButton(this);
    QAction* removeAction = new QAction(tr("Delete empty layers"), this);
    _removeEmptyToolButton->setDefaultAction(removeAction);
    removeAction->setIcon(QIcon(":/Themes/Default/icons/32/DeleteEmptyLayers.png"));
    ui->toolBarHorizontalLayout->addWidget(_removeEmptyToolButton);

    _moveUpToolButton = new QToolButton(this);
    QAction* upAction = new QAction(tr("Move layer up in list"), this);
    _moveUpToolButton->setDefaultAction(upAction);
    _moveUpToolButton->setArrowType(Qt::UpArrow);
    ui->toolBarHorizontalLayout->addWidget(_moveUpToolButton);

    _moveDownToolButton = new QToolButton(this);
    QAction* dnAction = new QAction(tr("Move layer down in list"), this);
    _moveDownToolButton->setDefaultAction(dnAction);
    _moveDownToolButton->setArrowType(Qt::DownArrow);
    ui->toolBarHorizontalLayout->addWidget(_moveDownToolButton);

    ui->toolBarHorizontalLayout->addStretch(1);
}

void LayerDialog::initialize(ShipCAD::LayerDialogData& data)
{
    _data = &data;
    // disconnect signal while we update the widget, crashes otherwise
    disconnect(ui->layerListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(listRowChanged(int)));
    _current = 0;
    ui->layerListWidget->clear();
    for (size_t i=0; i<data.layers.size(); i++) {
        ui->layerListWidget->addItem(data.layers[i].name);
        if (data.active == data.layers[i].data)
            _current = i;
    }
    ui->layerListWidget->setCurrentRow(_current);
    updateState();
    // connect the signal again
    connect(ui->layerListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(listRowChanged(int)));

}

void LayerDialog::updateState()
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    ui->nameLineEdit->setText(cprops.name);
    ui->hydrostaticsCheckBox->setChecked(cprops.hydrostatics ? Qt::Checked : Qt::Unchecked);
    ui->symmetricCheckBox->setChecked(cprops.symmetric ? Qt::Checked : Qt::Unchecked);
    ui->intersectionCurvesCheckBox->setChecked(cprops.intersection_curves ? Qt::Checked : Qt::Unchecked);
    ui->developableCheckBox->setChecked(cprops.developable ? Qt::Checked : Qt::Unchecked);
    ui->linesplanCheckBox->setChecked(cprops.show_linesplan ? Qt::Checked : Qt::Unchecked);
    ui->specificWeightLineEdit->setText(QString("%1").arg(cprops.material_density));
    ui->thicknessLineEdit->setText(QString("%1").arg(cprops.thickness));
    ui->alphaLineEdit->setText(QString("%1").arg(cprops.alpha));
    _colorView->setColor(cprops.color);
    _colorView->update();
}

void LayerDialog::retrieve(ShipCAD::LayerDialogData& data)
{
    // does nothing
}

void LayerDialog::listRowChanged(int index)
{
    _current = index;
    _data->active = _data->layers[_current].data;
    updateState();
}

void LayerDialog::nameChanged(const QString& nm)
{
    QListWidgetItem* cur = ui->layerListWidget->currentItem();
    cur->setText(nm);
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.name = nm;
}

void LayerDialog::weightChanged()
{
    float wt = ui->specificWeightLineEdit->text().toFloat();
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.material_density = wt;
    cout << "Layer:" << cprops.name.toStdString() << " wt:" << wt << endl;
}

void LayerDialog::thicknessChanged()
{
    float t = ui->thicknessLineEdit->text().toFloat();
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.thickness = t;
    cout << "Layer:" << cprops.name.toStdString() << " t:" << t << endl;
}

void LayerDialog::alphaChanged()
{
    float a = ui->alphaLineEdit->text().toFloat();
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.alpha = a;
    cout << "Layer:" << cprops.name.toStdString() << " alpha:" << a << endl;
}

void LayerDialog::hydroBoxChanged(int state)
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.hydrostatics = state == Qt::Checked;
}

void LayerDialog::symmBoxChanged(int state)
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.symmetric = state == Qt::Checked;
}

void LayerDialog::curveBoxChanged(int state)
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.intersection_curves = state == Qt::Checked;
}

void LayerDialog::devBoxChanged(int state)
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.developable = state == Qt::Checked;
}

void LayerDialog::showLinesBoxChanged(int state)
{
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    cprops.show_linesplan = state == Qt::Checked;
}

void LayerDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void LayerDialog::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value("layerdialog-geometry", QByteArray()).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
}

void LayerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("layerdialog-geometry", saveGeometry());
}

void LayerDialog::selectColor()
{
	cout << "LayerDialog::selectColor" << endl;
    LayerPropertiesForDialog& cprops = _data->layers[_current];
    ChooseColorDialogData data(tr("Choose color for Layer"), cprops.color);
    emit exeChooseColorDialog(data);
    if (data.accepted) {
        cprops.color = data.chosen;
        emit layerColorChanged(cprops.color);
    }
}
