#-------------------------------------------------
#
# Project created by QtCreator 2014-05-23T22:18:15
#
#-------------------------------------------------

QT       += opengl

TARGET = ShipCADlib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += shipcadlib.cpp \
    entity.cpp \
    filebuffer.cpp \
    plane.cpp \
    spline.cpp \
    viewport.cpp \
    openglwindow.cpp \
    utility.cpp \
    subdivbase.cpp \
    subdivsurface.cpp \
    subdivpoint.cpp \
    subdivedge.cpp

HEADERS += shipcadlib.h \
    entity.h \
    filebuffer.h \
    plane.h \
    spline.h \
    viewport.h \
    utility.h \
    exception.h \
    openglwindow.h \
    nurbsurface.h \
    subdivbase.h \
    subdivsurface.h \
    subdivedge.h \
    subdivface.h \
    subdivpoint.h \
    subdivcontrolcurve.h

INCLUDEPATH += $$PWD/../../lib/boost_1_55_0

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
