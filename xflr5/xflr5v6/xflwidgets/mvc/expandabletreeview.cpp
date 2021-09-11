/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>

#include "expandabletreeview.h"
#include <xflwidgets/customwts/crosscheckbox.h>
#include <xflwidgets/mvc/objecttreeitem.h>
#include <xflwidgets/mvc/objecttreemodel.h>


FontStruct ExpandableTreeView::s_TreeFontStruct;

ExpandableTreeView::ExpandableTreeView(QWidget *pParent) : QTreeView(pParent)
{
    initETV();

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setUniformRowHeights(true);
    setRootIsDecorated(true);
}


void ExpandableTreeView::initETV()
{
//    m_pCollapseAll  = new QAction(QIcon(":/icons/level--.png"), tr("Collapse all"),           this);
//    m_pExpandAll    = new QAction(QIcon(":/icons/level++.png"), tr("Expand all"),             this);
    m_pLevelMinus   = new QAction(QIcon(":/images/level-.png"),  tr("Collapse selected item"), this);
    m_pLevelPlus    = new QAction(QIcon(":/images/level+.png"),  tr("Expand selected item"),   this);
    m_pLevel0Action = new QAction(QIcon(":/images/level0.png"),  tr("Object level"),           this);
    m_pLevel1Action = new QAction(QIcon(":/images/level1.png"),  tr("Polar level"),            this);
    m_pLevel2Action = new QAction(QIcon(":/images/level2.png"),  tr("Operating Point level"),  this);

    m_pfrControls = new QFrame;
    {
        m_pleFilter = new QLineEdit;
        m_pleFilter->setClearButtonEnabled(true);
        m_pleFilter->setToolTip("Enter here the words to be used as filters for objects and polars and press Enter. "
                                "This will apply a filter to the polar graphs.<br>"
                                "If only one word is entered it will be used to filter objects OR polars.<br>"
                                "If two words are entered, e.g. \'planefilter T2\' the first word will be "
                                "used to filter planes AND the second will be used to filter polars.<br>"
                                "The filter is case-insensitive.");
        m_pchHideShowAll = new CrossCheckBox;
        m_pchHideShowAll->setToolTip("Hide or show all polars or operating point objects in the graphs.");
        QHBoxLayout *pHLayout = new QHBoxLayout;
        {
            QToolButton *pLevelPlus   = new QToolButton;
            QToolButton *pLevelMinus  = new QToolButton;
            QToolButton *pLevel0      = new QToolButton;
            QToolButton *pLevel1      = new QToolButton;
            QToolButton *pLevel2      = new QToolButton;

            pLevelPlus->setDefaultAction(m_pLevelPlus);
            pLevelMinus->setDefaultAction(m_pLevelMinus);
            pLevel0->setDefaultAction(m_pLevel0Action);
            pLevel1->setDefaultAction(m_pLevel1Action);
            pLevel2->setDefaultAction(m_pLevel2Action);

            pHLayout->addStretch();
            pHLayout->addWidget(pLevel0);
            pHLayout->addWidget(pLevel1);
            pHLayout->addWidget(pLevel2);
            pHLayout->addStretch();
            pHLayout->addWidget(pLevelMinus);
            pHLayout->addWidget(pLevelPlus);
            pHLayout->addStretch();
            pHLayout->addWidget(m_pleFilter);
            pHLayout->addWidget(m_pchHideShowAll);
        }
        m_pfrControls->setLayout(pHLayout);
    }

    connect(m_pLevel0Action,  SIGNAL(triggered(bool)), SLOT(onObjectLevel()));
    connect(m_pLevel1Action,  SIGNAL(triggered(bool)), SLOT(onPolarLevel()));
    connect(m_pLevel2Action,  SIGNAL(triggered(bool)), SLOT(onOpPointLevel()));
    connect(m_pLevelMinus,    SIGNAL(triggered(bool)), SLOT(onLevelMinus()));
    connect(m_pLevelPlus,     SIGNAL(triggered(bool)), SLOT(onLevelPlus()));

    connect(m_pchHideShowAll, SIGNAL(clicked(bool)),   SLOT(onHideShowAll(bool)));
}


/**
 * Collapse all items up to plane/foil level
 */
void ExpandableTreeView::onObjectLevel()
{
    QModelIndex currentIndex = this->currentIndex();
    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    ObjectTreeItem *pRootItem = pModel->rootItem();

    if(pCurItem)
    {
        while (pCurItem->level()>1)
        {
            pCurItem=pCurItem->parentItem();
        }
    }

    collapseAll();

    if(pCurItem)
    {
        QModelIndex newIndex = pModel->index(pRootItem, pCurItem);
        setCurrentIndex(newIndex);
        scrollTo(newIndex);
    }
}


void ExpandableTreeView::onPolarLevel()
{
    QModelIndex currentIndex = this->currentIndex();
    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    int itemDepth=0;
    if(pCurItem)
    {
        while (pCurItem->level()>1)
        {
            itemDepth++;
            pCurItem=pCurItem->parentItem();
        }
    }

    //collapse and expand all level 1
    collapseAll();
    ObjectTreeItem *pRootItem = pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex foilindex = pModel->index(i0,0, pRootItem);
        expand(foilindex);
    }


    if(pCurItem)
    {
        QModelIndex ind = pModel->index(0,0,pRootItem);
        expand(ind);
    }

    // select the appropriate index
    // there is no function to tell us the current selection's depth nor if it is hidden unfortunately
    QModelIndex newIndex;
    if(itemDepth==2)
    {
        newIndex = currentIndex.parent();
    }
    else
    {
        newIndex = currentIndex;
    }
    setCurrentIndex(newIndex);
    scrollTo(newIndex);
}


