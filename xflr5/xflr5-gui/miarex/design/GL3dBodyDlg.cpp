/****************************************************************************

	BodyDlg Class
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

*****************************************************************************/


#include <globals.h>
#include <misc/LinePickerDlg.h>
#include <misc/Settings.h>
#include <misc/LengthUnitDlg.h>
#include <misc/Units.h>
#include "./BodyTransDlg.h"
#include "./InertiaDlg.h"
#include "./BodyScaleDlg.h"
#include "./GL3dBodyDlg.h"
#include <miarex/mgt/XmlPlaneReader.h>
#include <miarex/mgt/XmlPlaneWriter.h>
#include <miarex/view/W3dPrefsDlg.h>
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
#include <math.h>


QByteArray GL3dBodyDlg::m_VerticalSplitterSizes;
QByteArray GL3dBodyDlg::m_HorizontalSplitterSizes;
QByteArray GL3dBodyDlg::m_LeftSplitterSizes;



QPoint GL3dBodyDlg::s_WindowPos=QPoint(20,20);
QSize  GL3dBodyDlg::s_WindowSize=QSize(900, 700);
bool GL3dBodyDlg::s_bWindowMaximized=true;


bool GL3dBodyDlg::s_bOutline    = true;
bool GL3dBodyDlg::s_bSurfaces   = true;
bool GL3dBodyDlg::s_bVLMPanels  = false;
bool GL3dBodyDlg::s_bAxes       = true;
bool GL3dBodyDlg::s_bShowMasses = false;
bool GL3dBodyDlg::s_bFoilNames  = false;

GL3dBodyDlg::GL3dBodyDlg(QWidget *pParent): QDialog(pParent)
{
	setWindowTitle(tr("Body Edition"));
	setWindowFlags(Qt::Window);
	setMouseTracking(true);

	m_pBodyGridDlg = new BodyGridDlg(this);
	m_pBody = NULL;
	m_pPointPrecision = NULL;
	m_pFramePrecision = NULL;

	m_pPointDelegate = NULL;
	m_pFrameDelegate = NULL;

	m_pPointModel = NULL;
	m_pFrameModel = NULL;

	//create a default pix from a random image - couldn't find a better way to do this
	m_pixTextLegend = QPixmap(":/images/xflr5_64.png");
	m_pixTextLegend.fill(Qt::transparent);

	QFontMetrics fm(Settings::s_TextFont);
	int w = fm.averageCharWidth()*19;
	int h = fm.height()*5;
	QRect rect(0,0,w,h);
	m_pixTextLegend = m_pixTextLegend.scaled(rect.size());

	m_StackPos  = 0; //the current position on the stack
	m_bResetFrame = true;

	m_bChanged    = false;
	m_bEnableName = true;

	m_pScaleBody        = new QAction(tr("Scale"), this);
	m_pGrid             = new QAction(tr("Grid Setup"), this);
	m_pResetScales      = new QAction(tr("Reset Scales")+("\t(R)"), this);

	m_pUndo= new QAction(QIcon(":/images/OnUndo.png"), tr("Undo"), this);
	m_pUndo->setStatusTip(tr("Cancels the last modification"));
	m_pUndo->setShortcut(Qt::CTRL + Qt::Key_Z);
	connect(m_pUndo, SIGNAL(triggered()), this, SLOT(onUndo()));

	m_pRedo = new QAction(QIcon(":/images/OnRedo.png"), tr("Redo"), this);
	m_pRedo->setStatusTip(tr("Restores the last cancelled modification"));
	m_pRedo->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Z);
	connect(m_pRedo, SIGNAL(triggered()), this, SLOT(onRedo()));

	m_pExportBodyGeom = new QAction(tr("Export Body Geometry to text File"), this);
	connect(m_pExportBodyGeom, SIGNAL(triggered()), this, SLOT(onExportBodyGeom()));

	m_pExportBodyDef = new QAction(tr("Export Body Definition to txt File"), this);
	connect(m_pExportBodyDef, SIGNAL(triggered()), this, SLOT(onExportBodyDef()));

	m_pExportBodyXML= new QAction(tr("Export body definition to an XML file"), this);
	connect(m_pExportBodyXML, SIGNAL(triggered()), this, SLOT(onExportBodyXML()));

	m_pImportBodyDef = new QAction(tr("Import Body Definition from a text file"), this);
	connect(m_pImportBodyDef, SIGNAL(triggered()), this, SLOT(onImportBodyDef()));

	m_pImportBodyXML= new QAction(tr("Import body definition from an XML file"), this);
	connect(m_pImportBodyXML, SIGNAL(triggered()), this, SLOT(onImportBodyXML()));

	m_pBodyInertia = new QAction(tr("Define Inertia")+"\tF12", this);
	connect(m_pBodyInertia, SIGNAL(triggered()), this, SLOT(onBodyInertia()));

	m_pTranslateBody = new QAction(tr("Translate"), this);
	connect(m_pTranslateBody, SIGNAL(triggered()), this, SLOT(onTranslateBody()));
	setupLayout();
	setTableUnits();

	connect(m_pScaleBody,        SIGNAL(triggered()), this, SLOT(onScaleBody()));
	connect(m_pResetScales,      SIGNAL(triggered()), this, SLOT(onResetScales()));
	connect(m_pGrid,             SIGNAL(triggered()), this, SLOT(onGrid()));

	connect(m_pctrlReset,      SIGNAL(clicked()), this, SLOT(onResetScales()));

	connect(m_pctrlAxes,       SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onAxes(bool)));
	connect(m_pctrlPanels,     SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onPanels(bool)));
	connect(m_pctrlSurfaces,   SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onSurfaces(bool)));
	connect(m_pctrlOutline,    SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onOutline(bool)));
	connect(m_pctrlShowMasses, SIGNAL(clicked(bool)), &m_gl3dBodyview, SLOT(onShowMasses(bool)));

	connect(m_pctrlIso,        SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3DIso()));
	connect(m_pctrlX,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3DFront()));
	connect(m_pctrlY,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3DLeft()));
	connect(m_pctrlZ,          SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3DTop()));
	connect(m_pctrlFlip,       SIGNAL(clicked()), &m_gl3dBodyview, SLOT(on3DFlip()));

	connect(&m_gl3dBodyview, SIGNAL(viewModified()), this, SLOT(onCheckViewIcons()));

