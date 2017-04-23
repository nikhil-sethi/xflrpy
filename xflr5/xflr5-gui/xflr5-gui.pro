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


CONFIG += qt
QT += opengl
QT += script
TEMPLATE = app


INCLUDEPATH += $$PWD/viewwidgets
INCLUDEPATH += $$PWD/viewwidgets/glWidgets
INCLUDEPATH += $$PWD/graph
INCLUDEPATH += $$PWD/misc
INCLUDEPATH += $$PWD/xdirect/xfoil_task
INCLUDEPATH += $$PWD/glcontextinfo

INCLUDEPATH += $$PWD/../xflr5-engine/objects
INCLUDEPATH += $$PWD/../xflr5-engine/objects/objects2d
INCLUDEPATH += $$PWD/../xflr5-engine/objects/objects3d
INCLUDEPATH += $$PWD/../xflr5-engine/XFoil
INCLUDEPATH += $$PWD/../xflr5-engine/analysis3d
INCLUDEPATH += $$PWD/../xflr5-engine/analysis3d/plane_analysis

DEPENDPATH += $$PWD/../xflr5-engine/objects
DEPENDPATH += $$PWD/../xflr5-engine/objects/objects2d
DEPENDPATH += $$PWD/../xflr5-engine/objects/objects3d
DEPENDPATH += $$PWD/../xflr5-engine/XFoil
DEPENDPATH += $$PWD/../xflr5-engine/analysis3d
DEPENDPATH += $$PWD/../xflr5-engine/analysis3d/plane_analysis

INCLUDEPATH += $$PWD/../graph-lib
DEPENDPATH += $$PWD/../graph-lib


INCLUDEPATH += $$PWD/../XFoil-lib
DEPENDPATH += $$PWD/../XFoil-lib


