#include <QString>
#include <QtTest>

#include "shipcadmodel.h"
#include "hydrostaticcalc.h"

using namespace ShipCAD;
using namespace std;

class HydrostaticcalcTest : public QObject
{
    Q_OBJECT

public:
    HydrostaticcalcTest();
	~HydrostaticcalcTest();
	
private:
	ShipCADModel* _model;
						
private Q_SLOTS:
    void testCase1();
};

HydrostaticcalcTest::HydrostaticcalcTest()
{
	_model = new ShipCADModel();
}
HydrostaticcalcTest::~HydrostaticcalcTest()
{
    delete _model;
}

void HydrostaticcalcTest::testCase1()
{
	HydrostaticCalc hc(_model);
	
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(HydrostaticcalcTest)

#include "tst_hydrostaticcalctest.moc"
