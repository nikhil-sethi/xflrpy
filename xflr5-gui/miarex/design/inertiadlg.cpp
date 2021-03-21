/****************************************************************************

    InertiaDlg Class
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
#include <globals/globals.h>
#include <misc/options/units.h>
#include <misc/options/settings.h>
#include <misc/text/doubleedit.h>
#include <misc/text/floatrditdelegate.h>
#include <objects/objects3d/plane.h>

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
    delete [] m_precision;
}


void InertiaDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Save) == pButton)    onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton) reject();
    else if (pButton==m_pctrlExportToAVL) onExportToAVL();
}


/** Destroys the PointMass objects in good order to avoid memory leaks */
void InertiaDlg::clearPointMasses()
{
    for(int ipm=m_PointMass.size()-1; ipm>=0; ipm--)
    {
        delete m_PointMass.at(ipm);
        m_PointMass.removeAt(ipm);
    }
}


void InertiaDlg::computeBodyAxisInertia()
{
    if(m_pPlane)     m_pPlane->computeBodyAxisInertia();
    else if(m_pWing) m_pWing->computeBodyAxisInertia();
    else if(m_pBody) m_pBody->computeBodyAxisInertia();
}


/**
* Computes the inertia in the frame of reference with origin at the CoG.
*
* Assumes that the data has been read.
*/
void InertiaDlg::computeInertia()
{
    int i, iw;
    double TotalMass, TotalIxx, TotalIyy, TotalIzz, TotalIxz;
    Vector3d TotalCoG, MassPos;

    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;
    m_VolumeCoG.set(0.0, 0.0, 0.0);

    Wing *pWing[MAXWINGS];
    pWing[0] = pWing[1] = pWing[2] = pWing[3] = nullptr;

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
        if(m_pWing->m_VolumeMass>PRECISION) m_pWing->computeVolumeInertia(m_VolumeCoG, m_CoGIxx, m_CoGIyy, m_CoGIzz, m_CoGIxz);
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
    m_pctrlXCoG->setValue(m_VolumeCoG.x*Units::mtoUnit());
    m_pctrlYCoG->setValue(m_VolumeCoG.y*Units::mtoUnit());
    m_pctrlZCoG->setValue(m_VolumeCoG.z*Units::mtoUnit());

    m_pctrlCoGIxx->setValue(m_CoGIxx*Units::kgm2toUnit());
    m_pctrlCoGIyy->setValue(m_CoGIyy*Units::kgm2toUnit());
    m_pctrlCoGIzz->setValue(m_CoGIzz*Units::kgm2toUnit());
    m_pctrlCoGIxz->setValue(m_CoGIxz*Units::kgm2toUnit());

    // take into account all point masses to calculate the total CoG and total mass
    TotalCoG.set(m_VolumeMass*m_VolumeCoG.x, m_VolumeMass*m_VolumeCoG.y, m_VolumeMass*m_VolumeCoG.z);
    TotalMass = m_VolumeMass;
    TotalIxx = TotalIyy = TotalIzz = TotalIxz = 0.0;

    for(i=0; i<m_PointMass.size(); i++)
    {
        TotalMass += m_PointMass[i]->mass();
        TotalCoG  += m_PointMass[i]->position() * m_PointMass[i]->mass();
    }

    if(m_pPlane)
    {
        for(iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                for(i=0; i<pWing[iw]->m_PointMass.size(); i++)
                {
                    TotalMass +=  pWing[iw]->m_PointMass[i]->mass();
                    TotalCoG  += (pWing[iw]->m_PointMass[i]->position() + m_pPlane->WingLE(iw)) * pWing[iw]->m_PointMass[i]->mass();
                }
            }
        }
    }

    if(m_pPlane && m_pPlane->body())
    {
        for(i=0; i<m_pPlane->body()->m_PointMass.size(); i++)
        {
            TotalMass +=  m_pPlane->body()->m_PointMass[i]->mass();
            TotalCoG  += (m_pPlane->body()->m_PointMass[i]->position()+m_pPlane->bodyPos()) * m_pPlane->body()->m_PointMass[i]->mass();
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
    TotalIxz = m_CoGIxz - m_VolumeMass *  LA.x*LA.z;

    //add the inertia contribution of all point masses in the Total CoG frame of reference
    for(i=0; i<m_PointMass.size(); i++)
    {
        if(m_PointMass[i]->mass()>PRECISION)
        {
            MassPos = TotalCoG - m_PointMass[i]->position();
            TotalIxx  += m_PointMass[i]->mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
            TotalIyy  += m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
            TotalIzz  += m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
            TotalIxz  -= m_PointMass[i]->mass() * (MassPos.x*MassPos.z);
        }
    }


    if(m_pPlane)
    {
        for(iw=0; iw<MAXWINGS; iw++)
        {
            if(pWing[iw])
            {
                for(i=0; i<pWing[iw]->m_PointMass.size(); i++)
                {
                    MassPos = TotalCoG - (pWing[iw]->m_PointMass[i]->position() + (m_pPlane != nullptr ? m_pPlane->WingLE(iw) : Vector3d(0.0, 0.0, 0.0)));
                    TotalIxx  += pWing[iw]->m_PointMass[i]->mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
                    TotalIyy  += pWing[iw]->m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
                    TotalIzz  += pWing[iw]->m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
                    TotalIxz  -= pWing[iw]->m_PointMass[i]->mass() * (MassPos.x*MassPos.z);
                }
            }
        }

        if(m_pPlane && m_pPlane->body())
        {
            for(i=0; i<m_pPlane->body()->m_PointMass.size(); i++)
            {
                MassPos = TotalCoG - (m_pPlane->bodyPos() + m_pPlane->body()->m_PointMass[i]->position());
                TotalIxx  += m_pPlane->body()->m_PointMass[i]->mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
                TotalIyy  += m_pPlane->body()->m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
                TotalIzz  += m_pPlane->body()->m_PointMass[i]->mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
                TotalIxz  -= m_pPlane->body()->m_PointMass[i]->mass() * (MassPos.x*MassPos.z);
            }
        }
    }

    //display the results
    m_pctrlTotalMass->setValue(TotalMass*Units::kgtoUnit());

    m_pctrlXTotalCoG->setValue(TotalCoG.x*Units::mtoUnit());
    m_pctrlYTotalCoG->setValue(TotalCoG.y*Units::mtoUnit());
    m_pctrlZTotalCoG->setValue(TotalCoG.z*Units::mtoUnit());

    m_pctrlTotalIxx->setValue(TotalIxx*Units::kgm2toUnit());
    m_pctrlTotalIyy->setValue(TotalIyy*Units::kgm2toUnit());
    m_pctrlTotalIzz->setValue(TotalIzz*Units::kgm2toUnit());
    m_pctrlTotalIxz->setValue(TotalIxz*Units::kgm2toUnit());
}



void InertiaDlg::contextMenuEvent(QContextMenuEvent *event)
{
    // Display the context menu
    if(m_pctrlMassTable->geometry().contains(event->pos())) m_pContextMenu->exec(event->globalPos());
    event->accept();
}


/**
* Fills the table with the object's point masses.
*/
void InertiaDlg::fillMassModel()
{
    QModelIndex index;

    m_pMassModel->setRowCount(m_PointMass.size()+1);

    int i;
    for(i=0; i<m_PointMass.size(); i++)
    {
        if(m_PointMass[i]->mass()>PRECISION || m_PointMass[i]->tag().length())
        {
            index = m_pMassModel->index(i, 0, QModelIndex());
            m_pMassModel->setData(index, m_PointMass[i]->mass()*Units::kgtoUnit());

            index = m_pMassModel->index(i, 1, QModelIndex());
            m_pMassModel->setData(index, m_PointMass[i]->position().x*Units::mtoUnit());

            index = m_pMassModel->index(i, 2, QModelIndex());
            m_pMassModel->setData(index, m_PointMass[i]->position().y*Units::mtoUnit());

            index = m_pMassModel->index(i, 3, QModelIndex());
            m_pMassModel->setData(index, m_PointMass[i]->position().z*Units::mtoUnit());

            index = m_pMassModel->index(i, 4, QModelIndex());
            m_pMassModel->setData(index, m_PointMass[i]->tag());
        }
    }

    //add an extra empty line for a new mass
    index = m_pMassModel->index(i, 0, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 1, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 2, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 3, QModelIndex());
    m_pMassModel->setData(index, 0.0);

    index = m_pMassModel->index(i, 4, QModelIndex());
    m_pMassModel->setData(index, "");
}


void InertiaDlg::initDialog()
{
    QString strMass, strLength, strInertia;

    strMass = Units::weightUnitLabel();
    strLength = Units::lengthUnitLabel();
    strInertia = Units::inertiaUnitLabel();

    m_pctrlMassUnit->setText(strMass);
    m_pctrlMassUnit2->setText(strMass);
    m_pctrlLengthUnit10->setText(strLength);
    m_pctrlLengthUnit11->setText(strLength);
    m_pctrlLengthUnit12->setText(strLength);
    m_pctrlLengthUnit20->setText(strLength);
    m_pctrlLengthUnit21->setText(strLength);
    m_pctrlLengthUnit22->setText(strLength);

    m_pctrlInertiaUnit1->setText(strInertia);
    m_pctrlInertiaUnit2->setText(strInertia);
    m_pctrlInertiaUnit3->setText(strInertia);
    m_pctrlInertiaUnit4->setText(strInertia);
    m_pctrlInertiaUnit10->setText(strInertia);
    m_pctrlInertiaUnit20->setText(strInertia);
    m_pctrlInertiaUnit30->setText(strInertia);
    m_pctrlInertiaUnit40->setText(strInertia);

    m_pMassModel->setHeaderData(0, Qt::Horizontal, tr("Mass") +" ("+strMass+")");
    m_pMassModel->setHeaderData(1, Qt::Horizontal, tr("x") +" ("+strLength+")");
    m_pMassModel->setHeaderData(2, Qt::Horizontal, tr("y")+" ("+strLength+")");
    m_pMassModel->setHeaderData(3, Qt::Horizontal, tr("z")+" ("+strLength+")");
    m_pMassModel->setHeaderData(4, Qt::Horizontal, tr("Description"));


    m_pctrlWingInertia->setEnabled(false);
    m_pctrlWing2Inertia->setEnabled(false);
    m_pctrlStabInertia->setEnabled(false);
    m_pctrlFinInertia->setEnabled(false);
    m_pctrlBodyInertia->setEnabled(false);

    int rc = m_pMassModel->rowCount();
    m_pMassModel->removeRows(0, rc);

    clearPointMasses();

    if(m_pWing)
    {
        m_VolumeMass = m_pWing->m_VolumeMass;
        for(int i=0; i<m_pWing->m_PointMass.size(); i++)
        {
            m_PointMass.append(new PointMass(m_pWing->m_PointMass[i]));
        }
        m_pctrlVolumeMass->setValue(m_pWing->m_VolumeMass * Units::kgtoUnit()); //we only display half a wing, AVL way
        m_pctrlVolumeMassLabel->setText(tr("Wing Mass:"));
        m_pctrlWingInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pWing->m_WingName);
    }
    else if (m_pBody)
    {
        m_VolumeMass = m_pBody->m_VolumeMass;
        for(int i=0; i<m_pBody->m_PointMass.size(); i++)
        {
            m_PointMass.append(new PointMass(m_pBody->m_PointMass[i]->mass(), m_pBody->m_PointMass[i]->position(), m_pBody->m_PointMass[i]->tag()));
        }
        m_pctrlVolumeMass->setValue(m_pBody->m_VolumeMass * Units::kgtoUnit());
        m_pctrlVolumeMassLabel->setText(tr("Body Mass:"));
        m_pctrlBodyInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pBody->m_BodyName);
    }
    else if (m_pPlane)
    {
        m_VolumeMass = m_pPlane->wing()->m_VolumeMass;
        if(m_pPlane->biPlane()) m_VolumeMass += m_pPlane->wing2()->m_VolumeMass;
        if(m_pPlane->stab())    m_VolumeMass += m_pPlane->stab()->m_VolumeMass;
        if(m_pPlane->fin())     m_VolumeMass += m_pPlane->fin()->m_VolumeMass;
        if(m_pPlane->body())    m_VolumeMass += m_pPlane->body()->m_VolumeMass;


        for(int i=0; i<m_pPlane->m_PointMass.size(); i++)
        {
            m_PointMass.append(new PointMass(m_pPlane->m_PointMass[i]));
        }

        m_pctrlVolumeMass->setValue(m_VolumeMass * Units::kgtoUnit());
        m_pctrlVolumeMassLabel->setText(tr("Volume Mass:"));
        m_pctrlVolumeMass->setEnabled(false);
        m_pctrlWingInertia->setEnabled(true);
        if(m_pPlane->biPlane()) m_pctrlWing2Inertia->setEnabled(true);
        if(m_pPlane->stab())    m_pctrlStabInertia->setEnabled(true);
        if(m_pPlane->fin())     m_pctrlFinInertia->setEnabled(true);
        if(m_pPlane->body())    m_pctrlBodyInertia->setEnabled(true);
        setWindowTitle(tr("Inertia properties for ")+m_pPlane->planeName());
    }
    if(m_pPlane) m_pctrlTopStack->setCurrentIndex(1);
    else         m_pctrlTopStack->setCurrentIndex(0);

    fillMassModel();
    computeInertia();
    setFocus();
}


