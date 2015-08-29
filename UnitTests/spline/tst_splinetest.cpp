#include <QString>
#include <QtTest>

#include "spline.h"
#include "plane.h"

using namespace ShipCAD;

class SplineTest : public QObject
{
    Q_OBJECT

public:
    SplineTest();

private Q_SLOTS:
    void testConstruct();
	void testInsert();
    void testInvert();
	void testDelete();
	void testInsertSpline();
    void testSimplify();
    void testIntersection();
};

SplineTest::SplineTest()
{
}

void SplineTest::testConstruct()
{
    Spline s;
	QVector3D p1(0,0,0);
	QVector3D p2(1,1,1);
	QVector3D p3(2,2,2);
	
    s.add(p1);
    s.add(p2);
    s.add(p3);

	QVERIFY(!s.isBuild());
	QVERIFY(s.getPoint(0) == p1);
	QVERIFY(s.getPoint(1) == p2);
	QVERIFY(s.getPoint(2) == p3);
	QVERIFY(!s.isKnuckle(0));
	QVERIFY(!s.isKnuckle(1));
	QVERIFY(!s.isKnuckle(2));
    QVERIFY2(s.numberOfPoints() == 3, "Failure");
}

void SplineTest::testInsert()
{
    Spline s;
	QVector3D p1(0,0,0);
	QVector3D p2(1,1,1);
	QVector3D p3(2,2,2);
	QVector3D p4(3,3,3);
	
    s.add(p1);
	s.setKnuckle(0, true);
    s.add(p2);
    s.add(p4);

	s.insert(2, p3);
	
	QVERIFY(s.getPoint(0) == p1);
	QVERIFY(s.getPoint(1) == p2);
	QVERIFY(s.getPoint(2) == p3);
	QVERIFY(s.getPoint(3) == p4);
	QVERIFY(s.isKnuckle(0));
	QVERIFY(!s.isKnuckle(1));
	QVERIFY(!s.isKnuckle(2));
	QVERIFY(!s.isKnuckle(3));
	QVERIFY(s.numberOfPoints() == 4);
}

void SplineTest::testInvert()
{
    Spline s;
	QVector3D p1(0,0,0);
	QVector3D p2(1,1,1);
	QVector3D p3(2,2,2);
	
    s.add(p1);
	s.setKnuckle(0, true);
    s.add(p2);
    s.add(p3);

	s.invert_direction();
	
	QVERIFY(s.getPoint(0) == p3);
	QVERIFY(s.getPoint(1) == p2);
	QVERIFY(s.getPoint(2) == p1);
	QVERIFY(!s.isKnuckle(0));
	QVERIFY(!s.isKnuckle(1));
	QVERIFY(s.isKnuckle(2));
}

void SplineTest::testDelete()
{
    Spline s;
	QVector3D p1(0,0,0);
	QVector3D p2(1,1,1);
	QVector3D p3(2,2,2);
	QVector3D p4(3,3,3);
	
    s.add(p1);
	s.setKnuckle(0, true);
    s.add(p2);
    s.add(p3);
	s.add(p4);
	s.setKnuckle(3, true);
	
	s.delete_point(2);

	QVERIFY(s.numberOfPoints() == 3);
	QVERIFY(s.getPoint(0) == p1);
	QVERIFY(s.getPoint(1) == p2);
	QVERIFY(s.getPoint(2) == p4);
	QVERIFY(s.isKnuckle(0));
	QVERIFY(!s.isKnuckle(1));
	QVERIFY(s.isKnuckle(2));

	s.delete_point(0);
	
	QVERIFY(s.numberOfPoints() == 2);
	QVERIFY(s.getPoint(0) == p2);
    QVERIFY(s.getPoint(1) == p4);
	QVERIFY(!s.isKnuckle(0));
	QVERIFY(s.isKnuckle(1));

	s.delete_point(1);
	QVERIFY(s.numberOfPoints() == 1);
	QVERIFY(s.getPoint(0) == p2);
	QVERIFY(!s.isKnuckle(0));

	s.delete_point(0);
	QVERIFY(s.numberOfPoints() == 0);
}