SOURCES += \
	XFLR5Application.cpp \
	globals.cpp \
	main.cpp \
	mainframe.cpp \
	viewwidgets/section2dwidget.cpp \
	viewwidgets/graphwidget.cpp \
	viewwidgets/graphtilewidget.cpp \
	viewwidgets/legendwidget.cpp \
	viewwidgets/BodyFrameWidget.cpp \
	viewwidgets/BodyLineWidget.cpp \
	viewwidgets/Direct2dDesign.cpp \
	viewwidgets/miarextilewidget.cpp \
	viewwidgets/xdirecttilewidget.cpp \
	viewwidgets/wingwidget.cpp \
	viewwidgets/oppointwidget.cpp \
	viewwidgets/inverseviewwidget.cpp \
	viewwidgets/ArcBall.cpp \
	misc/Settings.cpp \
	misc/SaveOptionsDlg.cpp \
	misc/ProgressDlg.cpp \
	misc/ModDlg.cpp \
	misc/PolarFilterDlg.cpp \
	misc/TranslatorDlg.cpp \
	misc/RenameDlg.cpp \
	misc/LinePickerDlg.cpp \
	misc/LineDelegate.cpp \
	misc/LineCbBox.cpp \
	misc/FloatEditDelegate.cpp \
	misc/ColorButton.cpp \
	misc/DoubleEdit.cpp \
	misc/IntEdit.cpp \
	misc/AboutQ5.cpp \
	misc/NewNameDlg.cpp \
	misc/ObjectPropsDlg.cpp \
	misc/LineBtn.cpp \
	misc/TextClrBtn.cpp \
	misc/MinTextEdit.cpp \
	misc/EditPlrDlg.cpp \
	misc/Units.cpp \
	misc/LengthUnitDlg.cpp \
	misc/exponentialslider.cpp \
	misc/stlexportdialog.cpp \
	miarex/Miarex.cpp \
	miarex/Objects3D.cpp \
	miarex/analysis/StabPolarDlg.cpp \
	miarex/analysis/CtrlTableDelegate.cpp \
	miarex/analysis/WAdvancedDlg.cpp \
	miarex/analysis/WPolarDlg.cpp  \
	miarex/analysis/LLTAnalysisDlg.cpp \
	miarex/analysis/PanelAnalysisDlg.cpp \
	miarex/analysis/AeroDataDlg.cpp \
	miarex/analysis/EditPolarDefDlg.cpp \
	miarex/design/GL3dWingDlg.cpp \
	miarex/design/GL3dBodyDlg.cpp \
	miarex/design/PlaneDlg.cpp \
	miarex/design/WingDelegate.cpp \
	miarex/design/WingScaleDlg.cpp \
	miarex/design/InertiaDlg.cpp \
	miarex/design/BodyGridDlg.cpp \
	miarex/design/BodyScaleDlg.cpp \
	miarex/design/BodyTableDelegate.cpp \
	miarex/design/BodyTransDlg.cpp \
	miarex/design/EditObjectDelegate.cpp \
	miarex/design/EditPlaneDlg.cpp \
	miarex/design/EditBodyDlg.cpp \
	miarex/design/wingseldlg.cpp \
	miarex/view/TargetCurveDlg.cpp \
	miarex/view/GL3DScales.cpp \
	miarex/view/StabViewDlg.cpp \
	miarex/view/W3dPrefsDlg.cpp \
	miarex/mgt/ImportObjectDlg.cpp\
	miarex/view/GLLightDlg.cpp \
	miarex/mgt/ManagePlanesDlg.cpp \
	miarex/mgt/PlaneTableDelegate.cpp \
	miarex/mgt/XmlPlaneReader.cpp \
	miarex/mgt/XmlPlaneWriter.cpp \
	miarex/mgt/xmlwpolarreader.cpp \
	miarex/mgt/xmlwpolarwriter.cpp \
	xdirect/XDirect.cpp \
	xdirect/ManageFoilsDlg.cpp \
	xdirect/XDirectStyleDlg.cpp \
	xdirect/analysis/BatchDlg.cpp \
	xdirect/analysis/BatchThreadDlg.cpp \
	xdirect/analysis/FoilPolarDlg.cpp \
	xdirect/analysis/ReListDlg.cpp \
	xdirect/analysis/XFoilAdvancedDlg.cpp \
	xdirect/analysis/XFoilAnalysisDlg.cpp \
	xdirect/analysis/XFoilTask.cpp \
	xdirect/geometry/CAddDlg.cpp \
	xdirect/geometry/FlapDlg.cpp \
	xdirect/geometry/FoilCoordDlg.cpp \
	xdirect/geometry/FoilGeomDlg.cpp \
	xdirect/geometry/InterpolateFoilsDlg.cpp \
	xdirect/geometry/LEDlg.cpp \
	xdirect/geometry/NacaFoilDlg.cpp \
	xdirect/geometry/TEGapDlg.cpp \
	xdirect/geometry/TwoDPanelDlg.cpp \
	xdirect/xmlpolarreader.cpp \
	xdirect/xmlpolarwriter.cpp \
	xdirect/objects2d.cpp \
	graph/GraphDlg.cpp \
	xinverse/FoilSelectionDlg.cpp \
	xinverse/PertDlg.cpp \
	xinverse/XInverse.cpp \
	xinverse/InverseOptionsDlg.cpp \
	design/FoilTableDelegate.cpp \
	design/LECircleDlg.cpp \
	design/AFoil.cpp \
	design/SplineCtrlsDlg.cpp \
	design/AFoilTableDlg.cpp \
	design/GridSettingsDlg.cpp \
	glcontextinfo/glrenderwindow.cpp \
	glcontextinfo/openglinfodlg.cpp \
	script/xflscriptexec.cpp \
	script/xflscriptreader.cpp \
	gui_objects/Spline5.cpp \
	gui_objects/SplineFoil.cpp \
    misc/scriptconsole.cpp \
	viewwidgets/glWidgets/gl3dview.cpp \
	viewwidgets/glWidgets/gl3dbodyview.cpp \
	viewwidgets/glWidgets/gl3dmiarexview.cpp \
	viewwidgets/glWidgets/gl3dwingview.cpp \
	viewwidgets/glWidgets/gl3dplaneview.cpp \
	misc/voidwidget.cpp


