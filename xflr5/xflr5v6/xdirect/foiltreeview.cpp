/****************************************************************************

    xfl5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QHeaderView>

#include "foiltreeview.h"

#include <globals/mainframe.h>
#include <misc/options/saveoptions.h>
//#include <twodwidgets/foildesignwt.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects2d/oppoint.h>
#include <xflobjects/objects2d/polar.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/mvc/expandabletreeview.h>
#include <xflwidgets/mvc/objecttreedelegate.h>
#include <xflwidgets/mvc/objecttreeitem.h>
#include <xflwidgets/mvc/objecttreemodel.h>
//#include <xfl/xdirect/menus/xdirectmenus.h>


MainFrame *FoilTreeView::s_pMainFrame = nullptr;
XDirect *FoilTreeView::s_pXDirect   = nullptr;


FoilTreeView::FoilTreeView(QWidget *pParent) : QWidget(pParent)
{
    m_pStruct = nullptr;
    m_pModel  = nullptr;
    m_pDelegate = nullptr;

    m_Selection = xfl::NONE;
    m_pptoObjectProps = new PlainTextOutput;

    setupLayout();

    QStringList labels;
    labels << tr("Object")  << "1234567"<< "";


    m_pModel = new ObjectTreeModel(this);
    m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);

    m_pStruct->setModel(m_pModel);
    connect(m_pStruct->m_pleFilter, SIGNAL(returnPressed()), this, SLOT(onSetFilter()));

    m_pStruct->header()->hide();
    m_pStruct->header()->setStretchLastSection(false);
    m_pStruct->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pStruct->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_pStruct->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    int av = m_pStruct->treeFontStruct().averageCharWidth();
#ifdef Q_OS_WIN
    m_pStruct->header()->resizeSection(1, 11*av);
    m_pStruct->header()->resizeSection(2, 5*av);
#else
    m_pStruct->header()->resizeSection(1, 7*av);
    m_pStruct->header()->resizeSection(2, 3*av);
#endif

    m_pDelegate = new ObjectTreeDelegate(this);
    m_pStruct->setItemDelegate(m_pDelegate);

    connect(m_pStruct, SIGNAL(pressed(QModelIndex)), SLOT(onItemClicked(QModelIndex)));
    //	connect(m_pStruct, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onItemDoubleClicked(QModelIndex)));
    connect(m_pStruct->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex, QModelIndex)));
}


FoilTreeView::~FoilTreeView()
{
}


void FoilTreeView::showEvent(QShowEvent *pEvent)
{
    m_pMainSplitter->restoreState(m_SplitterSizes);
    QWidget::showEvent(pEvent);
}


void FoilTreeView::hideEvent(QHideEvent *pEvent)
{
    m_SplitterSizes = m_pMainSplitter->saveState();

    QWidget::hideEvent(pEvent);
}


void FoilTreeView::updateObjectView()
{
    setObjectProperties();
    fillModelView();

    if (s_pXDirect->m_bPolarView)
    {
        if (XDirect::curPolar()) selectPolar(XDirect::curPolar());
        else                     selectFoil(XDirect::curFoil());
    }
    else
    {
        if      (XDirect::curOpp())   selectOpPoint();
        else if (XDirect::curPolar()) selectPolar(XDirect::curPolar());
        else                          selectFoil(XDirect::curFoil());
    }
}


/** updates the plane items after the WPolars or PlaneOpps have been changed, deleted or something */
void FoilTreeView::updateFoil(Foil const *pFoil)
{
    if(!pFoil) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        if(pFoilItem->name().compare(pFoil->name())==0)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(planeindex);
            if(!pItem) continue;
            m_pModel->blockSignals(true);
            m_pModel->removeRows(0, pItem->rowCount(), planeindex);
            m_pModel->blockSignals(false);

            fillPolars(pFoilItem, pFoil);
            break;
        }
    }
}


