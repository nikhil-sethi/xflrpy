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

#include <QHeaderView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QPainter>
#include <QFileDialog>
#include <QShowEvent>


#include <globals/mainframe.h>

#include <xfl3d/controls/w3dprefs.h>
#include <xfl3d/views/gl3dbodyview.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/editors/bodyframewt.h>
#include <xflobjects/editors/bodylinewt.h>
#include <xflobjects/editors/bodyscaledlg.h>
#include <xflobjects/editors/bodytransdlg.h>
#include <xflobjects/editors/editbodydlg.h>
#include <xflobjects/editors/editobjectdelegate.h>
#include <xflobjects/editors/inertiadlg.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/xml/xmlplanereader.h>
#include <xflobjects/xml/xmlplanewriter.h>



QByteArray EditBodyDlg::s_WindowGeometry;
QByteArray EditBodyDlg::m_HSplitterSizes;


bool EditBodyDlg::s_bOutline    = true;
bool EditBodyDlg::s_bSurfaces   = true;
bool EditBodyDlg::s_bVLMPanels  = false;
bool EditBodyDlg::s_bAxes       = true;
bool EditBodyDlg::s_bShowMasses = false;
bool EditBodyDlg::s_bFoilNames  = false;


EditBodyDlg::EditBodyDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Body object explorer");
    setWindowFlags(Qt::Window);

    m_pStruct = nullptr;
    m_pDelegate = nullptr;
    m_pModel = nullptr;

    m_enumActiveWingType = xfl::OTHERWING;
    m_iActivePointMass = -1;

    m_bChanged                 = false;

    m_pInsertBefore  = new QAction(tr("Insert before"), this);
    m_pInsertAfter   = new QAction(tr("Insert after"), this);
    m_pDeleteItem = new QAction(tr("Delete"), this);

    m_pContextMenu = new QMenu(tr("Section"),this);
    m_pContextMenu->addAction(m_pInsertBefore);
    m_pContextMenu->addAction(m_pInsertAfter);
    m_pContextMenu->addAction(m_pDeleteItem);

    m_PixText = QPixmap(107, 97);
    m_PixText.fill(Qt::transparent);

    createActions();
    setupLayout();
}


void EditBodyDlg::createActions()
{
    m_pScaleBody        = new QAction(tr("Scale"), this);
    connect(m_pScaleBody,        SIGNAL(triggered()), this, SLOT(onScaleBody()));

    m_pExportBodyGeom = new QAction(tr("Export Body Geometry to text File"), this);
    connect(m_pExportBodyGeom, SIGNAL(triggered()), this, SLOT(onExportBodyGeom()));

    m_pExportBodyXML= new QAction(tr("Export body definition to an XML file"), this);
    connect(m_pExportBodyXML, SIGNAL(triggered()), this, SLOT(onExportBodyXML()));


    m_pImportBodyXML= new QAction(tr("Import body definition from an XML file"), this);
    connect(m_pImportBodyXML, SIGNAL(triggered()), this, SLOT(onImportBodyXML()));

    m_pBodyInertia = new QAction(tr("Define Inertia")+"\tF12", this);
    connect(m_pBodyInertia, SIGNAL(triggered()), this, SLOT(onBodyInertia()));

    m_pTranslateBody = new QAction(tr("Translate"), this);
    connect(m_pTranslateBody, SIGNAL(triggered()), this, SLOT(onTranslateBody()));
}

/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void EditBodyDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_WindowGeometry);
    if(m_HSplitterSizes.length()>0)
        m_pHorizontalSplitter->restoreState(m_HSplitterSizes);
    resizeTreeView();


    m_pglBodyView->update();

    pEvent->accept();
}



/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void EditBodyDlg::hideEvent(QHideEvent *pEvent)
{
    m_HSplitterSizes  = m_pHorizontalSplitter->saveState();
    //    m_LeftSplitterSizes        = m_pLeftSplitter->saveState();
    //    m_MiddleSplitterSizes      = m_pMiddleSplitter->saveState();
    s_WindowGeometry = saveGeometry();
    pEvent->accept();
}


void EditBodyDlg::resizeEvent(QResizeEvent *pEvent)
{
    resizeTreeView();
    if(m_pglBodyView->width()>0 && m_pglBodyView->height()>0)
    {
        m_PixText = m_PixText.scaled(m_pglBodyView->rect().size());
        m_PixText.fill(Qt::transparent);
    }
    pEvent->accept();
}


void EditBodyDlg::onResize()
{
    resizeTreeView();
    //    resize3DView();
}


void EditBodyDlg::resizeTreeView()
{
    QList<int> leftSizes;
    leftSizes.append(int(height()*95/100));
    leftSizes.append(int(height()*5/100));
    m_pLeftSplitter->setSizes(leftSizes);

    QList<int> midlleSizes;
    midlleSizes.append(int(height()*45/100));
    midlleSizes.append(int(height()*45/100));
    midlleSizes.append(int(height()*5/100));
    m_pMiddleSplitter->setSizes(midlleSizes);


    int ColumnWidth = int(double(m_pStruct->width())/15);
    m_pStruct->setColumnWidth(0,ColumnWidth*6);
    m_pStruct->setColumnWidth(1,ColumnWidth*3);
    m_pStruct->setColumnWidth(2,ColumnWidth*3);
}


