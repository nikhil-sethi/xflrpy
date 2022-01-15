/****************************************************************************

    GL3dBodyDlg Class
    Copyright (C) 2009-2019 André Deperrois

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

#include <QFileDialog>
#include <QColorDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QHeaderView>



#include "bodydlg.h"


#include <misc/lengthunitdlg.h>
#include <xfl3d/controls/arcball.h>
#include <xfl3d/controls/w3dprefs.h>
#include <xfl3d/views/gl3dbodyview.h>
#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/editors/bodyframewt.h>
#include <xflobjects/editors/bodylinewt.h>
#include <xflobjects/editors/bodyscaledlg.h>
#include <xflobjects/editors/bodytransdlg.h>
#include <xflobjects/editors/inertiadlg.h>
#include <xflobjects/objects3d/body.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/xml/xmlplanereader.h>
#include <xflobjects/xml/xmlplanewriter.h>
#include <xflwidgets/color/colormenubtn.h>
#include <xflwidgets/customwts/bodytabledelegate.h>
#include <xflwidgets/customwts/doubleedit.h>

QByteArray BodyDlg::m_VerticalSplitterSizes;
QByteArray BodyDlg::m_HorizontalSplitterSizes;
QByteArray BodyDlg::m_LeftSplitterSizes;



QByteArray BodyDlg::s_WindowGeometry;



bool BodyDlg::s_bOutline    = true;
bool BodyDlg::s_bSurfaces   = true;
bool BodyDlg::s_bVLMPanels  = false;
bool BodyDlg::s_bAxes       = true;
bool BodyDlg::s_bShowMasses = false;
bool BodyDlg::s_bFoilNames  = false;

BodyDlg::BodyDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Body Edition"));
    setWindowFlags(Qt::Window);
    setMouseTracking(true);

    m_pBody = nullptr;

    m_pPointDelegate = nullptr;
    m_pFrameDelegate = nullptr;

    m_pPointModel = nullptr;
    m_pFrameModel = nullptr;

    m_StackPos  = 0; //the current position on the stack
    m_bResetFrame = true;

    m_bChanged    = false;
    m_bEnableName = true;

    m_pScaleBody        = new QAction(tr("Scale"), this);
    m_pResetScales      = new QAction(tr("Reset Scales")+("\t(R)"), this);

    m_pUndo= new QAction(QIcon(":/images/OnUndo.png"), tr("Undo"), this);
    m_pUndo->setStatusTip(tr("Cancels the last modification"));
    m_pUndo->setShortcut(QKeySequence::Undo);

    m_pRedo = new QAction(QIcon(":/images/OnRedo.png"), tr("Redo"), this);
    m_pRedo->setStatusTip(tr("Restores the last cancelled modification"));
    m_pRedo->setShortcut(QKeySequence::Redo);

    m_pExportBodyGeom = new QAction(tr("Export Body Geometry to text File"), this);
    m_pExportBodyDef = new QAction(tr("Export Body Definition to txt File"), this);
    m_pExportBodyXML= new QAction(tr("Export body definition to an XML file"), this);
    m_pImportBodyDef = new QAction(tr("Import Body Definition from a text file"), this);
    m_pImportBodyXML= new QAction(tr("Import body definition from an XML file"), this);
    m_pBodyInertia = new QAction(tr("Define Inertia")+"\tF12", this);
    m_pTranslateBody = new QAction(tr("Translate"), this);

    setupLayout();
    setTableUnits();
    connectSignals();
}


void BodyDlg::connectSignals()
{
    connect(m_pUndo, SIGNAL(triggered()), SLOT(onUndo()));
    connect(m_pRedo, SIGNAL(triggered()), SLOT(onRedo()));
    connect(m_pExportBodyGeom, SIGNAL(triggered()), SLOT(onExportBodyGeom()));
    connect(m_pExportBodyDef, SIGNAL(triggered()), SLOT(onExportBodyDef()));
    connect(m_pExportBodyXML, SIGNAL(triggered()), SLOT(onExportBodyXML()));
    connect(m_pImportBodyDef, SIGNAL(triggered()), SLOT(onImportBodyDef()));
    connect(m_pImportBodyXML, SIGNAL(triggered()), SLOT(onImportBodyXML()));
    connect(m_pTranslateBody, SIGNAL(triggered()), SLOT(onTranslateBody()));
    connect(m_pBodyInertia,   SIGNAL(triggered()), SLOT(onBodyInertia()));

    connect(m_pScaleBody,        SIGNAL(triggered()), SLOT(onScaleBody()));
    connect(m_pResetScales,      SIGNAL(triggered()), SLOT(onResetScales()));

    connect(m_ppbReset,      SIGNAL(clicked()), SLOT(onResetScales()));

    connect(m_pchAxes,       SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onAxes(bool)));
    connect(m_pchPanels,     SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onPanels(bool)));
    connect(m_pchSurfaces,   SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,    SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onOutline(bool)));
    connect(m_pchShowMasses, SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onShowMasses(bool)));

    connect(m_ptbIso,        SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dIso()));
    connect(m_ptbX,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dFront()));
    connect(m_ptbY,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dLeft()));
    connect(m_ptbZ,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dTop()));
    connect(m_ptbFlip,       SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3dFlip()));

    connect(&m_gl3dBodyview, SIGNAL(viewModified()), SLOT(onCheckViewIcons()));

    connect(m_pslPanelBunch, SIGNAL(sliderMoved(int)), SLOT(onNURBSPanels()));

    connect(m_prbFlatPanels, SIGNAL(clicked()), SLOT(onLineType()));
    connect(m_prbBSplines,   SIGNAL(clicked()), SLOT(onLineType()));
    connect(m_pcbBodyColor,  SIGNAL(clickedCB(QColor)), SLOT(onBodyColor(QColor)));

    connect(m_pleBodyName,     SIGNAL(editingFinished()), SLOT(onBodyName()));


    connect(m_pdeNHoopPanels,  SIGNAL(editingFinished()), SLOT(onNURBSPanels()));
    connect(m_pdeNXPanels,     SIGNAL(editingFinished()), SLOT(onNURBSPanels()));
    connect(m_pcbXDegree,      SIGNAL(activated(int)), SLOT(onSelChangeXDegree(int)));
    connect(m_pcbHoopDegree,   SIGNAL(activated(int)), SLOT(onSelChangeHoopDegree(int)));

    connect(m_ppbUndo,       SIGNAL(clicked()),SLOT(onUndo()));
    connect(m_ppbRedo,       SIGNAL(clicked()),SLOT(onRedo()));

    connect(m_pSelectionModelFrame, SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onFrameItemClicked(QModelIndex)));
    connect(m_pFrameDelegate,       SIGNAL(closeEditor(QWidget*)), SLOT(onFrameCellChanged(QWidget*)));
    connect(m_pSelectionModelPoint, SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(onPointItemClicked(QModelIndex)));
    connect(m_pPointDelegate,       SIGNAL(closeEditor(QWidget*)), SLOT(onPointCellChanged(QWidget*)));

    connect(m_pBodyLineWt, SIGNAL(frameSelChanged()), SLOT(onFrameClicked()));
    connect(m_pFrameWt,    SIGNAL(pointSelChanged()), SLOT(onPointClicked()));

    connect(m_pBodyLineWt, SIGNAL(objectModified()), SLOT(onUpdateBody()));
    connect(m_pFrameWt,    SIGNAL(objectModified()), SLOT(onUpdateBody()));
}


void BodyDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)           accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void BodyDlg::setTableUnits()
{
    QString length;
    Units::getLengthUnitLabel(length);

    m_pFrameModel->setHeaderData(0, Qt::Horizontal, "x ("+length+")");
    m_pFrameModel->setHeaderData(1, Qt::Horizontal, tr("NPanels"));
    m_pPointModel->setHeaderData(0, Qt::Horizontal, "y ("+length+")");
    m_pPointModel->setHeaderData(1, Qt::Horizontal, "z ("+length+")");
    m_pPointModel->setHeaderData(2, Qt::Horizontal, tr("NPanels"));
}


BodyDlg::~BodyDlg()
{
    clearStack(-1);
    if(m_pFrameDelegate)  delete m_pFrameDelegate;

    if(m_pPointDelegate)  delete m_pPointDelegate;
    delete m_pFrameModel;
    delete m_pPointModel;
}


void BodyDlg::fillFrameCell(int iItem, int iSubItem)
{
    QModelIndex ind;

    switch (iSubItem)
    {
        case 0:
        {
            ind = m_pFrameModel->index(iItem, 0, QModelIndex());
            m_pFrameModel->setData(ind, m_pBody->frame(iItem)->m_Position.x * Units::mtoUnit());
            break;
        }
        case 1:
        {
            ind = m_pFrameModel->index(iItem, 1, QModelIndex());
            m_pFrameModel->setData(ind, m_pBody->m_xPanels[iItem]);
            break;
        }
        default:
        {
            break;
        }
    }
}


void BodyDlg::fillFrameDataTable()
{
    if(!m_pBody) return;
    m_pFrameModel->setRowCount(m_pBody->frameCount());

    for(int i=0; i<m_pBody->frameCount(); i++)
    {
        fillFrameTableRow(i);
    }
}


void BodyDlg::fillFrameTableRow(int row)
{
    QModelIndex ind;

    ind = m_pFrameModel->index(row, 0, QModelIndex());
    m_pFrameModel->setData(ind, m_pBody->frame(row)->m_Position.x * Units::mtoUnit());

    ind = m_pFrameModel->index(row, 1, QModelIndex());
    m_pFrameModel->setData(ind, m_pBody->m_xPanels[row]);
}


void BodyDlg::fillPointDataTable()
{
    if(!m_pBody) return;

    m_pPointModel->setRowCount(m_pBody->sideLineCount());
    for(int i=0; i<m_pBody->sideLineCount(); i++)
    {
        fillPointTableRow(i);
    }
}


void BodyDlg::fillPointCell(int iItem, int iSubItem)
{
    QModelIndex ind;

    if(!m_pBody) return;
    int l = m_pBody->m_iActiveFrame;

    switch (iSubItem)
    {
        case 0:
        {
            ind = m_pPointModel->index(iItem, 0, QModelIndex());
            m_pPointModel->setData(ind, m_pBody->frame(l)->m_CtrlPoint[iItem].y * Units::mtoUnit());
            break;
        }
        case 1:
        {
            ind = m_pPointModel->index(iItem, 1, QModelIndex());
            m_pPointModel->setData(ind, m_pBody->frame(l)->m_CtrlPoint[iItem].z*Units::mtoUnit());
            break;
        }
        case 2:
        {
            ind = m_pPointModel->index(iItem, 2, QModelIndex());
            m_pPointModel->setData(ind,m_pBody->m_hPanels[iItem]);
            break;
        }

        default:
        {
            break;
        }
    }
}


void BodyDlg::fillPointTableRow(int row)
{
    if(!m_pFrame) return;
    QModelIndex ind;

    ind = m_pPointModel->index(row, 0, QModelIndex());
    m_pPointModel->setData(ind, m_pFrame->m_CtrlPoint[row].y * Units::mtoUnit());

    ind = m_pPointModel->index(row, 1, QModelIndex());
    m_pPointModel->setData(ind, m_pFrame->m_CtrlPoint[row].z * Units::mtoUnit());

    ind = m_pPointModel->index(row, 2, QModelIndex());
    m_pPointModel->setData(ind, m_pBody->m_hPanels[row]);
}


void BodyDlg::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    bool bCtrl  = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    if(pEvent->modifiers() & Qt::ControlModifier) bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_Z:
        {
            if(bCtrl)
            {
                if(bShift)
                {
                    onRedo();
                }
                else onUndo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }
        case Qt::Key_Y:
        {
            if(bCtrl)
            {
                onRedo();
                pEvent->accept();
            }
            else pEvent->ignore();
            break;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_F12:
        {
            onBodyInertia();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void BodyDlg::setViewControls()
{
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
    m_ptbIso->setChecked(false);
}


void BodyDlg::onBodyName()
{
    if(m_pBody)
    {
        m_pBody->m_Name = m_pleBodyName->text();
        m_pBody->m_Description = m_pteBodyDescription->toPlainText();
    }
}


void BodyDlg::onBodyColor(QColor clr)
{   
    if(!m_pBody) return;

    if(clr.isValid())
    {
        m_pBody->setColor(clr);
    }

    m_gl3dBodyview.resetGLBody();
    m_gl3dBodyview.update();
}


/**
 * Unselects all the 3D-view icons.
 */
