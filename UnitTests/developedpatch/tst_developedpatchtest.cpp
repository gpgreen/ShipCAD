#include <QString>
#include <QtTest>
#include <vector>

#include "developedpatch.h"
#include "subdivlayer.h"
#include "subdivface.h"
#include "subdivpoint.h"
#include "subdivedge.h"
#include "subdivsurface.h"

using namespace ShipCAD;
using namespace std;

class DevelopedpatchTest : public QObject
{
    Q_OBJECT

public:
    DevelopedpatchTest();

private Q_SLOTS:
    void testConstruct();
    void testUnrollSimpleFace();
};

DevelopedpatchTest::DevelopedpatchTest()
{
}

void DevelopedpatchTest::testConstruct()
{
    SubdivisionSurface surface;
    SubdivisionLayer layer(&surface);
    
    DevelopedPatch dp(&layer);
    QVERIFY2(true, "Failure");
}

void DevelopedpatchTest::testUnrollSimpleFace()
{
    SubdivisionSurface surface;
    SubdivisionLayer layer(&surface);
    
    DevelopedPatch dp(&layer);

    
    SubdivisionControlFace *face = new SubdivisionControlFace(&surface);

    // create vertex points
    SubdivisionControlPoint *pt1 = SubdivisionControlPoint::construct(&surface);
    pt1->setCoordinate(QVector3D(0,0,0));
    SubdivisionControlPoint *pt2 = SubdivisionControlPoint::construct(&surface);
    pt2->setCoordinate(QVector3D(1,0,0));
    SubdivisionControlPoint *pt3 = SubdivisionControlPoint::construct(&surface);
    pt3->setCoordinate(QVector3D(1,1,0));
    SubdivisionControlPoint *pt4 = SubdivisionControlPoint::construct(&surface);
    pt4->setCoordinate(QVector3D(0,1,0));

    face->addPoint(pt1);
    face->addPoint(pt2);
    face->addPoint(pt3);
    face->addPoint(pt4);

    // create edges
    SubdivisionEdge *edge1 = SubdivisionEdge::construct(&surface);
    edge1->setPoints(pt1, pt2);
    edge1->addFace(face);
    pt1->addEdge(edge1);
    pt2->addEdge(edge1);
    SubdivisionEdge *edge2 = SubdivisionEdge::construct(&surface);
    edge2->setPoints(pt2, pt3);
    edge2->addFace(face);
    pt2->addEdge(edge2);
    pt3->addEdge(edge2);
    SubdivisionEdge *edge3 = SubdivisionEdge::construct(&surface);
    edge3->setPoints(pt3, pt4);
    edge3->addFace(face);
    pt3->addEdge(edge3);
    pt4->addEdge(edge3);
    SubdivisionEdge *edge4 = SubdivisionEdge::construct(&surface);
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

    vector<SubdivisionControlFace*> patches;
    layer.addControlFace(face);
    patches.push_back(face);

    dp.unroll(patches);

    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(DevelopedpatchTest)

#include "tst_developedpatchtest.moc"
