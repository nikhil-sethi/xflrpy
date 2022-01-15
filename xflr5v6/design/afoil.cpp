/****************************************************************************

    AFoil Class
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

#include <QAction>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QHBoxLayout>


#include <design/afoil.h>
#include <design/afoiltabledlg.h>
#include <design/foiltabledelegate.h>
#include <design/lecircledlg.h>
#include <design/splinectrlsdlg.h>
#include <globals/mainframe.h>
#include <gui_objects/splinefoil.h>

#include <xflobjects/editors/renamedlg.h>
#include <xdirect/geometry/cadddlg.h>
#include <xdirect/geometry/flapdlg.h>
#include <xdirect/geometry/foilcoorddlg.h>
#include <xdirect/geometry/foilgeomdlg.h>
#include <xdirect/geometry/interpolatefoilsdlg.h>
#include <xdirect/geometry/ledlg.h>
#include <xdirect/geometry/nacafoildlg.h>
#include <xdirect/geometry/tegapdlg.h>
#include <xdirect/geometry/twodpaneldlg.h>
#include <xflobjects/objects2d/objects2d.h>
#include <xdirect/xdirect.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects2d/foil.h>
#include <xflobjects/objects_global.h>
#include <xflwidgets/line/linemenu.h>
#include <xfoil.h>


MainFrame *AFoil::s_pMainFrame = nullptr;

/**
 * The public constructor
 * @param parent a pointer to the MainFrame window
 */
AFoil::AFoil(QWidget *parent)  : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_p2dWidget = nullptr;

    m_StackPos = 0;

    m_ptvFoil = nullptr;

    m_pSF = new SplineFoil();
    m_pSF->m_bModified = false;
    m_pSF->initSplineFoil();

    s_pMainFrame->m_pUndoAFoilAct = s_pMainFrame->m_pRedoAFoilAct = nullptr;

    clearStack();
    takePicture();

    m_bStored        = false;

    m_pBufferFoil = new Foil();

    m_StackPos = 0;

    setupLayout();

    SplineCtrlsDlg::s_pAFoil    = this;
}


AFoil::~AFoil()
{
    clearStack(-1);
    if(m_pSF) delete m_pSF;
    if(m_pBufferFoil) delete m_pBufferFoil;
}


/**
 * Initializes the state of the button widgets and of the QAction objects.
 */
void AFoil::setControls()
{
    s_pMainFrame->m_pAFoilDelete->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilRename->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilExport->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilDuplicateFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pShowCurrentFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pHideCurrentFoil->setEnabled(Objects2d::curFoil());

    s_pMainFrame->m_pAFoilDerotateFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilEditCoordsFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilInterpolateFoils->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilNormalizeFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilRefineGlobalFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilRefineLocalFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilScaleFoil->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilSetFlap->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilSetLERadius->setEnabled(Objects2d::curFoil());
    s_pMainFrame->m_pAFoilSetTEGap->setEnabled(Objects2d::curFoil());

    s_pMainFrame->m_pShowLegend->setChecked(m_p2dWidget->m_bShowLegend);

    s_pMainFrame->m_pAFoilSplineMenu->setEnabled(!Objects2d::curFoil());
    s_pMainFrame->m_pAFoilSplineMenu_AFoilCtxMenu->setEnabled(!Objects2d::curFoil());
    s_pMainFrame->m_pInsertSplinePt->setEnabled(!Objects2d::curFoil());
    s_pMainFrame->m_pRemoveSplinePt->setEnabled(!Objects2d::curFoil());

    s_pMainFrame->m_pUndoAFoilAct->setEnabled(m_StackPos>0);
    s_pMainFrame->m_pRedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
    s_pMainFrame->statusBar()->clearMessage();
}


/**
 * Fills the table with the data from the Foil objects.
 */
void AFoil::fillFoilTable()
{
    m_pFoilModel->setRowCount(Objects2d::foilCount()+1);

    QString name;
    QModelIndex ind;

    double Thickness=0, xThickness=0, Camber=0, xCamber=0;
    int points = 0;

    if(m_pSF)
    {
        name = m_pSF->splineFoilName();
        Thickness  = m_pSF->thickness();
        xThickness = m_pSF->xThickness();
        Camber     = m_pSF->camber();
        xCamber    = m_pSF->xCamber();
        points     = m_pSF->m_OutPoints;
    }

    ind = m_pFoilModel->index(0, 0, QModelIndex());
    m_pFoilModel->setData(ind,name);

    ind = m_pFoilModel->index(0, 1, QModelIndex());
    m_pFoilModel->setData(ind, Thickness);

    ind = m_pFoilModel->index(0, 2, QModelIndex());
    m_pFoilModel->setData(ind, xThickness);

    ind = m_pFoilModel->index(0, 3, QModelIndex());
    m_pFoilModel->setData(ind, Camber);

    ind = m_pFoilModel->index(0, 4, QModelIndex());
    m_pFoilModel->setData(ind,xCamber);

    ind = m_pFoilModel->index(0, 5, QModelIndex());
    m_pFoilModel->setData(ind, points);

    for(int i=0; i<Objects2d::foilCount(); i++)
    {
        fillTableRow(i+1);
    }
}


