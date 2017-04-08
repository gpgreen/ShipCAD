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
    restoreGeometry(settings.value("mirrordialog-geometry").toByteArray());
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