void InertiaDlg::keyPressEvent(QKeyEvent *event)
{
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
            break;
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
    double CoGIxx, CoGIyy, CoGIzz, CoGIxz;
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


    if(m_pPlane)     FileName = m_pPlane->planeName();
    else if(m_pWing) FileName = m_pWing->m_WingName;
    else if(m_pBody) FileName = m_pBody->m_BodyName;
    FileName.replace("/", " ");
    FileName += ".mass";
    FileName = QFileDialog::getSaveFileName(this, tr("Export Mass Properties"),Settings::s_LastDirName + "/"+FileName,
                                            tr("AVL Mass File (*.mass)"), &filter);
    if(!FileName.length()) return;

    int pos = FileName.lastIndexOf("/");
    if(pos>0) Settings::s_LastDirName = FileName.left(pos);

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
    if(m_pPlane)      out << "#   "+m_pPlane->planeName()+"\n";
    else if(m_pWing)  out << "#   "+m_pWing->m_WingName+"\n";
    else if(m_pBody)  out << "#   "+m_pBody->m_BodyName+"\n";
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
                strong += pWing[iw]->m_WingName;
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

    for (int i=0; i<m_PointMass.size(); i++)
    {
        if(m_PointMass[i]->mass()>0.0)
        {
            strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                    .arg(m_PointMass[i]->mass() / Munit,    10, 'g', 3)
                    .arg(m_PointMass[i]->position().x/Lunit, 10, 'g', 3)
                    .arg(m_PointMass[i]->position().y/Lunit, 10, 'g', 3)
                    .arg(m_PointMass[i]->position().z/Lunit, 10, 'g', 3);
            strong += " ! " + m_PointMass[i]->tag();
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
                for (int i=0; i<pWing[iw]->m_PointMass.size(); i++)
                {
                    if(m_pPlane->wing()->m_PointMass[i]->mass()>0.0)
                    {
                        strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                                .arg(pWing[iw]->m_PointMass[i]->mass() / Munit,    10, 'g', 3)
                                .arg((pWing[iw]->m_PointMass[i]->position().x+m_pPlane->WingLE(iw).x)/Lunit, 10, 'g', 3)
                                .arg((pWing[iw]->m_PointMass[i]->position().y+m_pPlane->WingLE(iw).y)/Lunit, 10, 'g', 3)
                                .arg((pWing[iw]->m_PointMass[i]->position().z+m_pPlane->WingLE(iw).z)/Lunit, 10, 'g', 3);

                        strong += " ! " + pWing[iw]->m_PointMass[i]->tag();
                        out << strong+"\n";
                    }
                }
            }
        }

        if(m_pPlane->body())
        {
            //fin
            for (int i=0; i<m_pPlane->body()->m_PointMass.size(); i++)
            {
                if(m_pPlane->body()->m_PointMass[i]->mass()>0.0)
                {
                    strong = QString("%1 %2 %3 %4      0.000      0.000      0.000")
                            .arg(m_pPlane->body()->m_PointMass[i]->mass() / Munit,    10, 'g', 3)
                            .arg(m_pPlane->body()->m_PointMass[i]->position().x/Lunit, 10, 'g', 3)
                            .arg(m_pPlane->body()->m_PointMass[i]->position().y/Lunit, 10, 'g', 3)
                            .arg(m_pPlane->body()->m_PointMass[i]->position().z/Lunit, 10, 'g', 3);
                    strong += " ! " + m_pPlane->body()->m_PointMass[i]->tag();
                    out << strong+"\n";
                }
            }
        }
    }

    XFile.close();
}


