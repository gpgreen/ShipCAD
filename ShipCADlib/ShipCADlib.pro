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
    geometry.cpp \
    openglwindow.cpp

HEADERS += shipcadlib.h \
    utility.h \
    geometry.h \
    exception.h \
    openglwindow.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
