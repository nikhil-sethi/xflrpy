/****************************************************************************

	PlaneDlg Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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

****************************************************************************/

#include <QFileDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QMenu>
#include <math.h>
#include <misc/options/displayoptions.h>

#include <globals.h>
#include <misc/options/Units.h>
#include <miarex/Objects3D.h>
#include <miarex/mgt/ImportObjectDlg.h>
#include <miarex/mgt/XmlPlaneReader.h>
#include "PlaneDlg.h"
#include "GL3dWingDlg.h"
#include "GL3dBodyDlg.h"
#include "EditBodyDlg.h"
#include "InertiaDlg.h"

QSize PlaneDlg::s_WindowSize(1031,783);
QPoint PlaneDlg::s_WindowPosition(131, 77);
bool PlaneDlg::s_bWindowMaximized =false;


PlaneDlg::PlaneDlg(QWidget *parent) :QDialog(parent)
{
	setWindowTitle(tr("Plane Editor"));
	m_pPlane = NULL;


	m_bAcceptName         = true;
	m_bChanged            = false;
	m_bDescriptionChanged = false;

	setupLayout();

	connect(m_pctrlBiplane,   SIGNAL(clicked()), this, SLOT(onBiplane()));
	connect(m_pctrlStabCheck, SIGNAL(clicked()), this, SLOT(onStab()));
	connect(m_pctrlFinCheck,  SIGNAL(clicked()), this, SLOT(onFin()));

	connect(m_pctrlSymFin,    SIGNAL(clicked()), this, SLOT(onSymFin()));
	connect(m_pctrlDoubleFin, SIGNAL(clicked()), this, SLOT(onDoubleFin()));

	connect(m_pctrlDefineWing,  SIGNAL(clicked()), this, SLOT(onDefineWing()));
	connect(m_pctrlImportWing,  SIGNAL(clicked()), this, SLOT(onImportWing()));
	connect(m_pctrlDefineWing2, SIGNAL(clicked()), this, SLOT(onDefineWing2()));
	connect(m_pctrlImportWing2, SIGNAL(clicked()), this, SLOT(onImportWing2()));
	connect(m_pctrlDefineStab,  SIGNAL(clicked()), this, SLOT(onDefineStab()));
	connect(m_pctrlDefineFin,   SIGNAL(clicked()), this, SLOT(onDefineFin()));

	connect(m_pctrlPlaneInertia, SIGNAL(clicked()), this, SLOT(onInertia()));

	connect(OKButton,     SIGNAL(clicked()), this, SLOT(onOK()));
	connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(m_pctrlBody,       SIGNAL(clicked()), this, SLOT(onBodyCheck()));

	connect(m_pctrlPlaneDescription, SIGNAL(textChanged()), this, SLOT(onDescriptionChanged()));

	connect(m_pctrlWingTilt,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlWingTilt2, SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlStabTilt,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlFinTilt,   SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlXLEWing,   SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlZLEWing,   SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlXLEWing2,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlZLEWing2,  SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlXLEStab,   SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlZLEStab,   SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlXLEFin,    SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlYLEFin,    SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlZLEFin,    SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlXBody,     SIGNAL(editingFinished()), this, SLOT(onChanged()));
	connect(m_pctrlZBody,     SIGNAL(editingFinished()), this, SLOT(onChanged()));
}




void PlaneDlg::initDialog()
{
	QString len, surf;
	Units::getLengthUnitLabel(len);
	Units::getAreaUnitLabel(surf);

	m_pctrlLen1->setText(len);
	m_pctrlLen2->setText(len);
	m_pctrlLen3->setText(len);
	m_pctrlLen4->setText(len);
	m_pctrlLen5->setText(len);
	m_pctrlLen6->setText(len);
	m_pctrlLen7->setText(len);
	m_pctrlLen8->setText(len);
	m_pctrlLen9->setText(len);
	m_pctrlLen10->setText(len);
	m_pctrlLen11->setText(len);
	m_pctrlLen12->setText(len);
	m_pctrlLen13->setText(len);

	m_pctrlSurf1->setText(surf);
	m_pctrlSurf2->setText(surf);
	m_pctrlSurf3->setText(surf);

	m_pctrlPlaneName->setText(m_pPlane->planeName());

	if(m_pPlane->m_PlaneDescription.length()) m_pctrlPlaneDescription->setPlainText(m_pPlane->m_PlaneDescription);
	else                                      m_pctrlPlaneDescription->setPlainText("");

	setParams();
	setResults();

	if(!m_bAcceptName) m_pctrlPlaneName->setEnabled(false);
	m_bChanged = false;

	m_pPlane->m_Wing[0].createSurfaces(m_pPlane->m_WingLE[0],   0.0, m_pPlane->m_WingTiltAngle[0]);//necessary for eventual inertia calculations
	m_pPlane->m_Wing[1].createSurfaces(m_pPlane->m_WingLE[1],   0.0, m_pPlane->m_WingTiltAngle[1]);//necessary for eventual inertia calculations
	m_pPlane->m_Wing[2].createSurfaces(m_pPlane->m_WingLE[2],   0.0, m_pPlane->m_WingTiltAngle[2]);//necessary for eventual inertia calculations
	m_pPlane->m_Wing[3].createSurfaces(m_pPlane->m_WingLE[3], -90.0, m_pPlane->m_WingTiltAngle[3]);//necessary for eventual inertia calculations
}


