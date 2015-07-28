#include <QString>
#include <QtTest>
#include "subdivpoint.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "subdivsurface.h"

using namespace ShipCADGeometry;

class SubdivcontrolpointTest : public QObject
{
    Q_OBJECT

public:
    SubdivcontrolpointTest();
    ~SubdivcontrolpointTest();
private:
    SubdivisionSurface* _owner;
private Q_SLOTS:
    void testCaseConstruct();
    void testCaseSetVertexLockedSelected();
};

SubdivcontrolpointTest::SubdivcontrolpointTest()
{
    _owner = new SubdivisionSurface();
}

SubdivcontrolpointTest::~SubdivcontrolpointTest()
{
    delete _owner;
}

void SubdivcontrolpointTest::testCaseConstruct()
{
    SubdivisionControlPoint *pt = SubdivisionControlPoint::construct(_owner);
    QVERIFY(pt->getCoordinate()[0] == 0 &&
            pt->getCoordinate()[1] == 0 &&
            pt->getCoordinate()[2] == 0);
    float c = pt->getCurvature();
    QVERIFY(c == 0.0 || c == 360.0);
    QVERIFY(!pt->isBoundaryVertex());
    QVERIFY(pt->getNormal()[0] == 0 &&
            pt->getNormal()[1] == 0 &&
            pt->getNormal()[2] == 0);
    QVERIFY(pt->getVertexType() == SubdivisionPoint::svRegular);
    QVERIFY(!pt->isLocked());
    QVERIFY(pt->isVisible());
    QVERIFY(!pt->isLeak());
    QVERIFY(!pt->isSelected());
}

void SubdivcontrolpointTest::testCaseSetVertexLockedSelected()
{
    SubdivisionControlPoint *pt = SubdivisionControlPoint::construct(_owner);
    pt->setVertexType(SubdivisionPoint::svCorner);
    QVERIFY(pt->getVertexType() == SubdivisionPoint::svCorner);
    pt->setLocked(true);
    pt->setSelected(true);
    QVERIFY(pt->isLocked());
    QVERIFY(pt->isSelected());
}

QTEST_APPLESS_MAIN(SubdivcontrolpointTest)

#include "tst_subdivcontrolpointtest.moc"
