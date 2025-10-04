/****************************************************************************

    GL3dBodyDlg Class
    Copyright (C) 2009-2019 André Deperrois

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
#include <QRadioButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QPushButton>
#include <QToolButton>
#include <QSplitter>
#include <QSettings>
#include <QPixmap>
#include <QDialogButtonBox>

#include <xfl3d/views/gl3dbodyview.h>

class ColorMenuBtn;
class DoubleEdit;
class BodyTableDelegate;
class BodyGridDlg;
class Body;
class BodyLineWt;
class BodyFrameWt;
class Frame;


class BodyDlg : public QDialog
{
    Q_OBJECT
    friend class MainFrame;
    friend class QSail7;
    friend class Miarex;
    friend class GLLightDlg;
    friend class BodyScaleDlg;
    friend class BoatDlg;
    friend class PlaneDlg;
    friend class gl3dBodyView;

    public:
        BodyDlg(QWidget *pParent=nullptr);
        ~BodyDlg();

    private slots:
        void onBodyColor(QColor clr);
        void onBodyInertia();
        void onBodyName();
        void onButton(QAbstractButton *pButton);
        void onCheckViewIcons();
        void onEdgeWeight();
        void onExportBodyDef();
        void onExportBodyGeom();
        void onExportBodyXML();
        void onFrameCellChanged(QWidget *);
        void onFrameClicked();
        void onFrameItemClicked(const QModelIndex &index);

        void onImportBodyDef() ;
        void onImportBodyXML();
        void onLineType();
        void onNURBSPanels();
        void onPointCellChanged(QWidget *);
        void onPointClicked();
        void onPointItemClicked(const QModelIndex &index);
        void onRedo();
        void onResetScales();
        void onScaleBody();
        void onSelChangeHoopDegree(int sel);
        void onSelChangeXDegree(int sel);
        void onTranslateBody();
        void onUndo();
        void onUpdateBody();

    private:
        void keyPressEvent(QKeyEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void accept() override;
        void reject() override;

        void blockSignalling(bool bBlock);
        void connectSignals();

        void fillFrameTableRow(int row);
        void fillFrameDataTable();
        void fillFrameCell(int iItem, int iSubItem);
        void readFrameSectionData(int sel);

        void fillPointCell(int iItem, int iSubItem);
        void fillPointTableRow(int row);
        void fillPointDataTable();
        void readPointSectionData(int sel);

        void setFrame(int iFrame);
        void setFrame(Frame *pFrame);

        void setupLayout();
        void setViewControls();
        void setTableUnits();

        void setControls();
        void updateView();

        bool initDialog(Body *pBody);
        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

        void resizeTables();

    private:
        bool setBody(Body *pBody=nullptr);

        void clearStack(int pos=0);
        void setPicture();
        void takePicture();



    private:
        gl3dBodyView m_gl3dBodyview;
        BodyLineWt *m_pBodyLineWt;
        BodyFrameWt *m_pFrameWt;

        static QByteArray s_WindowGeometry;

        static bool s_bOutline;
        static bool s_bSurfaces;
        static bool s_bVLMPanels;
        static bool s_bAxes;
        static bool s_bShowMasses;
        static bool s_bFoilNames;


        QWidget *m_pwtControls;

        QSplitter *m_pLeftSplitter, *m_pHorizontalSplitter, *m_pVerticalSplitter;
        static QByteArray m_VerticalSplitterSizes, m_HorizontalSplitterSizes, m_LeftSplitterSizes;

        QCheckBox *m_pchAxes, *m_pchSurfaces, *m_pchOutline, *m_pchPanels, *m_pchShowMasses;
        QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ, *m_ptbIso, *m_ptbFlip;
        QPushButton *m_ppbReset;
        QPushButton *m_ppbUndo, *m_ppbRedo;

        QDialogButtonBox *m_pButtonBox;

        QSlider *m_pslEdgeWeight;
        QSlider *m_pslPanelBunch;

        QLineEdit *m_pleBodyName;
        QTextEdit *m_pteBodyDescription;

        QRadioButton *m_prbFlatPanels, *m_prbBSplines;
        ColorMenuBtn *m_pcbBodyColor;
        DoubleEdit *m_pdeNXPanels, *m_pdeNHoopPanels;
        QComboBox *m_pcbXDegree, *m_pcbHoopDegree;
        QPushButton *m_ppbMenuButton;
        QMenu *BodyMenu;

        QTableView *m_ptvFrames, *m_ptvPoints;
        QStandardItemModel *m_pFrameModel, *m_pPointModel;
        BodyTableDelegate *m_pFrameDelegate, *m_pPointDelegate;
        QItemSelectionModel *m_pSelectionModelPoint, *m_pSelectionModelFrame;

        QAction *m_pScaleBody;
        QAction *m_pResetScales;
        QAction *m_pUndo, *m_pRedo;
        QAction *m_pExportBodyDef, *m_pImportBodyDef, *m_pExportBodyGeom, *m_pTranslateBody, *m_pBodyInertia;// *m_pSetupLight;
        QAction *m_pExportBodyXML, *m_pImportBodyXML;

        int m_StackPos;                /**< the current position on the Undo stack */
        QVector<Body*> m_UndoStack;      /**< the stack of incremental modifications to the SplineFoil;
                                     we can't use the QStack though, because we need to access
                                     any point in the case of multiple undo operations */

        //    bool m_bStored;
        bool m_bResetFrame;
        bool m_bChanged;


        Frame *m_pFrame;
        Body *m_pBody;


        bool m_bEnableName;
};

