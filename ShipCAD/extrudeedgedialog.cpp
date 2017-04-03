#include <QSettings>

#include "extrudeedgedialog.h"
#include "ui_extrudeedgedialog.h"
#include "controller.h"
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

void ExtrudeEdgeDialog::initialize(ShipCAD::ExtrudeEdgeDialogData& data)
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
    restoreGeometry(settings.value("extrudeedgedialog-geometry").toByteArray());
}

void ExtrudeEdgeDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("extrudeedgedialog-geometry", saveGeometry());
}

