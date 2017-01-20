#include <QString>
#include <QtTest>

#include "subdivedge.h"
#include "subdivpoint.h"
#include "subdivsurface.h"
#include "filebuffer.h"

using namespace ShipCAD;

class SubdivControlEdgeTest : public QObject
{
    Q_OBJECT

public:
    SubdivControlEdgeTest();

    SubdivisionControlEdge* getNonDefault();

private Q_SLOTS:
    void init();
    void cleanup();
    void testConstruct();
    void testWriteRead();

private:
    SubdivisionSurface* _owner;
};

SubdivControlEdgeTest::SubdivControlEdgeTest()
    : _owner(0)
{
   // does nothing
}

void SubdivControlEdgeTest::init()
{
    _owner = new SubdivisionSurface();
}

void SubdivControlEdgeTest::cleanup()
{
    delete _owner;
    _owner = 0;
}

SubdivisionControlEdge* SubdivControlEdgeTest::getNonDefault()
{
    SubdivisionControlPoint* pt1 = SubdivisionControlPoint::construct(_owner);
    _owner->addControlPoint(pt1);
    SubdivisionControlPoint* pt2 = SubdivisionControlPoint::construct(_owner);
    _owner->addControlPoint(pt2);
    SubdivisionControlEdge* edge = _owner->addControlEdge(pt1, pt2);
    // set attributes
    return edge;
}

void SubdivControlEdgeTest::testConstruct()
{
    SubdivisionControlEdge* edge = getNonDefault();
    QVERIFY2(_owner->hasControlEdge(edge), "has edge");
}

void SubdivControlEdgeTest::testWriteRead()
{
    SubdivisionControlEdge* edgeW = getNonDefault();
    FileBuffer dest;
    edgeW->saveBinary(dest);
    size_t orig = dest.size();
    SubdivisionControlEdge* edgeR = new SubdivisionControlEdge(_owner);
    dest.reset();
    edgeR->loadBinary(dest);
    QVERIFY(dest.pos() == orig);
    QVERIFY(edgeW->startPoint() == edgeR->startPoint());
    QVERIFY(edgeW->endPoint() == edgeR->endPoint());
    QVERIFY(edgeW->isBoundaryEdge() == edgeR->isBoundaryEdge());
    QVERIFY(edgeW->isControlEdge() == edgeR->isControlEdge());
    QVERIFY(edgeW->numberOfFaces() == edgeR->numberOfFaces());
    QVERIFY(edgeW->isCrease() == edgeR->isCrease());
    QVERIFY(edgeW->getCurve() == edgeR->getCurve());
    QVERIFY(edgeW->getColor() == edgeR->getColor());
    QVERIFY(edgeW->isSelected() == edgeR->isSelected());
    QVERIFY(edgeW->isVisible() == edgeR->isVisible());
    delete edgeR;
}

QTEST_APPLESS_MAIN(SubdivControlEdgeTest)

#include "tst_subdivcontroledgetest.moc"
