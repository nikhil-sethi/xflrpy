/****************************************************************************

	MainFrame Class

    Copyright (C) 2008-2017 Andre Deperrois adeperrois@xflr5.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*****************************************************************************/
/**
 *@file mainframe.h
 *@brief This file contains the description of the MainFrame class associated to the application's main window
 *
*/
#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <QWidget>
#include <QDir>
#include <QTimer>
#include <QStringList>
#include <QStackedWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QLabel>
#include <QPointer>
#include <QTranslator>
#include <QMainWindow>
#include <QList>
#include <gui_enums.h>
#include <analysis3d_enums.h>
#include <gui_params.h>
#include <QGraph.h>
#include <miarex/view/GLLightDlg.h>
#include <misc/voidwidget.h>

class QMiarex;
class QXDirect;
class QAFoil;
class QXInverse;
class gl3dMiarexView;
class InverseViewWidget;
class QGraph;
class Direct2dDesign;
class XDirectTileWidget;
class MiarexTileWidget;
class CVector;
class GLLightDlg;
class GL3DScales;
class Foil;
class Polar;
class OpPoint;

/**
*@class MainFrame
*@brief The class associated to the application's main window.

  The class fills many functions:
  - it creates the child windows and toolbars of the application
  - it manages the loading and saving of settings
  - it stores and manages the arrays of data as member variables
  - it manages the load & save operations of project files 
  
  This class will remain only partially documented.
*/
class MainFrame : public QMainWindow
{
	friend class InverseViewWidget;
	friend class QXDirect;
	friend class QMiarex;
	friend class Objects3D;
	friend class QAFoil;
	friend class QXInverse;
	friend class Miarex;
	friend class Body;
	friend class Wing;
	friend class WPolar;
	friend class OpPoint;
	friend class CWOpp;
	friend class Plane;
	friend class BodyGridDlg;
	friend class XFoilAnalysisDlg;
	friend class FoilPolarDlg;
	friend class BatchDlg;
	friend class BatchThreadDlg;
	friend class InterpolateFoilsDlg;
	friend class WingDlg;
	friend class WPolarDlg;
	friend class StabPolarDlg;
	friend class StabViewDlg;
	friend class PlaneDlg;
	friend class PanelAnalysisDlg;
	friend class GL3dBodyDlg;
	friend class GL3DScales;
	friend class ManageBodiesDlg;
	friend class WingScaleDlg;
	friend class BodyTransDlg;
	friend class GL3dWingDlg;
	friend class WAdvancedDlg;
	friend class GraphDlg;
	friend class LLTAnalysisDlg;
	friend class wySettingsDlg;
	friend class ManageFoilsDlg;
	friend class ManagePlanesDlg;
	friend class InertiaDlg;
	friend class TranslatorDlg;
	friend class Settings;
	friend class Direct2dDesign;
	friend class GraphTileWidget;
	friend class MiarexTileWidget;
	friend class XDirectTileWidget;
	friend class OpPointWidget;
	friend class gl3dView;
	friend class gl3dMiarexView;
	friend class LanguageOptions;

