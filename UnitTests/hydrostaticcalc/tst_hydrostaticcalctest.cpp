#include <QString>
#include <QtTest>

#include "shipcadmodel.h"
#include "hydrostaticcalc.h"
#include "subdivsurface.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "projsettings.h"
#include "utility.h"

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
    face_points.push_back(p4);
	face_points.push_back(p3);
    face_points.push_back(p2);
    s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p1);
    face_points.push_back(p2);
	face_points.push_back(p6);
    face_points.push_back(p5);
    s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p2);
    face_points.push_back(p3);
	face_points.push_back(p7);
    face_points.push_back(p6);
    s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p3);
    face_points.push_back(p4);
	face_points.push_back(p8);
    face_points.push_back(p7);
    s->addControlFace(face_points);

	face_points.clear();
	face_points.push_back(p4);
    face_points.push_back(p1);
	face_points.push_back(p5);
    face_points.push_back(p8);
    s->addControlFace(face_points);

    // make all edges, creased
    for (size_t i=0; i<s->numberOfControlEdges(); i++) {
        SubdivisionControlEdge* e = s->getControlEdge(i);
        e->setCrease(true);
    }
    // set resolution to medium
    _model->setPrecision(fpMedium);
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
    QVERIFY2(FuzzyCompare(hc.getData().absolute_draft, .5, 1E-2), "draft s/b 0.5");
    QVERIFY2(FuzzyCompare(hc.getData().volume, .5, 1E-2), "volume s/b .5");
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
    QVERIFY(hc.getData().model_min == ZERO);
    QVERIFY(hc.getData().model_max == QVector3D(1,.5,1));
    QVERIFY(qFuzzyCompare(hc.getData().wl_min, QVector3D(0,-.5,.5)));
    QVERIFY(qFuzzyCompare(hc.getData().wl_max, QVector3D(1,.5,.5)));
    QVERIFY(qFuzzyCompare(hc.getData().sub_min, QVector3D(0,-.5,0)));
    QVERIFY(qFuzzyCompare(hc.getData().sub_max, QVector3D(1,.5,.5)));
    QVERIFY2(FuzzyCompare(hc.getData().absolute_draft, .5, 1E-2), "draft s/b 0.5");
    QVERIFY2(FuzzyCompare(hc.getData().volume, .5, 1E-2), "volume s/b .5");
    QVERIFY(FuzzyCompare(hc.getData().displacement, .5125, 1E-2));
    QVERIFY(qFuzzyCompare(hc.getData().center_of_buoyancy, QVector3D(0.5,0,.25)));
    QVERIFY(FuzzyCompare(hc.getData().lcb_perc, 0, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().length_waterline, 1, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().beam_waterline, 1, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().block_coefficient, 1, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().wetted_surface, 4, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().mainframe_area, .5, 1E-2));
    //QVERIFY(qFuzzyCompare(hc.getData().mainframe_cog, QVector3D(0.5,0,.25)));
    QVERIFY(FuzzyCompare(hc.getData().mainframe_coeff, 1, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().waterplane_area, 1, 1E-2));
    //QVERIFY(qFuzzyCompare(hc.getData().waterplane_cog, QVector3D(0.5,0,.5)));
    QVERIFY(FuzzyCompare(hc.getData().waterplane_entrance_angle, -90, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().waterplane_coeff, 1, 1E-2));
    //QVERIFY(qFuzzyCompare(hc.getData().waterplane_mom_inertia, QVector2D(0.04166,-.08333)));
    QVERIFY(FuzzyCompare(hc.getData().km_transverse, 0.3333, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().km_longitudinal, 0.0833, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().lateral_area, .5, 1E-2));
    //QVERIFY(qFuzzyCompare(hc.getData().lateral_cog, QVector3D(0.5,0.001,.25)));
    QVERIFY(FuzzyCompare(hc.getData().prism_coefficient, 1, 1E-2));
    QVERIFY(FuzzyCompare(hc.getData().vert_prism_coefficient, 1, 1E-2));
    QVERIFY(hc.getData().sac.size() == 0);
    QVERIFY(!hc.hasError(feNothingSubmerged) && !hc.hasError(feNotEnoughBuoyancy) && !hc.hasError(feMakingWater));
}

QTEST_APPLESS_MAIN(HydrostaticcalcTest)

#include "tst_hydrostaticcalctest.moc"
