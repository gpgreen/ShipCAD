#include <QString>
#include <QtTest>

#include "projsettings.h"
#include "shipcadmodel.h"
#include "filebuffer.h"

using namespace ShipCAD;

class ProjsettingsTest : public QObject
{
    Q_OBJECT

public:
    ProjsettingsTest();

    ProjectSettings* getNonDefault();

private Q_SLOTS:
    void init();
    void cleanup();
    void testConstruct();
    void testWriteRead();

private:
    ShipCADModel* _model;
};

ProjsettingsTest::ProjsettingsTest()
    : _model(0)
{
   // does nothing
}

void ProjsettingsTest::init()
{
    _model = new ShipCADModel();
    //qDebug("called at start of each test");
}

void ProjsettingsTest::cleanup()
{
    delete _model;
    _model = 0;
    //qDebug("called at end of each test");
}

ProjectSettings* ProjsettingsTest::getNonDefault()
{
    ProjectSettings* settings = new ProjectSettings(_model);
    // set attributes
    settings->setName("Testship");
    settings->setDesigner("designer");
    settings->setLength(3.0f);
    settings->setBeam(2.0f);
    settings->setDraft(1.0f);
    settings->setAppendageCoefficient(1.02f);
    settings->setShadeUnderwaterShip(false);
    settings->setUnderWaterColor(Qt::black);
    settings->setUnits(fuImperial);
    // set water density after setting units, as that changes it
    settings->setWaterDensity(1.2f);
    settings->setMainframeLocation(1.2f);
    settings->setDisableModelCheck(true);
    settings->setComment("a comment");
    settings->setFileCreatedBy("a designer");
    settings->setHydrostaticCoefficients(fcActualData); // TODO: changes owner calcs
    settings->setSimplifyIntersections(false);
    settings->setStartDraft(0.5f);
    settings->setEndDraft(1.5f);
    settings->setDraftStep(0.1f);
    settings->setTrim(0.2f);
    settings->setUseDisplacementIncrements(false);
    // TODO: displacements
    settings->setMinDisplacement(40.0f);
    settings->setMaxDisplacement(45.0f);
    settings->setDisplacementInc(1.0f);
    // TODO: angles
    // TODO: stab trims
    settings->setFreeTrim(false);
    settings->setFVCG(1.5f);
    // TODO: need to change this so it doesn't match default
    settings->setSavePreview(false);
    
    return settings;
}

void ProjsettingsTest::testConstruct()
{
    ProjectSettings* settings = getNonDefault();
    QVERIFY2(_model->isFileChanged(), "is file changed");
    delete settings;
}

void ProjsettingsTest::testWriteRead()
{
    ProjectSettings* settingsW = getNonDefault();
    FileBuffer dest;
    settingsW->saveBinary(dest);
    ProjectSettings* settingsR = new ProjectSettings(_model);
    dest.reset();
    settingsR->loadBinary(dest, 0);
    QVERIFY(settingsW->getName() == settingsR->getName());
    QVERIFY(settingsW->getDesigner() == settingsR->getDesigner());
    QVERIFY(settingsW->getLength() == settingsR->getLength());
    QVERIFY(settingsW->getBeam() == settingsR->getBeam());
    QVERIFY(settingsW->getDraft() == settingsR->getDraft());
    QVERIFY(settingsW->isMainParticularsSet() == settingsR->isMainParticularsSet());
    QVERIFY(settingsW->getWaterDensity() == settingsR->getWaterDensity());
    QVERIFY(settingsW->getAppendageCoefficient() == settingsR->getAppendageCoefficient());
    QVERIFY(settingsW->isShadeUnderwaterShip() == settingsR->isShadeUnderwaterShip());
    QVERIFY(settingsW->getUnderWaterColor() == settingsR->getUnderWaterColor());
    QVERIFY(settingsW->getUnits() == settingsR->getUnits());
    QVERIFY(settingsW->getMainframeLocation() == settingsR->getMainframeLocation());
    QVERIFY(settingsW->isDisableModelCheck() == settingsR->isDisableModelCheck());
    QVERIFY(settingsW->getComment() == settingsR->getComment());
    QVERIFY(settingsW->getFileCreatedBy() == settingsR->getFileCreatedBy());
    QVERIFY(settingsW->getHydrostaticCoefficients() == settingsR->getHydrostaticCoefficients());
    QVERIFY(settingsW->isSavePreview() == settingsR->isSavePreview());
    QVERIFY(settingsW->isSimplifyIntersections() == settingsR->isSimplifyIntersections());
    QVERIFY(settingsW->useDisplacementIncrements() == settingsR->useDisplacementIncrements());
    QVERIFY(settingsW->getStartDraft() == settingsR->getStartDraft());
    QVERIFY(settingsW->getTrim() == settingsR->getTrim());
    QVERIFY(settingsW->getEndDraft() == settingsR->getEndDraft());
    QVERIFY(settingsW->getDraftStep() == settingsR->getDraftStep());
    QVERIFY(settingsW->getMinDisplacement() == settingsR->getMinDisplacement());
    QVERIFY(settingsW->getMaxDisplacement() == settingsR->getMaxDisplacement());
    QVERIFY(settingsW->getDisplacementInc() == settingsR->getDisplacementInc());
    QVERIFY(settingsW->isFreeTrim() == settingsR->isFreeTrim());
    QVERIFY(settingsW->getFVCG() == settingsR->getFVCG());
    delete settingsW;
    delete settingsR;
}

QTEST_APPLESS_MAIN(ProjsettingsTest)

#include "tst_projsettingstest.moc"
