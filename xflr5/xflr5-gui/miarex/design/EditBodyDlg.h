/****************************************************************************

    EditBodyDlg Class
    Copyright (C) 2015-2019 Andre Deperrois

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


#ifndef EDITBODYDLG_H
#define EDITBODYDLG_H

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

#include <objects/objects3d/vector3d.h>
#include <analysis3d/analysis3d_enums.h>

class gl3dBodyView;
class Body;
class BodyLineWt;
class BodyFrameWt;
class EditObjectDelegate;
class Section2dWidget;
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

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);


    bool intersectObject(Vector3d AA,  Vector3d U, Vector3d &I);
    void connectSignals();
    void identifySelection(const QModelIndex &indexSel);
    void initDialog(Body *pBody);
    void setupLayout();
    void fillBodyTreeView();
    void readBodyTree(QModelIndex indexLevel);
    void readInertiaTree(double &volumeMass, QList<PointMass *> &pointMasses, QModelIndex indexLevel);
    void readVectorTree(Vector3d &V, QModelIndex indexLevel);
    void readPointMassTree(PointMass *ppm, QModelIndex indexLevel);
    void readBodyFrameTree(Frame *pFrame, QModelIndex indexLevel);
    void resizeTreeView();
    void setActiveFrame(int iFrame);
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
    void accept();
    void reject();
    void paintBodyLegend(QPainter &painter);
    void createActions();

public:
    static bool s_bWindowMaximized;
    static QPoint s_WindowPosition;   /**< the position on the client area of the dialog's topleft corner */
    static QSize s_WindowSize;	 /**< the window size in the client area */
    static QByteArray m_HorizontalSplitterSizes;

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

    QCheckBox *m_pctrlAxes, *m_pctrlLight, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels;
    QCheckBox *m_pctrlShowMasses;

    QAction *m_pExportBodyGeom, *m_pBodyInertia;// *m_pSetupLight;
    QAction *m_pExportBodyXML, *m_pImportBodyXML;
    QAction *m_pScaleBody, *m_pTranslateBody;


    QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
    QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso, *m_pctrlFlip;

    QMenu *m_pContextMenu;
    QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteItem;

    QSplitter *m_pHorizontalSplitter, *m_pLeftSplitter, *m_pMiddleSplitter;

    QPushButton *m_pctrlRedraw, *m_pctrlMenuButton;
    QPushButton *m_pctrlReset;

    QDialogButtonBox *m_pButtonBox;

    bool m_bChanged;


    XFLR5::enumWingType m_enumActiveWingType;
    int m_iActivePointMass;
};

#endif // EDITBODYDLG_H