void PlaneDlg::keyPressEvent(QKeyEvent *event)
{
	// Prevent Return Key from closing App
	switch (event->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!OKButton->hasFocus() && !CancelButton->hasFocus())
			{
				OKButton->setFocus();
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
			return;
		}
		default:
			event->ignore();
	}
}


void PlaneDlg::onBiplane()
{
	m_bChanged = true;
	m_pPlane->m_bBiplane = m_pctrlBiplane->isChecked();
	if(m_pPlane->wing2())
	{
		m_pctrlDefineWing2->setEnabled(true);
		m_pctrlImportWing2->setEnabled(true);
		m_pctrlWingTilt2->setEnabled(true);
		m_pctrlZLEWing2->setEnabled(true);
		m_pctrlXLEWing2->setEnabled(true);
	}
	else
	{
		m_pctrlDefineWing2->setEnabled(false);
		m_pctrlImportWing2->setEnabled(false);
		m_pctrlWingTilt2->setEnabled(false);
		m_pctrlZLEWing2->setEnabled(false);
		m_pctrlXLEWing2->setEnabled(false);
	}
}



void PlaneDlg::onBodyCheck()
{
	m_bChanged = true;

	if(m_pctrlBody->isChecked())
	{
		m_pPlane->m_bBody= true;
	}
	else
	{
		m_pPlane->m_bBody= false;
	}

	m_pctrlXBody->setEnabled(m_pctrlBody->isChecked());
	m_pctrlZBody->setEnabled(m_pctrlBody->isChecked());
	m_pctrlBodyActions->setEnabled(m_pctrlBody->isChecked());

	setResults();
}



void PlaneDlg::onChanged()
{
	m_bChanged = true;
	readParams();
	setResults();
}


void PlaneDlg::onDescriptionChanged()
{
	m_bDescriptionChanged = true;
}



void PlaneDlg::onDefineWing()
{
	Wing *pSaveWing = new Wing();
	pSaveWing->duplicate(m_pPlane->wing());

	GL3dWingDlg wingDlg(this);

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
}


void PlaneDlg::onDefineFin()
{
	Wing *pSaveWing = new Wing();
	pSaveWing->duplicate(m_pPlane->fin());

	GL3dWingDlg wingDlg(this);
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
}


void PlaneDlg::onDefineStab()
{
	Wing *pSaveWing = new Wing();
	pSaveWing->duplicate(m_pPlane->stab());

	GL3dWingDlg wingDlg(this);
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
}



void PlaneDlg::onDefineWing2()
{
	Wing *pSaveWing = new Wing();
	pSaveWing->duplicate(m_pPlane->wing2());

	GL3dWingDlg wingDlg(this);
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
}


void PlaneDlg::onDoubleFin()
{
	if (m_pctrlDoubleFin->isChecked())
	{
		m_pctrlYLEFin->setEnabled(true);
		m_pPlane->m_bDoubleFin = true;
		m_pPlane->m_bSymFin    = false;
		m_pctrlSymFin->setChecked(false);
	}
	else
	{
		m_pctrlYLEFin->setEnabled(false);
		m_pPlane->m_bDoubleFin = false;
	}
	m_bChanged = true;
	setResults();
}


void PlaneDlg::onDefineBody()
{
	if(!m_pPlane->body()) return;

	Body memBody;
	memBody.duplicate(m_pPlane->body());
	Vector3d v = m_pPlane->bodyPos();
    v.x = -v.x; v.z=-v.z;
 //   m_pPlane->m_pBody->Translate(v,false);
	GL3dBodyDlg glbDlg(this);
	glbDlg.m_bEnableName = false;
	glbDlg.initDialog(m_pPlane->body());
	glbDlg.setWindowState(Qt::WindowMaximized);

	if(glbDlg.exec() == QDialog::Accepted)
	{
		m_bChanged = true;
		setResults();
	}
	else m_pPlane->body()->duplicate(&memBody);

 //   m_pPlane->m_pBody->Translate(m_pPlane->BodyPos(),false);
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
	ebDlg.move(GL3dBodyDlg::s_WindowPos);
	ebDlg.resize(GL3dBodyDlg::s_WindowSize);
	if(GL3dBodyDlg::s_bWindowMaximized) ebDlg.setWindowState(Qt::WindowMaximized);


	if(ebDlg.exec() == QDialog::Accepted)
	{
		m_bChanged = true;
		setResults();
	}
	else m_pPlane->body()->duplicate(&memBody);}