void InertiaDlg::onInsertMassRow()
{
    int sel;
    sel = m_pctrlMassTable->currentIndex().row();

    m_PointMass.insert(sel, new PointMass(0.0, Vector3d(0.0,0.0,0.0), ""));

    fillMassModel();
    m_pctrlMassTable->closePersistentEditor(m_pctrlMassTable->currentIndex());

    QModelIndex index = m_pMassModel->index(sel, 0, QModelIndex());
    m_pctrlMassTable->setCurrentIndex(index);
    //    m_pctrlMassTable->openPersistentEditor(index);
}


void InertiaDlg::onDeleteMassRow()
{
    int sel;
    m_pctrlMassTable->closePersistentEditor(m_pctrlMassTable->currentIndex());
    sel = m_pctrlMassTable->currentIndex().row();

    m_PointMass.removeAt(sel);

    fillMassModel();
}



void InertiaDlg::onOK()
{
    readData();

    if(m_pWing)
    {
        m_pWing->m_VolumeMass = m_VolumeMass;
        m_pWing->clearPointMasses();

        for(int i=0; i<m_PointMass.size(); i++)
        {
            if(m_PointMass[i]->mass()>PRECISION)
            {
                m_pWing->m_PointMass.append(new PointMass(m_PointMass[i]->mass(), m_PointMass[i]->position(), m_PointMass[i]->tag()));
            }
        }
    }
    else if(m_pBody)
    {
        m_pBody->m_VolumeMass = m_VolumeMass;
        m_pBody->clearPointMasses();

        for(int i=0; i< m_PointMass.size(); i++)
        {
            if(m_PointMass[i]->mass()>PRECISION)
            {
                m_pBody->m_PointMass.append(new PointMass(m_PointMass[i]->mass(), m_PointMass[i]->position(), m_PointMass[i]->tag()));
            }
        }
    }
    else if(m_pPlane)
    {
        m_pPlane->clearPointMasses();

        for(int i=0; i<m_PointMass.size(); i++)
        {
            if(m_PointMass[i]->mass()>PRECISION)
            {
                m_pPlane->m_PointMass.append(new PointMass(m_PointMass[i]->mass(), m_PointMass[i]->position(), m_PointMass[i]->tag()));
            }
        }
    }

    computeBodyAxisInertia();

    accept();
}