//	connect(m_pctrlEdgeWeight, SIGNAL(sliderReleased()), this, SLOT(onEdgeWeight()));
	connect(m_pctrlPanelBunch, SIGNAL(sliderMoved(int)), this, SLOT(onNURBSPanels()));

	connect(m_pctrlFlatPanels, SIGNAL(clicked()), this, SLOT(onLineType()));
	connect(m_pctrlBSplines,   SIGNAL(clicked()), this, SLOT(onLineType()));
	connect(m_pctrlBodyColor,  SIGNAL(clicked()), this, SLOT(onBodyColor()));

	connect(m_pctrlBodyName,   SIGNAL(editingFinished()), this, SLOT(onBodyName()));
	connect(m_pctrlTextures,   SIGNAL(clicked()), this, SLOT(onTextures()));
	connect(m_pctrlColor,      SIGNAL(clicked()), this, SLOT(onTextures()));

	connect(m_pctrlNHoopPanels,SIGNAL(editingFinished()), this, SLOT(onNURBSPanels()));
	connect(m_pctrlNXPanels,   SIGNAL(editingFinished()), this, SLOT(onNURBSPanels()));
	connect(m_pctrlXDegree,    SIGNAL(activated(int)), this, SLOT(onSelChangeXDegree(int)));
	connect(m_pctrlHoopDegree, SIGNAL(activated(int)), this, SLOT(onSelChangeHoopDegree(int)));

	connect(m_pctrlUndo, SIGNAL(clicked()),this, SLOT(onUndo()));
	connect(m_pctrlRedo, SIGNAL(clicked()),this, SLOT(onRedo()));

	connect(m_pSelectionModelFrame, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onFrameItemClicked(QModelIndex)));
	connect(m_pFrameDelegate,       SIGNAL(closeEditor(QWidget *)), this, SLOT(onFrameCellChanged(QWidget *)));
	connect(m_pSelectionModelPoint, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onPointItemClicked(QModelIndex)));
	connect(m_pPointDelegate,       SIGNAL(closeEditor(QWidget *)), this, SLOT(onPointCellChanged(QWidget *)));

	connect(m_pctrlOK,     SIGNAL(clicked()),this, SLOT(accept()));
	connect(m_pctrlCancel, SIGNAL(clicked()),this, SLOT(reject()));

	connect(m_pBodyLineWidget, SIGNAL(frameSelChanged()), this, SLOT(onFrameClicked()));
	connect(m_pFrameWidget, SIGNAL(pointSelChanged()), this, SLOT(onPointClicked()));

	connect(m_pBodyLineWidget, SIGNAL(objectModified()), this, SLOT(onUpdateBody()));
	connect(m_pFrameWidget,    SIGNAL(objectModified()), this, SLOT(onUpdateBody()));
}


void GL3dBodyDlg::setTableUnits()
{
	QString length;
	Units::getLengthUnitLabel(length);

	m_pFrameModel->setHeaderData(0, Qt::Horizontal, "x ("+length+")");
	m_pFrameModel->setHeaderData(1, Qt::Horizontal, tr("NPanels"));
	m_pPointModel->setHeaderData(0, Qt::Horizontal, "y ("+length+")");
	m_pPointModel->setHeaderData(1, Qt::Horizontal, "z ("+length+")");
	m_pPointModel->setHeaderData(2, Qt::Horizontal, tr("NPanels"));
}


GL3dBodyDlg::~GL3dBodyDlg()
{
	clearStack(-1);
	delete m_pBodyGridDlg;
	if(m_pFramePrecision) delete [] m_pFramePrecision;
	if(m_pFrameDelegate)  delete m_pFrameDelegate;

	if(m_pPointPrecision) delete [] m_pPointPrecision;
	if(m_pPointDelegate)  delete m_pPointDelegate;
	delete m_pFrameModel;
	delete m_pPointModel;
}


void GL3dBodyDlg::fillFrameCell(int iItem, int iSubItem)
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


void GL3dBodyDlg::fillFrameDataTable()
{
	if(!m_pBody) return;
	int i;

	m_pFrameModel->setRowCount(m_pBody->frameCount());

	for(i=0; i<m_pBody->frameCount(); i++)
	{
		fillFrameTableRow(i);
	}
}


void GL3dBodyDlg::fillFrameTableRow(int row)
{
	QModelIndex ind;

	ind = m_pFrameModel->index(row, 0, QModelIndex());
	m_pFrameModel->setData(ind, m_pBody->frame(row)->m_Position.x * Units::mtoUnit());

	ind = m_pFrameModel->index(row, 1, QModelIndex());
	m_pFrameModel->setData(ind, m_pBody->m_xPanels[row]);

}


void GL3dBodyDlg::fillPointCell(int iItem, int iSubItem)
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


void GL3dBodyDlg::fillPointDataTable()
{
	if(!m_pBody) return;
	int i;

	m_pPointModel->setRowCount(m_pBody->sideLineCount());
	for(i=0; i<m_pBody->sideLineCount(); i++)
	{
		fillPointTableRow(i);
	}
}


void GL3dBodyDlg::fillPointTableRow(int row)
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


void GL3dBodyDlg::keyPressEvent(QKeyEvent *event)
{
	bool bShift = false;
	bool bCtrl  = false;
	if(event->modifiers() & Qt::ShiftModifier)   bShift =true;
	if(event->modifiers() & Qt::ControlModifier) bCtrl =true;

	switch (event->key())
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
				event->accept();
			}
			else event->ignore();
			break;
		}
		case Qt::Key_Y:
		{
			if(bCtrl)
			{
				onRedo();
				event->accept();
			}
			else event->ignore();
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!m_pctrlOK->hasFocus() && !m_pctrlCancel->hasFocus()) m_pctrlOK->setFocus();
			else if(m_pctrlOK->hasFocus()) accept();
			else if(m_pctrlCancel->hasFocus()) reject();
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
			event->ignore();
	}
}


void GL3dBodyDlg::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		default:
			event->ignore();
	}
}



void GL3dBodyDlg::setViewControls()
{
	m_pctrlX->setChecked(false);
	m_pctrlY->setChecked(false);
	m_pctrlZ->setChecked(false);
	m_pctrlIso->setChecked(false);
}


void GL3dBodyDlg::onBodyName()
{
	if(m_pBody)
	{
		m_pBody->m_BodyName = m_pctrlBodyName->text();
		m_pBody->m_BodyDescription = m_pctrlBodyDescription->toPlainText();
	}
}


void GL3dBodyDlg::onTextures()
{
	if(m_pBody) m_pBody->m_bTextures = m_pctrlTextures->isChecked();
	m_gl3dBodyview.resetGLBody(true);
	setControls();
	updateView();
}

