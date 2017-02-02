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
