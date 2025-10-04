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

#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QColorDialog>

#include <xflobjects/editors/wingdlg.h>

#include <xfl3d/controls/w3dprefs.h>
#include <xfl3d/views/gl3dwingview.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/editors/inertiadlg.h>
#include <xflobjects/editors/wingdelegate.h>
#include <xflobjects/editors/wingscaledlg.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wing.h>
#include <xflobjects/objects_global.h>
#include <xflobjects/xml/xmlplanereader.h>
#include <xflobjects/xml/xmlplanewriter.h>
#include <xflwidgets/color/colormenubtn.h>
#include <xflwidgets/customwts/doubleedit.h>


QByteArray WingDlg::s_WindowGeometry;
QByteArray WingDlg::s_HSplitterSizes;
QByteArray WingDlg::s_LeftSplitterSizes;

bool WingDlg::s_bOutline    = true;
bool WingDlg::s_bSurfaces   = true;
bool WingDlg::s_bVLMPanels  = false;
bool WingDlg::s_bAxes       = true;
bool WingDlg::s_bShowMasses = false;
bool WingDlg::s_bFoilNames  = false;

WingDlg::WingDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Wing Edition"));
    setWindowFlags(Qt::Window);

    m_pWing = nullptr;

    m_iSection   = -1;

    m_pspLeftSide = nullptr;

    m_bEnableName              = true;
    m_bAcceptName              = true;
    m_bTrans                   = false;
    m_bRightSide               = true;
    m_bChanged                 = false;
    m_bDescriptionChanged      = false;

    m_LastPoint.setX(0);
    m_LastPoint.setY(0);

    m_pResetScales   = new QAction(tr("Reset Scales"),   this);
    m_pInsertBefore  = new QAction(tr("Insert Before"),  this);
    m_pInsertAfter   = new QAction(tr("Insert after"),   this);
    m_pDeleteSection = new QAction(tr("Delete section"), this);
    m_pResetSection  = new QAction(tr("Reset section"),  this);

    m_pContextMenu = new QMenu(tr("Section"),this);
    m_pContextMenu->addAction(m_pInsertBefore);
    m_pContextMenu->addAction(m_pInsertAfter);
    m_pContextMenu->addAction(m_pDeleteSection);
    m_pContextMenu->addAction(m_pResetSection);

    setupLayout();

    connectSignals();

    setMouseTracking(true);
}


WingDlg::~WingDlg()
{
    if(m_pWingModel)    delete m_pWingModel;
    if(m_pWingDelegate) delete m_pWingDelegate;
}


bool WingDlg::checkWing()
{
    if(!m_pWing->m_Name.length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a name for the wing"));
        m_pleWingName->setFocus();
        return false;
    }

    for (int k=1; k<m_pWing->NWingSection(); k++)
    {
        if(m_pWing->YPosition(k)*1.00001 < m_pWing->YPosition(k-1))
        {
            QMessageBox::warning(this, tr("Warning"), tr("Warning : Panel sequence is inconsistent. The sections should be ordered from root to tip"));
            return false;
        }
    }

    for (int k=0; k<m_pWing->NWingSection(); k++)
    {
        if(fabs(m_pWing->Chord(k))<0.0001)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Zero length chords will cause a division by zero and should be avoided."));
            return false;
        }
    }
    for (int k=1; k<m_pWing->NWingSection(); k++)
    {
        WingSection const pSection = m_pWing->m_Section.at(k);
        Foil const *pLeftFoil = Objects2d::foil(pSection.m_LeftFoilName);
        Foil const *pRightFoil = Objects2d::foil(pSection.m_RightFoilName);
        if(pLeftFoil )
        {
            if((pLeftFoil->m_TEXHinge>=99&& pLeftFoil->m_bTEFlap) ||(pLeftFoil->m_LEXHinge<0.01&&pLeftFoil->m_bLEFlap))
            {
                QMessageBox::warning(this, tr("Warning"), pLeftFoil->name()+": "+tr("Zero length flaps will cause a division by zero and should be avoided."));
                return false;
            }
        }
        if(pRightFoil)
        {
            if((pRightFoil->m_TEXHinge>=99&& pRightFoil->m_bTEFlap) ||(pRightFoil->m_LEXHinge<0.01&&pRightFoil->m_bLEFlap))
            {
                QMessageBox::warning(this, tr("Warning"), pRightFoil->name()+": "+tr("Zero length flaps will cause a division by zero and should be avoided."));
                return false;
            }
        }
    }

    int NYPanels = 0;
    for(int j=0; j<m_pWing->NWingSection()-1; j++)
    {
        NYPanels += m_pWing->NYPanels(j);
    }

    if(m_pWing->m_nFlaps>=20)
    {
        QString strong = tr("Only 10 flaps x 2 will be handled");
        if (QMessageBox::Ok != QMessageBox::question(window(), tr("Question"), strong, QMessageBox::Ok|QMessageBox::Cancel))
            return false;
    }
    return true;
}



void WingDlg::computeGeometry()
{
    // Computes the wing's characteristics from the panel data
    m_pWing->computeGeometry();
    m_pWing->createSurfaces(Vector3d(0.0,0.0,0.0), 0.0, 0.0);

    for (int j=0; j<m_pWing->m_Surface.size(); j++)
        m_pWing->surface(j)->setMeshSidePoints(nullptr, 0.0, 0.0);
}



void WingDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    // Display the context menu
    if(m_ptvWingSections->geometry().contains(pEvent->pos())) m_pContextMenu->exec(pEvent->globalPos());
}


void WingDlg::connectSignals()
{
    connect(m_pInsertBefore,  SIGNAL(triggered()), SLOT(onInsertBefore()));
    connect(m_pInsertAfter,   SIGNAL(triggered()), SLOT(onInsertAfter()));
    connect(m_pDeleteSection, SIGNAL(triggered()), SLOT(onDeleteSection()));
    connect(m_pResetSection,  SIGNAL(triggered()), SLOT(onResetSection()));

    connect(m_pResetScales,   SIGNAL(triggered()), m_pglWingView, SLOT(on3dReset()));
    connect(m_ptbIso,         SIGNAL(clicked()),   m_pglWingView, SLOT(on3dIso()));
    connect(m_ptbX,           SIGNAL(clicked()),   m_pglWingView, SLOT(on3dFront()));
    connect(m_ptbY,           SIGNAL(clicked()),   m_pglWingView, SLOT(on3dLeft()));
    connect(m_ptbZ,           SIGNAL(clicked()),   m_pglWingView, SLOT(on3dTop()));
    connect(m_ppbReset,       SIGNAL(clicked()),   m_pglWingView, SLOT(on3dReset()));
    connect(m_ptbFlip,        SIGNAL(clicked()),   m_pglWingView, SLOT(on3dFlip()));


    connect(m_pchFoilNames,   SIGNAL(clicked()),SLOT(onFoilNames()));
    connect(m_pchShowMasses,  SIGNAL(clicked()),SLOT(onShowMasses()));

    connect(m_pchAxes,        SIGNAL(clicked()), SLOT(onAxes()));
    connect(m_pchPanels,      SIGNAL(clicked()), SLOT(onPanels()));
    connect(m_pchSurfaces,    SIGNAL(clicked()), SLOT(onSurfaces()));
    connect(m_pchOutline,     SIGNAL(clicked()), SLOT(onOutline()));

    connect(m_ppbInsertBefore,  SIGNAL(clicked()), SLOT(onInsertBefore()));
    connect(m_ppbInsertAfter,   SIGNAL(clicked()), SLOT(onInsertAfter()));
    connect(m_ppbDeleteSection, SIGNAL(clicked()), SLOT(onDeleteSection()));

    connect(m_ppbResetMesh,     SIGNAL(clicked()), SLOT(onResetMesh()));
    connect(m_pcmbWingColor,    SIGNAL(clickedCB(QColor)), SLOT(onWingColor(QColor)));

    connect(m_pchSymetric,      SIGNAL(clicked()), SLOT(onSymetric()));
    connect(m_prbRightSide,     SIGNAL(clicked()), SLOT(onSide()));
    connect(m_prbLeftSide,      SIGNAL(clicked()), SLOT(onSide()));

    connect(m_pteWingDescription, SIGNAL(textChanged()), SLOT(onDescriptionChanged()));

    connect(m_pInertia,       SIGNAL(triggered()),    SLOT(onInertia()));
    connect(m_pScaleWing,     SIGNAL(triggered()),    SLOT(onScaleWing()));
    connect(m_pglWingView,    SIGNAL(viewModified()), SLOT(onCheckViewIcons()));
    connect(m_pImportWingAct, SIGNAL(triggered()),    SLOT(onImportWing()));
    connect(m_pExportWingAct, SIGNAL(triggered()),    SLOT(onExportWing()));
    connect(m_pImportWingXml, SIGNAL(triggered()),    SLOT(onImportWingFromXML()));
    connect(m_pExportWingXml, SIGNAL(triggered()),    SLOT(onExportWingToXML()));
}


