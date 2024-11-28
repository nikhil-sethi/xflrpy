/****************************************************************************

    EditBodyDlg Class
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
#include <QPixmap>
#include <QSettings>
#include <QFile>
#include <QSplitter>
#include <QTreeView>
#include <QStandardItemModel>
#include <QToolButton>
#include <QList>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/core_enums.h>

class gl3dBodyView;
class Body;
class BodyLineWt;
class BodyFrameWt;
class EditObjectDelegate;
class Section2dWt;
class PointMass;
class Frame;

class EditBodyDlg : public QDialog
{
    Q_OBJECT

    friend class Wing;
    friend class Plane;
    friend class Miarex;
    friend class gl3dBodyView;
    friend class gl3dView;

    public:
        EditBodyDlg(QWidget *pParent = nullptr);

        bool intersectObject(Vector3d AA,  Vector3d U, Vector3d &I);
        void connectSignals();
        void fillBodyTreeView();
        void identifySelection(const QModelIndex &indexSel);
        void initDialog(Body *pBody);
        void readBodyFrameTree(Frame *pFrame, QModelIndex indexLevel);
        void readBodyTree(QModelIndex indexLevel);
        void readInertiaTree(double &volumeMass, QVector<PointMass*> &pointmasses, QModelIndex indexlevel);
        void readInertiaTree(double &volumeMass, QVector<PointMass> &pointMasses, QModelIndex indexLevel);
        void readPointMassTree(PointMass *ppm, QModelIndex indexLevel);
        void readVectorTree(Vector3d &V, QModelIndex indexLevel);
        void resizeTreeView();
        void setActiveFrame(int iFrame);
        void setupLayout();
        void updateViews();

        QList<QStandardItem *> prepareRow(const QString &first, const QString &second="", const QString &third="",  const QString &fourth="");
        QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
        QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
        QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);

        static bool loadSettings(QSettings &settings);
        static bool saveSettings(QSettings &settings);

    private slots:
        void onRedraw();
        void onRefillBodyTree();
        void onItemClicked(const QModelIndex &index);
        void onResize();
        void onCheckViewIcons();

        void onInsertBefore();
        void onInsertAfter();
        void onDelete();
        void onExportBodyGeom();
        void onExportBodyXML();
        void onImportBodyXML();
        void onScaleBody();
        void onTranslateBody();
        void onBodyInertia();
        void onButton(QAbstractButton *pButton);

    private:
        void showEvent(QShowEvent *event) override;
        void hideEvent(QHideEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void contextMenuEvent(QContextMenuEvent *event) override;
        void accept() override;
        void reject() override;
        void paintBodyLegend(QPainter &painter);
        void createActions();

    public:
        static QByteArray s_WindowGeometry;
        static QByteArray m_HSplitterSizes;

        QPixmap m_PixText;

    private:
        Body *m_pBody;

        static bool s_bOutline;
        static bool s_bSurfaces;
        static bool s_bVLMPanels;
        static bool s_bAxes;
        static bool s_bShowMasses;
        static bool s_bFoilNames;

        QTreeView * m_pStruct;
        EditObjectDelegate *m_pDelegate;
        QStandardItemModel *m_pModel;

        gl3dBodyView *m_pglBodyView;
        BodyLineWt *m_pBodyLineWidget;
        BodyFrameWt *m_pFrameWidget;

        QCheckBox *m_pchAxes, *m_pchLight, *m_pchSurfaces, *m_pchOutline, *m_pchPanels;
        QCheckBox *m_pchShowMasses;

        QAction *m_pExportBodyGeom, *m_pBodyInertia;// *m_pSetupLight;
        QAction *m_pExportBodyXML, *m_pImportBodyXML;
        QAction *m_pScaleBody, *m_pTranslateBody;


        QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
        QToolButton *m_ptbX, *m_ptbY, *m_ptbZ, *m_ptbIso, *m_ptbFlip;

        QMenu *m_pContextMenu;
        QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteItem;

        QSplitter *m_pHorizontalSplitter, *m_pLeftSplitter, *m_pMiddleSplitter;

        QPushButton *m_ppbRedraw, *m_ppbMenu;
        QPushButton *m_ppbReset;

        QDialogButtonBox *m_pButtonBox;

        bool m_bChanged;


        xfl::enumWingType m_enumActiveWingType;
        int m_iActivePointMass;
};

