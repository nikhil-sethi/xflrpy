
# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT       += core widgets

TARGET = xflr-server
TEMPLATE = lib

DEFINES += XFLRSERVER_LIBRARY

#VERSION = 1.00

SOURCES += \
    xflrServer.cpp

HEADERS += \
    xflrServer.h \
    xflr-server_global.h 

INCLUDEPATH += .
INCLUDEPATH += $$PWD/../XFoil-lib/
INCLUDEPATH += $$PWD/../xflr5-gui/
INCLUDEPATH += $$PWD/../xflr5-engine/
INCLUDEPATH += $$PWD/../rpclib/include/

# Forces re-build if a library header or source file has been modified
DEPENDPATH += $$PWD/../XFoil-lib/
DEPENDPATH += $$PWD/../xflr5-engine/
DEPENDPATH += $$PWD/../xflr5-gui/

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

LIBS += -L../rpclib/build -lrpc

