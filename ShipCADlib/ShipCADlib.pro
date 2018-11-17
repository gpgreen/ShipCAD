#-------------------------------------------------
#
# Project created by QtCreator 2014-05-23T22:18:15
#
#-------------------------------------------------

include (../common.pri)

QT       += opengl gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ShipCADlib
TEMPLATE = lib
CONFIG += staticlib

TRANSLATIONS = shipcadlib_es.ts

SOURCES += shipcadlib.cpp \
    shipcadmodel.cpp \
    entity.cpp \
    filebuffer.cpp \
    plane.cpp \
    spline.cpp \
    viewport.cpp \
    viewportview.cpp \
    openglwindow.cpp \
    utility.cpp \
    subdivbase.cpp \
    subdivsurface.cpp \
    subdivpoint.cpp \
    subdivedge.cpp \
    subdivface.cpp \
    subdivcontrolcurve.cpp \
    nurbsurface.cpp \
    subdivlayer.cpp \
    version.cpp \
    shader.cpp \
    projsettings.cpp \
    undoobject.cpp \
    visibility.cpp \
    intersection.cpp \
    marker.cpp \
    hydrostaticcalc.cpp \
    preferences.cpp \
    controller.cpp \
    flowline.cpp \
    backgroundimage.cpp \
    developedpatch.cpp \
    dialogdata.cpp \
    drawfaces.cpp

HEADERS += shipcadlib.h \
    dialogdata.h \
    entity.h \
    filebuffer.h \
    plane.h \
    spline.h \
    viewport.h \
    viewportview.h \
    utility.h \
    exception.h \
    openglwindow.h \
    nurbsurface.h \
    subdivbase.h \
    subdivsurface.h \
    subdivedge.h \
    subdivface.h \
    subdivpoint.h \
    subdivcontrolcurve.h \
    subdivlayer.h \
    version.h \
    shader.h \
    projsettings.h \
    hydrostaticcalc.h \
    shipcadmodel.h \
    intersection.h \
    marker.h \
    flowline.h \
    visibility.h \
    preferences.h \
    undoobject.h \
    resistance.h \
    backgroundimage.h \
    pointervec.h \
    controller.h \
    grid.h \
    developedpatch.h \
    mempool.h \
    orderedmap.h \
    predicate.h \
    tempvar.h \
    drawfaces.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
