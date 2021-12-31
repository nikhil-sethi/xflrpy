#-------------------------------------------------
#
# Project created by QtCreator 2017-01-17T05:34:23
#
#-------------------------------------------------

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
# DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
QMAKE_CXXFLAGS += -Wno-deprecated-copy
QT       -= gui

TARGET = xflr5-engine
TEMPLATE = lib

DEFINES += XFLR5ENGINE_LIBRARY

#VERSION = 1.00

SOURCES += \
    analysis3d/analysis3d_globals.cpp \
    analysis3d/matrix.cpp \
    analysis3d/plane_analysis/lltanalysis.cpp \
    analysis3d/plane_analysis/panelanalysis.cpp \
    analysis3d/plane_analysis/planeanalysistask.cpp \
    objects/objects2d/blxfoil.cpp \
    objects/objects2d/foil.cpp \
    objects/objects2d/opppoint.cpp \
    objects/objects2d/polar.cpp \
    objects/objects2d/spline.cpp \
    objects/objects3d/body.cpp \
    objects/objects3d/frame.cpp \
    objects/objects3d/nurbssurface.cpp \
    objects/objects3d/panel.cpp \
    objects/objects3d/plane.cpp \
    objects/objects3d/planeopp.cpp \
    objects/objects3d/quaternion.cpp \
    objects/objects3d/surface.cpp \
    objects/objects3d/vector3d.cpp \
    objects/objects3d/wing.cpp \
    objects/objects3d/wingopp.cpp \
    objects/objects3d/wpolar.cpp \
    objects/objects_global.cpp


HEADERS += \
    analysis3d/analysis3d_enums.h \
    analysis3d/analysis3d_globals.h \
    analysis3d/analysis3d_params.h \
    analysis3d/matrix.h \
    analysis3d/plane_analysis/lltanalysis.h \
    analysis3d/plane_analysis/panelanalysis.h \
    analysis3d/plane_analysis/planeanalysistask.h \
    analysis3d/plane_analysis/planetaskevent.h \
    objects/objectcolor.h \
    objects/objects2d/blxfoil.h \
    objects/objects2d/foil.h \
    objects/objects2d/oppoint.h \
    objects/objects2d/polar.h \
    objects/objects2d/spline.h \
    objects/objects3d/body.h \
    objects/objects3d/frame.h \
    objects/objects3d/nurbssurface.h \
    objects/objects3d/panel.h \
    objects/objects3d/plane.h \
    objects/objects3d/planeopp.h \
    objects/objects3d/pointmass.h \
    objects/objects3d/quaternion.h \
    objects/objects3d/surface.h \
    objects/objects3d/vector3d.h \
    objects/objects3d/wing.h \
    objects/objects3d/wingopp.h \
    objects/objects3d/wingsection.h \
    objects/objects3d/wpolar.h \
    objects/objects_global.h \
    xflr5-engine_global.h \


INCLUDEPATH += $$PWD/objects
INCLUDEPATH += $$PWD/analysis3d

INCLUDEPATH += ../XFoil-lib
DEPENDPATH  += ../XFoil-lib

OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc


win32 {
#prevent qmake from making useless \debug and \release subdirs
    CONFIG -= debug_and_release debug_and_release_target

    LIBS += -L../XFoil-lib -lXFoil
}



macx{
#    CONFIG += lib_bundle
#    CONFIG += i386
    QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
    QMAKE_MAC_SDK = macosx
}


unix{
    isEmpty(PREFIX){
        PREFIX = /usr/local
    }
    target.path = $$PREFIX/lib
    INSTALLS += target
}
