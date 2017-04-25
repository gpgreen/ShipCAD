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
#include "utility.h"

using namespace ShipCAD;
using namespace std;

LayerDialog::LayerDialog(QWidget *parent) :
    QDialog(parent), _current(0),
    ui(new Ui::LayerDialog), _data(nullptr),
    _newToolButton(nullptr), _removeEmptyToolButton(nullptr),
    _moveUpToolButton(nullptr), _moveDownToolButton(nullptr),
    _colorAction(nullptr), _newLayerAction(nullptr), _deleteEmptyAction(nullptr),
    _moveUpAction(nullptr), _moveDownAction(nullptr), _colorView(nullptr),
    _areastr(""), _weightstr(""), _lengthstr("")
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
}

void LayerDialog::initialize(ShipCAD::LayerDialogData* data, bool delete_data,
                             ShipCAD::unit_type_t units)
{
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
}

void LayerDialog::updateLayerList()
{
    QSignalBlocker block(ui->layerListWidget);
    ui->layerListWidget->clear();
    for (size_t i=0; i<_data->layers.size(); i++) {
        ui->layerListWidget->addItem(_data->layers[i].name);
    }
    ui->layerListWidget->setCurrentRow(_current);
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
    if (_current == 0)
        _moveUpAction->setEnabled(false);
    else
        _moveUpAction->setEnabled(true);
    if (_current == _data->layers.size() - 1)
        _moveDownAction->setEnabled(false);
    else
        _moveDownAction->setEnabled(true);
    ui->areaLabel->setText(QString(tr("Area %1 %2")
                                   .arg(truncate(cprops.layer_properties.surface_area, 2))
                                   .arg(_areastr)));
    ui->weightLabel->setText(QString(tr("Weight %1 %2")
                                     .arg(truncate(cprops.layer_properties.weight, 2))
                                     .arg(_weightstr)));
    ui->cogLabel->setText(QString(tr("Center of gravity %1,%2,%3 %4")
                                  .arg(MakeLength(cprops.layer_properties.surface_center_of_gravity.x(), 3, 7))
                                  .arg(MakeLength(cprops.layer_properties.surface_center_of_gravity.y(), 3, 7))
                                  .arg(MakeLength(cprops.layer_properties.surface_center_of_gravity.z(), 3, 7))
                                  .arg(_lengthstr)));
}

void LayerDialog::listRowChanged(int index)
{
    _current = index;
    _data->active = _data->layers[_current].data;
    updateState();
    emit activeLayerChanged(index);
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
    ChooseColorDialogData cdata(tr("Choose color for Layer"), cprops.color);
    emit exeChooseColorDialog(cdata);
    if (cdata.accepted) {
        cprops.color = cdata.chosen;
        emit layerColorChanged(cprops.color);
    }
}

void LayerDialog::moveUp()
{
    vector<LayerPropertiesForDialog>::iterator i = _data->layers.begin();
    i += _current;
    vector<LayerPropertiesForDialog>::iterator j = i;
    --i;
    j = _data->layers.insert(i, *j);
    i = j;
    ++j; ++j;
    _data->layers.erase(j);
    --_current;
    updateLayerList();
    //updateState();
    emit reorderLayerList(_data);
}

void LayerDialog::moveDown()
{
    vector<LayerPropertiesForDialog>::iterator i = _data->layers.begin();
    i += _current;
    vector<LayerPropertiesForDialog>::iterator j = i;
    ++i; ++i;
    j = _data->layers.insert(i, *j);
    i = j;
    --j; --j;
    _data->layers.erase(j);
    ++_current;
    updateLayerList();
    //updateState();
    emit reorderLayerList(_data);
}