void PlaneDlg::onFin()
{
	m_bChanged = true;
	if(m_pctrlFinCheck->isChecked())
	{
		m_pctrlSymFin->setEnabled(true);
		m_pctrlDoubleFin->setEnabled(true);
		m_pctrlFinTilt->setEnabled(true);
		m_pctrlXLEFin->setEnabled(true);
		if (m_pctrlDoubleFin->isChecked()) m_pctrlYLEFin->setEnabled(true);
		else                               m_pctrlYLEFin->setEnabled(false);

		m_pctrlZLEFin->setEnabled(true);
		m_pctrlDefineFin->setEnabled(true);
		m_pPlane->m_bFin = true;
	}
	else
	{
		m_pctrlSymFin->setEnabled(false);
		m_pctrlDoubleFin->setEnabled(false);
		m_pctrlFinTilt->setEnabled(false);
		m_pctrlXLEFin->setEnabled(false);
		m_pctrlYLEFin->setEnabled(false);
		m_pctrlZLEFin->setEnabled(false);
		m_pctrlDefineFin->setEnabled(false);
		m_pPlane->m_bFin = false;
	}
	setResults();
}


void PlaneDlg::onImportPlaneBody()
{
	ImportObjectDlg dlg(this);
	if(m_pPlane->body()) dlg.m_ObjectName = m_pPlane->body()->m_BodyName;
	else                 dlg.m_ObjectName.clear();
	dlg.initDialog(false);

	if(dlg.exec() == QDialog::Accepted)
	{
		Body *pOldBody = Objects3D::getBody(dlg.m_ObjectName);
		if(pOldBody)
		{
			m_pPlane->setBody(pOldBody);
		}
	}
}


void PlaneDlg::onDefaultBody()
{
	QString strong = tr("Revert to the default body definition ?");
	if (QMessageBox::Yes != QMessageBox::question(this, tr("Question"), strong, QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)) return;

	Body aNewBody;
	m_pPlane->m_Body.duplicate(&aNewBody);
	m_bChanged = true;
}




void PlaneDlg::onImportXMLBody()
{
	QString PathName;
	PathName = QFileDialog::getOpenFileName(this, tr("Open XML File"),
											Settings::s_LastDirName,
											tr("Plane XML file")+"(*.xml)");
	if(!PathName.length())		return ;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

	QFile XFile(PathName);
	if (!XFile.open(QIODevice::ReadOnly))
	{
		QString strange = tr("Could not read the file\n")+PathName;
		QMessageBox::warning(this, tr("Warning"), strange);
		return;
	}

	Plane a_plane;
	a_plane.hasBody() = true;
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
//	if(m_pPlane->body()) delete m_pPlane->body();

//	Body *pXMLBody = new Body;
//	pXMLBody->duplicate(a_plane.body());
	m_pPlane->body()->duplicate(a_plane.body());
	m_pctrlBody->setChecked(true);
}




void PlaneDlg::onImportWing()
{
	ImportObjectDlg dlg(this);
	dlg.m_ObjectName = m_pPlane->planeName();
	dlg.initDialog(true);

	if(dlg.exec() == QDialog::Accepted)
	{
		m_bChanged = true;
		dlg.m_ObjectName.replace("/Main wing","");
		Wing *pWing = Objects3D::getWing(dlg.m_ObjectName);
		if(pWing)
		{
			m_pPlane->wing()->duplicate(pWing);
			m_pPlane->wing()->setWingColor(pWing->wingColor());
		}
	}
}


void PlaneDlg::onImportWing2()
{
	ImportObjectDlg dlg(this);
	dlg.m_ObjectName = m_pPlane->wing()->m_WingName;
	dlg.initDialog(true);

	if(dlg.exec() == QDialog::Accepted)
	{
		m_bChanged = true;
		Wing *pWing = Objects3D::getWing(dlg.m_ObjectName);
		if(pWing)
		{
			m_pPlane->wing2()->duplicate(pWing);
			m_pPlane->wing2()->setWingColor(pWing->wingColor());
		}
	}
}



void PlaneDlg::onInertia()
{
	if(!m_pPlane) return;
	readParams();

	m_pPlane->createSurfaces();//necessary for inertia calculations

    InertiaDlg dlg(this);
	dlg.m_pBody = NULL;
	dlg.m_pWing = NULL;
	dlg.m_pPlane = m_pPlane;

	//save inertia properties
	QList<PointMass*> memPtMass;
	memPtMass.clear();
	for(int im=0; im<m_pPlane->m_PointMass.size(); im++)
	{
		memPtMass.append(new PointMass(m_pPlane->m_PointMass[im]));
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
			m_pPlane->m_PointMass.append(new PointMass(memPtMass[im]));
		}
	}
}


void PlaneDlg::onPlaneName()
{
	m_pPlane->setPlaneName(m_pctrlPlaneName->text());
}