void EditBodyDlg::contextMenuEvent(QContextMenuEvent *event)
{
    // Display the context menu

    if(!m_pBody->activeFrame() && m_iActivePointMass<0 ) return;
    if(m_pBody->activeFrame())
    {
        m_pInsertBefore->setText(tr("Insert body frame before"));
        m_pInsertAfter->setText(tr("Insert body frame after"));
        m_pDeleteItem->setText(tr("Delete body frame"));
    }
    else if(m_iActivePointMass>=0)
    {
        m_pInsertBefore->setText(tr("Insert point mass before"));
        m_pInsertAfter->setText(tr("Insert point mass after"));
        m_pDeleteItem->setText(tr("Delete point Mass"));
    }

    if(m_pStruct->geometry().contains(event->pos())) m_pContextMenu->exec(event->globalPos());
}


void EditBodyDlg::setupLayout()
{
    QStringList labels;
    labels << tr("Object") << tr("Field")<<tr("Value")<<tr("Unit");

    m_pStruct = new QTreeView;

#if QT_VERSION >= 0x050000
    m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif

    //    m_pPlaneStruct->header()->setDefaultSectionSize(239);
    m_pStruct->header()->setStretchLastSection(true);
    m_pStruct->header()->setDefaultAlignment(Qt::AlignCenter);

    m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
    //    m_pStruct->setIndentation(31);
    m_pStruct->setWindowTitle(tr("Objects"));

    m_pModel = new QStandardItemModel(this);
    m_pModel->setColumnCount(4);
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(labels);
    m_pStruct->setModel(m_pModel);


    QFont fnt;
    QFontMetrics fm(fnt);
    m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
    m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
    m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);


    QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
    m_pStruct->setSelectionModel(selectionModel);
    connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


    m_pDelegate = new EditObjectDelegate(this);
    m_pStruct->setItemDelegate(m_pDelegate);
    connect(m_pDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onRedraw()));


    QSizePolicy szPolicyMinimumExpanding;
    szPolicyMinimumExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyMinimumExpanding.setVerticalPolicy(QSizePolicy::MinimumExpanding);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pLeftSplitter = new QSplitter(Qt::Vertical, this);;
        {
            m_pStruct->setSizePolicy(szPolicyMinimumExpanding);

            m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
            {
                m_ppbRedraw = new QPushButton(tr("Regenerate") + "\t(F4)");

                m_ppbMenu = new QPushButton(tr("Actions"));

                QMenu *pBodyMenu = new QMenu(tr("Actions..."),this);

                pBodyMenu->addAction(m_pImportBodyXML);
                pBodyMenu->addAction(m_pExportBodyXML);
                pBodyMenu->addSeparator();
                pBodyMenu->addAction(m_pExportBodyGeom);
                pBodyMenu->addSeparator();
                pBodyMenu->addAction(m_pBodyInertia);
                pBodyMenu->addSeparator();
                pBodyMenu->addAction(m_pTranslateBody);
                pBodyMenu->addAction(m_pScaleBody);
                pBodyMenu->addSeparator();
                m_ppbMenu->setMenu(pBodyMenu);
                m_pButtonBox->addButton(m_ppbRedraw, QDialogButtonBox::ActionRole);
                m_pButtonBox->addButton(m_ppbMenu, QDialogButtonBox::ActionRole);
                connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
            }


            m_pLeftSplitter->addWidget(m_pStruct);
            m_pLeftSplitter->addWidget(m_pButtonBox);
        }


        m_pMiddleSplitter = new QSplitter(Qt::Vertical, this);
        {
            QWidget *p3DCtrlBox = new QWidget;
            {
                p3DCtrlBox->setSizePolicy(szPolicyMaximum);
                QHBoxLayout *pThreeDViewControlsLayout = new QHBoxLayout;
                {
                    QGridLayout *pThreeDParamsLayout = new QGridLayout;
                    {
                        m_pchAxes         = new QCheckBox(tr("Axes"), this);
                        m_pchSurfaces     = new QCheckBox(tr("Surfaces"), this);
                        m_pchOutline      = new QCheckBox(tr("Outline"), this);
                        m_pchPanels       = new QCheckBox(tr("Panels"), this);
                        m_pchShowMasses   = new QCheckBox(tr("Masses"), this);

                        m_pchAxes->setSizePolicy(szPolicyMaximum);
                        m_pchSurfaces->setSizePolicy(szPolicyMaximum);
                        m_pchOutline->setSizePolicy(szPolicyMaximum);
                        m_pchPanels->setSizePolicy(szPolicyMaximum);
                        m_pchShowMasses->setSizePolicy(szPolicyMaximum);

                        pThreeDParamsLayout->addWidget(m_pchAxes, 1,1);
                        pThreeDParamsLayout->addWidget(m_pchPanels, 1,3);
                        pThreeDParamsLayout->addWidget(m_pchSurfaces, 1,2);
                        pThreeDParamsLayout->addWidget(m_pchOutline, 2,2);
                        pThreeDParamsLayout->addWidget(m_pchShowMasses, 2,3);
                    }

                    QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
                    {
                        m_ptbX          = new QToolButton;
                        m_ptbY          = new QToolButton;
                        m_ptbZ          = new QToolButton;
                        m_ptbIso        = new QToolButton;
                        m_ptbFlip       = new QToolButton;
                        int iconSize =32;
                        if(m_ptbX->iconSize().height()<=iconSize)
                        {
                            m_ptbX->setIconSize(QSize(iconSize,iconSize));
                            m_ptbY->setIconSize(QSize(iconSize,iconSize));
                            m_ptbZ->setIconSize(QSize(iconSize,iconSize));
                            m_ptbIso->setIconSize(QSize(iconSize,iconSize));
                            m_ptbFlip->setIconSize(QSize(iconSize,iconSize));
                        }
                        m_pXView    = new QAction(QIcon(":/images/OnXView.png"), tr("X View"), this);
                        m_pYView    = new QAction(QIcon(":/images/OnYView.png"), tr("Y View"), this);
                        m_pZView    = new QAction(QIcon(":/images/OnZView.png"), tr("Z View"), this);
                        m_pIsoView  = new QAction(QIcon(":/images/OnIsoView.png"), tr("Iso View"), this);
                        m_pFlipView = new QAction(QIcon(":/images/OnFlipView.png"), tr("Flip View"), this);
                        m_pXView->setCheckable(true);
                        m_pYView->setCheckable(true);
                        m_pZView->setCheckable(true);
                        m_pIsoView->setCheckable(true);

                        m_ptbX->setDefaultAction(m_pXView);
                        m_ptbY->setDefaultAction(m_pYView);
                        m_ptbZ->setDefaultAction(m_pZView);
                        m_ptbIso->setDefaultAction(m_pIsoView);
                        m_ptbFlip->setDefaultAction(m_pFlipView);
                        pAxisViewLayout->addWidget(m_ptbX);
                        pAxisViewLayout->addWidget(m_ptbY);
                        pAxisViewLayout->addWidget(m_ptbZ);
                        pAxisViewLayout->addWidget(m_ptbIso);
                        pAxisViewLayout->addWidget(m_ptbFlip);
                    }

                    m_ppbReset = new QPushButton(tr("Reset scale"));
                    pThreeDViewControlsLayout->addLayout(pThreeDParamsLayout);
                    pThreeDViewControlsLayout->addStretch();
                    pThreeDViewControlsLayout->addLayout(pAxisViewLayout);
                    pThreeDViewControlsLayout->addStretch();
                    pThreeDViewControlsLayout->addWidget(m_ppbReset);

                }
                p3DCtrlBox->setLayout(pThreeDViewControlsLayout);
            }
            m_pBodyLineWidget = new BodyLineWt(this);
            m_pBodyLineWidget->setSizePolicy(szPolicyMaximum);
            m_pBodyLineWidget->sizePolicy().setVerticalStretch(2);

            m_pglBodyView = new gl3dBodyView(this);
            m_pglBodyView->m_bOutline    = s_bOutline;
            m_pglBodyView->m_bSurfaces   = s_bSurfaces;
            m_pglBodyView->m_bVLMPanels  = s_bVLMPanels;
            m_pglBodyView->m_bAxes       = s_bAxes;
            m_pglBodyView->m_bShowMasses = s_bShowMasses;
            m_pglBodyView->m_bFoilNames  = s_bFoilNames;

            m_pglBodyView->sizePolicy().setVerticalStretch(5);
            p3DCtrlBox->sizePolicy().setVerticalStretch(2);

            m_pMiddleSplitter->addWidget(m_pBodyLineWidget);
            m_pMiddleSplitter->addWidget(m_pglBodyView);
            m_pMiddleSplitter->addWidget(p3DCtrlBox);
        }

        m_pFrameWidget = new BodyFrameWt(this);

        m_pHorizontalSplitter->addWidget(m_pLeftSplitter);
        m_pHorizontalSplitter->addWidget(m_pMiddleSplitter);
        m_pHorizontalSplitter->addWidget(m_pFrameWidget);

        QList<int> horizontalSizes;
        horizontalSizes.append(30);
        horizontalSizes.append(60);
        horizontalSizes.append(10);
        m_pHorizontalSplitter->setSizes(horizontalSizes);

        QList<int> leftSplitterSizes;
        leftSplitterSizes.append(95);
        leftSplitterSizes.append( 5);
        m_pLeftSplitter->setSizes(leftSplitterSizes);

        QList<int> middleSplitterSizes;
        middleSplitterSizes.append(30);
        middleSplitterSizes.append(60);
        middleSplitterSizes.append(5);
        m_pMiddleSplitter->setSizes(middleSplitterSizes);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        pMainLayout->addWidget(m_pHorizontalSplitter);
    }
    setLayout(pMainLayout);
    connectSignals();
    //    resize(s_Size);
}


void EditBodyDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)          accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
    else if(m_ppbRedraw==pButton) onRedraw();
}


void EditBodyDlg::initDialog(Body *pBody)
{
    m_pBody = pBody;
    m_pBodyLineWidget->setBody(pBody);
    m_pFrameWidget->setBody(pBody);
    m_pglBodyView->setBody(m_pBody);
    m_pglBodyView->setScale(pBody->length());

    fillBodyTreeView();

    m_pchSurfaces->setChecked(m_pglBodyView->m_bSurfaces);
    m_pchOutline->setChecked(m_pglBodyView->m_bOutline);
    m_pchAxes->setChecked(m_pglBodyView->m_bAxes);
    m_pchPanels->setChecked(m_pglBodyView->m_bVLMPanels);
    m_pchShowMasses->setChecked(m_pglBodyView->m_bShowMasses);
}




void EditBodyDlg::keyPressEvent(QKeyEvent *event)
{
    //    bool bShift = false;
    //    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    //    if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else accept();

            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_F4:
        {
            onRedraw();
            return;
        }

        default:
            event->ignore();
    }
}


void EditBodyDlg::accept()
{
    s_bOutline    = m_pglBodyView->m_bOutline;
    s_bSurfaces   = m_pglBodyView->m_bSurfaces;
    s_bVLMPanels  = m_pglBodyView->m_bVLMPanels;
    s_bAxes       = m_pglBodyView->m_bAxes;
    s_bShowMasses = m_pglBodyView->m_bShowMasses;
    s_bFoilNames  = m_pglBodyView->m_bFoilNames;

    done(QDialog::Accepted);
}


