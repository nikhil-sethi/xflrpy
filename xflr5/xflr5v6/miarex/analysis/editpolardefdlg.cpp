/****************************************************************************

    EditPolarDefDlg Class
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


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFontMetrics>
#include <QMessageBox>
#include <QShowEvent>
#include <QHideEvent>
#include <QDebug>

#include "editpolardefdlg.h"

#include <xflobjects/editors/editobjectdelegate.h>
#include <xflanalysis/analysis3d_globals.h>
#include <xflcore/core_enums.h>
#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/plane.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>


QByteArray EditPolarDefDlg::s_Geometry;

EditPolarDefDlg::EditPolarDefDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle("Polar object explorer");
    m_pWPolar = nullptr;
    m_pPlane = nullptr;
    setupLayout();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void EditPolarDefDlg::showEvent(QShowEvent *pEvent)
{
    restoreGeometry(s_Geometry);
    pEvent->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void EditPolarDefDlg::hideEvent(QHideEvent *pEvent)
{
    s_Geometry = saveGeometry();
    pEvent->accept();
}


void EditPolarDefDlg::resizeEvent(QResizeEvent *pEvent)
{
    int ColumnWidth = int(double(m_pStruct->width())/4.0);
    m_pStruct->setColumnWidth(0,ColumnWidth);
    m_pStruct->setColumnWidth(1,ColumnWidth);
    m_pStruct->setColumnWidth(2,ColumnWidth);
    pEvent->accept();
}


void EditPolarDefDlg::setupLayout()
{
    QStringList labels;
    labels << tr("Object") << tr("Field")<<tr("Value")<<tr("Unit");

    m_pStruct = new QTreeView;
#if QT_VERSION >= 0x050000
    m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif
    //    m_pPlaneStruct->header()->setDefaultSectionSize(239);

    m_pStruct->header()->setStretchLastSection(true);
    m_pStruct->header()->setDefaultAlignment(Qt::AlignCenter);

    //    m_pStruct->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
    //    m_pStruct->setIndentation(31);
    m_pStruct->setWindowTitle(tr("Objects"));

    m_pModel = new QStandardItemModel(this);
    m_pModel->setColumnCount(4);
    m_pModel->clear();
    m_pModel->setHorizontalHeaderLabels(labels);

    m_pStruct->setModel(m_pModel);

    /*    QItemSelectionModel *pSelectionModel = new QItemSelectionModel(m_pModel);
    m_pStruct->setSelectionModel(pSelectionModel);
    connect(pSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));*/

    connect(m_pModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(onItemChanged()));

    m_pDelegate = new EditObjectDelegate(this);
    m_pStruct->setItemDelegate(m_pDelegate);

    QFont fnt;
    QFontMetrics fm(fnt);
    m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
    m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
    m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);


    QVBoxLayout *pVBox = new QVBoxLayout;
    {
        pVBox->addWidget(m_pStruct);

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Discard);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
        }

        pVBox->addWidget(m_pButtonBox);
    }
    setLayout(pVBox);
}


void EditPolarDefDlg::onButton(QAbstractButton *pButton)
{
    if (     m_pButtonBox->button(QDialogButtonBox::Ok)      == pButton)  accept();
    else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)  reject();
}


void EditPolarDefDlg::keyPressEvent(QKeyEvent *pEvent)
{
    // Prevent Return Key from closing App
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus())
            {
                readData();
                m_pButtonBox->setFocus();
                return;
            }
            break;
        }
        case Qt::Key_Escape:
        {
            reject();
            break;
        }
        default:
            pEvent->ignore();
    }
}