void FoilTreeView::setObjectProperties()
{
    QString props;
    switch(m_Selection)
    {
        //		{NONE, FOIL, POLAR, OPPOINT}
        case xfl::FOIL:
        {
            if(XDirect::curFoil())
            {
                props = XDirect::curFoil()->properties();
                break;
            }
            break;
        }
        case xfl::POLAR:
        {
            if(XDirect::curPolar())
            {
                s_pXDirect->curPolar()->getProperties(props);
                break;
            }
            break;
        }
        case xfl::OPPOINT:
        {
            if(XDirect::curOpp())
            {
                XDirect::curOpp()->getOppProperties(props, XDirect::curFoil(), false);
                break;
            }
            break;
        }
        default:
        {
            props.clear();
            break;
        }
    }
    m_pptoObjectProps->setPlainText(props);
}


void FoilTreeView::setupLayout()
{
    m_pStruct = new ExpandableTreeView;
    m_pStruct->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pStruct->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pStruct->setUniformRowHeights(true);
    m_pStruct->setRootIsDecorated(true);
    connect(m_pStruct, SIGNAL(switchAll(bool)), SLOT(onSwitchAll(bool)));

    m_pMainSplitter = new QSplitter;
    m_pMainSplitter->setOrientation(Qt::Vertical);
    {
        m_pMainSplitter->addWidget(m_pStruct);
        m_pMainSplitter->addWidget(m_pptoObjectProps);
    }
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pStruct->cmdWidget());
        pMainLayout->addWidget(m_pMainSplitter);
    }
    setLayout(pMainLayout);
}


void FoilTreeView::fillModelView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    ObjectTreeItem *pRootItem = m_pModel->rootItem();

    m_pStruct->selectionModel()->blockSignals(true);

    for(int i=0; i<Objects2d::foilCount(); i++)
    {
        Foil const *pFoil = Objects2d::foilAt(i);
        if(!pFoil) continue;

        LineStyle ls(pFoil->theStyle());
        ObjectTreeItem *pFoilItem = m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), foilState(pFoil));
        fillPolars(pFoilItem, pFoil);
    }

    m_pStruct->selectionModel()->blockSignals(false);
}


void FoilTreeView::fillPolars(ObjectTreeItem *pFoilItem, Foil const *pFoil)
{
    if(!pFoil || !pFoilItem) return;

    for(int iPolar=0; iPolar<Objects2d::polarCount(); iPolar++)
    {
        Polar *pPolar = Objects2d::polarAt(iPolar);
        if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
        {
            Polar *pPolar = Objects2d::polarAt(iPolar);
            if(!pPolar) continue;
            if(pPolar && pPolar->foilName().compare(pFoil->name())==0)
            {
                LineStyle ls(pPolar->theStyle());
                ls.m_bIsEnabled = true;
                m_pModel->appendRow(pFoilItem, pPolar->name(), ls, polarState(pPolar));
            }
            addOpps(pPolar);
        }
    }
}


