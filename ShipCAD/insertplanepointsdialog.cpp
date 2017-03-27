#include "insertplanepointsdialog.h"
#include "ui_insertplanepointsdialog.h"

#include <QSettings>
#include "utility.h"

using namespace ShipCAD;
//using namespace std;

InsertPlanePointsDialog::InsertPlanePointsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InsertPlanePointsDialog)
{
    ui->setupUi(this);
    readSettings();
}

InsertPlanePointsDialog::~InsertPlanePointsDialog()
{
    saveSettings();
    delete ui;
}

QString InsertPlanePointsDialog::distanceValue() const
{
    return ui->distanceLineEdit->text();
}

bool InsertPlanePointsDialog::transversePlane() const
{
    return ui->transversePlaneRadioButton->isChecked();
}

bool InsertPlanePointsDialog::horizontalPlane() const
{
    return ui->horizontalPlaneRadioButton->isChecked();
}

bool InsertPlanePointsDialog::verticalPlane() const
{
    return ui->verticalPlaneRadioButton->isChecked();
}

bool InsertPlanePointsDialog::addControlCurveSelected() const
{
    return ui->addCurveCheckBox->isChecked();
}

void InsertPlanePointsDialog::setExtents(float min, float max)
{
    ui->minValueLabel->setText(MakeLength(min, 4, 10));
    ui->maxValueLabel->setText(MakeLength(max, 4, 10));
}

void InsertPlanePointsDialog::readSettings()
{
    QSettings settings;
    restoreGeometry(settings.value("insertplanedialog-geometry").toByteArray());
}

void InsertPlanePointsDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("insertplanedialog-geometry", saveGeometry());
}

void InsertPlanePointsDialog::closeEvent(QCloseEvent* event)
{
    saveSettings();
    QDialog::closeEvent(event);
}