/**
 * Fills the data from a Foil object in the specified table row.
 * @param row the index of the row to be filled
 */
void AFoil::fillTableRow(int row)
{
    QModelIndex ind;

    Foil const*pFoil = Objects2d::foilAt(row-1);

    ind = m_pFoilModel->index(row, 0, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->name());

    ind = m_pFoilModel->index(row, 1, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->thickness());

    ind = m_pFoilModel->index(row, 2, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->xThickness());

    ind = m_pFoilModel->index(row, 3, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->camber());

    ind = m_pFoilModel->index(row, 4, QModelIndex());
    m_pFoilModel->setData(ind,pFoil->xCamber());

    ind = m_pFoilModel->index(row, 5, QModelIndex());
    m_pFoilModel->setData(ind,pFoil->m_n);


    if(pFoil && pFoil->m_bTEFlap)
    {
        ind = m_pFoilModel->index(row, 6, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEFlapAngle);

        ind = m_pFoilModel->index(row, 7, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEXHinge/100.0);

        ind = m_pFoilModel->index(row, 8, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEYHinge/100.0);

    }
    if(pFoil && pFoil->m_bLEFlap)
    {
        ind = m_pFoilModel->index(row, 9, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEFlapAngle);

        ind = m_pFoilModel->index(row, 10, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEXHinge/100.0);

        ind = m_pFoilModel->index(row, 11, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEYHinge/100.0);
    }

    ind = m_pFoilModel->index(row, 12, QModelIndex());
    if(pFoil->isVisible()) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
    else                   m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);
    QStandardItem *pItem = m_pFoilModel->item(row,12);
    if(pItem) pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);

    ind = m_pFoilModel->index(row, 13, QModelIndex());
    if(pFoil->bCenterLine()) m_pFoilModel->setData(ind, Qt::Checked, Qt::CheckStateRole);
    else                     m_pFoilModel->setData(ind, Qt::Unchecked, Qt::CheckStateRole);
    pItem = m_pFoilModel->item(row,13);
    if(pItem) pItem->setFlags(Qt::ItemIsEnabled|Qt::ItemIsUserCheckable);
}


/**
 * Overrides the QWidget's keyPressEvent method.
 * Dispatches the key press event
 * @param event the QKeyEvent
 */
void AFoil::keyPressEvent(QKeyEvent *pEvent)
{
    bool bShift = false;
    if(pEvent->modifiers() & Qt::ShiftModifier)   bShift =true;
    bool bCtrl = false;
    if(pEvent->modifiers() & Qt::ControlModifier)   bCtrl =true;

    switch (pEvent->key())
    {
        case Qt::Key_F2:
        {
            onRenameFoil();
            break;
        }
        case Qt::Key_F3:
        {
            if(Objects2d::curFoil())
            {
                if(bShift) onAFoilCadd();
                else       onAFoilPanels();
            }
            break;
        }
        case Qt::Key_F4:
        {
            onStoreSplines();
            break;
        }
        case Qt::Key_F9:
        {
            onAFoilFoilGeom();
            break;
        }
        case Qt::Key_F10:
        {
            onAFoilSetFlap();
            break;
        }
        case Qt::Key_F11:
        {
            onAFoilInterpolateFoils();
            break;
        }
        case Qt::Key_Z:
        {
            if(bCtrl)
            {
                if(bShift) onRedo();
                else       onUndo();
            }
            break;
        }

        default:
            pEvent->ignore();
    }
}


/**
 * Loads the user's default settings from the application QSettings object.
 * @param pSettings a pointer to the QSettings object
 */
void AFoil::loadSettings(QSettings &settings)
{
    settings.beginGroup("DirectDesign");
    {
        m_pSF->theStyle().loadSettings(settings, "SplineFoil");

        m_pSF->m_bOutPoints  = settings.value("SFOutPoints").toBool();
        m_pSF->m_bCenterLine = settings.value("SFCenterLine").toBool();

        m_pSF->m_Intrados.m_iRes =  qMax(settings.value("LowerRes",79).toInt(), 10);
        m_pSF->m_Extrados.m_iRes =  qMax(settings.value("UpperRes",79).toInt(), 10);

        m_pSF->m_Extrados.splineCurve();
        m_pSF->m_Intrados.splineCurve();

        m_p2dWidget->m_bLECircle          = settings.value("LECircle").toBool();
        m_p2dWidget->m_bShowLegend        = settings.value("Legend").toBool();

        QString str;
        for(int i=0; i<16; i++)
        {
            str = QString("Column_%1").arg(i);
            m_ptvFoil->setColumnWidth(i, settings.value(str,40).toInt());
            if(settings.value(str+"_hidden", false).toBool()) m_ptvFoil->hideColumn(i);
        }

        m_p2dWidget->m_Grid.loadSettings(settings);
    }
    settings.endGroup();
}


/**
 * The user has requested that the foil be derotated.
 */
