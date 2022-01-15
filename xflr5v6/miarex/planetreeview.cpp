/****************************************************************************

        xflr5 v6
        Copyright (C) Andre Deperrois
        GNU General Public License v3

*****************************************************************************/

#include <QVBoxLayout>
#include <QHeaderView>

#include "planetreeview.h"

#include <globals/mainframe.h>
#include <miarex/miarex.h>

#include <miarex/view/stabviewdlg.h>
#include <xflcore/mathelem.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/planeopp.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customwts/plaintextoutput.h>
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/mvc/expandabletreeview.h>
#include <xflwidgets/mvc/objecttreedelegate.h>
#include <xflwidgets/mvc/objecttreeitem.h>
#include <xflwidgets/mvc/objecttreemodel.h>


MainFrame *PlaneTreeView::s_pMainFrame = nullptr;
Miarex *PlaneTreeView::s_pMiarex = nullptr;
QByteArray PlaneTreeView::s_SplitterSizes;

PlaneTreeView::PlaneTreeView(QWidget *pParent) : QWidget(pParent)
{
    m_pTreeView = nullptr;
    m_pModel  = nullptr;

    m_Selection = xfl::NOOBJECT;
    m_pptObjectProps = new PlainTextOutput;

    setupLayout();

    m_pModel = new ObjectTreeModel(this);
    m_pModel->setHeaderData(0, Qt::Horizontal, "Objects", Qt::DisplayRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::EditRole);
    m_pModel->setHeaderData(1, Qt::Horizontal, "1234567890123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, "123", Qt::DisplayRole);
    m_pModel->setHeaderData(2, Qt::Horizontal, Qt::AlignRight, Qt::TextAlignmentRole);

    m_pTreeView->setModel(m_pModel);

    m_pTreeView->setRootIndex(QModelIndex());

    m_pTreeView->header()->hide();
    m_pTreeView->header()->setStretchLastSection(false);
    m_pTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pTreeView->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_pTreeView->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    int av = m_pTreeView->treeFontStruct().averageCharWidth();
#ifdef Q_OS_WIN
    m_pTreeView->header()->resizeSection(1, 11*av);
    m_pTreeView->header()->resizeSection(2, 5*av);
#else
    m_pTreeView->header()->resizeSection(1, 7*av);
    m_pTreeView->header()->resizeSection(2, 3*av);
#endif
    m_pDelegate = new ObjectTreeDelegate(this);
    m_pTreeView->setItemDelegate(m_pDelegate);

    connect(m_pTreeView, SIGNAL(pressed(QModelIndex)),         SLOT(onItemClicked(QModelIndex)));
    connect(m_pTreeView, SIGNAL(doubleClicked(QModelIndex)),   SLOT(onItemDoubleClicked(QModelIndex)));
    connect(m_pTreeView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), SLOT(onCurrentRowChanged(QModelIndex)));
    connect(m_pTreeView->m_pleFilter, SIGNAL(returnPressed()), SLOT(onSetFilter()));
}


PlaneTreeView::~PlaneTreeView()
{
}


void PlaneTreeView::showEvent(QShowEvent *pEvent)
{
    m_pMainSplitter->restoreState(s_SplitterSizes);
    QWidget::showEvent(pEvent);
}


void PlaneTreeView::hideEvent(QHideEvent *pEvent)
{
    s_SplitterSizes = m_pMainSplitter->saveState();
    QWidget::hideEvent(pEvent);
}


