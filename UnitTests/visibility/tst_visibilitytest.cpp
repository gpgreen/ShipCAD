#include <QString>
#include <QtTest>

#include "visibility.h"
#include "shipcadmodel.h"
#include "filebuffer.h"

using namespace ShipCAD;

class VisibilityTest : public QObject
{
    Q_OBJECT

public:
    VisibilityTest();

    Visibility* getNonDefault();

private Q_SLOTS:
    void init();
    void cleanup();
    void testConstruct();
    void testWriteRead();

private:
    ShipCADModel* _model;
};

VisibilityTest::VisibilityTest()
    : _model(0)
{
   // does nothing
}

void VisibilityTest::init()
{
    _model = new ShipCADModel();
}

void VisibilityTest::cleanup()
{
    delete _model;
    _model = 0;
}

Visibility* VisibilityTest::getNonDefault()
{
    Visibility* vis = new Visibility(_model);
    // set attributes
    vis->setModelView(mvBoth);
    vis->setShowControlNet(false);
    vis->setShowInteriorEdges(true);
    vis->setShowStations(false);
    vis->setShowButtocks(false);
    vis->setShowWaterlines(false);
    vis->setShowNormals(false);
    vis->setShowGrid(false);
    vis->setShowDiagonals(false);
    vis->setShowMarkers(false);
    vis->setShowCurvature(false);
    vis->increaseCurvatureScale();
    vis->setShowControlCurves(false);
    vis->setCursorIncrement(0.2f);
    vis->setShowHydrostaticData(false);
    vis->setShowHydrostaticDisplacement(false);
    vis->setShowHydrostaticLateralArea(false);
    vis->setShowHydrostaticSectionalAreas(false);
    vis->setShowHydrostaticMetacentricHeight(false);
    vis->setShowHydrostaticLCF(false);
    vis->setShowFlowlines(false);
    return vis;
}

void VisibilityTest::testConstruct()
{
    Visibility* vis = getNonDefault();
    QVERIFY2(_model->isFileChanged(), "is file changed");
    delete vis;
}

void VisibilityTest::testWriteRead()
{
    Visibility* visW = getNonDefault();
    FileBuffer dest;
    visW->saveBinary(dest);
    Visibility* visR = new Visibility(_model);
    dest.reset();
    visR->loadBinary(dest);
    QVERIFY(visW->getModelView() == visR->getModelView());
    QVERIFY(visW->isShowControlNet() == visR->isShowControlNet());
    QVERIFY(visW->isShowInteriorEdges() == visR->isShowInteriorEdges());
    QVERIFY(visW->isShowStations() == visR->isShowStations());
    QVERIFY(visW->isShowButtocks() == visR->isShowButtocks());
    QVERIFY(visW->isShowWaterlines() == visR->isShowWaterlines());
    QVERIFY(visW->isShowNormals() == visR->isShowNormals());
    QVERIFY(visW->isShowGrid() == visR->isShowGrid());
    QVERIFY(visW->isShowDiagonals() == visR->isShowDiagonals());
    QVERIFY(visW->isShowMarkers() == visR->isShowMarkers());
    QVERIFY(visW->isShowCurvature() == visR->isShowCurvature());
    QVERIFY(visW->getCurvatureScale() == visR->getCurvatureScale());
    QVERIFY(visW->isShowControlCurves() == visR->isShowControlCurves());
    QVERIFY(visW->getCursorInc() == visR->getCursorInc());
    QVERIFY(visW->isShowHydrostaticData() == visR->isShowHydrostaticData());
    QVERIFY(visW->isShowHydrostaticDisplacement() == visR->isShowHydrostaticDisplacement());
    QVERIFY(visW->isShowHydrostaticLateralArea() == visR->isShowHydrostaticLateralArea());
    QVERIFY(visW->isShowHydrostaticSectionalAreas() == visR->isShowHydrostaticSectionalAreas());
    QVERIFY(visW->isShowHydrostaticMetacentricHeight() == visR->isShowHydrostaticMetacentricHeight());
    QVERIFY(visW->isShowHydrostaticLCF() == visR->isShowHydrostaticLCF());
    QVERIFY(visW->isShowFlowlines() == visR->isShowFlowlines());
    delete visW;
    delete visR;
}

QTEST_APPLESS_MAIN(VisibilityTest)

#include "tst_visibilitytest.moc"
