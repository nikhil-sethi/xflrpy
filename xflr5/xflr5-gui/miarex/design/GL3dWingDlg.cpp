/****************************************************************************

    GL3dWingDlg Class
    Copyright (C) 2009-2019 Andre Deperrois

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
#include <math.h>

#include <globals/globals.h>
#include <miarex/design/GL3dWingDlg.h>
#include <miarex/design/GL3dWingDlg.h>
#include <miarex/design/InertiaDlg.h>
#include <miarex/design/InertiaDlg.h>
#include <miarex/design/WingDelegate.h>
#include <miarex/design/WingScaleDlg.h>
#include <miarex/design/WingScaleDlg.h>
#include <miarex/mgt/XmlPlaneReader.h>
#include <miarex/mgt/XmlPlaneWriter.h>
#include <miarex/objects3d.h>
#include <miarex/view/W3dPrefsDlg.h>
#include <misc/color/ColorButton.h>
#include <misc/options/units.h>
#include <misc/options/displayoptions.h>
#include <misc/text/DoubleEdit.h>
#include <objects/objects3d/Plane.h>
#include <objects/objects3d/Surface.h>
#include <objects/objects3d/Wing.h>
#include <objects/objects_global.h>
#include <viewwidgets/glWidgets/gl3dwingview.h>
#include <xdirect/objects2d.h>




QList <Foil*> *GL3dWingDlg::s_poaFoil;

QPoint GL3dWingDlg::s_WindowPos=QPoint(75,55);
QSize  GL3dWingDlg::s_WindowSize=QSize(1200, 900);
bool GL3dWingDlg::s_bWindowMaximized=false;

bool GL3dWingDlg::s_bOutline    = true;
bool GL3dWingDlg::s_bSurfaces   = true;
bool GL3dWingDlg::s_bVLMPanels  = false;
bool GL3dWingDlg::s_bAxes       = true;
bool GL3dWingDlg::s_bShowMasses = false;
bool GL3dWingDlg::s_bFoilNames  = false;

GL3dWingDlg::GL3dWingDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Wing Edition"));
    setWindowFlags(Qt::Window);

    m_pWing = nullptr;

    m_precision = nullptr;

    m_iSection   = -1;

    m_pLeftSideSplitter = nullptr;

    m_bResetglSectionHighlight = true;
    m_bResetglWing             = true;
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



GL3dWingDlg::~GL3dWingDlg()
{
    if(m_pWingModel)    delete m_pWingModel;
    if(m_pWingDelegate) delete m_pWingDelegate;
    if(m_precision)     delete [] m_precision;
}



bool GL3dWingDlg::checkWing()
{
    if(!m_pWing->m_WingName.length())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Please enter a name for the wing"));
        m_pctrlWingName->setFocus();
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

    for (int k=1; k<m_pWing->NWingSection(); k++)
    {
        if(fabs(m_pWing->Chord(k))<0.0001)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Zero length chords will cause a division by zero and should be avoided."));
            return false;
        }
        WingSection *pSection = m_pWing->m_WingSection.at(k);
        Foil *pLeftFoil = Objects2d::foil(pSection->m_LeftFoilName);
        Foil *pRightFoil = Objects2d::foil(pSection->m_RightFoilName);
        if(pLeftFoil )
        {
            if((pLeftFoil->m_TEXHinge>=99&& pLeftFoil->m_bTEFlap) ||(pLeftFoil->m_LEXHinge<0.01&&pLeftFoil->m_bLEFlap))
            {
                QMessageBox::warning(this, tr("Warning"), pLeftFoil->foilName()+": "+tr("Zero length flaps will cause a division by zero and should be avoided."));
                return false;
            }
        }
        if(pRightFoil)
        {
            if((pRightFoil->m_TEXHinge>=99&& pRightFoil->m_bTEFlap) ||(pRightFoil->m_LEXHinge<0.01&&pRightFoil->m_bLEFlap))
            {
                QMessageBox::warning(this, tr("Warning"), pRightFoil->foilName()+": "+tr("Zero length flaps will cause a division by zero and should be avoided."));
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



void GL3dWingDlg::computeGeometry()
{
    // Computes the wing's characteristics from the panel data
    m_pWing->computeGeometry();
    m_pWing->createSurfaces(Vector3d(0.0,0.0,0.0), 0.0, 0.0);

    for (int j=0; j<m_pWing->m_Surface.size(); j++)
        m_pWing->m_Surface.at(j)->setSidePoints(NULL, 0.0, 0.0);
}



void GL3dWingDlg::contextMenuEvent(QContextMenuEvent *event)
{
    // Display the context menu
    if(m_pctrlWingTable->geometry().contains(event->pos())) m_pContextMenu->exec(event->globalPos());
}


void GL3dWingDlg::connectSignals()
{
    connect(m_pInsertBefore,  SIGNAL(triggered()), this, SLOT(onInsertBefore()));
    connect(m_pInsertAfter,   SIGNAL(triggered()), this, SLOT(onInsertAfter()));
    connect(m_pDeleteSection, SIGNAL(triggered()), this, SLOT(onDeleteSection()));
    connect(m_pResetSection,  SIGNAL(triggered()), this, SLOT(onResetSection()));

    connect(m_pResetScales, SIGNAL(triggered()), m_pglWingView, SLOT(on3DReset()));

    connect(m_pctrlIso,        SIGNAL(clicked()), m_pglWingView, SLOT(on3DIso()));
    connect(m_pctrlX,          SIGNAL(clicked()), m_pglWingView, SLOT(on3DFront()));
    connect(m_pctrlY,          SIGNAL(clicked()), m_pglWingView, SLOT(on3DLeft()));
    connect(m_pctrlZ,          SIGNAL(clicked()), m_pglWingView, SLOT(on3DTop()));
    connect(m_pctrlReset,      SIGNAL(clicked()), m_pglWingView, SLOT(on3DReset()));
    connect(m_pctrlFlip,       SIGNAL(clicked()), m_pglWingView, SLOT(on3DFlip()));


    connect(m_pctrlFoilNames,  SIGNAL(clicked()),this, SLOT(onFoilNames()));
    connect(m_pctrlShowMasses, SIGNAL(clicked()),this, SLOT(onShowMasses()));

    connect(m_pctrlAxes,       SIGNAL(clicked()), this, SLOT(onAxes()));
    connect(m_pctrlPanels,     SIGNAL(clicked()), this, SLOT(onPanels()));
    connect(m_pctrlSurfaces,   SIGNAL(clicked()), this, SLOT(onSurfaces()));
    connect(m_pctrlOutline,    SIGNAL(clicked()), this, SLOT(onOutline()));

    connect(m_pctrlInsertBefore,  SIGNAL(clicked()), this, SLOT(onInsertBefore()));
    connect(m_pctrlInsertAfter,   SIGNAL(clicked()), this, SLOT(onInsertAfter()));
    connect(m_pctrlDeleteSection, SIGNAL(clicked()), this, SLOT(onDeleteSection()));

    connect(m_pctrlResetMesh,     SIGNAL(clicked()), this, SLOT(onResetMesh()));
    connect(m_pctrlWingColor,     SIGNAL(clicked()), this, SLOT(onWingColor()));
    connect(m_pctrlTextures,      SIGNAL(clicked()), this, SLOT(onTextures()));
    connect(m_pctrlColor,         SIGNAL(clicked()), this, SLOT(onTextures()));
    connect(m_pctrlSymetric,      SIGNAL(clicked()), this, SLOT(onSymetric()));
    connect(m_pctrlRightSide,     SIGNAL(clicked()), this, SLOT(onSide()));
    connect(m_pctrlLeftSide,      SIGNAL(clicked()), this, SLOT(onSide()));

    connect(m_pctrlWingDescription, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));

    connect(m_pInertia,       SIGNAL(triggered()), this, SLOT(onInertia()));
    connect(m_pScaleWing,     SIGNAL(triggered()), this, SLOT(onScaleWing()));
    connect(m_pglWingView,    SIGNAL(viewModified()), this, SLOT(onCheckViewIcons()));
    connect(m_pImportWingAct, SIGNAL(triggered()),this, SLOT(onImportWing()));
    connect(m_pExportWingAct, SIGNAL(triggered()),this, SLOT(onExportWing()));
    connect(m_pImportWingXml, SIGNAL(triggered()),this, SLOT(onImportWingFromXML()));
    connect(m_pExportWingXml, SIGNAL(triggered()),this, SLOT(onExportWingToXML()));
}


/**
 * Unselects all the 3D-view icons.
 */