void GL3dBodyDlg::onBodyColor()
{
	QColorDialog::ColorDialogOptions dialogOptions = QColorDialog::ShowAlphaChannel;
#ifdef Q_OS_MAC
#if QT_VERSION >= 0x040700
	dialogOptions |= QColorDialog::DontUseNativeDialog;
#endif
#endif
	QColor Color = QColorDialog::getColor(m_pBody->bodyColor(),
									  this, "Color selection", dialogOptions);


	if(Color.isValid())
	{
		m_pBody->bodyColor() = Color;
		m_gl3dBodyview.resetGLBody(true);

		m_pctrlBodyColor->setColor(Color);
		update();
	}
}


/**
 * Unselects all the 3D-view icons.
 */
void GL3dBodyDlg::onCheckViewIcons()
{
	m_pctrlIso->setChecked(false);
	m_pctrlX->setChecked(false);
	m_pctrlY->setChecked(false);
	m_pctrlZ->setChecked(false);
}



void GL3dBodyDlg::onBodyInertia()
{
	if(!m_pBody) return;
	InertiaDlg dlg(this);
	dlg.m_pBody  = m_pBody;
	dlg.m_pPlane = NULL;
	dlg.m_pWing  = NULL;
	dlg.initDialog();
	dlg.move(pos().x()+25, pos().y()+25);
	if(dlg.exec()==QDialog::Accepted) m_bChanged=true;
	m_pBody->computeBodyAxisInertia();
	m_bChanged = true;
	updateView();
}



void GL3dBodyDlg::onExportBodyXML()
{
	if(!m_pBody)return ;// is there anything to export ?

	QString filter = "XML file (*.xml)";
	QString FileName, strong;

	strong = m_pBody->bodyName();
	FileName = QFileDialog::getSaveFileName(this, tr("Export plane definition to xml file"),
											Settings::s_LastDirName +'/'+strong,
											filter,
											&filter);

	if(!FileName.length()) return;
	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	pos = FileName.indexOf(".xml", Qt::CaseInsensitive);
	if(pos<0) FileName += ".xml";


	QFile XFile(FileName);
	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	XMLPlaneWriter planeWriter(XFile);

	planeWriter.writeXMLBody(m_pBody);

	XFile.close();
}



void GL3dBodyDlg::onExportBodyDef()
{
	if(!m_pBody) return;

	QString FileName;

	FileName = m_pBody->m_BodyName;
	FileName.replace("/", " ");

	FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Definition"),
											Settings::s_LastDirName,
											QObject::tr("Text Format (*.txt)"));
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return;

	QTextStream outStream(&XFile);

	m_pBody->exportBodyDefinition(outStream, Units::mtoUnit());
}


void GL3dBodyDlg::onExportBodyGeom()
{
	if(!m_pBody) return;
	QString LengthUnit, FileName;

	Units::getLengthUnitLabel(LengthUnit);

	FileName = m_pBody->m_BodyName;
	FileName.replace("/", " ");

	int type = 1;

	QString filter =".csv";

	FileName = QFileDialog::getSaveFileName(this, QObject::tr("Export Body Geometry"),
											Settings::s_LastDirName ,
											QObject::tr("Text File (*.txt);;Comma Separated Values (*.csv)"),
											&filter);
	if(!FileName.length()) return;

	int pos = FileName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = FileName.left(pos);
	pos = FileName.lastIndexOf(".csv");
	if (pos>0) type = 2;

	QFile XFile(FileName);

	if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

	QTextStream out(&XFile);

	m_pBody->exportGeometry(out, Units::mtoUnit(), type, NXPOINTS, NHOOPPOINTS);
}


void GL3dBodyDlg::onImportBodyDef()
{
	Body memBody;
	memBody.duplicate(m_pBody);

	double mtoUnit = 1.0;

	LengthUnitDlg luDlg(this);

	luDlg.m_Question = QObject::tr("Choose the length unit to read this file :");
	luDlg.InitDialog(Units::lengthUnitIndex());

	if(luDlg.exec() == QDialog::Accepted)
	{
		mtoUnit = luDlg.mtoUnit();
	}
	else return;

	QString PathName;

	PathName = QFileDialog::getOpenFileName(this, QObject::tr("Open File"),
											Settings::s_LastDirName,
											QObject::tr("All files (*.*)"));
	if(!PathName.length()) return;
	int pos = PathName.lastIndexOf("/");
	if(pos>0) Settings::s_LastDirName = PathName.left(pos);

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

	m_gl3dBodyview.resetGLBody(true);

	m_bChanged = true;

	updateView();
}



void GL3dBodyDlg::onImportBodyXML()
{
//	Body memBody;
//	memBody.duplicate(m_pBody);

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
	XMLPlaneReader planeReader(XFile, &a_plane);
	planeReader.readXMLPlaneFile();

	XFile.close();

	if(planeReader.hasError())
	{
		QString errorMsg = planeReader.errorString() + QString("\nline %1 column %2").arg(planeReader.lineNumber()).arg(planeReader.columnNumber());
		QMessageBox::warning(this, "XML read", errorMsg, QMessageBox::Ok);
//		m_pBody->duplicate(&memBody);
		return;
	}

	m_pBody->duplicate(a_plane.body());
	setBody();

	m_gl3dBodyview.resetGLBody(true);


	m_bChanged = true;

	updateView();
}




void GL3dBodyDlg::readFrameSectionData(int sel)
{
	if(sel>=m_pFrameModel->rowCount()) return;
	double x;
	int k;

	bool bOK;
	QString strong;
	QStandardItem *pItem;

	pItem = m_pFrameModel->item(sel,0);

	strong = pItem->text();
	strong.replace(" ","");
	x = strong.toDouble(&bOK);
	if(bOK) m_pBody->frame(sel)->setuPosition(x / Units::mtoUnit());

	for(int ic=0; ic<m_pBody->frame(sel)->pointCount(); ic++)
	{
		m_pBody->frame(sel)->m_CtrlPoint[ic].x  = x / Units::mtoUnit();
	}

	pItem = m_pFrameModel->item(sel,1);
	strong = pItem->text();
	strong.replace(" ","");
	k = strong.toInt(&bOK);
	if(bOK) m_pBody->m_xPanels[sel] = k;
}


/** The user has clicked a point in the body line view */
void GL3dBodyDlg::onFrameClicked()
{
	m_pctrlFrameTable->selectRow(m_pBody->m_iActiveFrame);
}


void GL3dBodyDlg::onFrameItemClicked(const QModelIndex &index)
{
	m_pBody->m_iActiveFrame = index.row();
	setFrame(m_pBody->m_iActiveFrame);
	updateView();
}