void PlaneTreeView::setObjectProperties()
{
    QString lenlab, arealab, masslab, speedlab;
    Units::getLengthUnitLabel(lenlab);
    Units::getAreaUnitLabel(arealab);
    Units::getMassUnitLabel(masslab);
    Units::getSpeedUnitLabel(speedlab);
    Plane const *pPlane = s_pMiarex->m_pCurPlane;
    QString props;
    switch(m_Selection)
    {
        case xfl::PLANE:
        {
            if(pPlane)
            {
                if(pPlane->description().length())
                {
                    props = pPlane->description() + "\n\n";
                }
                props += pPlane->planeData(false);
            }
            break;
        }
        case xfl::WPOLAR:
        {
            if(s_pMiarex->m_pCurWPolar && pPlane)
            {


                s_pMiarex->m_pCurWPolar->getProperties(props, pPlane,
                                                       Units::mtoUnit(), Units::kgtoUnit(), Units::mstoUnit(), Units::m2toUnit(),
                                                       lenlab, masslab, speedlab, arealab);
                break;
            }
            break;
        }
        case xfl::PLANEOPP:
        {
            if(s_pMiarex->m_pCurPOpp)
            {
                s_pMiarex->m_pCurPOpp->getProperties(props, lenlab, masslab, speedlab,
                                                     Units::mtoUnit(), Units::kgtoUnit(), Units::mstoUnit());
                        break;
            }
            break;
        }
        case xfl::STABILITYMODE:
        {
            if(s_pMiarex->m_pCurPOpp)
            {
                fillEigenThings(props);
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
    m_pptObjectProps->setPlainText(props);
}


void PlaneTreeView::setPropertiesFont(QFont const &fnt)
{
    m_pptObjectProps->setFont(fnt);
}


void PlaneTreeView::setTreeFontStruct(const FontStruct &fntstruct)
{
    m_pTreeView->setFont(fntstruct.font());
    m_pDelegate->setTreeFontStruct(fntstruct);
    //    setFont(fntstruct.font());
}


void PlaneTreeView::fillEigenThings(QString &props)
{
    StabViewDlg *pStabView = s_pMainFrame->m_pStabView;
    if(pStabView->m_iCurrentMode<0 || pStabView->m_iCurrentMode>8)
    {
        props.clear();
        return;
    }
    std::complex<double> eigenvalue;
    double OmegaN, Omega1, Dsi;
    QString strange;
//    double u0, mac, span;

    OmegaN = Omega1 = Dsi = 0;
//    double u0 = mac = span = 0;

    QString ModeDescription;
    if(pStabView->m_iCurrentMode<5) ModeDescription = QString::asprintf("Longitudinal mode %d\n", pStabView->m_iCurrentMode+1);
    else                            ModeDescription = QString::asprintf("Lateral mode %d\n",      pStabView->m_iCurrentMode-4+1);

    if(s_pMiarex->m_pCurPlane && s_pMiarex->m_pCurPOpp && s_pMiarex->m_pCurWPolar->isStabilityPolar())
    {
        //We normalize the mode before display and only for display purposes
//        u0   = s_pMiarex->m_pCurPOpp->QInf();
//        mac  = s_pMiarex->m_pCurPlane->mac();
//        span = s_pMiarex->m_pCurPlane->planformSpan();

        eigenvalue = s_pMiarex->m_pCurPOpp->m_EigenValue[pStabView->m_iCurrentMode];
        if(eigenvalue.imag()>=0.0) strange = QString::asprintf("   Lambda=%9.4f+%9.4fi\n", eigenvalue.real(), eigenvalue.imag());
        else                       strange = QString::asprintf("   Lambda=%9.4f-%9.4fi\n", eigenvalue.real(), eigenvalue.imag());

        ModeDescription.append(strange);

        modeProperties(eigenvalue, OmegaN, Omega1, Dsi);
        //		Omega1 = qAbs(eigenvalue.imag());
        //		OmegaN = sqrt(eigenvalue.real()*eigenvalue.real()+Omega1*Omega1);
        //		Dsi = -eigenvalue.real()/Omega1;


        if(Omega1>PRECISION)
        {
            strange = QString::asprintf("   Fd=%6.3f Hz\n", Omega1/2.0/PI);
            ModeDescription.append(strange);
        }

        if(Omega1 > PRECISION)
        {
            strange = QString::asprintf("   FN=%6.3f Hz\n",OmegaN/2.0/PI);
            ModeDescription.append(strange);
            strange = QString::asprintf("   Xi=%6.3f\n",Dsi);
            ModeDescription.append(strange);
        }

        if(fabs(eigenvalue.real())>PRECISION && fabs(eigenvalue.imag())<PRECISION)
        {
            strange = QString::asprintf("   T2=%6.3f s\n", log(2)/fabs(eigenvalue.real()));
            if(eigenvalue.real()<0.0)
            {
                strange = QString::asprintf("   tau=%6.3f\n", -1.0/eigenvalue.real());
            }
        }

        props = ModeDescription;
    }
}


void PlaneTreeView::setupLayout()
{
    m_pTreeView = new ExpandableTreeView;
    m_pTreeView->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    m_pTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_pTreeView->setUniformRowHeights(true);
    m_pTreeView->setRootIsDecorated(true);
    m_pTreeView->setMinimumHeight(100);
    connect(m_pTreeView, SIGNAL(switchAll(bool)), SLOT(onSwitchAll(bool)));

    m_pMainSplitter = new QSplitter;
    connect(m_pMainSplitter, SIGNAL(splitterMoved(int,int)), SLOT(onSplitterMoved()));
    m_pMainSplitter->setOrientation(Qt::Vertical);
    {
        m_pMainSplitter->addWidget(m_pTreeView);
        m_pMainSplitter->addWidget(m_pptObjectProps);
    }
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pTreeView->cmdWidget());
        pMainLayout->addWidget(m_pMainSplitter);
    }
    setLayout(pMainLayout);
}


void PlaneTreeView::fillModelView()
{
    m_pModel->removeRows(0, m_pModel->rowCount());

    ObjectTreeItem *pRootItem = m_pModel->rootItem();

    for(int iPlane=0; iPlane<Objects3d::planeCount(); iPlane++)
    {
        Plane const *pPlane = Objects3d::planeAt(iPlane);
        if(!pPlane) continue;

        LineStyle ls(pPlane->theStyle());
        ObjectTreeItem *pPlaneItem = m_pModel->appendRow(pRootItem, pPlane->name(), pPlane->theStyle(), planeState(pPlane));

        fillWPolars(pPlaneItem, pPlane);
    }
}


/** updates the plane items after the WPolars or PlaneOpps have been changed, deleted or something */
void PlaneTreeView::updatePlane(const Plane *pPlane)
{
    if(!pPlane) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        if(pPlaneItem->name().compare(pPlane->name())==0)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(planeindex);
            if(!pItem) continue;
            m_pModel->blockSignals(true);
            m_pModel->removeRows(0, pItem->rowCount(), planeindex);
            m_pModel->blockSignals(false);

            fillWPolars(pPlaneItem, pPlane);
            break;
        }
    }
}


void PlaneTreeView::fillWPolars(ObjectTreeItem *pPlaneItem, const Plane *pPlane)
{
    if(!pPlane || !pPlaneItem) return;

    for(int iPolar=0; iPolar<Objects3d::polarCount(); iPolar++)
    {
        WPolar *pWPolar = Objects3d::polarAt(iPolar);
        if(!pWPolar) continue;
        if(pWPolar && pWPolar->planeName().compare(pPlane->name())==0)
        {
            LineStyle ls(pWPolar->theStyle());
            ls.m_bIsEnabled = true;
            m_pModel->appendRow(pPlaneItem, pWPolar->name(), ls, wPolarState(pWPolar));

            addPOpps(pWPolar);
        }
    }
}


