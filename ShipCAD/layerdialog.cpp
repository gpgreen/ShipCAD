#include "layerdialog.h"
#include "ui_layerdialog.h"
#include "shipcadlib.h"

LayerDialog::LayerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LayerDialog)
{
    ui->setupUi(this);
    readSettings();
}

LayerDialog::~LayerDialog()
{
    saveSettings();
    delete ui;
}

void LayerDialog::initialize(ShipCAD::LayerDialogData& data)
{
    // does nothing
}

void LayerDialog::retrieve(ShipCAD::LayerDialogData& data)
{
}

void LayerDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void LayerDialog::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("layerdialog-geometry").toByteArray());
}

void LayerDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("layerdialog-geometry", saveGeometry());
}

