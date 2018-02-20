#include <QSignalBlocker>
#include "projectsettingsdialog.h"
#include "ui_projectsettingsdialog.h"

#include "dialogdata.h"
#include "projsettings.h"
#include "visibility.h"

using namespace ShipCAD;

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectSettingsDialog), _data(nullptr)
{
    ui->setupUi(this);
    ui->unitsComboBox->addItem(tr("Imperial"));
    ui->unitsComboBox->addItem(tr("Metric"));
    ui->coefficientComboBox->addItem(tr("Project settings"));
    ui->coefficientComboBox->addItem(tr("Actual data"));
    ui->lengthLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    ui->beamLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    ui->draftLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    ui->midshipLocationLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    ui->waterDensityLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );
    ui->appendageCoefficientLineEdit->setValidator( new QDoubleValidator(0, 10000000, 4) );

    // connect
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(retrieve()));
    connect(ui->unitsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitsChanged(int)));
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete ui;
}

void ProjectSettingsDialog::unitsChanged(int index)
{
    if (index == 0)
        _data->units = fuImperial;
    else if (index == 1)
        _data->units = fuMetric;
    _data->settings.changeUnitsInSettingsOnly(_data->units);
    initialize(_data);
}

void ProjectSettingsDialog::initialize(ProjectSettingsDialogData* data)
{
    const QSignalBlocker blocker(this);
    _data = data;
    ui->projectNameLineEdit->setText(_data->settings.getName());
    ui->designerLineEdit->setText(_data->settings.getDesigner());
    ui->commentLineEdit->setText(_data->settings.getComment());
    ui->fileCreatedByLineEdit->setText(_data->settings.getFileCreatedBy());
    switch(_data->units) {
    case fuImperial:
        ui->unitsComboBox->setCurrentIndex(0);
        break;
    case fuMetric:
        ui->unitsComboBox->setCurrentIndex(1);
        break;
    }
    QString lenstr = LengthStr(_data->units);
    QString densitystr = DensityStr(_data->settings.getUnits());
    ui->shadeUnderwaterCheckBox->setChecked(_data->settings.isShadeUnderwaterShip());
    ui->savePreviewImageCheckBox->setChecked(_data->settings.isSimplifyIntersections());
    ui->lengthLineEdit->setText(QString("%1").arg(_data->settings.getLength()));
    ui->lengthUnitLabel->setText(lenstr);
    ui->beamLineEdit->setText(QString("%1").arg(_data->settings.getBeam()));
    ui->beamUnitLabel->setText(lenstr);
    ui->draftLineEdit->setText(QString("%1").arg(_data->settings.getDraft()));
    ui->draftUnitLabel->setText(lenstr);
    ui->midshipLocationLineEdit->setText(QString("%1").arg(_data->settings.getMainframeLocation()));
    ui->midshipLocationUnitLabel->setText(lenstr);
    ui->defaultMidshipLocationCheckBox->setChecked(_data->settings.useDefaultMainframeLocation());
    ui->appendageCoefficientLineEdit->setText(QString("%1").arg(_data->settings.getAppendageCoefficient()));
    ui->waterDensityLineEdit->setText(QString("%1").arg(_data->settings.getWaterDensity()));
    ui->waterDensityUnitLabel->setText(densitystr);
    switch(_data->settings.getHydrostaticCoefficients())
    {
    case fcProjectSettings:
        ui->coefficientComboBox->setCurrentIndex(0);
        break;
    case fcActualData:
        ui->coefficientComboBox->setCurrentIndex(1);
        break;
    }
    ui->disableSurfaceCheckCheckBox->setChecked(_data->settings.isDisableModelCheck());
    ui->displacementCheckBox->setChecked(_data->visibility.isShowHydrostaticDisplacement());
    ui->sectionalAreasCheckBox->setChecked(_data->visibility.isShowHydrostaticSectionalAreas());
    ui->metacentricCheckBox->setChecked(_data->visibility.isShowHydrostaticMetacentricHeight());
    ui->longitudinalCheckBox->setChecked(_data->visibility.isShowHydrostaticLCF());
    ui->lateralAreaCheckBox->setChecked(_data->visibility.isShowHydrostaticLateralArea());
}

void ProjectSettingsDialog::retrieve()
{
    _data->settings.setName(ui->projectNameLineEdit->text());
    _data->settings.setDesigner(ui->designerLineEdit->text());
    _data->settings.setComment(ui->commentLineEdit->text());
    _data->settings.setFileCreatedBy(ui->fileCreatedByLineEdit->text());
    // don't use setUnits here as it will changes the model's coordinates, if
    // the units are changed, we want to do that in the controller, not here
    if (ui->unitsComboBox->currentIndex() == 0)
        _data->units = fuImperial;
    else if (ui->unitsComboBox->currentIndex() == 1)
        _data->units = fuMetric;
    _data->settings.setShadeUnderwaterShip(ui->shadeUnderwaterCheckBox->isChecked());
    _data->settings.setSimplifyIntersections(ui->savePreviewImageCheckBox->isChecked());
    _data->settings.setLength(ui->lengthLineEdit->text().toFloat());
    _data->settings.setBeam(ui->beamLineEdit->text().toFloat());
    _data->settings.setDraft(ui->draftLineEdit->text().toFloat());
    _data->settings.setMainframeLocation(ui->midshipLocationLineEdit->text().toFloat());
    _data->settings.setDefaultMainframeLocation(ui->defaultMidshipLocationCheckBox->isChecked());
    _data->settings.setAppendageCoefficient(ui->appendageCoefficientLineEdit->text().toFloat());
    _data->settings.setWaterDensity(ui->waterDensityLineEdit->text().toFloat());
    if (ui->coefficientComboBox->currentIndex() == 0)
        _data->settings.setHydrostaticCoefficients(fcProjectSettings);
    else if (ui->coefficientComboBox->currentIndex() == 1)
        _data->settings.setHydrostaticCoefficients(fcActualData);
    _data->settings.setDisableModelCheck(ui->disableSurfaceCheckCheckBox->isChecked());
    _data->visibility.setShowHydrostaticDisplacement(ui->displacementCheckBox->isChecked());
    _data->visibility.setShowHydrostaticSectionalAreas(ui->sectionalAreasCheckBox->isChecked());
    _data->visibility.setShowHydrostaticMetacentricHeight(ui->metacentricCheckBox->isChecked());
    _data->visibility.setShowHydrostaticLCF(ui->longitudinalCheckBox->isChecked());
    _data->visibility.setShowHydrostaticLateralArea(ui->lateralAreaCheckBox->isChecked());
}