void PlaneDlg::onOK()
{
	int j;

	readParams();

	m_pPlane->m_PlaneDescription = m_pctrlPlaneDescription->toPlainText();

	m_pPlane->computePlane();


	//check the number of surfaces
	int nSurfaces = 0;
	for (j=0; j<m_pPlane->wing()->NWingSection()-1; j++)
	{
		if(qAbs(m_pPlane->wing()->YPosition(j)-m_pPlane->wing()->YPosition(j+1)) > Wing::s_MinPanelSize) nSurfaces+=2;
	}
	if(m_pPlane->stab())
	{
		for (j=0; j<m_pPlane->stab()->NWingSection()-1; j++)
		{
			if(qAbs(m_pPlane->stab()->YPosition(j)-m_pPlane->stab()->YPosition(j+1)) > Wing::s_MinPanelSize) nSurfaces+=2;
		}
	}

	if(m_pPlane->fin())
	{
		for (j=0; j<m_pPlane->fin()->NWingSection()-1; j++)
		{
			if(qAbs(m_pPlane->fin()->YPosition(j)-m_pPlane->fin()->YPosition(j+1)) > Wing::s_MinPanelSize)
			{
				if((m_pPlane->m_bSymFin) || m_pPlane->m_bDoubleFin)
					nSurfaces += 2;
				else
					nSurfaces += 1;
			}
		}
	}

	m_pPlane->computeBodyAxisInertia();

	m_pPlane->m_bBody = m_pctrlBody->isChecked();

	accept();
}



void PlaneDlg::onStab()
{
	m_bChanged = true;
	if(m_pctrlStabCheck->isChecked())
	{
		m_pctrlDefineStab->setEnabled(true);
		m_pctrlXLEStab->setEnabled(true);
		m_pctrlZLEStab->setEnabled(true);
		m_pctrlStabTilt->setEnabled(true);
		m_pPlane->m_bStab = true;
	}
	else
	{
		m_pctrlDefineStab->setEnabled(false);
		m_pctrlXLEStab->setEnabled(false);
		m_pctrlZLEStab->setEnabled(false);
		m_pctrlStabTilt->setEnabled(false);
		m_pPlane->m_bStab = false;
	}
	setResults();
}


void PlaneDlg::onSymFin()
{	
	if (m_pctrlSymFin->isChecked()) 
	{
		m_pctrlYLEFin->setEnabled(false);
		m_pPlane->m_bSymFin    = true;
		m_pPlane->m_bDoubleFin = false;
		m_pctrlDoubleFin->setChecked(false);
		m_pctrlYLEFin->setEnabled(false);
	}
	else
	{
//		m_pctrlDoubleFin->setEnabled(true);
		m_pctrlYLEFin->setEnabled(true);
		m_pPlane->m_bSymFin = false;
	}
	m_bChanged = true;
	setResults();
}




void PlaneDlg::readParams()
{
	onPlaneName();
	m_pPlane->m_WingTiltAngle[0] = m_pctrlWingTilt->value();
	m_pPlane->m_WingTiltAngle[1] = m_pctrlWingTilt2->value();
	m_pPlane->m_WingTiltAngle[2] = m_pctrlStabTilt->value();
	m_pPlane->m_WingTiltAngle[3] = m_pctrlFinTilt->value();

	m_pPlane->m_WingLE[0].x = m_pctrlXLEWing->value() / Units::mtoUnit();
	m_pPlane->m_WingLE[0].z = m_pctrlZLEWing->value() / Units::mtoUnit();

	m_pPlane->m_WingLE[1].x = m_pctrlXLEWing2->value() / Units::mtoUnit();
	m_pPlane->m_WingLE[1].z = m_pctrlZLEWing2->value() / Units::mtoUnit();

	m_pPlane->m_WingLE[2].x = m_pctrlXLEStab->value() / Units::mtoUnit();
	m_pPlane->m_WingLE[2].z = m_pctrlZLEStab->value() / Units::mtoUnit();

	m_pPlane->m_WingLE[3].x = m_pctrlXLEFin->value() / Units::mtoUnit();
	m_pPlane->m_WingLE[3].y = m_pctrlYLEFin->value() / Units::mtoUnit();
	m_pPlane->m_WingLE[3].z = m_pctrlZLEFin->value() / Units::mtoUnit();

	m_pPlane->m_BodyPos.x = m_pctrlXBody->value() / Units::mtoUnit();
	m_pPlane->m_BodyPos.z = m_pctrlZBody->value() / Units::mtoUnit();

	if(m_pctrlBiplane->isChecked())   m_pPlane->m_bBiplane = true;
	else                              m_pPlane->m_bBiplane = false;
	if(m_pctrlStabCheck->isChecked()) m_pPlane->m_bStab = true;
	else                              m_pPlane->m_bStab = false;
	if(m_pctrlFinCheck->isChecked())  m_pPlane->m_bFin  = true;
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
//	reject();
	done(QDialog::Rejected);
}