void EditBodyDlg::reject()
{
    if(m_bChanged)
    {
        QString strong = tr("Save the changes ?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            accept();
            return;
        }
        else if(QMessageBox::Cancel == Ans) return;
    }


    s_bOutline    = m_pglBodyView->m_bOutline;
    s_bSurfaces   = m_pglBodyView->m_bSurfaces;
    s_bVLMPanels  = m_pglBodyView->m_bVLMPanels;
    s_bAxes       = m_pglBodyView->m_bAxes;
    s_bShowMasses = m_pglBodyView->m_bShowMasses;
    s_bFoilNames  = m_pglBodyView->m_bFoilNames;

    done(QDialog::Rejected);
}


void EditBodyDlg::connectSignals()
{
    connect(m_pBodyLineWidget, SIGNAL(objectModified()), SLOT(onRefillBodyTree()));
    connect(m_pFrameWidget,    SIGNAL(objectModified()), SLOT(onRefillBodyTree()));

    connect(m_pInsertBefore,   SIGNAL(triggered()), SLOT(onInsertBefore()));
    connect(m_pInsertAfter,    SIGNAL(triggered()), SLOT(onInsertAfter()));
    connect(m_pDeleteItem,     SIGNAL(triggered()), SLOT(onDelete()));

    connect(m_ppbRedraw,     SIGNAL(clicked()), SLOT(onRedraw()));

    connect(m_ppbReset,      SIGNAL(clicked()), m_pglBodyView, SLOT(on3DReset()));

    connect(m_pchAxes,       SIGNAL(clicked(bool)), m_pglBodyView, SLOT(onAxes(bool)));
    connect(m_pchPanels,     SIGNAL(clicked(bool)), m_pglBodyView, SLOT(onPanels(bool)));
    connect(m_pchSurfaces,   SIGNAL(clicked(bool)), m_pglBodyView, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,    SIGNAL(clicked(bool)), m_pglBodyView, SLOT(onOutline(bool)));
    connect(m_pchShowMasses, SIGNAL(clicked(bool)), m_pglBodyView, SLOT(onShowMasses(bool)));

    connect(m_ptbIso,        SIGNAL(clicked()), m_pglBodyView, SLOT(on3DIso()));
    connect(m_ptbX,          SIGNAL(clicked()), m_pglBodyView, SLOT(on3DFront()));
    connect(m_ptbY,          SIGNAL(clicked()), m_pglBodyView, SLOT(on3DLeft()));
    connect(m_ptbZ,          SIGNAL(clicked()), m_pglBodyView, SLOT(on3DTop()));
    connect(m_ptbFlip,       SIGNAL(clicked()), m_pglBodyView, SLOT(on3DFlip()));

    connect(m_pHorizontalSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onResize()));

    connect(m_pglBodyView, SIGNAL(viewModified()), SLOT(onCheckViewIcons()));
}


/**
 * Unselects all the 3D-view icons.
 */
void EditBodyDlg::onCheckViewIcons()
{
    m_ptbIso->setChecked(false);
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
}



bool EditBodyDlg::intersectObject(Vector3d AA,  Vector3d U, Vector3d &I)
{
    return m_pBody->intersectFlatPanels(AA, AA+U*10, I);
}



QList<QStandardItem *> EditBodyDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
    for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> EditBodyDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::BOOLVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> EditBodyDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::INTEGER, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}


QList<QStandardItem *> EditBodyDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);

    return rowItems;
}