void ExpandableTreeView::onOpPointLevel()
{
    QModelIndex currentIndex = this->currentIndex();

    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());
    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);

    //expand all levels
    ObjectTreeItem *pRootItem = pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex objectindex = pModel->index(i0,0);
        expand(objectindex);
        ObjectTreeItem *pObjectItem = pModel->itemFromIndex(objectindex);
        for(int i1=0; i1< pObjectItem->rowCount(); i1++)
        {
            QModelIndex polarindex = pModel->index(i1,0,pObjectItem);
            if(polarindex.isValid())
            {
                expand(polarindex);
            }
        }
    }
    if(pCurItem)
    {
        QModelIndex ind = pModel->index(0,0,pCurItem);
        setCurrentIndex(ind);
        scrollTo(ind);
    }
}


/**
 * Expand the selected index
 */
void ExpandableTreeView::onLevelPlus()
{
    QModelIndex curidx = currentIndex();
    if(curidx.isValid())
    {
        if(!isExpanded(curidx)) expand(curidx);
        else
        {
            ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

            ObjectTreeItem *pItem = pModel->itemFromIndex(curidx);
            for(int iRow=0; iRow<pItem->rowCount(); iRow++)
            {
                QModelIndex idx = pModel->index(iRow, 0, pItem);

                if(idx.isValid()) expand(idx);
                else
                {
                    qDebug()<<"invvvalllid index";
                }
            }
        }
    }
}


/**
 * collapse the parent of the current index
 */
void ExpandableTreeView::onLevelMinus()
{
    QModelIndex currentIndex = this->currentIndex();

    ObjectTreeModel *pModel = dynamic_cast<ObjectTreeModel*>(model());

    ObjectTreeItem *pCurItem = pModel->itemFromIndex(currentIndex);
    // find the top level item
    int itemDepth=0;
    if(!pCurItem) return;
    while (pCurItem->level()>1)
    {
        itemDepth++;
        pCurItem=pCurItem->parentItem();
    }
    //find the depth of expansion relative to the current item
    int expandeddepth =1;
    for(int iRow=0; iRow<pCurItem->rowCount(); iRow++)
    {
        ObjectTreeItem *pChildItem = pCurItem->child(iRow);
        QModelIndex childIndex = pModel->index(iRow, 0, pCurItem);
        if(pChildItem && pChildItem->rowCount()>0 && isExpanded(childIndex))
        {
            expandeddepth = 2;
        }
    }

    if(expandeddepth==2)
    {
        for(int iRow=0; iRow<pCurItem->rowCount(); iRow++)
        {
            QModelIndex childIndex = pModel->index(iRow, 0, pCurItem);
            collapse(childIndex);
        }
        expandeddepth--;
    }
    else if(expandeddepth==1)
    {
        QModelIndex index = pModel->index(pModel->rootItem(), pCurItem);
        collapse(index);
        expandeddepth--;
    }

    // select the appropriate index
    // there is no function to tell us the current selection's depth nor if it is hidden unfortunately
    QModelIndex newIndex;
    if(itemDepth==2)
    {
        if(expandeddepth==1) newIndex = currentIndex.parent();
        else                 newIndex = currentIndex.parent().parent();
    }
    else if (itemDepth==1)
    {
        if      (expandeddepth==1) newIndex = currentIndex;
        else if (expandeddepth==0) newIndex = currentIndex.parent();
    }
    else
    {
        newIndex = currentIndex;
    }
    setCurrentIndex(newIndex);
    scrollTo(newIndex);
}


QSize ExpandableTreeView::sizeHint() const
{
    int w = 19 * s_TreeFontStruct.averageCharWidth();
    int h = s_TreeFontStruct.height();
    return QSize(w, 25*h);
}


int ExpandableTreeView::sizeHintForColumn(int column) const
{
    if      (column==0) return 10*s_TreeFontStruct.averageCharWidth();
    else if (column==2) return  2*s_TreeFontStruct.averageCharWidth();
    else if (column==1) return  5*s_TreeFontStruct.averageCharWidth();
    return 5*s_TreeFontStruct.averageCharWidth();
}


void ExpandableTreeView::setOverallCheckedState(Qt::CheckState state)
{
    m_pchHideShowAll->setCheckState(state);
}


void ExpandableTreeView::onHideShowAll(bool bChecked)
{
    if(m_pchHideShowAll->checkState()==Qt::PartiallyChecked)
        m_pchHideShowAll->setCheckState(Qt::Checked); // prevent the partially checked state when clicking

    m_pleFilter->clear();
    emit switchAll(bChecked);
}


void ExpandableTreeView::enableSelectBox(bool bEnable)
{
    if(m_pchHideShowAll) m_pchHideShowAll->setEnabled(bEnable);
}