void GL3dBodyDlg::onFrameCellChanged(QWidget *)
{
	takePicture();
	m_bChanged = true;
//	int n = m_pBody->m_iActiveFrame;
	readFrameSectionData(m_pBody->m_iActiveFrame);
	m_gl3dBodyview.resetGLBody(true);


	updateView();
}


void GL3dBodyDlg::onGrid()
{
	m_pBodyGridDlg->initDialog();
	m_pBodyGridDlg->exec();

	updateView();
}




void GL3dBodyDlg::onLineType()
{
	m_bChanged = true;
	if(m_pctrlFlatPanels->isChecked())
	{
		m_pBody->m_LineType = XFLR5::BODYPANELTYPE;
		m_pctrlNXPanels->setEnabled(false);
		m_pctrlNHoopPanels->setEnabled(false);
		m_pctrlXDegree->setEnabled(false);
		m_pctrlHoopDegree->setEnabled(false);
	}
	else
	{
		m_pBody->m_LineType = XFLR5::BODYSPLINETYPE;
		m_pctrlNXPanels->setEnabled(true);
		m_pctrlNHoopPanels->setEnabled(true);
		m_pctrlXDegree->setEnabled(true);
		m_pctrlHoopDegree->setEnabled(true);
	}
	m_gl3dBodyview.resetGLBody(true);

	updateView();
}


void GL3dBodyDlg::onPointCellChanged(QWidget *)
{
	if(!m_pFrame) return;

	takePicture();
	m_bChanged = true;
	for(int ip=0; ip<m_pPointModel->rowCount(); ip++)
		readPointSectionData(ip);
	m_gl3dBodyview.resetGLBody(true);
	updateView();
}


/** The user has clicked a point in the frame view */
void GL3dBodyDlg::onPointClicked()
{
	if(m_pFrame)
		m_pctrlPointTable->selectRow(m_pFrame->s_iSelect);
}


void GL3dBodyDlg::onPointItemClicked(const QModelIndex &index)
{
	if(!m_pFrame) return;
	m_pFrame->s_iSelect = index.row();
	m_pFrame->s_iHighlight = index.row();
	updateView();
}



void GL3dBodyDlg::onResetScales()
{
	m_gl3dBodyview.on3DReset();
	m_pBodyLineWidget->onResetScales();
	m_pFrameWidget->onResetScales();
	updateView();
}


void GL3dBodyDlg::onScaleBody()
{
	if(!m_pBody) return;

	BodyScaleDlg dlg(this);

	dlg.m_FrameID = m_pBody->m_iActiveFrame;
	dlg.initDialog();

	if(dlg.exec()==QDialog::Accepted)
	{
		takePicture();
		m_pBody->scale(dlg.m_XFactor, dlg.m_YFactor, dlg.m_ZFactor, dlg.m_bFrameOnly, dlg.m_FrameID);
		m_gl3dBodyview.resetGLBody(true);

		fillFrameDataTable();
		fillPointDataTable();

		updateView();
	}
}


void GL3dBodyDlg::onUpdateBody()
{
	m_bChanged = true;
	m_gl3dBodyview.resetGLBody(true);

	fillFrameDataTable();
	fillPointDataTable();
	updateView();
}


void GL3dBodyDlg::onSelChangeXDegree(int sel)
{
	if(!m_pBody) return;
	if (sel <0) return;

	takePicture();
	m_bChanged = true;

	m_pBody->m_SplineSurface.m_iuDegree = sel+1;
	m_pBody->setNURBSKnots();
	m_gl3dBodyview.resetGLBody(true);

	updateView();
}


void GL3dBodyDlg::onSelChangeHoopDegree(int sel)
{
	if(!m_pBody) return;
	if (sel <0) return;

	m_bChanged = true;

	takePicture();

	m_pBody->m_SplineSurface.m_ivDegree = sel+1;
	m_pBody->setNURBSKnots();
	m_gl3dBodyview.resetGLBody(true);

	updateView();
}


void GL3dBodyDlg::onEdgeWeight()
{
/*	if(!m_pBody) return;

	m_bChanged = true;
	takePicture();

	double w= (double)m_pctrlEdgeWeight->value()/100.0 + 1.0;
	m_pBody->setEdgeWeight(w, w);

	m_bResetglBody   = true;
	updateView();*/
}



void GL3dBodyDlg::onNURBSPanels()
{
	if(!m_pBody) return;

	m_bChanged = true;
	takePicture();

	m_pBody->m_Bunch = m_pctrlPanelBunch->sliderPosition()/100.0;

	m_pBody->m_nhPanels = m_pctrlNHoopPanels->value();
	m_pBody->m_nxPanels = m_pctrlNXPanels->value();
	m_pBody->setPanelPos();

	m_gl3dBodyview.resetGLBody(true);

	updateView();
}





void GL3dBodyDlg::onTranslateBody()
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

		m_gl3dBodyview.resetGLBody(true);

		updateView();
	}
}



void GL3dBodyDlg::readPointSectionData(int sel)
{
	if(sel>=m_pPointModel->rowCount()) return;
	if(!m_pFrame) return;

	double d;
	int k;

	bool bOK;
	QString strong;
	QStandardItem *pItem;

	pItem = m_pPointModel->item(sel,0);

	strong = pItem->text();
	strong.replace(" ","");
	d =strong.toDouble(&bOK);
	if(bOK) m_pFrame->m_CtrlPoint[sel].y =d / Units::mtoUnit();

	pItem = m_pPointModel->item(sel,1);
	strong = pItem->text();
	strong.replace(" ","");
	d =strong.toDouble(&bOK);
	if(bOK) m_pFrame->m_CtrlPoint[sel].z =d / Units::mtoUnit();

	pItem = m_pPointModel->item(sel,2);
	strong = pItem->text();
	strong.replace(" ","");
	k =strong.toInt(&bOK);
	if(bOK) m_pBody->m_hPanels[sel] = k;
}


void GL3dBodyDlg::accept()
{
	if(m_pBody)
	{
		m_pBody->bodyDescription() = m_pctrlBodyDescription->toPlainText();
		m_pBody->bodyColor() = m_pctrlBodyColor->color();
	}

	s_bWindowMaximized= isMaximized();
	s_WindowPos = pos();
	s_WindowSize = size();

	s_bOutline    = m_gl3dBodyview.m_bOutline;
	s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
	s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
	s_bAxes       = m_gl3dBodyview.m_bAxes;
	s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
	s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

	done(QDialog::Accepted);
}