HEADERS += \
	globals.h \
	mainframe.h \
	XFLR5Application.h \
	viewwidgets/section2dwidget.h \
	viewwidgets/graphwidget.h \
	viewwidgets/graphtilewidget.h \
	viewwidgets/legendwidget.h \
	viewwidgets/BodyFrameWidget.h \
	viewwidgets/BodyLineWidget.h \
	viewwidgets/Direct2dDesign.h \
	viewwidgets/miarextilewidget.h \
	viewwidgets/xdirecttilewidget.h \
	viewwidgets/wingwidget.h \
	viewwidgets/oppointwidget.h \
	viewwidgets/inverseviewwidget.h \
	viewwidgets/ArcBall.h \
	misc/Settings.h \
	misc/SaveOptionsDlg.h \
	misc/ModDlg.h \
	misc/PolarFilterDlg.h \
	misc/TranslatorDlg.h \
	misc/RenameDlg.h \
	misc/LinePickerDlg.h \
	misc/LineDelegate.h \
	misc/FloatEditDelegate.h \
	misc/ColorButton.h \
	misc/LineCbBox.h \
	misc/AboutQ5.h \
	misc/DoubleEdit.h \
	misc/IntEdit.h \
	misc/ProgressDlg.h \
	misc/NewNameDlg.h \
	misc/ObjectPropsDlg.h \
	misc/LineBtn.h \
	misc/TextClrBtn.h \
	misc/MinTextEdit.h \
	misc/EditPlrDlg.h \
	misc/Units.h \
	misc/LengthUnitDlg.h \
	misc/exponentialslider.h \
	misc/stlexportdialog.h \
	miarex/Miarex.h \
	miarex/Objects3D.h \
	miarex/analysis/WAdvancedDlg.h \
	miarex/analysis/WPolarDlg.h \
	miarex/analysis/StabPolarDlg.h \
	miarex/analysis/CtrlTableDelegate.h \
	miarex/analysis/LLTAnalysisDlg.h \
	miarex/analysis/PanelAnalysisDlg.h \
	miarex/analysis/AeroDataDlg.h \
	miarex/analysis/EditPolarDefDlg.h \
	miarex/design/InertiaDlg.h \
	miarex/design/GL3dBodyDlg.h \
	miarex/design/WingScaleDlg.h \
	miarex/design/WingDelegate.h \
	miarex/design/PlaneDlg.h \
	miarex/design/BodyGridDlg.h \
	miarex/design/BodyTableDelegate.h \
	miarex/design/BodyScaleDlg.h \
	miarex/design/GL3dWingDlg.h \
	miarex/design/BodyTransDlg.h \
	miarex/design/EditObjectDelegate.h \
	miarex/design/EditPlaneDlg.h \
	miarex/design/EditBodyDlg.h \
	miarex/design/wingseldlg.h \
	miarex/view/TargetCurveDlg.h \
	miarex/view/GL3DScales.h \
	miarex/view/StabViewDlg.h \
	miarex/view/GLLightDlg.h \
	miarex/view/W3dPrefsDlg.h \
	miarex/mgt/ImportObjectDlg.h \
	miarex/mgt/ManagePlanesDlg.h \
	miarex/mgt/PlaneTableDelegate.h \
	miarex/mgt/XmlPlaneReader.h \
	miarex/mgt/XmlPlaneWriter.h \
	miarex/mgt/xmlwpolarreader.h \
	miarex/mgt/xmlwpolarwriter.h \
	xdirect/XDirect.h \
	xdirect/ManageFoilsDlg.h \
	xdirect/XDirectStyleDlg.h \
	xdirect/analysis/BatchDlg.h \
	xdirect/analysis/BatchThreadDlg.h \
	xdirect/analysis/FoilPolarDlg.h \
	xdirect/analysis/ReListDlg.h \
	xdirect/analysis/XFoilAdvancedDlg.h \
	xdirect/analysis/XFoilAnalysisDlg.h \
	xdirect/analysis/XFoilTask.h \
	xdirect/analysis/xfoiltaskevent.h \
	xdirect/geometry/CAddDlg.h \
	xdirect/geometry/FlapDlg.h \
	xdirect/geometry/FoilCoordDlg.h \
	xdirect/geometry/FoilGeomDlg.h \
	xdirect/geometry/InterpolateFoilsDlg.h \
	xdirect/geometry/LEDlg.h \
	xdirect/geometry/NacaFoilDlg.h \
	xdirect/geometry/TEGapDlg.h \
	xdirect/geometry/TwoDPanelDlg.h \
	xdirect/xmlpolarreader.h \
	xdirect/xmlpolarwriter.h \
	xdirect/objects2d.h \
	xinverse/XInverse.h \
	xinverse/InverseOptionsDlg.h \
	xinverse/FoilSelectionDlg.h \
	xinverse/PertDlg.h \
	graph/GraphDlg.h \
	design/AFoil.h \
	design/LECircleDlg.h \
	design/SplineCtrlsDlg.h \
	design/FoilTableDelegate.h \
	design/AFoilTableDlg.h \
	design/GridSettingsDlg.h \
	glcontextinfo/glrenderwindow.h \
	glcontextinfo/openglinfodlg.h \
	script/xflscriptexec.h \
	script/xflscriptreader.h \
	gui_objects/Spline5.h \
	gui_objects/SplineFoil.h \
	gui_enums.h \
	gui_params.h \
    misc/scriptconsole.h \
	viewwidgets/glWidgets/gl3dview.h \
	viewwidgets/glWidgets/gl3dbodyview.h \
	viewwidgets/glWidgets/gl3dmiarexview.h \
	viewwidgets/glWidgets/gl3dwingview.h \
	viewwidgets/glWidgets/gl3dplaneview.h \
	misc/voidwidget.h
	

