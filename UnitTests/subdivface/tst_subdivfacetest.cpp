#include <QString>
#include <QtTest>

class SubdivfaceTest : public QObject
{
    Q_OBJECT

public:
    SubdivfaceTest();

private Q_SLOTS:
    void testCaseConstruct();
};

SubdivfaceTest::SubdivfaceTest()
{
}

void SubdivfaceTest::testCaseConstruct()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(SubdivfaceTest)

#include "tst_subdivfacetest.moc"
