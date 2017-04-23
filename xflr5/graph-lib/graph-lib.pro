#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T20:08:35
#
#-------------------------------------------------

#QT       -= gui

TARGET = graph-lib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += GRAPHLIB_LIBRARY

SOURCES += \
    Curve.cpp \
    Graph.cpp \
    QGraph.cpp \
    graph_globals.cpp

HEADERS +=\
        graph-lib_global.h \
    Curve.h \
    Graph.h \
    QGraph.h \
    graph_globals.h \
    linestyle.h

macx {
    CONFIG(release, debug|release) {
        OBJECTS_DIR = ./build/release
        MOC_DIR = ./build/release
        RCC_DIR = ./build/release
        UI_HEADERS_DIR = ./build/release
    }
    CONFIG(debug, debug|release) {
        OBJECTS_DIR = ./build/debug
        MOC_DIR = ./build/debug
                RCC_DIR = ./build/debug
        UI_HEADERS_DIR = ./build/debug
    }
    CONFIG += i386
    QMAKE_MAC_SDK = macosx
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
