/****************************************************************************

    PlaneDlg Class
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


#include <QFileDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QMenu>


#include <xflobjects/editors/wingdlg.h>
#include <xflobjects/editors/planedlg.h>
#include <xfl3d/views/gl3dplaneview.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/editors/editbodydlg.h>
#include <xflobjects/editors/bodydlg.h>
#include <xflobjects/editors/importobjectdlg.h>
#include <xflobjects/editors/inertiadlg.h>
#include <xflobjects/objects3d/objects3d.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/xml/xmlplanereader.h>
#include <xflwidgets/customwts/doubleedit.h>

QByteArray PlaneDlg::s_Geometry;

bool PlaneDlg::s_bOutline(true);
bool PlaneDlg::s_bSurfaces(true);
bool PlaneDlg::s_bVLMPanels(false);
bool PlaneDlg::s_bAxes(true);
bool PlaneDlg::s_bShowMasses(false);
bool PlaneDlg::s_bFoilNames(false);


PlaneDlg::PlaneDlg(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Plane Editor"));
    m_pPlane = nullptr;

    m_bAcceptName         = true;
    m_bChanged            = false;
    m_bDescriptionChanged = false;

    setupLayout();
    connectSignals();
}


void PlaneDlg::connectSignals()
{
    connect(m_pchBiplane,   SIGNAL(clicked()), SLOT(onBiplane()));
    connect(m_pchStabCheck, SIGNAL(clicked()), SLOT(onStab()));
    connect(m_pchFinCheck,  SIGNAL(clicked()), SLOT(onFin()));

    connect(m_pchSymFin,    SIGNAL(clicked()), SLOT(onSymFin()));
    connect(m_pchDoubleFin, SIGNAL(clicked()), SLOT(onDoubleFin()));

    connect(m_ppbDefineWing,  SIGNAL(clicked()), SLOT(onDefineWing()));
    connect(m_ppbImportWing,  SIGNAL(clicked()), SLOT(onImportWing()));
    connect(m_ppbDefineWing2, SIGNAL(clicked()), SLOT(onDefineWing2()));
    connect(m_ppbImportWing2, SIGNAL(clicked()), SLOT(onImportWing2()));
    connect(m_ppbDefineStab,  SIGNAL(clicked()), SLOT(onDefineStab()));
    connect(m_ppbDefineFin,   SIGNAL(clicked()), SLOT(onDefineFin()));

    connect(m_ppbPlaneInertia, SIGNAL(clicked()), SLOT(onInertia()));


    connect(m_pchBody,       SIGNAL(clicked()), SLOT(onBodyCheck()));

    connect(m_ptePlaneDescription, SIGNAL(textChanged()), SLOT(onDescriptionChanged()));

    connect(m_pdeWingTilt,  SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeWingTilt2, SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeStabTilt,  SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeFinTilt,   SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeXLEWing,   SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeZLEWing,   SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeXLEWing2,  SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeZLEWing2,  SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeXLEStab,   SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeZLEStab,   SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeXLEFin,    SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeYLEFin,    SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeZLEFin,    SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeXBody,     SIGNAL(valueChanged()), SLOT(onChanged()));
    connect(m_pdeZBody,     SIGNAL(valueChanged()), SLOT(onChanged()));

    connect(m_pchAxes,       SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onAxes(bool)));
    connect(m_pchPanels,     SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onPanels(bool)));
    connect(m_pchSurfaces,   SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onSurfaces(bool)));
    connect(m_pchOutline,    SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onOutline(bool)));
    connect(m_pchFoilNames,  SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onFoilNames(bool)));
    connect(m_pchShowMasses, SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onShowMasses(bool)));


    connect(m_ppbReset,      SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dReset()));
    connect(m_ptbIso,        SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dIso()));
    connect(m_ptbX,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dFront()));
    connect(m_ptbY,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dLeft()));
    connect(m_ptbZ,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dTop()));
    connect(m_ptbFlip,       SIGNAL(clicked()), m_pglPlaneView, SLOT(on3dFlip()));

    connect(m_pslClipPlanePos, SIGNAL(sliderMoved(int)), m_pglPlaneView, SLOT(onClipPlane(int)));
}



void PlaneDlg::onButton(QAbstractButton *pButton)
{
    if (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)           onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}



void PlaneDlg::initDialog()
{
    m_plePlaneName->setText(m_pPlane->name());

    if(m_pPlane->m_PlaneDescription.length()) m_ptePlaneDescription->setPlainText(m_pPlane->m_PlaneDescription);
    else                                      m_ptePlaneDescription->setPlainText("");

    setParams();
    setResults();

    if(!m_bAcceptName) m_plePlaneName->setEnabled(false);
    m_bChanged = false;

    m_pPlane->m_Wing[0].createSurfaces(m_pPlane->m_WingLE[0],   0.0, m_pPlane->m_WingTiltAngle[0]);//necessary for eventual inertia calculations
    m_pPlane->m_Wing[1].createSurfaces(m_pPlane->m_WingLE[1],   0.0, m_pPlane->m_WingTiltAngle[1]);//necessary for eventual inertia calculations
    m_pPlane->m_Wing[2].createSurfaces(m_pPlane->m_WingLE[2],   0.0, m_pPlane->m_WingTiltAngle[2]);//necessary for eventual inertia calculations
    m_pPlane->m_Wing[3].createSurfaces(m_pPlane->m_WingLE[3], -90.0, m_pPlane->m_WingTiltAngle[3]);//necessary for eventual inertia calculations

    m_pglPlaneView->setPlane(m_pPlane);
}


void PlaneDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}


void PlaneDlg::hideEvent(QHideEvent *)
{
    s_Geometry = saveGeometry();
}



void PlaneDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
            }
            else
            {
                onOK();
             }
            return;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            event->ignore();
    }
}


void PlaneDlg::onBiplane()
{
    m_bChanged = true;
    m_pPlane->m_bBiplane = m_pchBiplane->isChecked();
    if(m_pPlane->wing2())
    {
        m_ppbDefineWing2->setEnabled(true);
        m_ppbImportWing2->setEnabled(true);
        m_pdeWingTilt2->setEnabled(true);
        m_pdeZLEWing2->setEnabled(true);
        m_pdeXLEWing2->setEnabled(true);
    }
    else
    {
        m_ppbDefineWing2->setEnabled(false);
        m_ppbImportWing2->setEnabled(false);
        m_pdeWingTilt2->setEnabled(false);
        m_pdeZLEWing2->setEnabled(false);
        m_pdeXLEWing2->setEnabled(false);
    }
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}



void PlaneDlg::onBodyCheck()
{
    m_bChanged = true;

    if(m_pchBody->isChecked())
    {
        m_pPlane->m_bBody= true;
    }
    else
    {
        m_pPlane->m_bBody= false;
    }

    m_pdeXBody->setEnabled(m_pchBody->isChecked());
    m_pdeZBody->setEnabled(m_pchBody->isChecked());
    m_ppbBodyActions->setEnabled(m_pchBody->isChecked());

    setResults();
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onChanged()
{
    m_bChanged = true;
    readParams();
    setResults();

    m_pPlane->createSurfaces();
    m_pPlane->computePlane();

    m_pglPlaneView->m_bResetglPlane = true;
    m_pglPlaneView->update();
}


void PlaneDlg::onDescriptionChanged()
{
    m_bDescriptionChanged = true;
}


void PlaneDlg::onDefineWing()
{
    Wing *pSaveWing = new Wing();
    pSaveWing->duplicate(m_pPlane->wing());

    WingDlg wingDlg(this);

    wingDlg.m_bAcceptName = true;
    wingDlg.initDialog(m_pPlane->wing());

    if(wingDlg.exec() ==QDialog::Accepted)
    {
        setResults();
        m_bChanged = true;
    }
    else   m_pPlane->wing()->duplicate(pSaveWing);
    m_pPlane->wing()->createSurfaces(m_pPlane->m_WingLE[0], 0.0, m_pPlane->m_WingTiltAngle[0]);//necessary for eventual inertia calculations

    delete pSaveWing;
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDefineFin()
{
    Wing *pSaveWing = new Wing();
    pSaveWing->duplicate(m_pPlane->fin());

    WingDlg wingDlg(this);
    wingDlg.m_bAcceptName = true;
    wingDlg.initDialog(m_pPlane->fin());

    if(wingDlg.exec() ==QDialog::Accepted)
    {
        setResults();
        m_bChanged = true;
    }
    else   m_pPlane->fin()->duplicate(pSaveWing);
    m_pPlane->fin()->createSurfaces(m_pPlane->m_WingLE[3], -90.0, m_pPlane->m_WingTiltAngle[3]);//necessary for eventual inertia calculations

    delete pSaveWing;
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDefineStab()
{
    Wing *pSaveWing = new Wing();
    pSaveWing->duplicate(m_pPlane->stab());

    WingDlg wingDlg(this);
    wingDlg.m_bAcceptName = true;
    wingDlg.initDialog(m_pPlane->stab());

    if(wingDlg.exec() == QDialog::Accepted)
    {
        setResults();
        m_bChanged = true;
    }
    else  m_pPlane->stab()->duplicate(pSaveWing);
    m_pPlane->stab()->createSurfaces(m_pPlane->m_WingLE[2], 0.0, m_pPlane->m_WingTiltAngle[2]);//necessary for eventual inertia calculations

    delete pSaveWing;
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDefineWing2()
{
    Wing *pSaveWing = new Wing();
    pSaveWing->duplicate(m_pPlane->wing2());

    WingDlg wingDlg(this);
    wingDlg.m_bAcceptName = true;
    wingDlg.initDialog(m_pPlane->wing2());

    if(wingDlg.exec() ==QDialog::Accepted)
    {
        setResults();
        m_bChanged = true;
    }
    else   m_pPlane->wing2()->duplicate(pSaveWing);
    m_pPlane->wing2()->createSurfaces(m_pPlane->m_WingLE[1], 0.0, m_pPlane->m_WingTiltAngle[1]);//necessary for eventual inertia calculations

    delete pSaveWing;
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDoubleFin()
{
    if (m_pchDoubleFin->isChecked())
    {
        m_pdeYLEFin->setEnabled(true);
        m_pPlane->setDoubleFin(true);
        m_pPlane->setSymFin(false);
        m_pchSymFin->setChecked(false);
    }
    else
    {
        m_pdeYLEFin->setEnabled(false);
        m_pPlane->setDoubleFin(false);
    }
    m_pPlane->createSurfaces();

    m_bChanged = true;
    setResults();
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDefineBody()
{
    if(!m_pPlane->body()) return;

    Body memBody;
    memBody.duplicate(m_pPlane->body());
    Vector3d v = m_pPlane->bodyPos();
    v.x = -v.x; v.z=-v.z;
    //   m_pPlane->m_pBody->Translate(v,false);
    BodyDlg glbDlg(this);
    glbDlg.m_bEnableName = false;
    glbDlg.initDialog(m_pPlane->body());

    if(glbDlg.exec() == QDialog::Accepted)
    {
        m_bChanged = true;
        setResults();
    }
    else m_pPlane->body()->duplicate(&memBody);

    //   m_pPlane->m_pBody->Translate(m_pPlane->BodyPos(),false);
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


/**
 * The user has requested an edition of the current body
 * Launch the edition interface, and on return, insert the body i.a.w. user instructions
 */
