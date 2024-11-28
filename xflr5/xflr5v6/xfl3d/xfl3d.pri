

HEADERS += \
    $$PWD/testgl/boid.h \
    $$PWD/testgl/gl3dboids2.h \
    xfl3d/controls/arcball.h \
    xfl3d/controls/gllightdlg.h \
    xfl3d/controls/light.h \
    xfl3d/glinfo/opengldlg.h \
    xfl3d/globals/gl_globals.h \
    xfl3d/globals/w3dprefs.h \
    xfl3d/testgl/gl2dfractal.h \
    xfl3d/testgl/gl2dnewton.h \
    xfl3d/testgl/gl2dview.h \
    xfl3d/testgl/gl3dattractors.h \
    xfl3d/testgl/gl3dboids.h \
    xfl3d/testgl/gl3dhydrogen.h \
    xfl3d/testgl/gl3dlorenz.h \
    xfl3d/testgl/gl3dlorenz2.h \
    xfl3d/testgl/gl3doptim2d.h \
    xfl3d/testgl/gl3dsagittarius.h \
    xfl3d/testgl/gl3dshadow.h \
    xfl3d/testgl/gl3dsolarsys.h \
    xfl3d/testgl/gl3dspace.h \
    xfl3d/testgl/gl3dsurface.h \
    xfl3d/testgl/gl3dsurfaceplot.h \
    xfl3d/testgl/gl3dtestglview.h \
    xfl3d/testgl/spaceobject.h \
    xfl3d/views/gl3dbodyview.h \
    xfl3d/views/gl3dplaneview.h \
    xfl3d/views/gl3dview.h \
    xfl3d/views/gl3dwingview.h \
    xfl3d/views/gl3dxflview.h \
    xfl3d/views/shadloc.h \



SOURCES += \
    $$PWD/testgl/gl3dboids2.cpp \
    xfl3d/controls/arcball.cpp \
    xfl3d/controls/gllightdlg.cpp \
    xfl3d/glinfo/opengldlg.cpp \
    xfl3d/globals/gl_globals.cpp \
    xfl3d/globals/w3dprefs.cpp \
    xfl3d/testgl/gl2dfractal.cpp \
    xfl3d/testgl/gl2dnewton.cpp \
    xfl3d/testgl/gl2dview.cpp \
    xfl3d/testgl/gl3dattractors.cpp \
    xfl3d/testgl/gl3dboids.cpp \
    xfl3d/testgl/gl3dhydrogen.cpp \
    xfl3d/testgl/gl3dlorenz.cpp \
    xfl3d/testgl/gl3dlorenz2.cpp \
    xfl3d/testgl/gl3doptim2d.cpp \
    xfl3d/testgl/gl3dsagittarius.cpp \
    xfl3d/testgl/gl3dshadow.cpp \
    xfl3d/testgl/gl3dsolarsys.cpp \
    xfl3d/testgl/gl3dspace.cpp \
    xfl3d/testgl/gl3dsurface.cpp \
    xfl3d/testgl/gl3dsurfaceplot.cpp \
    xfl3d/testgl/gl3dtestglview.cpp \
    xfl3d/testgl/spaceobject.cpp \
    xfl3d/views/gl3dbodyview.cpp \
    xfl3d/views/gl3dplaneview.cpp \
    xfl3d/views/gl3dview.cpp \
    xfl3d/views/gl3dwingview.cpp \
    xfl3d/views/gl3dxflview.cpp \


RESOURCES += \
    xfl3d/resources/shaders.qrc \
    xfl3d/resources/text_files.qrc

DISTFILES += \
    $$PWD/resources/shaders/boids2/boids2_CS.glsl