void BodyDlg::onCheckViewIcons()
{
    m_ptbIso->setChecked(false);
    m_ptbX->setChecked(false);
    m_ptbY->setChecked(false);
    m_ptbZ->setChecked(false);
}


void BodyDlg::onBodyInertia()
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
    m_bChanged = true;
    updateView();
}


void BodyDlg::onExportBodyXML()
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



void BodyDlg::onExportBodyDef()
{
    if(!m_pBody) return;

    QString FileName;

    FileName = m_pBody->m_Name;
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Definition"),
                                            xfl::s_LastDirName,
                                            QObject::tr("Text Format (*.txt)"));
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream outStream(&XFile);

    m_pBody->exportBodyDefinition(outStream, Units::mtoUnit());
}


void BodyDlg::onExportBodyGeom()
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


void BodyDlg::onImportBodyDef()
{
    Body memBody;
    memBody.duplicate(m_pBody);

    double mtoUnit = 1.0;

    LengthUnitDlg luDlg(this);

    luDlg.m_Question = QObject::tr("Choose the length unit to read this file :");
    luDlg.initDialog(Units::lengthUnitIndex());

    if(luDlg.exec() == QDialog::Accepted)
    {
        mtoUnit = luDlg.mtoUnit();
    }
    else return;

    QString PathName;

    PathName = QFileDialog::getOpenFileName(this, QObject::tr("Open File"),
                                            xfl::s_LastDirName,
                                            QObject::tr("All files (*.*)"));
    if(!PathName.length()) return;
    int pos = PathName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = PathName.left(pos);

    QFile XFile(PathName);
    if (!XFile.open(QIODevice::ReadOnly))
    {
        QString strange = QObject::tr("Could not read the file\n")+PathName;
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        return;
    }

    QTextStream in(&XFile);
    QString errorMsg;
    if(!m_pBody->importDefinition(in, mtoUnit, errorMsg))
    {
        QMessageBox::warning(this, QObject::tr("Warning"), errorMsg);
        m_pBody->duplicate(&memBody);
        return;
    }

    XFile.close();

    setBody();

    m_gl3dBodyview.resetGLBody();

    m_bChanged = true;

    updateView();
}