void AFoil::onAFoilDerotateFoil()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    double angle = m_pBufferFoil->deRotate();
    QString str = QString(tr("Foil has been de-rotated by %1 degrees")).arg(angle,6,'f',3);
    s_pMainFrame->statusBar()->showMessage(str);

    //then duplicate the buffer foil and add it
    Foil *pNewFoil = new Foil();
    pNewFoil->copyFoil(m_pBufferFoil);
    xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
    pNewFoil->setLineStipple(Line::SOLID);
    pNewFoil->setLineWidth(1);

    addNewFoil(pNewFoil);
    fillFoilTable();
    selectFoil(pNewFoil);

    m_pBufferFoil->setVisible(false);

    m_p2dWidget->update();
}


/**
 * The user has requested that the Foil be normalized to unit length.
 */
void AFoil::onAFoilNormalizeFoil()
{
    if(!Objects2d::curFoil()) return;
    double length = Objects2d::curFoil()->normalizeGeometry();
    Objects2d::curFoil()->initFoil();
    QString str = QString(tr("Foil has been normalized from %1  to 1.000")).arg(length,7,'f',3);

    s_pMainFrame->statusBar()->showMessage(str);

    m_p2dWidget->update();
}


/**
 * The user has requested a local refinement of the panels of the current Foil.
 */
void AFoil::onAFoilCadd()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    CAddDlg caDlg(s_pMainFrame);
    caDlg.m_pBufferFoil = m_pBufferFoil;
    caDlg.m_pMemFoil    = Objects2d::curFoil();
    caDlg.initDialog();

    if(QDialog::Accepted == caDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());

    }
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}

/**
 * The user has requested the display of a circle at the L.E. position.
 */
void AFoil::onAFoilLECircle()
{
    LECircleDlg LECircleDlg(this);
    LECircleDlg.m_Radius      = m_p2dWidget->m_LERad;
    LECircleDlg.m_bShowRadius = m_p2dWidget->m_bLECircle;
    LECircleDlg.initDialog();

    if(LECircleDlg.exec()==QDialog::Accepted)
    {
        m_p2dWidget->m_LERad = LECircleDlg.m_Radius;
        m_p2dWidget->m_bLECircle = LECircleDlg.m_bShowRadius;
    }
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to refine globally the Foil.
*/
void AFoil::onAFoilPanels()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    TwoDPanelDlg tdpDlg(s_pMainFrame);
    tdpDlg.m_pBufferFoil = m_pBufferFoil;
    tdpDlg.m_pMemFoil    = Objects2d::curFoil();
    tdpDlg.initDialog();

    if(QDialog::Accepted == tdpDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else
            delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
        //        m_pXFoil->foilName() ="";

    }

    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}

/**
 * The user has requested the launch of the interface to edit the Foil coordinates manually.
*/
void AFoil::onAFoilFoilCoordinates()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    FoilCoordDlg fcDlg(s_pMainFrame);
    fcDlg.m_pMemFoil    = Objects2d::curFoil();
    fcDlg.m_pBufferFoil = m_pBufferFoil;
    fcDlg.initDialog();

    if(QDialog::Accepted == fcDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        pNewFoil->setPointStyle(Line::NOSYMBOL);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setHighLight(-1);

        addNewFoil(pNewFoil);
        fillFoilTable();
        selectFoil(pNewFoil);
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
    }
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested to perform an edition of the current foil's thickness and camber properties.
 */
void AFoil::onAFoilFoilGeom()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    FoilGeomDlg fgeDlg(s_pMainFrame);
    fgeDlg.m_pMemFoil    = Objects2d::curFoil();
    fgeDlg.m_pBufferFoil = m_pBufferFoil;
    fgeDlg.initDialog();

    if(QDialog::Accepted == fgeDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);
        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else
        {
            delete pNewFoil;
        }
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
        //        m_pXFoil->foilName() ="";
    }
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to modify the gap at the Foil's trailing edge.
 */
void AFoil::onAFoilSetTEGap()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    TEGapDlg teDlg(s_pMainFrame);
    teDlg.m_pBufferFoil = m_pBufferFoil;
    teDlg.m_pMemFoil    = Objects2d::curFoil();
    teDlg.initDialog();

    if(QDialog::Accepted == teDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
        //        m_pXFoil->foilName() ="";
        //to un-initialize XFoil in case user switches to XInverse
        //Thanks Jean-Marc !
    }

    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to modify the radius of the Foil's leading edge.
 */
void AFoil::onAFoilSetLERadius()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    LEDlg leDlg(s_pMainFrame);
    leDlg.m_pBufferFoil = m_pBufferFoil;
    leDlg.m_pMemFoil    = Objects2d::curFoil();
    leDlg.initDialog();

    if(QDialog::Accepted == leDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else
            delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
        //        m_pXFoil->foilName() ="";

    }

    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to create a foil from the interpolation of two existing Foil objects.
 */
