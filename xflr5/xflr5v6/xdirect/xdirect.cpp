/****************************************************************************

    XDirect Class
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

#include <QDebug>
#include <QFileDialog>
#include <QGroupBox>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>


#include <globals/mainframe.h>
#include <misc/editplrdlg.h>
#include <misc/options/settingswt.h>
#include <misc/polarfilterdlg.h>
#include <xdirect/analysis/batchctrldlg.h>
#include <xdirect/analysis/batchthreaddlg.h>
#include <xdirect/analysis/foilpolardlg.h>
#include <xdirect/analysis/xfoiladvanceddlg.h>
#include <xdirect/foiltreeview.h>
#include <xdirect/geometry/cadddlg.h>
#include <xdirect/geometry/flapdlg.h>
#include <xdirect/geometry/foilcoorddlg.h>
#include <xdirect/geometry/foilgeomdlg.h>
#include <xdirect/geometry/interpolatefoilsdlg.h>
#include <xdirect/geometry/ledlg.h>
#include <xdirect/geometry/nacafoildlg.h>
#include <xdirect/geometry/tegapdlg.h>
#include <xdirect/geometry/twodpaneldlg.h>
#include <xdirect/optim2d/optim2d.h>
#include <xdirect/xdirect.h>
#include <xdirect/xdirectstyledlg.h>
#include <xdirect/xml/xmlpolarreader.h>
#include <xdirect/xml/xmlpolarwriter.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflgraph/containers/xdirecttilewt.h>
#include <xflgraph/controls/graphdlg.h>
#include <xflgraph/curve.h>
#include <xflobjects/editors/renamedlg.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/customdlg/textdlg.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/mintextedit.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/line/linecbbox.h>
#include <xflwidgets/line/linedelegate.h>
#include <xflwidgets/line/linepickerwt.h>
#include <xinverse/foilselectiondlg.h>


bool XDirect::s_bAlpha = true;
bool XDirect::s_bInitBL = true;
bool XDirect::s_bKeepOpenErrors = true;


int XDirect::s_TimeUpdateInterval = 100;

MainFrame *XDirect::s_pMainFrame(nullptr);


/**
*The public constructor.
*/
XDirect::XDirect(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_pXFADlg = new XFoilAnalysisDlg(this);
    connect(m_pXFADlg, SIGNAL(analysisFinished(Polar*)), SLOT(onTaskFinished(Polar*)));

    m_pOpPointWidget = nullptr;

    m_LineStyle = {true, Line::SOLID, 1, QColor(0,0,0), Line::NOSYMBOL};

    setupLayout();

    m_pAnimateTimer = new QTimer(this);
    m_posAnimate = 0; // no animation to start with
    connectSignals();

    m_bAnimate        = false;
    m_bAnimatePlus    = false;
    m_bCpGraph        = true;

    m_bXPressed = m_bYPressed = false;

    m_bResetCurves    = true;

    m_bTrans          = false;
    m_bType1          = true;
    m_bType2          = true;
    m_bType3          = true;
    m_bType4          = true;
    m_bNeutralLine    = true;
    m_bShowInviscid   = false;
    m_bCurOppOnly     = true;

    m_bPolarView      = true;
    m_iPlrGraph = 0;
    m_iPlrView  = xfl::ALLGRAPHS;

    m_posAnimate = 0;

    setCurPolar(nullptr);
    setCurOpp(nullptr);


    for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
    {
        m_PlrGraph.append(new Graph);
        m_PlrGraph.at(ig)->setGraphName(QString("Polar_Graph_%1").arg(ig));
        m_PlrGraph.at(ig)->setGraphType(GRAPH::POLARGRAPH);
        m_PlrGraph[ig]->setXMin(0.0);
        m_PlrGraph[ig]->setXMax(0.1);
        m_PlrGraph[ig]->setYMin(-0.1);
        m_PlrGraph[ig]->setYMax(0.1);
        m_PlrGraph[ig]->setScaleType(2);
        m_PlrGraph[ig]->setBorderStyle({true, Line::SOLID, 3, QColor(200,200,200), Line::NOSYMBOL});
        if(ig==0) m_PlrGraph[ig]->setVariables(2,1);
        if(ig==1) m_PlrGraph[ig]->setVariables(0,1);
        if(ig==2) m_PlrGraph[ig]->setVariables(0,5);
        if(ig==3) m_PlrGraph[ig]->setVariables(6,1);
        if(ig==4) m_PlrGraph[ig]->setVariables(0,10);
        if(ig==5) m_PlrGraph[ig]->setVariables(0,11);
    }
    for(int ig=0; ig<MAXPOLARGRAPHS; ig++) setGraphTitles(m_PlrGraph[ig]);

    m_CpGraph.setGraphType(GRAPH::OPPGRAPH);
    m_CpGraph.setScaleType(1);
    m_CpGraph.setXTitle(tr("X"));
    m_CpGraph.setYTitle(tr("Cp"));
    m_CpGraph.setInverted(true);
    m_CpGraph.setXMin(0.0);
    m_CpGraph.setXMax(1.0);
    m_CpGraph.setYMin(-0.1);
    m_CpGraph.setYMax(0.1);
    m_CpGraph.setBorderStyle({true, Line::SOLID, 3, QColor(200,200,200), Line::NOSYMBOL});

    m_CpGraph.setGraphName("Cp_Graph");
    m_CpGraph.setVariables(0,0);

    BatchAbstractDlg::initReList();

    NacaFoilDlg::s_pXFoil         = &m_XFoil;
    InterpolateFoilsDlg::s_pXFoil = &m_XFoil;
    CAddDlg::s_pXFoil             = &m_XFoil;
    TwoDPanelDlg::s_pXFoil        = &m_XFoil;
    FoilGeomDlg::s_pXFoil         = &m_XFoil;
    TEGapDlg::s_pXFoil            = &m_XFoil;
    LEDlg::s_pXFoil               = &m_XFoil;
}



/**
 * The public destructor.
 */
XDirect::~XDirect()
{
    for(int ig=m_PlrGraph.count()-1; ig>=0; ig--)
    {
        delete m_PlrGraph.at(ig);
        m_PlrGraph.removeAt(ig);
    }
    if(m_pXFADlg) delete m_pXFADlg;
}


void XDirect::setCurFoil(Foil*pFoil)    {Objects2d::setCurFoil(pFoil);}
void XDirect::setCurPolar(Polar*pPolar) {Objects2d::setCurPolar(pPolar);}
void XDirect::setCurOpp(OpPoint* pOpp)  {Objects2d::setCurOpp(pOpp);}

Foil *   XDirect::curFoil()  {return Objects2d::curFoil();}
Polar*   XDirect::curPolar() {return Objects2d::curPolar();}
OpPoint* XDirect::curOpp()   {return Objects2d::curOpp();}


/** Sets the state of the window's widgets i.a.w. the state of the active ojbects and views. */
void XDirect::setControls()
{
    m_pFoilTreeView->setCurveParams();
    m_pFoilTreeView->setOverallCheckStatus();

    m_pDisplayBox->setVisible(!m_bPolarView);

    s_pMainFrame->m_pOpPointsAct->setChecked(!m_bPolarView);
    s_pMainFrame->m_pPolarsAct->setChecked(m_bPolarView);

    //    s_pMainFrame->m_pShowPanels->setChecked(m_bShowPanels);
    s_pMainFrame->m_pShowNeutralLine->setChecked(m_bNeutralLine);
    s_pMainFrame->m_pShowInviscidCurve->setChecked(m_bShowInviscid);

    s_pMainFrame->m_psetCpVarGraph->setChecked(m_CpGraph.yVariable()==0);
    s_pMainFrame->m_psetQVarGraph->setChecked(m_CpGraph.yVariable()==1);

    s_pMainFrame->m_pExportBLData->setEnabled(Objects2d::curOpp());

    m_pchShowPressure->setEnabled(!m_bPolarView && Objects2d::curOpp());
    m_pchActiveOppOnly->setEnabled(!m_bPolarView);
    m_pchActiveOppOnly->setChecked(m_bCurOppOnly);
    m_pchShowBL->setEnabled(!m_bPolarView && Objects2d::curOpp());
    m_pchAnimate->setEnabled(!m_bPolarView && Objects2d::curOpp());
    m_pslAnimateSpeed->setEnabled(!m_bPolarView && Objects2d::curOpp() && m_pchAnimate->isChecked());

    s_pMainFrame->m_pCurrentFoilMenu->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pCurrentFoilMenu_OperFoilCtxMenu->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pCurrentFoilMenu_OperPolarCtxMenu->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pCurrentPolarMenu->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pCurrentPolarMenu_OperFoilCtxMenu->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pCurrentPolarMenu_OperPolarCtxMenu->setEnabled(Objects2d::curPolar());

    s_pMainFrame->m_pRenameCurFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pDirectDuplicateCurFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pDeleteCurFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pExportCurFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pRenameCurFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pDefinePolarAct->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pDeleteFoilOpps->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pDeleteFoilPolars->setEnabled(Objects2d::curFoil());

    s_pMainFrame->m_pEditPolarPts->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pDeletePolar->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pExportCurPolar->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pHidePolarOpps->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pShowPolarOpps->setEnabled(Objects2d::curPolar());
    s_pMainFrame->m_pDeletePolarOpps->setEnabled(Objects2d::curPolar());

    s_pMainFrame->m_pDerotateFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pNormalizeFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pRefineLocalFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pRefineGlobalFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pEditCoordsFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pScaleFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pSetLERadius->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pSetTEGap->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pSetFlap->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pInterpolateFoils->setEnabled(Objects2d::curFoil());

    s_pMainFrame->m_pCurrentOppMenu->setEnabled(Objects2d::curOpp());
    s_pMainFrame->m_pDeleteCurOpp->setEnabled(Objects2d::curOpp());
    s_pMainFrame->m_pExportCurOpp->setEnabled(Objects2d::curOpp());

    s_pMainFrame->checkGraphActions();
}


/**
* Connects signals and slots
*/
void XDirect::connectSignals()
{
    connect(this, SIGNAL(projectModified()), s_pMainFrame, SLOT(onProjectModified()));

    connect(m_prbSpec1,         SIGNAL(clicked()),            SLOT(onSpec()));
    connect(m_prbSpec2,         SIGNAL(clicked()),            SLOT(onSpec()));
    connect(m_prbSpec3,         SIGNAL(clicked()),            SLOT(onSpec()));
    connect(m_ppbAnalyze,       SIGNAL(clicked()),            SLOT(onAnalyze()));
    connect(m_pchSequence,      SIGNAL(clicked()),            SLOT(onSequence()));
    connect(m_pchStoreOpp,      SIGNAL(clicked()),            SLOT(onStoreOpp()));

    connect(m_pchAnimate,       SIGNAL(clicked(bool)),        SLOT(onAnimate(bool)));
    connect(m_pslAnimateSpeed,  SIGNAL(valueChanged(int)),    SLOT(onAnimateSpeed(int)));
    connect(m_pAnimateTimer,    SIGNAL(timeout()),            SLOT(onAnimateSingle()));

    connect(m_pchActiveOppOnly, SIGNAL(clicked(bool)), SLOT(onCurOppOnly()));
    connect(m_pchShowBL,        SIGNAL(clicked(bool)), s_pMainFrame->m_pXDirectTileWidget->opPointWidget(), SLOT(onShowBL(bool)));
    connect(m_pchShowPressure,  SIGNAL(clicked(bool)), s_pMainFrame->m_pXDirectTileWidget->opPointWidget(), SLOT(onShowPressure(bool)));
}


/**
* Creates a curve of the Cp graph for a specified OpPoint instance, or for all the instances of OpPoint.
* @param pOpp a pointer to the instance of the operating point, the data of which is used to build the CCurve objects
*/
void XDirect::createOppCurves(OpPoint *pOpp)
{
    OpPoint *pOpPoint(nullptr);
    if(pOpp) pOpPoint = pOpp; else pOpPoint = Objects2d::curOpp();

    Curve *pCurve1(nullptr);
    QString str;

    m_CpGraph.deleteCurves();

    for (int k=0; k<Objects2d::oppCount(); k++)
    {
        pOpp = Objects2d::oppAt(k);
        bool bShow = pOpp->isVisible();
        if(m_bCurOppOnly && pOpp!=curOpp()) bShow = false;
        if (pOpp && bShow)
        {
            pCurve1    = m_CpGraph.addCurve();

            //                pCurve1->setPoints(pOpp->pointStyle());
            LineStyle ls(pOpp->theStyle());
            pCurve1->setLineStyle(ls);
            pCurve1->setName(pOpp->opPointName());

            fillOppCurve(pOpp, &m_CpGraph, pCurve1);

            if(m_bShowInviscid && pOpPoint && m_CpGraph.yVariable()<2 && pOpp==curOpp())
            {
                Curve *pCpi = m_CpGraph.addCurve();
                pCpi->setSymbol(pOpPoint->pointStyle());
                pCpi->setStipple(Line::DASH);
                pCpi->setColor(pOpPoint->color().darker(150));
                pCpi->setWidth(pOpPoint->lineWidth());
                str= QString("-Re=%1-Alpha=%2_Inviscid").arg(pOpPoint->Reynolds(),8,'f',0).arg(pOpPoint->aoa(),5,'f',2);
                str = pOpPoint->foilName()+str;
                pCpi->setName(str);
                fillOppCurve(pOpPoint, &m_CpGraph, pCpi, true);
            }
        }
    }
}


/**
*Creates the curves of the graphs for all the visible polars.
*/
void XDirect::createPolarCurves()
{
    for(int ig=0; ig<MAXPOLARGRAPHS; ig++) m_PlrGraph[ig]->deleteCurves();

    for (int k=0; k<Objects2d::polarCount(); k++)
    {
        Polar *pPolar = Objects2d::polarAt(k);

        if (pPolar->isVisible() && pPolar->m_Alpha.size()>0)
        {
            if ((pPolar->polarType()==xfl::FIXEDSPEEDPOLAR      && m_bType1) ||
                    (pPolar->polarType()==xfl::FIXEDLIFTPOLAR   && m_bType2) ||
                    (pPolar->polarType()==xfl::RUBBERCHORDPOLAR && m_bType3) ||
                    (pPolar->polarType()==xfl::FIXEDAOAPOLAR    && m_bType4))
            {

                Curve* pCurve[MAXPOLARGRAPHS];
                for(int ic=0; ic<MAXPOLARGRAPHS; ic++) pCurve[ic]=nullptr;
                //                Curve* pTr2Curve = nullptr;
                for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
                {
                    pCurve[ig] = m_PlrGraph[ig]->addCurve();
                    pCurve[ig]->setLineStyle(pPolar->theStyle());

                    fillPolarCurve(pCurve[ig], pPolar, m_PlrGraph[ig]->xVariable(), m_PlrGraph[ig]->yVariable());
                    pCurve[ig]->setName(pPolar->polarName());
                }
            }
        }
    }
}


/**
 * Fills the Cp graph curve with the data from the OpPoint.
 * @param pOpp a pointer to the OpPoint for which the curve is drawn
 * @param pGraph a pointer to the Graph to which the curve belongs
 * @param pCurve a pointer to the CCurve which will be filled with the data from the OpPoint
 * @param bInviscid true if the inviscid resutls should be displayed, false if the viscous results should be displayed
 */