void InertiaDlg::readData()
{
    QModelIndex index;
    bool bOK=false;
    double mass=0, x=0,y=0,z=0;

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
            m_PointMass.append(new PointMass(mass/Units::kgtoUnit(),
                                             Vector3d(x/Units::mtoUnit(), y/Units::mtoUnit(), z/Units::mtoUnit()),
                                             tag));
        }
    }
    m_VolumeMass = m_pctrlVolumeMass->value() / Units::kgtoUnit();
}


void InertiaDlg::setupLayout()
{
    QString strMass, strLength;
    Units::getWeightUnitLabel(strMass);
    Units::getLengthUnitLabel(strLength);

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

    QSizePolicy szPolicyMinimum;
    szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
    szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

    QSizePolicy szPolicyMaximum;
    szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
    szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

    QVBoxLayout *MessageLayout = new QVBoxLayout;
    QLabel *Label1 = new QLabel(tr("This is a calculation form for a rough order of magnitude for the inertia tensor."));
    QLabel *Label2 = new QLabel(tr("Refer to the Guidelines for explanations."));
    MessageLayout->addWidget(Label1);
    MessageLayout->addWidget(Label2);

    //___________Volume Mass, Center of gravity, and inertias__________
    m_pctrlTopStack = new QStackedWidget;

    QGroupBox *pObjectMassBox = new QGroupBox(tr("Object Mass - Volume only, excluding point masses"));
    {
        QHBoxLayout *pObjectMassLayout = new QHBoxLayout;
        m_pctrlVolumeMassLabel  = new QLabel(tr("Wing Mass="));
        m_pctrlMassUnit   = new QLabel("kg");
        m_pctrlVolumeMass = new DoubleEdit(1.00,3);

        QGroupBox *pCoGBox = new QGroupBox(tr("Center of gravity"));
        {
            QGridLayout *pCoGLayout = new QGridLayout;
            {
                QLabel *CoGLabel = new QLabel(tr("Center of gravity"));
                CoGLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *XCoGLab = new QLabel("X_CoG=");
                QLabel *YCoGLab = new QLabel("Y_CoG=");
                QLabel *ZCoGLab = new QLabel("Z_CoG=");
                m_pctrlXCoG = new DoubleEdit(0.00,3);
                m_pctrlYCoG = new DoubleEdit(0.00,3);
                m_pctrlZCoG = new DoubleEdit(0.00,3);
                m_pctrlXCoG->setEnabled(false);
                m_pctrlYCoG->setEnabled(false);
                m_pctrlZCoG->setEnabled(false);
                m_pctrlLengthUnit10 = new QLabel("m");
                m_pctrlLengthUnit11 = new QLabel("m");
                m_pctrlLengthUnit12 = new QLabel("m");
                m_pctrlVolumeMassLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                XCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                YCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                ZCoGLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlXCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlYCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlZCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pCoGLayout->addWidget(m_pctrlVolumeMassLabel,1,1);
                pCoGLayout->addWidget(m_pctrlVolumeMass,1,2);
                pCoGLayout->addWidget(m_pctrlMassUnit,1,3);
                pCoGLayout->addWidget(XCoGLab,2,1);
                pCoGLayout->addWidget(YCoGLab,3,1);
                pCoGLayout->addWidget(ZCoGLab,4,1);
                //    CoGLayout->addWidget(CoGLabel,2,1);
                pCoGLayout->addWidget(m_pctrlXCoG,2,2);
                pCoGLayout->addWidget(m_pctrlYCoG,3,2);
                pCoGLayout->addWidget(m_pctrlZCoG,4,2);
                pCoGLayout->addWidget(m_pctrlLengthUnit10,2,3);
                pCoGLayout->addWidget(m_pctrlLengthUnit11,3,3);
                pCoGLayout->addWidget(m_pctrlLengthUnit12,4,3);
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
                m_pctrlCoGIxx = new DoubleEdit(1.0,3);
                m_pctrlCoGIyy = new DoubleEdit(1.2,3);
                m_pctrlCoGIzz = new DoubleEdit(-1.5,3);
                m_pctrlCoGIxz = new DoubleEdit(4.2,3);
                m_pctrlCoGIxx->setEnabled(false);
                m_pctrlCoGIyy->setEnabled(false);
                m_pctrlCoGIzz->setEnabled(false);
                m_pctrlCoGIxz->setEnabled(false);
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
                m_pctrlInertiaUnit1 = new QLabel("kg.m2");
                m_pctrlInertiaUnit2 = new QLabel("kg.m2");
                m_pctrlInertiaUnit3 = new QLabel("kg.m2");
                m_pctrlInertiaUnit4 = new QLabel("kg.m2");
                pResultsLayout->addWidget(LabIxx,1,1);
                pResultsLayout->addWidget(LabIyy,2,1);
                pResultsLayout->addWidget(LabIzz,3,1);
                pResultsLayout->addWidget(LabIxz,4,1);
                pResultsLayout->addWidget(m_pctrlCoGIxx,1,2);
                pResultsLayout->addWidget(m_pctrlCoGIyy,2,2);
                pResultsLayout->addWidget(m_pctrlCoGIzz,3,2);
                pResultsLayout->addWidget(m_pctrlCoGIxz,4,2);
                pResultsLayout->addWidget(m_pctrlInertiaUnit1,1,3);
                pResultsLayout->addWidget(m_pctrlInertiaUnit2,2,3);
                pResultsLayout->addWidget(m_pctrlInertiaUnit3,3,3);
                pResultsLayout->addWidget(m_pctrlInertiaUnit4,4,3);
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
            m_pctrlWingInertia  = new QPushButton(tr("Main Wing"));
            m_pctrlWing2Inertia = new QPushButton(tr("Second Wing"));
            m_pctrlStabInertia  = new QPushButton(tr("Elevator"));
            m_pctrlFinInertia   = new QPushButton(tr("Fin"));
            m_pctrlBodyInertia  = new QPushButton(tr("Body"));
            QHBoxLayout *pWingInertiasLayout = new QHBoxLayout;
            {
                pWingInertiasLayout->addStretch();
                pWingInertiasLayout->addWidget(m_pctrlWingInertia);
                pWingInertiasLayout->addStretch();
                pWingInertiasLayout->addWidget(m_pctrlWing2Inertia);
                pWingInertiasLayout->addStretch();
            }
            QVBoxLayout *pAxisInertiasLayout = new QVBoxLayout;
            {
                QHBoxLayout *pStabLayout = new QHBoxLayout;
                {
                    pStabLayout->addStretch();
                    pStabLayout->addWidget(m_pctrlStabInertia);
                    pStabLayout->addStretch();
                }
                QHBoxLayout *pFinStabLayout = new QHBoxLayout;
                {
                    pFinStabLayout->addStretch();
                    pFinStabLayout->addWidget(m_pctrlFinInertia);
                    pFinStabLayout->addStretch();
                }
                QHBoxLayout *pBodyLayout = new QHBoxLayout;
                {
                    pBodyLayout->addStretch();
                    pBodyLayout->addWidget(m_pctrlBodyInertia);
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

    m_pctrlTopStack->addWidget(pObjectMassBox);
    m_pctrlTopStack->addWidget(pObjectSelectionBox);

    //___________________Point Masses__________________________
    QLabel *pPointMasses = new QLabel(tr("Additional Point Masses"));
    m_pctrlMassTable = new QTableView(this);
    m_pctrlMassTable->setSizePolicy(szPolicyExpanding);
    m_pctrlMassTable->setFont(Settings::s_TableFont);
    m_pctrlMassTable->setMinimumHeight(150);
    m_pctrlMassTable->horizontalHeader()->setStretchLastSection(true);
    m_pctrlMassTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pctrlMassTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pctrlMassTable->setEditTriggers(QAbstractItemView::CurrentChanged |
                                      QAbstractItemView::DoubleClicked |
                                      QAbstractItemView::SelectedClicked |
                                      QAbstractItemView::EditKeyPressed |
                                      QAbstractItemView::AnyKeyPressed);


    m_pMassModel = new QStandardItemModel(this);
    m_pMassModel->setRowCount(10);//temporary
    m_pMassModel->setColumnCount(5);

    m_pctrlMassTable->setModel(m_pMassModel);

    m_pFloatDelegate = new FloatEditDelegate(this);
    m_pctrlMassTable->setItemDelegate(m_pFloatDelegate);
    m_precision = new int[5];
    m_precision[0] = 3;
    m_precision[1] = 3;
    m_precision[2] = 3;
    m_precision[3] = 3;
    m_precision[4] = -1;// Not a number, will be detected as such by FloatEditDelegate
    m_pFloatDelegate->setPrecision(m_precision);

    connect(m_pFloatDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onCellChanged(QWidget *)));

    //________________Total Mass, Center of gravity, and inertias__________
    QGroupBox *pTotalMassBox = new QGroupBox(tr("Total Mass = Volume + point masses"));
    {
        QHBoxLayout *pTotalMassLayout = new QHBoxLayout;
        m_pctrlTotalMassLabel   = new QLabel(tr("Total Mass="));
        m_pctrlMassUnit2        = new QLabel("kg");
        m_pctrlTotalMass        = new DoubleEdit(1.00,3);
        m_pctrlTotalMass->setEnabled(false);

        QGroupBox *pTotalCoGBox = new QGroupBox(tr("Center of gravity"));
        {
            QGridLayout *pTotalCoGLayout = new QGridLayout;
            {
                QLabel *pTotalLabel = new QLabel(tr("Center of gravity"));
                pTotalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *pXTotalLab = new QLabel("X_CoG=");
                QLabel *pYTotalLab = new QLabel("Y_CoG=");
                QLabel *pZTotalLab = new QLabel("Z_CoG=");
                m_pctrlXTotalCoG = new DoubleEdit(0.00,3);
                m_pctrlYTotalCoG = new DoubleEdit(0.00,3);
                m_pctrlZTotalCoG = new DoubleEdit(0.00,3);
                m_pctrlXTotalCoG->setEnabled(false);
                m_pctrlYTotalCoG->setEnabled(false);
                m_pctrlZTotalCoG->setEnabled(false);
                m_pctrlLengthUnit20 = new QLabel("m");
                m_pctrlLengthUnit21 = new QLabel("m");
                m_pctrlLengthUnit22 = new QLabel("m");
                m_pctrlTotalMassLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pXTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pYTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pZTotalLab->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlXTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlYTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                m_pctrlZTotalCoG->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
                pTotalCoGLayout->addWidget(m_pctrlTotalMassLabel,1,1);
                pTotalCoGLayout->addWidget(m_pctrlTotalMass,1,2);
                pTotalCoGLayout->addWidget(m_pctrlMassUnit2,1,3);
                pTotalCoGLayout->addWidget(pXTotalLab,2,1);
                pTotalCoGLayout->addWidget(pYTotalLab,3,1);
                pTotalCoGLayout->addWidget(pZTotalLab,4,1);
                pTotalCoGLayout->addWidget(m_pctrlXTotalCoG,2,2);
                pTotalCoGLayout->addWidget(m_pctrlYTotalCoG,3,2);
                pTotalCoGLayout->addWidget(m_pctrlZTotalCoG,4,2);
                pTotalCoGLayout->addWidget(m_pctrlLengthUnit20,2,3);
                pTotalCoGLayout->addWidget(m_pctrlLengthUnit21,3,3);
                pTotalCoGLayout->addWidget(m_pctrlLengthUnit22,4,3);
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
                m_pctrlTotalIxx = new DoubleEdit(1.0,5);
                m_pctrlTotalIyy = new DoubleEdit(1.2,5);
                m_pctrlTotalIzz = new DoubleEdit(-1.5,5);
                m_pctrlTotalIxz = new DoubleEdit(4.2,5);
                m_pctrlTotalIxx->setEnabled(false);
                m_pctrlTotalIyy->setEnabled(false);
                m_pctrlTotalIzz->setEnabled(false);
                m_pctrlTotalIxz->setEnabled(false);
                QLabel *LabTotIxx = new QLabel("Ixx=");
                QLabel *LabTotIyy = new QLabel("Iyy=");
                QLabel *LabTotIzz = new QLabel("Izz=");
                QLabel *LabTotIxz = new QLabel("Ixz=");
                LabTotIxx->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabTotIyy->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabTotIzz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                LabTotIxz->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QLabel *LabInertiaTotal = new QLabel(tr("Inertia in CoG Frame"));
                LabInertiaTotal->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                m_pctrlInertiaUnit10 = new QLabel("kg.m2");
                m_pctrlInertiaUnit20 = new QLabel("kg.m2");
                m_pctrlInertiaUnit30 = new QLabel("kg.m2");
                m_pctrlInertiaUnit40 = new QLabel("kg.m2");
                pTotalInertiaLayout->addWidget(LabTotIxx,1,1);
                pTotalInertiaLayout->addWidget(LabTotIyy,2,1);
                pTotalInertiaLayout->addWidget(LabTotIzz,3,1);
                pTotalInertiaLayout->addWidget(LabTotIxz,4,1);
                pTotalInertiaLayout->addWidget(m_pctrlTotalIxx,1,2);
                pTotalInertiaLayout->addWidget(m_pctrlTotalIyy,2,2);
                pTotalInertiaLayout->addWidget(m_pctrlTotalIzz,3,2);
                pTotalInertiaLayout->addWidget(m_pctrlTotalIxz,4,2);
                pTotalInertiaLayout->addWidget(m_pctrlInertiaUnit10,1,3);
                pTotalInertiaLayout->addWidget(m_pctrlInertiaUnit20,2,3);
                pTotalInertiaLayout->addWidget(m_pctrlInertiaUnit30,3,3);
                pTotalInertiaLayout->addWidget(m_pctrlInertiaUnit40,4,3);
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
        m_pctrlExportToAVL = new QPushButton(tr("Export to AVL"));
        m_pButtonBox->addButton(m_pctrlExportToAVL, QDialogButtonBox::ActionRole);
        connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
    }

    QVBoxLayout * pMainLayout = new QVBoxLayout(this);
    {
        pMainLayout->addLayout(MessageLayout);
        pMainLayout->addWidget(m_pctrlTopStack);
        pMainLayout->addWidget(pPointMasses);
        pMainLayout->addWidget(m_pctrlMassTable);
        pMainLayout->addWidget(pTotalMassBox);
        pMainLayout->addWidget(m_pButtonBox);
    }
    setLayout(pMainLayout);

    connect(m_pctrlWingInertia,  SIGNAL(clicked()), this, SLOT(onWingInertia()));
    connect(m_pctrlWing2Inertia, SIGNAL(clicked()), this, SLOT(onWing2Inertia()));
    connect(m_pctrlStabInertia,  SIGNAL(clicked()), this, SLOT(onStabInertia()));
    connect(m_pctrlFinInertia,   SIGNAL(clicked()), this, SLOT(onFinInertia()));
    connect(m_pctrlBodyInertia,  SIGNAL(clicked()), this, SLOT(onBodyInertia()));
    connect(m_pctrlVolumeMass,   SIGNAL(editingFinished()), SLOT(onVolumeMass()));
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
    if(!m_pctrlMassTable || !m_pMassModel) return;
    int w = m_pctrlMassTable->width();
    int w7 = int((double)(w-25)/7);

    m_pctrlMassTable->setColumnWidth(0,w7);
    m_pctrlMassTable->setColumnWidth(1,w7);
    m_pctrlMassTable->setColumnWidth(2,w7);
    m_pctrlMassTable->setColumnWidth(3,w7);
    m_pctrlMassTable->setColumnWidth(4,3*w7);

    pEvent->accept();
}


void InertiaDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}



void InertiaDlg::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}