void GL3dBodyDlg::reject()
{
	s_bWindowMaximized= isMaximized();
	s_WindowPos = pos();
	s_WindowSize = size();

	s_bOutline    = m_gl3dBodyview.m_bOutline;
	s_bSurfaces   = m_gl3dBodyview.m_bSurfaces;
	s_bVLMPanels  = m_gl3dBodyview.m_bVLMPanels;
	s_bAxes       = m_gl3dBodyview.m_bAxes;
	s_bShowMasses = m_gl3dBodyview.m_bShowMasses;
	s_bFoilNames  = m_gl3dBodyview.m_bFoilNames;

	if(m_bChanged)
	{
		m_pBody->m_BodyName = m_pctrlBodyName->text();

		int res = QMessageBox::question(this, tr("Body Dlg Exit"), tr("Save the Body ?"), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
		if (QMessageBox::No == res)
		{
			m_pBody = NULL;
			QDialog::reject();
		}
		else if (QMessageBox::Cancel == res) return;
		else
		{
			m_pBody = NULL;
			done(QDialog::Accepted);
			return;
		}
	}
	else m_pBody = NULL;

	done(QDialog::Rejected);
}



void GL3dBodyDlg::resizeEvent(QResizeEvent *event)
{
//	SetBodyScale();
//	SetRectangles();

	resizeTables();
	event->accept();
}



bool GL3dBodyDlg::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GL3dBody");
	{
		s_WindowPos =  pSettings->value("BodyWindowPos", QPoint(20,20)).toPoint();
		s_WindowSize = pSettings->value("BodyWindowSize", QSize(900,700)).toSize();
		s_bWindowMaximized        = pSettings->value("WindowMaximized", false).toBool();
		m_HorizontalSplitterSizes = pSettings->value("HorizontalSplitterSizes").toByteArray();
		m_LeftSplitterSizes = pSettings->value("LeftSplitterSizes").toByteArray();
	}
	pSettings->endGroup();
	BodyGridDlg::loadSettings(pSettings);
	return true;
}



bool GL3dBodyDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("GL3dBody");
	{
		pSettings->setValue("BodyWindowPos",s_WindowPos);
		pSettings->setValue("BodyWindowSize",s_WindowSize);
		pSettings->setValue("WindowMaximized", s_bWindowMaximized);
		pSettings->setValue("HorizontalSplitterSizes", m_HorizontalSplitterSizes);
		pSettings->setValue("LeftSplitterSizes", m_LeftSplitterSizes);

	}
	pSettings->endGroup();
	BodyGridDlg::saveSettings(pSettings);
	return true;
}



void GL3dBodyDlg::setControls()
{
	m_pctrlBodyName->setEnabled(m_bEnableName);

	m_pctrlBodyColor->setEnabled(m_pctrlColor->isChecked());

	m_pctrlOutline->setChecked(m_gl3dBodyview.m_bOutline);
	m_pctrlPanels->setChecked(m_gl3dBodyview.m_bVLMPanels);
	m_pctrlAxes->setChecked(m_gl3dBodyview.m_bAxes);
	m_pctrlShowMasses->setChecked(m_gl3dBodyview.m_bShowMasses);
	m_pctrlSurfaces->setChecked(m_gl3dBodyview.m_bSurfaces);


	m_pctrlUndo->setEnabled(m_StackPos>0);
	m_pctrlRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);

//	m_pctrlEdgeWeight->setSliderPosition((int)((m_pBody->m_SplineSurface.m_EdgeWeightu-1.0)*100.0));

	if(m_pBody && m_pBody->m_LineType==XFLR5::BODYPANELTYPE)
	{
		m_pctrlNXPanels->setEnabled(false);
		m_pctrlNHoopPanels->setEnabled(false);
		m_pctrlXDegree->setEnabled(false);
		m_pctrlHoopDegree->setEnabled(false);
	}
	else if(m_pBody && m_pBody->m_LineType==XFLR5::BODYSPLINETYPE)
	{
		m_pctrlNXPanels->setEnabled(true);
		m_pctrlNHoopPanels->setEnabled(true);
		m_pctrlXDegree->setEnabled(true);
		m_pctrlHoopDegree->setEnabled(true);
	}

	if(m_pBody)
	{
		m_pctrlPanelBunch->setSliderPosition((int)(m_pBody->m_Bunch*100.0));
		m_pctrlBodyColor->setColor(m_pBody->m_BodyColor);

		m_pctrlNXPanels->setValue(m_pBody->m_nxPanels);
		m_pctrlNHoopPanels->setValue(m_pBody->m_nhPanels);

		m_pctrlXDegree->setCurrentIndex(m_pBody->m_SplineSurface.m_iuDegree-1);
		m_pctrlHoopDegree->setCurrentIndex(m_pBody->m_SplineSurface.m_ivDegree-1);
	}
}


bool GL3dBodyDlg::initDialog(Body *pBody)
{
	if(!pBody) return false;

	m_pctrlFrameTable->setFont(Settings::s_TableFont);
	m_pctrlPointTable->setFont(Settings::s_TableFont);

	m_gl3dBodyview.setBody(pBody);

	return setBody(pBody);
}


bool GL3dBodyDlg::setBody(Body *pBody)
{
	if(pBody) m_pBody = pBody;

	m_pctrlColor->setChecked(!m_pBody->textures());
	m_pctrlTextures->setChecked(m_pBody->textures());

	m_pctrlFlatPanels->setChecked(m_pBody->m_LineType==XFLR5::BODYPANELTYPE);
	m_pctrlBSplines->setChecked(m_pBody->m_LineType==XFLR5::BODYSPLINETYPE);

	m_pBodyLineWidget->setBody(m_pBody);
	m_pFrameWidget->setBody(m_pBody);

	m_pFrame = m_pBody->activeFrame();

	setControls();
	fillFrameDataTable();
	fillPointDataTable();

	m_pctrlBodyName->setText(m_pBody->m_BodyName);

	takePicture();

	return true;
}


void GL3dBodyDlg::setFrame(int iFrame)
{
	if(!m_pBody) return;
	if(iFrame<0 || iFrame>=m_pBody->frameCount()) m_pFrame = NULL;
	else                                          m_pFrame = m_pBody->frame(iFrame);
	m_pBody->m_iActiveFrame = iFrame;

	m_gl3dBodyview.resetGLBody(true);

	fillPointDataTable();;
}


void GL3dBodyDlg::setFrame(Frame *pFrame)
{
	if(!m_pBody || !pFrame) return;

	m_pBody->setActiveFrame(pFrame);

	m_gl3dBodyview.resetGLBody(true);

	fillPointDataTable();;
}



