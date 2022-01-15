# -------------------------------------------------
# Project created by QtCreator 2009-02-14T15:30:46
# -------------------------------------------------

# message(qmake version: $$[QMAKE_VERSION])
# message(Qt version: $$[QT_VERSION])

#Qt5.4 required for QOpenGLWidget instead of QGLWidget
lessThan(QT_MAJOR_VERSION, 5) {
  error("Qt5.4 or greater is required for xflr5 v6")
}
else
{
    lessThan(QT_MINOR_VERSION, 4) {
      error("Qt5.4 or greater is required for xflr5 v6")
    }
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)

VERSION = 6.49

CONFIG += qt
QT += widgets opengl network xml

TEMPLATE = app
TARGET = xflr5


# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/
# Forces re-build if a library header or source file has been modified
DEPENDPATH += $$PWD/../XFoil-lib/


OBJECTS_DIR = ./objects
MOC_DIR     = ./moc
RCC_DIR     = ./rcc
DESTDIR     = .


win32 {
#prevent qmake from making useless \debug and \release subdirs
    CONFIG -= debug_and_release debug_and_release_target

    # add console support for scripts
    CONFIG += console

    #hide the console
    LIBS += -lKernel32 -lUser32

    LIBS += -lopenGL32

    RC_FILE = ../win/xflr5.rc
}


linux-g++{

    # VARIABLES
    isEmpty(PREFIX):PREFIX = /usr/local
    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share/xflr5

    desktop.path = $$DATADIR
    desktop.files += ../linux/$${TARGET}.desktop

    icon128.path = $$DATADIR
    icon128.files += ../res/$${TARGET}.png

    translations.path = $$DATADIR/translations
    translations.files = ../translations/*.qm

    target.path = $$BINDIR

    # MAKE INSTALL
    INSTALLS += target desktop icon128 translations
}


macx{
    DESTDIR = ../
#    CONFIG += i386
    QMAKE_MAC_SDK = macosx
#    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4

    OTHER_FILES += ./mac/Info.plist
    QMAKE_INFO_PLIST = ./mac/Info.plist
    ICON = ./mac/xflr5.icns

#message($$OUT_PWD)
#    LIBS += -F$$OUT_PWD/../XFoil-lib
#    LIBS += -framework XFoil

    # link to the lib:
    LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil
    # deploy the libs:
    XFoil.files = $$OUT_PWD/../XFoil-lib/libXFoil.1.dylib
    XFoil.path = Contents/Frameworks
    QMAKE_BUNDLE_DATA += XFoil

    # make the app find the libs:
    QMAKE_RPATHDIR = @executable_path/../Frameworks

    LIBS += -framework CoreFoundation

    #other files to be bundled
    LicenseFile.files = $$PWD/../License.txt
    LicenseFile.path = Contents
    QMAKE_BUNDLE_DATA += LicenseFile
}

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4

LIBS += -L../XFoil-lib -lXFoil


include(xflr5v6.pri)
include(xfl3d/xfl3d.pri)
include(xflcore/xflcore.pri)
include(xflgeom/xflgeom.pri)
include(xflgraph/xflgraph.pri)
include(xflobjects/xflobjects.pri)
include(xflscript/xflscript.pri)
include(xflwidgets/xflwidgets.pri)
include(xflanalysis/xflanalysis.pri)





