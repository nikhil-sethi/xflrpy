/****************************************************************************

    InertiaDlg Class
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

//performs an automatic evaluation of the object's CoG and inertia properties

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTextStream>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QDialogButtonBox>


#include "inertiadlg.h"

#include <xflcore/displayoptions.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/plane.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/customwts/floateditdelegate.h>
#include <xflwidgets/customwts/cptableview.h>

QByteArray InertiaDlg::s_Geometry;


InertiaDlg::InertiaDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Inertia Properties"));

    m_pPlane = nullptr;
    m_pWing = nullptr;
    m_pBody = nullptr;

    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;

    m_VolumeMass = 0.0;

    clearPointMasses();

    m_bChanged = false;

    m_pInsertMassRow  = new QAction(tr("Insert Before"), this);
    m_pDeleteMassRow = new QAction(tr("Delete"), this);

    m_pContextMenu = new QMenu(tr("Point Mass"),this);
    m_pContextMenu->addAction(m_pInsertMassRow);
    m_pContextMenu->addAction(m_pDeleteMassRow);

    setupLayout();
}


InertiaDlg::~InertiaDlg()
{
    clearPointMasses();
    delete m_pMassModel;
}


void InertiaDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)    onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (pButton==m_ppbExportToAVL) onExportToAVL();
}


void InertiaDlg::computeBodyAxisInertia()
{
    if     (m_pPlane) m_pPlane->computeBodyAxisInertia();
    else if(m_pWing)  m_pWing->computeBodyAxisInertia();
    else if(m_pBody)  m_pBody->computeBodyAxisInertia();
}


/**
* Computes the inertia in the frame of reference with origin at the CoG.
*
* Assumes that the data has been read.
*/
void InertiaDlg::computeInertia()
{
    double TotalMass(0), TotalIxx(0), TotalIyy(0), TotalIzz(0), TotalIxz(0);
    Vector3d TotalCoG, MassPos;

    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;
    m_VolumeCoG.set(0.0, 0.0, 0.0);

    Wing const *pWing[]{nullptr,nullptr,nullptr,nullptr};

    if(m_pPlane)
    {
        pWing[0] =  m_pPlane->wing();
        if(m_pPlane->biPlane()) pWing[1] = m_pPlane->wing2();
        if(m_pPlane->stab())    pWing[2] = m_pPlane->stab();
        if(m_pPlane->fin())     pWing[3] = m_pPlane->fin();
    }
    else if(m_pWing)
    {
        pWing[0] = m_pWing;
    }

    // First evaluate the object's volume inertia, i.e. without point masses,
    // in the frame of reference with origin at the object's self CoG
    if(m_pWing)
    {
        m_pWing->m_VolumeMass = m_VolumeMass;
        if(m_pWing->m_VolumeMass>PRECISION)
        {
//            m_pWing->computeGeometry();
            m_pWing->computeVolumeInertia(m_VolumeCoG, m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz);
        }
    }
    else if(m_pBody)
    {
        m_pBody->m_VolumeMass = m_VolumeMass;
        if(m_pBody->m_VolumeMass>PRECISION) m_pBody->computeVolumeInertia(m_VolumeCoG, m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz);
    }
    else if(m_pPlane)
    {
        m_pPlane->computeVolumeInertia(m_VolumeMass, m_VolumeCoG, m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz);
    }

    // and display the results
    m_pdeXCoG->setValue(m_VolumeCoG.x*Units::mtoUnit());
    m_pdeYCoG->setValue(m_VolumeCoG.y*Units::mtoUnit());
    m_pdeZCoG->setValue(m_VolumeCoG.z*Units::mtoUnit());

    m_pdeCoGIxx->setValue(m_CoGIxx*Units::kgm2toUnit());
    m_pdeCoGIyy->setValue(m_CoGIyy*Units::kgm2toUnit());
    m_pdeCoGIzz->setValue(m_CoGIzz*Units::kgm2toUnit());
    m_pdeCoGIxz->setValue(m_CoGIxz*Units::kgm2toUnit());

    // take into account all point masses to calculate the total CoG and total mass
    TotalCoG.set(m_VolumeMass*m_VolumeCoG.x, m_VolumeMass*m_VolumeCoG.y, m_VolumeMass*m_VolumeCoG.z);
    TotalMass = m_VolumeMass;

    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        TotalMass += pm.mass();
        TotalCoG  += pm.position() * pm.mass();
    }

    if(m_pPlane)
    {
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                for(int im=0; im<pWing[iw]->m_PointMass.size(); im++)
                {
                    PointMass const &pm = pWing[iw]->m_PointMass.at(im);
                    TotalMass +=  pm.mass();
                    TotalCoG  += (pm.position() + m_pPlane->wingLE(iw)) * pm.mass();
                }
            }
        }
    }

    if(m_pPlane && m_pPlane->body())
    {
        for(int im=0; im<m_pPlane->body()->m_PointMass.size(); im++)
        {
            PointMass const &pm = m_pPlane->body()->m_PointMass.at(im);
            TotalMass +=  pm.mass();
            TotalCoG  += (pm.position()+m_pPlane->bodyPos()) * pm.mass();
        }
    }

    if(TotalMass>PRECISION) TotalCoG *= 1.0/TotalMass;
    else                    TotalCoG.set(0.0,0.0,0.0);

    //Total inertia in CoG referential
    //Apply Huygens theorem to convert the object's inertia to the new frame
    Vector3d LA = TotalCoG - m_VolumeCoG;
    TotalIxx = m_CoGIxx + m_VolumeMass * (LA.y*LA.y+ LA.z*LA.z);
    TotalIyy = m_CoGIyy + m_VolumeMass * (LA.x*LA.x+ LA.z*LA.z);
    TotalIzz = m_CoGIzz + m_VolumeMass * (LA.x*LA.x+ LA.y*LA.y);
    TotalIxz = m_CoGIxz + m_VolumeMass *  LA.x*LA.z;

    //add the inertia contribution of all point masses in the Total CoG frame of reference
    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        if(pm.mass()>PRECISION)
        {
            MassPos = pm.position() - TotalCoG;
            TotalIxx  += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
            TotalIyy  += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
            TotalIzz  += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
            TotalIxz  += pm.mass() * (MassPos.x*MassPos.z);
        }
    }

    if(m_pPlane)
    {
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                for(int im=0; im<pWing[iw]->m_PointMass.size(); im++)
                {
                    PointMass const &pm = pWing[iw]->m_PointMass.at(im);

                    MassPos = TotalCoG - (pm.position() + (m_pPlane != nullptr ? m_pPlane->wingLE(iw) : Vector3d(0.0, 0.0, 0.0)));
                    TotalIxx  += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
                    TotalIyy  += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
                    TotalIzz  += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
                    TotalIxz  += pm.mass() * (MassPos.x*MassPos.z);
                }
            }
        }

        if(m_pPlane && m_pPlane->body())
        {
            for(int im=0; im<m_pPlane->body()->m_PointMass.size(); im++)
            {
                PointMass const &pm = m_pPlane->body()->m_PointMass.at(im);
                MassPos = TotalCoG - (m_pPlane->bodyPos() + pm.position());
                TotalIxx  += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
                TotalIyy  += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
                TotalIzz  += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
                TotalIxz  += pm.mass() * (MassPos.x*MassPos.z);
            }
        }
    }

    //display the results
    m_pdeTotalMass->setValue(TotalMass*Units::kgtoUnit());

    m_pdeXTotalCoG->setValue(TotalCoG.x*Units::mtoUnit());
    m_pdeYTotalCoG->setValue(TotalCoG.y*Units::mtoUnit());
    m_pdeZTotalCoG->setValue(TotalCoG.z*Units::mtoUnit());

    m_pdeTotalIxx->setValue(TotalIxx*Units::kgm2toUnit());
    m_pdeTotalIyy->setValue(TotalIyy*Units::kgm2toUnit());
    m_pdeTotalIzz->setValue(TotalIzz*Units::kgm2toUnit());
    m_pdeTotalIxz->setValue(TotalIxz*Units::kgm2toUnit());
}


