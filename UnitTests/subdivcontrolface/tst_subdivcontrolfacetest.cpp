/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#include <QString>
#include <QtTest>
#include <stdexcept>
#include <cmath>
#include <vector>
#include "subdivpoint.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "subdivsurface.h"

using namespace ShipCADGeometry;
using namespace std;

class SubdivControlfaceTest : public QObject
{
    Q_OBJECT

public:
    SubdivControlfaceTest();
    ~SubdivControlfaceTest();
private:
    SubdivisionSurface* _owner;

private Q_SLOTS:
    void testCaseConstruct();
    void testCaseAddPoints();
    void testCaseInsertPoint();
    void testCaseCalcFacePoint();
    void testCaseSubdivision4pt();
    void testCaseSubdivision3pt();
    void testCaseSubdivision5pt();
};

SubdivControlfaceTest::SubdivControlfaceTest()
{
    _owner = new SubdivisionSurface();
}

SubdivControlfaceTest::~SubdivControlfaceTest()
{
    delete _owner;
}

void SubdivControlfaceTest::testCaseConstruct()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);
    QVERIFY(face->numberOfPoints() == 0);
    QVERIFY(face->getLayer() == 0);
    QVERIFY(face->getMin()[0] == 0.0 &&
            face->getMin()[1] == 0.0 &&
            face->getMin()[2] == 0.0);
    QVERIFY(face->getMax()[0] == 0.0 &&
            face->getMax()[1] == 0.0 &&
            face->getMax()[2] == 0.0);
	QVERIFY2(!face->isSelected(), "sb not selected");
        QVERIFY2(!face->isVisible(), "sb !visible");
}

void SubdivControlfaceTest::testCaseAddPoints()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(-1,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(0,2,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(_owner);
    pt4->setCoordinate(QVector3D(0,0,1));
    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);
    QVERIFY2(face->numberOfPoints() == 3, "number of points");
    QVERIFY2(face->hasPoint(pt1) && face->hasPoint(pt2) && face->hasPoint(pt3), "has point");
    QVERIFY2(!face->hasPoint(pt4), "not have point");
    QVERIFY2(face->getPoint(0) == pt1, "get pt1");
    QVERIFY2(face->getPoint(1) == pt2, "get pt2");
    QVERIFY2(face->getPoint(2) == pt3, "get pt3");
    QVERIFY2(face->getArea() == 2.0, "area");
    QVector3D fc = face->getFaceCenter();
    QVERIFY2(fc[0] == 0 && abs(fc[1] - 0.6666667) <= 0.000001 && fc[2] == 0, "face center");
    QVector3D fn = face->getFaceNormal();
    QVERIFY2(fn[0] == 0 && fn[1] == 0 && fn[2] == -1, "face normal");
}

void SubdivControlfaceTest::testCaseInsertPoint()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(-1,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(0,2,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,0,0));

    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(_owner);
    pt4->setCoordinate(QVector3D(0,0,1));

    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);

    face->insertPoint(1, pt4);
    QVERIFY2(face->getPoint(0) == pt1, "sb pt1");
    QVERIFY2(face->getPoint(1) == pt4, "sb pt4");
    QVERIFY2(face->getPoint(2) == pt2, "sb pt2");
    QVERIFY2(face->getPoint(3) == pt3, "sb pt3");

    // verify range error
    //QVERIFY_EXCEPTION_THROWN(face->insertPoint(4, pt4), range_error);
}