void PlaneDlg::setParams()
{
	if(m_pPlane->body()) m_pctrlBody->setChecked(true);
	else                 m_pctrlBody->setChecked(false);

	m_pctrlBody->setEnabled(true);
	m_pctrlXBody->setEnabled(m_pPlane->m_bBody);
	m_pctrlZBody->setEnabled(m_pPlane->m_bBody);
	m_pctrlBodyActions->setEnabled(m_pPlane->m_bBody);

	m_pctrlPlaneName->setText(m_pPlane->planeName());
	m_pctrlWingTilt->setValue(m_pPlane->m_WingTiltAngle[0]);
	m_pctrlWingTilt2->setValue(m_pPlane->m_WingTiltAngle[1]);
	m_pctrlStabTilt->setValue(m_pPlane->m_WingTiltAngle[2]);
	m_pctrlFinTilt->setValue(m_pPlane->m_WingTiltAngle[3]);

	m_pctrlXLEWing->setValue(m_pPlane->m_WingLE[0].x * Units::mtoUnit());
	m_pctrlZLEWing->setValue(m_pPlane->m_WingLE[0].z * Units::mtoUnit());

	m_pctrlXLEWing2->setValue(m_pPlane->m_WingLE[1].x * Units::mtoUnit());
	m_pctrlZLEWing2->setValue(m_pPlane->m_WingLE[1].z * Units::mtoUnit());

	m_pctrlXLEStab->setValue(m_pPlane->m_WingLE[2].x * Units::mtoUnit());
	m_pctrlZLEStab->setValue(m_pPlane->m_WingLE[2].z * Units::mtoUnit());

	m_pctrlXBody->setValue(m_pPlane->m_BodyPos.x * Units::mtoUnit());
	m_pctrlZBody->setValue(m_pPlane->m_BodyPos.z * Units::mtoUnit());

	m_pctrlBiplane->setChecked(m_pPlane->wing2());
	onBiplane();

	m_pctrlXLEFin->setValue(m_pPlane->m_WingLE[3].x* Units::mtoUnit());
	m_pctrlYLEFin->setValue(m_pPlane->m_WingLE[3].y* Units::mtoUnit());
	m_pctrlZLEFin->setValue(m_pPlane->m_WingLE[3].z* Units::mtoUnit());
	m_pctrlFinCheck->setChecked(m_pPlane->m_bFin);
	m_pctrlDoubleFin->setChecked(m_pPlane->m_bDoubleFin);
	m_pctrlSymFin->setChecked(m_pPlane->m_bSymFin);
	onFin();
	m_pctrlStabCheck->setChecked(m_pPlane->stab());
	onStab();
}