void InertiaDlg::contextMenuEvent(QContextMenuEvent *pEvent)
{
    // Display the context menu
    if(m_ptvMass->geometry().contains(pEvent->pos())) m_pContextMenu->exec(pEvent->globalPos());
    pEvent->accept();
}


/**
* Fills the table with the object's point masses.
*/
void InertiaDlg::fillMassModel()
{
    QModelIndex index;

    m_pMassModel->setRowCount(m_PointMass.size()+1);

    int im(0);
    for(im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        if(pm.mass()>PRECISION || pm.tag().length())
        {
            index = m_pMassModel->index(im, 0, QModelIndex());
            m_pMassModel->setData(index, pm.mass()*Units::kgtoUnit());

            index = m_pMassModel->index(im, 1, QModelIndex());
            m_pMassModel->setData(index, pm.position().x*Units::mtoUnit());

            index = m_pMassModel->index(im, 2, QModelIndex());
            m_pMassModel->setData(index, pm.position().y*Units::mtoUnit());

            index = m_pMassModel->index(im, 3, QModelIndex());
            m_pMassModel->setData(index, pm.position().z*Units::mtoUnit());

            index = m_pMassModel->index(im, 4, QModelIndex());
            m_pMassModel->setData(index, pm.tag());
        }
    }

    //add an extra empty line for a new mass
    index = m_pMassModel->index(im, 0, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(im, 1, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(im, 2, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(im, 3, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(im, 4, QModelIndex());
    m_pMassModel->setData(index, "");
}


void InertiaDlg::initDialog()
{
    m_ppbWingInertia->setEnabled(false);
    m_ppbWing2Inertia->setEnabled(false);
    m_ppbStabInertia->setEnabled(false);
    m_ppbFinInertia->setEnabled(false);
    m_ppbBodyInertia->setEnabled(false);

    int rc = m_pMassModel->rowCount();
    m_pMassModel->removeRows(0, rc);

    clearPointMasses();

    if(m_pWing)
    {
        m_VolumeMass = m_pWing->m_VolumeMass;
        m_PointMass = m_pWing->m_PointMass;

        m_pdeVolumeMass->setValue(m_pWing->m_VolumeMass * Units::kgtoUnit()); //we only display half a wing, AVL way
        m_plabVolumeMassLabel->setText(tr("Wing Mass:"));
        m_ppbWingInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pWing->m_Name);
    }
    else if (m_pBody)
    {
        m_VolumeMass = m_pBody->m_VolumeMass;
        m_PointMass = m_pBody->m_PointMass;

        m_pdeVolumeMass->setValue(m_pBody->m_VolumeMass * Units::kgtoUnit());
        m_plabVolumeMassLabel->setText(tr("Body Mass:"));
        m_ppbBodyInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pBody->m_Name);
    }
    else if (m_pPlane)
    {
        m_VolumeMass = m_pPlane->wing()->m_VolumeMass;
        if(m_pPlane->biPlane()) m_VolumeMass += m_pPlane->wing2()->m_VolumeMass;
        if(m_pPlane->stab())    m_VolumeMass += m_pPlane->stab()->m_VolumeMass;
        if(m_pPlane->fin())     m_VolumeMass += m_pPlane->fin()->m_VolumeMass;
        if(m_pPlane->body())    m_VolumeMass += m_pPlane->body()->m_VolumeMass;


        for(int im=0; im<m_pPlane->m_PointMass.size(); im++)
        {
            m_PointMass.append(m_pPlane->m_PointMass.at(im));
        }

        m_pdeVolumeMass->setValue(m_VolumeMass * Units::kgtoUnit());
        m_plabVolumeMassLabel->setText(tr("Volume Mass:"));
        m_pdeVolumeMass->setEnabled(false);
        m_ppbWingInertia->setEnabled(true);
        if(m_pPlane->biPlane()) m_ppbWing2Inertia->setEnabled(true);
        if(m_pPlane->stab())    m_ppbStabInertia->setEnabled(true);
        if(m_pPlane->fin())     m_ppbFinInertia->setEnabled(true);
        if(m_pPlane->body())    m_ppbBodyInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pPlane->name());
    }
    if(m_pPlane) m_pswTop->setCurrentIndex(1);
    else         m_pswTop->setCurrentIndex(0);

    fillMassModel();
    computeInertia();
    setFocus();
}


void InertiaDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
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
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        default:
            pEvent->ignore();
    }
}