void GL3dWingDlg::onCheckViewIcons()
{
    m_pctrlIso->setChecked(false);
    m_pctrlX->setChecked(false);
    m_pctrlY->setChecked(false);
    m_pctrlZ->setChecked(false);
}



void GL3dWingDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)         onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void GL3dWingDlg::createXPoints(int NXPanels, int XDist, Foil *pFoilA, Foil *pFoilB, double *xPointA, double *xPointB, int &NXLead, int &NXFlap)
{
    // the chordwise panel distribution is set i.a.w. with the flap hinges;

    int l;
    int NXFlapA, NXFlapB, NXLeadA, NXLeadB;
    double dl, dl2;
    double xHingeA, xHingeB;
    if(pFoilA && pFoilA->m_bTEFlap) xHingeA=pFoilA->m_TEXHinge/100.0; else xHingeA=1.0;
    if(pFoilB && pFoilB->m_bTEFlap) xHingeB=pFoilB->m_TEXHinge/100.0; else xHingeB=1.0;

    NXFlapA = (int)((1.0-xHingeA) * NXPanels);
    NXFlapB = (int)((1.0-xHingeB) * NXPanels);
    if(pFoilA && pFoilA->m_bTEFlap && NXFlapA==0) NXFlapA++;
    if(pFoilB && pFoilB->m_bTEFlap && NXFlapB==0) NXFlapB++;
    NXLeadA = NXPanels - NXFlapA;
    NXLeadB = NXPanels - NXFlapB;

    NXFlap  = qMax(NXFlapA, NXFlapB);
    if(NXFlap>NXPanels/2) NXFlap=(int)NXPanels/2;
    NXLead  = NXPanels - NXFlap;

    for(l=0; l<NXFlapA; l++)
    {
        dl =  (double)l;
        dl2 = (double)NXFlapA;
        if(XDist==1)
            xPointA[l] = 1.0 - (1.0-xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointA[l] = 1.0 - (1.0-xHingeA) * (dl/dl2);
    }
    for(l=0; l<NXLeadA; l++)
    {
        dl =  (double)l;
        dl2 = (double)NXLeadA;
        if(XDist==1)
            xPointA[l+NXFlapA] = xHingeA - (xHingeA)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointA[l+NXFlapA] = xHingeA - (xHingeA) * (dl/dl2);
    }

    for(l=0; l<NXFlapB; l++)
    {
        dl =  (double)l;
        dl2 = (double)NXFlapB;
        if(XDist==1)
            xPointB[l] = 1.0 - (1.0-xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointB[l] = 1.0 - (1.0-xHingeB) * (dl/dl2);
    }
    for(l=0; l<NXLeadB; l++)
    {
        dl =  (double)l;
        dl2 = (double)NXLeadB;
        if(XDist==1)
            xPointB[l+NXFlapB] = xHingeB - (xHingeB)/2.0 * (1.0-cos(dl*PI /dl2));
        else
            xPointB[l+NXFlapB] = xHingeB - (xHingeB) * (dl/dl2);
    }

    xPointA[NXPanels] = 0.0;
    xPointB[NXPanels] = 0.0;
}


void GL3dWingDlg::fillDataTable()
{
    if(!m_pWing) return;
    int i;
    m_pWingModel->setRowCount(m_pWing->NWingSection());

    for(i=0; i<m_pWing->NWingSection(); i++)
    {
        fillTableRow(i);
    }
}



void GL3dWingDlg::fillTableRow(int row)
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
    if(m_bRightSide) m_pWingModel->setData(ind, m_pWing->rightFoil(row));
    else             m_pWingModel->setData(ind, m_pWing->leftFoil(row));

    if(row<m_pWing->NWingSection())
    {
        ind = m_pWingModel->index(row, 6, QModelIndex());
        m_pWingModel->setData(ind, m_pWing->NXPanels(row));

        if(m_pWing->XPanelDist(row)==XFLR5::UNIFORM)       strong = tr("Uniform");
        else if(m_pWing->XPanelDist(row)==XFLR5::COSINE)   strong = tr("Cosine");
        ind = m_pWingModel->index(row, 7, QModelIndex());
        m_pWingModel->setData(ind, strong);

        ind = m_pWingModel->index(row, 8, QModelIndex());
        m_pWingModel->setData(ind, m_pWing->NYPanels(row));

        if(m_pWing->YPanelDist(row)==XFLR5::UNIFORM)            strong = tr("Uniform");
        else if(m_pWing->YPanelDist(row)==XFLR5::COSINE)        strong = tr("Cosine");
        else if(m_pWing->YPanelDist(row)==XFLR5::SINE)          strong = tr("Sine");
        else if(m_pWing->YPanelDist(row)== XFLR5::INVERSESINE)  strong = tr("-Sine");
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





/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void GL3dWingDlg::glMake3DObjects()
{
    if(m_bResetglSectionHighlight || m_bResetglWing)
    {
        if(m_iSection>=0)
        {
            m_pglWingView->glMakeWingSectionHighlight(m_pWing, m_iSection, m_bRightSide);
            m_bResetglSectionHighlight = false;
        }
    }

    if(m_bResetglWing)
    {
        m_bResetglWing = false;

        m_pglWingView->glMakeWingGeometry(0, m_pWing, nullptr);
        m_pglWingView->glMakeWingEditMesh(m_pglWingView->m_vboEditWingMesh[0], m_pWing);
    }
}



bool GL3dWingDlg::initDialog(Wing *pWing)
{
    QString str;
    m_iSection = 0;

    Units::getAreaUnitLabel(str);
    m_pctrlAreaUnit1->setText(str);
    m_pctrlAreaUnit2->setText(str);

    Units::getLengthUnitLabel(str);

    m_pctrlLength1->setText(str);
    m_pctrlLength2->setText(str);
    m_pctrlLength3->setText(str);
    m_pctrlLength4->setText(str);

    m_pWing = pWing;
    if(!m_pWing) return false;
    m_pglWingView->setWing(m_pWing);
    computeGeometry();

    m_pctrlWingName->setText(m_pWing->m_WingName);
    if(m_pWing->m_WingDescription.length())
    {
        m_pctrlWingDescription->setPlainText(m_pWing->m_WingDescription);
    }
    else
    {
        m_pctrlWingDescription->setPlainText("");
    }

    if(!m_bAcceptName) m_pctrlWingName->setEnabled(false);
    m_pctrlSymetric->setChecked(m_pWing->m_bSymetric);
    m_pctrlRightSide->setChecked(m_pWing->m_bSymetric);
    m_pctrlLeftSide->setEnabled(!m_pWing->m_bSymetric);
    m_pctrlRightSide->setChecked(m_bRightSide);
    m_pctrlLeftSide->setChecked(!m_bRightSide);


    m_pctrlSurfaces->setChecked(m_pglWingView->m_bSurfaces);
    m_pctrlOutline->setChecked(m_pglWingView->m_bOutline);
    m_pctrlAxes->setChecked(m_pglWingView->m_bAxes);
    m_pctrlPanels->setChecked(m_pglWingView->m_bVLMPanels);
    m_pctrlFoilNames->setChecked(m_pglWingView->m_bFoilNames);
    m_pctrlShowMasses->setChecked(m_pglWingView->m_bShowMasses);

    m_pctrlColor->setChecked(!m_pWing->textures());
    m_pctrlTextures->setChecked(m_pWing->textures());
    m_pctrlWingColor->setColor(color(m_pWing->m_WingColor));
    m_pctrlWingColor->setEnabled(m_pctrlColor->isChecked());

    m_pctrlWingTable->setFont(Settings::s_TableFont);

    m_pWingModel = new QStandardItemModel;
    m_pWingModel->setRowCount(30);//temporary
    m_pWingModel->setColumnCount(10);

    m_pWingModel->setHeaderData(0, Qt::Horizontal, tr("y (")+str+")");
    m_pWingModel->setHeaderData(1, Qt::Horizontal, tr("chord (")+str+")");
    m_pWingModel->setHeaderData(2, Qt::Horizontal, tr("offset (")+str+")");
    m_pWingModel->setHeaderData(3, Qt::Horizontal, QObject::tr("dihedral")+QString::fromUtf8("(°)"));
    m_pWingModel->setHeaderData(4, Qt::Horizontal, QObject::tr("twist")+QString::fromUtf8("(°)"));
    m_pWingModel->setHeaderData(5, Qt::Horizontal, QObject::tr("foil"));
    m_pWingModel->setHeaderData(6, Qt::Horizontal, QObject::tr("X-panels"));
    m_pWingModel->setHeaderData(7, Qt::Horizontal, QObject::tr("X-dist"));
    m_pWingModel->setHeaderData(8, Qt::Horizontal, QObject::tr("Y-panels"));
    m_pWingModel->setHeaderData(9, Qt::Horizontal, QObject::tr("Y-dist"));

    m_pctrlWingTable->setModel(m_pWingModel);


    QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pWingModel);
    m_pctrlWingTable->setSelectionModel(selectionModel);
    connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));


    m_pWingDelegate = new WingDelegate(this);
    m_pctrlWingTable->setItemDelegate(m_pWingDelegate);
    connect(m_pWingDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));

    m_precision = new int[10];
    m_precision[0] = 3;
    m_precision[1] = 3;
    m_precision[2] = 3;
    m_precision[3] = 1;
    m_precision[4] = 2;
    m_precision[5] = 1;
    m_precision[6] = 0;
    m_precision[7] = 0;
    m_precision[8] = 0;
    m_precision[9] = 0;
    m_pWingDelegate->setPrecision(m_precision);
    m_pWingDelegate->m_pWingSection = &m_pWing->m_WingSection;
    m_pWingDelegate->m_poaFoil = s_poaFoil;
    fillDataTable();
    setWingData();
    m_pctrlWingTable->selectRow(m_iSection);
    setCurrentSection(m_iSection);
    return true;
}


