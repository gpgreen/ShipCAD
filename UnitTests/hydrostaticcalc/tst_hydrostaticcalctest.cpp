#include <QString>
#include <QtTest>

#include "shipcadmodel.h"
#include "hydrostaticcalc.h"
#include "subdivsurface.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "projsettings.h"

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
    void testCalculateVolume();
    void testCalculate();
};

HydrostaticcalcTest::HydrostaticcalcTest()
{
	_model = new ShipCADModel();
    ProjectSettings& ps = _model->getProjectSettings();
    ps.setLength(1.0);
    ps.setBeam(1.0);
    ps.setDraft(0.5);

    // build a 1m x 1m cube with an open top
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

void HydrostaticcalcTest::testCalculateVolume()
{
	HydrostaticCalc hc(_model);

    Plane wl(0,0,1,-.5);
	hc.calculateVolume(wl);
	
    QStringList s;
    hc.addData(s, fhSingleCalculation, ' ');
    QFile of("testcalculatevolume.txt");
    if (!of.open(QFile::WriteOnly | QFile::Truncate))
        QVERIFY2(false, "Couldn't open data file");
    QTextStream os(&of);
    for (int i=0; i<s.size(); i++)
        os << s[i] << endl;
    of.close();

    QVERIFY2(hc.isCalculated(), "s/b calculated");
    QVERIFY2(hc.getData().absolute_draft == .5, "draft s/b 0.5");
    QVERIFY2(hc.getData().volume == .5, "volume s/b .5");
}

void HydrostaticcalcTest::testCalculate()
{
    HydrostaticCalc hc(_model);
    hc.setDraft(0.5);
    hc.setHeelingAngle(0.0);
    hc.setTrim(0.0);
    hc.addCalculationType(hcAll);

    hc.calculate();

    QStringList s;
    hc.addData(s, fhSingleCalculation, ' ');
    QFile of("testcalculate.txt");
    if (!of.open(QFile::WriteOnly | QFile::Truncate))
        QVERIFY2(false, "Couldn't open data file");
    QTextStream os(&of);
    for (int i=0; i<s.size(); i++)
        os << s[i] << endl;
    of.close();

    QVERIFY2(hc.isCalculated(), "s/b calculated");
    QVERIFY2(hc.getData().absolute_draft == .5, "draft s/b 0.5");
    QVERIFY2(hc.getData().volume == .5, "volume s/b .5");
}

QTEST_APPLESS_MAIN(HydrostaticcalcTest)

#include "tst_hydrostaticcalctest.moc"