void XDirect::fillOppCurve(OpPoint *pOpp, Graph *pGraph, Curve *pCurve, bool bInviscid)
{
    Foil const*pOpFoil = Objects2d::foil(pOpp->foilName());

    m_CpGraph.resetLimits();
    m_CpGraph.setAuto(true);
    m_CpGraph.setInverted(false);
    switch(m_CpGraph.yVariable())
    {
        case 0:
        {
            m_CpGraph.setInverted(true);
            for (int j=0; j<pOpp->m_n; j++)
            {
                if(!bInviscid)
                {
                    if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Cpv[j]);
                }
                else
                {
                    pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Cpi[j]);
                }
            }
            pGraph->setYTitle(tr("Cp"));
            break;
        }
        case 1:
        {
            for (int j=0; j<pOpp->m_n; j++)
            {
                if(!bInviscid)
                {
                    if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Qv[j]);
                }
                else
                {
                    pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Qi[j]);
                }
            }
            pGraph->setYTitle(tr("Q"));
            break;
        }
        case 2:  //shear coeff
        {
            pGraph->setYTitle("sqrt(Max Shear)");
            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();
            Curve * pCurve2 = pGraph->addCurve();
            Curve * pCurve3 = pGraph->addCurve();

            pCurve0->setName("sqrt(Ctau_top)");
            pCurve1->setName("sqrt(CtauEq_top)");
            pCurve2->setName("sqrt(Ctau_bot)");
            pCurve3->setName("sqrt(CtauEq_bot)");

            int it1 = pOpp->blx.itran[1];
            int it2 = pOpp->blx.itran[2];

            for (int i=it1; i<=pOpp->blx.nside1-1; i++) pCurve0->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.ctau[i][1]);
            for (int i=2;   i<=pOpp->blx.nside1-1; i++) pCurve1->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.ctq[i][1]);

            for (int i=it2; i<=pOpp->blx.nside2-1; i++) pCurve2->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.ctau[i][2]);
            for (int i=2;   i<=pOpp->blx.nside2-1; i++) pCurve3->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.ctq[i][2]);
            break;
        }
        case 3:  //Dstar & theta TOP
        {
            pGraph->setYTitle("D* & Theta Top");

            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();

            pCurve0->setWidth(2);
            pCurve1->setWidth(2);
            pCurve0->setColor(QColor(55,155,75));
            pCurve1->setColor(QColor(55,75,155));

            pCurve0->setName("D*");
            pCurve1->setName("Theta");

            for (int i=2; i<pOpp->blx.nside1; i++)
            {
                pCurve0->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.dstr[i][1]);
                pCurve1->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.thet[i][1]);
            }
            break;
        }
        case 4:  //DStar & theta BOT
        {
            qDebug("fillin bottom");
            pGraph->setYTitle("D* & Theta Bot");
            Curve * pCurve0 = pGraph->addCurve();
            Curve * pCurve1 = pGraph->addCurve();

            pCurve0->setName("D*");
            pCurve1->setName("Theta");

            pCurve0->setWidth(2);
            pCurve1->setWidth(2);
            pCurve0->setColor(QColor(55,155,75));
            pCurve1->setColor(QColor(55,75,155));

            for (int i=2; i<pOpp->blx.nside2; i++)
            {
                pCurve0->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.dstr[i][2]);
                pCurve1->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.thet[i][2]);
            }
            break;
        }
        case 5:
        {
            pGraph->setYTitle("Log(Re_Theta)");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            for (int i=2; i<=pOpp->blx.nside1-1; i++){
                if (pOpp->blx.RTheta[i][1]>0.0) y[i][1] = log10( pOpp->blx.RTheta[i][1] );
                else                             y[i][1] = 0.0;
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++){
                if (pOpp->blx.RTheta[i][2]>0.0) y[i][2] = log10( pOpp->blx.RTheta[i][2] );
                else                             y[i][2] = 0.0;
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 6:
        {
            pGraph->setYTitle(tr("Re_Theta"));
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("ReTheta_Top");
            pBotCurve->setName("ReTheta_Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            for (int i=2; i<=pOpp->blx.nside1-1; i++) pTopCurve->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.RTheta[i][1]);
            for (int i=2; i<=pOpp->blx.nside2-1; i++) pBotCurve->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.RTheta[i][2]);
            break;
        }
        case 7:  //Amplification factor
        {
            pGraph->setYTitle("N/N0");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();

            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            for (int ibl=2; ibl<pOpp->blx.nside1; ibl++)
            {
                y[ibl][1] = pOpp->blx.ctau[ibl][1];
            }
            for (int ibl=2; ibl<pOpp->blx.nside2; ibl++)
            {
                y[ibl][2] = pOpp->blx.ctau[ibl][2];
            }

            for (int i=2; i<=pOpp->blx.itran[1]-2; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.itran[2]-2; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 8:
        {
            pGraph->setYTitle("Dissipation Coef.");
            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Dissipation-Top");
            pBotCurve->setName("Dissipation-Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double qrf = pOpp->blx.qinf;

            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                y[ibl][1] = pOpp->blx.dis[ibl][1] / qrf/ qrf/ qrf;
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                y[ibl][2] = pOpp->blx.dis[ibl][2] / qrf/ qrf/ qrf;
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 9:  //friction coefficient
        {
            pGraph->setYTitle("tau");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Wall_shear_Top");
            pBotCurve->setName("Wall_shear_Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double que = 0.5*pOpp->blx.qinf*pOpp->blx.qinf;

            double y[IVX][ISX];
            memset(y, 0, IVX*ISX*sizeof(double));
            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                y[ibl][1] = pOpp->blx.tau[ibl][1] / que;
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                y[ibl][2] = pOpp->blx.tau[ibl][2] / que;
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 10:
        {
            pGraph->setYTitle("Ue");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            double y[IVX][3];
            memset(y, 0, 3*IVX*sizeof(double));
            double uei=0;

            //---- fill compressible ue arrays
            for (int ibl=2; ibl<= pOpp->blx.nside1;ibl++)
            {
                uei = pOpp->blx.uedg[ibl][1];
                y[ibl][1] = uei * (1.0-pOpp->blx.tklam)
                        / (1.0-pOpp->blx.tklam*(uei/pOpp->blx.qinf)*(uei/pOpp->blx.qinf));
            }
            for (int ibl=2; ibl<= pOpp->blx.nside2;ibl++)
            {
                uei = pOpp->blx.uedg[ibl][2];
                y[ibl][2] = uei * (1.0-pOpp->blx.tklam)
                        / (1.0-pOpp->blx.tklam*(uei/pOpp->blx.qinf)*(uei/pOpp->blx.qinf));
            }

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], y[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], y[i][2]);
            }
            break;
        }
        case 11: //Hk
        {
            pGraph->setYTitle("Hk");
            Curve * pTopCurve = pGraph->addCurve();
            Curve * pBotCurve = pGraph->addCurve();
            pTopCurve->setName("Top");
            pBotCurve->setName("Bot");

            pTopCurve->setWidth(2);
            pBotCurve->setWidth(2);
            pTopCurve->setColor(QColor(55,155,75));
            pBotCurve->setColor(QColor(55,75,155));

            for (int i=2; i<=pOpp->blx.nside1-1; i++)
            {
                pTopCurve->appendPoint(pOpp->blx.xbl[i][1], pOpp->blx.Hk[i][1]);
            }
            for (int i=2; i<=pOpp->blx.nside2-1; i++)
            {
                pBotCurve->appendPoint(pOpp->blx.xbl[i][2], pOpp->blx.Hk[i][2]);
            }

            break;
        }
        default:
        {
            for (int j=0; j<pOpp->m_n; j++)
            {
                if(!bInviscid)
                {
                    if(pOpp->m_bViscResults) pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Cpv[j]);
                }
                else{
                    pCurve->appendPoint(pOpFoil->m_x[j], pOpp->Cpi[j]);
                }
            }
            pGraph->setYTitle(tr("Cp"));
            break;
        }
    }
}




/**
*Fills a CCurve object with data from a Polar object
* @param pCurve a pointer to the CCurve object to be filled with the data from the Polar object
* @param pPolar a pointer to the Polar object from which the data will extracted
* @param XVar the index of the variable for the curve's x-axis
* @param YVar the index of the variable for the curve's y-axis
*/
void XDirect::fillPolarCurve(Curve *pCurve, Polar *pPolar, int XVar, int YVar) const
{
    QVector<double> const &pX = pPolar->getGraphVariable(XVar);
    QVector<double> const &pY = pPolar->getGraphVariable(YVar);
    double fx = 1.0;
    double fy = 1.0;

    pCurve->setSelected(-1);

    if(XVar == 3) fx = 10000.0;
    if(YVar == 3) fy = 10000.0;

    for (int i=0; i<pPolar->m_Alpha.size(); i++)
    {
        if (XVar==14)
        {
            if(pX[i]>0.0)
            {
                if (YVar==12)
                {
                    if(pY[i]>0.0)
                    {
                        pCurve->appendPoint(1.0/sqrt(pX[i]), 1.0/sqrt(pY[i]));
                    }
                }
                else
                {
                    pCurve->appendPoint(1.0/sqrt(pX[i]), pY[i]*fy);
                }
            }
        }
        else{
            if (YVar==14)
            {
                if(pY[i]>0.0)
                {
                    pCurve->appendPoint(pX[i]*fx, 1.0/sqrt(pY[i]));
                }
            }
            else
            {
                pCurve->appendPoint(pX[i]*fx, pY[i]*fy);
            }
        }

        if(Objects2d::curOpp() && Graph::isHighLighting())
        {
            if(pPolar->hasOpp(curOpp()))
            {
                if(pPolar->isFixedaoaPolar())
                {
                    if(qAbs(pPolar->m_Re[i]-Objects2d::curOpp()->m_Reynolds)<1.0)
                    {
                        pCurve->setSelected(i);
                    }
                }
                else
                {
                    if(qAbs(pPolar->m_Alpha[i]-Objects2d::curOpp()->m_Alpha)<0.0001)
                    {
                        pCurve->setSelected(i);
                    }
                }
            }
        }
    }
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void XDirect::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
//    bool bCtrl = false;
//    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if(m_ppbAnalyze->hasFocus())  onAnalyze();
            else
            {
                activateWindow();
                m_ppbAnalyze->setFocus();
            }
            break;
        case Qt::Key_Escape:
            stopAnimate();
            updateView();
            break;
        case Qt::Key_H:
        {
            if(m_bPolarView && pEvent->modifiers().testFlag(Qt::ControlModifier))
            {
                s_pMainFrame->onHighlightOperatingPoint();
            }
            break;
        }
        case Qt::Key_L:
            s_pMainFrame->onLogFile();
            break;
        case Qt::Key_X:
            m_bXPressed = true;
            break;
        case Qt::Key_Y:
            m_bYPressed = true;
            break;
        case Qt::Key_F2:
        {
            if(bShift) onRenameCurPolar();
            else       onRenameCurFoil();
            break;
        }
        case Qt::Key_F3:
        {
            if(bShift) onCadd();
            else       onRefinePanelsGlobally();
            break;
        }
        case Qt::Key_F5:
        {
            if(!m_bPolarView) return;
            onOpPointView();
            break;
        }
        case Qt::Key_F8:
        {
            if(m_bPolarView) return;
            onPolarView();
            break;
        }
        case Qt::Key_F9:
        {
            onFoilGeom();
            break;
        }
        case Qt::Key_F10:
        {
            onSetFlap();
            break;
        }
        case Qt::Key_F11:
        {
            onInterpolateFoils();
            break;
        }
/*        case Qt::Key_F12:
        {
            onOptim2d();
            break;
        }*/
        default:
            QWidget::keyPressEvent(pEvent);
    }

    pEvent->accept();
}


/**
 * Overrides the QWidget's keyReleaseEvent method.
 * Dispatches the key release event
 * @param event the QKeyEvent
 */
void XDirect::keyReleaseEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_X:
            if(!pEvent->isAutoRepeat()) m_bXPressed = false;
            break;
        case Qt::Key_Y:
            if(!pEvent->isAutoRepeat()) m_bYPressed = false;
            break;
        default:
            QWidget::keyReleaseEvent(pEvent);
    }

    pEvent->accept();
}


/**
 * Loads the user's default settings from the application QSettings object
 * @param pSettings a pointer to the QSettings object
 */
void XDirect::loadSettings(QSettings &settings)
{
    int oppVar = 0;
    settings.beginGroup("XDirect");
    {
        OpPoint::setStoreOpp(settings.value("StoreOpp",       OpPoint::bStoreOpp()).toBool());
        s_bAlpha          = settings.value("AlphaSpec",       s_bAlpha).toBool();
        s_bInitBL         = settings.value("InitBL",          s_bInitBL).toBool();
        m_bPolarView      = settings.value("PolarView",       m_bPolarView).toBool();

        m_bType1          = settings.value("Type1", m_bType1).toBool();
        m_bType2          = settings.value("Type2", m_bType2).toBool();
        m_bType3          = settings.value("Type3", m_bType3).toBool();
        m_bType4          = settings.value("Type4", m_bType4).toBool();
        m_bNeutralLine    = settings.value("NeutralLine",  m_bNeutralLine).toBool();
        m_bCurOppOnly     = settings.value("CurOppOnly",   m_bCurOppOnly).toBool();
        m_bShowInviscid   = settings.value("ShowInviscid", m_bShowInviscid).toBool();
        m_bCpGraph        = settings.value("ShowCpGraph",  m_bCpGraph).toBool();

        oppVar = settings.value("OppVar", 0).toInt();
        s_TimeUpdateInterval = settings.value("TimeUpdateInterval", s_TimeUpdateInterval).toInt();

        m_iPlrGraph      = settings.value("PlrGraph", m_iPlrGraph).toInt();

        switch(settings.value("PlrView").toInt())
        {
            case 1:
                m_iPlrView = xfl::ONEGRAPH;
                break;
            case 2:
                m_iPlrView = xfl::TWOGRAPHS;
                break;
            case 4:
                m_iPlrView = xfl::FOURGRAPHS;
                break;
            default:
                m_iPlrView = xfl::ALLGRAPHS;
                break;
        }

        m_XFoil.setVAccel(settings.value("VAccel", m_XFoil.VAccel()).toDouble());
        s_bKeepOpenErrors = settings.value("KeepOpenErrors", s_bKeepOpenErrors).toBool();

        XFoilTask::s_bAutoInitBL    = settings.value("AutoInitBL", XFoilTask::s_bAutoInitBL).toBool();
        XFoilTask::s_IterLim        = settings.value("IterLim",    XFoilTask::s_IterLim).toInt();

        XFoil::setFullReport(settings.value("FullReport", XFoil::fullReport()).toBool());

        BatchThreadDlg::s_bUpdatePolarView = settings.value("BatchUpdatePolarView", BatchThreadDlg::s_bUpdatePolarView).toBool();
        BatchThreadDlg::s_nThreads = settings.value("MaxThreads", BatchThreadDlg::s_nThreads).toInt();

        m_pFoilTreeView->setSplitterSize(settings.value("FoilTreeSplitterSizes").toByteArray());
    }
    settings.endGroup();

    for(int ig=0; ig<m_PlrGraph.count(); ig++) m_PlrGraph[ig]->loadSettings(settings);

    m_CpGraph.loadSettings(settings);

    if(oppVar>=2) oppVar=0;
    m_CpGraph.setYVariable(oppVar);

    if(m_CpGraph.yVariable() == 0 || m_CpGraph.yVariable()>=2)
    {
        m_CpGraph.setYTitle(tr("Cp"));
        m_CpGraph.setInverted(true);
    }
    else
    {
        m_CpGraph.setYTitle(tr("Q"));
        m_CpGraph.setInverted(false);
    }

    for(int ig=0; ig<MAXPOLARGRAPHS; ig++)
    {
        //        Graph *pGraph = m_PlrGraph[ig];
        setGraphTitles(m_PlrGraph[ig]);
    }
    m_pOpPointWidget->loadSettings(settings);

    XFoilAnalysisDlg::loadSettings(settings);
    FoilPolarDlg::loadSettings(settings);
    EditPlrDlg::loadSettings(settings);
    Optim2d::loadSettings(settings);
    BatchAbstractDlg::loadSettings(settings);
    BatchCtrlDlg::loadSettings(settings);
}