void GL3dBodyDlg::setupLayout()
{
	int i;
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

	m_gl3dBodyview.m_pParent = this;
	m_gl3dBodyview.m_bOutline    = s_bOutline;
	m_gl3dBodyview.m_bSurfaces   = s_bSurfaces;
	m_gl3dBodyview.m_bVLMPanels  = s_bVLMPanels;
	m_gl3dBodyview.m_bAxes       = s_bAxes;
	m_gl3dBodyview.m_bShowMasses = s_bShowMasses;
	m_gl3dBodyview.m_bFoilNames  = s_bFoilNames;

	QVBoxLayout *pControlsLayout = new QVBoxLayout;
	{
		QGridLayout *pThreeDParamsLayout = new QGridLayout;
		{
			m_pctrlAxes       = new QCheckBox(tr("Axes"));
			m_pctrlSurfaces   = new QCheckBox(tr("Surfaces"));
			m_pctrlOutline    = new QCheckBox(tr("Outline"));
			m_pctrlPanels     = new QCheckBox(tr("Panels"));
			m_pctrlShowMasses = new QCheckBox(tr("Masses"));
			m_pctrlAxes->setSizePolicy(szPolicyMinimum);
			m_pctrlSurfaces->setSizePolicy(szPolicyMinimum);
			m_pctrlOutline->setSizePolicy(szPolicyMinimum);
			m_pctrlPanels->setSizePolicy(szPolicyMinimum);
			pThreeDParamsLayout->addWidget(m_pctrlAxes, 1,1);
			pThreeDParamsLayout->addWidget(m_pctrlPanels, 1,2);
			pThreeDParamsLayout->addWidget(m_pctrlSurfaces, 2,1);
			pThreeDParamsLayout->addWidget(m_pctrlOutline, 2,2);
			pThreeDParamsLayout->addWidget(m_pctrlShowMasses, 2,3);
		}

		QHBoxLayout *pAxisViewLayout = new QHBoxLayout;
		{
			m_pctrlX          = new QToolButton;
			m_pctrlY          = new QToolButton;
			m_pctrlZ          = new QToolButton;
			m_pctrlIso        = new QToolButton;
			m_pctrlFlip       = new QToolButton;
			if(m_pctrlX->iconSize().height()<=48)
			{
				m_pctrlX->setIconSize(QSize(32,32));
				m_pctrlY->setIconSize(QSize(32,32));
				m_pctrlZ->setIconSize(QSize(32,32));
				m_pctrlIso->setIconSize(QSize(32,32));
				m_pctrlFlip->setIconSize(QSize(32,32));
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

		QHBoxLayout* pThreeDViewLayout = new QHBoxLayout;
		{
			m_pctrlReset      = new QPushButton(tr("Reset Scale"));
			m_pctrlReset->setSizePolicy(szPolicyMinimum);

			pThreeDViewLayout->addWidget(m_pctrlReset);
		}


		QHBoxLayout *pActionButtonsLayout = new QHBoxLayout;
		{
			m_pctrlUndo = new QPushButton(QIcon(":/images/OnUndo.png"), tr("Undo"));
			m_pctrlRedo = new QPushButton(QIcon(":/images/OnRedo.png"), tr("Redo"));

			m_pctrlMenuButton = new QPushButton(tr("Other"));

			BodyMenu = new QMenu(tr("Actions..."),this);

			BodyMenu->addAction(m_pGrid);
			BodyMenu->addSeparator();
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
			m_pctrlMenuButton->setMenu(BodyMenu);

			pActionButtonsLayout->addWidget(m_pctrlUndo);
			pActionButtonsLayout->addWidget(m_pctrlRedo);
			pActionButtonsLayout->addWidget(m_pctrlMenuButton);
		}

		QHBoxLayout *pCommandButtonsLayout = new QHBoxLayout;
		{
			m_pctrlOK = new QPushButton(tr("Save and Close"));
			m_pctrlOK->setAutoDefault(true);
			m_pctrlCancel = new QPushButton(tr("Cancel"));
			m_pctrlCancel->setAutoDefault(false);
			pCommandButtonsLayout->addWidget(m_pctrlOK);
			pCommandButtonsLayout->addWidget(m_pctrlCancel);
		}

		pControlsLayout->addLayout(pAxisViewLayout);
		pControlsLayout->addLayout(pThreeDParamsLayout);
		pControlsLayout->addLayout(pThreeDViewLayout);
		pControlsLayout->addStretch(1);
		pControlsLayout->addLayout(pActionButtonsLayout);
		pControlsLayout->addStretch(1);
		pControlsLayout->addLayout(pCommandButtonsLayout);
	}


	QVBoxLayout *pBodyParamsLayout = new QVBoxLayout;
	{
		m_pctrlBodyName = new QLineEdit(tr("BodyName"));

		QGroupBox *pStyleBox = new QGroupBox(tr("Style"));
		{
			QHBoxLayout *pStyleLayout = new QHBoxLayout;
			{
				m_pctrlTextures = new QRadioButton(tr("Textures"));
				m_pctrlColor    = new QRadioButton(tr("Color"));
				pStyleLayout->addWidget(m_pctrlTextures);
				pStyleLayout->addWidget(m_pctrlColor);

				m_pctrlBodyColor = new ColorButton(this);
				m_pctrlBodyColor->setSizePolicy(szPolicyMinimum);
				pStyleLayout->addStretch();
				pStyleLayout->addWidget(m_pctrlBodyColor);
			}
			pStyleBox->setLayout(pStyleLayout);
		}
		QLabel *BodyDes = new QLabel(tr("Description:"));

		m_pctrlBodyDescription = new QTextEdit();
		m_pctrlBodyDescription->setToolTip(tr("Enter here a short description for the body"));
		pBodyParamsLayout->setStretchFactor(m_pctrlBodyDescription,1);

		pBodyParamsLayout->addWidget(m_pctrlBodyName);
		pBodyParamsLayout->addWidget(pStyleBox);
		pBodyParamsLayout->addWidget(BodyDes);
		pBodyParamsLayout->addWidget(m_pctrlBodyDescription);
		pBodyParamsLayout->addStretch(1);
	}


	QVBoxLayout *pBodySettingsLayout = new QVBoxLayout;
	{
		QGroupBox *pBodyTypeBox = new QGroupBox(tr("Type"));
		{
			QHBoxLayout *pBodyTypeLayout = new QHBoxLayout;
			{
				m_pctrlFlatPanels = new QRadioButton(tr("Flat Panels"));
				m_pctrlBSplines   = new QRadioButton(tr("BSplines"));
				m_pctrlFlatPanels->setSizePolicy(szPolicyMinimum);
				m_pctrlBSplines->setSizePolicy(szPolicyMinimum);
				pBodyTypeLayout->addWidget(m_pctrlFlatPanels);
				pBodyTypeLayout->addWidget(m_pctrlBSplines);
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

			m_pctrlXDegree = new QComboBox;
			m_pctrlHoopDegree = new QComboBox;
			m_pctrlNXPanels = new DoubleEdit;
			m_pctrlNHoopPanels = new DoubleEdit;
/*			m_pctrlEdgeWeight = new QSlider(Qt::Horizontal);
			m_pctrlEdgeWeight->setMinimum(0);
			m_pctrlEdgeWeight->setMaximum(100);
			m_pctrlEdgeWeight->setSliderPosition(1);
			m_pctrlEdgeWeight->setTickInterval(10);
			m_pctrlEdgeWeight->setTickPosition(QSlider::TicksBelow);
			m_pctrlEdgeWeight->setSizePolicy(szPolicyMinimum);*/

			m_pctrlPanelBunch= new QSlider(Qt::Horizontal);
			m_pctrlPanelBunch->setMinimum(0	);
			m_pctrlPanelBunch->setMaximum(100.0);
			m_pctrlPanelBunch->setSliderPosition(0);
			m_pctrlPanelBunch->setTickInterval(10);
			m_pctrlPanelBunch->setTickPosition(QSlider::TicksBelow);
			m_pctrlPanelBunch->setSizePolicy(szPolicyMinimum);


			lab1->setSizePolicy(szPolicyMinimum);
			lab2->setSizePolicy(szPolicyMinimum);
			lab3->setSizePolicy(szPolicyMinimum);
			lab4->setSizePolicy(szPolicyMinimum);
			m_pctrlXDegree->setSizePolicy(szPolicyMinimum);
			m_pctrlHoopDegree->setSizePolicy(szPolicyMinimum);
			m_pctrlNXPanels->setSizePolicy(szPolicyMinimum);
			m_pctrlNHoopPanels->setSizePolicy(szPolicyMinimum);
			m_pctrlNXPanels->setPrecision(0);
			m_pctrlNHoopPanels->setPrecision(0);
			pSplineParams->addWidget(lab1,1,2, Qt::AlignCenter);
			pSplineParams->addWidget(lab2,1,3, Qt::AlignCenter);
			pSplineParams->addWidget(lab3,2,1, Qt::AlignRight);
			pSplineParams->addWidget(lab4,3,1, Qt::AlignRight);
			pSplineParams->addWidget(m_pctrlXDegree,2,2);
			pSplineParams->addWidget(m_pctrlHoopDegree,2,3);
			pSplineParams->addWidget(m_pctrlNXPanels,3,2);
			pSplineParams->addWidget(m_pctrlNHoopPanels,3,3);
//			SplineParams->addWidget(labWeight,4,1);
//			SplineParams->addWidget(m_pctrlEdgeWeight,4,2,1,2);
			pSplineParams->addWidget(labBunch,5,1);
			pSplineParams->addWidget(m_pctrlPanelBunch,5,2,1,2);
		}

		pBodySettingsLayout->addWidget(pBodyTypeBox);
		pBodySettingsLayout->addStretch();
		pBodySettingsLayout->addLayout(pSplineParams);
	}


	QVBoxLayout * pFramePosLayout = new QVBoxLayout;
	{
		m_pctrlFrameTable = new QTableView;
	//	m_pctrlFrameTable->setSizePolicy(szPolicyMinimum);
		m_pctrlFrameTable->setWindowTitle(tr("Frames"));
		QLabel *LabelFrame = new QLabel(tr("Frame Positions"));
		LabelFrame->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		pFramePosLayout->addWidget(LabelFrame);
	//	FramePosLayout->addStretch(1);
		m_pctrlFrameTable->setSelectionMode(QAbstractItemView::SingleSelection);
		m_pctrlFrameTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pctrlFrameTable->setEditTriggers(QAbstractItemView::CurrentChanged |
										   QAbstractItemView::DoubleClicked |
										   QAbstractItemView::SelectedClicked |
										   QAbstractItemView::EditKeyPressed |
										   QAbstractItemView::AnyKeyPressed);
		pFramePosLayout->addWidget(m_pctrlFrameTable);
	}


	QVBoxLayout * pFramePointLayout = new QVBoxLayout;
	{
		m_pctrlPointTable = new QTableView;
	//	m_pctrlPointTable->setSizePolicy(szPolicyMinimum);
		m_pctrlPointTable->setWindowTitle(tr("Points"));
		QLabel *LabelPoints = new QLabel(tr("Current Frame Definition"));
		LabelPoints->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
		pFramePointLayout->addWidget(LabelPoints);
	//	FramePointLayout->addStretch(1);
		m_pctrlPointTable->setSelectionMode(QAbstractItemView::SingleSelection);
		m_pctrlPointTable->setSelectionBehavior(QAbstractItemView::SelectRows);
		m_pctrlPointTable->setEditTriggers(QAbstractItemView::CurrentChanged |
										   QAbstractItemView::DoubleClicked |
										   QAbstractItemView::SelectedClicked |
										   QAbstractItemView::EditKeyPressed |
										   QAbstractItemView::AnyKeyPressed);
		pFramePointLayout->addWidget(m_pctrlPointTable);
	}

	m_pctrlControlsWidget = new QWidget;
	{
		QHBoxLayout *pAllControls = new QHBoxLayout;
		{
			pAllControls->addLayout(pBodyParamsLayout);
			pAllControls->addStretch(1);
			pAllControls->addLayout(pBodySettingsLayout);
			pAllControls->addStretch(1);
			pAllControls->addLayout(pFramePosLayout);
			pAllControls->addStretch(1);
			pAllControls->addLayout(pFramePointLayout);
			pAllControls->addStretch(1);
			pAllControls->addLayout(pControlsLayout);
		}
		m_pctrlControlsWidget->setLayout(pAllControls);
	}


	m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
	{
		m_pLeftSplitter = new QSplitter(Qt::Vertical, this);
		{
			m_pBodyLineWidget = new BodyLineWidget(this);
			m_pBodyLineWidget->setSizePolicy(szPolicyMaximum);
			m_pBodyLineWidget->sizePolicy().setVerticalStretch(2);

			m_pLeftSplitter->addWidget(m_pBodyLineWidget);
			m_pLeftSplitter->addWidget(&m_gl3dBodyview);
		}
		m_pFrameWidget = new BodyFrameWidget(this);
		m_pHorizontalSplitter->addWidget(m_pLeftSplitter);
		m_pHorizontalSplitter->addWidget(m_pFrameWidget);
	}

	m_pVerticalSplitter = new QSplitter(Qt::Vertical, this);
	{
		m_pVerticalSplitter->addWidget(m_pHorizontalSplitter);
		m_pVerticalSplitter->addWidget(m_pctrlControlsWidget);
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		pMainLayout->addWidget(m_pVerticalSplitter);
	}
	setLayout(pMainLayout);

	for (i=1; i<6; i++)
	{
		str = QString("%1").arg(i);
		m_pctrlXDegree->addItem(str);
		m_pctrlHoopDegree->addItem(str);
	}

	//Setup Frame table
	m_pctrlFrameTable->horizontalHeader()->setStretchLastSection(true);

	m_pFrameModel = new QStandardItemModel;
	m_pFrameModel->setRowCount(10);//temporary
	m_pFrameModel->setColumnCount(2);

	m_pctrlFrameTable->setModel(m_pFrameModel);

	m_pSelectionModelFrame = new QItemSelectionModel(m_pFrameModel);
	m_pctrlFrameTable->setSelectionModel(m_pSelectionModelFrame);

	m_pFrameDelegate = new BodyTableDelegate(this);
	m_pctrlFrameTable->setItemDelegate(m_pFrameDelegate);
	m_pFramePrecision = new int[2];
	m_pFramePrecision[0] = 3;//five digits for x and y coordinates
	m_pFramePrecision[1] = 0;
	m_pFrameDelegate->setPointer(m_pFramePrecision);

	//Setup Point Table
	m_pctrlPointTable->horizontalHeader()->setStretchLastSection(true);

	m_pPointModel = new QStandardItemModel(this);
	m_pPointModel->setRowCount(10);//temporary
	m_pPointModel->setColumnCount(3);
	m_pctrlPointTable->setModel(m_pPointModel);
	m_pSelectionModelPoint = new QItemSelectionModel(m_pPointModel);
	m_pctrlPointTable->setSelectionModel(m_pSelectionModelPoint);

	m_pPointDelegate = new BodyTableDelegate;
	m_pctrlPointTable->setItemDelegate(m_pPointDelegate);
	m_pPointPrecision = new int[3];
	m_pPointPrecision[0] = 3;//five digits for x and y coordinates
	m_pPointPrecision[1] = 3;
	m_pPointPrecision[2] = 0;
	m_pPointDelegate->setPointer(m_pPointPrecision);
}


void GL3dBodyDlg::showEvent(QShowEvent *event)
{
	move(s_WindowPos);
	resize(s_WindowSize);
	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);
	if(m_VerticalSplitterSizes.length()>0)
		m_pHorizontalSplitter->restoreState(m_VerticalSplitterSizes);
	if(m_HorizontalSplitterSizes.length()>0)
		m_pHorizontalSplitter->restoreState(m_HorizontalSplitterSizes);
	if(m_LeftSplitterSizes.length()>0)
		m_pLeftSplitter->restoreState(m_LeftSplitterSizes);

	setTableUnits();
	m_bChanged    = false;
	m_gl3dBodyview.resetGLBody(true);

	resizeTables();

	updateView();

	event->accept();
}


/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void GL3dBodyDlg::hideEvent(QHideEvent *event)
{
	s_WindowPos = pos();
	s_WindowSize = size();
	s_bWindowMaximized = isMaximized();
	m_VerticalSplitterSizes  = m_pVerticalSplitter->saveState();
	m_HorizontalSplitterSizes  = m_pHorizontalSplitter->saveState();
	m_LeftSplitterSizes  = m_pLeftSplitter->saveState();
	event->accept();
}


void GL3dBodyDlg::resizeTables()
{
	int ColumnWidth = (int)((double)(m_pctrlFrameTable->width())/2.5);
	m_pctrlFrameTable->setColumnWidth(0,ColumnWidth);
	m_pctrlFrameTable->setColumnWidth(1,ColumnWidth);
//	m_pctrlFrameTable->setColumnWidth(2,ColumnWidth);
	ColumnWidth = (int)((double)(m_pctrlPointTable->width())/4);
	m_pctrlPointTable->setColumnWidth(0,ColumnWidth);
	m_pctrlPointTable->setColumnWidth(1,ColumnWidth);
	m_pctrlPointTable->setColumnWidth(2,ColumnWidth);
}



/**
  * Clears the stack starting at a given position.
  * @param the first stack element to remove
  */
void GL3dBodyDlg::clearStack(int pos)
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
void GL3dBodyDlg::setPicture()
{
	Body *pTmpBody = m_UndoStack.at(m_StackPos);
	m_pBody->duplicate(pTmpBody);
	m_pBody->setNURBSKnots();
	fillFrameDataTable();
	m_pFrame = m_pBody->activeFrame();
	fillPointDataTable();
	m_gl3dBodyview.resetGLBody(true);

	updateView();
}



/**
 * Copies the current Body object to a new Body and pushes it on the stack.
 */
void GL3dBodyDlg::takePicture()
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
	m_pctrlUndo->setEnabled(m_StackPos>0);
	m_pctrlRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::onRedo()
{
	if(m_StackPos<m_UndoStack.size()-1)
	{
		m_StackPos++;
		setPicture();
	}
	m_pctrlUndo->setEnabled(m_StackPos>0);
	m_pctrlRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::onUndo()
{
	if(m_StackPos>0)
	{
		m_StackPos--;
		setPicture();
		m_pctrlRedo->setEnabled(true);
	}
	else
	{
		//nothing to restore
	}
	m_pctrlUndo->setEnabled(m_StackPos>0);
	m_pctrlRedo->setEnabled(m_StackPos<m_UndoStack.size()-1);
}


void GL3dBodyDlg::updateView()
{
	if(isVisible()) m_gl3dBodyview.update();
	m_pFrameWidget->update();
	m_pBodyLineWidget->update();
}


void GL3dBodyDlg::blockSignalling(bool bBlock)
{
	blockSignals(bBlock);
	m_pPointDelegate->blockSignals(bBlock);
	m_pFrameDelegate->blockSignals(bBlock);
	m_pctrlPointTable->blockSignals(bBlock);
	m_pctrlFrameTable->blockSignals(bBlock);

	m_pSelectionModelPoint->blockSignals(bBlock);
	m_pSelectionModelFrame->blockSignals(bBlock);
}