void EditBodyDlg::fillBodyTreeView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());


    QList<QStandardItem*> dataItem;
    QStandardItem *rootItem = m_pModel->invisibleRootItem();

    QModelIndex ind = m_pModel->index(0,0);
    m_pStruct->expand(ind);


    QList<QStandardItem*> bodyFolder = prepareRow("Body");
    rootItem->appendRow(bodyFolder);

    m_pStruct->expand(m_pModel->indexFromItem(bodyFolder.first()));

    dataItem = prepareRow("", "Name", m_pBody->name());
    bodyFolder.first()->appendRow(dataItem);

    dataItem = prepareRow("", "Type", bodyPanelType(m_pBody->bodyType()));
    dataItem.at(2)->setData(xfl::BODYTYPE, Qt::UserRole);
    bodyFolder.first()->appendRow(dataItem);

    QList<QStandardItem*> bodyColorFolder = prepareRow("Color");
    bodyFolder.first()->appendRow(bodyColorFolder);
    {
        QList<QStandardItem*> dataItem = prepareIntRow("", "red", m_pBody->color().red());
        bodyColorFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "green", m_pBody->color().green());
        bodyColorFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "blue", m_pBody->color().blue());
        bodyColorFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "alpha", m_pBody->color().alpha());
        bodyColorFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
    bodyFolder.first()->appendRow(bodyInertiaFolder);
    {
        if(m_iActivePointMass>=0) m_pStruct->expand(m_pModel->indexFromItem(bodyInertiaFolder.first()));

        QList<QStandardItem*> dataItem = prepareDoubleRow( "Structural (volume) mass", "", m_pBody->m_VolumeMass*Units::kgtoUnit(), Units::massUnitLabel());
        bodyInertiaFolder.first()->appendRow(dataItem);

        for(int iwm=0; iwm<m_pBody->m_PointMass.size(); iwm++)
        {
            PointMass const &pm = m_pBody->m_PointMass.at(iwm);
            QList<QStandardItem*> bodyPointMassFolder = prepareRow(QString("Point_mass_%1").arg(iwm+1));

            bodyInertiaFolder.first()->appendRow(bodyPointMassFolder);
            {
                if(m_iActivePointMass==iwm)
                {
                    m_pStruct->expand(m_pModel->indexFromItem(bodyPointMassFolder.first()));
                }
                QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm.tag());
                bodyPointMassFolder.first()->appendRow(dataItem);

                dataItem = prepareDoubleRow("", "mass", pm.mass()*Units::kgtoUnit(), Units::massUnitLabel());
                bodyPointMassFolder.first()->appendRow(dataItem);

                dataItem = prepareDoubleRow("", "x",pm.position().x*Units::mtoUnit(), Units::lengthUnitLabel());
                bodyPointMassFolder.first()->appendRow(dataItem);

                dataItem = prepareDoubleRow("", "y", pm.position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
                bodyPointMassFolder.first()->appendRow(dataItem);

                dataItem = prepareDoubleRow("", "z", pm.position().z*Units::mtoUnit(), Units::lengthUnitLabel());
                bodyPointMassFolder.first()->appendRow(dataItem);
            }
        }
    }

    QList<QStandardItem*> NURBSFolder = prepareRow("NURBS");
    bodyFolder.first()->appendRow(NURBSFolder);
    {
        QList<QStandardItem*> dataItem = prepareIntRow("", "NURBS degree (lengthwise)", m_pBody->splineSurface()->uDegree());
        NURBSFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "NURBS degree (hoop)", m_pBody->splineSurface()->vDegree());
        NURBSFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "Mesh panels (lengthwise)", m_pBody->m_nxPanels);
        NURBSFolder.first()->appendRow(dataItem);

        dataItem = prepareIntRow("", "Mesh panels (hoop)", m_pBody->m_nhPanels);
        NURBSFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> hoopFolder = prepareRow("Hoop_panels (FLATPANELS case)");
    bodyFolder.first()->appendRow(hoopFolder);
    {
        for(int isl=0; isl<m_pBody->sideLineCount(); isl++)
        {
            QList<QStandardItem*> dataItem = prepareIntRow("", QString("Hoop panels in stripe %1").arg(isl+1), m_pBody->m_hPanels.at(isl));
            hoopFolder.first()->appendRow(dataItem);
        }
    }

    QList<QStandardItem*> bodyFrameFolder = prepareRow("Frames");
    bodyFolder.first()->appendRow(bodyFrameFolder);
    {
        m_pStruct->expand(m_pModel->indexFromItem(bodyFrameFolder.first()));

        for(int iFrame=0; iFrame <m_pBody->splineSurface()->frameCount(); iFrame++)
        {
            Frame *pFrame = m_pBody->splineSurface()->frameAt(iFrame);

            QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame_%1").arg(iFrame+1));
            bodyFrameFolder.first()->appendRow(sectionFolder);
            {
                if(m_pBody->m_iActiveFrame==iFrame) m_pStruct->expand(m_pModel->indexFromItem(sectionFolder.first()));

                dataItem = prepareIntRow("", "Lengthwise panels (FLATPANELS case)", m_pBody->m_xPanels.at(iFrame));
                sectionFolder.first()->appendRow(dataItem);

                QList<QStandardItem*> dataItem = prepareDoubleRow("x_Position", "x", pFrame->m_Position.x*Units::mtoUnit(), Units::lengthUnitLabel());
                sectionFolder.first()->appendRow(dataItem);

                for(int iPt=0; iPt<pFrame->pointCount(); iPt++)
                {
                    QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt+1));
                    sectionFolder.first()->appendRow(pointFolder);
                    {
                        if(Frame::selectedIndex()==iPt) m_pStruct->expand(m_pModel->indexFromItem(pointFolder.first()));

                        Vector3d Pt(pFrame->point(iPt));
                        QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", Pt.x*Units::mtoUnit(), Units::lengthUnitLabel());
                        pointFolder.first()->appendRow(dataItem);

                        dataItem = prepareDoubleRow("", "y", Pt.y*Units::mtoUnit(), Units::lengthUnitLabel());
                        pointFolder.first()->appendRow(dataItem);

                        dataItem = prepareDoubleRow("", "z", Pt.z*Units::mtoUnit(), Units::lengthUnitLabel());
                        pointFolder.first()->appendRow(dataItem);
                    }
                }
            }
        }
    }
}


void EditBodyDlg::updateViews()
{
    m_pglBodyView->update();
    m_pFrameWidget->update();
    m_pBodyLineWidget->update();

}


void EditBodyDlg::onRedraw()
{
    QStandardItem *pItem = m_pModel->itemFromIndex(m_pModel->index(0,0));
    readBodyTree(pItem->child(0,0)->index());

    m_pglBodyView->resetGLBody();

    m_bChanged = true;
    updateViews();
}