void SplineTest::testInsertSpline()
{
    Spline s1;
	QVector3D p1(0,0,0);
	QVector3D p2(1,1,1);
	QVector3D p3(2,2,2);
	QVector3D p4(3,3,3);

    s1.add(p1);
	s1.setKnuckle(0, true);
    s1.add(p2);
    s1.add(p3);
	s1.add(p4);
	s1.setKnuckle(3, true);

	// copy the spline
	Spline s1a(s1);
	Spline s1b(s1);
	Spline s1c(s1);
	
	Spline s2;
	QVector3D p5(4,4,4);
	QVector3D p6(5,5,5);
	QVector3D p7(6,6,6);

	s2.add(p5);
	s2.add(p6);
	s2.add(p7);

	// don't invert, not a duplicate start
	s1.insert_spline(2, false, false, s2);

    QVERIFY(s1.numberOfPoints() == 7);
	QVERIFY(s1.getPoint(0) == p1);
	QVERIFY(s1.getPoint(1) == p2);
	QVERIFY(s1.getPoint(2) == p5);
	QVERIFY(s1.getPoint(3) == p6);
	QVERIFY(s1.getPoint(4) == p7);
	QVERIFY(s1.getPoint(5) == p3);
	QVERIFY(s1.getPoint(6) == p4);
	QVERIFY(s1.isKnuckle(0));
	QVERIFY(!s1.isKnuckle(1));
	QVERIFY(!s1.isKnuckle(2));
	QVERIFY(!s1.isKnuckle(3));
	QVERIFY(!s1.isKnuckle(4));
	QVERIFY(!s1.isKnuckle(5));
	QVERIFY(s1.isKnuckle(6));

	// invert, not a duplicate start
	s1a.insert_spline(2, true, false, s2);

    QVERIFY(s1a.numberOfPoints() == 7);
	QVERIFY(s1a.getPoint(0) == p1);
	QVERIFY(s1a.getPoint(1) == p2);
	QVERIFY(s1a.getPoint(2) == p7);
	QVERIFY(s1a.getPoint(3) == p6);
	QVERIFY(s1a.getPoint(4) == p5);
	QVERIFY(s1a.getPoint(5) == p3);
	QVERIFY(s1a.getPoint(6) == p4);
	QVERIFY(s1a.isKnuckle(0));
	QVERIFY(!s1a.isKnuckle(1));
	QVERIFY(!s1a.isKnuckle(2));
	QVERIFY(!s1a.isKnuckle(3));
	QVERIFY(!s1a.isKnuckle(4));
	QVERIFY(!s1a.isKnuckle(5));
	QVERIFY(s1a.isKnuckle(6));

	Spline s3;
	s3.add(p5);
	s3.add(p6);
	s3.add(p7);
	s3.setKnuckle(2, true);
	s3.add(p3);
	
	// don't invert, a duplicate point
	s1b.insert_spline(4, false, true, s3);

    QVERIFY(s1b.numberOfPoints() == 7);
	QVERIFY(s1b.getPoint(0) == p1);
	QVERIFY(s1b.getPoint(1) == p2);
	QVERIFY(s1b.getPoint(2) == p3);
	QVERIFY(s1b.getPoint(3) == p4);
	QVERIFY(s1b.getPoint(4) == p5);
	QVERIFY(s1b.getPoint(5) == p6);
	QVERIFY(s1b.getPoint(6) == p7);
	QVERIFY(s1b.isKnuckle(0));
	QVERIFY(!s1b.isKnuckle(1));
	QVERIFY(!s1b.isKnuckle(2));
	QVERIFY(s1b.isKnuckle(3));
	QVERIFY(!s1b.isKnuckle(4));
	QVERIFY(!s1b.isKnuckle(5));
	QVERIFY(s1b.isKnuckle(6));

	// invert, a duplicate point
    s1c.insert_spline(4, true, true, s3);

    QVERIFY(s1c.numberOfPoints() == 7);
    QVERIFY(s1c.getPoint(0) == p1);
    QVERIFY(s1c.getPoint(1) == p2);
    QVERIFY(s1c.getPoint(2) == p3);
    QVERIFY(s1c.getPoint(3) == p4);
    QVERIFY(s1c.getPoint(4) == p7);
    QVERIFY(s1c.getPoint(5) == p6);
    QVERIFY(s1c.getPoint(6) == p5);
    QVERIFY(s1c.isKnuckle(0));
    QVERIFY(!s1c.isKnuckle(1));
    QVERIFY(!s1c.isKnuckle(2));
    QVERIFY(s1c.isKnuckle(3));
    QVERIFY(s1c.isKnuckle(4));
    QVERIFY(!s1c.isKnuckle(5));
    QVERIFY(!s1c.isKnuckle(6));
}