void PlaneTreeView::addPOpps(WPolar const *pWPolar)
{
    if(!pWPolar) pWPolar = s_pMiarex->curWPolar();
    if(!pWPolar) return;

    bool bAdded(false);
//    m_pModel->blockSignals(true);

    //find this polar's plane parent
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pWPolarItem = pPlaneItem->child(jr);
                if(pWPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    if(pWPolarItem->rowCount())
                        m_pModel->removeRows(0, pWPolarItem->rowCount(), polarindex);

                    for(int iOpp=0; iOpp<Objects3d::planeOppCount(); iOpp++)
                    {
                        PlaneOpp const *pPOpp = Objects3d::planeOppAt(iOpp);
                        if(pPOpp->planeName().compare(pWPolar->planeName())==0 && pPOpp->polarName().compare(pWPolar->polarName())==0)
                        {
                            QString strange = pPOpp->name();
                            ObjectTreeItem *pPOppItem = nullptr;
                            if(s_pMiarex->isPOppView())
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = true;
                                pPOppItem = m_pModel->appendRow(pWPolarItem, strange, ls, pPOpp->isVisible() ? Qt::Checked : Qt::Unchecked);
                            }
                            else
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = false;
                                pPOppItem = m_pModel->appendRow(pWPolarItem, strange, ls, Qt::PartiallyChecked);
                            }
                            if(pPOpp->isT7Polar() && pPOppItem)
                            {
                                for(int iMode=0; iMode<8; iMode++)
                                {
                                    m_pModel->appendRow(pPOppItem, QString::asprintf("Mode %d", iMode+1), LineStyle(), Qt::Unchecked);
                                }
                            }
                        }
                    }
                    bAdded = true;
                    break;
                }
            }
        }
        if(bAdded) break;
    }

//    m_pModel->blockSignals(false);

    setOverallCheckStatus();
}


void PlaneTreeView::onCurrentRowChanged(QModelIndex currentfilteredidx)
{
    setObjectFromIndex(currentfilteredidx);
    s_pMiarex->updateView();
}


void PlaneTreeView::onItemClicked(const QModelIndex &index)
{
    PlaneOpp *m_pPOpp = s_pMiarex->m_pCurPOpp;
    WPolar *m_pWPolar = s_pMiarex->m_pCurWPolar;
    Plane* m_pPlane = s_pMiarex->m_pCurPlane;

    if(index.column()==1)
    {
        ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

        if(m_pPOpp)
        {
            if(s_pMiarex->isPOppView())
            {
                LineStyle ls(m_pPOpp->theStyle());
                LineMenu *pLineMenu = new LineMenu(nullptr);
                pLineMenu->initMenu(ls);
                pLineMenu->exec(QCursor::pos());
                ls = pLineMenu->theStyle();
                m_pPOpp->setLineStipple(ls.m_Stipple);
                m_pPOpp->setLineWidth(ls.m_Width);
                m_pPOpp->setLineColor(ls.m_Color);
                m_pPOpp->setPointStyle(ls.m_Symbol);
                pItem->setTheStyle(ls);
                s_pMiarex->resetCurves();
                emit s_pMiarex->projectModified();
            }
        }
        else if(m_pWPolar)
        {
            LineStyle ls(m_pWPolar->theStyle());
            LineMenu *pLineMenu = new LineMenu(nullptr);
            pLineMenu->initMenu(ls);
            pLineMenu->exec(QCursor::pos());
            ls = pLineMenu->theStyle();

            Objects3d::setWPolarStyle(m_pWPolar, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
            setCurveParams();

            if(pItem) pItem->setTheStyle(ls);
            s_pMiarex->resetCurves();
            emit s_pMiarex->projectModified();
        }
        else if(m_pPlane)
        {
            if(!s_pMiarex->is3dView())
            {
                LineStyle ls(m_pPlane->theStyle());
                LineMenu *pLineMenu = new LineMenu(nullptr);
                pLineMenu->initMenu(ls);
                pLineMenu->exec(QCursor::pos());
                ls = pLineMenu->theStyle();

                Objects3d::setPlaneStyle(m_pPlane, ls, pLineMenu->styleChanged(), pLineMenu->widthChanged(), pLineMenu->colorChanged(), pLineMenu->pointsChanged());
                setCurveParams();

                if(pItem) pItem->setTheStyle(ls);
                s_pMiarex->resetCurves();
                emit s_pMiarex->projectModified();

                s_pMiarex->resetCurves();
            }
        }
    }
    else if (index.column()==2)
    {
        if(m_pPOpp)
        {
            if(s_pMiarex->isPOppView())
            {
                ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
                if(pItem)
                {
                    m_pPOpp->setVisible(!m_pPOpp->isVisible());
                    setCurveParams();
                    s_pMiarex->resetCurves();
                    emit s_pMiarex->projectModified();
                }
            }
        }
        else if(m_pWPolar)
        {
            ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
            if(pItem)
            {
                Qt::CheckState state = wPolarState(m_pWPolar);
                if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                    Objects3d::setWPolarVisible(m_pWPolar, true);
                else
                    Objects3d::setWPolarVisible(m_pWPolar, false);

                setCurveParams();
                s_pMiarex->resetCurves();
                emit s_pMiarex->projectModified();
            }
        }
        else if(m_pPlane)
        {
            if(!s_pMiarex->is3dView())
            {
                ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);
                if(pItem)
                {
                    Qt::CheckState state = planeState(m_pPlane);
                    if(state==Qt::PartiallyChecked || state==Qt::Unchecked)
                        Objects3d::setPlaneVisible(m_pPlane, true,  s_pMiarex->isStabPolarView());
                    else if(state==Qt::Checked)
                        Objects3d::setPlaneVisible(m_pPlane, false, s_pMiarex->isStabPolarView());

                    setCurveParams();
                    s_pMiarex->resetCurves();
                    emit s_pMiarex->projectModified();
                }
            }
        }
        setOverallCheckStatus();
    }

    m_pModel->updateData();
    s_pMiarex->setControls();
    s_pMiarex->updateView();

    update();
}


void PlaneTreeView::onItemDoubleClicked(const QModelIndex &index)
{
    setObjectFromIndex(index);

    s_pMiarex->setControls();
    s_pMiarex->updateView();
/*    if(index.column()==0)
    {
        if(m_Selection==xfl::PLANE)
        {
            s_pMiarex->onEditCurPlane();
        }
        else if(m_Selection==xfl::WPOLAR)
        {
            s_pMiarex->onEditCurWPolar();
        }
    } */
}


/**
 * Sets the new current object, a Plane, a WPolar, a PlaneOpp or a Mode
 * from the new index
 */
