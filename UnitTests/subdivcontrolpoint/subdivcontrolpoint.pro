#-------------------------------------------------
#
# Project created by QtCreator 2015-07-27T17:39:37
#
#-------------------------------------------------

QT       += testlib opengl

QT       -= gui

TARGET = tst_subdivcontrolpointtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_subdivcontrolpointtest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../ShipCADlib/release/ -lShipCADlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../ShipCADlib/debug/ -lShipCADlib
else:unix: LIBS += -L$$OUT_PWD/../../ShipCADlib/ -lShipCADlib

INCLUDEPATH += $$PWD/../../ShipCADlib
DEPENDPATH += $$PWD/../../ShipCADlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../ShipCADlib/release/libShipCADlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../ShipCADlib/debug/libShipCADlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../ShipCADlib/release/ShipCADlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../../ShipCADlib/debug/ShipCADlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../../ShipCADlib/libShipCADlib.a

# BOOST
win32: {
INCLUDEPATH += $$PWD/../../../lib/boost_1_55_0
LIBS += -L$$PWD/../../../lib/boost_1_55_0/stage/lib/ -llibboost_system-vc100-mt-gd-1_55
}

unix:!symbian {
    INCLUDEPATH += /usr/local/include
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