void InertiaDlg::onCellChanged(QWidget *)
{
    readData();
    computeInertia();
    m_bChanged = true;
    fillMassModel();//to add an empty line
}



void InertiaDlg::onVolumeMass()
{
    readData();
    computeInertia();
    m_bChanged = true;
}


/**
* Exports the mass and inertia data to AVL format
*/
void InertiaDlg::onExportToAVL()
{
    if (!m_pWing && !m_pBody && !m_pPlane) return;
    QString filter =".mass";

    QString FileName, strong;
    double CoGIxx(0), CoGIyy(0), CoGIzz(0), CoGIxz(0);
    Vector3d CoG;

    Wing *pWing[MAXWINGS];
    pWing[0] = pWing[1] = pWing[2] = pWing[3] = nullptr;

    if(m_pPlane)
    {
        pWing[0] =  m_pPlane->wing();
        if(m_pPlane->biPlane()) pWing[1] = m_pPlane->wing2();
        if(m_pPlane->stab())    pWing[2] = m_pPlane->stab();
        if(m_pPlane->fin())     pWing[3] = m_pPlane->fin();
    }
    else
    {
        pWing[0] = m_pWing;
    }


    if(m_pPlane)     FileName = m_pPlane->name();
    else if(m_pWing) FileName = m_pWing->m_Name;
    else if(m_pBody) FileName = m_pBody->m_Name;
    FileName.replace("/", " ");
    FileName += ".mass";
    FileName = QFileDialog::getSaveFileName(this, tr("Export Mass Properties"), xfl::s_LastDirName + "/"+FileName,
                                            tr("AVL Mass File (*.mass)"), &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    pos = FileName.lastIndexOf(".");
    if(pos<0) FileName += ".mass";

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;
    QTextStream out(&XFile);
    out.setCodec("UTF-8");

    double Lunit = 1./Units::mtoUnit();
    double Munit = 1./Units::kgtoUnit();
    double Iunit = 1./Units::kgm2toUnit();

    out << "#-------------------------------------------------\n";
    out << "#\n";
    if(m_pPlane)      out << "#   "+m_pPlane->name()+"\n";
    else if(m_pWing)  out << "#   "+m_pWing->m_Name+"\n";
    else if(m_pBody)  out << "#   "+m_pBody->m_Name+"\n";
    out << "#\n";
    out << "#  Dimensional unit and parameter data.\n";
    out << "#  Mass & Inertia breakdown.\n";
    out << "#-------------------------------------------------\n";
    out << "#  Names and scalings for units to be used for trim and eigenmode calculations.\n";
    out << "#  The Lunit and Munit values scale the mass, xyz, and inertia table data below.\n";
    out << "#  Lunit value will also scale all lengths and areas in the AVL input file.\n";
    out << "#\n";
    strong = QString("Lunit = %1 m").arg(Lunit,10,'f',4);
    out << strong+"\n";
    strong = QString("Munit = %1 kg").arg(Munit,10,'f',4);
    out << strong+"\n";
    out << "Tunit = 1.0 s\n";
    out << "#-------------------------\n";
    out << "#  Gravity and density to be used as default values in trim setup (saves runtime typing).\n";
    out << "#  Must be in the unit names given above (i.e. m,kg,s).\n";
    out << "g   = 9.81\n";
    out << "rho = 1.225\n";
    out << "#-------------------------\n";
    out << "#  Mass & Inertia breakdown.\n";
    out << "#  x y z  is location of item's own CG.\n";
    out << "#  Ixx... are item's inertias about item's own CG.\n";
    out << "#\n";
    out << "#  x,y,z system here must be exactly the same one used in the .avl input file\n";
    out << "#     (same orientation, same origin location, same length units)\n";
    out << "#\n";
    out << "#     mass          x          y          z        Ixx        Iyy        Izz        Ixy        Ixz        Iyz \n";

    if(m_pWing)
    {
        // in accordance with AVL input format,
        // we need to convert the inertia in the wing's CoG system
        // by applying Huyghen/Steiner's theorem

        strong = QString(tr("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10! Inertia of both left and right wings"))
                .arg(m_VolumeMass /Munit, 10, 'g', 3)
                .arg(m_VolumeCoG.x/Lunit, 10, 'g', 3)
                .arg(m_VolumeCoG.y/Lunit, 10, 'g', 3)  //should be zero
                .arg(m_VolumeCoG.z/Lunit, 10, 'g', 3)
                .arg(m_CoGIxx/Iunit,  10, 'g', 3)
                .arg(m_CoGIyy/Iunit,  10, 'g', 3)
                .arg(m_CoGIzz/Iunit,  10, 'g', 3)
                .arg(0.0,  10, 'g', 3).arg(m_CoGIxz/Iunit,  10, 'g', 3).arg(0.0,  10, 'g', 3);
        out << strong+"\n";
    }
    else if (m_pBody)
    {
        strong = QString(tr("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ! Body inertia"))
                .arg(m_VolumeMass /Munit, 10, 'g', 3)
                .arg(m_VolumeCoG.x/Lunit, 10, 'g', 3)
                .arg(m_VolumeCoG.y/Lunit, 10, 'g', 3)
                .arg(m_VolumeCoG.z/Lunit, 10, 'g', 3)
                .arg(m_CoGIxx/Iunit,  10, 'g', 3)
                .arg(m_CoGIyy/Iunit,  10, 'g', 3)
                .arg(m_CoGIzz/Iunit,  10, 'g', 3)
                .arg(0.0,10, 'g', 3)
                .arg(m_CoGIxz/Iunit,10, 'g', 3)
                .arg(0.0,10, 'g', 3);
        out << strong+"\n";
    }
    else if (m_pPlane)
    {
        // we write out each object contribution individually
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                pWing[iw]->computeGeometry();
                pWing[iw]->computeVolumeInertia(CoG, CoGIxx, CoGIyy, CoGIzz, CoGIxz);
                strong = QString(tr("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ! "))
                        .arg(pWing[iw]->m_VolumeMass /Munit, 10, 'g', 3)
                        .arg(CoG.x/Lunit, 10, 'g', 3)
                        .arg(CoG.y/Lunit, 10, 'g', 3)
                        .arg(CoG.z/Lunit, 10, 'g', 3)
                        .arg(CoGIxx/Iunit,10, 'g', 3)
                        .arg(CoGIyy/Iunit,10, 'g', 3)
                        .arg(CoGIzz/Iunit,10, 'g', 3)
                        .arg(0.0,10, 'g', 3)
                        .arg(CoGIxz/Iunit,10, 'g', 3)
                        .arg(0.0,10, 'g', 3);
                strong += pWing[iw]->m_Name;
                out << strong+"\n";
            }
        }

        if(m_pPlane->body())
        {
            m_pPlane->body()->computeVolumeInertia(CoG, CoGIxx, CoGIyy, CoGIzz, CoGIxz);
            strong = QString(tr("%1 %2 %3 %4 %5 %6 %7 %8 %9 %10 ! Body's inertia"))
                    .arg(m_pPlane->body()->m_VolumeMass /Munit, 10, 'g', 3)
                    .arg(CoG.x/Lunit, 10, 'g', 3)
                    .arg(CoG.y/Lunit, 10, 'g', 3)
                    .arg(CoG.z/Lunit, 10, 'g', 3)
                    .arg(CoGIxx/Iunit,10, 'g', 3)
                    .arg(CoGIyy/Iunit,10, 'g', 3)
                    .arg(CoGIzz/Iunit,10, 'g', 3)
                    .arg(0.0,10, 'g', 3)
                    .arg(CoGIxz/Iunit,10, 'g', 3)
                    .arg(0.0,10, 'g', 3);
            /*         .arg(m_pPlane->Body()->m_VolumeMass /Munit, 10, 'g', 3)
                                                 .arg(m_pPlane->Body()->m_VolumeCoG.x/Lunit, 10, 'g', 3)
                                                 .arg(m_pPlane->Body()->m_VolumeCoG.y/Lunit, 10, 'g', 3)
                                                 .arg(m_pPlane->Body()->m_VolumeCoG.z/Lunit, 10, 'g', 3)
                         .arg(m_pPlane->Body()->m_CoGIxx/Iunit,10, 'g', 3)
                         .arg(m_pPlane->Body()->m_CoGIyy/Iunit,10, 'g', 3)
                         .arg(m_pPlane->Body()->m_CoGIzz/Iunit,10, 'g', 3)
                         .arg(0.0,10, 'g', 3)
                         .arg(m_pPlane->Body()->m_CoGIxz/Iunit,10, 'g', 3)
                                                 .arg(0.0,10, 'g', 3); */
            out << strong+"\n";
        }
    }

    for (int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        if(pm.mass()>0.0)
        {
            strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                    .arg(pm.mass() / Munit,    10, 'g', 3)
                    .arg(pm.position().x/Lunit, 10, 'g', 3)
                    .arg(pm.position().y/Lunit, 10, 'g', 3)
                    .arg(pm.position().z/Lunit, 10, 'g', 3);
            strong += " ! " + pm.tag();
            out << strong+"\n";
        }
    }

    if(m_pPlane)
    {
        // need to write the point masses for the objects
        //Main Wing
        for(int iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                for (int im=0; im<pWing[iw]->m_PointMass.size(); im++)
                {
                    PointMass const &pm = pWing[iw]->m_PointMass.at(im);
                    if(pm.mass()>0.0)
                    {
                        strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                                .arg(pm.mass() / Munit,    10, 'g', 3)
                                .arg((pm.position().x+m_pPlane->wingLE(iw).x)/Lunit, 10, 'g', 3)
                                .arg((pm.position().y+m_pPlane->wingLE(iw).y)/Lunit, 10, 'g', 3)
                                .arg((pm.position().z+m_pPlane->wingLE(iw).z)/Lunit, 10, 'g', 3);

                        strong += " ! " + pm.tag();
                        out << strong+"\n";
                    }
                }
            }
        }

        if(m_pPlane->body())
        {
            //fin
            for (int im=0; im<m_pPlane->body()->m_PointMass.size(); im++)
            {
                PointMass const &pm = m_pPlane->body()->m_PointMass.at(im);
                if(pm.mass()>0.0)
                {
                    strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                            .arg(pm.mass() / Munit,    10, 'g', 3)
                            .arg(pm.position().x/Lunit, 10, 'g', 3)
                            .arg(pm.position().y/Lunit, 10, 'g', 3)
                            .arg(pm.position().z/Lunit, 10, 'g', 3);
                    strong += " ! " + pm.tag();
                    out << strong+"\n";
                }
            }
        }
    }

    XFile.close();
}