void PlaneTreeView::setObjectFromIndex(QModelIndex index)
{
    ObjectTreeItem *pSelectedItem = nullptr;

    if(index.column()==0)
    {
        pSelectedItem = m_pModel->itemFromIndex(index);
    }
    else if(index.column()>=1)
    {
        QModelIndex ind = index.siblingAtColumn(0);
        pSelectedItem = m_pModel->itemFromIndex(ind);
    }

    if(!pSelectedItem) return;

    if(pSelectedItem->level()==1)
    {
        s_pMiarex->setPlane(pSelectedItem->name());
        s_pMiarex->m_pCurWPolar = nullptr;
        s_pMiarex->m_pCurPOpp = nullptr;
        m_Selection = xfl::PLANE;
    }
    else if(pSelectedItem->level()==2)
    {
        ObjectTreeItem *pPlaneItem = pSelectedItem->parentItem();
        Plane *pPlane  = Objects3d::plane(pPlaneItem->name());
        WPolar *pWPolar = Objects3d::wPolar(pPlane, pSelectedItem->name());
        if(pPlane!=s_pMiarex->curPlane())
            s_pMiarex->setPlane(pPlane);
        s_pMiarex->setWPolar(pWPolar);
        s_pMiarex->m_pCurPOpp = nullptr;

        m_Selection = xfl::WPOLAR;
    }
    else if(pSelectedItem->level()==3)
    {
        ObjectTreeItem *pWPolarItem = pSelectedItem->parentItem();
        ObjectTreeItem *pPlaneItem = pWPolarItem->parentItem();
        Plane *pPlane  = Objects3d::plane(pPlaneItem->name());
        WPolar *pWPolar = Objects3d::wPolar(pPlane, pWPolarItem->name());

        PlaneOpp *pPOpp   = Objects3d::getPlaneOpp(pPlane, pWPolar, pSelectedItem->name().trimmed().toDouble());

        m_Selection = xfl::PLANEOPP;

        if(pPlane!=s_pMiarex->m_pCurPlane)
        {
            s_pMiarex->setPlane(pPlane);
            s_pMiarex->setWPolar(pWPolar);
        }
        else if(pWPolar != s_pMiarex->m_pCurWPolar) s_pMiarex->setWPolar(pWPolar);
        if(pPOpp)
        {
            s_pMiarex->setPlaneOpp(pPOpp);
            s_pMiarex->resetCurves();
        }
    }
    else if(pSelectedItem->level()==4)
    {
        //three parents, the user has clicked a Mode
        ObjectTreeItem *pPOppItem   = pSelectedItem->parentItem();
        ObjectTreeItem *pWPolarItem = pPOppItem->parentItem();
        ObjectTreeItem *pPlaneItem  = pWPolarItem->parentItem();
        Plane    *pPlane  = Objects3d::plane(pPlaneItem->name());
        WPolar   *pWPolar = Objects3d::wPolar(pPlane, pWPolarItem->name());
        PlaneOpp *pPOpp   = Objects3d::getPlaneOpp(pPlane, pWPolar, pPOppItem->name().toDouble());

        s_pMiarex->setPlane(pPlane);
        s_pMiarex->setWPolar(pWPolar);
        s_pMiarex->setPlaneOpp(pPOpp);

        int iMode = pSelectedItem->name().rightRef(1).toInt()-1;
        if(iMode>=0 && iMode<8)
        {
            m_Selection = xfl::STABILITYMODE;
//            s_pMiarex->setMode(iMode);
            s_pMiarex->resetCurves();
            s_pMiarex->updateView();
        }
    }
    else m_Selection = xfl::NOOBJECT;

    s_pMiarex->setControls();
    setObjectProperties();
}


void PlaneTreeView::insertWPolar(WPolar const *pWPolar)
{
    if(!pWPolar) pWPolar = s_pMiarex->curWPolar();
    if(!pWPolar) return;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);

        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pWPolar->planeName())==0)
        {
            ObjectTreeItem *pNewPolarItem = nullptr;
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pPlaneItem->child(jr);
                if(pOldPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)==0)
                {
                    pNewPolarItem = pOldPolarItem;
                }
                else if(pOldPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)>0)
                {
                    //insert before
//                    pNewPolarItem = pPlaneItem->insertRow(jr, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
                    pNewPolarItem = m_pModel->insertRow(pPlaneItem, jr, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
                }
                if(pNewPolarItem) break;
            }
            if(!pNewPolarItem)
            {
                //append
                pNewPolarItem = m_pModel->appendRow(pPlaneItem, pWPolar->name(), pWPolar->theStyle(), wPolarState(pWPolar));
            }

            if(pNewPolarItem)
            {
                addPOpps(pWPolar);

                // set the curve data
                LineStyle ls(pWPolar->theStyle());
                ls.m_bIsEnabled = (s_pMiarex->isPolarView() || s_pMiarex->isStabilityView());
                pNewPolarItem->setTheStyle(ls);
                pNewPolarItem->setCheckState(wPolarState(pWPolar));
            }
            return;
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::insertPlane(Plane* pPlane)
{
    if(!pPlane) pPlane = s_pMiarex->curPlane();
    if(!pPlane) return;

    bool bInserted = false;
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pItem = m_pModel->item(ir);
        // insert alphabetically
        if(pItem->name().compare(pPlane->name())==0)
        {
            //A Plane of that name already exists
            return;
        }
        else if(pItem->name().compare(pPlane->name(), Qt::CaseInsensitive)>0)
        {
            //insert before
            m_pModel->insertRow(m_pModel->rootItem(), ir, pPlane->name(), pPlane->theStyle(), planeState(pPlane));
//            m_pModel->rootItem()->insertRow(ir, pPlane->planeName(), pPlane->theStyle(), planeState(pPlane));
            bInserted = true;
            break;
        }
    }
    if(!bInserted)
    {
        //not inserted, append
        m_pModel->appendRow(m_pModel->rootItem(), pPlane->name(), pPlane->theStyle(), planeState(pPlane));
    }

    for(int iwp=0; iwp<Objects3d::polarCount(); iwp++)
    {
        WPolar const *pWPolar = Objects3d::polarAt(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            insertWPolar(pWPolar);
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::selectPlane(Plane *pPlane)
{
    if(!pPlane) pPlane = s_pMiarex->m_pCurPlane;
    if(!pPlane) return;

//    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        if(pPlaneItem->name().compare(pPlane->name(), Qt::CaseInsensitive)==0)
        {
            m_Selection = xfl::PLANE;

            QModelIndex ind = m_pModel->index(ir, 0, m_pModel->rootItem());
            if(ind.isValid())
            {
                if(ind.isValid())
                {
                    m_pTreeView->setCurrentIndex(ind);
                    m_pTreeView->selectionModel()->select(ind, QItemSelectionModel::Rows);
                    m_pTreeView->scrollTo(ind);
                }
            }

            break;
        }
    }
//    m_pStruct->selectionModel()->blockSignals(false);
}


void PlaneTreeView::selectWPolar(WPolar *pWPolar, bool bSelectPOpp)
{
    if(!pWPolar) pWPolar = s_pMiarex->curWPolar();
    if(!pWPolar) return;

    //    m_pStruct->selectionModel()->blockSignals(true);

    bool bSelected=false;

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);

        QModelIndex planeindex = m_pModel->index(ir, 0);
        Q_ASSERT(planeindex.isValid());
        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)==0)
                {
                    m_Selection = xfl::WPOLAR;

                    QModelIndex polarindex = m_pModel->index(jr, 0, planeindex);
                    if(polarindex.isValid())
                    {
                        m_pTreeView->setCurrentIndex(polarindex);
                        m_pTreeView->selectionModel()->select(polarindex, QItemSelectionModel::Rows);
                        m_pTreeView->scrollTo(polarindex);

                        bSelected = true;
                    }
                    break;
                }
            }
        }
        if(bSelected) break;
    }
    //    m_pStruct->selectionModel()->blockSignals(false);
    if(bSelectPOpp) selectPlaneOpp();
    setObjectProperties();
    update();
}