void GL3dWingDlg::keyPressEvent(QKeyEvent *event)
{
    //	bool bShift = false;
    //	bool bCtrl  = false;
    //	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
    //	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (event->key())
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
            event->ignore();
    }
}




void GL3dWingDlg::onFoilNames()
{
    m_pglWingView->m_bFoilNames = m_pctrlFoilNames->isChecked();
    m_pglWingView->update();
}



void GL3dWingDlg::onShowMasses()
{
    m_pglWingView->m_bShowMasses = m_pctrlShowMasses->isChecked();
    m_pglWingView->update();
}


void GL3dWingDlg::onDescriptionChanged()
{
    m_bDescriptionChanged=true;
}


void GL3dWingDlg::onCellChanged(QWidget *)
{
    m_bChanged = true;
    m_bResetglWing = true;
    readParams();
    setWingData();
    m_pglWingView->update();
}



void GL3dWingDlg::onDeleteSection()
{
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection()) return;

    if(m_iSection==0)
    {
        QMessageBox::warning(this, tr("Warning"),tr("The first section cannot be deleted"));
        return;
    }
    m_pctrlWingTable->closePersistentEditor(m_pctrlWingTable->currentIndex());

    int ny, size;

    size = m_pWingModel->rowCount();
    if(size<=2) return;

    ny = m_pWing->NYPanels(m_iSection-1) + m_pWing->NYPanels(m_iSection);

    /*	for (k=m_iSection; k<size-1; k++)
    {
        m_pWing->TPos(k)      = m_pWing->TPos(k+1);
        m_pWing->TChord(k)    = m_pWing->TChord(k+1);
        m_pWing->TOffset(k)   = m_pWing->TOffset(k+1);
        m_pWing->TTwist(k)     = m_pWing->TTwist(k+1);
        m_pWing->TDihedral(k)  = m_pWing->TDihedral(k+1);
        m_pWing->NXPanels(k)   = m_pWing->NXPanels(k+1);
        m_pWing->NYPanels(k)   = m_pWing->NYPanels(k+1);
        m_pWing->XPanelDist(k) = m_pWing->XPanelDist(k+1);
        m_pWing->YPanelDist(k) = m_pWing->YPanelDist(k+1);
    }

    m_pWing->m_RightFoil.removeAt(m_iSection);
    m_pWing->m_LeftFoil.removeAt(m_iSection);*/

    m_pWing->removeWingSection(m_iSection);


    m_pWing->NYPanels(m_iSection-1) = ny;

    fillDataTable();
    computeGeometry();
    setWingData();
    m_bChanged = true;
    m_bResetglWing = true;
    m_pglWingView->update();
}




