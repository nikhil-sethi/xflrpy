#-------------------------------------------------
#
# Project created by QtCreator 2017-01-17T05:34:23
#
#-------------------------------------------------

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT       -= gui

TARGET = xflr5-engine
TEMPLATE = lib

DEFINES += XFLR5ENGINE_LIBRARY

#VERSION = 1.00

SOURCES += \
    objects/objects_global.cpp \
    objects/objects2d/Foil.cpp \
    objects/objects2d/OpPoint.cpp \
    objects/objects2d/Polar.cpp \
    objects/objects2d/Spline.cpp \
    analysis3d/matrix.cpp \
    analysis3d/analysis3d_globals.cpp \
    analysis3d/plane_analysis/LLTAnalysis.cpp \
    analysis3d/plane_analysis/PanelAnalysis.cpp \
    analysis3d/plane_analysis/planeanalysistask.cpp \
	objects/objects3d/vector3d.cpp \
    objects/objects3d/Body.cpp \
    objects/objects3d/Frame.cpp \
    objects/objects3d/NURBSSurface.cpp \
    objects/objects3d/Panel.cpp \
    objects/objects3d/Plane.cpp \
    objects/objects3d/PlaneOpp.cpp \
    objects/objects3d/Quaternion.cpp \
    objects/objects3d/Surface.cpp \
    objects/objects3d/Wing.cpp \
    objects/objects3d/WingOpp.cpp \
    objects/objects3d/WPolar.cpp


HEADERS += \
    objects/objects_global.h \
    objects/objects2d/Foil.h \
    objects/objects2d/OpPoint.h \
    objects/objects2d/Polar.h \
    objects/objects2d/Spline.h \
    objects/objects2d/xfoil_params.h \
    analysis3d/matrix.h \
    analysis3d/analysis3d_enums.h \
    analysis3d/analysis3d_globals.h \
    analysis3d/analysis3d_params.h \
    analysis3d/plane_analysis/LLTAnalysis.h \
    analysis3d/plane_analysis/PanelAnalysis.h \
    analysis3d/plane_analysis/planeanalysistask.h \
    analysis3d/plane_analysis/planetaskevent.h \
    objects/objects3d/vector3d.h \
    objects/objects3d/Body.h \
    objects/objects3d/Frame.h \
    objects/objects3d/NURBSSurface.h \
    objects/objects3d/Panel.h \
    objects/objects3d/Plane.h \
    objects/objects3d/PlaneOpp.h \
    objects/objects3d/PointMass.h \
    objects/objects3d/Quaternion.h \
    objects/objects3d/Surface.h \
    objects/objects3d/Wing.h \
    objects/objects3d/WingOpp.h \
    objects/objects3d/WingSection.h \
	objects/objects3d/WPolar.h \
	objects/objectcolor.h \
	xflr5-engine_global.h



INCLUDEPATH += $$PWD/objects
INCLUDEPATH += $$PWD/analysis3d


OBJECTS_DIR = ./build/objects
MOC_DIR = ./build/moc
RCC_DIR = ./build/rcc


macx{
#	CONFIG += lib_bundle
#	CONFIG += i386
	QMAKE_SONAME_PREFIX = @executable_path/../Frameworks
	QMAKE_MAC_SDK = macosx
}


unix{
	target.path = /usr/local/lib
    INSTALLS += target
}
