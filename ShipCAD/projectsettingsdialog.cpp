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
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete ui;
}

void ProjectSettingsDialog::initialize(ProjectSettingsDialogData* data)
{
    _data = data;
    ui->projectNameLineEdit->setText(data->settings.getName());
    ui->designerLineEdit->setText(data->settings.getDesigner());
    ui->commentLineEdit->setText(data->settings.getComment());
    ui->fileCreatedByLineEdit->setText(data->settings.getFileCreatedBy());
    switch(data->settings.getUnits()) {
    case fuImperial:
        ui->unitsComboBox->setCurrentIndex(0);
        break;
    case fuMetric:
        ui->unitsComboBox->setCurrentIndex(1);
        break;
    }
    QString lenstr = LengthStr(data->settings.getUnits());
    QString densitystr = DensityStr(data->settings.getUnits());
    ui->shadeUnderwaterCheckBox->setChecked(data->settings.isShadeUnderwaterShip());
    ui->savePreviewImageCheckBox->setChecked(data->settings.isSimplifyIntersections());
    ui->lengthLineEdit->setText(QString("%1").arg(data->settings.getLength()));
    ui->lengthUnitLabel->setText(lenstr);
    ui->beamLineEdit->setText(QString("%1").arg(data->settings.getBeam()));
    ui->beamUnitLabel->setText(lenstr);
    ui->draftLineEdit->setText(QString("%1").arg(data->settings.getDraft()));
    ui->draftUnitLabel->setText(lenstr);
    ui->midshipLocationLineEdit->setText(QString("%1").arg(data->settings.getMainframeLocation()));
    ui->midshipLocationUnitLabel->setText(lenstr);
    ui->defaultMidshipLocationCheckBox->setChecked(data->settings.useDefaultMainframeLocation());
    ui->appendageCoefficientLineEdit->setText(QString("%1").arg(data->settings.getAppendageCoefficient()));
    ui->waterDensityLineEdit->setText(QString("%1").arg(data->settings.getWaterDensity()));
    ui->waterDensityUnitLabel->setText(densitystr);
    switch(data->settings.getHydrostaticCoefficients())
    {
    case fcProjectSettings:
        ui->coefficientComboBox->setCurrentIndex(0);
        break;
    case fcActualData:
        ui->coefficientComboBox->setCurrentIndex(1);
        break;
    }
    ui->disableSurfaceCheckCheckBox->setChecked(data->settings.isDisableModelCheck());
    ui->displacementCheckBox->setChecked(data->visibility.isShowHydrostaticDisplacement());
    ui->sectionalAreasCheckBox->setChecked(data->visibility.isShowHydrostaticSectionalAreas());
    ui->metacentricCheckBox->setChecked(data->visibility.isShowHydrostaticMetacentricHeight());
    ui->longitudinalCheckBox->setChecked(data->visibility.isShowHydrostaticLCF());
    ui->lateralAreaCheckBox->setChecked(data->visibility.isShowHydrostaticLateralArea());
}