/**
 * The user has clicked the animate checkcbox
 * @param bChecked the new state of the checkbox
 */
void XDirect::onAnimate(bool bChecked)
{
    m_pslAnimateSpeed->setEnabled(bChecked);
    if(!Objects2d::curFoil() || !Objects2d::curPolar())
    {
        m_bAnimate = false;
        return;
    }

    if(bChecked)
    {
        for (int l=0; l< Objects2d::oppCount(); l++)
        {
            OpPoint*pOpPoint = Objects2d::oppAt(l);

            if (pOpPoint &&
                    pOpPoint->polarName()  == Objects2d::curPolar()->polarName() &&
                    pOpPoint->foilName() == Objects2d::curFoil()->name())
            {
                if(qAbs(Objects2d::curOpp()->m_Alpha - pOpPoint->aoa())<0.0001)
                    m_posAnimate = l-1;
            }
        }
        m_bAnimate  = true;
        int speed = m_pslAnimateSpeed->value();
        m_pAnimateTimer->setInterval(800-speed);
        m_pAnimateTimer->start();
    }
    else
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        if(m_posAnimate<0 || m_posAnimate>=Objects2d::oppCount()) return;
        OpPoint* pOpPoint = Objects2d::oppAt(m_posAnimate);
        if(pOpPoint) setOpp(pOpPoint->aoa());
        //        UpdateView();
        return;
    }
}


/**
 * Called by the animation timer.
 * Updates the display with the data of the next OpPoint.
 */
void XDirect::onAnimateSingle()
{
    QString str;
    bool bIsValid(false);

    OpPoint* pOpPoint(nullptr);

    if(Objects2d::oppCount()<=1) return;

    // find the next oppoint related to this foil and polar pair
    while(!bIsValid)
    {
        if(m_bAnimatePlus)
        {
            m_posAnimate++;
            if (m_posAnimate >= Objects2d::oppCount())
            {
                m_posAnimate = Objects2d::oppCount()-2;
                m_bAnimatePlus = false;
            }
        }
        else
        {
            m_posAnimate--;
            if (m_posAnimate <0)
            {
                m_posAnimate = 1;
                m_bAnimatePlus = true;
            }
        }
        if(m_posAnimate<0 || m_posAnimate>=Objects2d::oppCount()) return;

        pOpPoint = Objects2d::oppAt(m_posAnimate);

        if (pOpPoint &&
                pOpPoint->polarName()  == Objects2d::curPolar()->polarName() &&
                pOpPoint->foilName() == Objects2d::curFoil()->name() &&
                pOpPoint != Objects2d::curOpp())
        {
            bIsValid = true;
            createOppCurves(pOpPoint);
            setCurOpp(pOpPoint);

            if(!Objects2d::curPolar()->isFixedaoaPolar()) str = QString("%1").arg(Objects2d::curOpp()->m_Alpha,8,'f',2);
            else                                str = QString("%1").arg(Objects2d::curOpp()->Reynolds(),8,'f',2);
            updateView();
        }
    }
}


/**
 * the user has moved the slider which defines the animation speed
 * @param val the slider's new position
 */
void XDirect::onAnimateSpeed(int val)
{
    if(m_pAnimateTimer->isActive())
    {
        m_pAnimateTimer->setInterval(1000-val);
    }
}



/**
 * The user has clicked the analyze button.
 *
 * Reads the input parameters, initializes the analysis dialog box, and starts the analysis.
 */
void XDirect::onAnalyze()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    if      (m_prbSpec1->isChecked()) s_bAlpha = true;
    else if (m_prbSpec2->isChecked()) s_bAlpha = false;
    else if (m_prbSpec3->isChecked()) s_bAlpha = false;

    XFoilAnalysisDlg::s_bSequence = m_pchSequence->isChecked();

    double Alpha(0), AlphaMax(0), AlphaDelta(0);
    double Cl(0), ClMax(0), ClDelta(0);
    double Reynolds(0), ReynoldsMax(0), ReynoldsDelta(0);
    if(Objects2d::curPolar()->polarType()!=xfl::FIXEDAOAPOLAR)
    {
        if(s_bAlpha)
        {
            Alpha      = m_pdeAlphaMin->value();
            AlphaMax   = m_pdeAlphaMax->value();
            AlphaDelta = m_pdeAlphaDelta->value();
            m_pXFADlg->setAlpha(Alpha, AlphaMax, AlphaDelta);
        }
        else
        {
            Cl      = m_pdeAlphaMin->value();
            ClMax   = m_pdeAlphaMax->value();
            ClDelta = m_pdeAlphaDelta->value();
            m_pXFADlg->setCl(Cl, ClMax, ClDelta);
        }
    }
    else
    {
        Reynolds      = m_pdeAlphaMin->value();
        ReynoldsMax   = m_pdeAlphaMax->value();
        ReynoldsDelta = m_pdeAlphaDelta->value();
        m_pXFADlg->setRe(Reynolds, ReynoldsMax, ReynoldsDelta);
    }

    s_bInitBL   = m_pchInitBL->isChecked();
    OpPoint::setStoreOpp(m_pchStoreOpp->isChecked());

    m_ppbAnalyze->setEnabled(false);

    m_pXFADlg->m_pRmsGraph->copySettings(&Settings::s_RefGraph);

    m_pXFADlg->m_bAlpha = s_bAlpha;

    m_pXFADlg->show();
    m_pXFADlg->initDialog();
    m_pXFADlg->update();
    m_pXFADlg->analyze();
}


/**
 * Launches a batch analysis for control polars
 */
void XDirect::onBatchCtrlAnalysis()
{
    if(!Objects2d::curFoil()) return;

    onPolarView();
    updateView();

    m_ppbAnalyze->setEnabled(false);

    BatchCtrlDlg *pBatchCtrlDlg  = new BatchCtrlDlg;

    pBatchCtrlDlg->m_pFoil = Objects2d::curFoil();
    pBatchCtrlDlg->initDialog();
    pBatchCtrlDlg->exec();

    delete pBatchCtrlDlg;

    setPolar();
    m_pFoilTreeView->fillModelView();
    m_ppbAnalyze->setEnabled(true);

    setOpp();
    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested a local refinement of the panels of the current foil
 */
void XDirect::onCadd()
{
    stopAnimate();
    if(!Objects2d::curFoil())    return;
    onOpPointView();

    Foil *pCurFoil = curFoil(); //keep a reference to restore eventually
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());

    setCurOpp(nullptr);
    m_bResetCurves = true;

    setFoil(pNewFoil);

    CAddDlg caDlg(s_pMainFrame);
    caDlg.m_pBufferFoil = pNewFoil;
    caDlg.m_pMemFoil    = pCurFoil;
    caDlg.initDialog();
    Line::enumPointStyle psState = pNewFoil->pointStyle();
    if(psState==Line::NOSYMBOL) pNewFoil->setPointStyle(Line::LITTLECIRCLE);
    updateView();

    if(QDialog::Accepted == caDlg.exec())
    {
        pNewFoil->setPointStyle(psState);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }

    //reset everything
    setFoil(pCurFoil);

    if(Objects2d::curFoil())
        m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    delete pNewFoil;

    updateView();
}


/**
 * The user has requested to switch to the Cp graph view
 */
void XDirect::onCpGraph()
{
    onOpPointView();
    if(m_CpGraph.yVariable()!=0)
    {
        //        m_pCpGraph->ResetLimits();
        m_CpGraph.setAuto(true);
        m_CpGraph.setYVariable(0);
    }

    m_CpGraph.setInverted(true);
    m_bResetCurves = true;
    m_CpGraph.setYTitle(tr("Cp"));

    setControls();
    m_CpGraph.setXScale();
    setFoilScale();
    updateView();
}


/**
 * The user has toggled the request for the display of the inviscid Cp curve
 */
void XDirect::onCpi()
{
    m_bShowInviscid = !m_bShowInviscid;

    m_bResetCurves = true;
    setControls();
    updateView();
}


/**
 * The user has toggled the switch for the display of the current OpPoint only
 */
void XDirect::onCurOppOnly()
{
    m_bCurOppOnly = !m_bCurOppOnly;

    if(Objects2d::curOpp()) Objects2d::curOpp()->setVisible(true);
    m_bResetCurves = true;
    setAnalysisParams();
    updateView();
}


/**
 * The user has requested to define a new polar
 */
void XDirect::onDefinePolar()
{
    if(!Objects2d::curFoil()) return;

    FoilPolarDlg fpDlg(s_pMainFrame);

    fpDlg.initDialog();

    int res = fpDlg.exec();
    if (res == QDialog::Accepted)
    {
//        setCurPolar();
        Polar *pNewPolar = new Polar();

        if(DisplayOptions::isAlignedChildrenStyle())
        {
            pNewPolar->setTheStyle(Objects2d::curFoil()->theStyle());
        }
        else
        {
            QColor clr = xfl::randomColor(!DisplayOptions::isLightTheme());
            pNewPolar->setColor(clr.red(), clr.green(), clr.blue(), clr.alpha());
        }

        pNewPolar->setFoilName(Objects2d::curFoil()->name());
        pNewPolar->setPolarName(fpDlg.polarName());
        pNewPolar->setVisible(true);
        pNewPolar->copySpecification(&FoilPolarDlg::refPolar());

        Objects2d::addPolar(pNewPolar);
        setPolar(pNewPolar);

        m_pFoilTreeView->insertPolar(pNewPolar);
        m_pFoilTreeView->selectPolar(pNewPolar);
        updateView();
        emit projectModified();
    }
    setControls();
}


/**
 * The user has requested the deletion of the current Foil.
 * Deletes the Foil, and selects the next one in the array, if any.
 */
void XDirect::onDeleteCurFoil()
{
    Foil *pCurFoil = Objects2d::curFoil();
    if(!pCurFoil) return;

    QString strong;
    strong = tr("Are you sure you want to delete")  +"\n"+ Objects2d::curFoil()->name() +"\n";
    strong+= tr("and all associated OpPoints and Polars ?");

    int resp = QMessageBox::question(s_pMainFrame, tr("Question"), strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(resp != QMessageBox::Yes) return;


    m_pFoilTreeView->removeFoil(pCurFoil);

    Foil*pNextFoil = Objects2d::deleteFoil(pCurFoil);

    setFoil(pNextFoil);
    m_pFoilTreeView->fillModelView();
    m_pFoilTreeView->selectFoil(pNextFoil);

    m_bResetCurves = true;
    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested the deletion of the current OpPoint.
 */
void XDirect::onDeleteCurOpp()
{
    OpPoint* pOpPoint = Objects2d::curOpp();
    stopAnimate();

    if (!pOpPoint) return;
    QString strong,str;
    strong = tr("Are you sure you want to delete the Operating Point\n");
    if(Objects2d::curPolar()->polarType()!=xfl::FIXEDAOAPOLAR) str = QString("Alpha = %1").arg(pOpPoint->aoa(),0,'f',2);
    else                                                       str = QString("Reynolds = %1").arg(pOpPoint->Reynolds(),0,'f',0);
    strong += str;
    strong += "  ?";

    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
    {
        m_pFoilTreeView->removeOpPoint(Objects2d::curOpp());
        Objects2d::deleteOpp(Objects2d::curOpp());
        setOpp();
        updateView();
    }
    setControls();
}


/**
 * The user has requested the deletion of the current Polar.
 */
void XDirect::onDeleteCurPolar()
{
    if(!Objects2d::curPolar()) return;

    QString str;

    str = tr("Are you sure you want to delete the polar :\n  ") + Objects2d::curPolar()->polarName();
    str += tr("\n and all the associated OpPoints ?");

    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), str,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
    {
        // start by removing all OpPoints
        for (int l=Objects2d::oppCount()-1; l>=0; l--)
        {
            OpPoint *pOpPoint = Objects2d::oppAt(l);
            if (pOpPoint->polarName()  == Objects2d::curPolar()->polarName() &&
                  pOpPoint->foilName() == Objects2d::curFoil()->name())
            {
                Objects2d::deleteOppAt(l);
            }
        }
        // then remove the CPolar and update views
        for (int l=Objects2d::polarCount()-1; l>=0; l--)
        {
            if(Objects2d::curPolar() == Objects2d::polarAt(l))
            {
                m_pFoilTreeView->removePolar(Objects2d::curPolar());
                Objects2d::deletePolarAt(l);
                break;
            }
        }
        setCurOpp(nullptr);
        setCurPolar(nullptr);
    }

    setPolar();

    emit projectModified();
    updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Polar.
 */
void XDirect::onDeletePolarOpps()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    for(int i=Objects2d::oppCount()-1; i>=0; i--)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->polarName())
        {
            Objects2d::deleteOppAt(i);
        }
    }

    setCurOpp(nullptr);
    emit projectModified();

    m_pFoilTreeView->addOpps(Objects2d::curPolar());
    m_bResetCurves = true;

    setControls();
    updateView();
}


/**
 * The user has requested the deletion of the OpPoints associated to the current Foil.
 */
void XDirect::onDeleteFoilOpps()
{
    if(!Objects2d::curFoil()) return;

    for(int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName()==Objects2d::curFoil()->name())
        {
            m_pFoilTreeView->addOpps(pPolar);
        }
    }

    for(int i=Objects2d::oppCount()-1; i>=0; i--)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name())
        {
            Objects2d::deleteOppAt(i);
        }
    }
    setCurOpp(nullptr);

    m_bResetCurves = true;

    m_pFoilTreeView->updateFoil(Objects2d::curFoil());

    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested the deletion of the Polars associated to the current Foil.
 */
void XDirect::onDeleteFoilPolars()
{
    if(!Objects2d::curFoil()) return;

    stopAnimate();

    QString strong;

    strong = tr("Are you sure you want to delete polars and OpPoints\n");
    strong +=tr("associated to ")+Objects2d::curFoil()->name()  + " ?";
    if (QMessageBox::Yes == QMessageBox::question(s_pMainFrame, tr("Question"), strong,
                                                  QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))

    {
        // start by removing all OpPoints
        for (int l=  Objects2d::oppCount()-1; l>=0; l--)
        {
            OpPoint *pOpPoint = Objects2d::oppAt(l);
            if (pOpPoint->foilName() == Objects2d::curFoil()->name())
            {
                Objects2d::deleteOppAt(l);
            }
        }

        // then remove CPolar and update views
        for (int l=Objects2d::polarCount()-1; l>=0; l--)
        {
            Polar*pPolar = Objects2d::polarAt(l);
            if (pPolar->foilName() == Objects2d::curFoil()->name())
            {
                m_pFoilTreeView->removePolar(pPolar);
                Objects2d::deletePolarAt(l);
            }
        }
        setCurOpp(nullptr);

    }
    setCurPolar(nullptr);
    setPolar();

    m_bResetCurves = true;

    m_pFoilTreeView->updateFoil(Objects2d::curFoil());

    emit projectModified();

    setControls();
    updateView();
}


/**
 * The user has requested that the foil be derotated
 */