void SubdivControlfaceTest::testCaseCalcFacePoint()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(-1,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(0,2,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(_owner);
    pt4->setCoordinate(QVector3D(0,0,1));
    face->addPoint(pt1);
    //QVERIFY_EXCEPTION_THROWN(face->calculateFacePoint(), range_error);
    face->addPoint(pt2);
    //QVERIFY_EXCEPTION_THROWN(face->calculateFacePoint(), range_error);
    face->addPoint(pt3);
    QVERIFY2(face->calculateFacePoint() == 0, "3 point, no face pt");
    face->addPoint(pt4);
    QVERIFY2(face->calculateFacePoint() != 0, "4 point, has face pt");
}

void SubdivControlfaceTest::testCaseSubdivision4pt()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);

    // create vertex points
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(0,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,1,0));
    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(_owner);
    pt4->setCoordinate(QVector3D(0,1,0));

    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);
    face->addPoint(pt4);

    // create edges
    SubdivisionEdge *edge1 = SubdivisionEdge::construct(_owner);
    edge1->setPoints(pt1, pt2);
    edge1->addFace(face);
    pt1->addEdge(edge1);
    pt2->addEdge(edge1);
    SubdivisionEdge *edge2 = SubdivisionEdge::construct(_owner);
    edge2->setPoints(pt2, pt3);
    edge2->addFace(face);
    pt2->addEdge(edge2);
    pt3->addEdge(edge2);
    SubdivisionEdge *edge3 = SubdivisionEdge::construct(_owner);
    edge3->setPoints(pt3, pt4);
    edge3->addFace(face);
    pt3->addEdge(edge3);
    pt4->addEdge(edge3);
    SubdivisionEdge *edge4 = SubdivisionEdge::construct(_owner);
    edge4->setPoints(pt4, pt1);
    edge4->addFace(face);
    pt4->addEdge(edge4);
    pt1->addEdge(edge4);

    vector<pair<SubdivisionPoint*,SubdivisionPoint*> > vertexpoints;
    vector<pair<SubdivisionFace*,SubdivisionPoint*> > facepoints;
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> > edgepoints;
    vector<SubdivisionEdge*> controledges;

    // create vertex points
    for (size_t i=0; i<face->numberOfPoints(); i++)
        vertexpoints.push_back(make_pair(face->getPoint(i), face->getPoint(i)->calculateVertexPoint()));
    // create facepoints
    facepoints.push_back(make_pair(face, face->calculateFacePoint()));
    // create edgepoints
    edgepoints.push_back(make_pair(edge1, edge1->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge2, edge2->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge3, edge3->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge4, edge4->calculateEdgePoint()));

    face->subdivide(vertexpoints, edgepoints, facepoints, controledges);

    QVERIFY2(face->numberOfChildren() == 4, "sb 4 faces");
    QVERIFY2(face->numberOfEdges() == 4, "sb 4 interior edges");
    QVERIFY2(face->numberOfControlEdges() == 8, "sb 8 control edges");
    QVERIFY2(controledges.size() == 8, "sb 8 control edges");
    for (size_t i=0; i<face->numberOfChildren(); i++) {
        QVector3D n = face->getChild(i)->getFaceNormal();
        QVERIFY2(n.x() == 0, "face normal x sb 0");
        QVERIFY2(n.y() == 0, "face normal y sb 0");
        QVERIFY2(n.z() == 1.0, "face normal z sb 1");
    }
}

void SubdivControlfaceTest::testCaseSubdivision3pt()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);

    // create vertex points
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(0,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,1,0));

    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);

    // create edges
    SubdivisionEdge *edge1 = SubdivisionEdge::construct(_owner);
    edge1->setPoints(pt1, pt2);
    edge1->addFace(face);
    pt1->addEdge(edge1);
    pt2->addEdge(edge1);
    SubdivisionEdge *edge2 = SubdivisionEdge::construct(_owner);
    edge2->setPoints(pt2, pt3);
    edge2->addFace(face);
    pt2->addEdge(edge2);
    pt3->addEdge(edge2);
    SubdivisionEdge *edge3 = SubdivisionEdge::construct(_owner);
    edge3->setPoints(pt3, pt1);
    edge3->addFace(face);
    pt3->addEdge(edge3);
    pt1->addEdge(edge3);

    vector<pair<SubdivisionPoint*,SubdivisionPoint*> > vertexpoints;
    vector<pair<SubdivisionFace*,SubdivisionPoint*> > facepoints;
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> > edgepoints;
    vector<SubdivisionEdge*> controledges;

    // create vertex points
    for (size_t i=0; i<face->numberOfPoints(); i++)
        vertexpoints.push_back(make_pair(face->getPoint(i), face->getPoint(i)->calculateVertexPoint()));

    // no facepoints for 3 sided faces

    // create edgepoints
    edgepoints.push_back(make_pair(edge1, edge1->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge2, edge2->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge3, edge3->calculateEdgePoint()));

    face->subdivide(vertexpoints, edgepoints, facepoints, controledges);

    QVERIFY2(face->numberOfChildren() == 4, "sb 4 faces");
    QVERIFY2(face->numberOfEdges() == 3, "sb 3 interior edges");
    QVERIFY2(face->numberOfControlEdges() == 6, "sb 6 control edges");
    QVERIFY2(controledges.size() == 6, "sb 6 control edges");
    for (size_t i=0; i<face->numberOfChildren(); i++) {
        QVector3D n = face->getChild(i)->getFaceNormal();
        QVERIFY2(n.x() == 0, "face normal x sb 0");
        QVERIFY2(n.y() == 0, "face normal y sb 0");
        QVERIFY2(n.z() == 1.0, "face normal z sb 1");
    }
}