void EditBodyDlg::onRefillBodyTree()
{
    fillBodyTreeView();
    m_pglBodyView->resetGLBody();

    m_pglBodyView->update();
    m_pBodyLineWidget->update();
    m_pFrameWidget->update();
}


void EditBodyDlg::readBodyTree(QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex, subIndex;

    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();

        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(pItem->child(0,0))
        {
            if(object.compare("Color", Qt::CaseInsensitive)==0)
            {
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    field = subIndex.sibling(subIndex.row(),1).data().toString();
                    value = subIndex.sibling(subIndex.row(),2).data().toString();

                    dataIndex = subIndex.sibling(subIndex.row(),2);

                    if(field.compare("red", Qt::CaseInsensitive)==0)         m_pBody->color().setRed(dataIndex.data().toInt());
                    else if(field.compare("green", Qt::CaseInsensitive)==0)  m_pBody->color().setGreen(dataIndex.data().toInt());
                    else if(field.compare("blue", Qt::CaseInsensitive)==0)   m_pBody->color().setBlue(dataIndex.data().toInt());
                    else if(field.compare("alpha", Qt::CaseInsensitive)==0)  m_pBody->color().setAlpha(dataIndex.data().toInt());

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }while(subIndex.isValid());
            }
            else if(object.compare("Inertia", Qt::CaseInsensitive)==0)
            {
                QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
                if(pItem)
                {
                    double m=0;
                    readInertiaTree(m, m_pBody->m_PointMass, pItem->child(0,0)->index());
                    m_pBody->setVolumeMass(m);
                }
            }
            else if(object.compare("NURBS", Qt::CaseInsensitive)==0)
            {
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    field = subIndex.sibling(subIndex.row(),1).data().toString();
                    value = subIndex.sibling(subIndex.row(),2).data().toString();

                    dataIndex = subIndex.sibling(subIndex.row(),2);

                    if(field.compare("NURBS degree (lengthwise)", Qt::CaseInsensitive)==0)     m_pBody->splineSurface()->setuDegree(dataIndex.data().toInt());
                    else if(field.compare("NURBS degree (hoop)", Qt::CaseInsensitive)==0)      m_pBody->splineSurface()->setvDegree(dataIndex.data().toInt());
                    else if(field.compare("Mesh panels (lengthwise)", Qt::CaseInsensitive)==0) m_pBody->m_nxPanels = dataIndex.data().toInt();
                    else if(field.compare("Mesh panels (hoop)", Qt::CaseInsensitive)==0)       m_pBody->m_nhPanels = dataIndex.data().toInt();

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
            else if(object.compare("Hoop_panels (FLATPANELS case)", Qt::CaseInsensitive)==0)
            {
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    field = subIndex.sibling(subIndex.row(),1).data().toString();
                    value = subIndex.sibling(subIndex.row(),2).data().toString();
                    dataIndex = subIndex.sibling(subIndex.row(),2);

                    int idx = field.right(field.length()-22).toInt()-1;
                    m_pBody->m_hPanels[idx] =  dataIndex.data().toInt();

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
            else if(object.compare("Frames", Qt::CaseInsensitive)==0)
            {
                m_pBody->m_SplineSurface.clearFrames();
                subIndex = pItem->child(0,0)->index();
                do
                {
                    object = subIndex.sibling(subIndex.row(),0).data().toString();
                    if(object.indexOf("Frame_")>=0)
                    {
                        Frame *pFrame = new Frame;
                        QStandardItem *pSubItem = m_pModel->itemFromIndex(subIndex);
                        if(pSubItem->child(0,0))
                        {
                            readBodyFrameTree(pFrame, pSubItem->child(0,0)->index());
                            m_pBody->m_SplineSurface.appendFrame(pFrame);
                        }
                    }

                    subIndex = subIndex.sibling(subIndex.row()+1,0);
                }
                while(subIndex.isValid());
            }
        }
        else
        {
            //no more children
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            field = indexLevel.sibling(indexLevel.row(),1).data().toString();
            value = indexLevel.sibling(indexLevel.row(),2).data().toString();

            dataIndex = indexLevel.sibling(indexLevel.row(),2);

            if     (field.compare("Name", Qt::CaseInsensitive)==0) m_pBody->setName(value);
            else if(field.compare("Type", Qt::CaseInsensitive)==0) m_pBody->bodyType() = xfl::bodyPanelType(value);
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void EditBodyDlg::readBodyFrameTree(Frame *pFrame, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    double xPt = 0.0;

    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();

        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if (field.compare("Lengthwise panels (FLATPANELS case)", Qt::CaseInsensitive)==0)   m_pBody->m_xPanels.append(dataIndex.data().toInt());
        else if (object.compare("x_Position", Qt::CaseInsensitive)==0) xPt = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (object.indexOf("Point", Qt::CaseInsensitive)==0)
        {
            Vector3d Pt;
            QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
            if(pItem)
            {
                readVectorTree(Pt, pItem->child(0,0)->index());
                pFrame->appendPoint(Pt);
            }
        }
        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());

    pFrame->setuPosition(xPt);
}


void EditBodyDlg::readInertiaTree(double &volumeMass, QVector<PointMass*>&pointMasses, QModelIndex indexLevel)
{
    pointMasses.clear();

    QString object, field, value;
    QModelIndex dataIndex;
    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(pItem->child(0,0))
        {
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
            {
                PointMass *ppm = new PointMass;
                readPointMassTree(ppm, pItem->child(0,0)->index());
                pointMasses.append(ppm);
            }
        }
        else
        {
            //no more children
            object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            field = indexLevel.sibling(indexLevel.row(),1).data().toString();
            value = indexLevel.sibling(indexLevel.row(),2).data().toString();
            dataIndex = indexLevel.sibling(indexLevel.row(),2);

            if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   volumeMass = dataIndex.data().toDouble()/Units::kgtoUnit();
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void EditBodyDlg::readInertiaTree(double &volumeMass, QVector<PointMass>&pointmasses, QModelIndex indexlevel)
{
    pointmasses.clear();

    QString object, field, value;
    QModelIndex dataIndex;
    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexlevel);
        if(pItem->child(0,0))
        {
            object = indexlevel.sibling(indexlevel.row(),0).data().toString();
            if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
            {
                PointMass pm;
                readPointMassTree(&pm, pItem->child(0,0)->index());
                pointmasses.append(pm);
            }
        }
        else
        {
            //no more children
            object = indexlevel.sibling(indexlevel.row(),0).data().toString();
            field = indexlevel.sibling(indexlevel.row(),1).data().toString();
            value = indexlevel.sibling(indexlevel.row(),2).data().toString();
            dataIndex = indexlevel.sibling(indexlevel.row(),2);

            if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   volumeMass = dataIndex.data().toDouble()/Units::kgtoUnit();
        }

        indexlevel = indexlevel.sibling(indexlevel.row()+1,0);

    } while(indexlevel.isValid());
}


void EditBodyDlg::readPointMassTree(PointMass *ppm, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if      (field.compare("mass", Qt::CaseInsensitive)==0) ppm->setMass(dataIndex.data().toDouble()/Units::kgtoUnit());
        else if (field.compare("tag",  Qt::CaseInsensitive)==0) ppm->setTag(value);
        else if (field.compare("x",    Qt::CaseInsensitive)==0) ppm->setXPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("y",    Qt::CaseInsensitive)==0) ppm->setYPosition(dataIndex.data().toDouble()/Units::mtoUnit());
        else if (field.compare("z",    Qt::CaseInsensitive)==0) ppm->setZPosition(dataIndex.data().toDouble()/Units::mtoUnit());

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}



void EditBodyDlg::readVectorTree(Vector3d &V, QModelIndex indexLevel)
{
    QString object, field, value;
    QModelIndex dataIndex;
    // not expecting any more children
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        field = indexLevel.sibling(indexLevel.row(),1).data().toString();
        value = indexLevel.sibling(indexLevel.row(),2).data().toString();
        dataIndex = indexLevel.sibling(indexLevel.row(),2);

        if (field.compare("x", Qt::CaseInsensitive)==0)      V.x = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("y", Qt::CaseInsensitive)==0) V.y = dataIndex.data().toDouble()/Units::mtoUnit();
        else if (field.compare("z", Qt::CaseInsensitive)==0) V.z = dataIndex.data().toDouble()/Units::mtoUnit();

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
    } while(indexLevel.isValid());
}



