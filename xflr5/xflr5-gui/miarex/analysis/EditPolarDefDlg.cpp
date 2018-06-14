/****************************************************************************

	Techwing Application

	Copyright (C) Andre Deperrois techwinder@gmail.com

	All rights reserved.

*****************************************************************************/

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFontMetrics>
#include <QMessageBox>
#include <QShowEvent>
#include <QHideEvent>
#include <QtDebug>
#include "EditPolarDefDlg.h"
#include <analysis3d/analysis3d_globals.h>
#include <misc/options/Units.h>
#include <globals/globals.h>
#include <globals/gui_enums.h>


QSize EditPolarDefDlg::s_Size(579,783);
QPoint EditPolarDefDlg::s_Position(231, 97);


EditPolarDefDlg::EditPolarDefDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Polar object explorer");
	m_pWPolar = NULL;
	m_pPlane = NULL;
	setupLayout();
}



/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void EditPolarDefDlg::showEvent(QShowEvent *event)
{
	move(s_Position);
	event->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void EditPolarDefDlg::hideEvent(QHideEvent *event)
{
	s_Position = pos();
	event->accept();
}


void EditPolarDefDlg::resizeEvent(QResizeEvent *event)
{
	s_Size = size();
	int ColumnWidth = (int)((double)(m_pStruct->width())/4);
	m_pStruct->setColumnWidth(0,ColumnWidth);
	m_pStruct->setColumnWidth(1,ColumnWidth);
	m_pStruct->setColumnWidth(2,ColumnWidth);
	event->accept();
}


void EditPolarDefDlg::setupLayout()
{
	QStringList labels;
	labels << tr("Object") << tr("Field")<<tr("Value")<<tr("Unit");

	m_pStruct = new QTreeView;
#if QT_VERSION >= 0x050000
	m_pStruct->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif
//	m_pPlaneStruct->header()->setDefaultSectionSize(239);

	m_pStruct->header()->setStretchLastSection(true);
	m_pStruct->header()->setDefaultAlignment(Qt::AlignCenter);

//	m_pStruct->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
	m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
	m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
//	m_pStruct->setIndentation(31);
	m_pStruct->setWindowTitle(tr("Objects"));

	m_pModel = new QStandardItemModel(this);
	m_pModel->setColumnCount(4);
	m_pModel->clear();
	m_pModel->setHorizontalHeaderLabels(labels);

	m_pStruct->setModel(m_pModel);

/*	QItemSelectionModel *pSelectionModel = new QItemSelectionModel(m_pModel);
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

		QHBoxLayout *pCommandButtons = new QHBoxLayout;
		{
			pOKButton = new QPushButton(tr("OK"));
			pOKButton->setDefault(true);
			pCancelButton = new QPushButton(tr("Cancel"));
			pCommandButtons->addStretch(1);
			pCommandButtons->addWidget(pOKButton);
			pCommandButtons->addStretch(1);
			pCommandButtons->addWidget(pCancelButton);
			pCommandButtons->addStretch(1);
			connect(pOKButton,     SIGNAL(clicked()), this, SLOT(onOK()));
			connect(pCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		}
		pVBox->addLayout(pCommandButtons);
	}
	setLayout(pVBox);
	resize(s_Size);
}


void EditPolarDefDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!pOKButton->hasFocus() && !pCancelButton->hasFocus())
			{
				readData();
				pOKButton->setFocus();
				return;
			}
			else
			{
				onOK();
				return;
			}
			break;
		}
		case Qt::Key_Escape:
		{
			reject();
		}
		default:
			event->ignore();
	}
}



void EditPolarDefDlg::onItemChanged()
{
	readData();

	m_pWPolar->polarName()="a wing polar name";
	setAutoWPolarName(m_pWPolar, m_pPlane);

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


void EditPolarDefDlg::onOK()
{
	readData();

	if (m_pWPolar->analysisMethod()==XFLR5::VLMMETHOD)
	{
		m_pWPolar->bThinSurfaces()  = true;
		m_pWPolar->analysisMethod() = XFLR5::PANEL4METHOD;
	}
	else if (m_pWPolar->analysisMethod()==XFLR5::PANEL4METHOD && !m_pPlane->isWing())
	{
		m_pWPolar->bThinSurfaces()  = true;
	}
	else if (m_pWPolar->analysisMethod()==XFLR5::PANEL4METHOD && m_pPlane->isWing())
	{
		m_pWPolar->bThinSurfaces()  = false;
	}
	accept();
}


QList<QStandardItem *> EditPolarDefDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
	for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(XFLR5::STRING, Qt::UserRole);
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

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::BOOLVALUE, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);
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

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::INTEGER, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);
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

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::DOUBLEVALUE, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);
	return rowItems;
}


void EditPolarDefDlg::initDialog(Plane *pPlane, WPolar *pWPolar)
{
	m_pPlane  = pPlane;
	m_pWPolar = pWPolar;

	setAutoWPolarName(m_pWPolar, m_pPlane);

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
		dataItem.at(2)->setData(XFLR5::POLARTYPE, Qt::UserRole);
		polarTypeFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "Velocity", m_pWPolar->velocity(), Units::speedUnitLabel());
		polarTypeFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "Alpha", m_pWPolar->Alpha(), QString::fromUtf8("°"));
		polarTypeFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "Beta", m_pWPolar->Beta(), QString::fromUtf8("°"));
		polarTypeFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> analysisTypeFolder = prepareRow("Analysis Type");
	rootItem->appendRow(analysisTypeFolder);
	{
		if(m_pWPolar->analysisMethod()==XFLR5::LLTMETHOD) dataItem = prepareRow("", "Method", "LLTMETHOD");
		else if(m_pWPolar->bThinSurfaces())               dataItem = prepareRow("", "Method", "VLMMETHOD");
		else                                              dataItem = prepareRow("", "Method", "PANELMETHOD");
		dataItem.at(2)->setData(XFLR5::ANALYSISMETHOD, Qt::UserRole);
		analysisTypeFolder.first()->appendRow(dataItem);

		if(m_pWPolar->bDirichlet())  dataItem = prepareRow("", "Boundary condition", "DIRICHLET");
		else                         dataItem = prepareRow("", "Boundary condition", "NEUMANN");
		dataItem.at(2)->setData(XFLR5::BOUNDARYCONDITION, Qt::UserRole);
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
		dataItem.at(2)->setData(XFLR5::REFDIMENSIONS, Qt::UserRole);
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
		m_pWPolar->mass()   = m_pPlane->totalMass();
		m_pWPolar->CoG()    = m_pPlane->CoG();
		m_pWPolar->CoGIxx() = m_pPlane->m_CoGIxx;
		m_pWPolar->CoGIyy() = m_pPlane->m_CoGIyy;
		m_pWPolar->CoGIzz() = m_pPlane->m_CoGIzz;
		m_pWPolar->CoGIxz() = m_pPlane->m_CoGIxz;
	}

	dataItem = prepareBoolRow("", "Use plane inertia", m_pWPolar->bAutoInertia());
	inertiaFolder.first()->appendRow(dataItem);

	dataItem = prepareDoubleRow("", "Mass", m_pWPolar->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
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
	if (m_pWPolar->analysisMethod() == XFLR5::LLTMETHOD)
	{
		m_pWPolar->bViscous()      = true;
		m_pWPolar->bThinSurfaces() = true;
		m_pWPolar->bWakeRollUp()   = false;
		m_pWPolar->bTilted()       = false;
	}
	else if (m_pWPolar->analysisMethod() == XFLR5::VLMMETHOD)
	{
		m_pWPolar->bThinSurfaces() = true;
//		m_pWPolar->analysisMethod() = XFLR5::PANELMETHOD;
	}
	else if (m_pWPolar->analysisMethod() == XFLR5::PANEL4METHOD)
	{
		m_pWPolar->bThinSurfaces() = false;
	}

//	m_pWPolar->bThinSurfaces() = m_pWPolar->analysisMethod()==XFLR5::PANELMETHOD && m_pPlane->isWing();
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

			if     (field.compare("Name")==0)                    m_pWPolar->polarName()            = value;
			else if(field.compare("Type")==0)                    m_pWPolar->polarType()            = WPolarType(value);
			else if(field.compare("Velocity")==0)                m_pWPolar->velocity()             = dataIndex.data().toDouble()/Units::mstoUnit();
			else if(field.compare("Alpha")==0)                   m_pWPolar->Alpha()                = dataIndex.data().toDouble();
			else if(field.compare("Beta")==0)                    m_pWPolar->Beta()                 = dataIndex.data().toDouble();
			else if(field.compare("Method")==0)                  m_pWPolar->analysisMethod()       = analysisMethod(value);
			else if(field.compare("Boundary condition")==0)      m_pWPolar->boundaryCondition()    = boundaryCondition(value);
			else if(field.compare("Viscous")==0)                 m_pWPolar->bViscous()             = stringToBool(value);
			else if(field.compare("Tilted geometry")==0)         m_pWPolar->bTilted()              = stringToBool(value);
			else if(field.compare("Ignore body panels")==0)      m_pWPolar->bIgnoreBodyPanels()    = stringToBool(value);
			else if(field.compare("Use plane inertia")==0)       m_pWPolar->bAutoInertia()         = stringToBool(value);
			else if(field.compare("Mass")==0)                    m_pWPolar->mass()                 = dataIndex.data().toDouble()/Units::kgtoUnit();
			else if(field.compare("x")==0)                       m_pWPolar->CoG().x                = dataIndex.data().toDouble()/Units::mtoUnit();
			else if(field.compare("z")==0)                       m_pWPolar->CoG().z                = dataIndex.data().toDouble()/Units::mtoUnit();
			else if(field.compare("Ixx")==0)                     m_pWPolar->CoGIxx()               = dataIndex.data().toDouble()/Units::kgm2toUnit();
			else if(field.compare("Iyy")==0)                     m_pWPolar->CoGIyy()               = dataIndex.data().toDouble()/Units::kgm2toUnit();
			else if(field.compare("Izz")==0)                     m_pWPolar->CoGIzz()               = dataIndex.data().toDouble()/Units::kgm2toUnit();
			else if(field.compare("Ixz")==0)                     m_pWPolar->CoGIxz()               = dataIndex.data().toDouble()/Units::kgm2toUnit();
			else if(field.compare("Area")==0)                    m_pWPolar->referenceArea()        = referenceDimension(value);
			else if(field.compare("Reference Area")==0)          m_pWPolar->referenceArea()        = dataIndex.data().toDouble()/Units::m2toUnit();
			else if(field.compare("Reference Span Length")==0)   m_pWPolar->referenceSpanLength()  = dataIndex.data().toDouble()/Units::mtoUnit();
			else if(field.compare("Reference Chord Length")==0)  m_pWPolar->referenceChordLength() = dataIndex.data().toDouble()/Units::mtoUnit();
			else if(field.compare("Density")==0)                 m_pWPolar->density()              = dataIndex.data().toDouble();
			else if(field.compare("Viscosity")==0)               m_pWPolar->viscosity()            = dataIndex.data().toDouble();
			else if(field.compare("Ground effect")==0)           m_pWPolar->bGround()              = stringToBool(value);
			else if(field.compare("Height flight")==0)           m_pWPolar->groundHeight()         = dataIndex.data().toDouble()/Units::mtoUnit();

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
	strMass    = Units::weightUnitLabel();
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