void GL3dWingDlg::onInertia()
{
    InertiaDlg dlg(this);
    dlg.m_pWing = m_pWing;

    //save inertia properties
    QList<PointMass*> PtMass;
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


void GL3dWingDlg::onInsertBefore()
{
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection()) return;

    if(m_iSection<=0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No insertion possible before the first section"));
        return;
    }

    int n = m_iSection;

    m_pWing->insertSection(m_iSection);

    m_pWing->YPosition(n) = (m_pWing->YPosition(n+1) + m_pWing->YPosition(n-1)) /2.0;
    m_pWing->Chord(n)     = (m_pWing->Chord(n+1)     + m_pWing->Chord(n-1))     /2.0;
    m_pWing->Offset(n)    = (m_pWing->Offset(n+1)    + m_pWing->Offset(n-1))    /2.0;
    m_pWing->Twist(n)     = (m_pWing->Twist(n+1)     + m_pWing->Twist(n-1))     /2.0;
    m_pWing->Dihedral(n)  = (m_pWing->Dihedral(n+1)  + m_pWing->Dihedral(n-1))  /2.0;

    m_pWing->XPanelDist(n) = m_pWing->XPanelDist(n-1);
    m_pWing->YPanelDist(n) = m_pWing->YPanelDist(n-1);

    m_pWing->rightFoil(n) = m_pWing->rightFoil(n-1);
    m_pWing->leftFoil(n)  = m_pWing->leftFoil(n-1);

    m_pWing->NXPanels(n)   = m_pWing->NXPanels(n-1);


    int ny = m_pWing->NYPanels(n-1);
    m_pWing->NYPanels(n)   = (int)(ny/2);
    m_pWing->NYPanels(n-1) = ny-m_pWing->NYPanels(n);
    if(m_pWing->NYPanels(n)==0)   m_pWing->NYPanels(n)++;
    if(m_pWing->NYPanels(n-1)==0) m_pWing->NYPanels(n-1)++;


    fillDataTable();
    m_pctrlWingTable->closePersistentEditor(m_pctrlWingTable->currentIndex());
    computeGeometry();
    setWingData();

    m_bChanged = true;
    m_bResetglSectionHighlight = true;
    m_bResetglWing = true;
    m_pglWingView->update();
}




void GL3dWingDlg::onInsertAfter()
{
    if(m_iSection <0 || m_iSection>=m_pWing->NWingSection()) return;

    int n = m_iSection;

    if(n<0) n=m_pWing->NWingSection();

    m_pWing->insertSection(m_iSection+1);

    if(n<m_pWing->NWingSection()-2)
    {
        m_pWing->YPosition(n+1)      = (m_pWing->YPosition(n)      + m_pWing->YPosition(n+2))     /2.0;
        m_pWing->Chord(n+1)    = (m_pWing->Chord(n)    + m_pWing->Chord(n+2))   /2.0;
        m_pWing->Offset(n+1)   = (m_pWing->Offset(n)   + m_pWing->Offset(n+2))  /2.0;
        m_pWing->Twist(n+1)    = (m_pWing->Twist(n)    + m_pWing->Twist(n+2))   /2.0;
    }
    else
    {
        m_pWing->YPosition(n+1)     = m_pWing->YPosition(n)*1.1;
        m_pWing->Chord(n+1)   = m_pWing->Chord(n)/1.1;
        m_pWing->Offset(n+1)  = m_pWing->Offset(n) + m_pWing->Chord(n) - m_pWing->Chord(n) ;
        m_pWing->Twist(n+1)     = m_pWing->Twist(n);
    }

    m_pWing->Dihedral(n+1)  = m_pWing->Dihedral(n);
    m_pWing->NXPanels(n+1)   = m_pWing->NXPanels(n);
    m_pWing->NYPanels(n+1)   = m_pWing->NYPanels(n);
    m_pWing->XPanelDist(n+1) = m_pWing->XPanelDist(n);
    m_pWing->YPanelDist(n+1) = m_pWing->YPanelDist(n);
    m_pWing->rightFoil(n+1)  = m_pWing->rightFoil(n);
    m_pWing->leftFoil(n+1)   = m_pWing->leftFoil(n);

    int ny = m_pWing->NYPanels(n);
    m_pWing->NYPanels(n+1) = qMax(1,(int)(ny/2));
    m_pWing->NYPanels(n)   = qMax(1,ny-m_pWing->NYPanels(n+1));

    //	m_pWing->m_bVLMAutoMesh = true;

    fillDataTable();
    m_pctrlWingTable->closePersistentEditor(m_pctrlWingTable->currentIndex());

    computeGeometry();
    setWingData();
    m_bChanged = true;
    m_bResetglWing = true;
    m_pglWingView->update();
}



void GL3dWingDlg::onResetSection()
{
    int n = m_iSection;

    if((0 < n) && (n < (m_pWing->NWingSection()-1)))
    {
        double ratio;
        ratio = (m_pWing->YPosition(n) - m_pWing->YPosition(n - 1)) / (m_pWing->YPosition(n + 1) - m_pWing->YPosition(n - 1));

        m_pWing->Chord   (n) = m_pWing->Chord   (n-1) + ratio * (m_pWing->Chord   (n+1) - m_pWing->Chord   (n-1));
        m_pWing->Offset  (n) = m_pWing->Offset  (n-1) + ratio * (m_pWing->Offset  (n+1) - m_pWing->Offset  (n-1));
        m_pWing->Twist   (n) = m_pWing->Twist   (n-1) + ratio * (m_pWing->Twist   (n+1) - m_pWing->Twist   (n-1));

        // same code here that in OnResetMesh
        fillDataTable();
        setWingData();
        computeGeometry();
        m_bChanged = true;
        m_bResetglWing = true;
        m_pglWingView->update();
    }
}


void GL3dWingDlg::onItemClicked(const QModelIndex &index)
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



void GL3dWingDlg::onOK()
{
    readParams();

    if(!checkWing()) return;

    if(m_pWing->m_bSymetric)
    {
        for (int i=0; i<m_pWing->NWingSection(); i++)
        {
            m_pWing->leftFoil(i)   = m_pWing->rightFoil(i);
        }
    }

    m_pWing->computeGeometry();
    m_pWing->computeBodyAxisInertia();

    accept();
}


void GL3dWingDlg::onAxes()
{
    m_pglWingView->m_bAxes = m_pctrlAxes->isChecked();
    m_pglWingView->update();
}


void GL3dWingDlg::onSurfaces()
{
    m_pglWingView->m_bSurfaces = m_pctrlSurfaces->isChecked();
    m_pglWingView->update();
}


void GL3dWingDlg::onOutline()
{
    m_pglWingView->m_bOutline = m_pctrlOutline->isChecked();
    m_pglWingView->update();
}


void GL3dWingDlg::onPanels()
{
    m_pglWingView->m_bVLMPanels = m_pctrlPanels->isChecked();
    m_pglWingView->update();
}




void GL3dWingDlg::onResetMesh()
{
    VLMSetAutoMesh();
    fillDataTable();
    setWingData();
    computeGeometry();
    m_bChanged = true;
    m_bResetglWing = true;
    m_pglWingView->update();
}



void GL3dWingDlg::onScaleWing()
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
        m_bResetglWing = true;
        m_bResetglSectionHighlight = true;
        computeGeometry();
        m_pglWingView->update();
    }
}


void GL3dWingDlg::onSide()
{
    m_bRightSide = m_pctrlRightSide->isChecked();
    fillDataTable();

    m_bChanged = true;
    m_bResetglSectionHighlight = true;
    m_pglWingView->update();
}


