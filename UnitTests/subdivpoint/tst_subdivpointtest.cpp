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
#include "subdivpoint.h"
#include "subdivface.h"
#include "subdivedge.h"
#include "subdivsurface.h"

using namespace ShipCADGeometry;

class TestSubdivisionPoint : public QObject
{
    Q_OBJECT

public:
    TestSubdivisionPoint();
    ~TestSubdivisionPoint();
private:
    SubdivisionSurface* _owner;
						
private Q_SLOTS:
    void testCaseConstruct();
    void testCaseSetVertex();
    void testCaseSetCoord();
    void testCaseAddFace();
    void testCaseAddEdge();
};

TestSubdivisionPoint::TestSubdivisionPoint()
{
    _owner = new SubdivisionSurface();
}

TestSubdivisionPoint::~TestSubdivisionPoint()
{
	delete _owner;
}

void TestSubdivisionPoint::testCaseConstruct()
{
    SubdivisionPoint *pt = SubdivisionPoint::construct(_owner);
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
}

void TestSubdivisionPoint::testCaseSetVertex()
{
    SubdivisionPoint *pt = SubdivisionPoint::construct(_owner);
    pt->setVertexType(SubdivisionPoint::svCorner);
    QVERIFY(pt->getVertexType() == SubdivisionPoint::svCorner);
}

void TestSubdivisionPoint::testCaseSetCoord()
{
    SubdivisionPoint *pt = SubdivisionPoint::construct(_owner);
    pt->setCoordinate(QVector3D(1,2,3));
    QVERIFY(pt->getCoordinate()[0] == 1 &&
		pt->getCoordinate()[1] == 2 &&
		pt->getCoordinate()[2] == 3);
}

void TestSubdivisionPoint::testCaseAddFace()
{
    SubdivisionPoint *pt = SubdivisionPoint::construct(_owner);
	SubdivisionFace *f = SubdivisionFace::construct(_owner);
	pt->addFace(f);
	QVERIFY(pt->getFace(0) == f);
	QVERIFY(pt->numberOfFaces() == 1);
	QVERIFY(pt->indexOfFace(f) == 0);
	QVERIFY(pt->hasFace(f));
	pt->deleteFace(f);
	QVERIFY(pt->numberOfFaces() == 0);
	QVERIFY(!pt->hasFace(f));
}

void TestSubdivisionPoint::testCaseAddEdge()
{
    SubdivisionPoint *pt = SubdivisionPoint::construct(_owner);
	SubdivisionEdge *e = SubdivisionEdge::construct(_owner);
	pt->addEdge(e);
	QVERIFY(pt->getEdge(0) == e);
	QVERIFY(pt->numberOfEdges() == 1);
	QVERIFY(pt->hasEdge(e));
	pt->deleteEdge(e);
	QVERIFY(pt->numberOfEdges() == 0);
	QVERIFY(!pt->hasEdge(e));
}
	

QTEST_APPLESS_MAIN(TestSubdivisionPoint)

#include "tst_subdivpointtest.moc"