void PlaneDlg::onDefineBodyObject()
{
    if(!m_pPlane->body()) return;

    Body memBody;
    memBody.duplicate(m_pPlane->body());

    EditBodyDlg ebDlg(this);
    ebDlg.initDialog(m_pPlane->body());

    if(ebDlg.exec() == QDialog::Accepted)
    {
        m_bChanged = true;
        setResults();
    }
    else m_pPlane->body()->duplicate(&memBody);
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onFin()
{
    m_bChanged = true;
    if(m_pchFinCheck->isChecked())
    {
        m_pchSymFin->setEnabled(true);
        m_pchDoubleFin->setEnabled(true);
        m_pdeFinTilt->setEnabled(true);
        m_pdeXLEFin->setEnabled(true);
        if (m_pchDoubleFin->isChecked()) m_pdeYLEFin->setEnabled(true);
        else                               m_pdeYLEFin->setEnabled(false);

        m_pdeZLEFin->setEnabled(true);
        m_ppbDefineFin->setEnabled(true);
        m_pPlane->m_bFin = true;
    }
    else
    {
        m_pchSymFin->setEnabled(false);
        m_pchDoubleFin->setEnabled(false);
        m_pdeFinTilt->setEnabled(false);
        m_pdeXLEFin->setEnabled(false);
        m_pdeYLEFin->setEnabled(false);
        m_pdeZLEFin->setEnabled(false);
        m_ppbDefineFin->setEnabled(false);
        m_pPlane->m_bFin = false;
    }
    setResults();
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onImportPlaneBody()
{
    ImportObjectDlg dlg(this);
    if(m_pPlane->body()) dlg.m_ObjectName = m_pPlane->body()->m_Name;
    else                 dlg.m_ObjectName.clear();
    dlg.initDialog(false);

    if(dlg.exec() == QDialog::Accepted)
    {
        Body *pOldBody = Objects3d::getBody(dlg.m_ObjectName);
        if(pOldBody)
        {
            m_pPlane->setBody(pOldBody);
        }
    }
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onDefaultBody()
{
    QString strong = tr("Revert to the default body definition ?");
    if (QMessageBox::Yes != QMessageBox::question(this, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

    Body aNewBody;
    m_pPlane->m_Body.duplicate(&aNewBody);
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
    m_bChanged = true;
}


void PlaneDlg::onImportXMLBody()
{
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
    a_plane.setBody(true);
    XMLPlaneReader planeReader(XFile, &a_plane);
    planeReader.readXMLPlaneFile();

    XFile.close();

    if(planeReader.hasError())
    {
        QString errorMsg = planeReader.errorString() + QString("\nline %1 column %2").arg(planeReader.lineNumber()).arg(planeReader.columnNumber());
        QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
        return;
    }

    m_bChanged = true;
    //    if(m_pPlane->body()) delete m_pPlane->body();

    //    Body *pXMLBody = new Body;
    //    pXMLBody->duplicate(a_plane.body());
    m_pPlane->body()->duplicate(a_plane.body());
    m_pchBody->setChecked(true);
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onImportWing()
{
    ImportObjectDlg dlg(this);
    dlg.m_ObjectName = m_pPlane->name();
    dlg.initDialog(true);

    if(dlg.exec() == QDialog::Accepted)
    {
        m_bChanged = true;
        dlg.m_ObjectName.replace("/Main wing","");
        Wing *pWing = Objects3d::getWing(dlg.m_ObjectName);
        if(pWing)
        {
            m_pPlane->wing()->duplicate(pWing);
            m_pPlane->wing()->setColor(pWing->color());
        }
    }
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onImportWing2()
{
    ImportObjectDlg dlg(this);
    dlg.m_ObjectName = m_pPlane->wing()->m_Name;
    dlg.initDialog(true);

    if(dlg.exec() == QDialog::Accepted)
    {
        m_bChanged = true;
        Wing *pWing = Objects3d::getWing(dlg.m_ObjectName);
        if(pWing)
        {
            m_pPlane->wing2()->duplicate(pWing);
            m_pPlane->wing2()->setColor(pWing->color());
        }
    }
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onInertia()
{
    if(!m_pPlane) return;
    readParams();

    m_pPlane->createSurfaces();//necessary for inertia calculations

    InertiaDlg dlg(this);
    dlg.m_pBody = nullptr;
    dlg.m_pWing = nullptr;
    dlg.m_pPlane = m_pPlane;

    //save inertia properties
    QVector<PointMass> memPtMass;
    memPtMass.clear();
    for(int im=0; im<m_pPlane->m_PointMass.size(); im++)
    {
        memPtMass.append(m_pPlane->m_PointMass.at(im));
    }

    dlg.initDialog();
    if(dlg.exec()==QDialog::Accepted)
    {
        if(dlg.m_bChanged) m_bChanged = true;
    }
    else
    {
        //restore everything
        m_pPlane->clearPointMasses();
        for(int im=0; im<memPtMass.size(); im++)
        {
            m_pPlane->m_PointMass.append(memPtMass.at(im));
        }
    }
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onPlaneName()
{
    m_pPlane->setName(m_plePlaneName->text());
}


void PlaneDlg::onOK()
{
    readParams();

    m_pPlane->m_PlaneDescription = m_ptePlaneDescription->toPlainText();

    m_pPlane->computePlane();

    int nstations = m_pPlane->spanStationCount();
    if(nstations>MAXSPANSTATIONS)
    {
        QString strange;
        strange = QString::asprintf("The total number of span stations is %3d. Cannot exceed %3d. \n Please reduce the number of panels in the Y direction.",  nstations, MAXSPANSTATIONS);
        QMessageBox::warning(this, tr("Warning"), strange);
        return;
    }

    m_pPlane->computeBodyAxisInertia();

    m_pPlane->m_bBody = m_pchBody->isChecked();

    accept();
}


void PlaneDlg::onStab()
{
    m_bChanged = true;
    if(m_pchStabCheck->isChecked())
    {
        m_ppbDefineStab->setEnabled(true);
        m_pdeXLEStab->setEnabled(true);
        m_pdeZLEStab->setEnabled(true);
        m_pdeStabTilt->setEnabled(true);
        m_pPlane->m_bStab = true;
    }
    else
    {
        m_ppbDefineStab->setEnabled(false);
        m_pdeXLEStab->setEnabled(false);
        m_pdeZLEStab->setEnabled(false);
        m_pdeStabTilt->setEnabled(false);
        m_pPlane->m_bStab = false;
    }
    setResults();
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::onSymFin()
{    
    if (m_pchSymFin->isChecked())
    {
        m_pdeYLEFin->setEnabled(false);
        m_pPlane->setSymFin(true);
        m_pPlane->setDoubleFin(false);
        m_pchDoubleFin->setChecked(false);
        m_pdeYLEFin->setEnabled(false);
    }
    else
    {
        m_pdeYLEFin->setEnabled(true);
        m_pPlane->setSymFin(false);
    }

    m_pPlane->createSurfaces();

    m_bChanged = true;
    setResults();
    m_pglPlaneView->resetPlane();
    m_pglPlaneView->update();
}


void PlaneDlg::readParams()
{
    onPlaneName();
    m_pPlane->m_WingTiltAngle[0] = m_pdeWingTilt->value();
    m_pPlane->m_WingTiltAngle[1] = m_pdeWingTilt2->value();
    m_pPlane->m_WingTiltAngle[2] = m_pdeStabTilt->value();
    m_pPlane->m_WingTiltAngle[3] = m_pdeFinTilt->value();

    m_pPlane->m_WingLE[0].x = m_pdeXLEWing->value() / Units::mtoUnit();
    m_pPlane->m_WingLE[0].z = m_pdeZLEWing->value() / Units::mtoUnit();

    m_pPlane->m_WingLE[1].x = m_pdeXLEWing2->value() / Units::mtoUnit();
    m_pPlane->m_WingLE[1].z = m_pdeZLEWing2->value() / Units::mtoUnit();

    m_pPlane->m_WingLE[2].x = m_pdeXLEStab->value() / Units::mtoUnit();
    m_pPlane->m_WingLE[2].z = m_pdeZLEStab->value() / Units::mtoUnit();

    m_pPlane->m_WingLE[3].x = m_pdeXLEFin->value() / Units::mtoUnit();
    m_pPlane->m_WingLE[3].y = m_pdeYLEFin->value() / Units::mtoUnit();
    m_pPlane->m_WingLE[3].z = m_pdeZLEFin->value() / Units::mtoUnit();

    m_pPlane->m_BodyPos.x = m_pdeXBody->value() / Units::mtoUnit();
    m_pPlane->m_BodyPos.z = m_pdeZBody->value() / Units::mtoUnit();

    if(m_pchBiplane->isChecked())   m_pPlane->m_bBiplane = true;
    else                              m_pPlane->m_bBiplane = false;
    if(m_pchStabCheck->isChecked()) m_pPlane->m_bStab = true;
    else                              m_pPlane->m_bStab = false;
    if(m_pchFinCheck->isChecked())  m_pPlane->m_bFin  = true;
    else                              m_pPlane->m_bFin  = false;

}


void PlaneDlg::reject()
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
    //    reject();
    done(QDialog::Rejected);
}


void PlaneDlg::setParams()
{
    if(m_pPlane->body()) m_pchBody->setChecked(true);
    else                 m_pchBody->setChecked(false);

    m_pchBody->setEnabled(true);
    m_pdeXBody->setEnabled(m_pPlane->m_bBody);
    m_pdeZBody->setEnabled(m_pPlane->m_bBody);
    m_ppbBodyActions->setEnabled(m_pPlane->m_bBody);

    m_plePlaneName->setText(m_pPlane->name());
    m_pdeWingTilt->setValue(m_pPlane->m_WingTiltAngle[0]);
    m_pdeWingTilt2->setValue(m_pPlane->m_WingTiltAngle[1]);
    m_pdeStabTilt->setValue(m_pPlane->m_WingTiltAngle[2]);
    m_pdeFinTilt->setValue(m_pPlane->m_WingTiltAngle[3]);

    m_pdeXLEWing->setValue(m_pPlane->m_WingLE[0].x * Units::mtoUnit());
    m_pdeZLEWing->setValue(m_pPlane->m_WingLE[0].z * Units::mtoUnit());

    m_pdeXLEWing2->setValue(m_pPlane->m_WingLE[1].x * Units::mtoUnit());
    m_pdeZLEWing2->setValue(m_pPlane->m_WingLE[1].z * Units::mtoUnit());

    m_pdeXLEStab->setValue(m_pPlane->m_WingLE[2].x * Units::mtoUnit());
    m_pdeZLEStab->setValue(m_pPlane->m_WingLE[2].z * Units::mtoUnit());

    m_pdeXBody->setValue(m_pPlane->m_BodyPos.x * Units::mtoUnit());
    m_pdeZBody->setValue(m_pPlane->m_BodyPos.z * Units::mtoUnit());

    m_pchBiplane->setChecked(m_pPlane->wing2());
    onBiplane();

    m_pdeXLEFin->setValue(m_pPlane->m_WingLE[3].x* Units::mtoUnit());
    m_pdeYLEFin->setValue(m_pPlane->m_WingLE[3].y* Units::mtoUnit());
    m_pdeZLEFin->setValue(m_pPlane->m_WingLE[3].z* Units::mtoUnit());
    m_pchFinCheck->setChecked(m_pPlane->m_bFin);
    m_pchDoubleFin->setChecked(m_pPlane->bDoubleFin());
    m_pchSymFin->setChecked(m_pPlane->bSymFin());
    onFin();
    m_pchStabCheck->setChecked(m_pPlane->stab());
    onStab();
}


void PlaneDlg::setResults()
{
    QString str;

    //    double area = m_pPlane->Wing()->s_Area;
    //    if(m_pPlane->m_bBiplane) area += m_pPlane->Wing2()->m_Area;

    str = QString("%1").arg(m_pPlane->wing()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
    m_plabWingSurface->setText(str);

    if(m_pPlane->stab())   str = QString("%1").arg(m_pPlane->stab()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
    else str = " ";
    m_plabStabSurface->setText(str);

    if(m_pPlane->fin())    str = QString("%1").arg(m_pPlane->fin()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
    else str=" ";
    m_plabFinSurface->setText(str);

    double span = m_pPlane->wing()->m_PlanformSpan;
    if(m_pPlane->wing2()) span = qMax(m_pPlane->wing()->m_PlanformSpan, m_pPlane->wing2()->m_PlanformSpan);
    str = QString("%1").arg(span*Units::mtoUnit(),5,'f',2);
    m_plabWingSpan->setText(str);

    m_pPlane->computePlane();
    if(m_pPlane->stab())
    {
        double SLA = m_pPlane->m_WingLE[2].x + m_pPlane->stab()->Chord(0)/4.0 - (m_pPlane->m_WingLE[0].x + m_pPlane->wing()->Chord(0)/4.0);
        str = QString("%1").arg(SLA*Units::mtoUnit(),5,'f',2);
    }
    else  str=" ";
    m_plabStabLeverArm->setText(str);

    if(m_pPlane->stab())
    {
        str = QString("%1").arg(m_pPlane->tailVolume(),5,'f',2);
    }
    else str =" ";
    m_plabStabVolume->setText(str);


    str = QString("%1").arg(m_pPlane->VLMPanelTotal());
    m_plabVLMTotalPanels->setText(str);
}



void PlaneDlg::setupLayout()
{
    QString len  = Units::lengthUnitLabel();
    QString surf = Units::areaUnitLabel();
    QSizePolicy szPolicyMaximum(QSizePolicy::Maximum, QSizePolicy::Maximum);


    QHBoxLayout *pMainLayout = new QHBoxLayout;
    {
        QFrame *pLeftFrame = new QFrame;
        {
            QGridLayout *pLeftLayout = new QGridLayout;
            {
                QGroupBox *pNameBox = new QGroupBox(tr("Plane Description"));
                {
                    QVBoxLayout *pNameLayout = new QVBoxLayout;
                    {
                        m_plePlaneName = new QLineEdit(tr("Plane Name"));
                        m_ptePlaneDescription = new QTextEdit();
                        m_ptePlaneDescription->setToolTip(tr("Enter here a short description for the plane"));
                        QLabel *PlaneDescription = new QLabel(tr("Description:"));
                        m_ppbPlaneInertia = new QPushButton(tr("Plane Inertia"));
                        pNameLayout->addWidget(m_plePlaneName);
                        pNameLayout->addWidget(PlaneDescription);
                        pNameLayout->addWidget(m_ptePlaneDescription);
                        pNameLayout->addWidget(m_ppbPlaneInertia);
                    }
                    pNameBox->setLayout(pNameLayout);
                }

                QGroupBox *pMainWingBox = new QGroupBox(tr("Main Wing"));
                {
                    QGridLayout *pMainWingLayout = new QGridLayout;
                    {
                        QCheckBox *pMainWing = new QCheckBox(tr("Main wing"));
                        pMainWing->setChecked(true);
                        pMainWing->setEnabled(false);
                        pMainWingLayout->addWidget(pMainWing,1,1);
                        m_ppbDefineWing = new QPushButton(tr("Define"));
                        m_ppbImportWing = new QPushButton(tr("Import"));
                        QLabel *lab1 = new QLabel(tr("x="));
                        QLabel *lab2 = new QLabel(tr("z="));
                        QLabel *lab3 = new QLabel(tr("Tilt Angle="));
                        lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        m_pdeXLEWing = new DoubleEdit(0.0, 3);
                        m_pdeZLEWing = new DoubleEdit(0.0, 3);
                        m_pdeWingTilt = new DoubleEdit(0.0, 3);
                        QLabel *plabLen1 = new QLabel(len);
                        QLabel *plabLen2 = new QLabel(len);
                        QLabel *lab4 = new QLabel(QChar(0260));
                        pMainWingLayout->addWidget(m_ppbDefineWing, 2,1);
                        pMainWingLayout->addWidget(m_ppbImportWing, 3,1);
                        pMainWingLayout->addWidget(lab1,2,2);
                        pMainWingLayout->addWidget(lab2,3,2);
                        pMainWingLayout->addWidget(lab3,4,2);
                        pMainWingLayout->addWidget(m_pdeXLEWing,2,3);
                        pMainWingLayout->addWidget(m_pdeZLEWing,3,3);
                        pMainWingLayout->addWidget(m_pdeWingTilt,4,3);
                        pMainWingLayout->addWidget(plabLen1,2,4);
                        pMainWingLayout->addWidget(plabLen2,3,4);
                        pMainWingLayout->addWidget(lab4,4,4);
                        pMainWingLayout->setRowStretch(5,1);
                    }
                    pMainWingBox->setLayout(pMainWingLayout);
                }

                QGroupBox *pMainWing2Box = new QGroupBox(tr("Wing 2"));
                {
                    QGridLayout *pMainWing2Layout = new QGridLayout;
                    {
                        m_pchBiplane = new QCheckBox(tr("Biplane"));
                        pMainWing2Layout->addWidget(m_pchBiplane,1,1);
                        m_ppbDefineWing2 = new QPushButton(tr("Define"));
                        m_ppbImportWing2 = new QPushButton(tr("Import"));
                        QLabel *lab11 = new QLabel(tr("x="));
                        QLabel *lab12 = new QLabel(tr("z="));
                        QLabel *lab13 = new QLabel(tr("Tilt Angle="));
                        lab11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        lab12->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        lab13->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        m_pdeXLEWing2 = new DoubleEdit(0.0, 3);
                        m_pdeZLEWing2 = new DoubleEdit(0.0, 3);
                        m_pdeWingTilt2 = new DoubleEdit(0.0, 3);
                        QLabel *plabLen3 = new QLabel(len);
                        QLabel *plabLen4 = new QLabel(len);
                        QLabel *lab14 = new QLabel(QChar(0260));
                        pMainWing2Layout->addWidget(m_ppbDefineWing2, 2,1);
                        pMainWing2Layout->addWidget(m_ppbImportWing2, 3,1);
                        pMainWing2Layout->addWidget(lab11,2,2);
                        pMainWing2Layout->addWidget(lab12,3,2);
                        pMainWing2Layout->addWidget(lab13,4,2);
                        pMainWing2Layout->addWidget(m_pdeXLEWing2,2,3);
                        pMainWing2Layout->addWidget(m_pdeZLEWing2,3,3);
                        pMainWing2Layout->addWidget(m_pdeWingTilt2,4,3);
                        pMainWing2Layout->addWidget(plabLen3,2,4);
                        pMainWing2Layout->addWidget(plabLen4,3,4);
                        pMainWing2Layout->addWidget(lab14,4,4);
                    }
                    pMainWing2Box->setLayout(pMainWing2Layout);
                }

                QGroupBox *pStabBox = new QGroupBox(tr("Elevator"));
                {
                    QGridLayout *pStabLayout = new QGridLayout;
                    {
                        m_pchStabCheck = new QCheckBox(tr("Elevator"));
                        m_ppbDefineStab = new QPushButton(tr("Define"));
                        QLabel *plab21 = new QLabel(tr("x="));
                        QLabel *plab22 = new QLabel(tr("z="));
                        QLabel *plab23 = new QLabel(tr("Tilt Angle="));
                        plab21->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        plab22->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        plab23->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        m_pdeXLEStab = new DoubleEdit(550.0, 3);
                        m_pdeZLEStab = new DoubleEdit(550.0, 3);
                        m_pdeStabTilt = new DoubleEdit(0.0, 3);
                        QLabel *plabLen5 = new QLabel(len);
                        QLabel *plabLen6 = new QLabel(len);
                        QLabel *lab24 = new QLabel(QChar(0260));
                        pStabLayout->addWidget(m_pchStabCheck,1,1);
                        pStabLayout->addWidget(m_ppbDefineStab, 2,1);
                        pStabLayout->addWidget(plab21,2,2);
                        pStabLayout->addWidget(plab22,4,2);
                        pStabLayout->addWidget(plab23,5,2);
                        pStabLayout->addWidget(m_pdeXLEStab,2,3);
                        pStabLayout->addWidget(m_pdeZLEStab,4,3);
                        pStabLayout->addWidget(m_pdeStabTilt,5,3);
                        pStabLayout->addWidget(plabLen5,2,4);
                        pStabLayout->addWidget(plabLen6,4,4);
                        pStabLayout->addWidget(lab24,5,4);
                        pStabLayout->setRowStretch(6,1);
                    }
                    pStabBox->setLayout(pStabLayout);
                }

                QGroupBox *pFinBox = new QGroupBox(tr("Fin"));
                {
                    QGridLayout *pFinLayout = new QGridLayout;
                    {
                        m_pchFinCheck = new QCheckBox(tr("Fin"));
                        m_ppbDefineFin = new QPushButton(tr("Define"));
                        m_pchDoubleFin = new QCheckBox(tr("Double Fin"));
                        m_pchSymFin = new QCheckBox(tr("Two-sided Fin"));
                        QLabel *plab31 = new QLabel(tr("x="));
                        QLabel *plab32 = new QLabel(tr("y="));
                        QLabel *plab33 = new QLabel(tr("z="));
                        QLabel *plab34 = new QLabel(tr("Tilt Angle="));
                        plab31->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        plab32->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        plab33->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        plab34->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                        m_pdeXLEFin = new DoubleEdit(600.0, 3);
                        m_pdeYLEFin = new DoubleEdit(0.0, 3);
                        m_pdeZLEFin = new DoubleEdit(50.0, 3);
                        m_pdeFinTilt = new DoubleEdit(0.0, 3);
                        QLabel *plabLen7 = new QLabel(len);
                        QLabel *pLabLen8 = new QLabel(len);
                        QLabel *pLabLen9 = new QLabel(len);
                        QLabel *lab35 = new QLabel(QChar(0260));
                        pFinLayout->addWidget(m_pchFinCheck,1,1);
                        pFinLayout->addWidget(m_ppbDefineFin, 2,1);
                        pFinLayout->addWidget(m_pchSymFin, 3,1);
                        pFinLayout->addWidget(m_pchDoubleFin, 4,1);
                        pFinLayout->addWidget(plab31,2,2);
                        pFinLayout->addWidget(plab32,3,2);
                        pFinLayout->addWidget(plab33,4,2);
                        pFinLayout->addWidget(plab34,5,2);
                        pFinLayout->addWidget(m_pdeXLEFin,2,3);
                        pFinLayout->addWidget(m_pdeYLEFin,3,3);
                        pFinLayout->addWidget(m_pdeZLEFin,4,3);
                        pFinLayout->addWidget(m_pdeFinTilt,5,3);
                        pFinLayout->addWidget(plabLen7,2,4);
                        pFinLayout->addWidget(pLabLen8,3,4);
                        pFinLayout->addWidget(pLabLen9,4,4);
                        pFinLayout->addWidget(lab35,5,4);
                    }
                    pFinBox->setLayout(pFinLayout);
                }

                QGroupBox *pBodyBox = new QGroupBox(tr("Body"));
                {
                    QHBoxLayout *pBodyNameLayout = new QHBoxLayout;
                    {
                        m_pchBody = new QCheckBox(tr("Body"));

                        QAction *pDefineBody= new QAction(tr("Edit"), this);
                        connect(pDefineBody, SIGNAL(triggered()), this, SLOT(onDefineBody()));

                        QAction *pDefineBodyObject= new QAction(tr("Define (Advanced users)"), this);
                        connect(pDefineBodyObject, SIGNAL(triggered()), this, SLOT(onDefineBodyObject()));

                        QAction *pDefaultBody= new QAction(tr("Reset default"), this);
                        connect(pDefaultBody, SIGNAL(triggered()), this, SLOT(onDefaultBody()));

                        QAction *pImportXMLBody= new QAction(tr("Import body definition from an XML file"), this);
                        connect(pImportXMLBody, SIGNAL(triggered()), this, SLOT(onImportXMLBody()));

                        QAction *pImportPlaneBody= new QAction(tr("Import body definition from another plane"), this);
                        connect(pImportPlaneBody, SIGNAL(triggered()), this, SLOT(onImportPlaneBody()));


                        m_ppbBodyActions = new QPushButton(tr("Actions..."));

                        QMenu *pBodyMenu = new QMenu(tr("Actions..."),this);
                        pBodyMenu->addAction(pDefineBody);
                        pBodyMenu->addAction(pDefineBodyObject);
                        pBodyMenu->addAction(pDefaultBody);
                        pBodyMenu->addAction(pImportXMLBody);
                        pBodyMenu->addAction(pImportPlaneBody);
                        m_ppbBodyActions->setMenu(pBodyMenu);

                        pBodyNameLayout->addWidget(m_pchBody);
                        pBodyNameLayout->addWidget(m_ppbBodyActions);
                        pBodyNameLayout->addStretch(1);
                    }
                    QGridLayout *pBodyPos = new QGridLayout;
                    {
                        pBodyPos->setColumnStretch(0,3);
                        pBodyPos->setColumnStretch(1,0);
                        pBodyPos->setColumnStretch(2,0);
                        m_pdeXBody = new DoubleEdit(0.00);
                        m_pdeZBody = new DoubleEdit(0.00);
                        QLabel *lab41 = new QLabel(tr("x="));
                        QLabel *lab42 = new QLabel(tr("z="));
                        QLabel *pLabLen10 = new QLabel(len);
                        QLabel *pLabLen11 = new QLabel(len);
                        pBodyPos->addWidget(lab41,       1,1);
                        pBodyPos->addWidget(m_pdeXBody,  1,2);
                        pBodyPos->addWidget(pLabLen10,   1,3);
                        pBodyPos->addWidget(lab42,       2,1);
                        pBodyPos->addWidget(m_pdeZBody,  2,2);
                        pBodyPos->addWidget(pLabLen11,   2,3);
                    }
                    QVBoxLayout *pBodyLayout = new QVBoxLayout;
                    {
                        QLabel *BodyWarning = new QLabel(tr("Warning:\nIncluding the body in the analysis is not recommended.\nCheck the guidelines for explanations."));
                        pBodyLayout->addWidget(BodyWarning);
                        pBodyLayout->addLayout(pBodyNameLayout);
                        pBodyLayout->addLayout(pBodyPos);
                        pBodyLayout->addStretch(1);
                    }
                    pBodyBox->setLayout(pBodyLayout);
                }

                QGridLayout *pData1Layout = new QGridLayout;
                {
                    QLabel *plab101 = new QLabel(tr("Wing Area = "));
                    QLabel *plab102 = new QLabel(tr("Wing Span = "));
                    QLabel *plab103 = new QLabel(tr("Elev. Area = "));
                    QLabel *plab104 = new QLabel(tr("Elev. Lever Arm = "));
                    plab101->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    plab102->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    plab103->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    plab104->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabWingSurface    = new QLabel("1.00");
                    m_plabStabSurface    = new QLabel("2.00");
                    m_plabWingSpan       = new QLabel("3.00");
                    m_plabStabLeverArm   = new QLabel("4.00");
                    m_plabFinSurface     = new QLabel("5.00");
                    m_plabStabVolume     = new QLabel("6.00");
                    m_plabVLMTotalPanels = new QLabel("1234");
                    m_plabWingSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabStabSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabWingSpan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabStabLeverArm->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

                    QLabel *plabSurf1 = new QLabel(surf);
                    QLabel *plabSurf2 = new QLabel(surf);
                    QLabel *pLabLen12 = new QLabel(len, this);
                    QLabel *pLabLen13 = new QLabel(len, this);

                    pData1Layout->addWidget(plab101,              1, 1);
                    pData1Layout->addWidget(m_plabWingSurface,   1, 2);
                    pData1Layout->addWidget(plabSurf1,           1, 3);
                    pData1Layout->addWidget(plab102,              2, 1);
                    pData1Layout->addWidget(m_plabWingSpan,      2, 2);
                    pData1Layout->addWidget(pLabLen12,           2, 3);
                    pData1Layout->addWidget(plab103,              3, 1);
                    pData1Layout->addWidget(m_plabStabSurface,   3, 2);
                    pData1Layout->addWidget(plabSurf2,           3, 3);
                    pData1Layout->addWidget(plab104,              4, 1);
                    pData1Layout->addWidget(m_plabStabLeverArm,  4, 2);
                    pData1Layout->addWidget(pLabLen13,           4, 3);
                }

                QGridLayout *pData2Layout = new QGridLayout;
                {
                    QLabel *plabSurf3 = new QLabel(surf);
                    QLabel *plab105 = new QLabel(tr("Fin Area = "));
                    QLabel *plab106 = new QLabel(tr("TailVolume = "));
                    QLabel *plab108 = new QLabel(tr("Total Panels = "));
                    plab105->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    plab106->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    plab108->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabFinSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabStabVolume->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    m_plabVLMTotalPanels->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    pData2Layout->addWidget(plab105,               1, 1);
                    pData2Layout->addWidget(m_plabFinSurface,      1, 2);
                    pData2Layout->addWidget(plabSurf3,             1, 3);
                    pData2Layout->addWidget(plab106,               2, 1);
                    pData2Layout->addWidget(m_plabStabVolume,      2, 2);
                    pData2Layout->addWidget(plab108,               3, 1);
                    pData2Layout->addWidget(m_plabVLMTotalPanels,  3, 2);
                }

                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
                {
                    connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
                }

                pLeftLayout->addWidget(pNameBox,      1,1);
                pLeftLayout->addWidget(pBodyBox,      1,2);
                pLeftLayout->addWidget(pMainWingBox,  2,1);
                pLeftLayout->addWidget(pMainWing2Box, 2,2);
                pLeftLayout->addWidget(pStabBox,      3,1);
                pLeftLayout->addWidget(pFinBox,       3,2);
                pLeftLayout->addLayout(pData1Layout,  4,1);
                pLeftLayout->addLayout(pData2Layout,  4,2);
                pLeftLayout->setRowStretch(5,1);
                pLeftLayout->addWidget(m_pButtonBox,  6,1,1,2);
            }
            pLeftFrame->setLayout(pLeftLayout);
        }

        QFrame *pViewFrame = new QFrame;
        {
            QVBoxLayout *pViewLayout = new QVBoxLayout;
            {
                m_pglPlaneView = new gl3dPlaneView(this);
                m_pglPlaneView->m_bOutline    = s_bOutline;
                m_pglPlaneView->m_bSurfaces   = s_bSurfaces;
                m_pglPlaneView->m_bVLMPanels  = s_bVLMPanels;
                m_pglPlaneView->m_bAxes       = s_bAxes;
                m_pglPlaneView->m_bShowMasses = s_bShowMasses;
                m_pglPlaneView->m_bFoilNames  = s_bFoilNames;

                QWidget *p3DCtrlBox = new QWidget;
                {
                    p3DCtrlBox->setSizePolicy(szPolicyMaximum);
                    QHBoxLayout *pThreeDViewControlsLayout = new QHBoxLayout;
                    {
                        QGridLayout *pThreeDParamsLayout = new QGridLayout;
                        {
                            m_pchAxes         = new QCheckBox(tr("Axes"),       this);
                            m_pchSurfaces     = new QCheckBox(tr("Surfaces"),   this);
                            m_pchOutline      = new QCheckBox(tr("Outline"),    this);
                            m_pchPanels       = new QCheckBox(tr("Panels"),     this);
                            m_pchFoilNames    = new QCheckBox(tr("Foil Names"), this);
                            m_pchShowMasses   = new QCheckBox(tr("Masses"),     this);

                            m_pchAxes->setChecked(      s_bAxes);
                            m_pchSurfaces->setChecked(  s_bSurfaces);
                            m_pchOutline->setChecked(   s_bOutline);
                            m_pchPanels->setChecked(    s_bVLMPanels);
                            m_pchFoilNames->setChecked( s_bFoilNames);
                            m_pchShowMasses->setChecked(s_bShowMasses);

                            m_pchAxes->setSizePolicy(szPolicyMaximum);
                            m_pchSurfaces->setSizePolicy(szPolicyMaximum);
                            m_pchOutline->setSizePolicy(szPolicyMaximum);
                            m_pchPanels->setSizePolicy(szPolicyMaximum);
                            m_pchFoilNames->setSizePolicy(szPolicyMaximum);
                            m_pchShowMasses->setSizePolicy(szPolicyMaximum);

                            pThreeDParamsLayout->addWidget(m_pchAxes,       1,1);
                            pThreeDParamsLayout->addWidget(m_pchPanels,     1,2);
                            pThreeDParamsLayout->addWidget(m_pchSurfaces,   1,3);
                            pThreeDParamsLayout->addWidget(m_pchOutline,    2,1);
                            pThreeDParamsLayout->addWidget(m_pchFoilNames,  2,2);
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
                            m_pFlipView->setCheckable(false);

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
                        QVBoxLayout *pRightColLayout = new QVBoxLayout;
                        {
                            m_ppbReset = new QPushButton(tr("Reset scale"));
                            pRightColLayout->addWidget(m_ppbReset);
                            QHBoxLayout *pClipLayout = new QHBoxLayout;
                            {
                                QLabel *ClipLabel = new QLabel(tr("Clip:"));
                                m_pslClipPlanePos = new QSlider(Qt::Horizontal);
                                m_pslClipPlanePos->setMinimum(-100);
                                m_pslClipPlanePos->setMaximum(100);
                                m_pslClipPlanePos->setSliderPosition(0);
                                m_pslClipPlanePos->setTickInterval(10);
                                m_pslClipPlanePos->setTickPosition(QSlider::TicksBelow);
                                pClipLayout->addWidget(ClipLabel);
                                pClipLayout->addWidget(m_pslClipPlanePos,1);
                            }

                            pRightColLayout->addLayout(pClipLayout);
                        }

                        pThreeDViewControlsLayout->addLayout(pThreeDParamsLayout);
                        pThreeDViewControlsLayout->addStretch();
                        pThreeDViewControlsLayout->addLayout(pAxisViewLayout);
                        pThreeDViewControlsLayout->addStretch();
                        pThreeDViewControlsLayout->addLayout(pRightColLayout);

                    }
                    p3DCtrlBox->setLayout(pThreeDViewControlsLayout);
                }

                pViewLayout->addWidget(m_pglPlaneView);
                pViewLayout->addWidget(p3DCtrlBox);
                pViewLayout->setStretchFactor(m_pglPlaneView, 1);
            }
            pViewFrame->setLayout(pViewLayout);
        }
        pMainLayout->addWidget(pLeftFrame);
        pMainLayout->addWidget(pViewFrame);

        pMainLayout->setStretchFactor(pViewFrame, 1);
    }

    setLayout(pMainLayout);
}