void PlaneDlg::setResults()
{
	QString str;

//	double area = m_pPlane->Wing()->s_Area;
//	if(m_pPlane->m_bBiplane) area += m_pPlane->Wing2()->m_Area;

	str = QString("%1").arg(m_pPlane->wing()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
	m_pctrlWingSurface->setText(str);

	if(m_pPlane->stab())   str = QString("%1").arg(m_pPlane->stab()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
	else str = " ";
	m_pctrlStabSurface->setText(str);

	if(m_pPlane->fin())    str = QString("%1").arg(m_pPlane->fin()->m_PlanformArea*Units::m2toUnit(),7,'f',2);
	else str=" ";
	m_pctrlFinSurface->setText(str);

	double span = m_pPlane->wing()->m_PlanformSpan;
	if(m_pPlane->wing2()) span = qMax(m_pPlane->wing()->m_PlanformSpan, m_pPlane->wing2()->m_PlanformSpan);
	str = QString("%1").arg(span*Units::mtoUnit(),5,'f',2);
	m_pctrlWingSpan->setText(str);

	m_pPlane->computePlane();
	if(m_pPlane->stab())
	{
		double SLA = m_pPlane->m_WingLE[2].x + m_pPlane->stab()->Chord(0)/4.0 - m_pPlane->wing()->Chord(0)/4.0;
		str = QString("%1").arg(SLA*Units::mtoUnit(),5,'f',2);
	}
	else  str=" ";
	m_pctrlStabLeverArm->setText(str);

	if(m_pPlane->stab())
	{
		str = QString("%1").arg(m_pPlane->tailVolume(),5,'f',2);
	}
	else str =" ";
	m_pctrlStabVolume->setText(str);


	str = QString("%1").arg(m_pPlane->VLMPanelTotal());
	m_pctrlVLMTotalPanels->setText(str);
}



void PlaneDlg::setupLayout()
{
	QGroupBox *pNameBox = new QGroupBox(tr("Plane Description"));
	{
		QVBoxLayout *pNameLayout = new QVBoxLayout;
		{
			m_pctrlPlaneName = new QLineEdit(tr("Plane Name"));
			m_pctrlPlaneDescription = new QTextEdit();
			m_pctrlPlaneDescription->setToolTip(tr("Enter here a short description for the plane"));
			QLabel *PlaneDescription = new QLabel(tr("Description:"));
			m_pctrlPlaneInertia = new QPushButton(tr("Plane Inertia"));
			pNameLayout->addWidget(m_pctrlPlaneName);
			pNameLayout->addWidget(PlaneDescription);
			pNameLayout->addWidget(m_pctrlPlaneDescription);
			pNameLayout->addWidget(m_pctrlPlaneInertia);
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
			m_pctrlDefineWing = new QPushButton(tr("Define"));
			m_pctrlImportWing = new QPushButton(tr("Import"));
			QLabel *lab1 = new QLabel(tr("x="));
			QLabel *lab2 = new QLabel(tr("z="));
			QLabel *lab3 = new QLabel(tr("Tilt Angle="));
			lab1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_pctrlXLEWing = new DoubleEdit(0.0, 3);
			m_pctrlZLEWing = new DoubleEdit(0.0, 3);
			m_pctrlWingTilt = new DoubleEdit(0.0, 3);
			m_pctrlLen1 = new QLabel("mm");
			m_pctrlLen2 = new QLabel("mm");
			QLabel *lab4 = new QLabel(QString::fromUtf8("°"));
			pMainWingLayout->addWidget(m_pctrlDefineWing, 2,1);
			pMainWingLayout->addWidget(m_pctrlImportWing, 3,1);
			pMainWingLayout->addWidget(lab1,2,2);
			pMainWingLayout->addWidget(lab2,3,2);
			pMainWingLayout->addWidget(lab3,4,2);
			pMainWingLayout->addWidget(m_pctrlXLEWing,2,3);
			pMainWingLayout->addWidget(m_pctrlZLEWing,3,3);
			pMainWingLayout->addWidget(m_pctrlWingTilt,4,3);
			pMainWingLayout->addWidget(m_pctrlLen1,2,4);
			pMainWingLayout->addWidget(m_pctrlLen2,3,4);
			pMainWingLayout->addWidget(lab4,4,4);
			pMainWingLayout->setRowStretch(5,1);
		}
		pMainWingBox->setLayout(pMainWingLayout);
	}

	QGroupBox *pMainWing2Box = new QGroupBox(tr("Wing 2"));
	{
		QGridLayout *pMainWing2Layout = new QGridLayout;
		{
			m_pctrlBiplane = new QCheckBox(tr("Biplane"));
			pMainWing2Layout->addWidget(m_pctrlBiplane,1,1);
			m_pctrlDefineWing2 = new QPushButton(tr("Define"));
			m_pctrlImportWing2 = new QPushButton(tr("Import"));
			QLabel *lab11 = new QLabel(tr("x="));
			QLabel *lab12 = new QLabel(tr("z="));
			QLabel *lab13 = new QLabel(tr("Tilt Angle="));
			lab11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab12->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab13->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_pctrlXLEWing2 = new DoubleEdit(0.0, 3);
			m_pctrlZLEWing2 = new DoubleEdit(0.0, 3);
			m_pctrlWingTilt2 = new DoubleEdit(0.0, 3);
			m_pctrlLen3 = new QLabel("mm");
			m_pctrlLen4 = new QLabel("mm");
			QLabel *lab14 = new QLabel(QString::fromUtf8("°"));
			pMainWing2Layout->addWidget(m_pctrlDefineWing2, 2,1);
			pMainWing2Layout->addWidget(m_pctrlImportWing2, 3,1);
			pMainWing2Layout->addWidget(lab11,2,2);
			pMainWing2Layout->addWidget(lab12,3,2);
			pMainWing2Layout->addWidget(lab13,4,2);
			pMainWing2Layout->addWidget(m_pctrlXLEWing2,2,3);
			pMainWing2Layout->addWidget(m_pctrlZLEWing2,3,3);
			pMainWing2Layout->addWidget(m_pctrlWingTilt2,4,3);
			pMainWing2Layout->addWidget(m_pctrlLen3,2,4);
			pMainWing2Layout->addWidget(m_pctrlLen4,3,4);
			pMainWing2Layout->addWidget(lab14,4,4);
		}
		pMainWing2Box->setLayout(pMainWing2Layout);
	}

	QGroupBox *pStabBox = new QGroupBox(tr("Elevator"));
	{
		QGridLayout *pStabLayout = new QGridLayout;
		{
			m_pctrlStabCheck = new QCheckBox(tr("Elevator"));
			m_pctrlDefineStab = new QPushButton(tr("Define"));
			QLabel *lab21 = new QLabel(tr("x="));
			QLabel *lab22 = new QLabel(tr("z="));
			QLabel *lab23 = new QLabel(tr("Tilt Angle="));
			lab21->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab22->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab23->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_pctrlXLEStab = new DoubleEdit(550.0, 3);
			m_pctrlZLEStab = new DoubleEdit(550.0, 3);
			m_pctrlStabTilt = new DoubleEdit(0.0, 3);
			m_pctrlLen5 = new QLabel("mm");
			m_pctrlLen6 = new QLabel("mm");
			QLabel *lab24 = new QLabel(QString::fromUtf8("°"));
			pStabLayout->addWidget(m_pctrlStabCheck,1,1);
			pStabLayout->addWidget(m_pctrlDefineStab, 2,1);
			pStabLayout->addWidget(lab21,2,2);
			pStabLayout->addWidget(lab22,4,2);
			pStabLayout->addWidget(lab23,5,2);
			pStabLayout->addWidget(m_pctrlXLEStab,2,3);
			pStabLayout->addWidget(m_pctrlZLEStab,4,3);
			pStabLayout->addWidget(m_pctrlStabTilt,5,3);
			pStabLayout->addWidget(m_pctrlLen5,2,4);
			pStabLayout->addWidget(m_pctrlLen6,4,4);
			pStabLayout->addWidget(lab24,5,4);
			pStabLayout->setRowStretch(6,1);
		}
		pStabBox->setLayout(pStabLayout);
	}

	QGroupBox *pFinBox = new QGroupBox(tr("Fin"));
	{
		QGridLayout *pFinLayout = new QGridLayout;
		{
			m_pctrlFinCheck = new QCheckBox(tr("Fin"));
			m_pctrlDefineFin = new QPushButton(tr("Define"));
			m_pctrlDoubleFin = new QCheckBox(tr("Double Fin"));
			m_pctrlSymFin = new QCheckBox(tr("Two-sided Fin"));
			QLabel *lab31 = new QLabel(tr("x="));
			QLabel *lab32 = new QLabel(tr("y="));
			QLabel *lab33 = new QLabel(tr("z="));
			QLabel *lab34 = new QLabel(tr("Tilt Angle="));
			lab31->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab32->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab33->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			lab34->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			m_pctrlXLEFin = new DoubleEdit(600.0, 3);
			m_pctrlYLEFin = new DoubleEdit(0.0, 3);
			m_pctrlZLEFin = new DoubleEdit(50.0, 3);
			m_pctrlFinTilt = new DoubleEdit(0.0, 3);
			m_pctrlLen7= new QLabel("mm");
			m_pctrlLen8 = new QLabel("mm");
			m_pctrlLen9 = new QLabel("mm");
			QLabel *lab35 = new QLabel(QString::fromUtf8("°"));
			pFinLayout->addWidget(m_pctrlFinCheck,1,1);
			pFinLayout->addWidget(m_pctrlDefineFin, 2,1);
			pFinLayout->addWidget(m_pctrlSymFin, 3,1);
			pFinLayout->addWidget(m_pctrlDoubleFin, 4,1);
			pFinLayout->addWidget(lab31,2,2);
			pFinLayout->addWidget(lab32,3,2);
			pFinLayout->addWidget(lab33,4,2);
			pFinLayout->addWidget(lab34,5,2);
			pFinLayout->addWidget(m_pctrlXLEFin,2,3);
			pFinLayout->addWidget(m_pctrlYLEFin,3,3);
			pFinLayout->addWidget(m_pctrlZLEFin,4,3);
			pFinLayout->addWidget(m_pctrlFinTilt,5,3);
			pFinLayout->addWidget(m_pctrlLen7,2,4);
			pFinLayout->addWidget(m_pctrlLen8,3,4);
			pFinLayout->addWidget(m_pctrlLen9,4,4);
			pFinLayout->addWidget(lab35,5,4);
		}
		pFinBox->setLayout(pFinLayout);
	}

	QGroupBox *pBodyBox = new QGroupBox(tr("Body"));
	{
		QHBoxLayout *pBodyNameLayout = new QHBoxLayout;
		{
			m_pctrlBody = new QCheckBox(tr("Body"));

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


			m_pctrlBodyActions = new QPushButton(tr("Actions..."));

			QMenu *pBodyMenu = new QMenu(tr("Actions..."),this);
			pBodyMenu->addAction(pDefineBody);
			pBodyMenu->addAction(pDefineBodyObject);
			pBodyMenu->addAction(pDefaultBody);
			pBodyMenu->addAction(pImportXMLBody);
			pBodyMenu->addAction(pImportPlaneBody);
			m_pctrlBodyActions->setMenu(pBodyMenu);

			pBodyNameLayout->addWidget(m_pctrlBody);
			pBodyNameLayout->addWidget(m_pctrlBodyActions);
			pBodyNameLayout->addStretch(1);
		}
		QGridLayout *pBodyPos = new QGridLayout;
		{
			pBodyPos->setColumnStretch(0,3);
			pBodyPos->setColumnStretch(1,0);
			pBodyPos->setColumnStretch(2,0);
			m_pctrlXBody = new DoubleEdit(0.00);
			m_pctrlZBody = new DoubleEdit(0.00);
			QLabel *lab41 = new QLabel(tr("x="));
			QLabel *lab42 = new QLabel(tr("z="));
			m_pctrlLen10 = new QLabel("mm");
			m_pctrlLen11 = new QLabel("mm");
			pBodyPos->addWidget(lab41,1,1);
			pBodyPos->addWidget(m_pctrlXBody,1,2);
			pBodyPos->addWidget(m_pctrlLen10,1,3);
			pBodyPos->addWidget(lab42,2,1);
			pBodyPos->addWidget(m_pctrlZBody,2,2);
			pBodyPos->addWidget(m_pctrlLen11,2,3);
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
		QLabel *lab101 = new QLabel(tr("Wing Area = "));
		QLabel *lab102 = new QLabel(tr("Wing Span = "));
		QLabel *lab103 = new QLabel(tr("Elev. Area = "));
		QLabel *lab104 = new QLabel(tr("Elev. Lever Arm = "));
		lab101->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab102->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab103->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab104->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlWingSurface    = new QLabel("1.00");
		m_pctrlStabSurface    = new QLabel("2.00");
		m_pctrlWingSpan       = new QLabel("3.00");
		m_pctrlStabLeverArm   = new QLabel("4.00");
		m_pctrlFinSurface     = new QLabel("5.00");
		m_pctrlStabVolume     = new QLabel("6.00");
		m_pctrlVLMTotalPanels = new QLabel("1234");
		m_pctrlWingSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlStabSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlWingSpan->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlStabLeverArm->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		m_pctrlSurf1 = new QLabel("dm2", this);
		m_pctrlSurf2 = new QLabel("dm2", this);
		m_pctrlSurf3 = new QLabel("dm2", this);
		m_pctrlLen12 = new QLabel("mm", this);
		m_pctrlLen13 = new QLabel("mm", this);
//		m_pctrlVolume = new QLabel("mm3", this);
		pData1Layout->addWidget(lab101, 1, 1);
		pData1Layout->addWidget(m_pctrlWingSurface,1,2);
		pData1Layout->addWidget(m_pctrlSurf1, 1, 3);
		pData1Layout->addWidget(lab102, 2, 1);
		pData1Layout->addWidget(m_pctrlWingSpan, 2, 2);
		pData1Layout->addWidget(m_pctrlLen12, 2, 3);
		pData1Layout->addWidget(lab103, 3, 1);
		pData1Layout->addWidget(m_pctrlStabSurface, 3, 2);
		pData1Layout->addWidget(m_pctrlSurf2, 3, 3);
		pData1Layout->addWidget(lab104, 4, 1);
		pData1Layout->addWidget(m_pctrlStabLeverArm, 4, 2);
		pData1Layout->addWidget(m_pctrlLen13, 4, 3);
	}

	QGridLayout *pData2Layout = new QGridLayout;
	{
		QLabel *lab105 = new QLabel(tr("Fin Area = "));
		QLabel *lab106 = new QLabel(tr("TailVolume = "));
		QLabel *lab108 = new QLabel(tr("Total Panels = "));
		lab105->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab106->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		lab108->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlFinSurface->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlStabVolume->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		m_pctrlVLMTotalPanels->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		pData2Layout->addWidget(lab105, 1, 1);
		pData2Layout->addWidget(m_pctrlFinSurface, 1, 2);
		pData2Layout->addWidget(m_pctrlSurf3, 1, 3);
		pData2Layout->addWidget(lab106, 2, 1);
		pData2Layout->addWidget(m_pctrlStabVolume, 2, 2);
		pData2Layout->addWidget(lab108, 3, 1);
		pData2Layout->addWidget(m_pctrlVLMTotalPanels, 3, 2);
	}

	QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
	{
		OKButton = new QPushButton(tr("OK"));
		CancelButton = new QPushButton(tr("Cancel"));
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(OKButton);
		pCommandButtonsLayout->addStretch(1);
		pCommandButtonsLayout->addWidget(CancelButton);
		pCommandButtonsLayout->addStretch(1);
	}


	QGridLayout *pGeomLayout = new QGridLayout;
	{
		pGeomLayout->addWidget(pNameBox,1,1);
		pGeomLayout->addWidget(pBodyBox,1,2);
		pGeomLayout->addWidget(pMainWingBox,2,1);
		pGeomLayout->addWidget(pMainWing2Box,2,2);
		pGeomLayout->addWidget(pStabBox,3,1);
		pGeomLayout->addWidget(pFinBox,3,2);
		pGeomLayout->addLayout(pData1Layout,4,1);
		pGeomLayout->addLayout(pData2Layout,4,2);
	}


	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pGeomLayout);
		pMainLayout->addStretch(1);
		pMainLayout->addLayout(pCommandButtonsLayout);
		pMainLayout->addStretch(1);
	}
	setLayout(pMainLayout);
}
