void XDirect::onDerotateFoil()
{
    if(!Objects2d::curFoil()) return;
    QString str;
    stopAnimate();
    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(Objects2d::curFoil());
    xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
    setCurFoil(pNewFoil);
    updateView();

    double angle = pNewFoil->deRotate();
    str = QString(tr("The foil has been de-rotated by %1 degrees")).arg(angle,6,'f',3);
    s_pMainFrame->statusBar()->showMessage(str);

    if(addNewFoil(pNewFoil))
    {
        m_pFoilTreeView->insertFoil(pNewFoil);
        m_pFoilTreeView->selectFoil(pNewFoil);
        emit projectModified();
        updateView();
        return;
    }
    //restore things
    delete pNewFoil;
    setFoil(pCurFoil);
    updateView();
}


/**
 * The user has requested to modify the parameters of the active polar
 */
void XDirect::onEditPolarPts()
{
    if (!Objects2d::curPolar()) return;

    Polar *pMemPolar = new Polar;
    pMemPolar->copy(Objects2d::curPolar());

    EditPlrDlg epDlg(s_pMainFrame);
    epDlg.initDialog(this, Objects2d::curPolar(), nullptr, nullptr);

    LineStyle style(Objects2d::curPolar()->theStyle());

    Objects2d::curPolar()->setPointStyle(Line::LITTLECIRCLE);

    m_bResetCurves = true;
    updateView();

    if(epDlg.exec() == QDialog::Accepted)
    {
        emit projectModified();
    }
    else
    {
        Objects2d::curPolar()->copy(pMemPolar);
    }
    Objects2d::curPolar()->setStipple(style.m_Stipple);
    Objects2d::curPolar()->setWidth(style.m_Width);
    Objects2d::curPolar()->setColor(style.m_Color.red(), style.m_Color.green(), style.m_Color.blue());
    Objects2d::curPolar()->setPointStyle(style.m_Symbol);
    Objects2d::curPolar()->setVisible(style.m_bIsVisible);
    m_bResetCurves = true;
    updateView();

    delete pMemPolar;
}


/**
 * The user has requested the export of the current results stored in the XFoil object to a text file
 */
void XDirect::onExportBLData()
{
    if(!Objects2d::curOpp() || Objects2d::curOpp()->blx.nside1==0) return;
    if(!Objects2d::curFoil()) return;

    QString fileName,  OutString, strong;

    double xBL[IVX][ISX], UeVinf[IVX][ISX], Cf[IVX][ISX], Cd[IVX][ISX], AA0[IVX][ISX];
    double DStar[IVX][ISX], Theta[IVX][ISX];
    double uei;
    double que = 0.5*m_XFoil.QInf()*m_XFoil.QInf();
    double qrf = m_XFoil.QInf();
    int nside1, nside2, ibl;
    xfl::enumTextFileType type = xfl::TXT;

    fileName = Objects2d::curFoil()->name();
    fileName.replace("/", " ");

    fileName = QFileDialog::getSaveFileName(this, tr("Export Current XFoil Results"),
                                            xfl::s_LastDirName,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"));

    if(!fileName.length()) return;
    int pos = fileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = fileName.left(pos);

    pos  = fileName.lastIndexOf(".csv");
    if(pos>0) type = xfl::CSV;

    QFile destFile(fileName);

    if (!destFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&destFile);

    out << xfl::versionName();
    out << ("\n");
    strong = Objects2d::curFoil()->name()+ "\n";
    out << (strong);

    if(type==xfl::TXT)
        strong = QString("Alpha = %1,  Re = %2,  Ma= %3,  ACrit=%4\n\n")
                .arg(Objects2d::curOpp()->aoa(), 5, 'f',1)
                .arg(Objects2d::curOpp()->Reynolds(), 8, 'f',0)
                .arg(Objects2d::curOpp()->Mach(), 6, 'f',4)
                .arg(Objects2d::curOpp()->ACrit, 4, 'f',1);
    else
        strong = QString("Alpha =, %1,Re =, %3,Ma=, %3,ACrit =,%4\n\n")
                .arg(Objects2d::curOpp()->aoa(), 5, 'f',1)
                .arg(Objects2d::curOpp()->Reynolds(), 8, 'f',0)
                .arg(Objects2d::curOpp()->Mach(), 6, 'f',4)
                .arg(Objects2d::curOpp()->ACrit, 4, 'f',1);
    out << (strong);


    nside1 = Objects2d::curOpp()->blx.nside1;
    nside2 = Objects2d::curOpp()->blx.nside2;

    for (ibl=2; ibl<= nside1;ibl++)    xBL[ibl][1] = Objects2d::curOpp()->blx.xbl[ibl][1];
    for (ibl=2; ibl<= nside2;ibl++)    xBL[ibl][2] = Objects2d::curOpp()->blx.xbl[ibl][2];

    //write top first
    for (ibl=2; ibl<= nside1;ibl++)
    {
        uei = Objects2d::curOpp()->blx.uedg[ibl][1];
        UeVinf[ibl][1] = uei * (1.0-Objects2d::curOpp()->blx.tklam)
                / (1.0-Objects2d::curOpp()->blx.tklam*(uei/Objects2d::curOpp()->blx.qinf)*(uei/Objects2d::curOpp()->blx.qinf));
    }
    for (ibl=2; ibl<= nside2;ibl++)
    {
        uei = Objects2d::curOpp()->blx.uedg[ibl][2];
        UeVinf[ibl][2] = uei * (1.0-Objects2d::curOpp()->blx.tklam)
                / (1.0-Objects2d::curOpp()->blx.tklam*(uei/Objects2d::curOpp()->blx.qinf)*(uei/Objects2d::curOpp()->blx.qinf));
    }
    //---- fill compressible ue arrays
    for (ibl=2; ibl<= nside1;ibl++)    Cf[ibl][1] = Objects2d::curOpp()->blx.tau[ibl][1] / que;
    for (ibl=2; ibl<= nside2;ibl++)    Cf[ibl][2] = Objects2d::curOpp()->blx.tau[ibl][2] / que;

    //---- fill compressible ue arrays
    for (ibl=2; ibl<= nside1;ibl++)    Cd[ibl][1] = Objects2d::curOpp()->blx.dis[ibl][1] / qrf/ qrf/ qrf;
    for (ibl=2; ibl<= nside2;ibl++)    Cd[ibl][2] = Objects2d::curOpp()->blx.dis[ibl][2] / qrf/ qrf/ qrf;
    //NPlot
    for (ibl=2; ibl< nside1;ibl++)    AA0[ibl][1] = Objects2d::curOpp()->blx.ctau[ibl][1];
    for (ibl=2; ibl< nside2;ibl++)    AA0[ibl][2] = Objects2d::curOpp()->blx.ctau[ibl][2];

    for (ibl=2; ibl<= nside1; ibl++)
    {
        DStar[ibl][1] = Objects2d::curOpp()->blx.dstr[ibl][1];
        Theta[ibl][1] = Objects2d::curOpp()->blx.thet[ibl][1];
    }
    for (ibl=2; ibl<= nside2; ibl++)
    {
        DStar[ibl][2] = Objects2d::curOpp()->blx.dstr[ibl][2];
        Theta[ibl][2] = Objects2d::curOpp()->blx.thet[ibl][2];
    }

    out << tr("\nTop Side\n");
    if(type==xfl::TXT) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
    else                 OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
    out << (OutString);
    for (ibl=2; ibl<nside1; ibl++)
    {
        if(type==xfl::TXT)
            OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
                    .arg(xBL[ibl][1],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.Hk[ibl][1],8,'f',5)
                    .arg(UeVinf[ibl][1],8,'f',5)
                    .arg(Cf[ibl][1],8,'f',5)
                    .arg(Cd[ibl][1],8,'f',5)
                    .arg(AA0[ibl][1],8,'f',5)
                    .arg(DStar[ibl][1],8,'f',5)
                    .arg(Theta[ibl][1],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.ctq[ibl][1],8,'f',5);
        else
            OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
                    .arg(xBL[ibl][1],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.Hk[ibl][1],8,'f',5)
                    .arg(UeVinf[ibl][1],8,'f',5)
                    .arg(Cf[ibl][1],8,'f',5)
                    .arg(Cd[ibl][1],8,'f',5)
                    .arg(AA0[ibl][1],8,'f',5)
                    .arg(DStar[ibl][1],8,'f',5)
                    .arg(Theta[ibl][1],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.ctq[ibl][1],8,'f',5);
        out << (OutString);
    }
    out << tr("\n\nBottom Side\n");
    if(type==xfl::TXT) OutString = QString(tr("    x         Hk     Ue/Vinf      Cf        Cd     A/A0       D*       Theta      CTq\n"));
    else        OutString = QString(tr("x,Hk,Ue/Vinf,Cf,Cd,A/A0,D*,Theta,CTq\n"));
    out << (OutString);
    for (ibl=2; ibl<nside2; ibl++)
    {
        if(type==xfl::TXT)
            OutString = QString("%1  %2  %3  %4 %5 %6  %7  %8  %9\n")
                    .arg(xBL[ibl][2],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.Hk[ibl][2],8,'f',5)
                    .arg(UeVinf[ibl][2],8,'f',5)
                    .arg(Cf[ibl][2],8,'f',5)
                    .arg(Cd[ibl][2],8,'f',5)
                    .arg(AA0[ibl][2],8,'f',5)
                    .arg(DStar[ibl][2],8,'f',5)
                    .arg(Theta[ibl][2],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.ctq[ibl][2],8,'f',5);
        else
            OutString = QString("%1, %2, %3, %4, %5, %6, %7, %8, %9\n")
                    .arg(xBL[ibl][2],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.Hk[ibl][2],8,'f',5)
                    .arg(UeVinf[ibl][2],8,'f',5)
                    .arg(Cf[ibl][2],8,'f',5)
                    .arg(Cd[ibl][2],8,'f',5)
                    .arg(AA0[ibl][2],8,'f',5)
                    .arg(DStar[ibl][2],8,'f',5)
                    .arg(Theta[ibl][2],8,'f',5)
                    .arg(Objects2d::curOpp()->blx.ctq[ibl][2],8,'f',5);
        out << (OutString);
    }

    destFile.close();
}


/**
 * The user has requested the export of all polars to text files
 */
void XDirect::onExportAllPolarsTxt()
{
    QString DirName;
    //select the directory for output
    DirName = QFileDialog::getExistingDirectory(this,  tr("Export Directory"), xfl::s_LastDirName);
    onExportAllPolarsTxt(DirName, Settings::s_ExportFileType);
}


void XDirect::onExportAllPolarsTxt(QString DirName, xfl::enumTextFileType exporttype)
{
    QString FileName;
    QFile XFile;
    QTextStream out(&XFile);

    for(int l=0; l<Objects2d::polarCount(); l++)
    {
        Polar *pPolar = Objects2d::polarAt(l);
        FileName = DirName + "/" + pPolar->foilName() + "_" + pPolar->polarName();
        if(Settings::s_ExportFileType==xfl::TXT) FileName += ".txt";
        else                                       FileName += ".csv";

        XFile.setFileName(FileName);
        if (XFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            pPolar->exportPolar(out, xfl::versionName(), exporttype);
            XFile.close();
        }
        else
        {
            QString strange = tr("Could not write to the directory:") + DirName;
            QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
            return;
        }
    }
}


/**
 * The user has requested the export of all polars to .plr files
 */
void XDirect::onExportAllFoilPolars()
{
    if(!Objects2d::foilCount() || !Objects2d::polarCount()) return;

    QString FileName;
    FileName = ".plr";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), xfl::plrDirName()+"/"+FileName, tr("Polar File (*.plr)"));
    if(!FileName.length()) return;

    QString strong = FileName.right(4);
    if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly)) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::setPlrDirName(FileName.left(pos));

    QDataStream ar(&XFile);
#if QT_VERSION >= 0x040500
    ar.setVersion(QDataStream::Qt_4_5);
#endif
    ar.setByteOrder(QDataStream::LittleEndian);

    FoilSelectionDlg dlg(s_pMainFrame);
    dlg.initDialog(Objects2d::pOAFoil(), QStringList());

    if(Objects2d::curFoil())
        dlg.setFoilName(Objects2d::curFoil()->name());

    if(dlg.exec()==QDialog::Accepted)
        s_pMainFrame->saveFoilPolars(ar, dlg.foilList());

    XFile.close();
}


/**
 * The user has requested the creation of a .plr file with the Polars of the active Foil object.
 */
void XDirect::onSaveFoilPolars()
{
    if(!Objects2d::curFoil() || !Objects2d::polarCount()) return;

    QString FileName;
    FileName = Objects2d::curFoil()->name() + ".plr";
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Polar File"), xfl::plrDirName()+"/"+FileName, tr("Polar File (*.plr)"));
    if(!FileName.length()) return;

    QString strong = FileName.right(4);
    if(strong !=".plr" && strong !=".PLR") FileName += ".plr";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly)) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::setPlrDirName(FileName.left(pos));

    QDataStream ar(&XFile);
#if QT_VERSION >= 0x040500
    ar.setVersion(QDataStream::Qt_4_5);
#endif
    ar.setByteOrder(QDataStream::LittleEndian);

    QVector<Foil*> foilList = {Objects2d::curFoil()};
    s_pMainFrame->saveFoilPolars(ar, foilList);

    XFile.close();
}


/**
 * The user has requested the export of the current foil to a text file
 */
void XDirect::onExportCurFoil()
{
    if(!Objects2d::curFoil()) return;

    QString FileName;

    FileName = Objects2d::curFoil()->name();
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Export Foil"),
                                            xfl::s_LastDirName+"/"+FileName+".dat",
                                            tr("Foil File (*.dat)"));

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    Objects2d::curFoil()->exportFoil(out);
    XFile.close();
}


/**
 * The user has requested the export of the current OpPoint to a text file
 */
void XDirect::onExportCurOpp()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar() || !Objects2d::curOpp())    return;

    QString FileName;

    QString filter;
    if(Settings::s_ExportFileType==xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
                                            xfl::s_LastDirName ,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) Settings::s_ExportFileType = xfl::CSV;
    else       Settings::s_ExportFileType = xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    Objects2d::curOpp()->exportOpp(out, xfl::versionName(), Settings::s_ExportFileType, Objects2d::curFoil());
    XFile.close();
}


/**
 * The user has requested the export of the OpPoints associated to the current Polar to a text file
 */