void BodyDlg::onImportBodyXML()
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
    setBody();

    m_gl3dBodyview.resetGLBody();


    m_bChanged = true;

    updateView();
}


void BodyDlg::readFrameSectionData(int sel)
{
    if(sel>=m_pFrameModel->rowCount()) return;
    double x=0;
    int k=0;

    bool bOK=false;
    QString strong;
    QStandardItem *pItem;

    pItem = m_pFrameModel->item(sel,0);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    x = strong.toDouble(&bOK);
    if(bOK) m_pBody->frame(sel)->setuPosition(x / Units::mtoUnit());

    for(int ic=0; ic<m_pBody->frame(sel)->pointCount(); ic++)
    {
        m_pBody->frame(sel)->m_CtrlPoint[ic].x  = x / Units::mtoUnit();
    }

    pItem = m_pFrameModel->item(sel,1);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    k = strong.toInt(&bOK);
    if(bOK) m_pBody->m_xPanels[sel] = k;
}


/** The user has clicked a point in the body line view */
void BodyDlg::onFrameClicked()
{
    m_ptvFrames->selectRow(m_pBody->m_iActiveFrame);
}


void BodyDlg::onFrameItemClicked(const QModelIndex &index)
{
    m_pBody->m_iActiveFrame = index.row();
    setFrame(m_pBody->m_iActiveFrame);
    updateView();
}


void BodyDlg::onFrameCellChanged(QWidget *)
{
    takePicture();
    m_bChanged = true;
    //    int n = m_pBody->m_iActiveFrame;
    readFrameSectionData(m_pBody->m_iActiveFrame);
    m_gl3dBodyview.resetGLBody();


    updateView();
}