void InertiaDlg::onInsertMassRow()
{
    int sel = m_ptvMass->currentIndex().row();

    m_PointMass.insert(sel, {0.0, Vector3d(0.0,0.0,0.0), QString()});

    fillMassModel();
    m_ptvMass->closePersistentEditor(m_ptvMass->currentIndex());

    QModelIndex index = m_pMassModel->index(sel, 0, QModelIndex());
    m_ptvMass->setCurrentIndex(index);
}


void InertiaDlg::onDeleteMassRow()
{
    m_ptvMass->closePersistentEditor(m_ptvMass->currentIndex());
    int sel = m_ptvMass->currentIndex().row();
    if(sel>=0 && sel<m_PointMass.size())
    {
        m_PointMass.removeAt(sel);
        fillMassModel();
    }
}


void InertiaDlg::onOK()
{
    readData();

    if(m_pWing)
    {
        m_pWing->m_VolumeMass = m_VolumeMass;
        m_pWing->clearPointMasses();

        for(int im=0; im<m_PointMass.size(); im++)
        {
            PointMass const &pm = m_PointMass.at(im);

            if(pm.mass()>PRECISION)
            {
                m_pWing->m_PointMass.append(pm);
            }
        }
    }
    else if(m_pBody)
    {
        m_pBody->m_VolumeMass = m_VolumeMass;
        m_pBody->clearPointMasses();

        for(int im=0; im< m_PointMass.size(); im++)
        {
            PointMass const &pm = m_PointMass.at(im);
            if(pm.mass()>PRECISION)
            {
                m_pBody->m_PointMass.append(pm);
            }
        }
    }
    else if(m_pPlane)
    {
        m_pPlane->clearPointMasses();

        for(int im=0; im<m_PointMass.size(); im++)
        {
            PointMass const &pm = m_PointMass.at(im);
            if(pm.mass()>PRECISION)
            {
                m_pPlane->m_PointMass.append(pm);
            }
        }
    }

    computeBodyAxisInertia();

    accept();
}