void PlaneTreeView::selectPlaneOpp(PlaneOpp *pPOpp)
{
    if(!pPOpp) pPOpp = s_pMiarex->m_pCurPOpp;
    if(!pPOpp) return;

    bool bSelected = false;

    //    m_pStruct->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pPOpp->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                //				const QModelIndex &oldWPolarChild = pPlaneItem->index().child(jr, 0);
                //				PlaneTreeItem *pPolarItem = m_pModel->itemFromIndex(oldWPolarChild);
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);

                if(pPolarItem->name().compare(pPOpp->polarName(), Qt::CaseInsensitive)==0)
                {
                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        //						const QModelIndex &poppChild = pPolarItem->index().child(jr, 0);
                        //						PlaneTreeItem *poppItem = m_pModel->itemFromIndex(poppChild);
                        ObjectTreeItem *pPOppItem = pPolarItem->child(jr);
                        QModelIndex poppChild = m_pModel->index(jr,0, pPolarItem);
                        QString strange = pPOpp->name();

                        if(strange.compare(pPOppItem->name())==0)
                        {
                            bSelected = true;
                            m_Selection = xfl::PLANEOPP;
                            m_pTreeView->setCurrentIndex(poppChild);
                            m_pTreeView->scrollTo(poppChild);
                            break;
                        }
/*                        bool bOK=false;
                        QString strange = pPOppItem->name().trimmed();
                        double val = locale().toDouble(strange, &bOK);

                        if(bOK)
                        {
                            switch(pPOpp->polarType())
                            {
                            case xfl::T1POLAR:
                            case xfl::T2POLAR:
                            case xfl::T3POLAR:
                            {
                                bSelected = fabs(val-pPOpp->alpha())<0.0005;
                                break;
                            }
                            case xfl::T5POLAR:
                            {
                                bSelected = fabs(val-pPOpp->beta())<0.0005;
                                break;
                            }
                            case xfl::T7POLAR:
                            case xfl::T6POLAR:
                            {
                                bSelected = fabs(val-pPOpp->ctrl())<0.0005;
                                break;
                            }
                            default:
                                bSelected = false; //never reached
                                break;
                            }
                            if(bSelected)
                            {
                                m_Selection = xfl::PLANEOPP;
                                m_pTreeView->setCurrentIndex(poppChild);
                                m_pTreeView->scrollTo(poppChild);
                                break;
                            }
                        }*/
                    }
                }
                if(bSelected) break;
            }
        }
        if(bSelected) break;
    }
    setObjectProperties();

    //    m_pStruct->selectionModel()->blockSignals(false);
}


/**
 * Removes the plane defined by the pointer and returns the name
 * of the previous plane in the list, or of the next plane if none
 * @param pPlane a pointer to the plane object to be removed
 * @return the name of the next plane to select
 */
QString PlaneTreeView::removePlane(Plane *pPlane)
{
    if(!pPlane) return "";
    return removePlane(pPlane->name());
}


/**
 * Removes the plane defined by the name and returns the name
 * of the previous plane in the list, or of the next plane if none
 * @param planeName the name of the plane object to be removed
 * @return the name of the next plane to select
 */
QString PlaneTreeView::removePlane(QString const &planeName)
{
    if(!planeName.length()) return QString();

    m_pTreeView->selectionModel()->blockSignals(true);

    int irow = 0;
    for(irow=0; irow<m_pModel->rowCount(); irow++)
    {
        ObjectTreeItem *pItem = m_pModel->item(irow);
        // scan
        if(pItem && pItem->level()==1 && pItem->name().compare(planeName)==0)
        {
            // plane found
//            QModelIndex rootindex = m_pModel->index(0,0, QModelIndex());
//            ObjectTreeItem *proot = m_pModel->itemFromIndex(rootindex);
            m_pModel->removeRow(irow, QModelIndex());
            break;
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);

    if(irow+1<Objects3d::planeCount())
        return Objects3d::planeAt(irow+1)->name();
    else if(irow-1>=0)
        return Objects3d::planeAt(irow-1)->name();


    m_pModel->updateData();
    return QString();
}


QString PlaneTreeView::removeWPolar(WPolar *pWPolar)
{
    if(!pWPolar) return "";

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        QModelIndex planeindex = m_pModel->index(ir, 0);
        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pWPolar->planeName())==0)
        {
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pOldPolarItem = pPlaneItem->child(jr);

                if(pOldPolarItem)
                {
                    if(pOldPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)==0)
                    {
                        m_pModel->removeRow(jr, planeindex);

                        m_pTreeView->selectionModel()->blockSignals(false);

                        // find the previous item, or the next one if this polar is the first
                        if(pPlaneItem->rowCount())
                        {
                            jr =std::min(jr, pPlaneItem->rowCount()-1);
                            return pPlaneItem->child(jr)->name();
                        }
                        return QString();
                    }
                }
            }
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
    return QString();
}