void AFoil::onAFoilInterpolateFoils()
{
    if(Objects2d::foilCount()<2)
    {
        QMessageBox::warning(s_pMainFrame,tr("Warning"), tr("At least two foils are required"));
        return;
    }

    if(!Objects2d::curFoil()) selectFoil();
    if(!Objects2d::curFoil()) return;
    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    InterpolateFoilsDlg ifDlg(s_pMainFrame);
    ifDlg.m_pBufferFoil = m_pBufferFoil;
    ifDlg.initDialog();

    if(QDialog::Accepted == ifDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);
        pNewFoil->setName(ifDlg.m_NewFoilName);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else delete pNewFoil;

    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
    }
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface used to create a NACA type Foil.
 */
void AFoil::onAFoilNacaFoils()
{
    m_pBufferFoil->setNaca009();
    m_pBufferFoil->setName("Naca xxxx");
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    NacaFoilDlg nacaDlg(s_pMainFrame);
    nacaDlg.m_pBufferFoil = m_pBufferFoil;

    if(QDialog::Accepted == nacaDlg.exec())
    {
        //then duplicate the buffer foil and add it
        QString str;

        if(nacaDlg.s_Digits>0 && log10(double(nacaDlg.s_Digits))<4)
            str = QString("%1").arg(nacaDlg.s_Digits,4,10,QChar('0'));
        else
            str = QString("%1").arg(nacaDlg.s_Digits);
        str = "NACA "+ str;

        Foil *pNewFoil    = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);
        pNewFoil->setPointStyle(Line::NOSYMBOL);
        pNewFoil->setName(str);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        if(Objects2d::curFoil()) selectFoil(Objects2d::curFoil());
    }

    setControls();
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to define a L.E. or T.E. flap.
 */
void AFoil::onAFoilSetFlap()
{
    if(!Objects2d::curFoil()) return;

    m_pBufferFoil->copyFoil(Objects2d::curFoil());
    m_pBufferFoil->setName(Objects2d::curFoil()->name());
    m_pBufferFoil->setEditStyle();

    m_p2dWidget->update();

    FlapDlg flDlg(s_pMainFrame);
    flDlg.m_pMemFoil    = Objects2d::curFoil();
    flDlg.m_pBufferFoil = m_pBufferFoil;
    flDlg.initDialog();

    if(QDialog::Accepted == flDlg.exec())
    {
        //then duplicate the buffer foil and add it
        Foil *pNewFoil = new Foil();
        pNewFoil->copyFoil(m_pBufferFoil);
        xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
        pNewFoil->setLineStipple(Line::SOLID);
        pNewFoil->setLineWidth(1);

        if(addNewFoil(pNewFoil))
        {
            fillFoilTable();
            selectFoil(pNewFoil);
        }
        else delete pNewFoil;
    }
    else
    {
        fillFoilTable();
        selectFoil(Objects2d::curFoil());
    }
    m_pBufferFoil->setVisible(false);
    m_p2dWidget->update();
}


/**
 * The user has requested the deletion of the current Foil.
 */