void InertiaDlg::readData()
{
    QModelIndex index;
    bool bOK(false);
    double mass(0), x(0), y(0), z(0);

    QString tag;

    clearPointMasses();

    for (int i=0; i<m_pMassModel->rowCount(); i++)
    {
        index = m_pMassModel->index(i, 0, QModelIndex());
        mass = index.data().toDouble(&bOK);

        index = m_pMassModel->index(i, 1, QModelIndex());
        x = index.data().toDouble(&bOK);

        index = m_pMassModel->index(i, 2, QModelIndex());
        y = index.data().toDouble(&bOK);

        index = m_pMassModel->index(i, 3, QModelIndex());
        z = index.data().toDouble(&bOK);

        index = m_pMassModel->index(i, 4, QModelIndex());
        tag = index.data().toString();

        if(qAbs(mass)>PRECISION || qAbs(x)>PRECISION || qAbs(y)>PRECISION || qAbs(z)>PRECISION || tag.length())
        {
            m_PointMass.append({mass/Units::kgtoUnit(),
                                Vector3d(x/Units::mtoUnit(), y/Units::mtoUnit(), z/Units::mtoUnit()),
                                tag});
        }
    }
    m_VolumeMass = m_pdeVolumeMass->value() / Units::kgtoUnit();
}


