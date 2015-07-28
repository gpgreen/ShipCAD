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

# BOOST

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../lib/boost_1_55_0/stage/lib/ -llibboost_system-vc100-mt-1_55
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../lib/boost_1_55_0/stage/lib/ -llibboost_system-vc100-mt-gd-1_55
else:unix: LIBS += -L/usr/local/lib -lboost_system

win32:INCLUDEPATH += $$PWD/../../lib/boost_1_55_0
else:unix!symbian: INCLUDEPATH += /usr/local/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/lib/x86_64-linux-gnu/release/libboost_system.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../../usr/lib/x86_64-linux-gnu/debug/libboost_system.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/boost_1_55_0/stage/lib/libboost_system-vc100-mt-1_55.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../lib/boost_1_55_0/stage/lib/libboost_system-vc100-mt-gd-1_55.lib
else:unix: PRE_TARGETDEPS += /usr/local/lib/libboost_system.a
