/****************************************************************************

    GL3dWingDlg Class
    Copyright (C) 2009-2019 Andre Deperrois

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

#ifndef GL3DWINGDLG_H
#define GL3DWINGDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QSlider>
#include <QSettings>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QSplitter>
#include <QOpenGLBuffer>
#include <QDialogButtonBox>

#include <objects/objects3d/vector3d.h>

class gl3dWingView;
class DoubleEdit;
class ColorButton;
class Wing;
class WingDelegate;
class Panel;
class Foil;


class GL3dWingDlg: public QDialog
{
    Q_OBJECT


    friend class Miarex;
    friend class gl3dView;
    friend class gl3dWingView;
    friend class Wing;
    friend class GLLightDlg;
    friend class PlaneDlg;
    friend class WingDelegate;

public:
    GL3dWingDlg(QWidget *pParent=nullptr);
    ~GL3dWingDlg();

    bool intersectObject(Vector3d AA,  Vector3d U, Vector3d &I);
    void glMake3DObjects();
    void glMakeSectionHighlight(QOpenGLBuffer &vbo, int &nStrips, int &stripSize);
    int iSection() const {return m_iSection;}

    static void loadSettings(QSettings &settings);
    static void saveSettings(QSettings &settings);


private slots:
    void onAxes();
    void onCheckViewIcons();
    void onSurfaces();
    void onOutline();
    void onPanels();
    void onFoilNames();
    void onShowMasses();

    void onOK();
    void onDescriptionChanged();
    void onCellChanged(QWidget *);
    void onItemClicked(const QModelIndex &index);
    void onWingColor();
    void onTextures();
    void onSide();
    void onSymetric();
    void onInsertBefore();
    void onInsertAfter();
    void onDeleteSection();
    void onResetSection();
    void onResetMesh();
    void onScaleWing();
    void onInertia();
    void onImportWing();
    void onExportWing();
    void onImportWingFromXML();
    void onExportWingToXML();
    void onButton(QAbstractButton *pButton);

private:
    void accept();
    void reject();

    void keyPressEvent(QKeyEvent *pEvent);
    void resizeEvent(QResizeEvent *pEvent);
    void showEvent(QShowEvent *pEvent);
    void hideEvent(QHideEvent *pEvent);
    void contextMenuEvent(QContextMenuEvent *pEvent);

    bool checkWing();
    void createXPoints(int NXPanels, int XDist, Foil *pFoilA, Foil *pFoilB, double *xPointA, double *xPointB, int &NXLead, int &NXFlap);
    void computeGeometry();

    void setWingData();
    void fillDataTable();
    void fillTableRow(int row);
    void readParams();
    void readSectionData(int sel);
    void setCurrentSection(int section);
    void setScale();
    int VLMGetPanelTotal();
    bool VLMSetAutoMesh(int total=0);

    bool initDialog(Wing *pWing);
    void connectSignals();
    void setupLayout();


private:

    static QByteArray s_WindowGeometry;

    static bool s_bOutline;
    static bool s_bSurfaces;
    static bool s_bVLMPanels;
    static bool s_bAxes;
    static bool s_bShowMasses;
    static bool s_bFoilNames;

    gl3dWingView *m_pglWingView;              /**< a pointer to the openGL 3.0 widget where 3d calculations and rendering are performed */

    QLineEdit *m_pctrlWingName;
    QTextEdit *m_pctrlWingDescription;
    QCheckBox *m_pctrlSymetric;
    QRadioButton *m_pctrlLeftSide, *m_pctrlRightSide;
    ColorButton *m_pctrlWingColor;
    QPushButton *m_pctrlResetMesh;
    QPushButton *m_pctrlInsertBefore, *m_pctrlInsertAfter, *m_pctrlDeleteSection;
    QLabel *m_pctrlWingSpan, *m_pctrlWingArea, *m_pctrlMAC, *m_pctrlGeomChord;
    QLabel *m_pctrlAspectRatio, *m_pctrlTaperRatio, *m_pctrlSweep, *m_pctrlNFlaps;
    QLabel *m_pctrlVLMPanels, *m_pctrl3DPanels;
    QLabel *m_pctrlProjectedArea, *m_pctrlProjectedSpan;

    QLabel *m_pctrlLength1, *m_pctrlLength2, *m_pctrlLength3, *m_pctrlLength4;
    QLabel *m_pctrlAreaUnit1, *m_pctrlAreaUnit2, * m_pctrlVolumeUnit;
    QTableView *m_pctrlWingTable;
    QStandardItemModel *m_pWingModel;
    WingDelegate *m_pWingDelegate;

    QDialogButtonBox *m_pButtonBox;

    QCheckBox *m_pctrlAxes, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels, *m_pctrlFoilNames;
    QCheckBox *m_pctrlShowMasses;
    QPushButton *m_pctrlReset;
    QRadioButton *m_pctrlColor, *m_pctrlTextures;

    QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso, *m_pctrlFlip;
    QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;

    QAction *m_pScaleWing, *m_pInertia;
    QAction *m_pExportWingAct, *m_pImportWingAct;
    QAction *m_pExportWingXml, *m_pImportWingXml;

    QAction *m_pResetScales;


    QMenu *m_pContextMenu;
    QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteSection, *m_pResetSection;

    QWidget *m_pctrlControlsWidget;
    QSplitter *m_pctrlLeftSideSplitter;
    QSplitter *m_pctrlHSplitter;

    static QByteArray s_LeftSplitterSizes;
    static QByteArray s_HSplitterSizes;


    Wing *m_pWing;

    bool m_bAcceptName;
    bool m_bRightSide;
    bool m_bChanged, m_bDescriptionChanged;
    bool m_bTrans;
    bool m_bStored;
    bool m_bEnableName;
    bool m_bResetglWing;
    bool m_bResetglSectionHighlight;


    int m_iSection;


    QPoint m_MousePos;
    QPoint m_ptPopUp;
    QPoint m_LastPoint;


    Vector3d m_RealPopUp;

    Panel *m_pPanel;
    Vector3d *m_pNode;


    int  *m_precision;
};

#endif // GL3DWINGDLG_H
