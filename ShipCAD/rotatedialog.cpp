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
    restoreGeometry(settings.value("rotatedialog-geometry").toByteArray());
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