void SplineTest::testSimplify()
{
    Spline s;
    QVector3D p1(0,0,0);
    QVector3D p2(1,1,1);
    QVector3D p3(2,2,2);
    QVector3D p4(3,3,3);

    s.add(p1);
    s.add(p2);
    s.add(p3);
    s.add(p4);

    s.simplify(2);
    QVERIFY(s.numberOfPoints() == 4);
    QVERIFY(s.getPoint(0) == p1);
    QVERIFY(s.getPoint(1) == p2);
    QVERIFY(s.getPoint(2) == p3);
    QVERIFY(s.getPoint(3) == p4);
    QVERIFY(!s.isKnuckle(0));
    QVERIFY(!s.isKnuckle(1));
    QVERIFY(!s.isKnuckle(2));
    QVERIFY(!s.isKnuckle(3));
	
    // add a point and simplify again
    QVector3D p5(1.99f, 1.99f, 1.99f);
    s.insert(2, p5);
    s.simplify(2);
    QVERIFY(s.numberOfPoints() == 4);
    QVERIFY(s.getPoint(0) == p1);
    QVERIFY(s.getPoint(1) == p2);
    QVERIFY(s.getPoint(2) == p3);
    QVERIFY(s.getPoint(3) == p4);
    QVERIFY(!s.isKnuckle(0));
    QVERIFY(!s.isKnuckle(1));
    QVERIFY(!s.isKnuckle(2));
    QVERIFY(!s.isKnuckle(3));
}

void SplineTest::testIntersection()
{
    // build a u-shaped spline, square corners
    Spline s;
    QVector3D p1(0,1,0);
    QVector3D p2(0,0,0);
    QVector3D p3(1,0,0);
    QVector3D p4(1,1,0);

    s.add(p1);
    s.setKnuckle(0, true);
    s.add(p2);
    s.setKnuckle(1, true);
    s.add(p3);
    s.setKnuckle(2, true);
    s.add(p4);
    s.setKnuckle(3, true);

    // intersect the middle of the u with a plane
    Plane middle(QVector3D(0, .5, 0), QVector3D(1, .5, 0), QVector3D(1, .5, 1));

    QVector3D i1(0,0.5,0);
    QVector3D i2(1,0.5,0);

    IntersectionData output;
    bool result = s.intersect_plane(middle, output);
    QVERIFY(result);
    QVERIFY(output.points.size() == 2);
    QVERIFY(output.parameters.size() == 2);
    QVERIFY(qFuzzyCompare(output.points[0], i1));
    QVERIFY(qFuzzyCompare(output.points[1], i2));
    QVERIFY(qFuzzyCompare(s.value(output.parameters[0]), i1));
    QVERIFY(qFuzzyCompare(s.value(output.parameters[1]), i2));

    // intersect the bottom of the u with a plane
    Plane bottom(QVector3D(0, 0, 0), QVector3D(1, 0, 0), QVector3D(1, 0, 1));
    IntersectionData o1;
    result = s.intersect_plane(bottom, o1);
    QVERIFY(result);
    QVERIFY(o1.points.size() > 0);
    QVERIFY(o1.parameters.size() == o1.points.size());
    // will have a bunch of points here
    for (size_t i=0; i<o1.points.size(); i++) {
        QVERIFY(o1.points[i].y() == 0);
        QVERIFY(o1.points[i].z() == 0);
        QVector3D p = s.value(o1.parameters[i]);
        QVERIFY(p.y() == 0);
        QVERIFY(p.z() == 0);
    }

    // intersect the top of the u with a plane
    Plane top(QVector3D(0, 1, 0), QVector3D(1, 1, 0), QVector3D(1, 1, 1));
    QVector3D i3(0,1,0);
    QVector3D i4(1,1,0);
    IntersectionData o2;
    result = s.intersect_plane(top, o2);
    QVERIFY(result);
    QVERIFY(o2.points.size() == 2);
    QVERIFY(o2.parameters.size() == 2);
    QVERIFY(qFuzzyCompare(o2.points[0], i3));
    QVERIFY(qFuzzyCompare(o2.points[1], i4));
    QVERIFY(qFuzzyCompare(s.value(o2.parameters[0]), i3));
    QVERIFY(qFuzzyCompare(s.value(o2.parameters[1]), i4));

    // intersect the bottom of the u with a plane that cuts, not parallel
    Plane bottomcut(QVector3D(0.5, 0, 0), QVector3D(0.5, 0, 1), QVector3D(0.5, 1, 0));
    IntersectionData o3;
    QVector3D i5(0.5,0,0);
    result = s.intersect_plane(bottomcut, o3);
    QVERIFY(result);
    QVERIFY(o3.points.size() == 1);
    QVERIFY(o3.parameters.size() == 1);
    QVERIFY(qFuzzyCompare(o3.points[0], i5));
    QVERIFY(qFuzzyCompare(s.value(o3.parameters[0]), i5));
}

QTEST_APPLESS_MAIN(SplineTest)

#include "tst_splinetest.moc"