void InertiaDlg::setupLayout()
{
    QString strMass, strLength, strInertia;
    Units::getMassUnitLabel(strMass);
    Units::getLengthUnitLabel(strLength);
    strInertia = Units::inertiaUnitLabel();

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    QVBoxLayout *pMessageLayout = new QVBoxLayout;
    {
        QLabel *pLabel1 = new QLabel(tr("This is a calculation form for a rough order of magnitude for the inertia tensor."));
        QLabel *pLabel2 = new QLabel(tr("Refer to the Guidelines for explanations."));
        pMessageLayout->addWidget(pLabel1);
        pMessageLayout->addWidget(pLabel2);
    }

    //___________Volume Mass, Center of gravity, and inertias__________
    m_pswTop = new QStackedWidget;

    QGroupBox *pObjectMassBox = new QGroupBox(tr("Object Mass - Volume only, excluding point masses"));
    {
        QHBoxLayout *pObjectMassLayout = new QHBoxLayout;
        m_plabVolumeMassLabel  = new QLabel(tr("Wing Mass="));
        QLabel *m_plabMassUnit   = new QLabel(Units::massUnitLabel());
        m_pdeVolumeMass = new DoubleEdit(1.00,3);

        QGroupBox *pCoGBox = new QGroupBox(tr("Center of gravity"));
        {
            QGridLayout *pCoGLayout = new QGridLayout;
            {
                QLabel *CoGLabel = new QLabel(tr("Center of gravity"));
                CoGLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *XCoGLab = new QLabel("X_CoG=");
                QLabel *YCoGLab = new QLabel("Y_CoG=");
                QLabel *ZCoGLab = new QLabel("Z_CoG=");
                m_pdeXCoG = new DoubleEdit(0.00,3);
                m_pdeYCoG = new DoubleEdit(0.00,3);
                m_pdeZCoG = new DoubleEdit(0.00,3);
                m_pdeXCoG->setEnabled(false);
                m_pdeYCoG->setEnabled(false);
                m_pdeZCoG->setEnabled(false);
                QLabel *plabLengthUnit10 = new QLabel(strLength);
                QLabel *plabLengthUnit11 = new QLabel(strLength);
                QLabel *plabLengthUnit12 = new QLabel(strLength);
                m_plabVolumeMassLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                XCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                YCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ZCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeXCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeYCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeZCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pCoGLayout->addWidget(m_plabVolumeMassLabel,1,1);
                pCoGLayout->addWidget(m_pdeVolumeMass,1,2);
                pCoGLayout->addWidget(m_plabMassUnit,1,3);
                pCoGLayout->addWidget(XCoGLab,2,1);
                pCoGLayout->addWidget(YCoGLab,3,1);
                pCoGLayout->addWidget(ZCoGLab,4,1);
                //    CoGLayout->addWidget(CoGLabel,2,1);
                pCoGLayout->addWidget(m_pdeXCoG,2,2);
                pCoGLayout->addWidget(m_pdeYCoG,3,2);
                pCoGLayout->addWidget(m_pdeZCoG,4,2);
                pCoGLayout->addWidget(plabLengthUnit10,2,3);
                pCoGLayout->addWidget(plabLengthUnit11,3,3);
                pCoGLayout->addWidget(plabLengthUnit12,4,3);
                pCoGLayout->setColumnStretch(1,1);
                pCoGLayout->setColumnStretch(2,2);
                pCoGLayout->setColumnStretch(3,1);
            }
            pCoGBox->setLayout(pCoGLayout);
        }

        QGroupBox *pResultsBox = new QGroupBox(tr("Inertia in CoG Frame"));
        {
            QGridLayout *pResultsLayout = new QGridLayout;
            {
                m_pdeCoGIxx = new DoubleEdit(1.0,3);
                m_pdeCoGIyy = new DoubleEdit(1.2,3);
                m_pdeCoGIzz = new DoubleEdit(-1.5,3);
                m_pdeCoGIxz = new DoubleEdit(4.2,3);
                m_pdeCoGIxx->setEnabled(false);
                m_pdeCoGIyy->setEnabled(false);
                m_pdeCoGIzz->setEnabled(false);
                m_pdeCoGIxz->setEnabled(false);
                QLabel *LabIxx = new QLabel("Ixx=");
                QLabel *LabIyy = new QLabel("Iyy=");
                QLabel *LabIzz = new QLabel("Izz=");
                QLabel *LabIxz = new QLabel("Ixz=");
                LabIxx->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabIyy->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabIzz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabIxz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *LabInertiaObject = new QLabel(tr("Inertia in CoG Frame"));
                LabInertiaObject->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *plabInertiaUnit1 = new QLabel(strInertia);
                QLabel *plabInertiaUnit2 = new QLabel(strInertia);
                QLabel *plabInertiaUnit3 = new QLabel(strInertia);
                QLabel *plabInertiaUnit4 = new QLabel(strInertia);
                pResultsLayout->addWidget(LabIxx,1,1);
                pResultsLayout->addWidget(LabIyy,2,1);
                pResultsLayout->addWidget(LabIzz,3,1);
                pResultsLayout->addWidget(LabIxz,4,1);
                pResultsLayout->addWidget(m_pdeCoGIxx,1,2);
                pResultsLayout->addWidget(m_pdeCoGIyy,2,2);
                pResultsLayout->addWidget(m_pdeCoGIzz,3,2);
                pResultsLayout->addWidget(m_pdeCoGIxz,4,2);
                pResultsLayout->addWidget(plabInertiaUnit1,1,3);
                pResultsLayout->addWidget(plabInertiaUnit2,2,3);
                pResultsLayout->addWidget(plabInertiaUnit3,3,3);
                pResultsLayout->addWidget(plabInertiaUnit4,4,3);
                pResultsLayout->setColumnStretch(1,1);
                pResultsLayout->setColumnStretch(2,2);
                pResultsLayout->setColumnStretch(3,1);
            }
            pResultsBox->setLayout(pResultsLayout);
        }

        pObjectMassLayout->addWidget(pCoGBox);
        pObjectMassLayout->addWidget(pResultsBox);
        pObjectMassBox->setLayout(pObjectMassLayout);
    }

    QGroupBox *pObjectSelectionBox = new QGroupBox(tr("Component inertias"));
    {
        QVBoxLayout *pObjectLayout = new QVBoxLayout;
        {
            m_ppbWingInertia  = new QPushButton(tr("Main Wing"));
            m_ppbWing2Inertia = new QPushButton(tr("Second Wing"));
            m_ppbStabInertia  = new QPushButton(tr("Elevator"));
            m_ppbFinInertia   = new QPushButton(tr("Fin"));
            m_ppbBodyInertia  = new QPushButton(tr("Body"));
            QHBoxLayout *pWingInertiasLayout = new QHBoxLayout;
            {
                pWingInertiasLayout->addStretch();
                pWingInertiasLayout->addWidget(m_ppbWingInertia);
                pWingInertiasLayout->addStretch();
                pWingInertiasLayout->addWidget(m_ppbWing2Inertia);
                pWingInertiasLayout->addStretch();
            }
            QVBoxLayout *pAxisInertiasLayout = new QVBoxLayout;
            {
                QHBoxLayout *pStabLayout = new QHBoxLayout;
                {
                    pStabLayout->addStretch();
                    pStabLayout->addWidget(m_ppbStabInertia);
                    pStabLayout->addStretch();
                }
                QHBoxLayout *pFinStabLayout = new QHBoxLayout;
                {
                    pFinStabLayout->addStretch();
                    pFinStabLayout->addWidget(m_ppbFinInertia);
                    pFinStabLayout->addStretch();
                }
                QHBoxLayout *pBodyLayout = new QHBoxLayout;
                {
                    pBodyLayout->addStretch();
                    pBodyLayout->addWidget(m_ppbBodyInertia);
                    pBodyLayout->addStretch();
                }
                pAxisInertiasLayout->addStretch(3);
                pAxisInertiasLayout->addLayout(pStabLayout);
                pAxisInertiasLayout->addStretch(3);
                pAxisInertiasLayout->addLayout(pFinStabLayout);
                pAxisInertiasLayout->addStretch(3);
                pAxisInertiasLayout->addLayout(pBodyLayout);
                pAxisInertiasLayout->addStretch(3);
            }

            pObjectLayout->addLayout(pWingInertiasLayout);
            //            ObjectLayout->addStretch();
            pObjectLayout->addLayout(pAxisInertiasLayout);
            //            ObjectLayout->addStretch();
            pObjectSelectionBox->setLayout(pObjectLayout);
        }
    }

    m_pswTop->addWidget(pObjectMassBox);
    m_pswTop->addWidget(pObjectSelectionBox);

    //___________________Point Masses__________________________
    QLabel *pPointMasses = new QLabel(tr("Additional Point Masses"));
    m_ptvMass = new CPTableView(this);
    m_ptvMass->setSizePolicy(szPolicyExpanding);
    m_ptvMass->setFont(DisplayOptions::tableFont());
    m_ptvMass->setMinimumHeight(150);
    m_ptvMass->horizontalHeader()->setStretchLastSection(true);
    m_ptvMass->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ptvMass->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ptvMass->setEditTriggers(QAbstractItemView::CurrentChanged |
                                      QAbstractItemView::DoubleClicked |
                                      QAbstractItemView::SelectedClicked |
                                      QAbstractItemView::EditKeyPressed |
                                      QAbstractItemView::AnyKeyPressed);

    m_pMassModel = new QStandardItemModel(this);
    m_pMassModel->setRowCount(10);//temporary
    m_pMassModel->setColumnCount(5);

    m_pMassModel->setHeaderData(0, Qt::Horizontal, tr("Mass") +" ("+strMass+")");
    m_pMassModel->setHeaderData(1, Qt::Horizontal, tr("x") +" ("+strLength+")");
    m_pMassModel->setHeaderData(2, Qt::Horizontal, tr("y")+" ("+strLength+")");
    m_pMassModel->setHeaderData(3, Qt::Horizontal, tr("z")+" ("+strLength+")");
    m_pMassModel->setHeaderData(4, Qt::Horizontal, tr("Description"));

    m_ptvMass->setModel(m_pMassModel);

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_ptvMass->setItemDelegate(m_pFloatDelegate);
    m_pFloatDelegate->setPrecision({3,3,3,3,-1});

    connect(m_pFloatDelegate,  SIGNAL(closeEditor(QWidget*)), this, SLOT(onCellChanged(QWidget*)));

    //________________Total Mass, Center of gravity, and inertias__________
    QGroupBox *pTotalMassBox = new QGroupBox(tr("Total Mass = Volume + point masses"));
    {
        QHBoxLayout *pTotalMassLayout = new QHBoxLayout;
        m_plabTotalMassLabel   = new QLabel(tr("Total Mass="));
        QLabel *plabMassUnit2   = new QLabel(Units::massUnitLabel());
        m_pdeTotalMass        = new DoubleEdit(1.00,3);
        m_pdeTotalMass->setEnabled(false);

        QGroupBox *pTotalCoGBox = new QGroupBox(tr("Center of gravity"));
        {
            QGridLayout *pTotalCoGLayout = new QGridLayout;
            {
                QLabel *pTotalLabel = new QLabel(tr("Center of gravity"));
                pTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *pXTotalLab = new QLabel("X_CoG=");
                QLabel *pYTotalLab = new QLabel("Y_CoG=");
                QLabel *pZTotalLab = new QLabel("Z_CoG=");
                m_pdeXTotalCoG = new DoubleEdit(0.00,3);
                m_pdeYTotalCoG = new DoubleEdit(0.00,3);
                m_pdeZTotalCoG = new DoubleEdit(0.00,3);
                m_pdeXTotalCoG->setEnabled(false);
                m_pdeYTotalCoG->setEnabled(false);
                m_pdeZTotalCoG->setEnabled(false);
                QLabel *plabLengthUnit20 = new QLabel(strLength);
                QLabel *plabLengthUnit21 = new QLabel(strLength);
                QLabel *plabLengthUnit22 = new QLabel(strLength);
                m_plabTotalMassLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pXTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pYTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pZTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeXTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeYTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pdeZTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pTotalCoGLayout->addWidget(m_plabTotalMassLabel,1,1);
                pTotalCoGLayout->addWidget(m_pdeTotalMass,1,2);
                pTotalCoGLayout->addWidget(plabMassUnit2,1,3);
                pTotalCoGLayout->addWidget(pXTotalLab,2,1);
                pTotalCoGLayout->addWidget(pYTotalLab,3,1);
                pTotalCoGLayout->addWidget(pZTotalLab,4,1);
                pTotalCoGLayout->addWidget(m_pdeXTotalCoG,2,2);
                pTotalCoGLayout->addWidget(m_pdeYTotalCoG,3,2);
                pTotalCoGLayout->addWidget(m_pdeZTotalCoG,4,2);
                pTotalCoGLayout->addWidget(plabLengthUnit20,2,3);
                pTotalCoGLayout->addWidget(plabLengthUnit21,3,3);
                pTotalCoGLayout->addWidget(plabLengthUnit22,4,3);
                pTotalCoGLayout->setColumnStretch(1,1);
                pTotalCoGLayout->setColumnStretch(2,2);
                pTotalCoGLayout->setColumnStretch(3,1);
            }
            pTotalCoGBox->setLayout(pTotalCoGLayout);
        }

        QGroupBox *pTotalInertiaBox = new QGroupBox(tr("Inertia in CoG Frame"));
        {
            QGridLayout *pTotalInertiaLayout = new QGridLayout;
            {
                m_pdeTotalIxx = new DoubleEdit(1.0,5);
                m_pdeTotalIyy = new DoubleEdit(1.2,5);
                m_pdeTotalIzz = new DoubleEdit(-1.5,5);
                m_pdeTotalIxz = new DoubleEdit(4.2,5);
                m_pdeTotalIxx->setEnabled(false);
                m_pdeTotalIyy->setEnabled(false);
                m_pdeTotalIzz->setEnabled(false);
                m_pdeTotalIxz->setEnabled(false);
                QLabel *plabTotIxx = new QLabel("Ixx=");
                QLabel *plabTotIyy = new QLabel("Iyy=");
                QLabel *plabTotIzz = new QLabel("Izz=");
                QLabel *plabTotIxz = new QLabel("Ixz=");
                plabTotIxx->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                plabTotIyy->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                plabTotIzz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                plabTotIxz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *plabInertiaTotal = new QLabel(tr("Inertia in CoG Frame"));
                plabInertiaTotal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *plabInertiaUnit10 = new QLabel(strInertia);
                QLabel *plabInertiaUnit20 = new QLabel(strInertia);
                QLabel *plabInertiaUnit30 = new QLabel(strInertia);
                QLabel *plabInertiaUnit40 = new QLabel(strInertia);
                pTotalInertiaLayout->addWidget(plabTotIxx,1,1);
                pTotalInertiaLayout->addWidget(plabTotIyy,2,1);
                pTotalInertiaLayout->addWidget(plabTotIzz,3,1);
                pTotalInertiaLayout->addWidget(plabTotIxz,4,1);
                pTotalInertiaLayout->addWidget(m_pdeTotalIxx,1,2);
                pTotalInertiaLayout->addWidget(m_pdeTotalIyy,2,2);
                pTotalInertiaLayout->addWidget(m_pdeTotalIzz,3,2);
                pTotalInertiaLayout->addWidget(m_pdeTotalIxz,4,2);
                pTotalInertiaLayout->addWidget(plabInertiaUnit10,1,3);
                pTotalInertiaLayout->addWidget(plabInertiaUnit20,2,3);
                pTotalInertiaLayout->addWidget(plabInertiaUnit30,3,3);
                pTotalInertiaLayout->addWidget(plabInertiaUnit40,4,3);
                pTotalInertiaLayout->setColumnStretch(1,1);
                pTotalInertiaLayout->setColumnStretch(2,2);
                pTotalInertiaLayout->setColumnStretch(3,1);
            }
            pTotalInertiaBox->setLayout(pTotalInertiaLayout);
        }

        pTotalMassLayout->addWidget(pTotalCoGBox);
        pTotalMassLayout->addWidget(pTotalInertiaBox);
        pTotalMassBox->setLayout(pTotalMassLayout);
    }
    //__________________Control buttons___________________

    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Discard);
    {
        m_ppbExportToAVL = new QPushButton(tr("Export to AVL"));
        m_pButtonBox->addButton(m_ppbExportToAVL, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addLayout(pMessageLayout);
        pMainLayout->addWidget(m_pswTop);
        pMainLayout->addWidget(pPointMasses);
        pMainLayout->addWidget(m_ptvMass);
        pMainLayout->addWidget(pTotalMassBox);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connect(m_ppbWingInertia,  SIGNAL(clicked()), this, SLOT(onWingInertia()));
    connect(m_ppbWing2Inertia, SIGNAL(clicked()), this, SLOT(onWing2Inertia()));
    connect(m_ppbStabInertia,  SIGNAL(clicked()), this, SLOT(onStabInertia()));
    connect(m_ppbFinInertia,   SIGNAL(clicked()), this, SLOT(onFinInertia()));
    connect(m_ppbBodyInertia,  SIGNAL(clicked()), this, SLOT(onBodyInertia()));
    connect(m_pdeVolumeMass,   SIGNAL(editingFinished()), SLOT(onVolumeMass()));
    connect(m_pInsertMassRow,    SIGNAL(triggered()), SLOT(onInsertMassRow()));
    connect(m_pDeleteMassRow,    SIGNAL(triggered()), SLOT(onDeleteMassRow()));
}


void InertiaDlg::onWingInertia()
{
    InertiaDlg dlg(this);
    if(!m_pPlane->wing()) return;
    dlg.m_pWing  = m_pPlane->wing();
    dlg.m_pPlane = nullptr;
    dlg.m_pBody  = nullptr;
    dlg.initDialog();
    dlg.move(pos() + QPoint(43, 19));
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    computeInertia();
}


void InertiaDlg::onWing2Inertia()
{
    if(!m_pPlane->biPlane()) return;
    InertiaDlg dlg(this);
    dlg.m_pWing  = m_pPlane->wing2();
    dlg.m_pPlane = nullptr;
    dlg.m_pBody  = nullptr;
    dlg.initDialog();
    dlg.move(pos() + QPoint(43, 19));
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
    computeInertia();
}


void InertiaDlg::onStabInertia()
{
    if(!m_pPlane->stab()) return;
    InertiaDlg dlg(this);
    dlg.m_pWing  = m_pPlane->stab();
    dlg.m_pPlane = nullptr;
    dlg.m_pBody  = nullptr;
    dlg.initDialog();
    dlg.move(pos() + QPoint(43, 19));
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;

    computeInertia();
}


void InertiaDlg::onFinInertia()
{
    if(!m_pPlane->fin()) return;
    InertiaDlg dlg(this);
    dlg.m_pWing  = m_pPlane->fin();
    dlg.m_pPlane = nullptr;
    dlg.m_pBody  = nullptr;
    dlg.initDialog();
    dlg.move(pos() + QPoint(43, 19));

    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;

    computeInertia();
}


void InertiaDlg::onBodyInertia()
{
    if(!m_pPlane->body()) return;
    InertiaDlg dlg(this);
    dlg.m_pBody  = m_pPlane->body();
    dlg.m_pPlane = nullptr;
    dlg.m_pWing  = nullptr;
    dlg.initDialog();
    dlg.move(pos() + QPoint(43, 19));
    if(dlg.exec()==QDialog::Accepted) m_bChanged=true;

    computeInertia();
}


void InertiaDlg::resizeEvent(QResizeEvent *pEvent)
{
    if(!m_ptvMass || !m_pMassModel) return;
    int w = m_ptvMass->width();
    int w7 = int(double(w-25)/7);

    m_ptvMass->setColumnWidth(0,w7);
    m_ptvMass->setColumnWidth(1,w7);
    m_ptvMass->setColumnWidth(2,w7);
    m_ptvMass->setColumnWidth(3,w7);
    m_ptvMass->setColumnWidth(4,3*w7);

    pEvent->accept();
}


void InertiaDlg::showEvent(QShowEvent*)
{
    restoreGeometry(s_Geometry);
}



void InertiaDlg::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}