void GL3dWingDlg::onSymetric()
{
    if(m_pctrlSymetric->isChecked())
    {
        m_pWing->m_bSymetric  = true;
        m_bRightSide          = true;
        m_pctrlLeftSide->setEnabled(false);
        m_pctrlRightSide->setChecked(true);
        for(int i=0; i<m_pWing->NWingSection(); i++)
        {
            m_pWing->leftFoil(i) = m_pWing->rightFoil(i);
        }
    }
    else
    {
        m_pWing->m_bSymetric    = false;
        m_pctrlLeftSide->setEnabled(true);
    }

    m_bChanged = true;
    computeGeometry();
    m_bResetglWing             = true;
    m_bResetglSectionHighlight = true;
    m_pglWingView->update();
}


void GL3dWingDlg::onTextures()
{	
    if(m_pWing) m_pWing->m_bTextures = m_pctrlTextures->isChecked();
    m_bDescriptionChanged = true;
    m_pctrlWingColor->setEnabled(m_pctrlColor->isChecked());
    m_bResetglWing = true;
    m_pglWingView->update();
}


void GL3dWingDlg::onWingColor()
{
    if(!m_pWing) return;

    QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
    dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
    QColor clr = QColorDialog::getColor(color(m_pWing->wingColor()),
                                        this, "Color selection", dialogOptions);
    if(clr.isValid())
    {
        m_pWing->setWingColor(ObjectColor(clr.red(), clr.green(), clr.blue(), clr.alpha()));
        m_bDescriptionChanged = true;
    }

    m_pctrlWingColor->setColor(color(m_pWing->wingColor()));
    m_bResetglWing = true;
    m_pglWingView->update();
}



void GL3dWingDlg::readParams()
{
    m_pWing->m_WingName = m_pctrlWingName->text();
    QString strange = m_pctrlWingDescription->toPlainText();
    if(strange == tr("Wing Description")) strange="";
    m_pWing->m_WingDescription = strange;

    for (int i=0; i< m_pWingModel->rowCount();  i++)
    {
        readSectionData(i);
    }

    //Update Geometry
    computeGeometry();
}



void GL3dWingDlg::readSectionData(int sel)
{
    if(sel>=m_pWingModel->rowCount()) return;
    double d;

    bool bOK;
    QString strong;
    QStandardItem *pItem;

    pItem = m_pWingModel->item(sel,0);

    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->YPosition(sel) =d / Units::mtoUnit();

    pItem = m_pWingModel->item(sel,1);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->Chord(sel) =d / Units::mtoUnit();

    pItem = m_pWingModel->item(sel,2);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->Offset(sel) =d / Units::mtoUnit();

    pItem = m_pWingModel->item(sel,3);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->Dihedral(sel) =d;

    pItem = m_pWingModel->item(sel,4);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->Twist(sel) =d;

    pItem = m_pWingModel->item(sel,5);
    strong =pItem->text();

    if(m_pWing->m_bSymetric)
    {
        m_pWing->rightFoil(sel) = strong;
        m_pWing->leftFoil(sel)  = strong;
    }
    else
    {
        if(m_bRightSide)	m_pWing->rightFoil(sel) = strong;
        else                m_pWing->leftFoil(sel)  = strong;
    }

    pItem = m_pWingModel->item(sel,6);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->NXPanels(sel) = (int)qMax(1.0,d);

    pItem = m_pWingModel->item(sel,7);
    strong =pItem->text();
    strong.replace(" ","");
    if(strong==tr("Uniform"))		m_pWing->XPanelDist(sel) = XFLR5::UNIFORM;
    else if(strong==tr("Cosine"))	m_pWing->XPanelDist(sel) = XFLR5::COSINE;
    else if(strong==tr("Sine"))		m_pWing->XPanelDist(sel) = XFLR5::SINE;
    else if(strong==tr("-Sine"))	m_pWing->XPanelDist(sel) = XFLR5::INVERSESINE;

    pItem = m_pWingModel->item(sel,8);
    strong =pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pWing->NYPanels(sel) =(int)qMax(1.0,d);

    pItem = m_pWingModel->item(sel,9);
    strong =pItem->text();
    strong.replace(" ","");

    if(strong==tr("Uniform"))		m_pWing->YPanelDist(sel) = XFLR5::UNIFORM;
    else if(strong==tr("Cosine"))	m_pWing->YPanelDist(sel) = XFLR5::COSINE;
    else if(strong==tr("Sine"))		m_pWing->YPanelDist(sel) = XFLR5::SINE;
    else if(strong==tr("-Sine"))	m_pWing->YPanelDist(sel) = XFLR5::INVERSESINE;

}


void GL3dWingDlg::accept()
{
    s_bWindowMaximized= isMaximized();
    s_WindowPos = pos();
    s_WindowSize = size();

    s_bOutline    = m_pglWingView->m_bOutline;
    s_bSurfaces   = m_pglWingView->m_bSurfaces;
    s_bVLMPanels  = m_pglWingView->m_bVLMPanels;
    s_bAxes       = m_pglWingView->m_bAxes;
    s_bShowMasses = m_pglWingView->m_bShowMasses;
    s_bFoilNames  = m_pglWingView->m_bFoilNames;

    done(QDialog::Accepted);
}


void GL3dWingDlg::reject()
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

    s_bWindowMaximized= isMaximized();
    s_WindowPos = pos();
    s_WindowSize = size();

    s_bOutline    = m_pglWingView->m_bOutline;
    s_bSurfaces   = m_pglWingView->m_bSurfaces;
    s_bVLMPanels  = m_pglWingView->m_bVLMPanels;
    s_bAxes       = m_pglWingView->m_bAxes;
    s_bShowMasses = m_pglWingView->m_bShowMasses;
    s_bFoilNames  = m_pglWingView->m_bFoilNames;

    done(QDialog::Rejected);
}