void XDirect::onExportPolarOpps()
{
    if(!Objects2d::polarCount())
    {
        QMessageBox::warning(s_pMainFrame, tr("Warning"), "No Operating Points to export to file");
        return;
    }

    int i,j;
    QString FileName;

    QString filter;
    if(Settings::s_ExportFileType==xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = QFileDialog::getSaveFileName(this, tr("Export OpPoint"),
                                            xfl::s_LastDirName ,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);

    if(!FileName.length()) return;
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) Settings::s_ExportFileType = xfl::CSV;
    else       Settings::s_ExportFileType = xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);


    QString Header, strong;
    out<<xfl::versionName();
    out<<"\n\n";
    strong = Objects2d::curFoil()->name() + "\n";
    out << strong;



    for (i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpPoint = Objects2d::oppAt(i);
        if(pOpPoint->foilName() == Objects2d::curPolar()->foilName() && pOpPoint->polarName() == Objects2d::curPolar()->polarName() )
        {
            if(Settings::s_ExportFileType==xfl::TXT)
                strong = QString("Reynolds = %1   Mach = %2  NCrit = %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->ACrit, 3, 'f',1);
            else
                strong = QString("Reynolds =, %1,Mach =, %2,NCrit =, %3\n")
                        .arg(pOpPoint->Reynolds(), 7, 'f', 0)
                        .arg(pOpPoint->m_Mach, 4,'f',0)
                        .arg(pOpPoint->ACrit, 3, 'f',1);

            out<<strong;
            if(Settings::s_ExportFileType==1) Header = QString("  Alpha        Cd        Cl        Cm        XTr1      XTr2   TEHMom    Cpmn\n");
            else        Header = QString("Alpha,Cd,Cl,Cm,XTr1,XTr2,TEHMom,Cpmn\n");
            out<<Header;

            if(Settings::s_ExportFileType==xfl::TXT)
                strong = QString("%1   %2   %3   %4   %5   %6   %7  %8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->Xtr1,7,'f',3)
                        .arg(pOpPoint->Xtr2,7,'f',3)
                        .arg(pOpPoint->m_TE_HMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);
            else
                strong = QString("%1,%2,%3,%4,%5,%6,%7,%8\n")
                        .arg(pOpPoint->aoa(),7,'f',3)
                        .arg(pOpPoint->Cd,9,'f',3)
                        .arg(pOpPoint->Cl,7,'f',3)
                        .arg(pOpPoint->Cm,7,'f',3)
                        .arg(pOpPoint->Xtr1,7,'f',3)
                        .arg(pOpPoint->Xtr2,7,'f',3)
                        .arg(pOpPoint->m_TE_HMom,7,'f',4)
                        .arg(pOpPoint->Cpmn,7,'f',4);

            out<<strong;
            if(Settings::s_ExportFileType==xfl::TXT) out<< " Cpi          Cpv\n-----------------\n";
            else                                       out << "Cpi,Cpv\n";

            for (j=0; j<pOpPoint->m_n; j++)
            {
                if(pOpPoint->m_bViscResults)
                {
                    if(Settings::s_ExportFileType==xfl::TXT) strong = QString("%1   %2\n").arg(pOpPoint->Cpi[j], 7,'f',4).arg(pOpPoint->Cpv[j], 7, 'f',4);
                    else                                       strong = QString("%1,%2\n").arg(pOpPoint->Cpi[j], 7,'f',4).arg(pOpPoint->Cpv[j], 7, 'f',4);
                }
                else
                {
                    strong=QString("%1\n").arg(pOpPoint->Cpi[j],7,'f',4);
                }

                out << strong;
            }
            out << "\n\n";
        }
    }
    XFile.close();


}


/**
 * The user has requested the export of the current Polar to a text file
 */
void XDirect::onExportCurPolar()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar())    return;

    QString FileName, filter;

    if(Settings::s_ExportFileType==xfl::TXT) filter = "Text File (*.txt)";
    else                                       filter = "Comma Separated Values (*.csv)";

    FileName = Objects2d::curPolar()->polarName();
    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(this, tr("Export Polar"),
                                            xfl::s_LastDirName + "/"+FileName,
                                            tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
                                            &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);
    pos = FileName.lastIndexOf(".csv");
    if (pos>0) Settings::s_ExportFileType = xfl::CSV;
    else       Settings::s_ExportFileType = xfl::TXT;

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    Objects2d::curPolar()->exportPolar(out, xfl::versionName(), Settings::s_ExportFileType);
    XFile.close();
}


/**
 * The user has requested an edition of the current foil coordinates
 */
void XDirect::onFoilCoordinates()
{
    if(!Objects2d::curFoil())    return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(pCurFoil);
    pNewFoil->setPointStyle(Line::LITTLECIRCLE);
    setCurOpp(nullptr);
    m_bResetCurves = true;

    updateView();

    bool bFlap       = pCurFoil->m_bTEFlap;
    double FlapAngle = pCurFoil->m_TEFlapAngle;
    double Xh        = pCurFoil->m_TEXHinge;
    double Yh        = pCurFoil->m_TEXHinge;

    pNewFoil->m_bTEFlap = false;

    setCurFoil(pNewFoil);
    updateView();

    FoilCoordDlg fcoDlg(s_pMainFrame);
    fcoDlg.m_pMemFoil    = pCurFoil;
    fcoDlg.m_pBufferFoil = pNewFoil;
    fcoDlg.initDialog();

    if(QDialog::Accepted == fcoDlg.exec())
    {
        pNewFoil->m_bTEFlap = bFlap;
        pNewFoil->m_TEFlapAngle = FlapAngle;
        pNewFoil->m_TEXHinge = Xh;
        pNewFoil->m_TEYHinge = Yh;

        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
        }
        else
        {
            //reset everything
            setFoil(pCurFoil);
            m_XFoil.initXFoilGeometry(pCurFoil->m_n, pCurFoil->m_x, pCurFoil->m_y, pCurFoil->m_nx, pCurFoil->m_ny);
            delete pNewFoil;
        }

        emit projectModified();
    }
    else
    {
        //reset everything
        setCurFoil(pCurFoil);
        if(Objects2d::curFoil())
            m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
        delete pNewFoil;
    }

    curFoil()->setHighLight(-1);

    updateView();
}


/**
 * The user has requested to perform an edition of the current foil's thickness and camber properties.
 */
void XDirect::onFoilGeom()
{
    if(!Objects2d::curFoil())    return;

    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(pCurFoil);
    setCurFoil(pNewFoil);

    setCurOpp(nullptr);
    m_bResetCurves = true;
    updateView();

    FoilGeomDlg fgeDlg(s_pMainFrame);
    fgeDlg.m_pMemFoil = pCurFoil;
    fgeDlg.m_pBufferFoil = pNewFoil;
    fgeDlg.initDialog();

    if(fgeDlg.exec() == QDialog::Accepted)
    {
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());


        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }

    delete pNewFoil;
    setCurFoil(pCurFoil);

    if(Objects2d::curFoil()) m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);

    updateView();
}


/**
 * The user has requested to hide all OpPoints
 */
void XDirect::onHideAllOpps()
{
    for (int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        pOpp->setVisible(false);
    }
    m_bResetCurves = true;
    setAnalysisParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested to hide all polar curves
 */
void XDirect::onHideAllPolars()
{
    for (int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        pPolar->setVisible(false);
    }
    m_pFoilTreeView->setCurveParams();
    m_bResetCurves = true;

    updateView();
    emit projectModified();
}


/**
 * The user has requested to hide all polar curves associated to the current Foil
 */
void XDirect::onHideFoilPolars()
{
    if(!Objects2d::curFoil()) return;
    for (int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName() == Objects2d::curFoil()->name())
        {
            pPolar->setVisible(false);
        }
    }
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested to hide all OpPoint curves associated to the current Foil
 */
void XDirect::onHideFoilOpps()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    for(int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name())
            pOpp->setVisible(false);
    }
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested to hide all OpPoint curves associated to the current Polar
 */
void XDirect::onHidePolarOpps()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    for(int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->polarName())
            pOpp->setVisible(false);
    }
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * Imports the analysis definition from an XML file
 */
void XDirect::onImportXFoilPolars()
{
    QStringList pathNames;
    pathNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                              xfl::s_LastDirName,
                                              tr("XFoil Polar Format (*.*)"));

    if(!pathNames.size()) return ;
    int pos = pathNames.at(0).lastIndexOf("/");
    if(pos>0) xfl::s_xmlDirName = pathNames.at(0).left(pos);

    Polar *pPolar = nullptr;
    for(int iFile=0; iFile<pathNames.size(); iFile++)
    {
        QFile XFile(pathNames.at(iFile));
        pPolar = importXFoilPolar(XFile);
    }

    setCurOpp(nullptr);
    setPolar(pPolar);
    m_pFoilTreeView->fillModelView();
    updateView();
    emit projectModified();
}


/**
 * The user has requested to import a polar from a text file.
 * The Polar will be added to the array only if a Foil with the parent name exists.
 */