void BodyDlg::onLineType()
{
    m_bChanged = true;
    if(m_prbFlatPanels->isChecked())
    {
        m_pBody->m_LineType = xfl::BODYPANELTYPE;
        m_pdeNXPanels->setEnabled(false);
        m_pdeNHoopPanels->setEnabled(false);
        m_pcbXDegree->setEnabled(false);
        m_pcbHoopDegree->setEnabled(false);
    }
    else
    {
        m_pBody->m_LineType = xfl::BODYSPLINETYPE;
        m_pdeNXPanels->setEnabled(true);
        m_pdeNHoopPanels->setEnabled(true);
        m_pcbXDegree->setEnabled(true);
        m_pcbHoopDegree->setEnabled(true);
    }
    m_gl3dBodyview.resetGLBody();

    updateView();
}


void BodyDlg::onPointCellChanged(QWidget *)
{
    if(!m_pFrame) return;

    takePicture();
    m_bChanged = true;
    for(int ip=0; ip<m_pPointModel->rowCount(); ip++)
        readPointSectionData(ip);
    m_gl3dBodyview.resetGLBody();
    updateView();
}


/** The user has clicked a point in the frame view */
void BodyDlg::onPointClicked()
{
    if(m_pFrame)
        m_ptvPoints->selectRow(m_pFrame->selectedIndex());
}


void BodyDlg::onPointItemClicked(const QModelIndex &index)
{
    if(!m_pFrame) return;
    m_pFrame->setSelected(index.row());
    m_pFrame->setHighlighted(index.row());
    updateView();
}


void BodyDlg::onResetScales()
{
    m_gl3dBodyview.reset3dScale();
    m_pBodyLineWt->onResetScales();
    m_pFrameWt->onResetScales();
    updateView();
}


void BodyDlg::onScaleBody()
{
    if(!m_pBody) return;

    BodyScaleDlg dlg(this);

    dlg.m_FrameID = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        takePicture();
        m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
        m_gl3dBodyview.resetGLBody();

        fillFrameDataTable();
        fillPointDataTable();

        updateView();
    }
}


void BodyDlg::onUpdateBody()
{
    takePicture();

    m_bChanged = true;
    m_gl3dBodyview.resetGLBody();

    m_pFrame = m_pBody->activeFrame();

    fillFrameDataTable();
    fillPointDataTable();


    updateView();
}



void BodyDlg::onSelChangeXDegree(int sel)
{
    if(!m_pBody) return;
    if (sel <0) return;

    takePicture();
    m_bChanged = true;

    int deg = sel+1;
    if(deg>=m_pBody->nurbs().frameCount())
    {
        QString strange = tr("The degree must be less than the number of Frames");
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        deg=m_pBody->nurbs().frameCount();
        m_pcbXDegree->setCurrentIndex(m_pBody->nurbs().frameCount()-2);
    }

    m_pBody->m_SplineSurface.setuDegree(deg);
    m_pBody->setNURBSKnots();
    m_gl3dBodyview.resetGLBody();

    updateView();
}


void BodyDlg::onSelChangeHoopDegree(int sel)
{
    if(!m_pBody) return;
    if (sel<0) return;

    takePicture();

    m_bChanged = true;

    int deg = sel+1;
    if(deg>=m_pBody->nurbs().framePointCount())
    {
        QString strange = tr("The degree must be less than the number of side lines");
        QMessageBox::warning(this, QObject::tr("Warning"), strange);
        deg=m_pBody->nurbs().framePointCount();
        m_pcbHoopDegree->setCurrentIndex(m_pBody->nurbs().framePointCount()-2);
    }

    m_pBody->m_SplineSurface.setvDegree(deg);
    m_pBody->setNURBSKnots();
    m_gl3dBodyview.resetGLBody();

    updateView();
}


void BodyDlg::onEdgeWeight()
{
}


void BodyDlg::onNURBSPanels()
{
    if(!m_pBody) return;

    m_bChanged = true;
    takePicture();

    m_pBody->m_Bunch = m_pslPanelBunch->sliderPosition()/100.0;

    m_pBody->m_nhPanels = int(m_pdeNHoopPanels->value());
    m_pBody->m_nxPanels = int(m_pdeNXPanels->value());
    m_pBody->setPanelPos();

    m_gl3dBodyview.resetGLBody();

    updateView();
}


void BodyDlg::onTranslateBody()
{
    if(!m_pBody) return;

    BodyTransDlg dlg(this);
    dlg.m_FrameID    = m_pBody->m_iActiveFrame;
    dlg.initDialog();

    if(dlg.exec()==QDialog::Accepted)
    {
        takePicture();
        m_pBody->translate(dlg.m_XTrans, dlg.m_YTrans, dlg.m_ZTrans, dlg.m_bFrameOnly, dlg.m_FrameID);
        fillFrameDataTable();
        fillPointDataTable();

        m_gl3dBodyview.resetGLBody();

        updateView();
    }
}


void BodyDlg::readPointSectionData(int sel)
{
    if(sel>=m_pPointModel->rowCount()) return;
    if(!m_pFrame) return;

    double d=0;
    int k=0;

    bool bOK=false;
    QString strong;
    QStandardItem *pItem;

    pItem = m_pPointModel->item(sel,0);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pFrame->m_CtrlPoint[sel].y =d / Units::mtoUnit();

    pItem = m_pPointModel->item(sel,1);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    d =strong.toDouble(&bOK);
    if(bOK) m_pFrame->m_CtrlPoint[sel].z =d / Units::mtoUnit();

    pItem = m_pPointModel->item(sel,2);
    if(!pItem) return;
    strong = pItem->text();
    strong.replace(" ","");
    k =strong.toInt(&bOK);
    if(bOK) m_pBody->m_hPanels[sel] = k;
}


