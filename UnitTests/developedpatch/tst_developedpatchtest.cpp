#include <QString>
#include <QtTest>

#include "developedpatch.h"
#include "subdivlayer.h"
#include "subdivsurface.h"

using namespace ShipCAD;

class DevelopedpatchTest : public QObject
{
    Q_OBJECT

public:
    DevelopedpatchTest();

private Q_SLOTS:
    void testConstruct();
};

DevelopedpatchTest::DevelopedpatchTest()
{
}

void DevelopedpatchTest::testConstruct()
{
    SubdivisionSurface surface;
    SubdivisionLayer layer(&surface);
    
    DevelopedPatch dp(&layer);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(DevelopedpatchTest)

#include "tst_developedpatchtest.moc"
