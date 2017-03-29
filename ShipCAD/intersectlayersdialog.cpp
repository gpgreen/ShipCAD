#include <QSettings>

#include "intersectlayersdialog.h"
#include "ui_intersectlayersdialog.h"
#include "controller.h"
#include "subdivlayer.h"

IntersectLayersDialog::IntersectLayersDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IntersectLayersDialog)
{
    ui->setupUi(this);
    connect(ui->layer1ComboBox, SIGNAL(clicked()), SLOT(layer1Changed()));
    readSettings();
}

IntersectLayersDialog::~IntersectLayersDialog()
{
    saveSettings();
    delete ui;
}

void IntersectLayersDialog::initialize(ShipCAD::IntersectLayersDialogData& data)
{
    ui->layer1ComboBox->clear();
    ui->layer2ComboBox->clear();
    for (size_t i=0; i<data.layers.size(); i++) {
        ui->layer1ComboBox->addItem(data.layers[i]->getName());
        if (i == data.layer1)
            continue;
        ui->layer2ComboBox->addItem(data.layers[i]->getName());
    }
}

void IntersectLayersDialog::retrieve(ShipCAD::IntersectLayersDialogData& data)
{
    QString ln1 = ui->layer1ComboBox->currentText();
    QString ln2 = ui->layer2ComboBox->currentText();
    for (size_t i=0; i<data.layers.size(); i++) {
        if (ln1 == data.layers[i]->getName())
            data.layer1 = i;
        else if (ln2 == data.layers[i]->getName())
            data.layer2 = i;
    }
}

void IntersectLayersDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void IntersectLayersDialog::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("intersectlayersdialog-geometry").toByteArray());
}

void IntersectLayersDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("intersectlayersdialog-geometry", saveGeometry());
}

void IntersectLayersDialog::layer1Changed()
{
    // alter layer2 combo box to exclude layer picked in combo box1
    int idx2 = ui->layer2ComboBox->currentIndex();
    QString item2(ui->layer2ComboBox->itemText(idx2));
    ui->layer2ComboBox->clear();
    int idx1 = ui->layer1ComboBox->currentIndex();
    for (int i=0; i<ui->layer1ComboBox->count(); i++) {
        if (i == idx1)
            continue;
        ui->layer2ComboBox->addItem(ui->layer1ComboBox->itemText(i));
    }
    // select the first item
    ui->layer2ComboBox->setCurrentIndex(0);
}