void AFoil::onDeleteCurFoil()
{
    if(!Objects2d::curFoil()) return;

    QString strong;
    strong = tr("Are you sure you want to delete")  +"\n"+ Objects2d::curFoil()->name() +"\n";
    strong+= tr("and all associated OpPoints and Polars ?");

    int resp = QMessageBox::question(s_pMainFrame, tr("Question"), strong,  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if(resp != QMessageBox::Yes) return;

    Foil*pNextFoil = Objects2d::deleteFoil(Objects2d::curFoil());

    fillFoilTable();
    selectFoil(pNextFoil);
    m_p2dWidget->update();
    emit projectModified();
}


/**
 * The user has requested the duplication of the current Foil.
 */
void AFoil::onDuplicate()
{
    if(!Objects2d::curFoil()) return;
    Foil *pNewFoil = new Foil;
    pNewFoil->copyFoil(Objects2d::curFoil());
    xfl::setRandomFoilColor(pNewFoil, !DisplayOptions::isLightTheme());
    pNewFoil->initFoil();

    addNewFoil(pNewFoil);
    fillFoilTable();
    selectFoil(pNewFoil);
}


/**
 * The user has requested the export of the current Foil to a text file.
 */
void AFoil::onExportCurFoil()
{
    if(!Objects2d::curFoil())    return;

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
 * The user has requested the export of the current SplineFoil to a text file.
 */
void AFoil::onExportSplinesToFile()
{
    QString FoilName = tr("Spline Foil");
    QString FileName, strong;
    QString strOut;

    // deselect points so as not to interfere with other mouse commands
    m_pSF->m_Intrados.m_iSelect = -10;
    m_pSF->m_Extrados.m_iSelect = -10;

    //check that the number of output points is consistent with the array's size

    if(m_pSF->m_Extrados.m_iRes>IQX2)
    {
        strong = QString(tr("Too many output points on upper surface\n Max =%1")).arg(IQX2);
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
        return;
    }
    if(m_pSF->m_Intrados.m_iRes>IQX2)
    {
        strong = QString(tr("Too many output points on lower surface\n Max =%1")).arg(IQX2);
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
        return;
    }


    FileName.replace("/", " ");
    FileName = QFileDialog::getSaveFileName(this, tr("Export Splines"),
                                            xfl::s_LastDirName,
                                            tr("Text File (*.dat)"));

    if(!FileName.length()) return;
    int pos;
    pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);


    strOut = FoilName + "\n";
    out << strOut;

    m_pSF->exportToFile(out);
    XFile.close();
}


/**
 * A row has been clicked in the table of Foil objects.
 * @param index a QModelIndex object clicked in the table
 */
void AFoil::onFoilClicked(const QModelIndex& index)
{
    m_ptvFoil->blockSignals(true);
    m_pFoilModel->blockSignals(true);

    if(index.row()>=Objects2d::foilCount()+1) return;
    QStandardItem *pItem = m_pFoilModel->item(index.row(),0);
    if(!pItem) return;

    m_ptvFoil->selectRow(index.row());

    if(index.row()==0)
    {
        XDirect::setCurFoil(nullptr);
        if(index.column()==12)
        {
            m_pSF->setVisible(!m_pSF->isVisible());
        }
        else if(index.column()==13)
        {
            m_pSF->m_bCenterLine = !m_pSF->m_bCenterLine;
        }
        m_p2dWidget->update();
    }
    else if(index.row()>0)
    {
        Foil *pFoil= Objects2d::foil(pItem->text());
        XDirect::setCurFoil(pFoil);

        if(pFoil)
        {
            if(index.column()==12)
            {
                pFoil->setVisible(!pFoil->isVisible());
            }
            else if(index.column()==13)
            {
                pFoil->showCenterLine(!pFoil->bCenterLine());
            }
        }
        emit projectModified();
        m_p2dWidget->update();
    }

    if(index.column()==14) onFoilStyle();

    setControls();

    m_ptvFoil->blockSignals(false);
    m_pFoilModel->blockSignals(false);
}


/**
 * The user has requested an edition of the style of the active Foil.
 */
void AFoil::onFoilStyle()
{
    if(!Objects2d::curFoil())
    {
        LineStyle ls(m_pSF->theStyle());
        LineMenu *pLineMenu = new LineMenu(nullptr);
        pLineMenu->initMenu(ls);
        pLineMenu->exec(QCursor::pos());
        ls = pLineMenu->theStyle();

        m_pSF->setTheStyle(ls);
        m_pSF->m_Extrados.setTheStyle(ls);
        m_pSF->m_Intrados.setTheStyle(ls);
        m_p2dWidget->update();

        emit projectModified();
    }
    else
    {
        Foil *pFoil = Objects2d::curFoil();
        LineStyle ls(pFoil->theStyle());
        LineMenu *pLineMenu = new LineMenu(nullptr);
        pLineMenu->initMenu(ls);
        pLineMenu->exec(QCursor::pos());
        ls = pLineMenu->theStyle();
        pFoil->setTheStyle(ls);


        if(DisplayOptions::isAlignedChildrenStyle())
            Objects2d::setFoilChildrenStyle(Objects2d::curFoil());

        m_p2dWidget->update();
        emit projectModified();
    }
}


/**
 * The user has requested that the visibility of all Foil objects be turned off.
 */
void AFoil::onHideAllFoils()
{
    emit projectModified();

    for (int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pFoil = Objects2d::foilAt(k);
        pFoil->setVisible(false);
    }
    fillFoilTable();
    m_p2dWidget->update();
}


/**
 * The user has requested that the visibility of the active Foil object be turned off.
 */
void AFoil::onHideCurrentFoil()
{
    if(!Objects2d::curFoil()) return;
    showFoil(Objects2d::curFoil(), false);
    m_p2dWidget->update();

}


/**
 * The user has requested to restore the default settings for the splines.
 */
void AFoil::onNewSplines()
{
    if(m_pSF->m_bModified)
    {
        if (QMessageBox::Yes != QMessageBox::question(s_pMainFrame, tr("Question"), tr("Discard changes to Splines ?"),
                                                      QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel))
        {
            return;
        }
    }
    m_pSF->initSplineFoil();

    m_StackPos  = 0;
    clearStack(0);
    takePicture();

    emit projectModified();
    m_p2dWidget->update();
}


/**
 * The user has requested to rename the Foil object
 */
void AFoil::onRenameFoil()
{
    if(!Objects2d::curFoil()) return;

    QStringList NameList;
    for(int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pOldFoil = Objects2d::foilAt(k);
        NameList.append(pOldFoil->name());
    }

    RenameDlg renDlg(this);
    renDlg.initDialog(&NameList, Objects2d::curFoil()->name(), tr("Enter the foil's new name"));

    if(renDlg.exec() !=QDialog::Rejected)
    {
        Objects2d::renameThisFoil(Objects2d::curFoil(), renDlg.newName());
    }

    fillFoilTable();
    m_p2dWidget->update();
}


/**
 * The user has requested that the visibility of all Foil objects be turned on.
 */
void AFoil::onShowAllFoils()
{
    emit projectModified();

    for (int k=0; k<Objects2d::foilCount(); k++)
    {
        Foil*pFoil = Objects2d::foilAt(k);
        pFoil->setVisible(true);
    }
    fillFoilTable();
    m_p2dWidget->update();
}


/**
 * The user has requested that the visibility of the active Foil object be turned on.
 */
void AFoil::onShowCurrentFoil()
{
    if(!Objects2d::curFoil()) return;
    showFoil(Objects2d::curFoil(), true);
    m_p2dWidget->update();

}


/**
 * The user has toggled the visibility of the legend
 */
void AFoil::onShowLegend()
{
    m_p2dWidget->m_bShowLegend = !m_p2dWidget->m_bShowLegend;
    m_p2dWidget->update();
    setControls();
}


/**
 * The user has requested to convert the SplineFoil object to a Foil, and to store it in the database.
 */
void AFoil::onStoreSplines()
{
    if(m_pSF->m_Extrados.m_iRes>IQX2)
    {
        QString strong = QString(tr("Too many output points on upper surface\n Max =%1")).arg(IQX2);
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
        return;
    }
    if(m_pSF->m_Intrados.m_iRes>IQX2)
    {
        QString strong = QString(tr("Too many output points on lower surface\n Max =%1")).arg(IQX2);
        QMessageBox::warning(s_pMainFrame, tr("Warning"), strong, QMessageBox::Ok);
        return;
    }


    Foil *pNewFoil = new Foil();
    m_pSF->exportToBuffer(pNewFoil);
    pNewFoil->setName(m_pSF->splineFoilName());
    pNewFoil->initFoil();
    addNewFoil(pNewFoil);
    fillFoilTable();
    selectFoil(pNewFoil);

    m_p2dWidget->update();
}


/**
 * The user has requested the launch of the interface to edit SplineFoil data.
 */
void AFoil::onSplineControls()
{
    SplineCtrlsDlg dlg(this);
    dlg.m_pSF = m_pSF;
    dlg.initDialog();

    SplineFoil memSF;
    memSF.copy(m_pSF);

    if(dlg.exec() == QDialog::Accepted)
    {
        takePicture();
        fillFoilTable();
    }
    else m_pSF->copy(&memSF);
}


/**
 * Saves the user-defined settings.
 * @param pSettings a pointer to the QSetting object.
 */
void AFoil::saveSettings(QSettings &settings)
{
    settings.beginGroup("DirectDesign");
    {
        m_pSF->theStyle().saveSettings(settings, "SplineFoil");

        settings.setValue("SFOutPoints", m_pSF->m_bOutPoints);
        settings.setValue("SFCenterLine", m_pSF->m_bCenterLine);

        settings.setValue("LowerRes", m_pSF->m_Intrados.m_iRes);
        settings.setValue("UpperRes", m_pSF->m_Extrados.m_iRes);

        settings.setValue("LECircle", m_p2dWidget->m_bLECircle);
        settings.setValue("Legend", m_p2dWidget->m_bShowLegend );

        QString str;
        for(int i=0; i<16; i++)
        {
            str = QString("Column_%1").arg(i);
            settings.setValue(str,m_ptvFoil->columnWidth(i));
        }
        for(int i=0; i<16; i++)
        {
            str = QString("Column_%1").arg(i);
            settings.setValue(str+"_hidden", m_ptvFoil->isColumnHidden(i));
        }

        m_p2dWidget->m_Grid.saveSettings(settings);
    }
    settings.endGroup();
}


/**
 * The user has requested the context menu associated to the Foil table.
 * @param position the right-click positon
 */
void AFoil::onFoilTableCtxMenu(const QPoint &)
{
    s_pMainFrame->m_pAFoilTableCtxMenu->exec(cursor().pos());
}


/**
 * Sets up the GUI.
 */
void AFoil::setupLayout()
{
    m_ptvFoil = new QTableView(this);
    m_ptvFoil->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ptvFoil->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ptvFoil->setContextMenuPolicy(Qt::CustomContextMenu);
    m_ptvFoil->setWordWrap(false);
    m_ptvFoil->setFont(DisplayOptions::tableFont());
    m_ptvFoil->horizontalHeader()->setFont(DisplayOptions::tableFont());

    connect(m_ptvFoil, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(onFoilTableCtxMenu(const QPoint &)));

    QHBoxLayout *pMainLayout = new QHBoxLayout;
    pMainLayout->addWidget(m_ptvFoil);
    setLayout(pMainLayout);


    m_pFoilModel = new QStandardItemModel(this);
    m_pFoilModel->setRowCount(10);//temporary
    m_pFoilModel->setColumnCount(15);

    m_pFoilModel->setHeaderData(0,  Qt::Horizontal, tr("Name"));
    m_pFoilModel->setHeaderData(1,  Qt::Horizontal, tr("Thickness (%)"));
    m_pFoilModel->setHeaderData(2,  Qt::Horizontal, tr("at (%)"));
    m_pFoilModel->setHeaderData(3,  Qt::Horizontal, tr("Camber (%)"));
    m_pFoilModel->setHeaderData(4,  Qt::Horizontal, tr("at (%)"));
    m_pFoilModel->setHeaderData(5,  Qt::Horizontal, tr("Points"));
    m_pFoilModel->setHeaderData(6,  Qt::Horizontal, tr("TE Flap (")+QChar(0260)+")");
    m_pFoilModel->setHeaderData(7,  Qt::Horizontal, tr("TE XHinge"));
    m_pFoilModel->setHeaderData(8,  Qt::Horizontal, tr("TE YHinge"));
    m_pFoilModel->setHeaderData(9,  Qt::Horizontal, tr("LE Flap (")+QChar(0260)+")");
    m_pFoilModel->setHeaderData(10, Qt::Horizontal, tr("LE XHinge"));
    m_pFoilModel->setHeaderData(11, Qt::Horizontal, tr("LE YHinge"));
    m_pFoilModel->setHeaderData(12, Qt::Horizontal, tr("Show"));
    m_pFoilModel->setHeaderData(13, Qt::Horizontal, tr("Centerline"));
    m_pFoilModel->setHeaderData(14, Qt::Horizontal, tr("Style"));
    m_ptvFoil->setModel(m_pFoilModel);
    m_ptvFoil->setWindowTitle(tr("Foils"));
    m_ptvFoil->horizontalHeader()->setStretchLastSection(true);

    m_pFoilDelegate = new FoilTableDelegate(this);
    m_pFoilDelegate->m_pAFoil = this;
    m_ptvFoil->setItemDelegate(m_pFoilDelegate);
    m_pFoilDelegate->m_pFoilModel = m_pFoilModel;

    m_ptvFoil->setColumnHidden(9, true);
    m_ptvFoil->setColumnHidden(10, true);
    m_ptvFoil->setColumnHidden(11, true);

    QVector<int> precision(16,2);
    m_pFoilDelegate->m_Precision = precision;
}


/**
 * Selects the specified foil in the table of Foil objects. This will highlight the corresponding row.
 * @param pFoil
 */
void AFoil::selectFoil(Foil* pFoil)
{
    if(pFoil)
    {
        QModelIndex ind;
        QString FoilName;

        for(int i=0; i< m_pFoilModel->rowCount(); i++)
        {
            ind = m_pFoilModel->index(i, 0, QModelIndex());
            FoilName = ind.model()->data(ind, Qt::EditRole).toString();

            if(FoilName == pFoil->name())
            {
                m_ptvFoil->selectRow(i);
                break;
            }
        }
    }
    else
    {
        m_ptvFoil->selectRow(0);
    }
    XDirect::setCurFoil(pFoil);
}


/**
 * Initializes the Foil table, the QWidget and the QAction objects from the data.
 * Selects the current foil in the table
 */
void AFoil::setAFoilParams()
{
    setTableFont();
    fillFoilTable();

    selectFoil(Objects2d::curFoil());
    setControls();
}


/** A signal has been received to update the foil table */
void AFoil::onUpdateFoilTable()
{
    fillFoilTable();
}


void AFoil::onSplinesModified()
{
    fillFoilTable();
    takePicture();
}


/**
 * Turns on or off the display of the current Foil object.
 * @param pFoil a pointer to the Foil object to show
 * @param bShow the new visibility status of the Foil
 */
void AFoil::showFoil(Foil* pFoil, bool bShow)
{
    if(!pFoil) return;
    pFoil->setVisible(bShow);
    emit projectModified();
}


/**
 * Copies the current SplineFoil object to a new SplineFoil object and pushes it on the stack.
 */
void AFoil::takePicture()
{
    //clear the downstream part of the stack which becomes obsolete
    clearStack(m_StackPos);

    // append a copy of the current object
    m_UndoStack.append(SplineFoil(m_pSF));

    // the new current position is the top of the stack
    m_StackPos = m_UndoStack.size()-1;

    m_bStored = true;

    if(s_pMainFrame && s_pMainFrame->m_pUndoAFoilAct && s_pMainFrame->m_pRedoAFoilAct)
    {
        s_pMainFrame->m_pUndoAFoilAct->setEnabled(m_StackPos>0);
        s_pMainFrame->m_pRedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
    }
}


/**
 * Restores a SplineFoil definition from the current position in the stack.
 */
void AFoil::setPicture()
{
    SplineFoil SF = m_UndoStack.at(m_StackPos);
    m_pSF->copy(&SF);
    m_pSF->m_Intrados.splineKnots();
    m_pSF->m_Intrados.splineCurve();
    m_pSF->m_Extrados.splineKnots();
    m_pSF->m_Extrados.splineCurve();
    m_pSF->updateSplineFoil();

    m_p2dWidget->update();
}


/**
 * The user has requested to Undo the last modification to the SplineFoil object.
 */
void AFoil::onUndo()
{
    if(m_StackPos>0)
    {
        m_StackPos--;
        setPicture();
        s_pMainFrame->m_pUndoAFoilAct->setEnabled(m_StackPos>0);
        s_pMainFrame->m_pRedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
    }
    else
    {
        //nothing to restore
    }
}


/**
 *The user has requested a Redo operation after an undo.
 */
void AFoil::onRedo()
{
    if(m_StackPos<m_UndoStack.size()-1)
    {
        m_StackPos++;
        setPicture();
        s_pMainFrame->m_pUndoAFoilAct->setEnabled(m_StackPos>0);
        s_pMainFrame->m_pRedoAFoilAct->setEnabled(m_StackPos<m_UndoStack.size()-1);
    }
}


/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void AFoil::clearStack(int pos)
{
    for(int il=m_UndoStack.size()-1; il>pos; il--)
    {
        m_UndoStack.removeAt(il);     // remove from the stack
    }
    m_StackPos = m_UndoStack.size()-1;
}




/**
 * The user has requested that the width of the columns of the Foil table be reset to default values.
 */
void AFoil::onResetColumnWidths()
{
    int unitwidth = int(double(m_ptvFoil->width())/16.0);
    m_ptvFoil->setColumnWidth(0, 3*unitwidth);
    for(int i=1; i<16; i++) m_ptvFoil->setColumnWidth(i, unitwidth);
    m_ptvFoil->setColumnHidden(9, true);
    m_ptvFoil->setColumnHidden(10, true);
    m_ptvFoil->setColumnHidden(11, true);
}

/**
 * The user has requested the lanuch of the interface to show or hide the columns of the Foil table.
 */
void AFoil::onAFoilTableColumns()
{
    AFoilTableDlg dlg(s_pMainFrame);

    dlg.m_bFoilName    = !m_ptvFoil->isColumnHidden(0);
    dlg.m_bThickness   = !m_ptvFoil->isColumnHidden(1);
    dlg.m_bThicknessAt = !m_ptvFoil->isColumnHidden(2);
    dlg.m_bCamber      = !m_ptvFoil->isColumnHidden(3);
    dlg.m_bCamberAt    = !m_ptvFoil->isColumnHidden(4);
    dlg.m_bPoints      = !m_ptvFoil->isColumnHidden(5);
    dlg.m_bTEFlapAngle = !m_ptvFoil->isColumnHidden(6);
    dlg.m_bTEXHinge    = !m_ptvFoil->isColumnHidden(7);
    dlg.m_bTEYHinge    = !m_ptvFoil->isColumnHidden(8);
    dlg.m_bLEFlapAngle = !m_ptvFoil->isColumnHidden(9);
    dlg.m_bLEXHinge    = !m_ptvFoil->isColumnHidden(10);
    dlg.m_bLEYHinge    = !m_ptvFoil->isColumnHidden(11);

    dlg.initDialog();
    dlg.exec();

    m_ptvFoil->setColumnHidden(0,  !dlg.m_bFoilName);
    m_ptvFoil->setColumnHidden(1,  !dlg.m_bThickness);
    m_ptvFoil->setColumnHidden(2,  !dlg.m_bThicknessAt);
    m_ptvFoil->setColumnHidden(3,  !dlg.m_bCamber);
    m_ptvFoil->setColumnHidden(4,  !dlg.m_bCamberAt);
    m_ptvFoil->setColumnHidden(5,  !dlg.m_bPoints);
    m_ptvFoil->setColumnHidden(6,  !dlg.m_bTEFlapAngle);
    m_ptvFoil->setColumnHidden(7,  !dlg.m_bTEXHinge);
    m_ptvFoil->setColumnHidden(8,  !dlg.m_bTEYHinge);
    m_ptvFoil->setColumnHidden(9,  !dlg.m_bLEFlapAngle);
    m_ptvFoil->setColumnHidden(10, !dlg.m_bLEXHinge);
    m_ptvFoil->setColumnHidden(11, !dlg.m_bLEYHinge);

}



/**
 * The client area has been resized. Update the column widths.
 * @param event the QResizeEvent.
 */
void AFoil::resizeEvent(QResizeEvent *pEvent)
{
    int ncol = m_ptvFoil->horizontalHeader()->count() - m_ptvFoil->horizontalHeader()->hiddenSectionCount();
    //add 1 to get double width for the name
    ncol++;

    //get column width and spare 10% for horizontal header
    int unitwidth = int(double(m_ptvFoil->width())/double(ncol)/1.1);

    m_ptvFoil->setColumnWidth(0, 2*unitwidth);
    for(int i=1; i<16; i++)    m_ptvFoil->setColumnWidth(i, unitwidth);
    pEvent->accept();
}



/** Sets the display font for the Foil table using the default defined in the MainFrame class/ */
void AFoil::setTableFont()
{
    m_ptvFoil->setFont(DisplayOptions::tableFont());
}



Foil* AFoil::addNewFoil(Foil *pFoil)
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
        emit projectModified();
        return pFoil;
    }
    return nullptr;
}


void AFoil::initDialog(FoilDesignWt *p2DWidget, XFoil *pXFoil)
{
    m_pXFoil = pXFoil;
    m_p2dWidget = p2DWidget;
    m_p2dWidget->setObjects(m_pBufferFoil, m_pSF);
}