RESOURCES += \
	scripts.qrc \
	images.qrc \
	shaders.qrc \
	textures.qrc



win32 {
    TARGET = XFLR5
	CONFIG(release, debug|release){
		LIBS += -L$$OUT_PWD/../xflr5-engine/release/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../XFoil-lib/release/ -lXFoil-lib
		LIBS += -L$$OUT_PWD/../graph-lib/release/ -lgraph-lib
	}
	else:CONFIG(debug, debug|release)
	{
		LIBS += -L$$OUT_PWD/../xflr5-engine/debug/ -lxflr5-engine
		LIBS += -L$$OUT_PWD/../XFoil-lib/debug/ -lXFoil-lib
		LIBS += -L$$OUT_PWD/../graph-lib/debug/ -lgraph-lib
	}

	RC_FILE = ../win/xflr5.rc
	LIBS += -lopenGL32
}


unix{
	TARGET = xflr5
	LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil-lib
	LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine
	LIBS += -L$$OUT_PWD/../graph-lib/ -lgraph-lib

#	release: DESTDIR = ../build/release
#	debug:   DESTDIR = ../build/debug

#	OBJECTS_DIR = $$DESTDIR/.o
#	MOC_DIR = $$DESTDIR/.moc
#	RCC_DIR = $$DESTDIR/.rcc

	# VARIABLES
	isEmpty(PREFIX):PREFIX = /usr
	BINDIR = $$PREFIX/bin
	DATADIR = $$PREFIX/share

	# MAKE INSTALL
	INSTALLS += target
	target.path = $$BINDIR
}

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
    DESTDIR = ../
    TARGET = XFLR5
    TEMPLATE = app
    CONFIG += i386
    QMAKE_MAC_SDK = macosx
    #QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
    #QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
    OTHER_FILES += ./mac/Info.plist
    LIBS += -L$$OUT_PWD/../XFoil-lib/ -lXFoil-lib
    LIBS += -L$$OUT_PWD/../xflr5-engine/ -lxflr5-engine
    LIBS += -L$$OUT_PWD/../graph-lib/ -lgraph-lib
    LIBS += -framework \
        CoreFoundation
    QMAKE_INFO_PLIST = ./mac/Info.plist
    ICON = ./mac/xflr5.icns
}

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4




