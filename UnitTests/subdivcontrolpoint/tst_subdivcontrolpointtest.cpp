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
#include "filebuffer.h"

using namespace ShipCAD;

class SubdivcontrolpointTest : public QObject
{
    Q_OBJECT

public:
    SubdivcontrolpointTest();
    ~SubdivcontrolpointTest();
private:
    SubdivisionSurface* _owner;
private Q_SLOTS:
    void testCaseConstruct();
    void testCaseSetVertexLockedSelected();
    void testWriteRead();
};

SubdivcontrolpointTest::SubdivcontrolpointTest()
{
    _owner = new SubdivisionSurface();
}

SubdivcontrolpointTest::~SubdivcontrolpointTest()
{
    delete _owner;
}

void SubdivcontrolpointTest::testCaseConstruct()
{
    SubdivisionControlPoint *pt = SubdivisionControlPoint::construct(_owner);
    QVERIFY(pt->getCoordinate()[0] == 0 &&
            pt->getCoordinate()[1] == 0 &&
            pt->getCoordinate()[2] == 0);
    float c = pt->getCurvature();
    QVERIFY(c == 0.0 || c == 360.0);
    QVERIFY(!pt->isBoundaryVertex());
    QVERIFY(pt->getNormal()[0] == 0 &&
            pt->getNormal()[1] == 0 &&
            pt->getNormal()[2] == 0);
    QVERIFY(pt->getVertexType() == svRegular);
    QVERIFY(!pt->isLocked());
    QVERIFY(pt->isVisible());
    QVERIFY(!pt->isLeak());
    QVERIFY(!pt->isSelected());
}

void SubdivcontrolpointTest::testCaseSetVertexLockedSelected()
{
    SubdivisionControlPoint *pt = SubdivisionControlPoint::construct(_owner);
    pt->setVertexType(svCorner);
    QVERIFY(pt->getVertexType() == svCorner);
    pt->setLocked(true);
    pt->setSelected(true);
    QVERIFY(pt->isLocked());
    QVERIFY(pt->isSelected());
}

void SubdivcontrolpointTest::testWriteRead()
{
    SubdivisionControlPoint *ptO = SubdivisionControlPoint::construct(_owner);
	SubdivisionControlPoint *ptI = SubdivisionControlPoint::construct(_owner);
    FileBuffer dest;
    ptO->save_binary(dest);
    size_t orig = dest.size();
    dest.reset();
    ptI->load_binary(dest);
    QVERIFY(dest.pos() == orig);
    QVERIFY(ptI->getCoordinate() == ptO->getCoordinate());
    QVERIFY(ptI->getVertexType() == ptO->getVertexType());
    QVERIFY(ptI->isSelected() == ptO->isSelected());
    QVERIFY(ptI->isLocked() == ptO->isLocked());
}


QTEST_APPLESS_MAIN(SubdivcontrolpointTest)

#include "tst_subdivcontrolpointtest.moc"