void PlaneTreeView::removeWPolars(Plane const*pPlane)
{
    if(!pPlane) return;

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pPlane->name())==0)
        {
            QModelIndex planeindex = m_pModel->index(ir, 0);
            Q_ASSERT(planeindex.isValid());

            m_pModel->removeRows(0, pPlaneItem->rowCount(), planeindex);
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
}


void PlaneTreeView::removeWPolarPOpps(WPolar const*pWPolar)
{
    if(!pWPolar) return;

    m_pTreeView->selectionModel()->blockSignals(true);
    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        // find the polar's parent Plane item
        if(pPlaneItem->name().compare(pWPolar->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().compare(pWPolar->polarName(), Qt::CaseInsensitive)==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);
                    m_pModel->removeRows(0, pPolarItem->rowCount(), polarindex);
                    break;
                }
            }
        }
    }

    m_pModel->updateData();
    setOverallCheckStatus();
    m_pTreeView->selectionModel()->blockSignals(false);
}




void PlaneTreeView::removePlaneOpp(PlaneOpp *pPOpp)
{
    if(!pPOpp) return;

//    m_pTreeView->selectionModel()->blockSignals(true);
//    m_pModel->blockSignals(true);

    bool bRemoved(false);

    for(int ir=0; ir<m_pModel->rowCount(); ir++)
    {
        ObjectTreeItem *pPlaneItem = m_pModel->item(ir);
        Plane *m_pPlane = Objects3d::plane(pPlaneItem->name());

        // find the polar's parent Plane
        if(pPlaneItem->name().compare(pPOpp->planeName())==0)
        {
            //find the WPolar item
            for(int jr=0; jr<pPlaneItem->rowCount(); jr++)
            {
                ObjectTreeItem *pPolarItem = pPlaneItem->child(jr);
                if(pPolarItem->name().compare(pPOpp->polarName(), Qt::CaseInsensitive)==0)
                {
                    QModelIndex polarindex = m_pModel->index(jr, 0, pPlaneItem);

                    WPolar *pWPolar = Objects3d::wPolar(m_pPlane, pPolarItem->name());
                    if(!pWPolar) continue;

                    //find the POpp item
                    for(int jr=0; jr<pPolarItem->rowCount(); jr++)
                    {
                        ObjectTreeItem *poppItem = pPolarItem->child(jr);
                        if(poppItem->name().compare(pPOpp->name())==0)
                        {
                            m_pModel->removeRow(jr,polarindex);
                            bRemoved = true;
                            break;
                        }
                    }
                }
                if(bRemoved) break;
            }
        }
    }

//    m_pModel->blockSignals(false);
//    m_pTreeView->selectionModel()->blockSignals(false);

//    m_pTreeView->update();
//    m_pModel->updateData();
    setOverallCheckStatus();
}