Qt::CheckState FoilTreeView::foilState(Foil const *pFoil) const
{
    bool bAll = true;
    bool bNone = true;
    if(s_pXDirect->isPolarView())
    {
        for(int iplr=0; iplr<Objects2d::polarCount(); iplr++)
        {
            Polar *pPolar = Objects2d::polarAt(iplr);
            if(pFoil->name()==pPolar->foilName())
            {
                bAll = bAll && pPolar->isVisible();
                bNone = bNone && !pPolar->isVisible();
            }
        }
        if(bAll && bNone)  return Qt::Unchecked; // foil has no polars
        else if(bAll)      return Qt::Checked;
        else if(bNone)     return Qt::Unchecked;
        else               return Qt::PartiallyChecked;
    }
    else
    {
        for(int iopp=0; iopp<Objects2d::oppCount(); iopp++)
        {
            OpPoint *pOpp = Objects2d::oppAt(iopp);
            if(pFoil->name()==pOpp->foilName())
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
        if(bAll)       return Qt::Checked;
        else if(bNone) return Qt::Unchecked;
        else           return Qt::PartiallyChecked;
    }
}


Qt::CheckState FoilTreeView::polarState(Polar const *pPolar) const
{
    if(s_pXDirect->isOppView())
    {
        bool bAll = true;
        bool bNone = true;
        for(int iopp=0; iopp<Objects2d::oppCount(); iopp++)
        {
            OpPoint const *pOpp = Objects2d::oppAt(iopp);
            if(pPolar->hasOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
        if     (bNone) return Qt::Unchecked;
        else if(bAll)  return Qt::Checked;
        else           return Qt::PartiallyChecked;
    }
    else if(s_pXDirect->isPolarView())
    {
        return pPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    return Qt::Unchecked;
}


void FoilTreeView::setObjectFromIndex(const QModelIndex &index)
{
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.sibling(index.row(), 0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(!pSelectedItem) return;

    if(pSelectedItem->level()==1)
    {
        Foil *m_pFoil = Objects2d::foil(pSelectedItem->name());
        s_pXDirect->setFoil(m_pFoil);
        XDirect::setCurPolar(nullptr);
        XDirect::setCurOpp(nullptr);
        m_Selection = xfl::FOIL;
    }
    else if(pSelectedItem->level()==2)
    {
        ObjectTreeItem const*pFoilItem = pSelectedItem->parentItem();
        Foil *m_pFoil = Objects2d::foil(pFoilItem->name());
        Polar *m_pPolar = Objects2d::getPolar(m_pFoil, pSelectedItem->name());
        s_pXDirect->setFoil(m_pFoil);
        s_pXDirect->setPolar(m_pPolar);
        XDirect::setCurOpp(nullptr);
        m_Selection = xfl::POLAR;
    }
    else if(pSelectedItem->level()==3)
    {
        ObjectTreeItem *pWPolarItem = pSelectedItem->parentItem();
        ObjectTreeItem *pFoilItem = pWPolarItem->parentItem();
        Foil *m_pFoil   = Objects2d::foil(pFoilItem->name());
        Polar *m_pPolar = Objects2d::getPolar(m_pFoil->name(), pWPolarItem->name());
        OpPoint *m_pOpp = Objects2d::getOpp(m_pFoil, m_pPolar, pSelectedItem->name().toDouble());

        s_pXDirect->setFoil(m_pFoil);
        s_pXDirect->setPolar(m_pPolar);
        s_pXDirect->setOpp(m_pOpp);
        m_Selection = xfl::OPPOINT;
    }
    else m_Selection = xfl::NONE;

    s_pXDirect->setControls();
    setObjectProperties();
}


void FoilTreeView::onCurrentRowChanged(QModelIndex currentIndex, QModelIndex)
{
    setObjectFromIndex(currentIndex);
    s_pXDirect->updateView();
}


void FoilTreeView::onItemClicked(const QModelIndex &index)
{
    if(QDate::currentDate().daysTo(QDate(2021,11,27))<=0)    return; // set a limit date for safety

    Foil *pFoil   = s_pXDirect->curFoil();
    Polar *pPolar = s_pXDirect->curPolar();
    OpPoint *pOpp = s_pXDirect->curOpp();
    if(index.column()==1)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

        if(pOpp)
        {
            LineStyle ls(pOpp->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            pOpp->setLineStipple(ls.m_Stipple);
            pOpp->setLineWidth(ls.m_Width);
            pOpp->setLineColor(ls.m_Color);
            pOpp->setPointStyle(ls.m_Symbol);
            pItem->setTheStyle(ls);
            emit(s_pXDirect->projectModified());
        }
        else if(pPolar)
        {
            LineStyle ls(pPolar->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            Objects2d::setPolarStyle(pPolar, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
            //			pItem->setData(QVariant::fromValue(ls), Qt::DisplayRole);
            setCurveParams();
            emit(s_pXDirect->projectModified());
        }
        else if(pFoil)
        {
            LineStyle ls(pFoil->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();
            Objects2d::setFoilStyle(pFoil, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
            //			pItem->setData(QVariant::fromValue(ls), Qt::DisplayRole);
            setCurveParams();
            //			s_pXDirect->updateBufferFoil();
            emit(s_pXDirect->projectModified());
        }
    }
    else if (index.column()==2)
    {
        if(pOpp)
        {
            pOpp->setVisible(!pOpp->isVisible());
            setCurveParams(); // to update polar and foil states
            emit(s_pXDirect->projectModified());
        }
        else if(pPolar)
        {
            if(s_pXDirect->isPolarView())
            {
                Objects2d::setPolarVisible(pPolar, !pPolar->isVisible());
            }
            else
            {
                Qt::CheckState state = polarState(pPolar);
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    Objects2d::setPolarVisible(pPolar, true);
                else if(state==Qt::Checked)
                    Objects2d::setPolarVisible(pPolar, false);
            }

            setCurveParams();
            emit(s_pXDirect->projectModified());
        }
        else if(pFoil)
        {
            Qt::CheckState state = foilState(pFoil);

            if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                Objects2d::setFoilVisible(pFoil, true, true);
            else if(state==Qt::Checked)
                Objects2d::setFoilVisible(pFoil, false, false);

            setCurveParams();
            emit(s_pXDirect->projectModified());
        }
        setOverallCheckStatus();
    }
    s_pXDirect->resetCurves();
//    s_pXDirect->m_pDFoilWt->resetLegend();
    s_pXDirect->updateView();

    m_pModel->updateData();
}


void FoilTreeView::onItemDoubleClicked(const QModelIndex &index)
{
    setObjectFromIndex(index);
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
    if(m_Selection==xfl::POLAR)
        s_pXDirect->onEditCurPolar();
}


void FoilTreeView::selectObjects()
{
    if     (s_pXDirect->curOpp())   selectOpPoint();
    else if(s_pXDirect->curPolar()) selectPolar(s_pXDirect->curPolar());
    else                            selectFoil(s_pXDirect->curFoil());
}



void FoilTreeView::selectFoil(Foil*pFoil)
{
    if(!pFoil) pFoil = XDirect::curFoil();
    if(!pFoil)
    {
        setObjectProperties();
        return;
    }

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pItem->name().compare(pFoil->name())==0)
        {
            // select the foil's row
            QModelIndex idx = m_pModel->index(pItem->parentItem(), pItem);
            m_pStruct->setCurrentIndex(idx);
            m_pStruct->scrollTo(idx);
            m_Selection = xfl::FOIL;

            break;
        }
    }
    setObjectProperties();
}


void FoilTreeView::selectPolar(Polar*pPolar)
{
    if(!pPolar) pPolar = XDirect::curPolar();
    if(!pPolar)
    {
        setObjectProperties();
        return;
    }

    // the foil item is the polar's parent item
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pFoilItem->name().compare(pPolar->foilName())==0)
        {
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                if(pFoilItem->child(jr))
                {
                    const QModelIndex &polarChild = m_pModel->index(jr,0,pFoilItem);

                    ObjectTreeItem *pPolarItem = m_pModel->itemFromIndex(polarChild);
                    // is it the correct polar name?
                    if(pPolarItem->name().compare(pPolar->polarName())==0)
                    {
                        // select the polar row
                        QModelIndex idx = m_pModel->index(pFoilItem, pPolarItem);
                        m_pStruct->expand(idx);
                        m_pStruct->setCurrentIndex(polarChild);
                        m_pStruct->scrollTo(polarChild);
                        m_Selection = xfl::POLAR;

                        break;
                    }
                }
            }
        }
    }
    setObjectProperties();
}


void FoilTreeView::selectOpPoint(OpPoint *pOpp)
{
    if(!pOpp) pOpp = XDirect::curOpp();
    if(!pOpp)
    {
        setObjectProperties();
        return;
    }

    bool bSelected = false;

    for(int ir = 0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // is it the correct foil name?
        if(pFoilItem->name().compare(pOpp->foilName())==0)
        {
            //browse the polars
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
                if(pPolarItem)
                {
                    const QModelIndex &polarChild = m_pModel->index(pFoilItem, pPolarItem);

                    ObjectTreeItem *pPolarItem = m_pModel->itemFromIndex(polarChild);
                    // is it the correct polar name?
                    if(pPolarItem && pPolarItem->name().compare(pOpp->polarName())==0)
                    {
                        //browse the opps
                        for(int kr=0; kr<pPolarItem->rowCount(); kr++)
                        {
                            Foil *m_pFoil = Objects2d::foil(pFoilItem->name());
                            Polar *m_pPolar = Objects2d::getPolar(m_pFoil, pPolarItem->name());


                            ObjectTreeItem *pOppItem = pPolarItem->child(kr);
                            if(pOppItem)
                            {
                                const QModelIndex &oppChild = m_pModel->index(pPolarItem, pOppItem);
                                double val = pOppItem->name().toDouble();
                                // is it the correct aoa?
                                bool bCorrect = false;
                                Q_ASSERT(m_pPolar!=nullptr);
                                if(m_pPolar->isFixedaoaPolar())
                                {
                                    bCorrect =(fabs(val-pOpp->Reynolds())<0.1);
                                }
                                else
                                {
                                    bCorrect =(fabs(val-pOpp->aoa())<0.001);
                                }
                                if(bCorrect)
                                {
                                    // select the opp row
                                    m_pStruct->setCurrentIndex(oppChild);
                                    m_pStruct->scrollTo(oppChild);
                                    m_Selection = xfl::OPPOINT;
                                    setCurveParams();
                                    bSelected = true;
                                    break;
                                }

                            }
                        }
                    }
                }
                if(bSelected) break;
            }
        }
        if(bSelected) break;
    }
    setObjectProperties();
}


void FoilTreeView::addOpps(Polar *pPolar)
{
    if(!pPolar) return;
    QString format = xfl::g_bLocalize ? "%L1" : "%1";
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        // find the polar's parent foil
        if(pFoilItem->name().compare(pPolar->foilName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);
                if(pOldPolarItem)
                {
                    const QModelIndex &oldPolarChild = m_pModel->index(pFoilItem, pOldPolarItem);
                    if(pOldPolarItem->name().compare(pPolar->polarName(), Qt::CaseInsensitive)==0)
                    {
                        //clear the children
                        m_pModel->removeRows(0, pOldPolarItem->rowCount(), oldPolarChild);
                        //insert POpps
                        for(int kr=0; kr<Objects2d::oppCount(); kr++)
                        {
                            OpPoint * pOpp = Objects2d::oppAt(kr);
                            if(pOpp->foilName().compare(pPolar->foilName())==0 && pOpp->polarName().compare(pPolar->polarName())==0)
                            {
                                QString strange;

                                if   (pPolar->isFixedaoaPolar()) strange = QString(format).arg(pOpp->Reynolds(), 0, 'f', 0);
                                else                             strange = QString(format).arg(pOpp->aoa(),      0, 'f', 3);

                                LineStyle ls(pOpp->theStyle());
                                ls.m_bIsEnabled = !s_pXDirect->isPolarView();
                                m_pModel->appendRow(pOldPolarItem, strange, ls, Qt::PartiallyChecked);
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
    setOverallCheckStatus();
}


void FoilTreeView::insertFoil(Foil *pFoil)
{
    if(!pFoil) pFoil = XDirect::curFoil();
    if(!pFoil) return;

    bool bInserted = false;
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // insert alphabetically
        if(pFoilItem->name().compare(pFoil->name())==0)
        {
            //A foil of that name already exists
            // clear its rows
            QModelIndex foilindex= m_pModel->index(m_pModel->rootItem(), pFoilItem);
            m_pModel->removeRows(0, pFoilItem->rowCount(), foilindex);
            bInserted = true;
            break;
        }
        else if(pFoilItem->name().compare(pFoil->name(), Qt::CaseInsensitive)>0)
        {
            //insert before
            m_pModel->insertRow(m_pModel->rootItem(), ir, pFoil->name(), pFoil->theStyle(), foilState(pFoil));
            bInserted = true;
            break;
        }
    }

    if(!bInserted)
    {
        //not inserted, append
        ObjectTreeItem* pRootItem = m_pModel->rootItem();
        m_pModel->appendRow(pRootItem, pFoil->name(), pFoil->theStyle(), foilState(pFoil));
    }

    for(int ip=0; ip<Objects2d::polarCount(); ip++)
    {
        Polar *pPolar = Objects2d::polarAt(ip);
        if(pPolar->foilName().compare(pFoil->name())==0)
            insertPolar(pPolar);
    }

    setOverallCheckStatus();
}


void FoilTreeView::insertPolar(Polar *pPolar)
{
    if(!pPolar) pPolar = XDirect::curPolar();
    if(!pPolar) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);

        // find the polar's parent foil
        if(pFoilItem->name().compare(pPolar->foilName())==0)
        {
            //if this polar name exists, don't insert it
            ObjectTreeItem *pNewPolarItem = nullptr;
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);
                if(pOldPolarItem->name().compare(pPolar->polarName(), Qt::CaseInsensitive)==0)
                {
                    pNewPolarItem = pOldPolarItem;
                }
                else if(pOldPolarItem->name().compare(pPolar->polarName(), Qt::CaseInsensitive)>0)
                {
                    //insert before
                    pNewPolarItem = m_pModel->insertRow(pFoilItem, jr, pPolar->name(), pPolar->theStyle(), polarState(pPolar));
                }
                if(pNewPolarItem) break;
            }

            if(!pNewPolarItem)
            {
                m_pModel->appendRow(pFoilItem, pPolar->name(), pPolar->theStyle(), polarState(pPolar));
                break;
            }
        }
    }

    setOverallCheckStatus();
}


void FoilTreeView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex idx = m_pStruct->currentIndex();
    ObjectTreeItem *pItem = m_pModel->itemFromIndex(idx);
    if(!pItem) return;
    QString strong;
    Foil *m_pFoil   = s_pXDirect->curFoil();
    Polar *m_pPolar = s_pXDirect->curPolar();
    OpPoint *m_pOpp = s_pXDirect->curOpp();

    if(pItem->level()==1)
    {
        m_pFoil = Objects2d::foil(pItem->name());
        if(m_pFoil) strong  = m_pFoil->name();
        else        strong.clear();
    }
    else if(pItem->level()==2)
    {
        strong = m_pPolar->polarName();
    }
    else if(pItem->level()==3)
    {
        ObjectTreeItem *pPolarItem = pItem->parentItem();
        ObjectTreeItem *pFoilItem = pPolarItem->parentItem();
        m_pFoil  = Objects2d::foil(pFoilItem->name());
        m_pPolar = Objects2d::getPolar(m_pFoil, pPolarItem->name());
        m_pOpp   = Objects2d::getOpp(m_pFoil, m_pPolar, pItem->name().toDouble());
        strong = pItem->name();
    }


    if(m_Selection==xfl::OPPOINT && m_pOpp)
        s_pMainFrame->m_pCurrentOppMenu->exec(pEvent->globalPos());
    else if(m_Selection==xfl::POLAR && m_pPolar)
        s_pMainFrame->m_pCurrentPolarMenu->exec(pEvent->globalPos());
    else if(m_Selection==xfl::FOIL && m_pFoil)
        s_pMainFrame->m_pCurrentFoilMenu->exec(pEvent->globalPos());
    s_pXDirect->updateView();
    update();

    pEvent->accept();
}


QString FoilTreeView::removeFoil(Foil* pFoil)
{
    if(!pFoil) return "false";
    return removeFoil(pFoil->name());
}


QString FoilTreeView::removeFoil(QString foilName)
{
    if(!foilName.length()) return "";

    m_pStruct->selectionModel()->blockSignals(true);

    for(int ir = 0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);
        // scan
        if(pItem->name().compare(foilName)==0)
        {
            // plane found
            m_pModel->removeRow(ir);
            break;
        }
    }
    m_pStruct->selectionModel()->blockSignals(false);

    setOverallCheckStatus();
    return QString();
}


QString FoilTreeView::removePolar(Polar *pPolar)
{
    if(!pPolar) return QString();

    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        // find the polar's parent Plane
        if(pFoilItem->name().compare(pPolar->foilName())==0)
        {
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pFoilItem->child(jr);

                if(pOldPolarItem)
                {
                    if(pOldPolarItem->name().compare(pPolar->polarName(), Qt::CaseInsensitive)==0)
                    {
                        m_pModel->removeRow(jr, planeindex);
//                        pPlaneItem->removeRow(jr);
                        m_pStruct->selectionModel()->blockSignals(false);

                        // find the previous item, or the next one if this polar is the first
                        if(pFoilItem->rowCount())
                        {
                            jr =std::min(jr, pFoilItem->rowCount()-1);
                            return pFoilItem->child(jr)->name();
                        }
                        return "";
                    }
                }
            }
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
    return QString(); /** @todo need to do better than that */
}


void FoilTreeView::removeOpPoint(OpPoint *pOpp)
{
    if(!pOpp) return;
    m_pStruct->selectionModel()->blockSignals(true);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pFoilItem = m_pModel->item(ir);
        // find the polar's parent foil
        if(pFoilItem->name().compare(pOpp->foilName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pFoilItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pFoilItem->child(jr);
                if(pPolarItem && pPolarItem->name().compare(pOpp->polarName(), Qt::CaseInsensitive)==0)
                {
                    //find the POpp item
                    for(int kr=0; kr<pPolarItem->rowCount(); kr++)
                    {
                        QModelIndex polarindex = m_pModel->index(pFoilItem, pPolarItem);
                        ObjectTreeItem *pOppItem = pPolarItem->child(kr);
                        if(pOppItem)
                        {
                            double aoa = pOppItem->name().toDouble();
                            if(fabs(aoa-pOpp->aoa())<0.0005)
                            {
                                m_pModel->removeRow(kr, polarindex);
                                m_pStruct->selectionModel()->blockSignals(false);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    setOverallCheckStatus();
    m_pStruct->selectionModel()->blockSignals(false);
}


void FoilTreeView::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Delete:
        {
            if     (m_Selection==xfl::OPPOINT) s_pXDirect->onDeleteCurOpp();
            else if(m_Selection==xfl::POLAR)   s_pXDirect->onDeleteCurPolar();
            else if(m_Selection==xfl::FOIL)    s_pXDirect->onDeleteCurFoil();

            pEvent->accept();
            break;
        }
        default:
            pEvent->ignore();
    }
    if(!pEvent->isAccepted())
        s_pXDirect->keyPressEvent(pEvent);
}


/** update the line properties for each polar and opp item in the treeview */
void FoilTreeView::setCurveParams()
{
    ObjectTreeItem const *pRootItem = m_pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        ObjectTreeItem *pFoilItem = pRootItem->child(i0);
        if(!pFoilItem) continue;
        Foil *pFoil = Objects2d::foil(pFoilItem->name());
        if(!pFoil) continue;

        LineStyle ls(pFoil->theStyle());
        ls.m_bIsEnabled = true;
        pFoilItem->setTheStyle(ls);
        pFoilItem->setCheckState(foilState(pFoil));

        for(int i1=0; i1< pFoilItem->rowCount(); i1++)
        {
            ObjectTreeItem *pPolarItem = pFoilItem->child(i1);
            if(pPolarItem)
            {
                Polar *pPolar = Objects2d::getPolar(pFoilItem->name(), pPolarItem->name());
                if(!pPolar) continue;

                LineStyle ls(pPolar->theStyle());
                ls.m_bIsEnabled = true;
                pPolarItem->setTheStyle(ls);

                if(s_pXDirect->isPolarView())
                {
                    bool bCheck = pPolar->isVisible();
                    bCheck = bCheck && (s_pXDirect->isPolarView() || s_pXDirect->isOppView()) ;
                    pPolarItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                }
                else
                {
                    pPolarItem->setCheckState(polarState(pPolar));
                }

                for(int i2=0; i2<pPolarItem->rowCount(); i2++)
                {
                    ObjectTreeItem *pOppItem = pPolarItem->child(i2);
                    if(pOppItem)
                    {
                        if(!pOppItem) continue;

                        double val = pOppItem->name().toDouble();
                        OpPoint *pOpp = Objects2d::getOpp(pFoil, pPolar, val);

                        if(!pOpp) continue;

                        LineStyle ls(pOpp->theStyle());
                        ls.m_bIsEnabled = s_pXDirect->isOppView();
                        pOppItem->setTheStyle(ls);

                        bool bCheck = pOpp->isVisible()&& s_pXDirect->isOppView();
                        pOppItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                    }
                }
            }
        }
    }
    m_pModel->updateData();
}


Qt::CheckState FoilTreeView::checkState(Foil *pFoil)
{
    //check if all foil polars are visible or not
    bool bAllVisible = false;
    bool bAllHidden = false;
    for(int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName().compare(pFoil->name())==0)
        {
            bAllVisible = bAllVisible && pPolar->isVisible();
            bAllHidden = bAllHidden && !pPolar->isVisible();
        }
    }

    if    (bAllVisible) return Qt::Checked;
    else if(bAllHidden) return Qt::Unchecked;
    return Qt::PartiallyChecked;
}


/** @todo */
Qt::CheckState FoilTreeView::checkState(Polar *pPolar)
{
    //check if all polar oppoints are visible or not
    bool bAllVisible = false;
    bool bAllHidden = false;
    for(int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pPolar->hasOpp(pOpp))
        {
            bAllVisible = bAllVisible && pOpp->isVisible();
            bAllHidden = bAllHidden && !pOpp->isVisible();
        }
    }

    if    (bAllVisible) return Qt::Checked;
    else if(bAllHidden) return Qt::Unchecked;
    return Qt::PartiallyChecked;
}


void FoilTreeView::setTreeFontStruct(const FontStruct &fntstruct)
{
    m_pStruct->setFont(fntstruct.font());
    m_pDelegate->setTreeFontStruct(fntstruct);
}


void FoilTreeView::onSwitchAll(bool bChecked)
{
    if(s_pXDirect->isOppView())
    {
        if(bChecked) s_pXDirect->onShowAllOpps();
        else         s_pXDirect->onHideAllOpps();
    }
    else if(s_pXDirect->isPolarView())
    {
        if(bChecked) s_pXDirect->onShowAllPolars();
        else         s_pXDirect->onHideAllPolars();
    }
    s_pXDirect->resetCurves();
    s_pXDirect->updateView();
    m_pModel->updateData();
}


void FoilTreeView::setOverallCheckStatus()
{
    if(s_pXDirect->isOppView())
    {
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects2d::oppCount(); io++)
        {
            OpPoint *const pOpp=Objects2d::oppAt(io);
            if(pOpp->isVisible()) bAllUnchecked = false;
            else                  bAllChecked   = false;
        }

        if     (bAllChecked)   m_pStruct->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pStruct->setOverallCheckedState(Qt::Unchecked);
        else                   m_pStruct->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else if(s_pXDirect->isPolarView())
    {
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects2d::polarCount(); io++)
        {
            Polar *const pPolar=Objects2d::polarAt(io);
            if(pPolar->isVisible()) bAllUnchecked = false;
            else                    bAllChecked   = false;
        }

        if     (bAllChecked)   m_pStruct->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pStruct->setOverallCheckedState(Qt::Unchecked);
        else                   m_pStruct->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else
    {
        m_pStruct->setOverallCheckedState(Qt::Unchecked);
    }
}


void FoilTreeView::onSetFilter()
{
    QString filter = m_pStruct->filter();
    QStringList filters = filter.split(QRegExp("\\s+"));

    if(filters.size()==0)
    {
        for(int jp=0; jp<Objects2d::polarCount(); jp++)
        {
            Objects2d::polarAt(jp)->setVisible(true);
        }
    }
    else if(filters.size()==1)
    {
        for(int jp=0; jp<Objects2d::polarCount(); jp++)
        {
            Polar *pPolar = Objects2d::polarAt(jp);
            bool bVisible = pPolar->name().contains(filter, Qt::CaseInsensitive);
            pPolar->setVisible(bVisible);
        }

        for(int ip=0; ip<Objects2d::foilCount(); ip++)
        {
            Foil const *pFoil = Objects2d::foilAt(ip);
            if(pFoil->name().contains(filter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects2d::polarCount(); jp++)
                {
                    Polar *pPolar = Objects2d::polarAt(jp);
                    if(pPolar->foilName()==pFoil->name())
                        pPolar->setVisible(true);
                }
            }
        }
    }
    else
    {
        QString foilfilter = filters.first();
        QString polarfilter = filters.at(1);
        for(int jp=0; jp<Objects2d::polarCount(); jp++)
        {
            Objects2d::polarAt(jp)->setVisible(false);
        }

        for(int ip=0; ip<Objects2d::foilCount(); ip++)
        {
            Foil const *pFoil = Objects2d::foilAt(ip);
            if(pFoil->name().contains(foilfilter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects2d::polarCount(); jp++)
                {
                    Polar *pPolar = Objects2d::polarAt(jp);
                    if(pPolar->foilName()==pFoil->name())
                    {
                        bool bVisible = pPolar->name().contains(polarfilter, Qt::CaseInsensitive);
                        pPolar->setVisible(bVisible);
                    }
                }
            }
        }
    }

    setCurveParams();

    m_pModel->updateData();
    setOverallCheckStatus();

    s_pXDirect->resetCurves();
    s_pXDirect->updateView();

    update();
}





