#include <QString>
#include <QtTest>

#include "spline.h"

using namespace ShipCAD;

class SplineTest : public QObject
{
    Q_OBJECT

public:
    SplineTest();

private Q_SLOTS:
    void testCase1();
};

SplineTest::SplineTest()
{
}

void SplineTest::testCase1()
{
    Spline s;
    s.add(QVector3D(0,0,0));
    s.add(QVector3D(1,1,1));
    s.add(QVector3D(2,2,2));

    QVERIFY2(s.numberOfPoints() == 3, "Failure");
}

QTEST_APPLESS_MAIN(SplineTest)

#include "tst_splinetest.moc"