void SubdivControlfaceTest::testCaseSubdivision5pt()
{
    SubdivisionControlFace *face = new SubdivisionControlFace(_owner);

    // create vertex points
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(_owner);
    pt1->setCoordinate(QVector3D(0,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(_owner);
    pt2->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(_owner);
    pt3->setCoordinate(QVector3D(1,1,0));
    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(_owner);
    pt4->setCoordinate(QVector3D(0.5,1.5,0));
    SubdivisionControlPoint *pt5 = SubdivisionControlPoint::construct(_owner);
    pt5->setCoordinate(QVector3D(0,1,0));

    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);
    face->addPoint(pt4);
    face->addPoint(pt5);

    // create edges
    SubdivisionEdge *edge1 = SubdivisionEdge::construct(_owner);
    edge1->setPoints(pt1, pt2);
    edge1->addFace(face);
    pt1->addEdge(edge1);
    pt2->addEdge(edge1);
    SubdivisionEdge *edge2 = SubdivisionEdge::construct(_owner);
    edge2->setPoints(pt2, pt3);
    edge2->addFace(face);
    pt2->addEdge(edge2);
    pt3->addEdge(edge2);
    SubdivisionEdge *edge3 = SubdivisionEdge::construct(_owner);
    edge3->setPoints(pt3, pt4);
    edge3->addFace(face);
    pt3->addEdge(edge3);
    pt4->addEdge(edge3);
    SubdivisionEdge *edge4 = SubdivisionEdge::construct(_owner);
    edge4->setPoints(pt4, pt5);
    edge4->addFace(face);
    pt4->addEdge(edge4);
    pt5->addEdge(edge4);
    SubdivisionEdge *edge5 = SubdivisionEdge::construct(_owner);
    edge5->setPoints(pt5, pt1);
    edge5->addFace(face);
    pt5->addEdge(edge5);
    pt1->addEdge(edge5);

    vector<pair<SubdivisionPoint*,SubdivisionPoint*> > vertexpoints;
    vector<pair<SubdivisionFace*,SubdivisionPoint*> > facepoints;
    vector<pair<SubdivisionEdge*,SubdivisionPoint*> > edgepoints;
    vector<SubdivisionEdge*> controledges;

    // create vertex points
    for (size_t i=0; i<face->numberOfPoints(); i++)
        vertexpoints.push_back(make_pair(face->getPoint(i), face->getPoint(i)->calculateVertexPoint()));
    // create facepoints
    facepoints.push_back(make_pair(face, face->calculateFacePoint()));
    // create edgepoints
    edgepoints.push_back(make_pair(edge1, edge1->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge2, edge2->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge3, edge3->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge4, edge4->calculateEdgePoint()));
    edgepoints.push_back(make_pair(edge5, edge5->calculateEdgePoint()));

    face->subdivide(vertexpoints, edgepoints, facepoints, controledges);

    QVERIFY2(face->numberOfChildren() == 5, "sb 5 faces");
    QVERIFY2(face->numberOfEdges() == 5, "sb 5 interior edges");
    QVERIFY2(face->numberOfControlEdges() == 10, "sb 10 control edge");
    QVERIFY2(controledges.size() == 10, "sb 10 control edges");
    for (size_t i=0; i<face->numberOfChildren(); i++) {
        QVector3D n = face->getChild(i)->getFaceNormal();
        QVERIFY2(n.x() == 0, "face normal x sb 0");
        QVERIFY2(n.y() == 0, "face normal y sb 0");
        QVERIFY2(n.z() == 1.0, "face normal z sb 1");
    }
}

QTEST_APPLESS_MAIN(SubdivControlfaceTest)

#include "tst_subdivcontrolfacetest.moc"