void GL3dWingDlg::setCurrentSection(int section)
{
    m_iSection = section;
    if(m_iSection <0 || m_iSection>m_pWing->NWingSection())
    {
        m_pctrlInsertAfter->setEnabled(false);
        m_pctrlInsertBefore->setEnabled(false);
        m_pctrlDeleteSection->setEnabled(false);
    }
    else
    {
        m_pctrlInsertAfter->setEnabled(true);
        m_pctrlInsertBefore->setEnabled(true);
        m_pctrlDeleteSection->setEnabled(true);

        QString str;
        str = tr("Insert after section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_pctrlInsertAfter->setText(str);

        str = tr("Insert before section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_pctrlInsertBefore->setText(str);

        str = tr("Delete section") +" %1";
        str = QString(str).arg(m_iSection+1);
        m_pctrlDeleteSection->setText(str);
    }
    m_bResetglSectionHighlight = true;
}



void GL3dWingDlg::setWingData()
{
    if(!m_pWing) return;
    //Updates the wing's properties after a change of geometry

    QString str;

    str = QString("%1").arg(m_pWing->m_PlanformArea*Units::m2toUnit(),7,'f',2);
    m_pctrlWingArea->setText(str);

    str = QString("%1").arg(m_pWing->m_PlanformSpan*Units::mtoUnit(),5,'f',2);
    m_pctrlWingSpan->setText(str);

    str = QString("%1").arg(m_pWing->m_ProjectedArea*Units::m2toUnit(),7,'f',2);
    m_pctrlProjectedArea->setText(str);

    str = QString("%1").arg(m_pWing->m_ProjectedSpan*Units::mtoUnit(),5,'f',2);
    m_pctrlProjectedSpan->setText(str);

    str = QString("%1").arg(m_pWing->m_GChord*Units::mtoUnit(),5,'f',2);
    m_pctrlGeomChord->setText(str);

    str = QString("%1").arg(m_pWing->m_MAChord*Units::mtoUnit(),5,'f',2);
    m_pctrlMAC->setText(str);

    str = QString("%1").arg(m_pWing->m_AR,5,'f',2);
    m_pctrlAspectRatio->setText(str);

    if(m_pWing->tipChord()>0.0) str = QString("%1").arg(m_pWing->m_TR,0,'f',2);
    else                        str = tr("Undefined");
    m_pctrlTaperRatio->setText(str);

    str = QString("%1").arg(m_pWing->averageSweep(),5,'f',2);
    m_pctrlSweep->setText(str);

    str = QString("%1").arg(m_pWing->VLMPanelTotal(true));
    m_pctrlVLMPanels->setText(str);

    str = QString("%1").arg(m_pWing->VLMPanelTotal(false));
    m_pctrl3DPanels->setText(str);
}



void GL3dWingDlg::setupLayout()
{
    setMinimumHeight(700);

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    m_pglWingView = new gl3dWingView(this);
    m_pglWingView->m_bOutline    = s_bOutline;
    m_pglWingView->m_bSurfaces   = s_bSurfaces;
    m_pglWingView->m_bVLMPanels  = s_bVLMPanels;
    m_pglWingView->m_bAxes       = s_bAxes;
    m_pglWingView->m_bShowMasses = s_bShowMasses;
    m_pglWingView->m_bFoilNames  = s_bFoilNames;

    /*_____________Start Top Layout Here____________*/

    m_pLeftSideSplitter = new QSplitter(Qt::Vertical, this);
    {
        QWidget *pNameWidget = new QWidget(this);
        {
            QHBoxLayout *pNameLayout = new QHBoxLayout;
            {
                m_pctrlWingName     = new QLineEdit(tr("WingName"));
                pNameLayout->addWidget(m_pctrlWingName);
                QHBoxLayout *pStyleLayout = new QHBoxLayout;
                {
                    m_pctrlColor    = new QRadioButton(tr("Color"));
                    m_pctrlTextures = new QRadioButton(tr("Textures"));
                    m_pctrlWingColor    = new ColorButton;
                    m_pctrlWingColor->setSizePolicy(szPolicyMaximum);

                    pStyleLayout->addWidget(m_pctrlTextures);
                    pStyleLayout->addWidget(m_pctrlColor);

                    pStyleLayout->addWidget(m_pctrlWingColor);
                }
                pNameLayout->addLayout(pStyleLayout);

            }
            pNameWidget->setLayout(pNameLayout);
        }

        QWidget *pSymWidget = new QWidget(this);
        {
            QHBoxLayout *pSymLayout = new QHBoxLayout;
            {
                m_pctrlSymetric     = new QCheckBox(tr("Symetric"));
                m_pctrlRightSide    = new QRadioButton(tr("Right Side"));
                m_pctrlLeftSide     = new QRadioButton(tr("Left Side"));
                m_pctrlInsertBefore   = new QPushButton("Insert Before");
                m_pctrlInsertAfter    = new QPushButton("Insert After");
                m_pctrlDeleteSection  = new QPushButton("Delete Section");

                pSymLayout->addWidget(m_pctrlSymetric);
                pSymLayout->addStretch();
                pSymLayout->addWidget(m_pctrlRightSide);
                pSymLayout->addWidget(m_pctrlLeftSide);
                pSymLayout->addStretch();
                pSymLayout->addWidget(m_pctrlInsertBefore);
                pSymLayout->addWidget(m_pctrlInsertAfter);
                pSymLayout->addWidget(m_pctrlDeleteSection);
            }
            pSymWidget->setLayout(pSymLayout);
        }

        m_pctrlWingTable = new QTableView(this);
        m_pctrlWingTable->setWindowTitle(QObject::tr("Wing definition"));
        m_pctrlWingTable->setWordWrap(false);
        m_pctrlWingTable->setSizePolicy(szPolicyMaximum);
        m_pctrlWingTable->setSelectionMode(QAbstractItemView::SingleSelection);
        m_pctrlWingTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_pctrlWingTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                          QAbstractItemView::DoubleClicked |
                                          QAbstractItemView::SelectedClicked |
                                          QAbstractItemView::EditKeyPressed |
                                          QAbstractItemView::AnyKeyPressed);
        QHeaderView *HorizontalHeader = m_pctrlWingTable->horizontalHeader();
        HorizontalHeader->setStretchLastSection(true);


        pNameWidget->sizePolicy().setVerticalStretch(1);
        pSymWidget->sizePolicy().setVerticalStretch(1);
        m_pctrlWingTable->sizePolicy().setVerticalStretch(2);
        m_pglWingView->sizePolicy().setVerticalStretch(10);
        m_pLeftSideSplitter->addWidget(pNameWidget);
        m_pLeftSideSplitter->addWidget(pSymWidget);
        m_pLeftSideSplitter->addWidget(m_pctrlWingTable);
        m_pLeftSideSplitter->addWidget(m_pglWingView);
    }

    QWidget *pDataWidget = new QWidget(this);
    {
        QGridLayout *pDataLayout = new QGridLayout;
        {
            m_pctrlLength1    = new QLabel("mm", this);
            m_pctrlLength2    = new QLabel("mm", this);
            m_pctrlLength3    = new QLabel("mm", this);
            m_pctrlLength4    = new QLabel("mm", this);
            m_pctrlAreaUnit1  = new QLabel("mm2", this);
            m_pctrlAreaUnit2  = new QLabel("mm2", this);
            m_pctrlLength1->setAlignment(Qt::AlignLeft);
            m_pctrlLength2->setAlignment(Qt::AlignLeft);
            m_pctrlLength3->setAlignment(Qt::AlignLeft);
            m_pctrlLength4->setAlignment(Qt::AlignLeft);
            m_pctrlAreaUnit1->setAlignment(Qt::AlignLeft);
            m_pctrlAreaUnit2->setAlignment(Qt::AlignLeft);

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
            m_pctrlWingSpan      = new QLabel("2000.00");
            m_pctrlWingArea      = new QLabel("30.0");
            m_pctrlProjectedArea = new QLabel("25.0");
            m_pctrlProjectedSpan = new QLabel("1900.0");;
            m_pctrlVLMPanels     = new QLabel("500");
            m_pctrl3DPanels      = new QLabel("1000");
            m_pctrlWingSpan->setAlignment(Qt::AlignRight);
            m_pctrlWingArea->setAlignment(Qt::AlignRight);
            m_pctrlProjectedSpan->setAlignment(Qt::AlignRight);
            m_pctrlProjectedArea->setAlignment(Qt::AlignRight);
            m_pctrlVLMPanels->setAlignment(Qt::AlignRight);
            m_pctrl3DPanels->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(m_pctrlWingSpan,   1,2);
            pDataLayout->addWidget(m_pctrlWingArea,   2,2);
            pDataLayout->addWidget(m_pctrlProjectedSpan,   3,2);
            pDataLayout->addWidget(m_pctrlProjectedArea,   4,2);
            pDataLayout->addWidget(m_pctrlVLMPanels, 13,2);
            pDataLayout->addWidget(m_pctrl3DPanels,  14,2);

            pDataLayout->addWidget(m_pctrlLength1,1,3);
            pDataLayout->addWidget(m_pctrlAreaUnit1,2,3);
            pDataLayout->addWidget(m_pctrlLength2,3,3);
            pDataLayout->addWidget(m_pctrlAreaUnit2,4,3);

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

            m_pctrlGeomChord    = new QLabel("170.0", this);
            m_pctrlMAC          = new QLabel("150.0", this);
            m_pctrlAspectRatio  = new QLabel("13.33", this);
            m_pctrlTaperRatio   = new QLabel("1.50", this);
            m_pctrlSweep        = new QLabel("2.58", this);
            m_pctrlNFlaps       = new QLabel("0", this);
            m_pctrlMAC->setAlignment(Qt::AlignRight);
            m_pctrlGeomChord->setAlignment(Qt::AlignRight);
            m_pctrlAspectRatio->setAlignment(Qt::AlignRight);
            m_pctrlTaperRatio->setAlignment(Qt::AlignRight);
            m_pctrlSweep->setAlignment(Qt::AlignRight);
            m_pctrlNFlaps->setAlignment(Qt::AlignRight);
            pDataLayout->addWidget(m_pctrlGeomChord,    6,2);
            pDataLayout->addWidget(m_pctrlMAC,          7,2);
            pDataLayout->addWidget(m_pctrlAspectRatio,  9,2);
            pDataLayout->addWidget(m_pctrlTaperRatio,  10,2);
            pDataLayout->addWidget(m_pctrlSweep,       11,2);
            pDataLayout->addWidget(m_pctrlNFlaps,      12,2);
            pDataLayout->addWidget(m_pctrlLength3, 6, 3);
            pDataLayout->addWidget(m_pctrlLength4, 7, 3);
            QLabel *lab30 = new QLabel(QString::fromUtf8("°"));
            lab30->setAlignment(Qt::AlignLeft);
            pDataLayout->addWidget(lab30, 11, 3);
        }
        pDataWidget->setLayout(pDataLayout);
    }


    /*_____________End Top Right Layout Here______________*/

    m_pctrlWingDescription = new QTextEdit();
    m_pctrlWingDescription->setToolTip(tr("Enter here a short description for the wing"));

    QLabel *WingDescription = new QLabel(tr("Description:"));

    /*_____________Start Bottom Right Layout Here_________*/
    QWidget *pRightSideWidget = new QWidget(this);
    {
        QVBoxLayout *pRightSideLayout = new QVBoxLayout(this);
        {
            QGridLayout *ThreeDParams = new QGridLayout;
            {
                m_pctrlAxes       = new QCheckBox(tr("Axes"), this);
                m_pctrlSurfaces   = new QCheckBox(tr("Surfaces"), this);
                m_pctrlOutline    = new QCheckBox(tr("Outline"), this);
                m_pctrlPanels     = new QCheckBox(tr("Panels"), this);
                m_pctrlFoilNames  = new QCheckBox(tr("Foil Names"), this);
                m_pctrlShowMasses = new QCheckBox(tr("Masses"), this);
                m_pctrlAxes->setSizePolicy(szPolicyMinimum);
                m_pctrlSurfaces->setSizePolicy(szPolicyMinimum);
                m_pctrlOutline->setSizePolicy(szPolicyMinimum);
                m_pctrlPanels->setSizePolicy(szPolicyMinimum);
                ThreeDParams->addWidget(m_pctrlAxes, 1,1);
                ThreeDParams->addWidget(m_pctrlPanels, 1,2);
                ThreeDParams->addWidget(m_pctrlSurfaces, 2,1);
                ThreeDParams->addWidget(m_pctrlOutline, 2,2);
                ThreeDParams->addWidget(m_pctrlFoilNames, 3,1);
                ThreeDParams->addWidget(m_pctrlShowMasses, 3,2);
            }

            QVBoxLayout *pThreeDViewLayout = new QVBoxLayout;
            {
                QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
                {
                    m_pctrlX          = new QToolButton;
                    m_pctrlY          = new QToolButton;
                    m_pctrlZ          = new QToolButton;
                    m_pctrlIso        = new QToolButton;
                    m_pctrlFlip       = new QToolButton;
                    int iconSize =32;
                    if(m_pctrlX->iconSize().height()<=iconSize)
                    {
                        m_pctrlX->setIconSize(QSize(iconSize,iconSize));
                        m_pctrlY->setIconSize(QSize(iconSize,iconSize));
                        m_pctrlZ->setIconSize(QSize(iconSize,iconSize));
                        m_pctrlIso->setIconSize(QSize(iconSize,iconSize));
                        m_pctrlFlip->setIconSize(QSize(iconSize,iconSize));
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

                    m_pctrlX->setDefaultAction(m_pXView);
                    m_pctrlY->setDefaultAction(m_pYView);
                    m_pctrlZ->setDefaultAction(m_pZView);
                    m_pctrlIso->setDefaultAction(m_pIsoView);
                    m_pctrlFlip->setDefaultAction(m_pFlipView);
                    pAxisViewLayout->addWidget(m_pctrlX);
                    pAxisViewLayout->addWidget(m_pctrlY);
                    pAxisViewLayout->addWidget(m_pctrlZ);
                    pAxisViewLayout->addWidget(m_pctrlIso);
                    pAxisViewLayout->addWidget(m_pctrlFlip);
                }

                m_pctrlReset = new QPushButton(tr("Reset scale"));

                pThreeDViewLayout->addStretch();
                pThreeDViewLayout->addLayout(pAxisViewLayout);
                pThreeDViewLayout->addWidget(m_pctrlReset);
            }


            QHBoxLayout *pWingModCommands = new QHBoxLayout;
            {
                m_pctrlResetMesh    = new QPushButton(tr("Reset Mesh"));


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

                pWingModCommands->addWidget(m_pctrlResetMesh);
                pWingModCommands->addWidget(pMenuButton);
            }

            m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
            {
                connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
            }

            QWidget *pAll3DControlsWidget = new QWidget(this);
            {
                QVBoxLayout *pAll3DControlsLayout = new QVBoxLayout;
                {
                    pAll3DControlsLayout->addLayout(ThreeDParams);
                    pAll3DControlsLayout->addLayout(pThreeDViewLayout);
                    pAll3DControlsLayout->addLayout(pWingModCommands);
                    pAll3DControlsLayout->addStretch();
                    pAll3DControlsLayout->addWidget(m_pButtonBox);
                }
                pAll3DControlsWidget->setLayout(pAll3DControlsLayout);
            }

            pRightSideLayout->addWidget(WingDescription);
            pRightSideLayout->addWidget(m_pctrlWingDescription);
            pRightSideLayout->addWidget(pDataWidget);
            pRightSideLayout->addWidget(pAll3DControlsWidget);
            pRightSideWidget->setLayout(pRightSideLayout);
        }
    }

    QSplitter *pHorizontSplitter = new QSplitter(Qt::Horizontal, this);
    {
        pHorizontSplitter->addWidget(m_pLeftSideSplitter);
        pHorizontSplitter->addWidget(pRightSideWidget);
        m_pLeftSideSplitter->sizePolicy().setHorizontalStretch(4);
        pRightSideWidget->sizePolicy().setHorizontalStretch(1);
    }

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(pHorizontSplitter);

    setLayout(pMainLayout);
}


void GL3dWingDlg::showEvent(QShowEvent *event)
{
    move(s_WindowPos);
    resize(s_WindowSize);
    if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

    m_bChanged = false;
    m_bResetglWing = true;

    m_pglWingView->update();
    event->accept();
}


void GL3dWingDlg::resizeEvent(QResizeEvent *event)
{
    if(m_pLeftSideSplitter)
    {
        QList<int>leftSideSizes;
        leftSideSizes.clear();
        leftSideSizes.append((int)height()/30);
        leftSideSizes.append((int)height()/30);
        leftSideSizes.append((int)5*height()/20);
        leftSideSizes.append((int)13*height()/20);
        m_pLeftSideSplitter->setSizes(leftSideSizes);
    }


    double w = (double)m_pctrlWingTable->width()*.97;
    int wFoil  = (int)(w/5.);
    int wCols  = (int)(w/11);

    m_pctrlWingTable->setColumnWidth(0, wCols);
    m_pctrlWingTable->setColumnWidth(1, wCols);
    m_pctrlWingTable->setColumnWidth(2, wCols);
    m_pctrlWingTable->setColumnWidth(3, wCols);
    m_pctrlWingTable->setColumnWidth(4, wCols);
    m_pctrlWingTable->setColumnWidth(5, wFoil);
    m_pctrlWingTable->setColumnWidth(6, wCols);
    m_pctrlWingTable->setColumnWidth(7, wCols);
    m_pctrlWingTable->setColumnWidth(8, wCols);
    m_pctrlWingTable->setColumnWidth(9, wCols);

    if(m_pWing)	m_pglWingView->set3DScale(m_pWing->planformSpan());
    m_pglWingView->update();
    event->accept();
}



int GL3dWingDlg::VLMGetPanelTotal()
{
    double MinPanelSize;
    if(Wing::s_MinPanelSize>0.0) MinPanelSize = Wing::s_MinPanelSize;
    else                         MinPanelSize = m_pWing->m_PlanformSpan/1000.0;

    int total = 0;
    for (int i=0; i<m_pWing->NWingSection()-1; i++)
    {
        //do not create a surface if its length is less than the critical size
        //			if(qAbs(m_pWing->TPos[j]-m_pWing->TPos(j+1))/m_pWing->m_Span >0.001){
        if (qAbs(m_pWing->YPosition(i)-m_pWing->YPosition(i+1)) > MinPanelSize)
            total +=m_pWing->NXPanels(i)*m_pWing->NYPanels(i);
    }
    //	if(!m_bMiddle) total *=2;
    if(!m_pWing->m_bIsFin) return total*2;
    else                   return total;
}


bool GL3dWingDlg::VLMSetAutoMesh(int total)
{
    m_bChanged = true;
    //split (NYTotal) panels on each side proportionnaly to length, and space evenly
    //Set VLMMATSIZE/NYTotal panels along chord
    int NYTotal, size;

    if(!total)
    {
        size = (int)(2000/4);//why not ? Too much refinement isn't worthwile
        NYTotal = 22;
    }
    else
    {
        size = total;
        NYTotal = (int)sqrt((float)size);
    }

    NYTotal *= 2;

    //	double d1, d2; //spanwise panel densities at i and i+1

    for (int i=0; i<m_pWing->NWingSection()-1;i++)
    {
        //		d1 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos[i],  3) + 0.5;
        //		d2 = 5./2./m_pWing->m_Span/m_pWing->m_Span/m_pWing->m_Span *8. * pow(m_pWing->TPos(i+1),3) + 0.5;
        //		m_pWing->NYPanels(i) = (int) (NYTotal * (0.8*d1+0.2*d2)* (m_pWing->TPos(i+1)-m_pWing->TPos(i))/m_pWing->m_Span);

        m_pWing->NYPanels(i) = (int)(qAbs(m_pWing->YPosition(i+1) - m_pWing->YPosition(i))* (double)NYTotal/m_pWing->m_PlanformSpan);

        m_pWing->NXPanels(i) = (int) (size/NYTotal);

        if(m_pWing->NYPanels(i)==0) m_pWing->NYPanels(i) = 1;
        if(m_pWing->NXPanels(i)==0) m_pWing->NXPanels(i) = 1;
    }

    return true;
}



void GL3dWingDlg::onImportWingFromXML()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(0,
                                                QString("Open XML File"),
                                                Settings::s_LastDirName,
                                                tr("Plane XML file")+"(*.xml)");
    QFileInfo fileinfo(path_to_file);
    Settings::s_LastDirName = fileinfo.path();

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
        strong.sprintf("error on line %d column %d",(int)planereader.lineNumber(),(int)planereader.columnNumber());
        errorMsg += planereader.errorString() + " at " + strong;
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        return;
    }

    m_pWing->duplicate(plane.wing());
    initDialog(m_pWing);
    readParams();
    setWingData();
    m_bChanged = true;

    m_bResetglWing = true;
    m_pglWingView->update();
}


