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


VERSION = 6.42

CONFIG += qt
QT += opengl

TEMPLATE = app
TARGET = xflr5


INCLUDEPATH += $$PWD/viewwidgets
INCLUDEPATH += $$PWD/viewwidgets/glWidgets
INCLUDEPATH += $$PWD/graph
INCLUDEPATH += $$PWD/misc
INCLUDEPATH += $$PWD/xdirect/xfoil_task
INCLUDEPATH += $$PWD/glcontextinfo

# The path to the libraries' header files required by the code at compile time
INCLUDEPATH += $$PWD/../XFoil-lib/
INCLUDEPATH += $$PWD/../xflr5-engine/
# Forces re-build if a library header or source file has been modified
DEPENDPATH += $$PWD/../XFoil-lib/
DEPENDPATH += $$PWD/../xflr5-engine/


OBJECTS_DIR = ./build/objects
MOC_DIR = ./build/moc
RCC_DIR = ./build/rcc


win32 {
#   Specify here the directories where the shared library files XFoil.dll and Xfl5-engine.dll are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries separately and declare them in the system's PATH
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)

	CONFIG(release, debug|release){
        LIBS += -L$$OUT_PWD/../xflr5-engine/release/ -lxflr5-engine
        LIBS += -L$$OUT_PWD/../XFoil-lib/release/ -lXFoil
	}
	else:CONFIG(debug, debug|release)
	{
        LIBS += -L$$OUT_PWD/../xflr5-engine/debug/ -lxflr5-engine
        LIBS += -L$$OUT_PWD/../XFoil-lib/debug/ -lXFoil
	}
	LIBS += -lopenGL32

	RC_FILE = ../win/xflr5.rc
}


unix{
#   Specify here the directories where the shared library files XFoil.so and Xfl5-engine.so are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries and  sudo make install
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console
#message($$OUT_PWD)

	LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil
	LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine

	# VARIABLES
	isEmpty(PREFIX):PREFIX = /usr/local
	BINDIR = $$PREFIX/bin
	DATADIR = $$PREFIX/share

	# MAKE INSTALL
	INSTALLS += target
	target.path = $$BINDIR
}


macx{
    DESTDIR = ../
#	CONFIG += i386
	QMAKE_MAC_SDK = macosx
#	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4

	OTHER_FILES += ./mac/Info.plist
	QMAKE_INFO_PLIST = ./mac/Info.plist
	ICON = ./mac/xflr5.icns

#   Specify here the directories where the shared library files XFoil.dylib and Xfl5-engine.dylib are located
#   The precise paths depend on QtCreator's settings
#   Alternate option is to compile the libraries separately and declare them in the system's PATH
#   Alternate option is to specify the absolute path instead of the relative path
#   Uncomment the following line to print the name of the variable OUT_PWD in the console

#	CONFIG(release, debug|release){
#		LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine
#		LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil
#	}
#	else:CONFIG(debug, debug|release)
#	{
#		LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine
#		LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil
#	}
#message($$OUT_PWD)
#	LIBS += -F$$OUT_PWD/../xflr5-engine
#	LIBS += -framework xflr5-engine
#	LIBS += -F$$OUT_PWD/../XFoil-lib
#	LIBS += -framework XFoil

	# link to the lib:
	LIBS += -L$$OUT_PWD/../xflr5-engine -lxflr5-engine
	# make the app find the libs:
	QMAKE_RPATHDIR = @executable_path/../Frameworks
	# deploy the libs:
	xflr5-engine.files = $$OUT_PWD/../xflr5-engine/libxflr5-engine.1.dylib
	xflr5-engine.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += xflr5-engine

	# link to the lib:
	LIBS += -L$$OUT_PWD/../XFoil-lib -lXFoil
	# make the app find the libs:
	QMAKE_RPATHDIR = @executable_path/../Frameworks
	# deploy the libs:
	XFoil.files = $$OUT_PWD/../XFoil-lib/libXFoil.1.dylib
	XFoil.path = Contents/Frameworks
	QMAKE_BUNDLE_DATA += XFoil

	LIBS += -framework CoreFoundation
}

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4


include(xflr5v6.pri)


