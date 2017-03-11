#include <iostream>

#include "pointdialog.h"
#include "ui_pointdialog.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "shipcadlib.h"

using namespace ShipCAD;
using namespace std;

PointDialog::PointDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PointDialog)
{
    ui->setupUi(this);
    setModal(false);
    connect(ui->checkBoxCornerPoint, SIGNAL(clicked(bool)), SIGNAL(cornerPointSelect(bool)));
    connect(ui->doubleSpinBoxX, SIGNAL(valueChanged(double)), SLOT(dialogUpdatePointCoord(double)));
    connect(ui->doubleSpinBoxY, SIGNAL(valueChanged(double)), SLOT(dialogUpdatePointCoord(double)));
    connect(ui->doubleSpinBoxZ, SIGNAL(valueChanged(double)), SLOT(dialogUpdatePointCoord(double)));
    ui->doubleSpinBoxX->setRange(-9999,9999);
    ui->doubleSpinBoxY->setRange(-9999,9999);
    ui->doubleSpinBoxZ->setRange(-9999,9999);
    ui->doubleSpinBoxX->setSingleStep(0.001);
    ui->doubleSpinBoxY->setSingleStep(0.001);
    ui->doubleSpinBoxZ->setSingleStep(0.001);
    readSettings();
}

PointDialog::~PointDialog()
{
    saveSettings();
    delete ui;
}

void PointDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

void PointDialog::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("pointdialog-geometry").toByteArray());
}

void PointDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("pointdialog-geometry", saveGeometry());
}

void PointDialog::setActive(bool active)
{
    if (active) {
        readSettings();
        show();
    }
    else {
        saveSettings();
        hide();
    }
}

void PointDialog::dialogUpdatePointCoord(double /*d*/)
{
    emit pointCoordChange(ui->doubleSpinBoxX->value(),
                          ui->doubleSpinBoxY->value(),
                          ui->doubleSpinBoxZ->value());
}

void PointDialog::controllerUpdatedPoint(SubdivisionControlPoint* pt)
{
    cout << "pointdialog: controller update point" << endl;
    if (pt == 0) {
        ui->doubleSpinBoxX->setValue(0.0);
        ui->doubleSpinBoxY->setValue(0.0);
        ui->doubleSpinBoxZ->setValue(0.0);
        ui->checkBoxCornerPoint->setChecked(false);
        ui->doubleSpinBoxX->setEnabled(false);
        ui->doubleSpinBoxY->setEnabled(false);
        ui->doubleSpinBoxZ->setEnabled(false);
        ui->checkBoxCornerPoint->setEnabled(false);
        return;
    }
    ui->doubleSpinBoxX->setValue(pt->getCoordinate().x());
    ui->doubleSpinBoxY->setValue(pt->getCoordinate().y());
    ui->doubleSpinBoxZ->setValue(pt->getCoordinate().z());
    ui->checkBoxCornerPoint->setChecked(pt->getVertexType() == svCorner);
    if (pt->isLocked()) {
        ui->doubleSpinBoxX->setEnabled(false);
        ui->doubleSpinBoxY->setEnabled(false);
        ui->doubleSpinBoxZ->setEnabled(false);
        ui->checkBoxCornerPoint->setEnabled(false);
    } else {
        ui->doubleSpinBoxX->setEnabled(true);
        ui->doubleSpinBoxY->setEnabled(true);
        ui->doubleSpinBoxZ->setEnabled(true);
        // points with more than 2 crease edges must always be a corner
        // enable corner point select if not regular, but not reqd to be
        // a corner
        size_t n = 0;
        for (size_t i=0; i<pt->numberOfEdges(); i++) {
            if (pt->getEdge(i)->isCrease())
                n++;
        }
        if (n > 0 && n <3)
            ui->checkBoxCornerPoint->setEnabled(true);
        else
            ui->checkBoxCornerPoint->setEnabled(false);
    }
}