void EditBodyDlg::onItemClicked(const QModelIndex &index)
{
    identifySelection(index);
    update();
}



void EditBodyDlg::identifySelection(const QModelIndex &indexSel)
{
    // we highlight wing sections and body frames
    // so check if the user's selection is one of these
    m_enumActiveWingType = xfl::OTHERWING;
    m_iActivePointMass = -1;

    QModelIndex indexLevel = indexSel;
    QString object;
    do
    {
        object = indexLevel.sibling(indexLevel.row(),0).data().toString();

        if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
        {
            setActiveFrame(object.right(object.length()-6).toInt() -1);
            //            Frame::setSelected(-1);
            m_pglBodyView->m_bResetglFrameHighlight = true;
            m_iActivePointMass = -1;
            return;
        }
        else if(object.indexOf("Point_Mass_", 0, Qt::CaseInsensitive)>=0)
        {
            m_iActivePointMass = object.right(object.length()-11).toInt() -1;
            setActiveFrame(-1);
            return;
        }
        else if(object.indexOf("Point", 0, Qt::CaseInsensitive)==0)
        {
            Frame::setSelected(object.right(object.length()-6).toInt() -1);
            //identify the parent Frame object

            indexLevel = indexLevel.parent();
            do
            {
                object = indexLevel.sibling(indexLevel.row(),0).data().toString();

                if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
                {
                    setActiveFrame(object.right(object.length()-6).toInt() -1);
                    return;
                }

                indexLevel = indexLevel.parent();
            } while(indexLevel.isValid());

            setActiveFrame(-1);
            return;
        }
        indexLevel = indexLevel.parent();
    } while(indexLevel.isValid());
}


void EditBodyDlg::setActiveFrame(int iFrame)
{
    m_pBody->setActiveFrame(m_pBody->frame(iFrame));
    m_pglBodyView->m_bResetglFrameHighlight = true;
    m_pglBodyView->update();
}


void EditBodyDlg::onInsertBefore()
{
    if(!m_pBody) return;
    if( m_pBody->activeFrame())
    {
        m_pBody->insertFrameBefore(m_pBody->m_iActiveFrame);

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillBodyTreeView();

        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->resetGLBody();
        m_pglBodyView->update();
    }
    else if(m_iActivePointMass>=0)
    {
        m_pBody->m_PointMass.insert(m_iActivePointMass, PointMass());

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());

        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->update();

    }
}



