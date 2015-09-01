#include <iostream>
#include <fstream>

#include <QString>
#include <QtTest>

#include "intersection.h"
#include "shipcadlib.h"
#include "shipcadmodel.h"
#include "subdivsurface.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "projsettings.h"
#include "utility.h"

using namespace ShipCAD;
using namespace std;

class IntersectionTest : public QObject
{
    Q_OBJECT

public:
    IntersectionTest();

    ShipCADModel* _model;

private Q_SLOTS:
    void testConstructWL();
    void testConstructSta();
    void testArea();
    void testDXF();
};

IntersectionTest::IntersectionTest()
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
    // set resolution to medium
    _model->setPrecision(fpMedium);
}

void IntersectionTest::testConstructWL()
{
    Plane halfcube(0,0,-1,0.5);

    Intersection i(_model, fiWaterline, halfcube, true);

    QVERIFY(i.getIntersectionType() == fiWaterline);
    QVERIFY(i.useHydrostaticsSurfacesOnly());
    QVERIFY(i.getSplines().size() == 0);
    QVERIFY(i.getPlane().a() == halfcube.a() && i.getPlane().b() == halfcube.b()
            && i.getPlane().c() == halfcube.c() && i.getPlane().d() == halfcube.d());

    i.rebuild();
    Spline* sp = i.getSplines().get(0);

    QVERIFY(sp->numberOfPoints() == 9);

    // write out the intersection
    ofstream os("intersection_wl_construct.txt");
    std::vector<Spline*>::iterator itr = i.getSplines().begin();
    for( ; itr!=i.getSplines().end(); itr++)
        (*itr)->dump(os);
    os.flush();
    os.close();
}

void IntersectionTest::testConstructSta()
{
    Plane halfcube(-1,0,0,0.5);

    Intersection i(_model, fiStation, halfcube, true);

    QVERIFY(i.getIntersectionType() == fiStation);
    QVERIFY(i.useHydrostaticsSurfacesOnly());
    QVERIFY(i.getSplines().size() == 0);
    QVERIFY(i.getPlane().a() == halfcube.a() && i.getPlane().b() == halfcube.b()
            && i.getPlane().c() == halfcube.c() && i.getPlane().d() == halfcube.d());

    i.rebuild();
    Spline* sp = i.getSplines().get(0);

    QVERIFY(sp->numberOfPoints() == 7);

    // write out the intersection
    ofstream os("intersection_sta_construct.txt");
    std::vector<Spline*>::iterator itr = i.getSplines().begin();
    for( ; itr!=i.getSplines().end(); itr++)
        (*itr)->dump(os);
    os.flush();
    os.close();
}

void IntersectionTest::testArea()
{
    Plane wlhalfcube(0,0,1,-0.5);
    Plane xhalfcube(1,0,0,-.5);
    Plane bhalfcube(0,1,0,-.25);

    float area;
    QVector3D cog;
    QVector2D moi;

    Intersection i(_model, fiWaterline, wlhalfcube, true);
    i.calculateArea(wlhalfcube, &area, &cog, &moi);
    QVERIFY(FuzzyCompare(area, 1.0, 1E-2));
    QVERIFY(FuzzyCompare(cog.x(),.5,1E-2));
    QVERIFY(FuzzyCompare(cog.y(),0,1E-2));
    QVERIFY(FuzzyCompare(cog.z(),.5,1E-2));

    Intersection i1(_model, fiStation, xhalfcube, true);
    i1.calculateArea(wlhalfcube, &area, &cog, &moi);
    QVERIFY(FuzzyCompare(area, 0.5, 1E-2));
    QVERIFY(FuzzyCompare(cog.x(),.5,1E-2));
    QVERIFY(FuzzyCompare(cog.y(),0,1E-2));
    QVERIFY(FuzzyCompare(cog.z(),.25,1E-2));

    Intersection i2(_model, fiButtock, bhalfcube, true);
    i2.calculateArea(wlhalfcube, &area, &cog, &moi);
    QVERIFY(FuzzyCompare(area, 0.5, 1E-2));
    QVERIFY(FuzzyCompare(cog.x(),.5,1E-2));
    QVERIFY(FuzzyCompare(cog.y(),.25,1E-2));
    QVERIFY(FuzzyCompare(cog.z(),.25,1E-2));
}

void IntersectionTest::testDXF()
{
    Plane wlhalfcube(0,0,1,-0.5);
    Plane xhalfcube(1,0,0,-.5);
    Plane bhalfcube(0,1,0,-.25);

    Intersection wl(_model, fiWaterline, wlhalfcube, true);
    QStringList dxf;
    wl.saveToDXF(dxf);
    ofstream os("wl_intersection.dxf");
    for (int i=0; i<dxf.size(); i++)
        os << dxf[i].toStdString() << "\r\n";
    os.close();
    QVERIFY(dxf.size() > 0);
}

QTEST_APPLESS_MAIN(IntersectionTest)

#include "tst_intersectiontest.moc"