Polar * XDirect::importXFoilPolar(QFile & txtFile)
{
    if (!txtFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+txtFile.fileName();
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return nullptr;
    }

    double Re(0), alpha(0), CL(0), CD(0), CDp(0), CM(0), Xt(0), Xb(0), Cpmn(0), HMom(0);
    QString FoilName, strong, str;
    // JX-mod
    Foil *pFoil(nullptr);

    QTextStream in(&txtFile);

    bool bOK(false), bOK2(false);
    int Line = 0;

    xfl::readAVLString(in, Line, strong);// XFoil or XFLR5 version
    xfl::readAVLString(in, Line, strong);// Foil Name

    FoilName = strong.right(strong.length()-22);
    FoilName = FoilName.trimmed();

    if(!Objects2d::foil(FoilName))
    {
        str = tr("No Foil with the name ")+FoilName;
        str+= tr("\ncould be found. The polar(s) will not be stored");
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    Polar *pPolar = new Polar;

    pPolar->setFoilName(FoilName);

    xfl::readAVLString(in, Line, strong);// analysis type

    int matype = strong.mid(0,2).toInt(&bOK);
    int retype = strong.mid(2,2).toInt(&bOK2);

    if(!bOK || !bOK2)
    {
        str = QString("Error reading line %1: Unrecognized Mach and Reynolds type.\nThe polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    if     (retype ==1 && matype ==1) pPolar->setPolarType(xfl::FIXEDSPEEDPOLAR);
    else if(retype ==2 && matype ==2) pPolar->setPolarType(xfl::FIXEDLIFTPOLAR);
    else if(retype ==3 && matype ==1) pPolar->setPolarType(xfl::RUBBERCHORDPOLAR);
    else                              pPolar->setPolarType(xfl::FIXEDSPEEDPOLAR);

    xfl::readAVLString(in, Line, strong);
    if(strong.length() < 34)
    {
        str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setXtrBot(strong.mid(9,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Bottom Transition value at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setXtrTop(strong.mid(28,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Top Transition value at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    // Mach     Re     NCrit
    xfl::readAVLString(in, Line, strong);// blank line
    if(strong.length() < 50)
    {
        str = QString("Error reading line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    pPolar->setMach(strong.mid(8,6).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading Mach Number at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }

    Re = strong.mid(24,10).toDouble(&bOK);
    if(!bOK)
    {
        str = QString("Error reading Reynolds Number at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    Re *=1000000.0;

    pPolar->setNCrit(strong.mid(52,8).toDouble(&bOK));
    if(!bOK)
    {
        str = QString("Error reading NCrit at line %1. The polar(s) will not be stored").arg(Line);
        delete pPolar;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return nullptr;
    }
    pPolar->setReynolds(Re);

    xfl::readAVLString(in, Line, strong);// column titles
    xfl::readAVLString(in, Line, strong);// underscores

    bool bRead = true;
    while(bRead)
    {
        bRead  = xfl::readAVLString(in, Line, strong);// polar data
        if(bRead)
        {
            if(strong.length())
            {
                //                textline = strong.toLatin1();
                //                text = textline.constData();
                //                res = sscanf(text, "%lf%lf%lf%lf%lf%lf%lf%lf%lf", &alpha, &CL, &CD, &CDp, &CM, &Xt, &Xb, &Cpmn, &HMom);

                //Do this the C++ way
                QStringList values;
#if QT_VERSION >= 0x050F00
            values = values = strong.split(" ", Qt::SkipEmptyParts);
#else
    values = values = strong.split(" ", QString::SkipEmptyParts);
#endif

                if(values.length()>=7)
                {
                    alpha  = values.at(0).toDouble();
                    CL     = values.at(1).toDouble();
                    CD     = values.at(2).toDouble();
                    CDp    = values.at(3).toDouble();
                    CM     = values.at(4).toDouble();
                    Xt     = values.at(5).toDouble();
                    Xb     = values.at(6).toDouble();

                    if(values.length() >= 9)
                    {
                        Cpmn    = values.at(7).toDouble();
                        HMom    = values.at(8).toDouble();
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, Xt, Xb, Cpmn, HMom,0.0,0.0,Re,0.0);
                    }
                    else
                    {
                        pPolar->addPoint(alpha, CD, CDp, CL, CM, Xt, Xb, 0.0, 0.0,0.0,0.0,Re,0.0);

                    }
                }
            }
        }
    }
    txtFile.close();

    // jx-mod Use xflr5 standards for naming and coloring of a new polar

    /*
        Re = pPolar->Reynolds()/1000000.0;
        QString strange = QString("T%1_Re%2_M%3")
                .arg(pPolar->polarType())
                .arg(Re,0,'f',3)                        // jx-mod Re has 3 decimals
                .arg(pPolar->Mach(),0,'f',2);
        str = QString("_N%1").arg(pPolar->NCrit(),0,'f',1);
        strange += str + "_Imported";

        pPolar->setPolarName(strange);
        QColor clr = getColor(1);
        pPolar->setColor(clr.red(), clr.green(), clr.blue());
    */

    pPolar->setAutoPolarName();
    pPolar->setPolarName(pPolar->polarName() + "_Imported");

    if(DisplayOptions::isAlignedChildrenStyle())
    {
        pFoil = Objects2d::foil(FoilName);
        pPolar->setTheStyle(pFoil->theStyle());
    }
    else
    {
        QColor clr = xfl::randomColor(!DisplayOptions::isLightTheme());
        pPolar->setColor(clr);
    }

    Objects2d::addPolar(pPolar);
    return pPolar;
}


/**
 * The user has requested to import a polar from a text file in JavaFoil format
 * The Polar will be added to the array only if a Foil with the parent name exists.
 *  @todo Note: this option has not been tested in years... the JavaFoil format may have changed since
 */
void XDirect::onImportJavaFoilPolar()
{
    QString FoilName;
    QString strong, str;

    QString PathName;
    bool bOK(false);
    QByteArray textline;
    const char *text(nullptr);

    PathName = QFileDialog::getOpenFileName(s_pMainFrame, tr("Open File"),
                                            xfl::s_LastDirName,
                                            tr("JavaFoil Polar Format (*.*)"));
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    QTextStream in(&XFile);

    bool bIsReading = true;
    int res(0), Line(0);
    double Re(0);

    double alpha(0), CL(0), CD(0), CM(0), Xt(0), Xb(0);

    Line = 0;
    if(!xfl::readAVLString(in, Line, FoilName)) return;


    FoilName = FoilName.trimmed();

    if(!Objects2d::foil(FoilName))
    {
        str = tr("No Foil with the name ")+FoilName;
        str+= tr("\ncould be found. The polar(s) will not be stored");
        QMessageBox::warning(s_pMainFrame, tr("Warning"), str);
        return;
    }
    if(!xfl::readAVLString(in, Line, strong)) return; //blank line

    while(bIsReading)
    {
        if(!xfl::readAVLString(in, Line, strong)) break; //Re number

        strong = strong.right(strong.length()-4);
        Re = strong.toDouble(&bOK);
        if(!bOK)
        {
            bIsReading = false;
        }
        else
        {
            Polar *pPolar = new Polar();
            pPolar->setFoilName(FoilName);
            pPolar->setReynolds(Re);
            pPolar->setPolarName(QString("T%1_Re2_M3_JavaFoil")
                                 .arg(pPolar->polarType())
                                 .arg(pPolar->Reynolds()/1000000.0,0,'f',2)
                                 .arg(pPolar->Mach(),0,'f',2));

            QColor clr = xfl::getObjectColor(1);
            pPolar->setColor(clr.red(), clr.green(), clr.blue());
            Objects2d::addPolar(pPolar);
            setCurPolar(pPolar);

            if(!xfl::readAVLString(in, Line, strong)) break;//?    Cl    Cd    Cm 0.25    TU    TL    SU    SL    L/D
            if(!xfl::readAVLString(in, Line, strong)) break;//[?]    [-]    [-]    [-]    [-]    [-]    [-]    [-]    [-]

            res = 6;
            while(res==6)
            {
                bIsReading  = xfl::readAVLString(in, Line, strong);//values
                if(!bIsReading) break;
                strong = strong.trimmed();
                if(strong.length())
                {
                    strong.replace(',', '.');

                    textline = strong.toLatin1();
                    text = textline.constData();

                    res = sscanf(text, "%lf%lf%lf%lf%lf%lf",&alpha, &CL, &CD, &CM, &Xt, &Xb);
                    if (res == 6)     pPolar->addPoint(alpha, CD, 0.0, CL, CM, Xt, Xb, 0.0, 0.0, 0.0, 0.0, Re,0.0);
                }
                else
                {
                    res = 0;
                }
            }
        }
        setCurOpp(nullptr);
        setPolar();
        m_pFoilTreeView->fillModelView();
        updateView();
        emit projectModified();
    }
}


/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void XDirect::onInterpolateFoils()
{
    if(Objects2d::foilCount()<2)
    {
        QMessageBox::warning(s_pMainFrame, tr("Warning"), tr("At least two foils are required"));
        return;
    }

    stopAnimate();

    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);

    InterpolateFoilsDlg ifDlg(s_pMainFrame);
    ifDlg.m_pBufferFoil = pNewFoil;
    ifDlg.initDialog();

    updateView();

    if(ifDlg.exec() == QDialog::Accepted)
    {
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setName(ifDlg.m_NewFoilName);

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }

    //restore things
    setCurFoil(pCurFoil);
    if(Objects2d::curFoil())
        m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    delete pNewFoil;
    updateView();
}


/**
 * Launches a multi-threaded batch analysis
 */
void XDirect::onMultiThreadedBatchAnalysis()
{
    if(!Objects2d::curFoil()) return;

    if(QThread::idealThreadCount()<2)
    {
        QString strange = tr("Not enough threads available for multithreading");
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    onPolarView();
    updateView();

    m_ppbAnalyze->setEnabled(false);

    BatchThreadDlg *pBatchThreadDlg  = new BatchThreadDlg;
    pBatchThreadDlg->m_pFoil  = Objects2d::curFoil();
    pBatchThreadDlg->initDialog();

    pBatchThreadDlg->exec();

    delete pBatchThreadDlg;

    setPolar();
    m_pFoilTreeView->fillModelView();

    m_ppbAnalyze->setEnabled(true);

    setOpp();
    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested the launch of the interface used to create a NACA type foil.
 */
void XDirect::onNacaFoils()
{
    stopAnimate();
    onOpPointView();

    Foil* pCurFoil = Objects2d::curFoil();
    setCurFoil(nullptr);
    setCurPolar(nullptr);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    updateView();

    Foil *pNacaFoil = new Foil;
    pNacaFoil->setName("Naca0009");
    m_XFoil.naca4(9, 50);
    for (int j=0; j< m_XFoil.nb; j++)
    {
        pNacaFoil->m_xb[j] = m_XFoil.xb[j+1];
        pNacaFoil->m_yb[j] = m_XFoil.yb[j+1];
        pNacaFoil->m_x[j]  = m_XFoil.xb[j+1];
        pNacaFoil->m_y[j]  = m_XFoil.yb[j+1];
    }
    pNacaFoil->m_nb = m_XFoil.nb;
    pNacaFoil->m_n = m_XFoil.nb;
    pNacaFoil->initFoil();

    setFoil(pNacaFoil);
    updateView();

    NacaFoilDlg nacaDlg(s_pMainFrame);
    nacaDlg.m_pBufferFoil = pNacaFoil;

    if (nacaDlg.exec() == QDialog::Accepted)
    {
        QString str;
        if(nacaDlg.s_Digits>0 && log10(double(nacaDlg.s_Digits))<4)
            str = QString("%1").arg(nacaDlg.s_Digits,4,10,QChar('0'));
        else
            str = QString("%1").arg(nacaDlg.s_Digits);
        str = "NACA "+ str;

        xfl::setRandomFoilColor(pNacaFoil, !DisplayOptions::isLightTheme());
        pNacaFoil->setName(str);

        if(addNewFoil(pNacaFoil))
        {
            m_pFoilTreeView->insertFoil(pNacaFoil);
            m_pFoilTreeView->selectFoil(pNacaFoil);
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);

    if(Objects2d::curFoil()) m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    delete pNacaFoil;
    updateView();
}


/**
 * The user has requested that the length of the current foil be normalized to 1.
 */
void XDirect::onNormalizeFoil()
{
    if(!Objects2d::curFoil()) return;
    QString str;
    stopAnimate();
    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(Objects2d::curFoil());
    xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
    setCurFoil(pNewFoil);
    updateView();

    double length = pNewFoil->normalizeGeometry();
    str = QString(tr("The foil has been normalized from %1  to 1.000")).arg(length,7,'f',3);
    s_pMainFrame->statusBar()->showMessage(str);
    if(addNewFoil(pNewFoil))
    {
        m_pFoilTreeView->insertFoil(pNewFoil);
        m_pFoilTreeView->selectFoil(pNewFoil);
        emit projectModified();
        updateView();
        return;
    }
    //restore things
    delete pNewFoil;
    setCurFoil(pCurFoil);
    if(Objects2d::curFoil()) m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    updateView();
}


/**
 * The user has requested to switch to the OpPoint view
 */
void XDirect::onOpPointView()
{
    if(!m_bPolarView) return;

    m_bPolarView = false;
    m_bResetCurves = true;
    setFoilScale();

    setAnalysisParams();

    setGraphTiles();
    setControls();

    updateView();
}


/**
 * The user has requested to switch to the Polar view
 */
void XDirect::onPolarView()
{
    if(m_bPolarView) return;
    m_bPolarView = true;
    m_bResetCurves = true;

    setAnalysisParams();

    setGraphTiles();
    setControls();

    updateView();
}


/**
 * The user has requested the launch of the interface used to filter the type of polars to be displayed.
 */
void XDirect::onPolarFilter()
{
    PolarFilterDlg pfDlg(s_pMainFrame);
    pfDlg.m_bMiarex = false;
    pfDlg.m_bType1 = m_bType1;
    pfDlg.m_bType2 = m_bType2;
    pfDlg.m_bType3 = m_bType3;
    pfDlg.m_bType4 = m_bType4;
    pfDlg.initDialog();

    if(pfDlg.exec()==QDialog::Accepted)
    {
        m_bType1 = pfDlg.m_bType1;
        m_bType2 = pfDlg.m_bType2;
        m_bType3 = pfDlg.m_bType3;
        m_bType4 = pfDlg.m_bType4;
        if(m_bPolarView)
        {
            m_bResetCurves = true;
            updateView();
        }
    }
}


/**
 * The user has requested the launch of the interface to refine globally the Foil
*/
void XDirect::onRefinePanelsGlobally()
{
    if(!Objects2d::curFoil())    return;
    stopAnimate();

    onOpPointView();

    Foil*pCurFoil = curFoil();

    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(pCurFoil);
    setFoil(pNewFoil);

    setCurOpp(nullptr);
    m_bResetCurves = true;

    TwoDPanelDlg tdpDlg(s_pMainFrame);
    tdpDlg.m_pBufferFoil = pNewFoil;
    tdpDlg.m_pMemFoil    = pCurFoil;
    Line::enumPointStyle psState = pNewFoil->pointStyle();
    if(psState==Line::NOSYMBOL)    pNewFoil->setPointStyle(Line::LITTLECIRCLE);

    updateView();

    tdpDlg.initDialog();

    if(QDialog::Accepted == tdpDlg.exec())
    {
        pNewFoil->setPointStyle(psState);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setFoil(pCurFoil);

    m_XFoil.initXFoilGeometry(pCurFoil->m_n, pCurFoil->m_x, pCurFoil->m_y, pCurFoil->m_nx, pCurFoil->m_ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the display of the velocity in the Cp graph.
 */
void XDirect::onQGraph()
{
    onOpPointView();
    if(m_CpGraph.yVariable()!=1)
    {
        m_CpGraph.resetLimits();
        m_CpGraph.setAuto(true);
        m_CpGraph.setYVariable(1);
    }
    m_CpGraph.setInverted(false);
    m_bResetCurves = true;
    m_CpGraph.setYTitle(tr("Q"));

    setControls();

    m_CpGraph.setXScale();
    setFoilScale();
    updateView();
}


/**
 * The user has requested to rename the Polar
 */
void XDirect::onRenameCurPolar()
{
    if(!Objects2d::curPolar()) return;
    if(!Objects2d::curFoil()) return;

    int resp(0);
    Polar* pPolar(nullptr);
    OpPoint * pOpp;
    QString OldName = Objects2d::curPolar()->polarName();

    QStringList NameList;
    for(int k=0; k<Objects2d::polarCount(); k++)
    {
        pPolar = Objects2d::polarAt(k);
        if(pPolar->foilName() == Objects2d::curFoil()->name())
            NameList.append(pPolar->polarName());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, Objects2d::curPolar()->polarName(), tr("Enter the new name for the foil polar :"));

    bool bExists = true;

    while (bExists)
    {
        resp = renDlg.exec();
        if(resp==QDialog::Accepted)
        {
            if (OldName == renDlg.newName()) return;
            //Is the new name already used ?
            bExists = false;
            for (int k=0; k<Objects2d::polarCount(); k++)
            {
                pPolar = Objects2d::polarAt(k);
                if ((pPolar->foilName()==Objects2d::curFoil()->name()) && (pPolar->polarName() == renDlg.newName()))
                {
                    bExists = true;
                    break;
                }
            }
            if(!bExists)
            {
                for (int l=Objects2d::oppCount()-1;l>=0; l--)
                {
                    pOpp = Objects2d::oppAt(l);
                    if (pOpp->polarName() == OldName &&
                            pOpp->foilName() == Objects2d::curFoil()->name())
                    {
                        pOpp->setPolarName(renDlg.newName());
                    }
                }
                Objects2d::curPolar()->setPolarName(renDlg.newName());
            }
            emit projectModified();
        }
        else if(resp ==10)
        {
            //user wants to overwrite
            if (OldName == renDlg.newName()) return;
            int k(0);
            for (k=0; k<Objects2d::polarCount(); k++)
            {
                pPolar = Objects2d::polarAt(k);
                if (pPolar->polarName() == renDlg.newName())
                {
                    break;
                }
            }
            for (int l=Objects2d::oppCount()-1;l>=0; l--)
            {
                pOpp = Objects2d::oppAt(l);
                if (pOpp->polarName() == Objects2d::curPolar()->polarName())
                {
                    Objects2d::deleteOppAt(l);

                    if(pOpp==Objects2d::curOpp())
                        setCurOpp(nullptr);
                }
            }
            Objects2d::deletePolarAt(k);
            if(pPolar==Objects2d::curPolar()) setCurPolar(nullptr);

            //and rename everything
            if(Objects2d::curPolar())
                Objects2d::curPolar()->setPolarName(renDlg.newName());

            for (int l=Objects2d::oppCount()-1;l>=0; l--)
            {
                pOpp = Objects2d::oppAt(l);
                if (pOpp->polarName() == OldName &&
                        pOpp->foilName() == Objects2d::curFoil()->name())
                {
                    pOpp->setPolarName(renDlg.newName());
                }
            }

            bExists = false;
            emit projectModified();
        }
        else
        {
            return ;//cancelled
        }
    }
    m_pFoilTreeView->fillModelView();
    m_pFoilTreeView->selectPolar(Objects2d::curPolar());
    updateView();
}


/**
 * The user has requested to reset all polar graph scales to their automatic default value
 */
void XDirect::onResetAllPolarGraphsScales()
{
    for(int ig=0; ig<m_PlrGraph.count(); ig++)
    {
        m_PlrGraph[ig]->setAuto(true);
        m_PlrGraph[ig]->resetXLimits();
        m_PlrGraph[ig]->resetYLimits();
    }
    updateView();
}


/**
 * The user has requested the deletion of the dataof the current Polar.
 * The associated OpPoint objects will be deleted too.
 */
void XDirect::onResetCurPolar()
{
    if(!Objects2d::curPolar()) return;
    Objects2d::curPolar()->resetPolar();

    for(int i=Objects2d::oppCount()-1;i>=0;i--)
    {
        OpPoint*pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->polarName())
        {
            Objects2d::deleteOppAt(i);
        }
    }
    setCurOpp(nullptr);

    m_pFoilTreeView->addOpps(Objects2d::curPolar());

    m_bResetCurves = true;
    updateView();

    emit projectModified();
}


/**
 * The user has toggled the switch for a sequential analysis.
 */
void XDirect::onSequence()
{
    XFoilAnalysisDlg::s_bSequence = m_pchSequence->isChecked();
    setOpPointSequence();
}


/**
 * The user has requested the launch of the interface to define a L.E. or T.E. flap.
 */
void XDirect::onSetFlap()
{
    if(!Objects2d::curFoil()) return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil =curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);
    setCurOpp(nullptr);

    m_bResetCurves = true;

    FlapDlg flpDlg(s_pMainFrame);
    flpDlg.m_pBufferFoil  = pNewFoil;
    flpDlg.m_pMemFoil     = pCurFoil;
    flpDlg.initDialog();

    if(QDialog::Accepted == flpDlg.exec())
    {
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }

    //reset everything
    setCurFoil(pCurFoil);

    if(Objects2d::curFoil())  m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);

    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the launch of the interface to modify the radius of the Foil's leading edge.
 */
void XDirect::onSetLERadius()
{
    if(!Objects2d::curFoil())    return;
    stopAnimate();
    onOpPointView();

    Foil *pCurFoil = curFoil();
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(curFoil());
    setCurFoil(pNewFoil);


    setCurOpp(nullptr);
    m_bResetCurves = true;

    LEDlg lDlg(s_pMainFrame);
    lDlg.m_pBufferFoil = pNewFoil;
    lDlg.m_pMemFoil    = pCurFoil;
    lDlg.initDialog();

    if(QDialog::Accepted == lDlg.exec())
    {
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);

    if(Objects2d::curFoil())
        m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the launch of the interface to modify the gap at the Foil's trailing edge.
 */
void XDirect::onSetTEGap()
{
    if(!Objects2d::curFoil())    return;
    stopAnimate();
    onOpPointView();

    Foil *pNewFoil = new Foil();
    Foil *pCurFoil = curFoil();
    pNewFoil->copyFoil(pCurFoil);

    setCurOpp(nullptr);
    m_bResetCurves = true;

    setCurFoil(pNewFoil);

    TEGapDlg tegDlg(s_pMainFrame);
    tegDlg.m_pBufferFoil = pNewFoil;
    tegDlg.m_pMemFoil    = pCurFoil;
    tegDlg.m_Gap         = Objects2d::curFoil()->TEGap();
    tegDlg.initDialog();

    if(QDialog::Accepted == tegDlg.exec())
    {
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());

        if(addNewFoil(pNewFoil))
        {
            m_pFoilTreeView->insertFoil(pNewFoil);
            m_pFoilTreeView->selectFoil(pNewFoil);
            emit projectModified();
            updateView();
            return;
        }
    }
    //reset everything
    setCurFoil(pCurFoil);

    if(Objects2d::curFoil())
        m_XFoil.initXFoilGeometry(Objects2d::curFoil()->m_n, Objects2d::curFoil()->m_x, Objects2d::curFoil()->m_y, Objects2d::curFoil()->m_nx, Objects2d::curFoil()->m_ny);
    delete pNewFoil;
    updateView();
}


/**
 * The user has requested the display of all OpPoint curves.
 */
void XDirect::onShowAllOpps()
{
    m_bCurOppOnly = false;

    for (int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        pOpp->setVisible(true);
    }
    emit projectModified();
    m_bResetCurves = true;

    m_pFoilTreeView->setCurveParams();

    updateView();
}


/**
 * The user has requested the display of all Polar curves.
 */
void XDirect::onShowAllPolars()
{
    for (int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        pPolar->setVisible(true);
    }
    emit projectModified();
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
}


/**
 * The user has requested the display of only the Polar curves associated to the active Foil
 */
void XDirect::onShowFoilPolarsOnly()
{
    if(!Objects2d::curFoil()) return;

    for (int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        pPolar->setVisible((pPolar->foilName() == Objects2d::curFoil()->name()));
    }
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested the display of the Polar curves associated to the active Foil
 */
void XDirect::onShowFoilPolars()
{
    if(!Objects2d::curFoil()) return;

    for (int i=0; i<Objects2d::polarCount(); i++)
    {
        Polar *pPolar = Objects2d::polarAt(i);
        if(pPolar->foilName() == Objects2d::curFoil()->name())
        {
            pPolar->setVisible(true);
        }
    }
    m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested the display of the OpPoint curves associated to the active Foil
 */
void XDirect::onShowFoilOpps()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    m_bCurOppOnly = false;

    for(int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name())
            pOpp->setVisible(true);
    }
    if(!m_bPolarView) m_bResetCurves = true;

    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has requested the display of the curves of all OpPoint objects associated to the active Polar.
 */
void XDirect::onShowPolarOpps()
{
    if(!Objects2d::curFoil() || !Objects2d::curPolar()) return;

    m_bCurOppOnly = false;

    for(int i=0; i<Objects2d::oppCount(); i++)
    {
        OpPoint *pOpp = Objects2d::oppAt(i);
        if(pOpp->foilName()==Objects2d::curFoil()->name() && pOpp->polarName()==Objects2d::curPolar()->polarName())
            pOpp->setVisible(true);
    }
    if(!m_bPolarView) m_bResetCurves = true;
    m_pFoilTreeView->setCurveParams();

    updateView();
    emit projectModified();
}


/**
 * The user has toggled the switch used to define the type of input parameter bewteen aoa, Cl, and Re
 */
void XDirect::onSpec()
{
    if      (m_prbSpec1->isChecked()) s_bAlpha = true;
    else if (m_prbSpec2->isChecked()) s_bAlpha = false;
    else if (m_prbSpec3->isChecked()) s_bAlpha = false;
}


/**
 * The user has toggled the switch which defines if OpPoints should be stored at the end of the analysis
 */
void XDirect::onStoreOpp()
{
    OpPoint::setStoreOpp(m_pchStoreOpp->isChecked());
}


void XDirect::onTaskFinished(Polar *pPolar)
{
    if(m_pXFADlg->m_bErrors && s_bKeepOpenErrors)
    {
    }
    else
        m_pXFADlg->hide();

    m_ppbAnalyze->setEnabled(true);

    s_bInitBL = !m_XFoil.isBLInitialized();
    m_pchInitBL->setChecked(s_bInitBL);;

    m_pFoilTreeView->addOpps(pPolar);
    if(s_bAlpha) setOpp(XFoilAnalysisDlg::s_Alpha);
    else         setOpp();
    m_pFoilTreeView->selectOpPoint();


    m_bResetCurves = true;

    setControls();
    updateView();

    emit projectModified();
}


/**
 * The user has requested the launch of the interface used to define advanced settings for the XFoil analysis
 */
void XDirect::onXFoilAdvanced()
{
    XFoilAdvancedDlg xfaDlg(s_pMainFrame);
    xfaDlg.m_IterLimit   = XFoilTask::s_IterLim;
    xfaDlg.m_bAutoInitBL = XFoilTask::s_bAutoInitBL;
    xfaDlg.m_VAccel      = XFoil::VAccel();
    xfaDlg.m_bFullReport = XFoil::fullReport();
    xfaDlg.initDialog();

    if (QDialog::Accepted == xfaDlg.exec())
    {
        XFoil::setVAccel(xfaDlg.m_VAccel);
        XFoil::setFullReport(xfaDlg.m_bFullReport);
        XFoilTask::s_bAutoInitBL  = xfaDlg.m_bAutoInitBL;
        XFoilTask::s_IterLim      = xfaDlg.m_IterLimit;
    }
}


/**
 * Saves the user-defined settings
 * @param pSettings a pointer to the QSetting object.
 */
void XDirect::saveSettings(QSettings &settings)
{
    settings.beginGroup("XDirect");
    {
        settings.setValue("AlphaSpec",          s_bAlpha);
        settings.setValue("StoreOpp",           OpPoint::bStoreOpp());
        settings.setValue("InitBL",             s_bInitBL);
        settings.setValue("PolarView",          m_bPolarView);
        settings.setValue("Type1",              m_bType1);
        settings.setValue("Type2",              m_bType2);
        settings.setValue("Type3",              m_bType3);
        settings.setValue("Type4",              m_bType4);
        settings.setValue("CurOppOnly",         m_bCurOppOnly);
        settings.setValue("ShowInviscid",       m_bShowInviscid);
        settings.setValue("ShowCpGraph",        m_bCpGraph);
        settings.setValue("OppVar",             m_CpGraph.yVariable());
        settings.setValue("TimeUpdateInterval", s_TimeUpdateInterval);
        settings.setValue("PlrGraph",           m_iPlrGraph);
        settings.setValue("NeutralLine",        m_bNeutralLine);

        switch(m_iPlrView)
        {
            case xfl::ONEGRAPH:
                settings.setValue("PlrView", 1);
                break;
            case xfl::TWOGRAPHS:
                settings.setValue("PlrView", 2);
                break;
            case xfl::FOURGRAPHS:
                settings.setValue("PlrView", 4);
                break;
            default:
                settings.setValue("PlrView", 0);
                break;
        }


        settings.setValue("AutoInitBL", XFoilTask::s_bAutoInitBL);
        settings.setValue("IterLim",    XFoilTask::s_IterLim);
        settings.setValue("FullReport", XFoil::fullReport());

        settings.setValue("BatchUpdatePolarView", BatchThreadDlg::s_bUpdatePolarView);
        settings.setValue("MaxThreads",           BatchThreadDlg::s_nThreads);


        settings.setValue("VAccel",         m_XFoil.VAccel());
        settings.setValue("KeepOpenErrors", s_bKeepOpenErrors);

        settings.setValue("FoilTreeSplitterSizes", m_pFoilTreeView->splitterSize());
    }
    settings.endGroup();

    for(int ig=0; ig<m_PlrGraph.count(); ig++)
        m_PlrGraph[ig]->saveSettings(settings);

    m_CpGraph.saveSettings(settings);
    m_pOpPointWidget->saveSettings(settings);

    XFoilAnalysisDlg::saveSettings(settings);
    EditPlrDlg::saveSettings(settings);
    FoilPolarDlg::saveSettings(settings);
    Optim2d::saveSettings(settings);
    BatchAbstractDlg::saveSettings(settings);
    BatchCtrlDlg::saveSettings(settings);
}


/**
 * Initializes the widget values, depending on the type of Polar
 */
void XDirect::setAnalysisParams()
{
    m_pchInitBL->setChecked(s_bInitBL);
    m_pchStoreOpp->setChecked(OpPoint::bStoreOpp());
    if(Objects2d::curPolar())
    {
        if(!Objects2d::curPolar()->isFixedaoaPolar())
        {
            if(s_bAlpha) m_prbSpec1->setChecked(true);
            else         m_prbSpec2->setChecked(true);
            m_prbSpec3->setEnabled(false);
            m_plabUnit1->setText("<p>&deg;</p>");
            m_plabUnit2->setText("<p>&deg;</p>");
            m_plabUnit3->setText("<p>&deg;</p>");
        }
        else
        {
            m_prbSpec3->setChecked(true);
            m_prbSpec3->setEnabled(true);

            m_plabUnit1->clear();
            m_plabUnit2->clear();
            m_plabUnit3->clear();
        }
    }
    else
    {
        if(s_bAlpha) m_prbSpec1->setChecked(true);
        else         m_prbSpec2->setChecked(true);
        m_prbSpec3->setEnabled(false);
    }

    setOpPointSequence();
}


/**
 * Initializes XDirect with the data of the input Foil object.
 * If no Foil is proposed in input,sets the first stock Foil in alphabetical order.
 * Sets the first Polar object belonging to this Foil, if any.
 * Sets the first OpPoint object belonging to this Polar, if any.
 * @param pFoil a pointer to the active Foil object, or NULL if a stock Foil should be used.
 * @return a pointer to the Foil object which has been set.
 */
Foil* XDirect::setFoil(Foil* pFoil)
{
    stopAnimate();

    setCurFoil(pFoil);

    if(!Objects2d::curFoil())
    {
        //take the first in the array, if any
        if(Objects2d::foilCount())
        {
            setCurFoil(Objects2d::foilAt(0));
        }
    }

    Foil *pCurFoil = Objects2d::curFoil();
    bool bRes = false;
    if(pCurFoil)
    {
        if(pCurFoil->m_n>=IQX-2)
        {
            QString strange;
            strange = QString::asprintf("Cannot initialize the foil %s:\nNumber of panels=%d, Max. size=%d,\nRecommended size=100-150 panels",
                                        pCurFoil->name().toStdString().c_str(), pCurFoil->m_n, IQX-2);
            QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        }
        else
        {
            double xh(0), yh(0);
            if(pCurFoil->m_bTEFlap) pCurFoil->getHingeAbsolutePos(xh, yh);
            bRes = m_XFoil.initXFoilGeometry(pCurFoil->m_n, pCurFoil->m_x,pCurFoil->m_y, pCurFoil->m_nx, pCurFoil->m_ny,
                                             pCurFoil->m_bTEFlap, xh, yh);
            if(!bRes)
            {
                QString strange;
                strange = QString::asprintf("Error while initializing the foil %s", pCurFoil->name().toStdString().c_str());
                QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
            }
        }
    }

    if(pCurFoil && !bRes)
    {
        setCurFoil(nullptr);
    }
    else
    {
        if(!Objects2d::curFoil())
        {
            setCurPolar(nullptr);
            setCurOpp(nullptr);
        }
        else if (Objects2d::curPolar() && Objects2d::curPolar()->foilName() !=Objects2d::curFoil()->name())
        {
            //            setCurPolar(nullptr);
            //            setCurOpp(nullptr);
        }
        else if (Objects2d::curOpp() && Objects2d::curOpp()->foilName()  !=Objects2d::curFoil()->name())
        {
            //            setCurOpp(nullptr);
        }
    }

    setPolar();

    return Objects2d::curFoil();
}



/**
 * Initializes XDirect with the specified Polar object.
 * If the specified polar is not valid, a stock polar associated to the current foil will be set.
 * Sets the first OpPoint object belonging to this Polar, if any.
 * Initializes the XFoil object with the Polar's data.
 * @param pPolar a pointer to the Polar object to set. If NULL, a stock polar associated to the current foil will be set.
 * @return a pointer to the Polar object which has been set.
 */
Polar * XDirect::setPolar(Polar *pPolar)
{
    stopAnimate();

    if(!Objects2d::curFoil()|| !Objects2d::curFoil()->name().length())
    {
        setCurPolar(nullptr);
        setCurOpp(nullptr);
        setAnalysisParams();
        return nullptr;
    }

    if(pPolar) setCurPolar(pPolar);

    if(!Objects2d::curPolar())
    {
        //try to get one from the object array
        for(int i=0; i<Objects2d::polarCount(); i++)
        {
            pPolar = Objects2d::polarAt(i);
            if(pPolar && pPolar->foilName()==Objects2d::curFoil()->name())
            {
                //set this one
                setCurPolar(pPolar);
                break;
            }
        }
    }

    if(Objects2d::curPolar())
    {
        if(Objects2d::curPolar()->foilName() != Objects2d::curFoil()->name())
        {

            bool bFound = false;
            for (int i=0; i<Objects2d::polarCount(); i++)
            {
                Polar *pOldPolar = Objects2d::polarAt(i);
                if ((pOldPolar->foilName() == Objects2d::curFoil()->name()) &&
                        (pOldPolar->polarName() == Objects2d::curPolar()->polarName()))
                {
                    setCurPolar(pOldPolar);
                    bFound = true;
                    break;
                }
            }
            if(!bFound)
            {
                setCurPolar(nullptr);
                setCurOpp(nullptr);
            }
        }
        s_bInitBL = true;
        m_pchInitBL->setChecked(s_bInitBL);
    }

    //    m_XFoil.InitXFoilAnalysis(Objects2d::curPolar(), s_bViscous); //useless, will be done in XFoilTask
    m_bResetCurves = true;
    setAnalysisParams();
    setOpp();
    return Objects2d::curPolar();
}


/**
 * Initializes XDirect with the OpPoint with the specified aoa.
 * If the OpPoint cannot be found for the active Foil and Polar, a stock OpPoint associated to the current foil and polar will be set.
 * @param Alpha the aoa of the OpPoint to ser
 * @return a pointer to the OpPoint object which has been set.
 */
OpPoint * XDirect::setOpp(double Alpha)
{
    OpPoint *pOpp(nullptr);

    if(!Objects2d::curFoil() || !Objects2d::curPolar())
    {
        setCurOpp(nullptr);
        return nullptr;
    }

    if(Alpha < -1234567.0) //the default
    {
        if(Objects2d::curOpp() && Objects2d::curOpp()->foilName() == Objects2d::curFoil()->name() &&
                Objects2d::curOpp()->polarName()==Objects2d::curPolar()->polarName())
            pOpp = Objects2d::curOpp();
        else if(Objects2d::curOpp())
        {
            //try to use the same alpha
            double aoa = Objects2d::curOpp()->aoa();
            pOpp = Objects2d::getOpp(Objects2d::curFoil(), Objects2d::curPolar(), aoa);
        }
    }
    else
    {
        pOpp = Objects2d::getOpp(Objects2d::curFoil(), Objects2d::curPolar(), Alpha);
    }
    return setOpp(pOpp);
}


OpPoint *XDirect::setOpp(OpPoint *pOpp)
{
    if(!pOpp)
    {
        //if unsuccessful so far,
        //try to get the first one from the array
        for(int iOpp=0; iOpp<Objects2d::oppCount(); iOpp++)
        {
            OpPoint *pOldOpp = Objects2d::oppAt(iOpp);
            if(pOldOpp->foilName()==Objects2d::curFoil()->name() && pOldOpp->polarName()==Objects2d::curPolar()->polarName())
            {
                pOpp = pOldOpp;
                break;
            }
        }
    }

    setCurOpp(pOpp);
    m_bResetCurves = true;

    setControls();

    return Objects2d::curOpp();
}


/**
 * Initializes the widgets with the sequence parameters for the type of the active Polar object.
 */
void XDirect::setOpPointSequence()
{
    m_pchSequence->setEnabled(Objects2d::curPolar());
    m_pdeAlphaMin->setEnabled(Objects2d::curPolar());
    m_ppbAnalyze->setEnabled(Objects2d::curPolar());
    m_pchInitBL->setEnabled(Objects2d::curPolar());
    m_pchStoreOpp->setEnabled(Objects2d::curPolar());

    if(XFoilAnalysisDlg::s_bSequence && Objects2d::curPolar())
    {
        m_pchSequence->setCheckState(Qt::Checked);
        m_pdeAlphaMax->setEnabled(true);
        m_pdeAlphaDelta->setEnabled(true);
    }
    else if (Objects2d::curPolar())
    {
        m_pchSequence->setCheckState(Qt::Unchecked);
        m_pdeAlphaMax->setEnabled(false);
        m_pdeAlphaDelta->setEnabled(false);
    }
    else
    {
        m_pdeAlphaMax->setEnabled(false);
        m_pdeAlphaDelta->setEnabled(false);
    }


    if(Objects2d::curPolar() && Objects2d::curPolar()->polarType()!=xfl::FIXEDAOAPOLAR)
    {
        if(m_prbSpec3->isChecked())
        {
            m_prbSpec1->setChecked(true);
            s_bAlpha = true;
        }

        if(s_bAlpha)
        {
            m_pdeAlphaMin->setValue(XFoilAnalysisDlg::s_Alpha);
            m_pdeAlphaMax->setValue(XFoilAnalysisDlg::s_AlphaMax);
            m_pdeAlphaDelta->setValue(XFoilAnalysisDlg::s_AlphaDelta);
        }
        else
        {
            m_pdeAlphaMin->setValue(XFoilAnalysisDlg::s_Cl);
            m_pdeAlphaMax->setValue(XFoilAnalysisDlg::s_ClMax);
            m_pdeAlphaDelta->setValue(XFoilAnalysisDlg::s_ClDelta);
        }
        m_prbSpec1->setEnabled(true);
        m_prbSpec2->setEnabled(true);
        m_prbSpec3->setEnabled(false);
    }
    else if(Objects2d::curPolar() && Objects2d::curPolar()->isFixedaoaPolar())
    {
        m_prbSpec3->setChecked(true);
        s_bAlpha = true;        // no choice with type 4 polars
        m_pdeAlphaMin->setValue(XFoilAnalysisDlg::s_ReMin);
        m_pdeAlphaMax->setValue(XFoilAnalysisDlg::s_ReMax);
        m_pdeAlphaDelta->setValue(XFoilAnalysisDlg::s_ReDelta);
        m_prbSpec1->setEnabled(false);
        m_prbSpec2->setEnabled(false);
        m_prbSpec3->setEnabled(true);
    }
    else
    {
        m_prbSpec1->setEnabled(false);
        m_prbSpec2->setEnabled(false);
        m_prbSpec3->setEnabled(false);
    }
}


/**
 * Sets the axes titles for the specified graph
 * @param pGraph a pointer to the Graph object for which the titles will be set
 */
void XDirect::setGraphTitles(Graph* pGraph)
{
    if(!pGraph) return;

    QString Title;
    Title = Polar::variableName(pGraph->xVariable());
    pGraph->setXTitle(Title);
    Title = Polar::variableName(pGraph->yVariable());
    pGraph->setYTitle(Title);
}



/**
 * Creates the GUI associated to the toolbar.
 */
void XDirect::setupLayout()
{
    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);


    QGroupBox *pAnalysisBox = new QGroupBox(tr("Analysis settings"));
    {
        pAnalysisBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        QVBoxLayout *pAnalysisGroup = new QVBoxLayout;
        {
            m_pchSequence = new QCheckBox(tr("Sequence"));
            m_pchStoreOpp = new QCheckBox(tr("Store Opp"));
            m_ppbAnalyze  = new QPushButton(tr("Analyze"));

            QHBoxLayout *pSpecVarsLayout = new QHBoxLayout;
            {
                m_prbSpec1 = new QRadioButton(QChar(0x3B1));

                m_prbSpec2 = new QRadioButton(tr("Cl"));
                m_prbSpec3 = new QRadioButton(tr("Re"));
                pSpecVarsLayout->addWidget(m_prbSpec1);
                pSpecVarsLayout->addWidget(m_prbSpec2);
                pSpecVarsLayout->addWidget(m_prbSpec3);
            }

            QGridLayout *pSequenceGroupLayout = new QGridLayout;
            {
                QLabel *pAlphaMinLab   = new QLabel(tr("Start="));
                QLabel *pAlphaMaxLab   = new QLabel(tr("End="));
                QLabel *pDeltaAlphaLab = new QLabel("<p>&Delta;=</p>");
                pDeltaAlphaLab->setAlignment(Qt::AlignRight);
                pAlphaMinLab->setAlignment(Qt::AlignRight);
                pAlphaMaxLab->setAlignment(Qt::AlignRight);

                m_plabUnit1 = new QLabel("<p>&deg;</p>");
                m_plabUnit2 = new QLabel("<p>&deg;</p>");
                m_plabUnit3 = new QLabel("<p>&deg;</p>");

                m_pdeAlphaMin   = new DoubleEdit();
                m_pdeAlphaMax   = new DoubleEdit();
                m_pdeAlphaDelta = new DoubleEdit();
                m_pdeAlphaMin->setAlignment(Qt::AlignRight);
                m_pdeAlphaMax->setAlignment(Qt::AlignRight);
                m_pdeAlphaDelta->setAlignment(Qt::AlignRight);
                pSequenceGroupLayout->addWidget(pAlphaMinLab,    1,1);
                pSequenceGroupLayout->addWidget(pAlphaMaxLab,    2,1);
                pSequenceGroupLayout->addWidget(pDeltaAlphaLab,  3,1);
                pSequenceGroupLayout->addWidget(m_pdeAlphaMin,   1,2);
                pSequenceGroupLayout->addWidget(m_pdeAlphaMax,   2,2);
                pSequenceGroupLayout->addWidget(m_pdeAlphaDelta, 3,2);
                pSequenceGroupLayout->addWidget(m_plabUnit1,     1,3);
                pSequenceGroupLayout->addWidget(m_plabUnit2,     2,3);
                pSequenceGroupLayout->addWidget(m_plabUnit3,     3,3);
            }

            m_pchInitBL   = new QCheckBox(tr("Init BL"));

            pAnalysisGroup->addLayout(pSpecVarsLayout);
            pAnalysisGroup->addStretch(1);
            pAnalysisGroup->addWidget(m_pchSequence);
            pAnalysisGroup->addLayout(pSequenceGroupLayout);
            pAnalysisGroup->addStretch(1);
            pAnalysisGroup->addWidget(m_pchInitBL);
            pAnalysisGroup->addWidget(m_pchStoreOpp);
            pAnalysisGroup->addWidget(m_ppbAnalyze);
        }
        pAnalysisBox->setLayout(pAnalysisGroup);

    }

    m_pDisplayBox = new QGroupBox(tr("Display"));
    {
        QVBoxLayout *pDisplayGroup = new QVBoxLayout;
        {
            m_pchActiveOppOnly = new QCheckBox(tr("Active operating point only"));
            m_pchShowBL        = new QCheckBox(tr("Displacement thickness"));
            m_pchShowPressure  = new QCheckBox(tr("Pressure"));
            m_pchAnimate       = new QCheckBox(tr("Animate"));
            m_pslAnimateSpeed  = new QSlider(Qt::Horizontal);
            m_pslAnimateSpeed->setMinimum(0);
            m_pslAnimateSpeed->setMaximum(1000);
            m_pslAnimateSpeed->setSliderPosition(500);
            m_pslAnimateSpeed->setTickInterval(50);
            m_pslAnimateSpeed->setTickPosition(QSlider::TicksBelow);
            pDisplayGroup->addWidget(m_pchActiveOppOnly);
            pDisplayGroup->addWidget(m_pchShowBL);
            pDisplayGroup->addWidget(m_pchShowPressure);
            pDisplayGroup->addWidget(m_pchAnimate);
            pDisplayGroup->addWidget(m_pslAnimateSpeed);
        }
        m_pDisplayBox->setLayout(pDisplayGroup);
        m_pDisplayBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
    }


    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(pAnalysisBox);
        pMainLayout->addStretch();
        pMainLayout->addWidget(m_pDisplayBox);
    }

    setLayout(pMainLayout);
}


/**
 * Interrupts the OpPoint animation
 */
void XDirect::stopAnimate()
{
    if(m_bAnimate)
    {
        m_pAnimateTimer->stop();
        m_bAnimate = false;
        m_pchAnimate->setChecked(false);
        setOpp();
    }
}


/**
 * Updates the curve's style based on the selection in the comboboxes.
 */
void XDirect::updateCurveStyle(LineStyle const &ls)
{
    m_LineStyle = ls;
    if(m_bPolarView && Objects2d::curPolar())
    {
        Objects2d::curPolar()->setColor(m_LineStyle.m_Color.red(), m_LineStyle.m_Color.green(), m_LineStyle.m_Color.blue());
        Objects2d::curPolar()->setStipple(m_LineStyle.m_Stipple);
        Objects2d::curPolar()->setWidth(m_LineStyle.m_Width);
        Objects2d::curPolar()->setPointStyle(m_LineStyle.m_Symbol);

        if(DisplayOptions::isAlignedChildrenStyle())
        {
            Objects2d::setPolarChildrenStyle(Objects2d::curPolar());
        }

        m_bResetCurves = true;
    }
    else if (!m_bPolarView && Objects2d::curOpp())
    {
        Objects2d::curOpp()->setTheStyle(m_LineStyle);
        m_bResetCurves = true;
    }

    updateView();
    emit projectModified();
}


/**
 * Refreshes the 2d central display.
 */
void XDirect::updateView()
{
    if (!m_bPolarView)
    {
        if(m_bResetCurves) createOppCurves();
    }
    else
    {
        if(m_bResetCurves) createPolarCurves();
    }
    s_pMainFrame->m_pXDirectTileWidget->update();
    update();
}


/**
 * The user has requested the duplication of the current Foil.
 */
void XDirect::onDuplicateFoil()
{
    if(!Objects2d::curFoil()) return;

    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(Objects2d::curFoil());

    if(addNewFoil(pNewFoil))
    {
        m_pFoilTreeView->insertFoil(pNewFoil);

        setFoil(pNewFoil);
        emit projectModified();
    }
}


/**
 * The user has requested to rename the current Foil.
 */
void XDirect::onRenameCurFoil()
{
    renameFoil(Objects2d::curFoil());
    m_pFoilTreeView->fillModelView();
    setFoil(Objects2d::curFoil());
    m_pFoilTreeView->setObjectProperties();
    emit projectModified();
}


void XDirect::onFoilDescription()
{
    if(!Objects2d::curFoil()) return;

    TextDlg dlg(Objects2d::curFoil()->description(), this);

    if(dlg.exec() != QDialog::Accepted) return;
    Objects2d::curFoil()->setDescription(dlg.newText());
    m_pFoilTreeView->setObjectProperties();
}


/**
 * Adds a new Foil to the array.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be added to the array
 * @return a pointer to the input Foil, or NULL if the operation was user-cancelled. @todo what's the point ?
 */
Foil* XDirect::addNewFoil(Foil *pFoil)
{
    if(!pFoil) return nullptr;
    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, pFoil->name(), tr("Enter the foil's new name"));
    if(renDlg.exec() != QDialog::Rejected)
    {
        pFoil->setName(renDlg.newName());
        Objects2d::insertThisFoil(pFoil);

        return pFoil;
    }
    return nullptr;
}


/**
 * Renames the current Foil.
 * Requests a name, and overwrites any former Foil with that name.
 * @param pFoil a pointer to the Foil to be renamed.
 */
void XDirect::renameFoil(Foil *pFoil)
{
    if(!pFoil) return;
    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(s_pMainFrame);
    renDlg.initDialog(&NameList, pFoil->name(), tr("Enter the foil's new name"));

    if(renDlg.exec() != QDialog::Rejected)
    {
        Objects2d::renameThisFoil(pFoil, renDlg.newName());
    }
}


void XDirect::setView(xfl::enumGraphView eView)
{
    if (m_bPolarView)
    {
        m_iPlrView = eView;
    }
}


void XDirect::setGraphTiles()
{
    if(m_bPolarView)
    {
        switch(m_iPlrView)
        {
            case xfl::ONEGRAPH:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 1, 0);
                break;
            case xfl::TWOGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 2, 0);
                break;
            case xfl::FOURGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, 4, 0);
                break;
            default:
            case xfl::ALLGRAPHS:
                s_pMainFrame->m_pXDirectTileWidget->setGraphList(m_PlrGraph, m_PlrGraph.count(), 0);
                break;
        }
    }
    else
    {
        QVector<Graph*> pGraphList;
        pGraphList.append(&m_CpGraph);
        s_pMainFrame->m_pXDirectTileWidget->setGraphList(pGraphList, 1, 0, Qt::Vertical);
    }
}


/**
 * Sets the Foil scale in the OpPoint view.
 */
void XDirect::setFoilScale()
{
    s_pMainFrame->m_pXDirectTileWidget->opPointWidget()->setFoilScale();
}


/**
 * Imports the analysis definition from an XML file
 */
void XDirect::onImportXMLAnalysis()
{
    QString PathName;
    PathName = QFileDialog::getOpenFileName(s_pMainFrame, tr("Open XML File"),
                                            xfl::s_LastDirName,
                                            tr("Analysis XML file")+"(*.xml)");
    if(!PathName.length())        return ;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+PathName;
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    XFile.close();
    importAnalysisFromXML(XFile);
}


/**
 * Imports the Analysis definition from an XML file
 */
void XDirect::importAnalysisFromXML(QFile &xmlFile)
{
    if (!xmlFile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+xmlFile.fileName();
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strange);
        return;
    }

    Polar *pPolar = new Polar;
    XmlPolarReader polarReader(xmlFile, pPolar);
    polarReader.readXMLPolarFile();

    if(polarReader.hasError())
    {
        QString errorMsg = polarReader.errorString() + QString("\nline %1 column %2").arg(polarReader.lineNumber()).arg(polarReader.columnNumber());
        QMessageBox::warning(s_pMainFrame, "XML read", errorMsg, QMessageBox::Ok);
    }
    else
    {
        Foil *pFoil = Objects2d::foil(pPolar->foilName());
        if(!pFoil && Objects2d::curFoil())
        {
            s_pMainFrame->statusBar()->showMessage(tr("Attaching the analysis to the active foil"));
            pPolar->setFoilName(Objects2d::curFoil()->name());
//            pFoil = Objects2d::curFoil();
        }
        else if(!pFoil)
        {
            s_pMainFrame->statusBar()->showMessage(tr("No foil to attach the polar to"));
            delete pPolar;
            return;
        }

        Objects2d::addPolar(pPolar);
        setCurOpp(nullptr);
        setCurPolar(pPolar);

        m_pFoilTreeView->insertPolar(pPolar);
        emit projectModified();
        setControls();
        setAnalysisParams();
    }
    updateView();
}


/**
 * Exports the analysis data to an XML file
 */
void XDirect::onExportXMLAnalysis()
{
    if(!Objects2d::curPolar()) return ;// is there anything to export ?

    Polar *pCurPolar = Objects2d::curPolar();
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = pCurPolar->polarName();
    strong.replace("/", "_");
    strong.replace(".", "_");

    FileName = QFileDialog::getSaveFileName(s_pMainFrame, tr("Export analysis definition to xml file"),
                                            xfl::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";


    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;


    XmlPolarWriter polarWriter(XFile);
    polarWriter.writeXMLPolar(pCurPolar);

    XFile.close();
}


void XDirect::onOptim2d()
{
    Optim2d o2d(s_pMainFrame);
    o2d.setFoil(curFoil());   
    o2d.exec();
    if(o2d.isModified())
    {
        m_pFoilTreeView->fillModelView();
        emit projectModified();
    }
    updateView();
}

