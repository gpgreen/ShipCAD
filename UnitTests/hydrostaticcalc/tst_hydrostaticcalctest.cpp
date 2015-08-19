#include <QString>
#include <QtTest>

#include "shipcadmodel.h"
#include "hydrostaticcalc.h"
#include "subdivsurface.h"
#include "subdivface.h"
#include "subdivedge.h"

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
	// build a cube with an open top
    SubdivisionSurface* s = _model->getSurface();
	QVector3D p1(0,0,0);
    QVector3D p2(1,0,0);
    QVector3D p3(1,.5,0);
    QVector3D p4(0,.5,0);
	QVector3D p5(0,0,1);
    QVector3D p6(1,0,1);
    QVector3D p7(1,.5,1);
    QVector3D p8(0,.5,1);
	vector<QVector3D> face_points;

	face_points.push_back(p1);
	face_points.push_back(p2);
	face_points.push_back(p3);
	face_points.push_back(p4);
    SubdivisionControlFace* f1 = s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p1);
	face_points.push_back(p2);
	face_points.push_back(p6);
	face_points.push_back(p5);
    SubdivisionControlFace* f2 = s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p2);
	face_points.push_back(p3);
	face_points.push_back(p7);
	face_points.push_back(p6);
    SubdivisionControlFace* f3 = s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p3);
	face_points.push_back(p4);
	face_points.push_back(p8);
	face_points.push_back(p7);
    SubdivisionControlFace* f4 = s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p4);
	face_points.push_back(p1);
	face_points.push_back(p5);
	face_points.push_back(p8);
    SubdivisionControlFace* f5 = s->addControlFace(face_points);

    // make all edges, creased
    for (size_t i=0; i<s->numberOfControlEdges(); i++) {
        SubdivisionControlEdge* e = s->getControlEdge(i);
        e->setCrease(true);
    }
}
HydrostaticcalcTest::~HydrostaticcalcTest()
{
    delete _model;
}

void HydrostaticcalcTest::testCase1()
{
	HydrostaticCalc hc(_model);

    Plane wl(0,0,1,-.5);
	hc.calculateVolume(wl);
	
    QVERIFY2(true, "Failure");
    QStringList s;
    hc.addData(s, fhSingleCalculation, ' ');
    QFile of("testcase1.txt");
    if (!of.open(QFile::WriteOnly | QFile::Truncate))
        QVERIFY2(false, "Couldn't open data file");
    QTextStream os(&of);
    for (int i=0; i<s.size(); i++)
        os << s[i] << endl;
    of.close();
}

QTEST_APPLESS_MAIN(HydrostaticcalcTest)

#include "tst_hydrostaticcalctest.moc"