/**
 * Unselects all the 3D-view icons.
 */
void WingDlg::onCheckViewIcons()
{
    m_ptbIso->setChecked(false);
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
}



void WingDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)         onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void WingDlg::createXPoints(int NXPanels, int XDist, Foil *pFoilA, Foil *pFoilB, double *xPointA, double *xPointB, int &NXLead, int &NXFlap)
{
    // the chordwise panel distribution is set i.a.w. with the flap hinges;

    int l;
    int NXFlapA, NXFlapB, NXLeadA, NXLeadB;
    double dl, dl2;
    double xHingeA, xHingeB;
    if(pFoilA && pFoilA->m_bTEFlap) xHingeA=pFoilA->m_TEXHinge/100.0; else xHingeA=1.0;
    if(pFoilB && pFoilB->m_bTEFlap) xHingeB=pFoilB->m_TEXHinge/100.0; else xHingeB=1.0;

    NXFlapA = int((1.0-xHingeA) * NXPanels);
    NXFlapB = int((1.0-xHingeB) * NXPanels);
    if(pFoilA && pFoilA->m_bTEFlap && NXFlapA==0) NXFlapA++;
    if(pFoilB && pFoilB->m_bTEFlap && NXFlapB==0) NXFlapB++;
    NXLeadA = NXPanels - NXFlapA;
    NXLeadB = NXPanels - NXFlapB;

    NXFlap  = qMax(NXFlapA, NXFlapB);
    if(NXFlap>NXPanels/2) NXFlap=int(NXPanels/2);
    NXLead  = NXPanels - NXFlap;

    for(l=0; l<NXFlapA; l++)
    {
        dl =  double(l);
        dl2 = double(NXFlapA);
        if(XDist==1)
            xPointA[l] = 1.0 - (1.0-xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointA[l] = 1.0 - (1.0-xHingeA) * (dl/dl2);
    }
    for(l=0; l<NXLeadA; l++)
    {
        dl =  double(l);
        dl2 = double(NXLeadA);
        if(XDist==1)
            xPointA[l+NXFlapA] = xHingeA - (xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointA[l+NXFlapA] = xHingeA - (xHingeA) * (dl/dl2);
    }

    for(l=0; l<NXFlapB; l++)
    {
        dl =  double(l);
        dl2 = double(NXFlapB);
        if(XDist==1)
            xPointB[l] = 1.0 - (1.0-xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointB[l] = 1.0 - (1.0-xHingeB) * (dl/dl2);
    }
    for(l=0; l<NXLeadB; l++)
    {
        dl =  double(l);
        dl2 = double(NXLeadB);
        if(XDist==1)
            xPointB[l+NXFlapB] = xHingeB - (xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointB[l+NXFlapB] = xHingeB - (xHingeB) * (dl/dl2);
    }

    xPointA[NXPanels] = 0.0;
    xPointB[NXPanels] = 0.0;
}


void WingDlg::fillDataTable()
{
    if(!m_pWing) return;

    m_pWingModel->setRowCount(m_pWing->NWingSection());

    for(int i=0; i<m_pWing->NWingSection(); i++)
    {
        fillTableRow(i);
    }
}


void WingDlg::fillTableRow(int row)
{
    QString strong;
    QModelIndex ind;

    ind = m_pWingModel->index(row, 0, QModelIndex());
    m_pWingModel->setData(ind, m_pWing->YPosition(row) * Units::mtoUnit());

    ind = m_pWingModel->index(row, 1, QModelIndex());
    m_pWingModel->setData(ind, m_pWing->Chord(row) * Units::mtoUnit());

    ind = m_pWingModel->index(row, 2, QModelIndex());
    m_pWingModel->setData(ind, m_pWing->Offset(row) * Units::mtoUnit());

    ind = m_pWingModel->index(row, 3, QModelIndex());
    m_pWingModel->setData(ind, m_pWing->Dihedral(row));

    ind = m_pWingModel->index(row, 4, QModelIndex());
    m_pWingModel->setData(ind, m_pWing->Twist(row));

    ind = m_pWingModel->index(row, 5, QModelIndex());
    if(m_bRightSide) m_pWingModel->setData(ind, m_pWing->rightFoilName(row));
    else             m_pWingModel->setData(ind, m_pWing->leftFoilName(row));

    if(row<m_pWing->NWingSection())
    {
        ind = m_pWingModel->index(row, 6, QModelIndex());
        m_pWingModel->setData(ind, m_pWing->NXPanels(row));

        if(m_pWing->XPanelDist(row)==xfl::UNIFORM)       strong = tr("Uniform");
        else if(m_pWing->XPanelDist(row)==xfl::COSINE)   strong = tr("Cosine");
        ind = m_pWingModel->index(row, 7, QModelIndex());
        m_pWingModel->setData(ind, strong);

        ind = m_pWingModel->index(row, 8, QModelIndex());
        m_pWingModel->setData(ind, m_pWing->NYPanels(row));

        if(m_pWing->YPanelDist(row)==xfl::UNIFORM)            strong = tr("Uniform");
        else if(m_pWing->YPanelDist(row)==xfl::COSINE)        strong = tr("Cosine");
        else if(m_pWing->YPanelDist(row)==xfl::SINE)          strong = tr("Sine");
        else if(m_pWing->YPanelDist(row)== xfl::INVERSESINE)  strong = tr("-Sine");
        ind = m_pWingModel->index(row, 9, QModelIndex());
        m_pWingModel->setData(ind, strong);
    }
    else
    {
        strong = " ";
        ind = m_pWingModel->index(row, 6, QModelIndex());
        m_pWingModel->setData(ind, 0);
        ind = m_pWingModel->index(row, 7, QModelIndex());
        m_pWingModel->setData(ind, " ");
        ind = m_pWingModel->index(row, 8, QModelIndex());
        m_pWingModel->setData(ind, 0);
        ind = m_pWingModel->index(row, 9, QModelIndex());
        m_pWingModel->setData(ind, " ");
    }
}


bool WingDlg::initDialog(Wing *pWing)
{
    m_iSection = 0;

    m_pWing = pWing;
    if(!m_pWing) return false;

    m_pglWingView->setWing(m_pWing);
    computeGeometry();

    m_pleWingName->setText(m_pWing->m_Name);
    if(m_pWing->m_Description.length())
    {
        m_pteWingDescription->setPlainText(m_pWing->m_Description);
    }
    else
    {
        m_pteWingDescription->setPlainText("");
    }

    if(!m_bAcceptName) m_pleWingName->setEnabled(false);
    m_pchSymetric->setChecked(m_pWing->m_bSymetric);
    m_prbRightSide->setChecked(m_pWing->m_bSymetric);
    m_prbLeftSide->setEnabled(!m_pWing->m_bSymetric);
    m_prbRightSide->setChecked(m_bRightSide);
    m_prbLeftSide->setChecked(!m_bRightSide);

    m_pchSurfaces->setChecked(m_pglWingView->m_bSurfaces);
    m_pchOutline->setChecked(m_pglWingView->m_bOutline);
    m_pchAxes->setChecked(m_pglWingView->m_bAxes);
    m_pchPanels->setChecked(m_pglWingView->m_bVLMPanels);
    m_pchFoilNames->setChecked(m_pglWingView->m_bFoilNames);
    m_pchShowMasses->setChecked(m_pglWingView->m_bShowMasses);

    m_pcmbWingColor->setColor(m_pWing->m_Color);

    m_ptvWingSections->setFont(DisplayOptions::tableFont());

    fillDataTable();
    setWingData();
    m_ptvWingSections->selectRow(m_iSection);
    setCurrentSection(m_iSection);
    return true;
}


void WingDlg::keyPressEvent(QKeyEvent *pEvent)
{
    //    bool bShift = false;
    //    bool bCtrl  = false;
    //    if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    //    if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else                          accept();

            break;
        }
        case Qt::Key_F12:
        {
            onInertia();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Delete:
        {
            onDeleteSection();
            break;
        }

        default:
            pEvent->ignore();
    }
}


void WingDlg::onFoilNames()
{
    m_pglWingView->m_bFoilNames = m_pchFoilNames->isChecked();
    m_pglWingView->update();
}


void WingDlg::onShowMasses()
{
    m_pglWingView->m_bShowMasses = m_pchShowMasses->isChecked();
    m_pglWingView->update();
}


void WingDlg::onDescriptionChanged()
{
    m_bDescriptionChanged=true;
}


void WingDlg::onCellChanged(QWidget *)
{
    m_bChanged = true;
    m_pglWingView->resetglWing();
    readParams();
    setWingData();
    m_pglWingView->update();
}


void WingDlg::onDeleteSection()
{
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection()) return;

    if(m_iSection==0)
    {
        QMessageBox::warning(this, tr("Warning"),tr("The first section cannot be deleted"));
        return;
    }
    m_ptvWingSections->closePersistentEditor(m_ptvWingSections->currentIndex());

    int size = m_pWingModel->rowCount();
    if(size<=2) return;

    int ny = m_pWing->NYPanels(m_iSection-1) + m_pWing->NYPanels(m_iSection);

    m_pWing->removeWingSection(m_iSection);

    m_pWing->setNYPanels(m_iSection-1, ny);

    fillDataTable();
    computeGeometry();
    setWingData();
    m_bChanged = true;
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onInertia()
{
    InertiaDlg dlg(this);
    dlg.m_pWing = m_pWing;

    //save inertia properties
    QVector<PointMass> PtMass;
    PtMass.clear();

    for(int i=0; i< m_pWing->m_PointMass.size(); i++)
    {
        PtMass.append(m_pWing->m_PointMass.at(i));
    }

    dlg.initDialog();
    if(dlg.exec() == QDialog::Accepted)
    {
        if(dlg.m_bChanged) m_bChanged = true;
        m_pglWingView->update();
    }
    else
    {
        // restore saved inertia
        m_pWing->clearPointMasses();
        for(int i=0; i< PtMass.size(); i++)
        {
            m_pWing->m_PointMass.append(PtMass.at(i));
        }
    }
}


void WingDlg::onInsertBefore()
{
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection()) return;

    if(m_iSection<=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No insertion possible before the first section"));
        return;
    }

    int n = m_iSection;

    m_pWing->insertSection(m_iSection);

    m_pWing->setYPosition(n, (m_pWing->YPosition(n+1) + m_pWing->YPosition(n-1)) /2.0);
    m_pWing->setChord(n,     (m_pWing->Chord(n+1)     + m_pWing->Chord(n-1))     /2.0);
    m_pWing->setOffset(n,    (m_pWing->Offset(n+1)    + m_pWing->Offset(n-1))    /2.0);
    m_pWing->setTwist(n,     (m_pWing->Twist(n+1)     + m_pWing->Twist(n-1))     /2.0);
    m_pWing->setDihedral(n,  (m_pWing->Dihedral(n+1)  + m_pWing->Dihedral(n-1))  /2.0);

    m_pWing->setXPanelDist(n, m_pWing->XPanelDist(n-1));
    m_pWing->setYPanelDist(n, m_pWing->YPanelDist(n-1));

    m_pWing->setRightFoilName(n, m_pWing->rightFoilName(n-1));
    m_pWing->setLeftFoilName(n, m_pWing->leftFoilName(n-1));

    m_pWing->setNXPanels(n, m_pWing->NXPanels(n-1));


    int ny = m_pWing->NYPanels(n-1);
    m_pWing->setNYPanels(n, std::max(2, ny/2));
    m_pWing->setNYPanels(n-1, std::max(2,ny-m_pWing->NYPanels(n)));


    fillDataTable();
    m_ptvWingSections->closePersistentEditor(m_ptvWingSections->currentIndex());
    computeGeometry();
    setWingData();

    m_bChanged = true;
    m_pglWingView->resetglHighlight();
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onInsertAfter()
{
    if(m_iSection <0 || m_iSection>=m_pWing->NWingSection()) return;

    int n = m_iSection;

    if(n<0) n=m_pWing->NWingSection();

    m_pWing->insertSection(m_iSection+1);

    if(n<m_pWing->NWingSection()-2)
    {
        m_pWing->setYPosition(n+1, (m_pWing->YPosition(n)      + m_pWing->YPosition(n+2))     /2.0);
        m_pWing->setChord(n+1,     (m_pWing->Chord(n)    + m_pWing->Chord(n+2))   /2.0);
        m_pWing->setOffset(n+1,    (m_pWing->Offset(n)   + m_pWing->Offset(n+2))  /2.0);
        m_pWing->setTwist(n+1,     (m_pWing->Twist(n)    + m_pWing->Twist(n+2))   /2.0);
    }
    else
    {
        m_pWing->setYPosition(n+1, m_pWing->YPosition(n)*1.1);
        m_pWing->setChord(n+1, m_pWing->Chord(n)/1.1);
        m_pWing->setOffset(n+1, m_pWing->Offset(n) + m_pWing->Chord(n) - m_pWing->Chord(n));
        m_pWing->setTwist(n+1, m_pWing->Twist(n));
    }

    m_pWing->setDihedral(n+1, m_pWing->Dihedral(n));
    m_pWing->setNXPanels(n+1, m_pWing->NXPanels(n));
    m_pWing->setNYPanels(n+1, m_pWing->NYPanels(n));
    m_pWing->setXPanelDist(n+1, m_pWing->XPanelDist(n));
    m_pWing->setYPanelDist(n+1, m_pWing->YPanelDist(n));
    m_pWing->setRightFoilName(n+1, m_pWing->rightFoilName(n));
    m_pWing->setLeftFoilName(n+1, m_pWing->leftFoilName(n));

    int ny = m_pWing->NYPanels(n);
    m_pWing->setNYPanels(n+1, std::max(2,ny/2));
    m_pWing->setNYPanels(n, std::max(2,ny-m_pWing->NYPanels(n+1)));

    //    m_pWing->m_bVLMAutoMesh = true;

    fillDataTable();
    m_ptvWingSections->closePersistentEditor(m_ptvWingSections->currentIndex());

    computeGeometry();
    setWingData();
    m_bChanged = true;
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onResetSection()
{
    int n = m_iSection;

    if((0 < n) && (n < (m_pWing->NWingSection()-1)))
    {
        double ratio;
        ratio = (m_pWing->YPosition(n) - m_pWing->YPosition(n - 1)) / (m_pWing->YPosition(n + 1) - m_pWing->YPosition(n - 1));

        m_pWing->setChord(n,  m_pWing->Chord(n-1)  + ratio * (m_pWing->Chord(n+1) - m_pWing->Chord(n-1)));
        m_pWing->setOffset(n, m_pWing->Offset(n-1) + ratio * (m_pWing->Offset(n+1) - m_pWing->Offset(n-1)));
        m_pWing->setTwist(n,  m_pWing->Twist(n-1)  + ratio * (m_pWing->Twist(n+1) - m_pWing->Twist(n-1)));

        // same code here that in OnResetMesh
        fillDataTable();
        setWingData();
        computeGeometry();
        m_bChanged = true;
        m_pglWingView->resetglWing();
        m_pglWingView->update();
    }
}


void WingDlg::onItemClicked(const QModelIndex &index)
{
    if(index.row()>=m_pWing->NWingSection())
    {
        //the user has filled a cell in the last line
        //so add an item before reading
        m_pWingModel->setRowCount(m_pWing->NWingSection()+1);
        fillTableRow(m_pWing->NWingSection());
    }
    setCurrentSection(index.row());
    m_pglWingView->update();
}



void WingDlg::onOK()
{
    readParams();

    if(!checkWing()) return;

    if(m_pWing->m_bSymetric)
    {
        for (int i=0; i<m_pWing->NWingSection(); i++)
        {
            m_pWing->setLeftFoilName(i, m_pWing->rightFoilName(i));
        }
    }

    m_pWing->computeGeometry();
    m_pWing->computeBodyAxisInertia();

    accept();
}


void WingDlg::onAxes()
{
    m_pglWingView->m_bAxes = m_pchAxes->isChecked();
    m_pglWingView->update();
}


void WingDlg::onSurfaces()
{
    m_pglWingView->m_bSurfaces = m_pchSurfaces->isChecked();
    m_pglWingView->update();
}


void WingDlg::onOutline()
{
    m_pglWingView->m_bOutline = m_pchOutline->isChecked();
    m_pglWingView->update();
}


void WingDlg::onPanels()
{
    m_pglWingView->m_bVLMPanels = m_pchPanels->isChecked();
    m_pglWingView->update();
}


void WingDlg::onResetMesh()
{
    VLMSetAutoMesh();
    fillDataTable();
    setWingData();
    computeGeometry();
    m_bChanged = true;
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onScaleWing()
{
    WingScaleDlg dlg(this);
    dlg.initDialog(m_pWing->m_PlanformSpan,
                   m_pWing->Chord(0),
                   m_pWing->averageSweep(),
                   m_pWing->Twist(m_pWing->NWingSection()-1),
                   m_pWing->m_PlanformArea,
                   m_pWing->m_AR,
                   m_pWing->m_TR);

    if(QDialog::Accepted == dlg.exec())
    {
        if (dlg.m_bSpan || dlg.m_bChord || dlg.m_bSweep || dlg.m_bTwist || dlg.m_bArea || dlg.m_bAR)
        {
            if(dlg.m_bSpan)  m_pWing->scaleSpan(dlg.m_NewSpan);
            if(dlg.m_bChord) m_pWing->scaleChord(dlg.m_NewChord);
            if(dlg.m_bSweep) m_pWing->scaleSweep(dlg.m_NewSweep);
            if(dlg.m_bTwist) m_pWing->scaleTwist(dlg.m_NewTwist);
            if(dlg.m_bArea)  m_pWing->scaleArea(dlg.m_NewArea);
            if(dlg.m_bAR)    m_pWing->scaleAR(dlg.m_NewAR);
            if(dlg.m_bTR)    m_pWing->scaleTR(dlg.m_NewTR);
        }

        fillDataTable();
        m_bChanged = true;
        m_pglWingView->resetglWing();
        m_pglWingView->resetglHighlight();
        computeGeometry();
        m_pglWingView->update();
    }
}


void WingDlg::onSide()
{
    m_bRightSide = m_prbRightSide->isChecked();
    fillDataTable();

    m_bChanged = true;
    m_pglWingView->resetglHighlight();
    m_pglWingView->update();
}


void WingDlg::onSymetric()
{
    if(m_pchSymetric->isChecked())
    {
        m_pWing->m_bSymetric  = true;
        m_bRightSide          = true;
        m_prbLeftSide->setEnabled(false);
        m_prbRightSide->setChecked(true);
        for(int i=0; i<m_pWing->NWingSection(); i++)
        {
            m_pWing->setLeftFoilName(i, m_pWing->rightFoilName(i));
        }
    }
    else
    {
        m_pWing->m_bSymetric    = false;
        m_prbLeftSide->setEnabled(true);
    }

    m_bChanged = true;
    computeGeometry();
    m_pglWingView->resetglWing();
    m_pglWingView->resetglHighlight();
    m_pglWingView->update();
}


void WingDlg::onWingColor(QColor clr)
{
    if(!m_pWing) return;

    if(clr.isValid())
    {
        m_pWing->setColor(clr);
        m_bDescriptionChanged = true;
    }

    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::readParams()
{
    m_pWing->m_Name = m_pleWingName->text();
    QString strange = m_pteWingDescription->toPlainText();
    if(strange == tr("Wing Description")) strange="";
    m_pWing->m_Description = strange;

    for (int i=0; i< m_pWingModel->rowCount();  i++)
    {
        readSectionData(i);
    }

    //Update Geometry
    computeGeometry();
}


void WingDlg::readSectionData(int sel)
{
    if(sel>=m_pWingModel->rowCount()) return;
    double d=0;

    bool bOK=false;
    QString strong;
    QStandardItem *pItem=nullptr;

    pItem = m_pWingModel->item(sel,0);

    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setYPosition(sel, d / Units::mtoUnit());

    pItem = m_pWingModel->item(sel,1);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setChord(sel, d / Units::mtoUnit());

    pItem = m_pWingModel->item(sel,2);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setOffset(sel, d / Units::mtoUnit());

    pItem = m_pWingModel->item(sel,3);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setDihedral(sel,d);

    pItem = m_pWingModel->item(sel,4);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setTwist(sel, d);

    pItem = m_pWingModel->item(sel,5);
    strong =pItem->text();

    if(m_pWing->m_bSymetric)
    {
        m_pWing->setRightFoilName(sel, strong);
        m_pWing->setLeftFoilName(sel, strong);
    }
    else
    {
        if(m_bRightSide)    m_pWing->setRightFoilName(sel, strong);
        else                m_pWing->setLeftFoilName(sel, strong);
    }

    pItem = m_pWingModel->item(sel,6);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setNXPanels(sel, int(qMax(1.0,d)));

    pItem = m_pWingModel->item(sel,7);
    strong =pItem->text();
    strong.replace(" ","");
    if     (strong==tr("Uniform"))   m_pWing->setXPanelDist(sel, xfl::UNIFORM);
    else if(strong==tr("Cosine"))    m_pWing->setXPanelDist(sel, xfl::COSINE);
    else if(strong==tr("Sine"))      m_pWing->setXPanelDist(sel, xfl::SINE);
    else if(strong==tr("-Sine"))     m_pWing->setXPanelDist(sel, xfl::INVERSESINE);

    pItem = m_pWingModel->item(sel,8);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->setNYPanels(sel, int(qMax(1.0,d)));

    pItem = m_pWingModel->item(sel,9);
    strong =pItem->text();
    strong.replace(" ","");

    if     (strong==tr("Uniform"))   m_pWing->setYPanelDist(sel, xfl::UNIFORM);
    else if(strong==tr("Cosine"))    m_pWing->setYPanelDist(sel, xfl::COSINE);
    else if(strong==tr("Sine"))      m_pWing->setYPanelDist(sel, xfl::SINE);
    else if(strong==tr("-Sine"))     m_pWing->setYPanelDist(sel, xfl::INVERSESINE);
}


void WingDlg::accept()
{
    s_bOutline    = m_pglWingView->m_bOutline;
    s_bSurfaces   = m_pglWingView->m_bSurfaces;
    s_bVLMPanels  = m_pglWingView->m_bVLMPanels;
    s_bAxes       = m_pglWingView->m_bAxes;
    s_bShowMasses = m_pglWingView->m_bShowMasses;
    s_bFoilNames  = m_pglWingView->m_bFoilNames;

    done(QDialog::Accepted);
}


void WingDlg::reject()
{
    if(m_bChanged)
    {
        QString strong = tr("Save the changes ?");
        int Ans = QMessageBox::question(this, tr("Question"), strong,
                                        QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (QMessageBox::Yes == Ans)
        {
            onOK();
            return;
        }
        else if(QMessageBox::Cancel == Ans) return;
    }

    s_bOutline    = m_pglWingView->m_bOutline;
    s_bSurfaces   = m_pglWingView->m_bSurfaces;
    s_bVLMPanels  = m_pglWingView->m_bVLMPanels;
    s_bAxes       = m_pglWingView->m_bAxes;
    s_bShowMasses = m_pglWingView->m_bShowMasses;
    s_bFoilNames  = m_pglWingView->m_bFoilNames;

    done(QDialog::Rejected);
}


void WingDlg::setCurrentSection(int section)
{
    m_iSection = section;
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection())
    {
        m_ppbInsertAfter->setEnabled(false);
        m_ppbInsertBefore->setEnabled(false);
        m_ppbDeleteSection->setEnabled(false);
    }
    else
    {
        m_ppbInsertAfter->setEnabled(true);
        m_ppbInsertBefore->setEnabled(true);
        m_ppbDeleteSection->setEnabled(true);

        QString str;
        str = tr("Insert after section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_ppbInsertAfter->setText(str);

        str = tr("Insert before section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_ppbInsertBefore->setText(str);

        str = tr("Delete section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_ppbDeleteSection->setText(str);
    }
    m_pglWingView->resetglHighlight();
}


void WingDlg::setWingData()
{
    if(!m_pWing) return;
    //Updates the wing's properties after a change of geometry

    QString str;

    str = QString("%1").arg(m_pWing->m_PlanformArea*Units::m2toUnit(),7,'f',2);
    m_plabWingArea->setText(str);

    str = QString("%1").arg(m_pWing->m_PlanformSpan*Units::mtoUnit(),5,'f',2);
    m_plabWingSpan->setText(str);

    str = QString("%1").arg(m_pWing->m_ProjectedArea*Units::m2toUnit(),7,'f',2);
    m_plabProjectedArea->setText(str);

    str = QString("%1").arg(m_pWing->m_ProjectedSpan*Units::mtoUnit(),5,'f',2);
    m_plabProjectedSpan->setText(str);

    str = QString("%1").arg(m_pWing->m_GChord*Units::mtoUnit(),5,'f',2);
    m_plabGeomChord->setText(str);

    str = QString("%1").arg(m_pWing->m_MAChord*Units::mtoUnit(),5,'f',2);
    m_plabMAC->setText(str);

    str = QString("%1").arg(m_pWing->m_AR,5,'f',2);
    m_plabAspectRatio->setText(str);

    if(m_pWing->tipChord()>0.0) str = QString("%1").arg(m_pWing->m_TR,0,'f',2);
    else                        str = tr("Undefined");
    m_plabTaperRatio->setText(str);

    str = QString("%1").arg(m_pWing->averageSweep(),5,'f',2);
    m_plabSweep->setText(str);

    str = QString("%1").arg(m_pWing->VLMPanelTotal(true));
    m_plabVLMPanels->setText(str);

    str = QString("%1").arg(m_pWing->VLMPanelTotal(false));
    m_plab3DPanels->setText(str);
}


void WingDlg::setupLayout()
{
    m_pglWingView = new gl3dWingView(this);
    m_pglWingView->m_bOutline    = s_bOutline;
    m_pglWingView->m_bSurfaces   = s_bSurfaces;
    m_pglWingView->m_bVLMPanels  = s_bVLMPanels;
    m_pglWingView->m_bAxes       = s_bAxes;
    m_pglWingView->m_bShowMasses = s_bShowMasses;
    m_pglWingView->m_bFoilNames  = s_bFoilNames;

    /*_____________Start Top Layout Here____________*/

    m_pspLeftSide = new QSplitter(Qt::Vertical, this);
    {
        QFrame *pDataFrame = new QFrame;
        {
            QVBoxLayout *pDataLayout = new QVBoxLayout;
            {
                QFrame *pNameWidget = new QFrame(this);
                {
                    QHBoxLayout *pNameLayout = new QHBoxLayout;
                    {
                        m_pleWingName   = new QLineEdit(tr("WingName"));
                        pNameLayout->addWidget(m_pleWingName);
                        m_pcmbWingColor = new ColorMenuBtn;
                        pNameLayout->addWidget(m_pcmbWingColor);
                    }
                    pNameWidget->setLayout(pNameLayout);
                }

                QFrame *pSymWidget = new QFrame(this);
                {
                    QHBoxLayout *pSymLayout = new QHBoxLayout;
                    {
                        m_pchSymetric     = new QCheckBox(tr("Symetric"));
                        m_prbRightSide    = new QRadioButton(tr("Right Side"));
                        m_prbLeftSide     = new QRadioButton(tr("Left Side"));
                        m_ppbInsertBefore   = new QPushButton("Insert Before");
                        m_ppbInsertAfter    = new QPushButton("Insert After");
                        m_ppbDeleteSection  = new QPushButton("Delete Section");

                        pSymLayout->addWidget(m_pchSymetric);
                        pSymLayout->addStretch();
                        pSymLayout->addWidget(m_prbRightSide);
                        pSymLayout->addWidget(m_prbLeftSide);
                        pSymLayout->addStretch();
                        pSymLayout->addWidget(m_ppbInsertBefore);
                        pSymLayout->addWidget(m_ppbInsertAfter);
                        pSymLayout->addWidget(m_ppbDeleteSection);
                    }
                    pSymWidget->setLayout(pSymLayout);
                }

                m_ptvWingSections = new QTableView(this);
                m_ptvWingSections->setWindowTitle(QObject::tr("Wing definition"));
                m_ptvWingSections->setWordWrap(false);
                m_ptvWingSections->setSelectionMode(QAbstractItemView::SingleSelection);
                m_ptvWingSections->setSelectionBehavior(QAbstractItemView::SelectRows);
                m_ptvWingSections->setEditTriggers(QAbstractItemView::CurrentChanged  |
                                                   QAbstractItemView::DoubleClicked   |
                                                   QAbstractItemView::SelectedClicked |
                                                   QAbstractItemView::EditKeyPressed  |
                                                   QAbstractItemView::AnyKeyPressed);
                m_ptvWingSections->horizontalHeader()->setStretchLastSection(true);

                m_pWingModel = new QStandardItemModel;
                m_pWingModel->setRowCount(30);//temporary
                m_pWingModel->setColumnCount(10);

                m_pWingModel->setHeaderData(0, Qt::Horizontal, tr("y (")+")");
                m_pWingModel->setHeaderData(1, Qt::Horizontal, tr("chord (")+")");
                m_pWingModel->setHeaderData(2, Qt::Horizontal, tr("offset (")+")");
                m_pWingModel->setHeaderData(3, Qt::Horizontal, QObject::tr("dihedral")+QString::fromUtf8("(°)"));
                m_pWingModel->setHeaderData(4, Qt::Horizontal, QObject::tr("twist")+QString::fromUtf8("(°)"));
                m_pWingModel->setHeaderData(5, Qt::Horizontal, QObject::tr("foil"));
                m_pWingModel->setHeaderData(6, Qt::Horizontal, QObject::tr("X-panels"));
                m_pWingModel->setHeaderData(7, Qt::Horizontal, QObject::tr("X-dist"));
                m_pWingModel->setHeaderData(8, Qt::Horizontal, QObject::tr("Y-panels"));
                m_pWingModel->setHeaderData(9, Qt::Horizontal, QObject::tr("Y-dist"));

                m_ptvWingSections->setModel(m_pWingModel);


                QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pWingModel);
                m_ptvWingSections->setSelectionModel(selectionModel);
                connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


                m_pWingDelegate = new WingDelegate(this);
                m_ptvWingSections->setItemDelegate(m_pWingDelegate);
                connect(m_pWingDelegate,  SIGNAL(closeEditor(QWidget*)), this, SLOT(onCellChanged(QWidget*)));

                QVector<int> m_precision = {3,3,3,1,2,1,0,0,0,0};
                m_pWingDelegate->setPrecision(m_precision);

                pDataLayout->addWidget(pNameWidget);
                pDataLayout->addWidget(pSymWidget);
                pDataLayout->addWidget(m_ptvWingSections);
            }
            pDataFrame->setLayout(pDataLayout);
        }

        m_pspLeftSide->addWidget(pDataFrame);
        m_pspLeftSide->addWidget(m_pglWingView);
        m_pspLeftSide->setStretchFactor(0,1);
        m_pspLeftSide->setStretchFactor(1,1);
        m_pspLeftSide->setStretchFactor(2,9);
        m_pspLeftSide->setStretchFactor(3,9);
    }

    QFrame *pDataWidget = new QFrame(this);
    {
        QGridLayout *pDataLayout = new QGridLayout;
        {
            QLabel *m_plabLength1    = new QLabel(Units::lengthUnitLabel(), this);
            QLabel *m_plabLength2    = new QLabel(Units::lengthUnitLabel(), this);
            QLabel *m_plabLength3    = new QLabel(Units::lengthUnitLabel(), this);
            QLabel *m_plabLength4    = new QLabel(Units::lengthUnitLabel(), this);
            QLabel *m_plabAreaUnit1  = new QLabel(Units::areaUnitLabel(),   this);
            QLabel *m_plabAreaUnit2  = new QLabel(Units::areaUnitLabel(),   this);
            m_plabLength1->setAlignment(  Qt::AlignLeft);
            m_plabLength2->setAlignment(  Qt::AlignLeft);
            m_plabLength3->setAlignment(  Qt::AlignLeft);
            m_plabLength4->setAlignment(  Qt::AlignLeft);
            m_plabAreaUnit1->setAlignment(Qt::AlignLeft);
            m_plabAreaUnit2->setAlignment(Qt::AlignLeft);

            QLabel *lab1 = new QLabel(tr("Wing Span"));
            QLabel *lab2 = new QLabel(tr("Area"));
            QLabel *lab3 = new QLabel(tr("Projected Span"));
            QLabel *lab4 = new QLabel(tr("Projected Area"));
            QLabel *lab13 = new QLabel(tr("Number of VLM Panels"));
            QLabel *lab14 = new QLabel(tr("Number of 3D Panels"));
            lab1->setAlignment(Qt::AlignRight);
            lab2->setAlignment(Qt::AlignRight);
            lab3->setAlignment(Qt::AlignRight);
            lab4->setAlignment(Qt::AlignRight);
            lab13->setAlignment(Qt::AlignRight);
            lab14->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(lab1,1,1);
            pDataLayout->addWidget(lab2,2,1);
            pDataLayout->addWidget(lab3,3,1);
            pDataLayout->addWidget(lab4,4,1);
            pDataLayout->addWidget(lab13,13,1);
            pDataLayout->addWidget(lab14,14,1);
            m_plabWingSpan      = new QLabel("2000.00");
            m_plabWingArea      = new QLabel("30.0");
            m_plabProjectedArea = new QLabel("25.0");
            m_plabProjectedSpan = new QLabel("1900.0");;
            m_plabVLMPanels     = new QLabel("500");
            m_plab3DPanels      = new QLabel("1000");
            m_plabWingSpan->setAlignment(Qt::AlignRight);
            m_plabWingArea->setAlignment(Qt::AlignRight);
            m_plabProjectedSpan->setAlignment(Qt::AlignRight);
            m_plabProjectedArea->setAlignment(Qt::AlignRight);
            m_plabVLMPanels->setAlignment(Qt::AlignRight);
            m_plab3DPanels->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(m_plabWingSpan,   1,2);
            pDataLayout->addWidget(m_plabWingArea,   2,2);
            pDataLayout->addWidget(m_plabProjectedSpan,   3,2);
            pDataLayout->addWidget(m_plabProjectedArea,   4,2);
            pDataLayout->addWidget(m_plabVLMPanels, 13,2);
            pDataLayout->addWidget(m_plab3DPanels,  14,2);

            pDataLayout->addWidget(m_plabLength1,1,3);
            pDataLayout->addWidget(m_plabAreaUnit1,2,3);
            pDataLayout->addWidget(m_plabLength2,3,3);
            pDataLayout->addWidget(m_plabAreaUnit2,4,3);

            QLabel *lab20 = new QLabel(tr("Mean Geom. Chord"), this);
            QLabel *lab21 = new QLabel(tr("Mean Aero Chord"), this);
            QLabel *lab23 = new QLabel(tr("Aspect ratio"), this);
            QLabel *lab24 = new QLabel(tr("Taper Ratio"), this);
            QLabel *lab25 = new QLabel(tr("Root to Tip Sweep"), this);
            QLabel *lab26 = new QLabel(tr("Number of Flaps"), this);
            lab20->setAlignment(Qt::AlignRight);
            lab21->setAlignment(Qt::AlignRight);
            lab23->setAlignment(Qt::AlignRight);
            lab24->setAlignment(Qt::AlignRight);
            lab25->setAlignment(Qt::AlignRight);
            lab26->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(lab20,6,1);
            pDataLayout->addWidget(lab21,7,1);
            pDataLayout->addWidget(lab23,9,1);
            pDataLayout->addWidget(lab24,10,1);
            pDataLayout->addWidget(lab25,11,1);
            pDataLayout->addWidget(lab26,12,1);

            m_plabGeomChord    = new QLabel("170.0", this);
            m_plabMAC          = new QLabel("150.0", this);
            m_plabAspectRatio  = new QLabel("13.33", this);
            m_plabTaperRatio   = new QLabel("1.50", this);
            m_plabSweep        = new QLabel("2.58", this);
            m_plabNFlaps       = new QLabel("0", this);
            m_plabMAC->setAlignment(Qt::AlignRight);
            m_plabGeomChord->setAlignment(Qt::AlignRight);
            m_plabAspectRatio->setAlignment(Qt::AlignRight);
            m_plabTaperRatio->setAlignment(Qt::AlignRight);
            m_plabSweep->setAlignment(Qt::AlignRight);
            m_plabNFlaps->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(m_plabGeomChord,    6,2);
            pDataLayout->addWidget(m_plabMAC,          7,2);
            pDataLayout->addWidget(m_plabAspectRatio,  9,2);
            pDataLayout->addWidget(m_plabTaperRatio,  10,2);
            pDataLayout->addWidget(m_plabSweep,       11,2);
            pDataLayout->addWidget(m_plabNFlaps,      12,2);
            pDataLayout->addWidget(m_plabLength3, 6, 3);
            pDataLayout->addWidget(m_plabLength4, 7, 3);
            QLabel *lab30 = new QLabel(QChar(0260));
            lab30->setAlignment(Qt::AlignLeft);
            pDataLayout->addWidget(lab30, 11, 3);

            pDataLayout->setRowStretch(15,1);
            pDataLayout->setColumnStretch(3,1);
        }
        pDataWidget->setLayout(pDataLayout);
    }


    /*_____________End Top Right Layout Here______________*/

    m_pteWingDescription = new QTextEdit();
    m_pteWingDescription->setToolTip(tr("Enter here a short description for the wing"));

    QLabel *WingDescription = new QLabel(tr("Description:"));

    /*_____________Start Bottom Right Layout Here_________*/
    QFrame *pRightSideWidget = new QFrame(this);
    {
        QVBoxLayout *pRightSideLayout = new QVBoxLayout(this);
        {
            QGridLayout *p3dParamsLayout = new QGridLayout;
            {
                m_pchAxes       = new QCheckBox(tr("Axes"), this);
                m_pchSurfaces   = new QCheckBox(tr("Surfaces"), this);
                m_pchOutline    = new QCheckBox(tr("Outline"), this);
                m_pchPanels     = new QCheckBox(tr("Panels"), this);
                m_pchFoilNames  = new QCheckBox(tr("Foil Names"), this);
                m_pchShowMasses = new QCheckBox(tr("Masses"), this);

                p3dParamsLayout->addWidget(m_pchAxes, 1,1);
                p3dParamsLayout->addWidget(m_pchPanels, 1,2);
                p3dParamsLayout->addWidget(m_pchSurfaces, 2,1);
                p3dParamsLayout->addWidget(m_pchOutline, 2,2);
                p3dParamsLayout->addWidget(m_pchFoilNames, 3,1);
                p3dParamsLayout->addWidget(m_pchShowMasses, 3,2);
            }

            QVBoxLayout *pThreeDViewLayout = new QVBoxLayout;
            {
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

                pThreeDViewLayout->addStretch();
                pThreeDViewLayout->addLayout(pAxisViewLayout);
                pThreeDViewLayout->addWidget(m_ppbReset);
            }


            QHBoxLayout *pWingModCommands = new QHBoxLayout;
            {
                m_ppbResetMesh    = new QPushButton(tr("Reset Mesh"));


                m_pScaleWing     = new QAction(tr("Scale Wing"), this);
                m_pInertia       = new QAction(tr("Inertia..."), this);
                m_pImportWingAct = new QAction(tr("Import Wing (deprecated, use XML)"), this);
                m_pExportWingAct = new QAction(tr("Export Wing (deprecated, use XML)"), this);
                m_pImportWingXml = new QAction(tr("Import Wing from xml file..."), this);
                m_pExportWingXml = new QAction(tr("Export Wing to xml file..."), this);

                QPushButton *pMenuButton = new QPushButton(tr("Other"));
                QMenu *pWingMenu = new QMenu(tr("Actions"), this);
                pWingMenu->addAction(m_pExportWingXml);
                pWingMenu->addAction(m_pImportWingXml);
                pWingMenu->addSeparator();
                pWingMenu->addAction(m_pExportWingAct);
                pWingMenu->addAction(m_pImportWingAct);
                pWingMenu->addSeparator();
                pWingMenu->addAction(m_pInertia);
                pWingMenu->addAction(m_pScaleWing);
                pMenuButton->setMenu(pWingMenu);

                pWingModCommands->addWidget(m_ppbResetMesh);
                pWingModCommands->addWidget(pMenuButton);
            }

            m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
            {
                connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
            }

            QFrame *pAll3DControlsWidget = new QFrame(this);
            {
                QVBoxLayout *pAll3DControlsLayout = new QVBoxLayout;
                {
                    pAll3DControlsLayout->addLayout(p3dParamsLayout);
                    pAll3DControlsLayout->addLayout(pThreeDViewLayout);
                    pAll3DControlsLayout->addLayout(pWingModCommands);
                    pAll3DControlsLayout->addStretch();
                    pAll3DControlsLayout->addWidget(m_pButtonBox);
                }
                pAll3DControlsWidget->setLayout(pAll3DControlsLayout);
            }

            pRightSideLayout->addWidget(WingDescription);
            pRightSideLayout->addWidget(m_pteWingDescription);
            pRightSideLayout->addWidget(pDataWidget);
            pRightSideLayout->addWidget(pAll3DControlsWidget);

            pRightSideLayout->setStretchFactor(pDataWidget, 1);

            pRightSideWidget->setLayout(pRightSideLayout);
        }
    }

    m_pspHorizontal = new QSplitter(Qt::Horizontal, this);
    {
        m_pspHorizontal->addWidget(m_pspLeftSide);
        m_pspHorizontal->addWidget(pRightSideWidget);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(m_pspHorizontal);

    setLayout(pMainLayout);
}


void WingDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_WindowGeometry);
    if(s_HSplitterSizes.length()>0)
        m_pspHorizontal->restoreState(s_HSplitterSizes);
    if(s_LeftSplitterSizes.length()>0)
        m_pspLeftSide->restoreState(s_LeftSplitterSizes);

    m_bChanged = false;
    m_pglWingView->resetglWing();

    m_pglWingView->update();

    resizeEvent(nullptr);
}


void WingDlg::hideEvent(QHideEvent *)
{
    s_HSplitterSizes  = m_pspHorizontal->saveState();
    s_LeftSplitterSizes  = m_pspLeftSide->saveState();

    s_WindowGeometry = saveGeometry();
}


void WingDlg::resizeEvent(QResizeEvent *)
{
    int w = m_ptvWingSections->width();
    w = int(double(w) *93.0/100.0);
    int wFoil  = w/5;
    int wCols  = w/11;

    m_ptvWingSections->setColumnWidth(0, wCols);
    m_ptvWingSections->setColumnWidth(1, wCols);
    m_ptvWingSections->setColumnWidth(2, wCols);
    m_ptvWingSections->setColumnWidth(3, wCols);
    m_ptvWingSections->setColumnWidth(4, wCols);
    m_ptvWingSections->setColumnWidth(5, wFoil);
    m_ptvWingSections->setColumnWidth(6, wCols);
    m_ptvWingSections->setColumnWidth(7, wCols);
    m_ptvWingSections->setColumnWidth(8, wCols);
    m_ptvWingSections->setColumnWidth(9, wCols);

    if(m_pWing)    m_pglWingView->setReferenceLength(m_pWing->planformSpan());
    m_pglWingView->update();
}



int WingDlg::VLMGetPanelTotal()
{
    double MinPanelSize;
    if(Wing::s_MinPanelSize>0.0) MinPanelSize = Wing::s_MinPanelSize;
    else                         MinPanelSize = m_pWing->m_PlanformSpan/1000.0;

    int total = 0;
    for (int i=0; i<m_pWing->NWingSection()-1; i++)
    {
        //do not create a surface if its length is less than the critical size
        //            if(qAbs(m_pWing->TPos[j]-m_pWing->TPos(j+1))/m_pWing->m_Span >0.001){
        if (qAbs(m_pWing->YPosition(i)-m_pWing->YPosition(i+1)) > MinPanelSize)
            total +=m_pWing->NXPanels(i)*m_pWing->NYPanels(i);
    }
    //    if(!m_bMiddle) total *=2;
    if(!m_pWing->m_bIsFin) return total*2;
    else                   return total;
}


bool WingDlg::VLMSetAutoMesh(int total)
{
    m_bChanged = true;
    //split (NYTotal) panels on each side proportionnaly to length, and space evenly
    //Set VLMMATSIZE/NYTotal panels along chord
    int NYTotal, size;

    if(!total)
    {
        size = 2000/4;//why not ? Too much refinement isn't worthwile
        NYTotal = 22;
    }
    else
    {
        size = total;
        NYTotal = int(sqrt(float(size)));
    }

    NYTotal *= 2;

    //    double d1, d2; //spanwise panel densities at i and i+1

    for (int i=0; i<m_pWing->NWingSection()-1;i++)
    {
        //        d1 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos[i],  3) + 0.5;
        //        d2 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos(i+1),3) + 0.5;
        //        m_pWing->NYPanels(i) = (int) (NYTotal * (0.8*d1+0.2*d2)* (m_pWing->TPos(i+1)-m_pWing->TPos(i))/m_pWing->m_Span);

        m_pWing->setNYPanels(i, int(qAbs(m_pWing->YPosition(i+1) - m_pWing->YPosition(i))* double(NYTotal)/m_pWing->m_PlanformSpan));

        m_pWing->setNXPanels(i, int(size/NYTotal));

        if(m_pWing->NYPanels(i)<2) m_pWing->setNYPanels(i, 2);
        if(m_pWing->NXPanels(i)<2) m_pWing->setNXPanels(i, 2);
    }

    return true;
}


void WingDlg::onImportWingFromXML()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(nullptr,
                                                QString("Open XML File"),
                                                xfl::s_LastDirName,
                                                tr("Plane XML file")+"(*.xml)");
    QFileInfo fileinfo(path_to_file);
    xfl::s_LastDirName = fileinfo.path();

    QFile xmlfile(path_to_file);

    if (!xmlfile.open(QIODevice::ReadOnly))
    {
        QString strange = tr("Could not read the file\n")+xmlfile.fileName();
        QMessageBox::warning(this, tr("Warning"), strange);
        return;
    }

    Plane plane;
    XMLPlaneReader planereader(xmlfile, &plane);
    planereader.readXMLPlaneFile();
    if(planereader.hasError())
    {
        QString strong;
        QString errorMsg;
        errorMsg = "Failed to read the file "+path_to_file+"\n";
        strong = QString::asprintf("error on line %d column %d", int(planereader.lineNumber()), int(planereader.columnNumber()));
        errorMsg += planereader.errorString() + " at " + strong;
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        return;
    }

    m_pWing->duplicate(plane.wing());
    initDialog(m_pWing);
    readParams();
    setWingData();
    m_bChanged = true;

    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onExportWingToXML()
{
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pWing->name().trimmed();
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(this, tr("Export to xml file"),
                                            xfl::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    QFileInfo fileInfo(FileName);
    xfl::s_LastDirName = fileInfo.path();

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XMLPlaneWriter planeWriter(XFile);
    planeWriter.writeXMLWing(*m_pWing);

    XFile.close();
}


void WingDlg::onImportWing()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(nullptr,
                                                QString("Open File"),
                                                xfl::s_LastDirName,
                                                QString("XFLR5 Wing file (*.xwimp)"));
    QString errorMsg;
    if(!m_pWing->importDefinition(path_to_file, errorMsg))
    {
        QMessageBox::warning(this, tr("Warning"), errorMsg);
    }
    initDialog(m_pWing);
    readParams();
    setWingData();
    m_bChanged = true;
    m_pglWingView->resetglWing();
    m_pglWingView->update();
}


void WingDlg::onExportWing()
{
    QString path_to_file;
    path_to_file = QFileDialog::getSaveFileName(nullptr,
                                                QString("Save File"),
                                                xfl::s_LastDirName,
                                                QString("XFLR5 Wing file (*.xwimp)"));
    if (!path_to_file.endsWith(".xwimp")) {
        path_to_file.append(".xwimp");
    }
    QString errorMsg;
    if(!m_pWing->exportDefinition(path_to_file, errorMsg))
    {
        QMessageBox::warning(this, tr("Warning"), errorMsg);
    }
}



bool WingDlg::intersectObject(Vector3d AA,  Vector3d U, Vector3d &I)
{
    double dist=0.0;

    for(int j=0; j<m_pWing->m_Surface.size(); j++)
    {
        if(xfl::intersect(m_pWing->surface(j)->m_LA,
                          m_pWing->surface(j)->m_LB,
                          m_pWing->surface(j)->m_TA,
                          m_pWing->surface(j)->m_TB,
                          m_pWing->surface(j)->m_Normal,
                          AA, U, I, dist))
            return true;
    }
    return false;
}


void WingDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("GL3dWingDlg");
    {
        s_WindowGeometry = settings.value("Geometry").toByteArray();
        s_HSplitterSizes = settings.value("HorizontalSplitterSizes").toByteArray();
        s_LeftSplitterSizes = settings.value("LeftSideSplitterSizes").toByteArray();
    }
    settings.endGroup();
}


void WingDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("GL3dWingDlg");
    {
        settings.setValue("Geometry", s_WindowGeometry);
        settings.setValue("HorizontalSplitterSizes", s_HSplitterSizes);
        settings.setValue("LeftSideSplitterSizes", s_LeftSplitterSizes);
    }
    settings.endGroup();
}