void EditPolarDefDlg::onItemChanged()
{
    readData();

    m_pWPolar->setPolarName("a wing polar name");
    xfl::setAutoWPolarName(m_pWPolar, m_pPlane);

    QModelIndex indexLevel = m_pModel->index(0,0);

    do
    {
        QString field  = indexLevel.sibling(indexLevel.row(),1).data().toString();
        if (field.compare("Name")==0)
        {
            m_pModel->setData(indexLevel.sibling(indexLevel.row(),2), m_pWPolar->polarName());
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}


void EditPolarDefDlg::accept()
{
    readData();

    if (m_pWPolar->analysisMethod()==xfl::VLMMETHOD)
    {
        m_pWPolar->setThinSurfaces(true);
        m_pWPolar->setAnalysisMethod(xfl::PANEL4METHOD);
    }
    else if (m_pWPolar->analysisMethod()==xfl::PANEL4METHOD && !m_pPlane->isWing())
    {
        m_pWPolar->setThinSurfaces(true);
    }
    else if (m_pWPolar->analysisMethod()==xfl::PANEL4METHOD && m_pPlane->isWing())
    {
        m_pWPolar->setThinSurfaces(false);
    }
    QDialog::accept();
}


QList<QStandardItem *> EditPolarDefDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
    for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> EditPolarDefDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::BOOLVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> EditPolarDefDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem);

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::INTEGER, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


QList<QStandardItem *> EditPolarDefDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
{
    QList<QStandardItem *> rowItems;
    rowItems.append(new QStandardItem(object));
    rowItems.append(new QStandardItem(field));
    rowItems.append(new QStandardItem);
    rowItems.at(2)->setData(value, Qt::DisplayRole);
    rowItems.append(new QStandardItem(unit));

    rowItems.at(0)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(1)->setData(xfl::STRING, Qt::UserRole);
    rowItems.at(2)->setData(xfl::DOUBLEVALUE, Qt::UserRole);
    rowItems.at(3)->setData(xfl::STRING, Qt::UserRole);
    return rowItems;
}


void EditPolarDefDlg::initDialog(Plane *pPlane, WPolar *pWPolar)
{
    m_pPlane  = pPlane;
    m_pWPolar = pWPolar;

    xfl::setAutoWPolarName(m_pWPolar, m_pPlane);

    showWPolar();
    m_pStruct->expandAll();
}


void EditPolarDefDlg::showWPolar()
{
    QList<QStandardItem*> dataItem;

    QStandardItem *rootItem = m_pModel->invisibleRootItem();
    dataItem = prepareRow("Polar Name", "Name", m_pWPolar->polarName());
    rootItem->appendRow(dataItem);

    QList<QStandardItem*> polarTypeFolder = prepareRow("Polar Type");
    rootItem->appendRow(polarTypeFolder);
    {
        dataItem = prepareRow("", "Type", WPolarType(m_pWPolar->polarType()));
        dataItem.at(2)->setData(xfl::POLARTYPE, Qt::UserRole);
        polarTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Velocity", m_pWPolar->velocity(), Units::speedUnitLabel());
        polarTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Alpha", m_pWPolar->Alpha(), QChar(0260));
        polarTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Beta", m_pWPolar->Beta(), QChar(0260));
        polarTypeFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> analysisTypeFolder = prepareRow("Analysis Type");
    rootItem->appendRow(analysisTypeFolder);
    {
        if(m_pWPolar->analysisMethod()==xfl::LLTMETHOD) dataItem = prepareRow("", "Method", "LLTMETHOD");
        else if(m_pWPolar->bThinSurfaces())               dataItem = prepareRow("", "Method", "VLMMETHOD");
        else                                              dataItem = prepareRow("", "Method", "PANELMETHOD");
        dataItem.at(2)->setData(xfl::ANALYSISMETHOD, Qt::UserRole);
        analysisTypeFolder.first()->appendRow(dataItem);

        if(m_pWPolar->bDirichlet())  dataItem = prepareRow("", "Boundary condition", "DIRICHLET");
        else                         dataItem = prepareRow("", "Boundary condition", "NEUMANN");
        dataItem.at(2)->setData(xfl::BOUNDARYCONDITION, Qt::UserRole);
        analysisTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareBoolRow("", "Viscous", m_pWPolar->bViscous());
        analysisTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareBoolRow("", "Tilted geometry", m_pWPolar->bTilted());
        analysisTypeFolder.first()->appendRow(dataItem);

        dataItem = prepareBoolRow("", "Ignore body panels", m_pWPolar->bIgnoreBodyPanels());
        analysisTypeFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> inertiaFolder = prepareRow("Inertia");
    rootItem->appendRow(inertiaFolder);
    fillInertiaData(inertiaFolder);

    QList<QStandardItem*> refDimensionsFolder = prepareRow("Reference Dimensions");
    rootItem->appendRow(refDimensionsFolder);
    {
        dataItem = prepareRow("", "Reference dimensions", referenceDimension(m_pWPolar->referenceDim()));
        dataItem.at(2)->setData(xfl::REFDIMENSIONS, Qt::UserRole);
        refDimensionsFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Reference Area", m_pWPolar->referenceArea() * Units::m2toUnit(), Units::areaUnitLabel());
        refDimensionsFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Reference Span Length", m_pWPolar->referenceSpanLength() * Units::mtoUnit(), Units::lengthUnitLabel());
        refDimensionsFolder.first()->appendRow(dataItem);

        dataItem = prepareDoubleRow("", "Reference Chord Length", m_pWPolar->referenceChordLength() * Units::mtoUnit(), Units::lengthUnitLabel());
        refDimensionsFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> environmentDataFolder = prepareRow("Environment data");
    rootItem->appendRow(environmentDataFolder);
    {
        QList<QStandardItem*> aeroDataFolder = prepareRow("Air data");
        environmentDataFolder.first()->appendRow(aeroDataFolder);
        {
            dataItem = prepareDoubleRow("", "Density", m_pWPolar->density(), "kg/m3");
            aeroDataFolder.first()->appendRow(dataItem);

            dataItem = prepareDoubleRow("", "Viscosity", m_pWPolar->viscosity(), QString::fromUtf8("m²/s"));
            aeroDataFolder.first()->appendRow(dataItem);
        }
        QList<QStandardItem*> heightFolder = prepareRow("Ground height data");
        environmentDataFolder.first()->appendRow(heightFolder);
        {
            dataItem = prepareBoolRow("", "Ground effect", m_pWPolar->bGround());
            heightFolder.first()->appendRow(dataItem);

            dataItem = prepareDoubleRow("", "Height flight", m_pWPolar->groundHeight()*Units::mtoUnit(), Units::lengthUnitLabel());
            heightFolder.first()->appendRow(dataItem);
        }
    }

    QList<QStandardItem*> stabControlFolder = prepareRow("Stability Controls");
    rootItem->appendRow(stabControlFolder);
    fillControlFields(stabControlFolder);
}


void EditPolarDefDlg::fillInertiaData(QList<QStandardItem *> inertiaFolder)
{
    QList<QStandardItem*> dataItem;

    if(m_pWPolar->m_bAutoInertia)
    {
        m_pWPolar->setMass(m_pPlane->totalMass());
        m_pWPolar->setCoG(m_pPlane->CoG());
        m_pWPolar->setCoGIxx(m_pPlane->m_CoGIxx);
        m_pWPolar->setCoGIyy(m_pPlane->m_CoGIyy);
        m_pWPolar->setCoGIzz(m_pPlane->m_CoGIzz);
        m_pWPolar->setCoGIxz(m_pPlane->m_CoGIxz);
    }

    dataItem = prepareBoolRow("", "Use plane inertia", m_pWPolar->bAutoInertia());
    inertiaFolder.first()->appendRow(dataItem);

    dataItem = prepareDoubleRow("", "Mass", m_pWPolar->mass()*Units::kgtoUnit(), Units::massUnitLabel());
    inertiaFolder.first()->appendRow(dataItem);

    QList<QStandardItem*> cogFolder = prepareRow("Center of Gravity");
    inertiaFolder.first()->appendRow(cogFolder);
    {
        dataItem = prepareDoubleRow("", "x", m_pWPolar->CoG().x*Units::mtoUnit(), Units::lengthUnitLabel());
        cogFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "z", m_pWPolar->CoG().z*Units::mtoUnit(), Units::lengthUnitLabel());
        cogFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> inertiaTensorFolder = prepareRow("Inertia tensor");
    inertiaFolder.first()->appendRow(inertiaTensorFolder);
    {
        dataItem = prepareDoubleRow("", "Ixx", m_pWPolar->CoGIxx()*Units::kgm2toUnit(), Units::inertiaUnitLabel());
        inertiaTensorFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Iyy", m_pWPolar->CoGIyy()*Units::kgm2toUnit(), Units::inertiaUnitLabel());
        inertiaTensorFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Izz", m_pWPolar->CoGIzz()*Units::kgm2toUnit(), Units::inertiaUnitLabel());
        inertiaTensorFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Ixz", m_pWPolar->CoGIxz()*Units::kgm2toUnit(), Units::inertiaUnitLabel());
        inertiaTensorFolder.first()->appendRow(dataItem);
    }
}


void EditPolarDefDlg::readData()
{
    readViewLevel(m_pModel->index(0,0));
    if (m_pWPolar->analysisMethod() == xfl::LLTMETHOD)
    {
        m_pWPolar->setViscous(true) ;
        m_pWPolar->setThinSurfaces(true);
        m_pWPolar->setWakeRollUp(false);
        m_pWPolar->setTilted(false);
    }
    else if (m_pWPolar->analysisMethod() == xfl::VLMMETHOD)
    {
        m_pWPolar->setThinSurfaces(true);
        //        m_pWPolar->analysisMethod() = XFLR5::PANELMETHOD;
    }
    else if (m_pWPolar->analysisMethod() == xfl::PANEL4METHOD)
    {
        m_pWPolar->setThinSurfaces(false);
    }

    //    m_pWPolar->bThinSurfaces() = m_pWPolar->analysisMethod()==XFLR5::PANELMETHOD && m_pPlane->isWing();
    return;
}



void EditPolarDefDlg::readViewLevel(QModelIndex indexLevel)
{
    do
    {
        QStandardItem *pItem = m_pModel->itemFromIndex(indexLevel);
        if(!pItem) return;
        else if(pItem->child(0,0))
        {
            QString object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            if(object.compare("Stability Controls")==0) readControlFields(pItem->child(0,0)->index());
            else                                        readViewLevel(pItem->child(0,0)->index());
        }
        else
        {
            //no more children
            QString object = indexLevel.sibling(indexLevel.row(),0).data().toString();
            QString field = indexLevel.sibling(indexLevel.row(),1).data().toString();

            QModelIndex dataIndex = indexLevel.sibling(indexLevel.row(),2);
            QString value = indexLevel.sibling(indexLevel.row(),2).data().toString();

            if     (field.compare("Name")==0)                    m_pWPolar->setPolarName(value);
            else if(field.compare("Type")==0)                    m_pWPolar->setPolarType(xfl::WPolarType(value));
            else if(field.compare("Velocity")==0)                m_pWPolar->setVelocity(dataIndex.data().toDouble()/Units::mstoUnit());
            else if(field.compare("Alpha")==0)                   m_pWPolar->setAlpha(dataIndex.data().toDouble());
            else if(field.compare("Beta")==0)                    m_pWPolar->setBeta(dataIndex.data().toDouble());
            else if(field.compare("Method")==0)                  m_pWPolar->setAnalysisMethod(xfl::analysisMethod(value));
            else if(field.compare("Boundary condition")==0)      m_pWPolar->setBoundaryCondition(xfl::boundaryCondition(value));
            else if(field.compare("Viscous")==0)                 m_pWPolar->setViscous(xfl::stringToBool(value));
            else if(field.compare("Tilted geometry")==0)         m_pWPolar->setTilted(xfl::stringToBool(value));
            else if(field.compare("Ignore body panels")==0)      m_pWPolar->setIgnoreBodyPanels(xfl::stringToBool(value));
            else if(field.compare("Use plane inertia")==0)       m_pWPolar->setAutoInertia(xfl::stringToBool(value));
            else if(field.compare("Mass")==0)                    m_pWPolar->setMass(dataIndex.data().toDouble()/Units::kgtoUnit());
            else if(field.compare("x")==0)                       m_pWPolar->setCoGx(dataIndex.data().toDouble()/Units::mtoUnit());
            else if(field.compare("z")==0)                       m_pWPolar->setCoGz(dataIndex.data().toDouble()/Units::mtoUnit());
            else if(field.compare("Ixx")==0)                     m_pWPolar->setCoGIxx(dataIndex.data().toDouble()/Units::kgm2toUnit());
            else if(field.compare("Iyy")==0)                     m_pWPolar->setCoGIyy(dataIndex.data().toDouble()/Units::kgm2toUnit());
            else if(field.compare("Izz")==0)                     m_pWPolar->setCoGIzz(dataIndex.data().toDouble()/Units::kgm2toUnit());
            else if(field.compare("Ixz")==0)                     m_pWPolar->setCoGIxz(dataIndex.data().toDouble()/Units::kgm2toUnit());
            else if(field.compare("Area")==0)                    m_pWPolar->setReferenceDim(xfl::referenceDimension(value));
            else if(field.compare("Reference Area")==0)          m_pWPolar->setReferenceArea(dataIndex.data().toDouble()/Units::m2toUnit());
            else if(field.compare("Reference Span Length")==0)   m_pWPolar->setReferenceSpanLength(dataIndex.data().toDouble()/Units::mtoUnit());
            else if(field.compare("Reference Chord Length")==0)  m_pWPolar->setReferenceChordLength(dataIndex.data().toDouble()/Units::mtoUnit());
            else if(field.compare("Density")==0)                 m_pWPolar->setDensity(dataIndex.data().toDouble());
            else if(field.compare("Viscosity")==0)               m_pWPolar->setViscosity(dataIndex.data().toDouble());
            else if(field.compare("Ground effect")==0)           m_pWPolar->setGroundEffect(xfl::stringToBool(value));
            else if(field.compare("Height flight")==0)           m_pWPolar->setGroundHeight(dataIndex.data().toDouble()/Units::mtoUnit());

        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}



void EditPolarDefDlg::readControlFields(QModelIndex indexLevel)
{
    if(!indexLevel.isValid()) return;

    do
    {
        QString object = indexLevel.sibling(indexLevel.row(),0).data().toString();
        QString field  = indexLevel.sibling(indexLevel.row(),1).data().toString();
        QString value  = indexLevel.sibling(indexLevel.row(),2).data().toString();
        QStandardItem *pItem = m_pModel->item(indexLevel.row());

        if(pItem->child(0,0))
        {

            if(object.compare("Mass gains", Qt::CaseInsensitive)==0)
            {
                //no more children
                QModelIndex childIndex= pItem->child(0,0)->index();
                do
                {
                    QString childObject = childIndex.sibling(childIndex.row(),0).data().toString();
                    QString childField = childIndex.sibling(childIndex.row(),1).data().toString();
                    QString childValue = childIndex.sibling(childIndex.row(),2).data().toString();

                    QModelIndex dataIndex = childIndex.sibling(childIndex.row(),2);

                    if     (childField.compare("Mass")==0)   m_pWPolar->m_inertiaGain[0]  = dataIndex.data().toInt()/Units::kgtoUnit();
                    else if(childField.compare("CoG.x")==0)  m_pWPolar->m_inertiaGain[1]  = dataIndex.data().toDouble()/Units::mtoUnit();
                    else if(childField.compare("CoG.z")==0)  m_pWPolar->m_inertiaGain[2]  = dataIndex.data().toDouble()/Units::mtoUnit();
                    else if(childField.compare("Ixx")==0)    m_pWPolar->m_inertiaGain[3]  = dataIndex.data().toDouble()/Units::kgm2toUnit();
                    else if(childField.compare("Iyy")==0)    m_pWPolar->m_inertiaGain[4]  = dataIndex.data().toDouble()/Units::kgm2toUnit();
                    else if(childField.compare("Izz")==0)    m_pWPolar->m_inertiaGain[5]  = dataIndex.data().toDouble()/Units::kgm2toUnit();
                    else if(childField.compare("Ixz")==0)    m_pWPolar->m_inertiaGain[6]  = dataIndex.data().toDouble()/Units::kgm2toUnit();
                    childIndex = childIndex.sibling(childIndex.row()+1,0);

                } while(childIndex.isValid());
            }
            else  if(object.compare("Angle gains", Qt::CaseInsensitive)==0)
            {
                QString field, value;
                int nControls = 0;
                QStandardItem *pItem = m_pModel->item(indexLevel.row());
                QModelIndex childIndex= pItem->child(0,0)->index();
                field = childIndex.sibling(childIndex.row(),1).data().toString();
                value = childIndex.sibling(childIndex.row(),2).data().toString();

                if(!m_pPlane->isWing())
                {
                    m_pWPolar->m_ControlGain[0] = value.toDouble();
                    ++nControls;

                    if(m_pPlane->stab())
                    {
                        childIndex = childIndex.sibling(childIndex.row()+1,0);
                        if(childIndex.isValid())
                        {
                            field = childIndex.sibling(childIndex.row(),1).data().toString();
                            value = childIndex.sibling(childIndex.row(),2).data().toString();
                            m_pWPolar->m_ControlGain[1] = value.toDouble();
                            ++nControls;
                        }
                        else return;
                    }
                }

                for(int i=0; i<m_pPlane->wing()->nFlaps(); i++)
                {
                    childIndex = childIndex.sibling(childIndex.row()+1,0);
                    if(childIndex.isValid())
                    {
                        field = childIndex.sibling(childIndex.row(),1).data().toString();
                        value = childIndex.sibling(childIndex.row(),2).data().toString();
                        m_pWPolar->m_ControlGain[nControls] = value.toDouble();
                        ++nControls;
                    }
                    else return;
                }


                if(m_pPlane->stab())
                {
                    for(int i=0; i<m_pPlane->stab()->nFlaps(); i++)
                    {
                        childIndex = childIndex.sibling(childIndex.row()+1,0);
                        if(childIndex.isValid())
                        {
                            field = childIndex.sibling(childIndex.row(),1).data().toString();
                            value = childIndex.sibling(childIndex.row(),2).data().toString();
                            m_pWPolar->m_ControlGain[nControls] = value.toDouble();
                            ++nControls;
                        }
                        else return;
                    }
                }

                if(m_pPlane->fin())
                {
                    for(int i=0; i<m_pPlane->fin()->nFlaps(); i++)
                    {
                        childIndex = childIndex.sibling(childIndex.row()+1,0);
                        if(childIndex.isValid())
                        {
                            field = childIndex.sibling(childIndex.row(),1).data().toString();
                            value = childIndex.sibling(childIndex.row(),2).data().toString();
                            m_pWPolar->m_ControlGain[nControls] = value.toDouble();
                            ++nControls;
                        }
                        else return;
                    }
                }
            }
        }

        indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

    } while(indexLevel.isValid());
}



void EditPolarDefDlg::fillControlFields(QList<QStandardItem*> stabControlFolder)
{
    QList<QStandardItem*> dataItem;
    QString strLength, strMass, strInertia;

    int i;
    strLength  = Units::lengthUnitLabel();
    strMass    = Units::massUnitLabel();
    strInertia = Units::inertiaUnitLabel();

    QList<QStandardItem*> massCtrlFolder = prepareRow("Mass gains");
    stabControlFolder.first()->appendRow(massCtrlFolder);
    {
        dataItem = prepareDoubleRow("", "Mass",  m_pWPolar->m_inertiaGain[0]*Units::kgtoUnit(), strMass+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "CoG.x", m_pWPolar->m_inertiaGain[1]*Units::mtoUnit(), strLength+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "CoG.z", m_pWPolar->m_inertiaGain[2]*Units::mtoUnit(), strLength+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Ixx",   m_pWPolar->m_inertiaGain[3]*Units::kgm2toUnit(), strInertia+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Iyy",   m_pWPolar->m_inertiaGain[4]*Units::kgm2toUnit(), strInertia+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Izz",   m_pWPolar->m_inertiaGain[5]*Units::kgm2toUnit(), strInertia+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
        dataItem = prepareDoubleRow("", "Ixz",   m_pWPolar->m_inertiaGain[6]*Units::kgm2toUnit(), strInertia+"/ctrl");
        massCtrlFolder.first()->appendRow(dataItem);
    }

    QList<QStandardItem*> angleCtrlFolder = prepareRow("Angle gains");
    stabControlFolder.first()->appendRow(angleCtrlFolder);
    {
        m_pWPolar->m_nControls = 0;
        if(!m_pPlane->isWing())
        {
            if(m_pWPolar->m_ControlGain.size()<=m_pWPolar->m_nControls)m_pWPolar->m_ControlGain.append(0.0);

            dataItem = prepareDoubleRow("", "Wing Tilt ", m_pWPolar->m_ControlGain[0], QString::fromUtf8("°/ctrl"));
            angleCtrlFolder.first()->appendRow(dataItem);

            ++m_pWPolar->m_nControls;

            if(m_pPlane->stab())
            {
                if(m_pWPolar->m_ControlGain.size()<=m_pWPolar->m_nControls)m_pWPolar->m_ControlGain.append(0.0);
                dataItem = prepareDoubleRow("", "Elevator Tilt ", m_pWPolar->m_ControlGain[1], QString::fromUtf8("°/ctrl"));
                angleCtrlFolder.first()->appendRow(dataItem);

                ++m_pWPolar->m_nControls;
            }
        }

        for(i=0; i<m_pPlane->wing()->nFlaps(); i++)
        {
            if(m_pWPolar->m_ControlGain.size()<=m_pWPolar->m_nControls)m_pWPolar->m_ControlGain.append(0.0);
            dataItem = prepareDoubleRow("", QString("Wing Flap %1 ").arg(i+1), m_pWPolar->m_ControlGain[m_pWPolar->m_nControls], QString::fromUtf8("°/ctrl"));
            angleCtrlFolder.first()->appendRow(dataItem);
            ++m_pWPolar->m_nControls;
        }


        if(m_pPlane->stab())
        {
            for(i=0; i<m_pPlane->stab()->nFlaps(); i++)
            {
                if(m_pWPolar->m_ControlGain.size()<=m_pWPolar->m_nControls)m_pWPolar->m_ControlGain.append(0.0);
                dataItem = prepareDoubleRow("", QString("Elevator Flap %1 ").arg(i+1), m_pWPolar->m_ControlGain[m_pWPolar->m_nControls], QString::fromUtf8("°/ctrl"));
                angleCtrlFolder.first()->appendRow(dataItem);
            }
            m_pWPolar->m_nControls += m_pPlane->stab()->nFlaps();
        }

        if(m_pPlane->fin())
        {
            for(i=0; i<m_pPlane->fin()->nFlaps(); i++)
            {
                if(m_pWPolar->m_ControlGain.size()<=m_pWPolar->m_nControls)m_pWPolar->m_ControlGain.append(0.0);
                dataItem = prepareDoubleRow("", QString("Fin Flap %1 ").arg(i+1), m_pWPolar->m_ControlGain[m_pWPolar->m_nControls], QString::fromUtf8("°/ctrl"));
                angleCtrlFolder.first()->appendRow(dataItem);
                ++m_pWPolar->m_nControls;
            }
        }
    }
}