void EditBodyDlg::onInsertAfter()
{
    if(m_pBody && m_pBody->activeFrame())
    {
        m_pBody->insertFrameAfter(m_pBody->m_iActiveFrame);

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillBodyTreeView();

        m_pBody->m_iActiveFrame++;
        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->resetGLBody();

        m_pglBodyView->update();
    }
    else if(m_iActivePointMass>=0)
    {
        if(m_pBody)
        {
            m_pBody->m_PointMass.insert(m_iActivePointMass+1, PointMass());
            m_iActivePointMass++;
        }

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillBodyTreeView();

        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->update();

    }
}


void EditBodyDlg::onDelete()
{
    if(m_pBody && m_pBody->activeFrame())
    {
        m_pBody->removeActiveFrame();

        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillBodyTreeView();

        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->resetGLBody();

        m_pglBodyView->update();
    }
    else if(m_iActivePointMass>=0)
    {
        if(m_pBody)
        {
            m_pBody->m_PointMass.removeAt(m_iActivePointMass);
        }


        m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
        fillBodyTreeView();

        m_bChanged = true;
        m_pglBodyView->m_bResetglFrameHighlight = true;
        m_pglBodyView->update();
    }
}




/**
 * Draws the wing legend in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 */
void EditBodyDlg::paintBodyLegend(QPainter &painter)
{
    painter.save();

    QPen textPen(DisplayOptions::textColor());
    painter.setPen(textPen);
    painter.setFont(DisplayOptions::textFont());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.restore();
}


bool EditBodyDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("EditBodyDlg");
    {
        //  we're reading/loading
        s_WindowGeometry = settings.value("Geometry").toByteArray();
        m_HSplitterSizes     = settings.value("HorizontalSplitterSizes").toByteArray();
    }
    settings.endGroup();
    return true;
}


bool EditBodyDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("EditBodyDlg");
    {
        settings.setValue("Geometry", s_WindowGeometry);
        settings.setValue("HorizontalSplitterSizes", m_HSplitterSizes);
    }
    settings.endGroup();

    return true;
}


void EditBodyDlg::onExportBodyGeom()
{
    if(!m_pBody) return;
    QString LengthUnit, FileName;

    Units::getLengthUnitLabel(LengthUnit);

    FileName = m_pBody->m_Name;
    FileName.replace("/", " ");

    int type = 1;

    QString filter =".csv";

    FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Geometry"),
                                            xfl::s_LastDirName ,
                                            QObject::tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) type = 2;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    m_pBody->exportGeometry(out, type, Units::mtoUnit(), NXPOINTS, NHOOPPOINTS);
}


void EditBodyDlg::onExportBodyXML()
{
    if(!m_pBody)return ;// is there anything to export ?

    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pBody->name();
    FileName = QFileDialog::getSaveFileName(this, tr("Export plane definition to xml file"),
                                            xfl::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    pos = FileName.indexOf(".xml", Qt::CaseInsensitive);
    if(pos<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XMLPlaneWriter planeWriter(XFile);

    planeWriter.writeXMLBody(m_pBody);

    XFile.close();
}


void EditBodyDlg::onImportBodyXML()
{
    //    Body memBody;
    //    memBody.duplicate(m_pBody);

    QString PathName;
    PathName = QFileDialog::getOpenFileName(this, tr("Open XML File"),
                                            xfl::s_LastDirName,
                                            tr("Plane XML file")+"(*.xml)");
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(this, tr("Warning"), strange);
        return;
    }

    Plane a_plane;
    XMLPlaneReader planeReader(XFile, &a_plane);
    planeReader.readXMLPlaneFile();

    XFile.close();

    if(planeReader.hasError())
    {
        QString errorMsg = planeReader.errorString() + QString("\nline %1 column %2").arg(planeReader.lineNumber()).arg(planeReader.columnNumber());
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        //        m_pBody->duplicate(&memBody);
        return;
    }

    m_pBody->duplicate(a_plane.body());
    initDialog(m_pBody);

    m_pglBodyView->resetGLBody();

    m_bChanged = true;

    updateViews();
}


void EditBodyDlg::onScaleBody()
{
    if(!m_pBody) return;

    BodyScaleDlg dlg(this);

    dlg.m_FrameID = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        m_bChanged = true;
        m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
        m_pglBodyView->resetGLBody();

        fillBodyTreeView();

        updateViews();
    }
}


void EditBodyDlg::onTranslateBody()
{
    if(!m_pBody) return;

    BodyTransDlg dlg(this);
    dlg.m_FrameID    = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        m_bChanged = true;
        m_pBody->translate(dlg.m_XTrans, dlg.m_YTrans, dlg.m_ZTrans, dlg.m_bFrameOnly, dlg.m_FrameID);
        fillBodyTreeView();
        m_pglBodyView->resetGLBody();
        updateViews();
    }
}


void EditBodyDlg::onBodyInertia()
{
    if(!m_pBody) return;
    InertiaDlg dlg(this);
    dlg.m_pBody  = m_pBody;
    dlg.m_pPlane = nullptr;
    dlg.m_pWing  = nullptr;
    dlg.initDialog();
    dlg.move(pos().x()+25, pos().y()+25);
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    m_pBody->computeBodyAxisInertia();
    fillBodyTreeView();
}







