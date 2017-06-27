#-------------------------------------------------
#
# Project created by QtCreator 2017-01-18T04:06:28
#
#-------------------------------------------------

QT       -= gui

TARGET = XFoil-lib
TEMPLATE = lib
CONFIG += staticlib

DEFINES += XFOILLIB_LIBRARY

SOURCES += \
    XFoil.cpp

HEADERS +=\
    xfoil-lib_global.h \
    XFoil.h \
    xfoil_params.h

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
    CONFIG += staticlib
    CONFIG += i386
    QMAKE_MAC_SDK = macosx
}

unix {
    target.path = /usr/lib
    INSTALLS += target
}
