/****************************************************************************

    GL3dWingDlg Class
    Copyright (C) André Deperrois

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

#pragma once

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

#include <xflgeom/geom3d/vector3d.h>

class gl3dWingView;
class DoubleEdit;
class ColorMenuBtn;
class Wing;
class WingDelegate;
class Panel;
class Foil;


class WingDlg: public QDialog
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
        WingDlg(QWidget *pParent=nullptr);
        ~WingDlg();

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
        void onWingColor(QColor clr);

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
        void accept() override;
        void reject() override;

        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void contextMenuEvent(QContextMenuEvent *pEvent) override;

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

        QLineEdit *m_pleWingName;
        QTextEdit *m_pteWingDescription;
        QCheckBox *m_pchSymetric;
        QRadioButton *m_prbLeftSide, *m_prbRightSide;
        ColorMenuBtn *m_pcmbWingColor;

        QPushButton *m_ppbResetMesh;
        QPushButton *m_ppbInsertBefore, *m_ppbInsertAfter, *m_ppbDeleteSection;
        QLabel *m_plabWingSpan, *m_plabWingArea, *m_plabMAC, *m_plabGeomChord;
        QLabel *m_plabAspectRatio, *m_plabTaperRatio, *m_plabSweep, *m_plabNFlaps;
        QLabel *m_plabVLMPanels, *m_plab3DPanels;
        QLabel *m_plabProjectedArea, *m_plabProjectedSpan;

        QTableView *m_ptvWingSections;
        QStandardItemModel *m_pWingModel;
        WingDelegate *m_pWingDelegate;

        QDialogButtonBox *m_pButtonBox;

        QCheckBox *m_pchAxes, *m_pchSurfaces, *m_pchOutline, *m_pchPanels, *m_pchFoilNames;
        QCheckBox *m_pchShowMasses;
        QPushButton *m_ppbReset;

        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ, *m_ptbIso, *m_ptbFlip;
        QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;

        QAction *m_pScaleWing, *m_pInertia;
        QAction *m_pExportWingAct, *m_pImportWingAct;
        QAction *m_pExportWingXml, *m_pImportWingXml;

        QAction *m_pResetScales;

        QMenu *m_pContextMenu;
        QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteSection, *m_pResetSection;

        QSplitter *m_pspLeftSide;
        QSplitter *m_pspHorizontal;

        static QByteArray s_LeftSplitterSizes;
        static QByteArray s_HSplitterSizes;


        Wing *m_pWing;

        bool m_bAcceptName;
        bool m_bRightSide;
        bool m_bChanged, m_bDescriptionChanged;
        bool m_bTrans;
        bool m_bStored;
        bool m_bEnableName;


        int m_iSection;


        QPoint m_MousePos;
        QPoint m_ptPopUp;
        QPoint m_LastPoint;


        Vector3d m_RealPopUp;

        Panel *m_pPanel;
        Vector3d *m_pNode;
};