	Q_OBJECT

public:
	MainFrame(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~MainFrame();

	XFLR5::enumApp loadXFLR5File(QString PathName);
	static MainFrame* self();

/*___________________________________________Methods_______________________________*/

public slots:
	void onAFoil();
	void onXDirect();
	void onXInverse();
	void onXInverseMixed();
	void onMiarex();
	void onExecuteScript();

private slots:
	void aboutQt();
	void aboutXFLR5();
	void onCurFoilStyle();
	void onExportCurGraph();
	void onCurGraphSettings();
	void onInsertProject();
	void onHighlightOperatingPoint();
	void onNewProject();
	void onLoadFile();
	void onLoadLastProject();
	void onLogFile();
	void onOpenGLInfo();
	void onPreferences();
	void onProjectModified();
	void onResetCurGraphScales();
	void onResetSettings();
	void onRestoreToolbars();
	bool onSaveProjectAs();
	void onSaveTimer();
	void onSaveViewToImageFile();
	void onSelChangeFoil(int sel);
	void onSelChangePolar(int sel);
	void onSelChangeOpp(int sel);
	void onSelChangePlane(int sel);
	void onSelChangePlaneOpp(int sel);
	void onSelChangeWPolar(int sel);
	void onSaveProject();
	void onManageFoils();
	void onSavePlaneAsProject();
	void onOpenRecentFile();
	void onShowMousePos();

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);
	void closeEvent (QCloseEvent * event);
	void showEvent(QShowEvent *event);

public:
	void addRecentFile(const QString &PathNAme);
	void checkGraphActions();
	void ClientToGL(QPoint const &point, CVector &real);
	void createDockWindows();
	void createToolbars();
	void createStatusBar();
	void createActions();
	void createMenus();
	void createGraphActions();
	void createXDirectActions();
	void createXDirectMenus();
	void createXDirectToolbar();
	void createXInverseActions();
	void createXInverseMenus();
	void createXInverseToolbar();
	void createMiarexActions();
	void createMiarexMenus();
	void createMiarexToolbar();
	void createAFoilActions();
	void createAFoilMenus();
	void createAFoilToolbar();
	void deleteProject(bool bClosing=false);
	void exportGraph(QGraph *pGraph);
	void GLToClient(CVector const &real, QPoint &point);
	static bool hasOpenGL(){return s_bOpenGL;}
	bool loadSettings();
	bool loadPolarFileV3(QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
	void readPolarFile(QDataStream &ar);
	bool saveProject(QString PathName="");
	void saveSettings();
	void selectFoil(void *pFoilPtr);
	void selectPolar(void *pPolarPtr);
	void selectOpPoint(void *pOppPtr);
	void selectPlane(void *pPlanePtr);
	void selectWPolar(void *pWPolarPtr);
	void selectPlaneOpp(void *pPlaneOppPtr);
	bool serializeProjectWPA(QDataStream &ar, bool bIsStoring);
	bool serializeProjectXFL(QDataStream &ar, bool bIsStoring);
	bool serializePlaneProject(QDataStream &ar);
	bool serializeOppXFL(OpPoint *pOpp, QDataStream &ar, bool bIsStoring, int ArchiveFormat=0);
	bool serializePolarXFL(Polar *pPolar, QDataStream &ar, bool bIsStoring);
	bool serializeFoilXFL(Foil *pFoil, QDataStream &ar, bool bIsStoring);

	void setMainFrameCentralWidget();
	void setGraphSettings(QGraph *pGraph);
	void setProjectName(QString PathName);
	void setMenus();
	void setupDataDir();
	QString shortenFileName(QString &PathName);
	void updateFoilListBox();
	void updatePolarListBox();
	void updateOppListBox();
	void updateRecentFileActions();
	void updatePlaneListBox();
	void updateView();
	void updateWPolarListBox();
	void updatePOppListBox();
	void writePolars(QDataStream &ar, void *pFoilPtr=NULL);

	static void setSaveState(bool bSave);
	static void readStyleSheet(QString styleSheetName, QString &styleSheet);
	static QColor getColor(int type);


	XFLR5::enumApp xflr5App(){return m_iApp;}
	QString &exportLastDirName() {return m_ExportLastDirName;}
	QString &exportGraphFilter() {return m_GraphExportFilter;}

/*___________________________________________Variables_______________________________*/

private:

	QAFoil *m_pAFoil;     /**< A void pointer to the instance of the QAFoil application. The pointer will be cast to the QAFoil type at runtime. This is necessary to prevent loop includes of header files. */
	QMiarex *m_pMiarex;    /**< A void pointer to the instance of the QMiarex application. The pointer will be cast to the QMiarex type at runtime. This is necessary to prevent loop includes of header files. */
	QXInverse *m_pXInverse;  /**< A void pointer to the instance of the QXInverse application. The pointer will be cast to the QXInverse type at runtime. This is necessary to prevent loop includes of header files. */
	QXDirect *m_pXDirect;   /**< A void pointer to the instance of the QXDirect application. The pointer will be cast to the QXDirect type at runtime. This is necessary to prevent loop includes of header files. */
	void *m_pStabView;  /** < A void pointer to the instance of the StabViewDlg window. */
	
	GLLightDlg m_glLightDlg;
	GL3DScales *m_pGL3DScales;


	static QPointer<MainFrame> _self; /**< necessary for MacOS >*/

	QStackedWidget *m_pctrlCentralWidget;     /** The stacked widget which is loaded at the center of the display area. The stack switches between the widgets depending on the user's request. */
	VoidWidget m_VoidWidget;
	InverseViewWidget *m_p2dWidget;           /** A pointer to the instance of the TwoDWidget which is used to perform 2d drawings */
	Direct2dDesign *m_pDirect2dWidget;        /** A pointer to the instance of the TwoDWidget which is used to perform 2d drawings of foils in Direct Design */
	gl3dMiarexView *m_pgl3dMiarexView;                  /** A pointer to the instance of the OpenGL 3.0 widget where 3d calculations and rendering are performed */
	MiarexTileWidget *m_pMiarexTileWidget;
	XDirectTileWidget *m_pXDirectTileWidget;

	QDockWidget *m_pctrlXDirectWidget, *m_pctrlMiarexWidget, *m_pctrlAFoilWidget, *m_pctrlXInverseWidget;
	QDockWidget *m_pctrl3DScalesWidget, *m_pctrlStabViewWidget;

	QToolBar *m_pctrlXDirectToolBar;   /**< The tool bar container which holds the instance of the QXDirect application  */
	QToolBar *m_pctrlXInverseToolBar;
	QToolBar *m_pctrlMiarexToolBar;
	QToolBar *m_pctrlAFoilToolBar;

	//Common Menus
	QMenu *m_pMainMenu;
	QMenu *m_pFileMenu, *m_pOptionsMenu, *m_pHelpMenu, *m_pGraphMenu;

	//AFoilMenus
	QMenu *m_pAFoilViewMenu, *m_pAFoilDesignMenu, *m_pAFoilSplineMenu;
	QMenu *m_pAFoilCtxMenu,*m_pAFoilCurrentFoilMenu, *m_pAFoilTableCtxMenu;
    QMenu *m_pAFoilDesignMenu_AFoilCtxMenu, *m_pAFoilSplineMenu_AFoilCtxMenu;

	//  XFoilAnalysis Menus
	QMenu *m_pXDirectViewMenu;
	QMenu *m_pXDirectFoilMenu, *m_pCurOppCtxMenu;
    QMenu *m_pCurrentFoilMenu, *m_pCurrentFoilMenu_OperFoilCtxMenu, *m_pCurrentFoilMenu_OperPolarCtxMenu;
    QMenu *m_pDesignMenu, *m_pDesignMenu_OperPolarCtxMenu;
	QMenu *m_pXFoilAnalysisMenu;
    QMenu *m_pOpPointMenu, *m_pXDirectCpGraphMenu, *m_pXDirectCpGraphMenu_OperPolarCtxMenu, *m_pCurrentOppMenu;
    QMenu *m_pPolarMenu, *m_pCurrentPolarMenu, *m_pCurrentPolarMenu_OperFoilCtxMenu, *m_pCurrentPolarMenu_OperPolarCtxMenu;
    QMenu *m_pGraphPolarMenu, *CurPolarGraphMenu;
    QMenu *m_pOperFoilCtxMenu, *m_pOperPolarCtxMenu;

	//XInverse menu
	QMenu *m_pXInverseViewMenu, *m_pXInverseFoilMenu, *m_pXInverseGraphMenu, *m_pInverseContextMenu;

	//Miarex Menus
	QMenu *m_pMiarexViewMenu;
	QMenu *m_pMiarexAnalysisMenu;
	QMenu *m_pPlaneMenu, *m_pCurrentPlaneMenu, *m_pCurWPlrMenu, *m_pCurWOppMenu;
    QMenu *m_pCurrentPlaneMenu_WOppCtxMenu, *m_pCurrentPlaneMenu_WCpCtxMenu, *m_pCurrentPlaneMenu_WTimeCtxMenu;
    QMenu *m_pCurrentPlaneMenu_WPlrCtxMenu, *m_pCurrentPlaneMenu_W3DCtxMenu, *m_pCurrentPlaneMenu_W3DStabCtxMenu;
    QMenu *m_pCurWPlrMenu_WOppCtxMenu, *m_pCurWPlrMenu_WCpCtxMenu, *m_pCurWPlrMenu_WTimeCtxMenu;
    QMenu *m_pCurWPlrMenu_WPlrCtxMenu, *m_pCurWPlrMenu_W3DCtxMenu, *m_pCurWPlrMenu_W3DStabCtxMenu;
    QMenu *m_pCurWOppMenu_WOppCtxMenu, *m_pCurWOppMenu_WCpCtxMenu, *m_pCurWOppMenu_WTimeCtxMenu;
    QMenu *m_pCurWOppMenu_W3DCtxMenu, *m_pCurWOppMenu_W3DStabCtxMenu;
    QMenu *m_pMiarexWPlrMenu, *m_pMiarexWOppMenu;
	QMenu *m_pWPlrCtxMenu, *m_pWOppCtxMenu, *m_pW3DCtxMenu, *m_pWCpCtxMenu, *m_pWTimeCtxMenu, *m_pW3DStabCtxMenu;

	//MainFrame actions
	QAction *m_pOnXDirectAct, *m_pOnMiarexAct, *m_pOnAFoilAct, *m_pOnXInverseAct, *m_pOnMixedInverseAct;
	QAction *m_pOpenAct, *m_pInsertAct;
	QAction *m_pSaveAct, *m_pSaveProjectAsAct,*m_pNewProjectAct, *m_pCloseProjectAct;

	QAction *m_pExitAct;
	QAction *m_pAboutAct, *m_pAboutQtAct, *m_pOpenGLAct;
	QAction *m_pPreferencesAct;
	QAction *m_pRecentFileActs[MAXRECENTFILES];
	QAction *m_pSeparatorAct;
	QAction *m_pSaveViewToImageFileAct, *m_pResetSettingsAct;
	QAction *m_pLoadLastProjectAction;

	//Graph Actions
	QAction *m_pSingleGraph[MAXGRAPHS], *m_pTwoGraphs, *m_pFourGraphs, *m_pAllGraphs;
	QAction *m_pGraphDlgAct;
	QAction *m_pShowMousePosAct;

	//AFoil Actions
	QAction *m_pZoomInAct, *m_pResetXScaleAct, *m_pResetYScaleAct, *m_pResetXYScaleAct;
	QAction *m_pZoomYAct, *m_pZoomLessAct, *m_pAFoilGridAct;
	QAction *m_pAFoilDelete, *m_pAFoilRename, *m_pAFoilExport, *m_pAFoilDuplicateFoil;
	QAction *m_pAFoilSetTEGap, *m_pAFoilSetLERadius, *m_pAFoilSetFlap, *m_pAFoilInterpolateFoils, *m_pAFoilNacaFoils;
	QAction *m_pAFoilDerotateFoil, *m_pAFoilNormalizeFoil, *m_pAFoilRefineLocalFoil, *m_pAFoilRefineGlobalFoil;
	QAction *m_pAFoilEditCoordsFoil, *m_pAFoilScaleFoil;
	QAction *m_pAFoilLECircle, *m_pShowLegend;
	QAction *m_pUndoAFoilAct, *m_pRedoAFoilAct;
	QAction *m_pHideAllFoils, *m_pShowAllFoils, *m_pShowCurrentFoil, *m_pHideCurrentFoil;
	QAction *m_pStoreSplineAct, *m_pNewSplinesAct, *m_pSplineControlsAct, *m_pExportSplinesToFileAct;
	QAction *m_pInsertSplinePt, *m_pRemoveSplinePt;
	QAction *m_pAFoilTableColumns, *m_pAFoilTableColumnWidths;
	QAction *m_pAFoilLoadImage, *m_pAFoilClearImage;

	//Miarex Actions
	QAction *m_pWPolarAct, *m_pWOppAct, *m_pW3DAct, *m_pCpViewAct, *m_pStabTimeAct, *m_pRootLocusAct;
	QAction *m_pW3DPrefsAct, *m_pW3DLightAct, *m_pW3DScalesAct, *m_pReset3DScale;
	QAction *m_pDefinePlaneAct, *m_pDefinePlaneObjectAct, *m_pEditPlaneAct, *m_pEditBodyAct, *m_pEditBodyObjectAct;
	QAction *m_pEditWingAct, *m_pEditStabAct, *m_pEditFinAct;
	QAction *m_pSavePlaneAsProjectAct, *m_pRenameCurPlaneAct, *m_pDeleteCurPlane, *m_pDuplicateCurPlane;
	QAction *m_pRenameCurWPolar, *m_pEditWPolarAct, *m_pEditWPolarPts, *m_pExportCurWPolar, *m_pResetCurWPolar;
	QAction *m_pShowPolarProps, *m_pShowWOppProps;
	QAction *m_pDeleteCurWPolar, *m_pDeleteCurWOpp;
	QAction *m_pEditObjectAct, *m_pEditWPolarObjectAct;
	QAction *m_pImportPlaneFromXml, *m_pImportAnalysisFromXml, *m_pExportPlaneToXML, *m_pExportAnalysisToXML;

	QAction *m_pMiarexPolarFilter;
	QAction *m_pAllGraphsScalesAct, *m_pAllGraphsSettings;
	QAction *m_pHideAllWPlrs, *m_pShowAllWPlrs;
	QAction *m_pHidePlaneWPlrs, *m_pShowPlaneWPlrs, *m_pShowPlaneWPlrsOnly, *m_pDeletePlaneWPlrs;
	QAction *m_pHidePlaneWOpps, *m_pShowPlaneWOpps, *m_pDeletePlaneWOpps;
	QAction *m_pExportCurWOpp, *m_pShowCurWOppOnly, *m_pHideAllWOpps, *m_pShowAllWOpps, *m_pDeleteAllWOpps, *m_pShowWPlrOppsOnly;
	QAction *m_pShowAllWPlrOpps, *m_pHideAllWPlrOpps, * m_pDeleteAllWPlrOpps;
	QAction *m_pDefineWPolar, *m_pDefineStabPolar, *m_pDefineWPolarObjectAct, *m_pAadvancedSettings;
	QAction *m_pShowTargetCurve, *m_pShowXCmRefLocation, *m_pShowStabCurve, *m_pShowFinCurve, *m_pShowWing2Curve;
	QAction *m_pExporttoAVL, *m_pExporttoSTL;
	QAction *m_pManagePlanesAct, *m_pScaleWingAct;
	QAction *m_pImportWPolars, *m_pExportWPolars, *m_pPlaneInertia;
	QAction *m_pShowFlapMoments;

	//XDirect Actions
	QAction *m_pPolarsAct, *m_pOpPointsAct, *m_pDeletePolar, *m_pDefinePolarAct, *m_pEditCurPolar, *m_pBatchAnalysisAct, *m_pResetCurPolar;
	QAction *m_pMultiThreadedBatchAct;
	QAction *m_pRestoreToolbarsAct;
	QAction *m_pExportCurPolar, *m_pExportAllPolars, *m_pHideFoilPolars, *m_pShowFoilPolars, *m_pShowFoilPolarsOnly, *m_pSaveFoilPolars,*m_pDeleteFoilPolars;
	QAction *m_pShowAllPolars, *m_pHideAllPolars, *m_pShowCurOppOnly, *m_pShowAllOpPoints, *m_pHideAllOpPoints, *m_pExportPolarOpps;
	QAction *m_pHideFoilOpps, *m_pShowFoilOpps, *m_pDeleteFoilOpps;
	QAction *m_pHidePolarOpps, *m_pShowPolarOpps, *m_pDeletePolarOpps;
	QAction *m_pExportCurOpp, *m_pDeleteCurOpp, *m_pGetOppProps, *m_pGetPolarProps;
	QAction *m_pViewXFoilAdvanced, *m_pViewLogFile, *m_pShowNeutralLine, *m_pResetFoilScale, *m_pShowInviscidCurve;
	QAction *m_pExportCurFoil, *m_pDeleteCurFoil, *m_pRenameCurFoil, *m_pSetCurFoilStyle;
	QAction *m_pDerotateFoil, *m_pNormalizeFoil, *m_pRefineLocalFoil, *m_pRefineGlobalFoil , *m_pEditCoordsFoil, *m_pScaleFoil;
	QAction *m_pSetTEGap, *m_pSetLERadius, *m_pSetFlap, *m_pInterpolateFoils, *m_pNacaFoils, *m_pDirectDuplicateCurFoil;

	QAction *m_pCurGraphDlgAct,*m_pExportCurGraphAct, *m_pResetCurGraphScales;

	QAction *m_pXDirectStyleAct;
	QAction *m_pXDirectPolarFilter;
	QAction *m_psetQVarGraph, *m_psetCpVarGraph;
	QAction *m_pExportBLData;
	QAction *m_pManageFoilsAct, *m_pRenamePolarAct;
	QAction *m_pImportJavaFoilPolar, *m_pImportXFoilPolar;
	QAction *m_pImportXMLFoilAnalysis, *m_pExportXMLFoilAnalysis;
	QAction *m_pHighlightOppAct;

	QComboBox *m_pctrlFoil, *m_pctrlPolar, * m_pctrlOpPoint;
	QComboBox *m_pctrlPlane, *m_pctrlPlanePolar, * m_pctrlPlaneOpp;
	QRadioButton *m_pctrlFullInverse, *m_pctrlMixedInverse;
	static QLabel *m_pctrlProjectName;

	//XInverse Actions
	QAction *m_pStoreFoil, *m_pExtractFoil, *m_pXInverseStyles, *m_pXInverseResetFoilScale, *m_pInverseInsertCtrlPt, *m_pInverseRemoveCtrlPt;
	QAction *m_pInvQInitial, *m_pInvQSpec, *m_pInvQViscous, *m_pInvQPoints, *m_pInvQReflected;
	QAction *m_pInverseZoomIn;


	//Script actions
	QAction *m_pExecScript;

	QStringList m_RecentFiles;

	XFLR5::enumApp m_iApp;                 /**< The identification number of the active app. */

	static bool s_bSaved;       /**< true if the project has not been modified since the last save operation. */
	bool m_bAutoLoadLast;       /**< true if the last project should be loaded on startup */
	bool m_bSaveOpps;           /**< true if the foil operating points should be serialized in the project file */
	bool m_bSaveWOpps;          /**< true if the wing operating points should be serialized in the project file */
	bool m_bAutoSave;           /**< true if the project should be auto-saved on regular intervals */
	bool m_bSaveSettings;       /**< true if user-defined settings should be saved on exit. */

	int m_SaveInterval;         /**< the time interval in muinutes between two project auto-saves */

	static QDir s_StylesheetDir;
	static QDir s_TranslationDir;
	static QString s_LanguageFilePath;

	QString m_ExportLastDirName, m_ImageDirName;
	QString m_FileName;         /**< The absolute path to the file of the current project. */

	static QList <QColor> s_ColorList;

	QString m_GraphExportFilter;

	XFLR5::enumImageFormat m_ImageFormat;   /**< The index of the type of image file which should be used. */
	QTimer *m_pSaveTimer;          /**< The timer which triggers the autosaving of the project at given intervals */

public:
	static bool s_bTrace;
	static bool s_bShowMousePos;
	static bool s_bOpenGL;
	static QFile *s_pTraceFile;
	static QString s_ProjectName;      /**< The Project's name. */


	QTranslator m_Translator;  /**< the translator object; due to a Qt bug, need to load twice: once from the main function, once from the mainframe */
};

#endif // MAINFRAME_H
 
