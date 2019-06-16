/****************************************************************************

    EditPlaneDlg Class
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


#ifndef EDITPLANEOBJECTDLG_H
#define EDITPLANEOBJECTDLG_H

#include <QDialog>
#include <QSettings>
#include <QPixmap>
#include <QSplitter>
#include <QTreeView>
#include <QStandardItemModel>
#include <QToolButton>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <objects/objects3d/vector3d.h>
#include <analysis3d/analysis3d_enums.h>

class Plane;
class Body;
class Wing;
class PointMass;
class Frame;
class gl3dPlaneView;
class EditObjectDelegate;

typedef enum {PLANE, BODY, WING, NOOBJECT} enumObjectType;


class EditPlaneDlg : public QDialog
{
    Q_OBJECT

    friend class Wing;
    friend class Plane;
    friend class Miarex;
    friend class gl3dPlaneView;
    friend class gl3dView;

public:
    EditPlaneDlg(QWidget *pParent = nullptr);

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *pEvent);
    void contextMenuEvent(QContextMenuEvent *event);

    bool intersectObject(Vector3d AA,  Vector3d U, Vector3d &I);
    void connectSignals();
    void glMake3DObjects();
    void identifySelection(const QModelIndex &indexSel);
    void initDialog(Plane *pPlane);
    void setupLayout();
    void fillPlaneTreeView();
    void fillWingTreeView(int iw, QList<QStandardItem *> &planeRootItem);
    void fillBodyTreeView(QStandardItem *planeRootItem);
    void fillPlaneMetaData(QStandardItem *item);
    void readPlaneTree();
    void readViewLevel(QModelIndex indexLevel);
    void readBodyTree(Body *pBody, QModelIndex indexLevel);
    void readWingTree(Wing *pWing, Vector3d &wingLE, double &tiltAngle, QModelIndex indexLevel);
    void readInertiaTree(double &volumeMass, QList<PointMass *> &pointMasses, QModelIndex indexLevel);
    void readVectorTree(Vector3d &V, QModelIndex indexLevel);
    void readWingSectionTree(Wing *pWing, QModelIndex indexLevel);
    void readPointMassTree(PointMass *ppm, QModelIndex indexLevel);
    void readBodyFrameTree(Body *pBody, Frame *pFrame, QModelIndex indexLevel);
    void resize3DView();
    void resizeTreeView();

    QList<QStandardItem *> prepareRow(const QString &first, const QString &second="", const QString &third="",  const QString &fourth="");
    QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
    QList<QStandardItem *> prepareIntRow(const QString &first, const QString &second, const int &third);
    QList<QStandardItem *> prepareDoubleRow(const QString &first, const QString &second, const double &third,  const QString &fourth);


    static bool loadSettings(QSettings &settings);
    static bool saveSettings(QSettings &settings);

private slots:
    void on3DReset();
    void onAutoRedraw();
    void onCheckViewIcons();
    void onDelete();
    void onEndEdit();
    void onInsertBefore();
    void onInsertAfter();
    void onItemClicked(const QModelIndex &index);
    void onOK();
    void onRedraw();
    void onResize();
    void onButton(QAbstractButton *pButton);

private:
    void accept();
    void reject();
    void paintPlaneLegend(QPainter &painter, Plane *pPlane, QRect drawRect);

public:
    static bool s_bWindowMaximized;
    static QPoint s_WindowPosition;   /**< the position on the client area of the dialog's topleft corner */
    static QSize s_WindowSize;	 /**< the window size in the client area */
    static QByteArray m_HorizontalSplitterSizes;

    QPixmap m_PixText;

private:
    Plane * m_pPlane;
    QTreeView * m_pStruct;
    EditObjectDelegate *m_pDelegate;
    QStandardItemModel *m_pModel;

    //	ThreeDWidget *m_pgl1Widget;
    gl3dPlaneView *m_pglPlaneView;

    QCheckBox *m_pctrlAxes, *m_pctrlLight, *m_pctrlSurfaces, *m_pctrlOutline, *m_pctrlPanels;
    QCheckBox *m_pctrlFoilNames, *m_pctrlShowMasses;

    QAction *m_pXView, *m_pYView, *m_pZView, *m_pIsoView, *m_pFlipView;
    QToolButton *m_pctrlX, *m_pctrlY, *m_pctrlZ, *m_pctrlIso, *m_pctrlFlip;

    QMenu *m_pContextMenu;
    QAction *m_pInsertBefore, *m_pInsertAfter, *m_pDeleteItem;

    QSplitter *m_pHorizontalSplitter, *m_pLeftSideSplitter, *m_pRightSideSplitter;

    QCheckBox *m_pctrlAutoRedraw;
    QPushButton *m_pctrlRedraw;
    QPushButton *m_pctrlReset;
    QDialogButtonBox *m_pButtonBox;
    QSlider *m_pctrlClipPlanePos;

    bool m_bChanged;
    bool m_bResetglSectionHighlight;
    bool m_bResetglPlane, m_bResetglBody;
    static bool s_bAutoRedraw;

    static bool s_bOutline;
    static bool s_bSurfaces;
    static bool s_bVLMPanels;
    static bool s_bAxes;
    static bool s_bShowMasses;
    static bool s_bFoilNames;

    enumObjectType m_enumActiveObject;
    XFLR5::enumWingType m_enumActiveWingType;
    int m_iActiveSection, m_iActiveFrame, m_iActivePointMass;
};

#endif // EDITPLANEOBJECTDLG_H