void PlaneTreeView::contextMenuEvent(QContextMenuEvent *pEvent)
{
    QModelIndex index = m_pTreeView->currentIndex();
    setObjectFromIndex(index);

    ObjectTreeItem *pItem = m_pModel->itemFromIndex(index);

    PlaneOpp *m_pPOpp = s_pMiarex->m_pCurPOpp;
    WPolar *m_pWPolar = s_pMiarex->m_pCurWPolar;
    Plane* m_pPlane = s_pMiarex->m_pCurPlane;

    QString strong;

    if(!pItem) return;

    if(pItem->level()==1)
    {
        // no parent, we have a plane
        m_pPlane = Objects3d::plane(pItem->name());
        if(m_pPlane) strong  = m_pPlane->name();
        else         strong.clear();
    }
    else if(pItem->level()==2)
    {
        //we have a WPolar;
        ObjectTreeItem *pParent = pItem->parentItem();
        m_pPlane = Objects3d::plane(pParent->name());
        m_pWPolar = Objects3d::wPolar(m_pPlane, pItem->name());
        if(m_pWPolar) strong = m_pWPolar->polarName();
        else          strong.clear();
    }
    else if(pItem->level()==3)
    {
        //we have a POpp selected;
        ObjectTreeItem *pParent = pItem->parentItem();
        ObjectTreeItem *pParent2 = pParent->parentItem();
        m_pPlane  = Objects3d::plane(pParent2->name());
        m_pWPolar = Objects3d::wPolar(m_pPlane, pParent->name());
        m_pPOpp   = Objects3d::getPlaneOpp(m_pPlane, m_pWPolar, pItem->name().toDouble());
        if(m_pPOpp) strong = pItem->name();
        else        strong.clear();
    }

    if     (m_Selection==xfl::PLANEOPP && m_pPOpp)
        s_pMainFrame->m_pCurWOppMenu->exec(pEvent->globalPos());
    else if(m_Selection==xfl::WPOLAR && m_pWPolar)
        s_pMainFrame->m_pCurWPlrMenu->exec(pEvent->globalPos());
    else if(m_Selection==xfl::PLANE && m_pPlane)
        s_pMainFrame->m_pCurrentPlaneMenu->exec(pEvent->globalPos());

    pEvent->accept();
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void PlaneTreeView::keyPressEvent(QKeyEvent *pEvent)
{
/*    PlaneOpp *m_pPOpp = s_pMiarex->m_pCurPOpp;
    WPolar *m_pWPolar = s_pMiarex->m_pCurWPolar;
    Plane* m_pPlane = s_pMiarex->m_pCurPlane;
*/
    switch (pEvent->key())
    {
        case Qt::Key_Delete:
        {
/*            if(m_Selection==xfl::PLANEOPP && m_pPOpp)
                s_pMiarex->onDeleteCurPOpp();
            else if(m_Selection==xfl::WPOLAR && m_pWPolar)
                s_pMiarex->onDeleteCurWPolar();
            else if(m_Selection==xfl::PLANE && m_pPlane)
                s_pMiarex->onDeleteCurPlane();*/

            pEvent->accept();
            return;
        }
        default:
            s_pMiarex->keyPressEvent(pEvent);
    }
}


void PlaneTreeView::loadSettings(QSettings &settings)
{
    settings.beginGroup("PlaneTreeView");
    {
        s_SplitterSizes = settings.value("HSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void PlaneTreeView::saveSettings(QSettings &settings)
{
    settings.beginGroup("PlaneTreeView");
    {
        settings.setValue("HSplitterSizes", s_SplitterSizes);
    }
    settings.endGroup();
}


/**
 * @brief PlaneTreeView::selectCurrentItem
 * Selects the active object, a PlaneOpp, a WPolar or a Plane
 */
void PlaneTreeView::selectCurrentObject()
{
    //	qDebug("selectCurrentObject");
    if(s_pMiarex->isPOppView() || s_pMiarex->m_iView==xfl::WCPVIEW)
    {
        if     (s_pMiarex->m_pCurPOpp)   selectPlaneOpp(s_pMiarex->m_pCurPOpp);
        else if(s_pMiarex->m_pCurWPolar) selectWPolar(s_pMiarex->m_pCurWPolar, false);
        else if(s_pMiarex->m_pCurPlane)  selectPlane(s_pMiarex->m_pCurPlane);
    }
    else if(s_pMiarex->m_iView==xfl::WPOLARVIEW || s_pMiarex->m_iView==xfl::STABPOLARVIEW)
    {
        if     (s_pMiarex->m_pCurWPolar) selectWPolar(s_pMiarex->m_pCurWPolar, false);
        else if(s_pMiarex->m_pCurPlane)  selectPlane(s_pMiarex->m_pCurPlane);
    }
    else if (s_pMiarex->is3dView() || s_pMiarex->m_iView==xfl::OTHERVIEW)
    {
        selectPlane(s_pMiarex->m_pCurPlane);
    }
}


void PlaneTreeView::selectObjects()
{
    if     (s_pMiarex->curPOpp())   selectPlaneOpp();
    else if(s_pMiarex->curWPolar()) selectWPolar(s_pMiarex->curWPolar(), false);
    else                            selectPlane(s_pMiarex->curPlane());
}


/** update the line properties for each polar and popp item in the treeview */
void PlaneTreeView::setCurveParams()
{
    ObjectTreeItem *pRootItem = m_pModel->rootItem();
    for(int i0=0; i0<pRootItem->rowCount(); i0++)
    {
        QModelIndex planeindex = m_pModel->index(i0,0);
        ObjectTreeItem *pPlaneItem = m_pModel->itemFromIndex(planeindex);
        Plane const *pPlane = nullptr;
        if(!pPlaneItem) pPlane = nullptr;
        else            pPlane = Objects3d::plane(pPlaneItem->name());
        if(!pPlane) continue;
        {
            pPlaneItem->setCheckState(planeState(pPlane));

            for(int i1=0; i1< pPlaneItem->rowCount(); i1++)
            {
                if(pPlaneItem->child(i1))
                {
                    ObjectTreeItem *pWPolarItem = pPlaneItem->child(i1);
                    WPolar const *pWPolar = nullptr;
                    if(pWPolarItem) pWPolar = Objects3d::wPolar(pPlane, pWPolarItem->name());
                    else            pWPolar = nullptr;

                    if(pWPolar)
                    {
                        LineStyle ls(pWPolar->theStyle());
                        ls.m_bIsEnabled = !s_pMiarex->is3dView();
                        pWPolarItem->setTheStyle(ls);
                        if(s_pMiarex->isPolarView() || s_pMiarex->isStabPolarView())
                        {
                            bool bCheck = pWPolar->isVisible();
                            bCheck = bCheck && (s_pMiarex->isPolarView() || s_pMiarex->isStabilityView() || s_pMiarex->isPOppView()) ;
                            pWPolarItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                        }
                        else
                        {
                            pWPolarItem->setCheckState(wPolarState(pWPolar));
                        }

                        for(int i2=0; i2<pWPolarItem->rowCount(); i2++)
                        {
                            ObjectTreeItem *pOppItem = pWPolarItem->child(i2);
                            PlaneOpp *pPOpp = nullptr;

                            if(pOppItem)
                            {
                                pPOpp = Objects3d::getPlaneOpp(pPlane, pWPolar, pOppItem->name().toDouble());
                            }
                            else pPOpp = nullptr;
                            if(pPOpp)
                            {
                                LineStyle ls(pPOpp->theStyle());
                                ls.m_bIsEnabled = s_pMiarex->isPOppView();
                                pOppItem->setTheStyle(ls);

                                bool bCheck = pPOpp->isVisible()&& s_pMiarex->isPOppView();
                                pOppItem->setCheckState(bCheck ? Qt::Checked : Qt::Unchecked);
                            }
                        }
                    }
                }
            }
        }
    }
    m_pModel->updateData();
}


Qt::CheckState PlaneTreeView::planeState(const Plane *pPlane) const
{
    bool bAll = true;
    bool bNone = true;
    if(s_pMiarex->isPolarView() || s_pMiarex->isStabPolarView())
    {
        for(int iplr=0; iplr<Objects3d::polarCount(); iplr++)
        {
            WPolar const*pWPolar = Objects3d::polarAt(iplr);
            if(pPlane->hasWPolar(pWPolar))
            {
                bAll = bAll && pWPolar->isVisible();
                bNone = bNone && !pWPolar->isVisible();
            }
        }
    }
    else if(s_pMiarex->isStabPolarView())
    {
        for(int iplr=0; iplr<Objects3d::polarCount(); iplr++)
        {
            WPolar const*pWPolar = Objects3d::polarAt(iplr);
            if(pWPolar->isStabilityPolar() && pPlane->hasWPolar(pWPolar))
            {
                bAll = bAll && pWPolar->isVisible();
                bNone = bNone && !pWPolar->isVisible();
            }
        }
    }
    else
    {
        for(int iopp=0; iopp<Objects3d::planeOppCount(); iopp++)
        {
            PlaneOpp const*pOpp = Objects3d::planeOppAt(iopp);
            if(pPlane->hasPOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
    }
    if(bNone)      return Qt::Unchecked;
    else if(bAll)  return Qt::Checked;
    else           return Qt::PartiallyChecked;
}


Qt::CheckState PlaneTreeView::wPolarState(WPolar const*pWPolar) const
{
    if(s_pMiarex->isPOppView())
    {
        bool bAll = true;
        bool bNone = true;
        for(int iopp=0; iopp<Objects3d::planeOppCount(); iopp++)
        {
            PlaneOpp const *pOpp = Objects3d::planeOppAt(iopp);
            if(pWPolar->hasPOpp(pOpp))
            {
                bAll = bAll && pOpp->isVisible();
                bNone = bNone && !pOpp->isVisible();
            }
        }
        if     (bNone) return Qt::Unchecked;
        else if(bAll)  return Qt::Checked;
        else           return Qt::PartiallyChecked;
    }
    else if(s_pMiarex->isPolarView())
    {
        return pWPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    else if(s_pMiarex->isStabPolarView())
    {
        return pWPolar->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    return Qt::Unchecked;
}


void PlaneTreeView::setOverallCheckStatus()
{
    if(s_pMiarex->isPOppView())
    {
        m_pTreeView->enableSelectBox(true);

        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects3d::planeOppCount(); io++)
        {
            PlaneOpp *const pPOpp=Objects3d::planeOppAt(io);
            if(pPOpp->isVisible()) bAllUnchecked = false;
            else                   bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else if(s_pMiarex->isPolarView())
    {
        m_pTreeView->enableSelectBox(true);
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects3d::polarCount(); io++)
        {
            WPolar *const pWPolar = Objects3d::polarAt(io);
            if(pWPolar->isVisible()) bAllUnchecked = false;
            else                     bAllChecked   = false;
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else if(s_pMiarex->isStabPolarView())
    {
        m_pTreeView->enableSelectBox(true);
        bool bAllChecked   = true;
        bool bAllUnchecked = true;
        for(int io=0; io<Objects3d::polarCount(); io++)
        {
            WPolar *const pWPolar = Objects3d::polarAt(io);
            if(pWPolar->isStabilityPolar())
            {
                if(pWPolar->isVisible()) bAllUnchecked = false;
                else                     bAllChecked   = false;
            }
        }

        if     (bAllChecked)   m_pTreeView->setOverallCheckedState(Qt::Checked);
        else if(bAllUnchecked) m_pTreeView->setOverallCheckedState(Qt::Unchecked);
        else                   m_pTreeView->setOverallCheckedState(Qt::PartiallyChecked);
    }
    else
    {
        m_pTreeView->enableSelectBox(false);
        m_pTreeView->setOverallCheckedState(Qt::Unchecked);
    }
}


void PlaneTreeView::onSwitchAll(bool bChecked)
{
    if(s_pMiarex->isPOppView())
    {
        if(bChecked) s_pMiarex->onShowAllWOpps();
        else         s_pMiarex->onHideAllWOpps();
    }
    else if(s_pMiarex->isPolarView())
    {
        if(bChecked) s_pMiarex->onShowAllWPolars();
        else         s_pMiarex->onHideAllWPolars();
    }
    else if(s_pMiarex->isStabPolarView())
    {
        if(bChecked) s_pMiarex->onShowAllWPolars();
        else         s_pMiarex->onHideAllWPolars();
    }
    m_pModel->updateData();
}


void PlaneTreeView::onSetFilter()
{
    QString filter = m_pTreeView->filter();
    QStringList filters = filter.split(QRegExp("\\s+"));

    if(filters.size()==0)
    {
        for(int jp=0; jp<Objects3d::polarCount(); jp++)
        {
            Objects3d::polarAt(jp)->setVisible(true);
        }
    }
    else if(filters.size()==1)
    {
        for(int jp=0; jp<Objects3d::polarCount(); jp++)
        {
            WPolar *pWPolar = Objects3d::polarAt(jp);
            bool bVisible = pWPolar->name().contains(filter, Qt::CaseInsensitive);
            pWPolar->setVisible(bVisible);
        }

        for(int ip=0; ip<Objects3d::planeCount(); ip++)
        {
            Plane const *pPlane = Objects3d::planeAt(ip);
            if(pPlane->name().contains(filter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects3d::polarCount(); jp++)
                {
                    WPolar *pWPolar = Objects3d::polarAt(jp);
                    if(pWPolar->planeName()==pPlane->name())
                        pWPolar->setVisible(true);
                }
            }
        }
    }
    else
    {
        QString planefilter = filters.first();
        QString polarfilter = filters.at(1);
        for(int jp=0; jp<Objects3d::polarCount(); jp++)
        {
            WPolar *pWPolar = Objects3d::polarAt(jp);
            pWPolar->setVisible(false);
        }

        for(int ip=0; ip<Objects3d::planeCount(); ip++)
        {
            Plane const *pPlane = Objects3d::planeAt(ip);
            if(pPlane->name().contains(planefilter, Qt::CaseInsensitive))
            {
                for(int jp=0; jp<Objects3d::polarCount(); jp++)
                {
                    WPolar *pWPolar = Objects3d::polarAt(jp);
                    if(pWPolar->planeName()==pPlane->name())
                    {
                        bool bVisible = pWPolar->name().contains(polarfilter, Qt::CaseInsensitive);
                        pWPolar->setVisible(bVisible);
                    }
                }
            }
        }
    }


    setCurveParams();
    setOverallCheckStatus();

    s_pMiarex->resetCurves();
    s_pMiarex->updateView();

    m_pTreeView->update();
}