void BodyDlg::accept()
{
    if(m_pBody)
    {
        m_pBody->setDescription(m_pteBodyDescription->toPlainText());
        QColor clr = m_pcbBodyColor->color();
        m_pBody->setColor(clr);
    }

    s_bOutline    = m_gl3dBodyview.m_bOutline;
    s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
    s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
    s_bAxes       = m_gl3dBodyview.m_bAxes;
    s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
    s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

    done(QDialog::Accepted);
}


void BodyDlg::reject()
{
    s_bOutline    = m_gl3dBodyview.m_bOutline;
    s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
    s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
    s_bAxes       = m_gl3dBodyview.m_bAxes;
    s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
    s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

    if(m_bChanged)
    {
        m_pBody->m_Name = m_pleBodyName->text();

        int res = QMessageBox::question(this, tr("Body Dlg Exit"), tr("Save the Body ?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        if (QMessageBox::No == res)
        {
            m_pBody = nullptr;
            QDialog::reject();
        }
        else if (QMessageBox::Cancel == res) return;
        else
        {
            m_pBody = nullptr;
            done(QDialog::Accepted);
            return;
        }
    }
    else m_pBody = nullptr;

    done(QDialog::Rejected);
}


void BodyDlg::resizeEvent(QResizeEvent *pEvent)
{
    //    SetBodyScale();
    //    SetRectangles();

    resizeTables();
    pEvent->accept();
}


bool BodyDlg::loadSettings(QSettings &settings)
{
    settings.beginGroup("GL3dBody");
    {
        s_WindowGeometry = settings.value("WindowGeom", QByteArray()).toByteArray();

        m_HorizontalSplitterSizes = settings.value("HorizontalSplitterSizes").toByteArray();
        m_LeftSplitterSizes = settings.value("LeftSplitterSizes").toByteArray();
    }
    settings.endGroup();
    return true;
}



bool BodyDlg::saveSettings(QSettings &settings)
{
    settings.beginGroup("GL3dBody");
    {
        settings.setValue("WindowGeom", s_WindowGeometry);

        settings.setValue("HorizontalSplitterSizes", m_HorizontalSplitterSizes);
        settings.setValue("LeftSplitterSizes", m_LeftSplitterSizes);

    }
    settings.endGroup();
    return true;
}


void BodyDlg::setControls()
{
    m_pleBodyName->setEnabled(m_bEnableName);

    m_pchOutline->setChecked(m_gl3dBodyview.m_bOutline);
    m_pchPanels->setChecked(m_gl3dBodyview.m_bVLMPanels);
    m_pchAxes->setChecked(m_gl3dBodyview.m_bAxes);
    m_pchShowMasses->setChecked(m_gl3dBodyview.m_bShowMasses);
    m_pchSurfaces->setChecked(m_gl3dBodyview.m_bSurfaces);


    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);

    if(m_pBody && m_pBody->m_LineType==xfl::BODYPANELTYPE)
    {
        m_pdeNXPanels->setEnabled(false);
        m_pdeNHoopPanels->setEnabled(false);
        m_pcbXDegree->setEnabled(false);
        m_pcbHoopDegree->setEnabled(false);
    }
    else if(m_pBody && m_pBody->m_LineType==xfl::BODYSPLINETYPE)
    {
        m_pdeNXPanels->setEnabled(true);
        m_pdeNHoopPanels->setEnabled(true);
        m_pcbXDegree->setEnabled(true);
        m_pcbHoopDegree->setEnabled(true);
    }

    if(m_pBody)
    {
        m_pslPanelBunch->setSliderPosition(int(m_pBody->m_Bunch*100.0));
        m_pcbBodyColor->setColor(m_pBody->m_Color);

        m_pdeNXPanels->setValue(m_pBody->m_nxPanels);
        m_pdeNHoopPanels->setValue(m_pBody->m_nhPanels);

        m_pcbXDegree->setCurrentIndex(m_pBody->m_SplineSurface.uDegree()-1);
        m_pcbHoopDegree->setCurrentIndex(m_pBody->m_SplineSurface.vDegree()-1);
    }
}


bool BodyDlg::initDialog(Body *pBody)
{
    if(!pBody) return false;

    m_ptvFrames->setFont(DisplayOptions::tableFont());
    m_ptvPoints->setFont(DisplayOptions::tableFont());

    m_gl3dBodyview.setBody(pBody);
    m_gl3dBodyview.setScale(pBody->length());

    m_pBodyLineWt->setUnitFactor(Units::mtoUnit());
    m_pFrameWt->setUnitFactor(Units::mtoUnit());

    return setBody(pBody);
}


bool BodyDlg::setBody(Body *pBody)
{
    if(pBody) m_pBody = pBody;

    m_prbFlatPanels->setChecked(m_pBody->m_LineType==xfl::BODYPANELTYPE);
    m_prbBSplines->setChecked(m_pBody->m_LineType==xfl::BODYSPLINETYPE);

    m_pBodyLineWt->setBody(m_pBody);
    m_pFrameWt->setBody(m_pBody);

    m_pFrame = m_pBody->activeFrame();

    setControls();
    fillFrameDataTable();
    fillPointDataTable();

    m_pleBodyName->setText(m_pBody->m_Name);

    takePicture();

    return true;
}


void BodyDlg::setFrame(int iFrame)
{
    if(!m_pBody) return;
    if(iFrame<0 || iFrame>=m_pBody->frameCount()) m_pFrame = nullptr;
    else                                          m_pFrame = m_pBody->frame(iFrame);
    m_pBody->m_iActiveFrame = iFrame;

    m_gl3dBodyview.resetGLBody();

    fillPointDataTable();;
}


void BodyDlg::setFrame(Frame *pFrame)
{
    if(!m_pBody || !pFrame) return;

    m_pBody->setActiveFrame(pFrame);

    m_gl3dBodyview.resetGLBody();

    fillPointDataTable();;
}



void BodyDlg::setupLayout()
{
    QString str;

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

//    m_gl3dBodyview.m_pglBodyDlg = this;
    m_gl3dBodyview.m_bOutline    = s_bOutline;
    m_gl3dBodyview.m_bSurfaces   = s_bSurfaces;
    m_gl3dBodyview.m_bVLMPanels  = s_bVLMPanels;
    m_gl3dBodyview.m_bAxes       = s_bAxes;
    m_gl3dBodyview.m_bShowMasses = s_bShowMasses;
    m_gl3dBodyview.m_bFoilNames  = s_bFoilNames;
    m_pwtControls = new QWidget;
    {
        QVBoxLayout *pControlsLayout = new QVBoxLayout;
        {
            QGridLayout *pThreeDParamsLayout = new QGridLayout;
            {
                m_pchAxes       = new QCheckBox(tr("Axes"));
                m_pchSurfaces   = new QCheckBox(tr("Surfaces"));
                m_pchOutline    = new QCheckBox(tr("Outline"));
                m_pchPanels     = new QCheckBox(tr("Panels"));
                m_pchShowMasses = new QCheckBox(tr("Masses"));
                m_pchAxes->setSizePolicy(szPolicyMinimum);
                m_pchSurfaces->setSizePolicy(szPolicyMinimum);
                m_pchOutline->setSizePolicy(szPolicyMinimum);
                m_pchPanels->setSizePolicy(szPolicyMinimum);
                pThreeDParamsLayout->addWidget(m_pchAxes, 1,1);
                pThreeDParamsLayout->addWidget(m_pchPanels, 1,2);
                pThreeDParamsLayout->addWidget(m_pchSurfaces, 2,1);
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
                if(m_ptbX->iconSize().height()<=48)
                {
                    m_ptbX->setIconSize(QSize(32,32));
                    m_ptbY->setIconSize(QSize(32,32));
                    m_ptbZ->setIconSize(QSize(32,32));
                    m_ptbIso->setIconSize(QSize(32,32));
                    m_ptbFlip->setIconSize(QSize(32,32));
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

            QHBoxLayout* pThreeDViewLayout = new QHBoxLayout;
            {
                m_ppbReset      = new QPushButton(tr("Reset Scale"));
                m_ppbReset->setSizePolicy(szPolicyMinimum);

                pThreeDViewLayout->addWidget(m_ppbReset);
            }


            QHBoxLayout *pActionButtonsLayout = new QHBoxLayout;
            {
                m_ppbUndo = new QPushButton(QIcon(":/images/OnUndo.png"), tr("Undo"));
                m_ppbRedo = new QPushButton(QIcon(":/images/OnRedo.png"), tr("Redo"));

                m_ppbMenuButton = new QPushButton(tr("Other"));

                BodyMenu = new QMenu(tr("Actions..."),this);

                BodyMenu->addAction(m_pImportBodyDef);
                BodyMenu->addAction(m_pExportBodyDef);
                BodyMenu->addSeparator();
                BodyMenu->addAction(m_pImportBodyXML);
                BodyMenu->addAction(m_pExportBodyXML);
                BodyMenu->addSeparator();
                BodyMenu->addAction(m_pExportBodyGeom);
                BodyMenu->addSeparator();
                BodyMenu->addAction(m_pBodyInertia);
                BodyMenu->addSeparator();
                BodyMenu->addAction(m_pTranslateBody);
                BodyMenu->addAction(m_pScaleBody);
                BodyMenu->addSeparator();
                m_ppbMenuButton->setMenu(BodyMenu);

                pActionButtonsLayout->addWidget(m_ppbUndo);
                pActionButtonsLayout->addWidget(m_ppbRedo);
                pActionButtonsLayout->addWidget(m_ppbMenuButton);
            }

            m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
            {
                connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
            }

            pControlsLayout->addLayout(pAxisViewLayout);
            pControlsLayout->addLayout(pThreeDParamsLayout);
            pControlsLayout->addLayout(pThreeDViewLayout);
            pControlsLayout->addStretch(1);
            pControlsLayout->addLayout(pActionButtonsLayout);
            pControlsLayout->addStretch(1);
            pControlsLayout->addWidget(m_pButtonBox);
        }

        QVBoxLayout *pBodyParamsLayout = new QVBoxLayout;
        {
            QHBoxLayout *pNameLayout = new QHBoxLayout;
            {
                m_pleBodyName = new QLineEdit(tr("BodyName"));
                m_pcbBodyColor = new ColorMenuBtn;
                pNameLayout->addWidget(m_pleBodyName);
                pNameLayout->addWidget(m_pcbBodyColor);
            }

//            QLabel *plabBodyDes = new QLabel(tr("Description:"));
            m_pteBodyDescription = new QTextEdit();
            m_pteBodyDescription->setToolTip(tr("Enter here a short description for the body"));
            pBodyParamsLayout->setStretchFactor(m_pteBodyDescription,1);

            pBodyParamsLayout->addLayout(pNameLayout);
            pBodyParamsLayout->addWidget(m_pteBodyDescription);
            pBodyParamsLayout->addStretch(1);
        }

        QVBoxLayout *pBodySettingsLayout = new QVBoxLayout;
        {
            QGroupBox *pBodyTypeBox = new QGroupBox(tr("Type"));
            {
                QHBoxLayout *pBodyTypeLayout = new QHBoxLayout;
                {
                    m_prbFlatPanels = new QRadioButton(tr("Flat Panels"));
                    m_prbBSplines   = new QRadioButton(tr("BSplines"));
                    m_prbFlatPanels->setSizePolicy(szPolicyMinimum);
                    m_prbBSplines->setSizePolicy(szPolicyMinimum);
                    pBodyTypeLayout->addWidget(m_prbFlatPanels);
                    pBodyTypeLayout->addWidget(m_prbBSplines);
                }
                pBodyTypeBox->setLayout(pBodyTypeLayout);
            }

            QGridLayout *pSplineParams = new QGridLayout;
            {
                QLabel *lab1 = new QLabel(tr("x"));
                QLabel *lab2 = new QLabel(tr("Hoop"));
                QLabel *lab3 = new QLabel(tr("Degree"));
                QLabel *lab4 = new QLabel(tr("Panels"));
                QLabel *labBunch = new QLabel(tr("Panel bunch"));

                m_pcbXDegree = new QComboBox;
                m_pcbHoopDegree = new QComboBox;
                m_pdeNXPanels = new DoubleEdit;
                m_pdeNHoopPanels = new DoubleEdit;

                m_pslPanelBunch= new QSlider(Qt::Horizontal);
                m_pslPanelBunch->setMinimum(0    );
                m_pslPanelBunch->setMaximum(100.0);
                m_pslPanelBunch->setSliderPosition(0);
                m_pslPanelBunch->setTickInterval(10);
                m_pslPanelBunch->setTickPosition(QSlider::TicksBelow);
                m_pslPanelBunch->setSizePolicy(szPolicyMinimum);


                lab1->setSizePolicy(szPolicyMinimum);
                lab2->setSizePolicy(szPolicyMinimum);
                lab3->setSizePolicy(szPolicyMinimum);
                lab4->setSizePolicy(szPolicyMinimum);
                m_pcbXDegree->setSizePolicy(szPolicyMinimum);
                m_pcbHoopDegree->setSizePolicy(szPolicyMinimum);
                m_pdeNXPanels->setSizePolicy(szPolicyMinimum);
                m_pdeNHoopPanels->setSizePolicy(szPolicyMinimum);
                m_pdeNXPanels->setDigits(0);
                m_pdeNHoopPanels->setDigits(0);
                pSplineParams->addWidget(lab1,1,2, Qt::AlignCenter);
                pSplineParams->addWidget(lab2,1,3, Qt::AlignCenter);
                pSplineParams->addWidget(lab3,2,1, Qt::AlignRight);
                pSplineParams->addWidget(lab4,3,1, Qt::AlignRight);
                pSplineParams->addWidget(m_pcbXDegree,2,2);
                pSplineParams->addWidget(m_pcbHoopDegree,2,3);
                pSplineParams->addWidget(m_pdeNXPanels,3,2);
                pSplineParams->addWidget(m_pdeNHoopPanels,3,3);
                pSplineParams->addWidget(labBunch,5,1);
                pSplineParams->addWidget(m_pslPanelBunch,5,2,1,2);
            }

            pBodySettingsLayout->addWidget(pBodyTypeBox);
            pBodySettingsLayout->addStretch();
            pBodySettingsLayout->addLayout(pSplineParams);
        }

        QVBoxLayout * pFramePosLayout = new QVBoxLayout;
        {
            m_ptvFrames = new QTableView;
            m_ptvFrames->setWindowTitle(tr("Frames"));
            QLabel *LabelFrame = new QLabel(tr("Frame Positions"));
            LabelFrame->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            pFramePosLayout->addWidget(LabelFrame);
            //    FramePosLayout->addStretch(1);
            m_ptvFrames->setSelectionMode(QAbstractItemView::SingleSelection);
            m_ptvFrames->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_ptvFrames->setEditTriggers(
                        QAbstractItemView::DoubleClicked |
                        QAbstractItemView::SelectedClicked |
                        QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::AnyKeyPressed);
            pFramePosLayout->addWidget(m_ptvFrames);
        }

        QVBoxLayout * pFramePointLayout = new QVBoxLayout;
        {
            m_ptvPoints = new QTableView;
            m_ptvPoints->setWindowTitle(tr("Points"));
            QLabel *LabelPoints = new QLabel(tr("Current Frame Definition"));
            LabelPoints->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
            pFramePointLayout->addWidget(LabelPoints);
            //    FramePointLayout->addStretch(1);
            m_ptvPoints->setSelectionMode(QAbstractItemView::SingleSelection);
            m_ptvPoints->setSelectionBehavior(QAbstractItemView::SelectRows);
            m_ptvPoints->setEditTriggers(
                        QAbstractItemView::DoubleClicked |
                        QAbstractItemView::SelectedClicked |
                        QAbstractItemView::EditKeyPressed |
                        QAbstractItemView::AnyKeyPressed);

            pFramePointLayout->addWidget(m_ptvPoints);
        }

        QHBoxLayout *pAllControls = new QHBoxLayout;
        {
            pAllControls->addLayout(pBodyParamsLayout);
            pAllControls->addLayout(pBodySettingsLayout);
            pAllControls->addLayout(pFramePosLayout);
            pAllControls->addLayout(pFramePointLayout);
            pAllControls->addLayout(pControlsLayout);
        }
        m_pwtControls->setLayout(pAllControls);
    }

    m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
    {
        m_pLeftSplitter = new QSplitter(Qt::Vertical, this);
        {
            m_pBodyLineWt = new BodyLineWt(this);
            m_pBodyLineWt->setSizePolicy(szPolicyMaximum);

            m_pLeftSplitter->addWidget(m_pBodyLineWt);
            m_pLeftSplitter->addWidget(&m_gl3dBodyview);
        }
        m_pFrameWt = new BodyFrameWt(this);
        m_pHorizontalSplitter->addWidget(m_pLeftSplitter);
        m_pHorizontalSplitter->addWidget(m_pFrameWt);
    }

    m_pVerticalSplitter = new QSplitter(Qt::Vertical, this);
    {
        m_pVerticalSplitter->addWidget(m_pHorizontalSplitter);
        m_pVerticalSplitter->addWidget(m_pwtControls);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        pMainLayout->addWidget(m_pVerticalSplitter);
    }
    setLayout(pMainLayout);

    for (int i=1; i<6; i++)
    {
        str = QString("%1").arg(i);
        m_pcbXDegree->addItem(str);
        m_pcbHoopDegree->addItem(str);
    }

    //Setup Frame table
    m_ptvFrames->horizontalHeader()->setStretchLastSection(true);

    m_pFrameModel = new QStandardItemModel;
    m_pFrameModel->setRowCount(10);//temporary
    m_pFrameModel->setColumnCount(2);

    m_ptvFrames->setModel(m_pFrameModel);

    m_pSelectionModelFrame = new QItemSelectionModel(m_pFrameModel);
    m_ptvFrames->setSelectionModel(m_pSelectionModelFrame);

    m_pFrameDelegate = new BodyTableDelegate(this);
    m_ptvFrames->setItemDelegate(m_pFrameDelegate);
    m_pFrameDelegate->setPrecision({3,0});

    //Setup Point Table
    m_ptvPoints->horizontalHeader()->setStretchLastSection(true);

    m_pPointModel = new QStandardItemModel(this);
    m_pPointModel->setRowCount(10);//temporary
    m_pPointModel->setColumnCount(3);
    m_ptvPoints->setModel(m_pPointModel);
    m_pSelectionModelPoint = new QItemSelectionModel(m_pPointModel);
    m_ptvPoints->setSelectionModel(m_pSelectionModelPoint);

    m_pPointDelegate = new BodyTableDelegate;
    m_ptvPoints->setItemDelegate(m_pPointDelegate);
    m_pPointDelegate->setPrecision({3,3,0});
}


void BodyDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_WindowGeometry);

    if(m_VerticalSplitterSizes.length()>0)
        m_pHorizontalSplitter->restoreState(m_VerticalSplitterSizes);
    if(m_HorizontalSplitterSizes.length()>0)
        m_pHorizontalSplitter->restoreState(m_HorizontalSplitterSizes);
    if(m_LeftSplitterSizes.length()>0)
        m_pLeftSplitter->restoreState(m_LeftSplitterSizes);

    setTableUnits();
    m_bChanged    = false;
    m_gl3dBodyview.resetGLBody();

    resizeTables();

    updateView();

    pEvent->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void BodyDlg::hideEvent(QHideEvent *pEvent)
{
    s_WindowGeometry = saveGeometry();

    m_VerticalSplitterSizes  = m_pVerticalSplitter->saveState();
    m_HorizontalSplitterSizes  = m_pHorizontalSplitter->saveState();
    m_LeftSplitterSizes  = m_pLeftSplitter->saveState();
    pEvent->accept();
}


void BodyDlg::resizeTables()
{
    int ColumnWidth = int(double(m_ptvFrames->width())/2.5);
    m_ptvFrames->setColumnWidth(0,ColumnWidth);
    m_ptvFrames->setColumnWidth(1,ColumnWidth);
    ColumnWidth = int(double(m_ptvPoints->width())/4);
    m_ptvPoints->setColumnWidth(0,ColumnWidth);
    m_ptvPoints->setColumnWidth(1,ColumnWidth);
    m_ptvPoints->setColumnWidth(2,ColumnWidth);
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void BodyDlg::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        delete m_UndoStack.at(il);
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}


/**
 * Restores a SplineFoil definition from the current position in the stack.
 */
void BodyDlg::setPicture()
{
    Body const *pTmpBody = m_UndoStack.at(m_StackPos);
    m_pBody->duplicate(pTmpBody);
    m_pBody->setNURBSKnots();


    fillFrameDataTable();

    m_pFrame = m_pBody->activeFrame();

    fillPointDataTable();

    m_gl3dBodyview.resetGLBody();

    updateView();
}



/**
 * Copies the current Body object to a new Body and pushes it on the stack.
 */
void BodyDlg::takePicture()
{
    m_bChanged = true;

    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    // append a copy of the current object
    Body *pBody = new Body();
    pBody->duplicate(m_pBody);
    m_UndoStack.append(pBody);

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void BodyDlg::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
        m_ppbRedo->setEnabled(true);
    }
    else
    {
        //nothing to restore
    }
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void BodyDlg::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
    }
    m_ppbUndo->setEnabled(m_StackPos>0);
    m_ppbRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void BodyDlg::updateView()
{
    if(isVisible()) m_gl3dBodyview.update();
    m_pFrameWt->update();
    m_pBodyLineWt->update();
}


void BodyDlg::blockSignalling(bool bBlock)
{
    blockSignals(bBlock);
    m_pPointDelegate->blockSignals(bBlock);
    m_pFrameDelegate->blockSignals(bBlock);
    m_ptvPoints->blockSignals(bBlock);
    m_ptvFrames->blockSignals(bBlock);

    m_pSelectionModelPoint->blockSignals(bBlock);
    m_pSelectionModelFrame->blockSignals(bBlock);
}