void GL3dWingDlg::onExportWingToXML()
{
    QString filter = "XML file (*.xml)";
    QString FileName, strong;

    strong = m_pWing->wingName().trimmed();
    strong.replace(' ', '_');
    FileName = QFileDialog::getSaveFileName(this, tr("Export to xml file"),
                                            Settings::s_LastDirName +'/'+strong,
                                            filter,
                                            &filter);

    if(!FileName.length()) return;
    QFileInfo fileInfo(FileName);
    Settings::s_LastDirName = fileInfo.path();

    if(FileName.indexOf(".xml", Qt::CaseInsensitive)<0) FileName += ".xml";

    QFile XFile(FileName);
    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    XMLPlaneWriter planeWriter(XFile);
    planeWriter.writeXMLWing(*m_pWing);

    XFile.close();
}


void GL3dWingDlg::onImportWing()
{
    QString path_to_file;
    path_to_file = QFileDialog::getOpenFileName(0,
                                                QString("Open File"),
                                                Settings::s_LastDirName,
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
    m_bResetglWing = true;
    m_pglWingView->update();
}


void GL3dWingDlg::onExportWing()
{
    QString path_to_file;
    path_to_file = QFileDialog::getSaveFileName(0,
                                                QString("Save File"),
                                                Settings::s_LastDirName,
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



bool GL3dWingDlg::intersectObject(Vector3d AA,  Vector3d U, Vector3d &I)
{
    double dist=0.0;

    for(int j=0; j<m_pWing->m_Surface.size(); j++)
    {
        if (Intersect(m_pWing->m_Surface.at(j)->m_LA,
                      m_pWing->m_Surface.at(j)->m_LB,
                      m_pWing->m_Surface.at(j)->m_TA,
                      m_pWing->m_Surface.at(j)->m_TB,
                      m_pWing->m_Surface.at(j)->Normal,
                      AA, U, I, dist))
            return true;
    }
    return false;
}
















