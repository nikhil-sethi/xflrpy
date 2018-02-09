#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T20:08:35
#
#-------------------------------------------------

#QT       -= gui

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


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
