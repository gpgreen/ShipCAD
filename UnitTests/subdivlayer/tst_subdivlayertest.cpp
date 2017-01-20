#include <QString>
#include <QtTest>

#include "subdivlayer.h"
#include "subdivsurface.h"
#include "filebuffer.h"

using namespace ShipCAD;

class SubdivisionLayerTest : public QObject
{
    Q_OBJECT

public:
    SubdivisionLayerTest();

    SubdivisionLayer* getNonDefault();

private Q_SLOTS:
    void init();
    void cleanup();
    void testConstruct();
    void testWriteRead();

private:
    SubdivisionSurface* _owner;
};

SubdivisionLayerTest::SubdivisionLayerTest()
    : _owner(0)
{
   // does nothing
}

void SubdivisionLayerTest::init()
{
    _owner = new SubdivisionSurface();
}

void SubdivisionLayerTest::cleanup()
{
    delete _owner;
    _owner = 0;
}

SubdivisionLayer* SubdivisionLayerTest::getNonDefault()
{
    SubdivisionLayer* layer = _owner->addNewLayer();
    // set attributes
    layer->setDescription("a layer");
    layer->setColor(Qt::black);
    layer->setVisible(false);
    layer->setSymmetric(false);
    layer->setDevelopable(true);
    layer->setUseForIntersections(false);
    layer->setUseInHydrostatics(false);
    layer->setMaterialDensity(1.0f);
    layer->setThickness(0.1f);
    layer->setShowInLinesplan(false);
    layer->setAlphaBlend(34);
    return layer;
}

void SubdivisionLayerTest::testConstruct()
{
    SubdivisionLayer* layer = getNonDefault();
    QVERIFY2(_owner->numberOfLayers() == 2, "number of layers");
    QVERIFY2(_owner->getLayer(1) == layer, "identity of layer");
    QVERIFY2(_owner->getActiveLayer() == layer, "active layer");
    QVERIFY2(_owner->hasLayer(layer), "has the layer");
}

void SubdivisionLayerTest::testWriteRead()
{
    SubdivisionLayer* layerW = getNonDefault();
    FileBuffer dest;
    layerW->saveBinary(dest);
    size_t orig = dest.size();
    SubdivisionLayer* layerR = new SubdivisionLayer(_owner);
    dest.reset();
    layerR->loadBinary(dest);
    QVERIFY(dest.pos() == orig);
    QVERIFY(layerW->getDescription() == layerR->getDescription());
    QVERIFY(layerW->getLayerID() == layerR->getLayerID());
    QVERIFY(layerW->getColor() == layerR->getColor());
    QVERIFY(layerW->isVisible() == layerR->isVisible());
    QVERIFY(layerW->isSymmetric() == layerR->isSymmetric());
    QVERIFY(layerW->isDevelopable() == layerR->isDevelopable());
    QVERIFY(layerW->useForIntersections() == layerR->useForIntersections());
    QVERIFY(layerW->useInHydrostatics() == layerR->useInHydrostatics());
    QVERIFY(layerW->getMaterialDensity() == layerR->getMaterialDensity());
    QVERIFY(layerW->getThickness() == layerR->getThickness());
    QVERIFY(layerW->showInLinesplan() == layerR->showInLinesplan());
    QVERIFY(layerW->getAlphaBlend() == layerR->getAlphaBlend());
    delete layerR;
}

QTEST_APPLESS_MAIN(SubdivisionLayerTest)

#include "tst_subdivlayertest.moc"
