/****************************************************************************

	ViewObjectDlg Class
	Copyright (C) 2015 Andre Deperrois adeperrois@xflr5.com

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

#include <misc/Units.h>
#include <misc/Settings.h>
#include <miarex/view/W3dPrefsDlg.h>
#include <objects3d/Surface.h>
#include "wingseldlg.h"
#include "EditPlaneDlg.h"
#include <globals.h>

#include <QApplication>
#include <QAction>
#include <QMenu>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>

QSize EditPlaneDlg::s_WindowSize(1031,783);
QPoint EditPlaneDlg::s_WindowPosition(131, 77);
bool EditPlaneDlg::s_bWindowMaximized =false;
bool EditPlaneDlg::s_bAutoRedraw = true;
QByteArray EditPlaneDlg::m_HorizontalSplitterSizes;


bool EditPlaneDlg::s_bOutline    = true;
bool EditPlaneDlg::s_bSurfaces   = true;
bool EditPlaneDlg::s_bVLMPanels  = false;
bool EditPlaneDlg::s_bAxes       = true;
bool EditPlaneDlg::s_bShowMasses = false;
bool EditPlaneDlg::s_bFoilNames  = false;


#define SECTIONHIGHLIGHT    1702



EditPlaneDlg::EditPlaneDlg(QWidget *pParent) : QDialog(pParent)
{
	setWindowTitle("Plane object explorer");
	setWindowFlags(Qt::Window);

	m_pPlane = NULL;
	m_pStruct = NULL;
	m_pDelegate = NULL;
	m_pModel = NULL;

	m_enumActiveObject = NOOBJECT;
	m_enumActiveWingType = XFLR5::OTHERWING;
	m_iActiveSection   = -1;
	m_iActiveFrame     = -1;
	m_iActivePointMass = -1;

	m_bResetglSectionHighlight = true;
	m_bResetglPlane            = true;
	m_bResetglBody             = true;
	m_bChanged                 = false;

	m_pInsertBefore  = new QAction(tr("Insert Before"), this);
	m_pInsertAfter   = new QAction(tr("Insert after"), this);
	m_pDeleteItem = new QAction(tr("Delete"), this);

	m_pContextMenu = new QMenu(tr("Section"),this);
	m_pContextMenu->addAction(m_pInsertBefore);
	m_pContextMenu->addAction(m_pInsertAfter);
	m_pContextMenu->addAction(m_pDeleteItem);

	m_PixText = QPixmap(107, 97);
	m_PixText.fill(Qt::transparent);

	setupLayout();
}


/**
 * Overrides the base class showEvent method. Moves the window to its former location.
 * @param event the showEvent.
 */
void EditPlaneDlg::showEvent(QShowEvent *event)
{
	move(s_WindowPosition);
	resize(s_WindowSize);
	m_pHorizontalSplitter->restoreState(m_HorizontalSplitterSizes);
	resizeTreeView();
	if(s_bWindowMaximized) setWindowState(Qt::WindowMaximized);

	m_pglPlaneView->update();
	event->accept();
}



/**
 * Overrides the base class hideEvent method. Stores the window's current position.
 * @param event the hideEvent.
 */
void EditPlaneDlg::hideEvent(QHideEvent *event)
{
	m_HorizontalSplitterSizes = m_pHorizontalSplitter->saveState();
	s_WindowPosition = pos();
	event->accept();
}


void EditPlaneDlg::resizeEvent(QResizeEvent *event)
{
	resizeTreeView();
	resize3DView();
	event->accept();
}



void EditPlaneDlg::onResize()
{
	resizeTreeView();
//	resize3DView();
}


void EditPlaneDlg::resizeTreeView()
{
	QList<int> leftSizes;
	leftSizes.append((int)(height()*95/100));
	leftSizes.append((int)(height()*5/100));
	m_pLeftSideSplitter->setSizes(leftSizes);

	QList<int> rightSizes;
	rightSizes.append((int)(height()*95/100));
	rightSizes.append((int)(height()*5/100));
	m_pRightSideSplitter->setSizes(rightSizes);

	int ColumnWidth = (int)((double)(m_pStruct->width())/15);
	m_pStruct->setColumnWidth(0,ColumnWidth*6);
	m_pStruct->setColumnWidth(1,ColumnWidth*3);
	m_pStruct->setColumnWidth(2,ColumnWidth*3);
}


void EditPlaneDlg::resize3DView()
{

	if(m_pglPlaneView->width()>0 && m_pglPlaneView->height()>0)
	{
		m_PixText = m_PixText.scaled(m_pglPlaneView->rect().size());
		m_PixText.fill(Qt::transparent);

		QPainter paint(&m_PixText);
		paintPlaneLegend(paint, m_pPlane, m_pglPlaneView->rect());

	}
}


void EditPlaneDlg::contextMenuEvent(QContextMenuEvent *event)
{
	if(m_iActiveFrame<0 && m_iActivePointMass<0 && m_iActiveSection<0)
	{
		WingSelDlg wsDlg;
		wsDlg.initDialog(m_pPlane);
		wsDlg.move(event->globalPos());
		wsDlg.exec();
		fillPlaneTreeView();
		onRedraw();
		return;
	}
	if(m_iActiveFrame>=0)
	{
		m_pInsertBefore->setText(tr("Insert body frame before"));
		m_pInsertAfter->setText(tr("Insert body frame after"));
		m_pDeleteItem->setText(tr("Delete body frame"));
	}
	else if(m_iActiveSection>=0)
	{
		m_pInsertBefore->setText(tr("Insert wing section before"));
		m_pInsertAfter->setText(tr("Insert wing section after"));
		m_pDeleteItem->setText(tr("Delete wing section"));
	}
	else if(m_iActivePointMass>=0)
	{
		m_pInsertBefore->setText(tr("Insert point mass before"));
		m_pInsertAfter->setText(tr("Insert point mass after"));
		m_pDeleteItem->setText(tr("Delete point Mass"));
	}
	else if(m_enumActiveObject!=NOOBJECT)
	{
		m_pInsertBefore->setText("Insert object before");
		m_pInsertAfter->setText("Insert object after");
		m_pDeleteItem->setText("Remove object");
	}

	if(m_pStruct->geometry().contains(event->pos())) m_pContextMenu->exec(event->globalPos());
}


void EditPlaneDlg::setupLayout()
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

	m_pStruct->setEditTriggers(QAbstractItemView::AllEditTriggers);
	m_pStruct->setSelectionBehavior (QAbstractItemView::SelectRows);
//	m_pStruct->setIndentation(31);
	m_pStruct->setWindowTitle(tr("Objects"));

	m_pModel = new QStandardItemModel(this);
	m_pModel->setColumnCount(4);
	m_pModel->clear();
	m_pModel->setHorizontalHeaderLabels(labels);
	m_pStruct->setModel(m_pModel);

	QFont fnt;
	QFontMetrics fm(fnt);
	m_pStruct->setColumnWidth(0, fm.averageCharWidth()*37);
	m_pStruct->setColumnWidth(1, fm.averageCharWidth()*29);
	m_pStruct->setColumnWidth(2, fm.averageCharWidth()*17);

	QItemSelectionModel *selectionModel = new QItemSelectionModel(m_pModel);
	m_pStruct->setSelectionModel(selectionModel);
	connect(selectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));

	m_pDelegate = new EditObjectDelegate(this);
	m_pStruct->setItemDelegate(m_pDelegate);
	connect(m_pDelegate,  SIGNAL(closeEditor(QWidget *)), this, SLOT(onEndEdit()));

	QSizePolicy szPolicyMinimumExpanding;
	szPolicyMinimumExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
	szPolicyMinimumExpanding.setVerticalPolicy(QSizePolicy::MinimumExpanding);

	QSizePolicy szPolicyMinimum;
	szPolicyMinimum.setHorizontalPolicy(QSizePolicy::Minimum);
	szPolicyMinimum.setVerticalPolicy(QSizePolicy::Minimum);

	QSizePolicy szPolicyMaximum;
	szPolicyMaximum.setHorizontalPolicy(QSizePolicy::Maximum);
	szPolicyMaximum.setVerticalPolicy(QSizePolicy::Maximum);

	QSizePolicy szPolicyPreferred;
	szPolicyPreferred.setHorizontalPolicy(QSizePolicy::Preferred);
	szPolicyPreferred.setVerticalPolicy(QSizePolicy::Preferred);

	QSizePolicy szPolicyFixed;
	szPolicyFixed.setHorizontalPolicy(QSizePolicy::Fixed);
	szPolicyFixed.setVerticalPolicy(QSizePolicy::Fixed);

	QSizePolicy szPolicyExpanding;
	szPolicyExpanding.setHorizontalPolicy(QSizePolicy::Expanding);
	szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);

	QSizePolicy szPolicyIgnored;
	szPolicyIgnored.setHorizontalPolicy(QSizePolicy::Ignored);
	szPolicyIgnored.setVerticalPolicy(QSizePolicy::Ignored);


	m_pHorizontalSplitter = new QSplitter(Qt::Horizontal, this);
	{
		m_pRightSideSplitter = new QSplitter(Qt::Vertical, this);
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
						m_pctrlAxes         = new QCheckBox(tr("Axes"), this);
						m_pctrlSurfaces     = new QCheckBox(tr("Surfaces"), this);
						m_pctrlOutline      = new QCheckBox(tr("Outline"), this);
						m_pctrlPanels       = new QCheckBox(tr("Panels"), this);
						m_pctrlFoilNames    = new QCheckBox(tr("Foil Names"), this);
						m_pctrlShowMasses       = new QCheckBox(tr("Masses"), this);

						m_pctrlAxes->setSizePolicy(szPolicyMaximum);
						m_pctrlSurfaces->setSizePolicy(szPolicyMaximum);
						m_pctrlOutline->setSizePolicy(szPolicyMaximum);
						m_pctrlPanels->setSizePolicy(szPolicyMaximum);
						m_pctrlFoilNames->setSizePolicy(szPolicyMaximum);
						m_pctrlShowMasses->setSizePolicy(szPolicyMaximum);

						pThreeDParamsLayout->addWidget(m_pctrlAxes, 1,1);
						pThreeDParamsLayout->addWidget(m_pctrlPanels, 1,2);
						pThreeDParamsLayout->addWidget(m_pctrlSurfaces, 1,3);
						pThreeDParamsLayout->addWidget(m_pctrlOutline, 2,1);
						pThreeDParamsLayout->addWidget(m_pctrlFoilNames, 2,2);
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
							m_pctrlX->setIconSize(QSize(24,24));
							m_pctrlY->setIconSize(QSize(24,24));
							m_pctrlZ->setIconSize(QSize(24,24));
							m_pctrlIso->setIconSize(QSize(24,24));
							m_pctrlFlip->setIconSize(QSize(24,24));
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
					QVBoxLayout *pRightColLayout = new QVBoxLayout;
					{
						m_pctrlReset = new QPushButton(tr("Reset scale"));
						pRightColLayout->addWidget(m_pctrlReset);
						QHBoxLayout *pClipLayout = new QHBoxLayout;
						{
							QLabel *ClipLabel = new QLabel(tr("Clip:"));
							m_pctrlClipPlanePos = new QSlider(Qt::Horizontal);
							m_pctrlClipPlanePos->setMinimum(-300);
							m_pctrlClipPlanePos->setMaximum(300);
							m_pctrlClipPlanePos->setSliderPosition(0);
							m_pctrlClipPlanePos->setTickInterval(30);
							m_pctrlClipPlanePos->setTickPosition(QSlider::TicksBelow);
							pClipLayout->addWidget(ClipLabel);
							pClipLayout->addWidget(m_pctrlClipPlanePos,1);
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
			m_pglPlaneView->sizePolicy().setVerticalStretch(5);
			p3DCtrlBox->sizePolicy().setVerticalStretch(1);

			m_pRightSideSplitter->addWidget(m_pglPlaneView);
			m_pRightSideSplitter->addWidget(p3DCtrlBox);
		}

		m_pLeftSideSplitter = new QSplitter(Qt::Vertical, this);
		{
			QWidget *pCommandWidget = new QWidget(this);
			{
				QVBoxLayout *pCommandLayout = new QVBoxLayout;
				{
					QHBoxLayout *pRedrawCommandLayout = new QHBoxLayout;
					{
						m_pctrlAutoRedraw = new QCheckBox(tr("Auto regeneration"));
						m_pctrlRedraw = new QPushButton(tr("Regenerate")+"\t(F4)");
						pRedrawCommandLayout->addWidget(m_pctrlAutoRedraw);
						pRedrawCommandLayout->addWidget(m_pctrlRedraw);
					}
					QHBoxLayout *pExitCommandLayout = new QHBoxLayout;
					{
						pOKButton = new QPushButton(tr("Save and Close"));
						pOKButton->setAutoDefault(true);
						QPushButton *pCancelButton = new QPushButton(tr("Cancel"));
						pCancelButton->setAutoDefault(false);
						pExitCommandLayout->addWidget(pOKButton);
						pExitCommandLayout->addWidget(pCancelButton);
						connect(pOKButton, SIGNAL(clicked()),this, SLOT(onOK()));
						connect(pCancelButton, SIGNAL(clicked()),this, SLOT(reject()));
					}
					pCommandLayout->addLayout(pRedrawCommandLayout);
					pCommandLayout->addLayout(pExitCommandLayout);
				}
				pCommandWidget->setLayout(pCommandLayout);
			}

			m_pStruct->sizePolicy().setVerticalStretch(17);
			pCommandWidget->sizePolicy().setVerticalStretch(1);

			m_pLeftSideSplitter->addWidget(m_pStruct);
			m_pLeftSideSplitter->addWidget(pCommandWidget);
		}
		m_pHorizontalSplitter->addWidget(m_pLeftSideSplitter);
		m_pHorizontalSplitter->addWidget(m_pRightSideSplitter);
	}

	QHBoxLayout *pMainLayout = new QHBoxLayout;
	{
		pMainLayout->addWidget(m_pHorizontalSplitter);
	}
	setLayout(pMainLayout);
	connectSignals();
//	resize(s_Size);
}


/**
* Creates the VertexBufferObjects for OpenGL 3.0
*/
void EditPlaneDlg::glMake3DObjects()
{
	Body *pCurBody = NULL;
	if(!m_pPlane) return;

	pCurBody = m_pPlane->body();

	if(m_bResetglBody && pCurBody)
	{
		m_pglPlaneView->glMakeBodySplines(pCurBody);
		m_bResetglBody = false;
	}

	if(m_bResetglPlane)
	{
		Body TranslatedBody;
		if(m_pPlane->body())
		{
			TranslatedBody.duplicate(m_pPlane->body());
			TranslatedBody.translate(m_pPlane->bodyPos());
		}

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlane->wing(iw))
			{
				m_pglPlaneView->glMakeWingGeometry(iw, m_pPlane->wing(iw), m_pPlane->body());
			}
		}

		m_bResetglPlane = false;
	}
}


void EditPlaneDlg::onOK()
{
	int j;

	readPlaneTree();

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

	accept();
}



void EditPlaneDlg::initDialog(Plane *pPlane)
{
	m_pPlane = pPlane;
	m_pPlane->createSurfaces();
	fillPlaneTreeView();

	m_pglPlaneView->setPlane(m_pPlane);
	m_pglPlaneView->setScale(qMax(m_pPlane->wing()->planformSpan(), m_pPlane->body() ? m_pPlane->body()->length() : 1.0));

	m_pctrlSurfaces->setChecked(m_pglPlaneView->m_bSurfaces);
	m_pctrlOutline->setChecked(m_pglPlaneView->m_bOutline);
	m_pctrlAxes->setChecked(m_pglPlaneView->m_bAxes);
	m_pctrlPanels->setChecked(m_pglPlaneView->m_bVLMPanels);
	m_pctrlFoilNames->setChecked(m_pglPlaneView->m_bFoilNames);
	m_pctrlShowMasses->setChecked(m_pglPlaneView->m_bShowMasses);
	m_pctrlClipPlanePos->setValue((int)(m_pglPlaneView->m_ClipPlanePos*100.0));

	m_pctrlAutoRedraw->setChecked(s_bAutoRedraw);
	m_pctrlRedraw->setEnabled(!s_bAutoRedraw);
}




void EditPlaneDlg::keyPressEvent(QKeyEvent *event)
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
			if(!pOKButton->hasFocus()) pOKButton->setFocus();
			else                       accept();

			break;
		}
		case Qt::Key_F4:
		{
			on3DReset();
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



void EditPlaneDlg::accept()
{
	s_bWindowMaximized= isMaximized();
	s_WindowPosition = pos();
	s_WindowSize = size();

	s_bOutline    = m_pglPlaneView->m_bOutline;
	s_bSurfaces   = m_pglPlaneView->m_bSurfaces;
	s_bVLMPanels  = m_pglPlaneView->m_bVLMPanels;
	s_bAxes       = m_pglPlaneView->m_bAxes;
	s_bShowMasses = m_pglPlaneView->m_bShowMasses;
	s_bFoilNames  = m_pglPlaneView->m_bFoilNames;

	done(QDialog::Accepted);
}


void EditPlaneDlg::reject()
{
	s_bWindowMaximized= isMaximized();
	s_WindowPosition = pos();
	s_WindowSize = size();

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

	s_bOutline    = m_pglPlaneView->m_bOutline;
	s_bSurfaces   = m_pglPlaneView->m_bSurfaces;
	s_bVLMPanels  = m_pglPlaneView->m_bVLMPanels;
	s_bAxes       = m_pglPlaneView->m_bAxes;
	s_bShowMasses = m_pglPlaneView->m_bShowMasses;
	s_bFoilNames  = m_pglPlaneView->m_bFoilNames;

	done(QDialog::Rejected);
}


void EditPlaneDlg::glCreateWingSectionHighlight(Wing *pWing)
{
	if(!pWing) return;

	int iSection = 0;
	int jSurf = 0;
	for(int jSection=0; jSection<pWing->NWingSection(); jSection++)
	{
		if(jSection==m_iActiveSection) break;
		if(qAbs(pWing->YPosition(jSection+1)-pWing->YPosition(jSection)) > Wing::s_MinPanelSize)
			iSection++;
	}

	glNewList(SECTIONHIGHLIGHT,GL_COMPILE);
	{
		glDisable(GL_DEPTH_TEST);
		glDisable (GL_LINE_STIPPLE);
		glColor3d(1.0, 0.0, 0.0);
		glLineWidth(3);

		if(iSection==0)
		{
			//define the inner left side surface
			if(!pWing->isFin())  jSurf = pWing->m_Surface.size()/2 - 1;
			else                 jSurf = pWing->m_Surface.size()   - 1;

			//plot B side outline
			glBegin(GL_LINE_STRIP);
			{
				for (int lx=0; lx<pWing->m_Surface.at(jSurf)->NXPanels(); lx++)
				{
					pWing->m_Surface.at(jSurf)->getPanel(pWing->m_Surface.at(jSurf)->NYPanels()-1, lx, TOPSURFACE);
					glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
							   pWing->m_Surface.at(jSurf)->TB.y,
							   pWing->m_Surface.at(jSurf)->TB.z);
				}

				glVertex3d(pWing->m_Surface.at(jSurf)->LB.x,
						   pWing->m_Surface.at(jSurf)->LB.y,
						   pWing->m_Surface.at(jSurf)->LB.z);

				for (int lx=pWing->m_Surface.at(jSurf)->NXPanels()-1; lx>=0; lx--)
				{
					pWing->m_Surface.at(jSurf)->getPanel(pWing->m_Surface.at(jSurf)->NYPanels()-1, lx, BOTSURFACE);
					glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
							   pWing->m_Surface.at(jSurf)->TB.y,
							   pWing->m_Surface.at(jSurf)->TB.z);
				}
			}
			glEnd();
		}
		else
		{
			if((pWing->m_bSymetric) && !pWing->m_bIsFin)
			{
				jSurf = pWing->m_Surface.size()/2 + iSection -1;

				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<pWing->m_Surface.at(jSurf)->NXPanels(); lx++)
					{
						pWing->m_Surface.at(jSurf)->getPanel(pWing->m_Surface.at(jSurf)->NYPanels()-1, lx, TOPSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
								 pWing->m_Surface.at(jSurf)->TB.y,
								 pWing->m_Surface.at(jSurf)->TB.z);
					}

					glVertex3d(pWing->m_Surface.at(jSurf)->LB.x,
							 pWing->m_Surface.at(jSurf)->LB.y,
							 pWing->m_Surface.at(jSurf)->LB.z);

					for (int lx=pWing->m_Surface.at(jSurf)->NXPanels()-1; lx>=0; lx--)
					{
						pWing->m_Surface.at(jSurf)->getPanel(pWing->m_Surface.at(jSurf)->NYPanels()-1, lx, BOTSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TB.x,
								 pWing->m_Surface.at(jSurf)->TB.y,
								 pWing->m_Surface.at(jSurf)->TB.z);
					}
				}
				glEnd();
			}

			if(pWing->m_bSymetric)
			{
				if(!pWing->m_bIsFin) jSurf = pWing->m_Surface.size()/2 - iSection;
				else                 jSurf = pWing->m_Surface.size()   - iSection;

				//plot A side outline
				glBegin(GL_LINE_STRIP);
				{
					for (int lx=0; lx<pWing->m_Surface.at(jSurf)->NXPanels(); lx++)
					{
						pWing->m_Surface.at(jSurf)->getPanel(0, lx, TOPSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TA.x,
								   pWing->m_Surface.at(jSurf)->TA.y,
								   pWing->m_Surface.at(jSurf)->TA.z);
					}

					glVertex3d(pWing->m_Surface.at(jSurf)->LA.x,
							   pWing->m_Surface.at(jSurf)->LA.y,
							   pWing->m_Surface.at(jSurf)->LA.z);

					for (int lx=pWing->m_Surface.at(jSurf)->NXPanels()-1; lx>=0; lx--)
					{
						pWing->m_Surface.at(jSurf)->getPanel(0, lx, BOTSURFACE);
						glVertex3d(pWing->m_Surface.at(jSurf)->TA.x,
								   pWing->m_Surface.at(jSurf)->TA.y,
								   pWing->m_Surface.at(jSurf)->TA.z);
					}
				}
				glEnd();
			}
		}
	}
	glEndList();
}



void EditPlaneDlg::connectSignals()
{
	connect(m_pInsertBefore,   SIGNAL(triggered()), this, SLOT(onInsertBefore()));
	connect(m_pInsertAfter,    SIGNAL(triggered()), this, SLOT(onInsertAfter()));
	connect(m_pDeleteItem,     SIGNAL(triggered()), this, SLOT(onDelete()));

	connect(m_pctrlAutoRedraw, SIGNAL(clicked()), this, SLOT(onAutoRedraw()));
	connect(m_pctrlRedraw,     SIGNAL(clicked()), this, SLOT(onRedraw()));
	connect(m_pctrlReset,      SIGNAL(clicked()), this, SLOT(on3DReset()));

	connect(m_pctrlReset,      SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DReset()));

	connect(m_pctrlAxes,       SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onAxes(bool)));
	connect(m_pctrlPanels,     SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onPanels(bool)));
	connect(m_pctrlSurfaces,   SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onSurfaces(bool)));
	connect(m_pctrlOutline,    SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onOutline(bool)));
	connect(m_pctrlFoilNames,  SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onFoilNames(bool)));
	connect(m_pctrlShowMasses, SIGNAL(clicked(bool)), m_pglPlaneView, SLOT(onShowMasses(bool)));

	connect(m_pctrlIso,        SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DIso()));
	connect(m_pctrlX,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DFront()));
	connect(m_pctrlY,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DLeft()));
	connect(m_pctrlZ,          SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DTop()));
	connect(m_pctrlFlip,       SIGNAL(clicked()), m_pglPlaneView, SLOT(on3DFlip()));

	connect(m_pctrlClipPlanePos, SIGNAL(sliderMoved(int)), m_pglPlaneView, SLOT(onClipPlane(int)));

	connect(m_pHorizontalSplitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onResize()));

	connect(m_pglPlaneView, SIGNAL(viewModified()), this, SLOT(onCheckViewIcons()));
}


/**
 * Unselects all the 3D-view icons.
 */
void EditPlaneDlg::onCheckViewIcons()
{
	m_pctrlIso->setChecked(false);
	m_pctrlX->setChecked(false);
	m_pctrlY->setChecked(false);
	m_pctrlZ->setChecked(false);
}




void EditPlaneDlg::on3DReset()
{
	m_pglPlaneView->setScale(qMax(m_pPlane->wing()->planformSpan(), m_pPlane->body() ? m_pPlane->body()->length() : 1.0));
	m_pglPlaneView->on3DReset();
}



void EditPlaneDlg::onAutoRedraw()
{
	s_bAutoRedraw = m_pctrlAutoRedraw->isChecked();
	m_pctrlRedraw->setEnabled(!s_bAutoRedraw);
}


void EditPlaneDlg::onRedraw()
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	readPlaneTree();

	m_pPlane->createSurfaces();
	m_pPlane->computePlane();


	m_bResetglPlane = true;
	m_bChanged = true;

	m_PixText.fill(Qt::transparent);
	QPainter paint(&m_PixText);
	paintPlaneLegend(paint, m_pPlane, m_pglPlaneView->rect());

	m_pglPlaneView->repaint();
	QApplication::restoreOverrideCursor();

}


void EditPlaneDlg::onEndEdit()
{
	if(s_bAutoRedraw) onRedraw();
}

bool EditPlaneDlg::intersectObject(Vector3d AA,  Vector3d U, Vector3d &I)
{
	Wing *pWingList[MAXWINGS] = {m_pPlane->wing(), m_pPlane->wing2(), m_pPlane->stab(), m_pPlane->fin()};

	for(int iw=0; iw<MAXWINGS; iw++)
	{
		if (pWingList[iw] && pWingList[iw]->intersectWing(AA, U, I)) return true;
	}

	if(m_pPlane->body())
	{
		if(m_pPlane->body()->intersectFlatPanels(AA, AA+U*10, I))return true;
	}
	return false;
}



QList<QStandardItem *> EditPlaneDlg::prepareRow(const QString &object, const QString &field, const QString &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem(object)  << new QStandardItem(field)  << new QStandardItem(value) << new QStandardItem(unit);
	for(int ii=0; ii<rowItems.size(); ii++) rowItems.at(ii)->setData(XFLR5::STRING, Qt::UserRole);
	return rowItems;
}


QList<QStandardItem *> EditPlaneDlg::prepareBoolRow(const QString &object, const QString &field, const bool &value)
{
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(object));
	rowItems.append(new QStandardItem(field));
	rowItems.append(new QStandardItem);
	rowItems.at(2)->setData(value, Qt::DisplayRole);
	rowItems.append(new QStandardItem);

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);

	return rowItems;
}


QList<QStandardItem *> EditPlaneDlg::prepareIntRow(const QString &object, const QString &field, const int &value)
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


QList<QStandardItem *> EditPlaneDlg::prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit)
{
	QList<QStandardItem *> rowItems;
	rowItems.append(new QStandardItem(object));
	rowItems.append(new QStandardItem(field));
	rowItems.append(new QStandardItem);
	rowItems.at(2)->setData(value, Qt::DisplayRole);
	rowItems.append(new QStandardItem(unit));

	rowItems.at(0)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(1)->setData(XFLR5::STRING, Qt::UserRole);
	rowItems.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
	rowItems.at(3)->setData(XFLR5::STRING, Qt::UserRole);

	return rowItems;
}


void EditPlaneDlg::fillPlaneTreeView()
{
	m_pModel->removeRows(0, m_pModel->rowCount());

	QStandardItem *rootItem = m_pModel->invisibleRootItem();
	rootItem->setText(m_pPlane->planeName());

	QList<QStandardItem*> planeRootItem = prepareRow("Plane", "Name", m_pPlane->planeName());
	rootItem->appendRow(planeRootItem);

	QModelIndex ind = m_pModel->index(0,0);
	m_pStruct->expand(ind);

	fillPlaneMetaData(planeRootItem.first());

	if(m_pPlane->body()) fillBodyTreeView(planeRootItem.first());

	QList<QStandardItem*> wingFolder = prepareRow("Wings");
	planeRootItem.first()->appendRow(wingFolder);
	{
		QModelIndex ind = m_pModel->indexFromItem(wingFolder.first());
		m_pStruct->expand(ind);

		for(int iw=0; iw<MAXWINGS; iw++)
		{
			if(m_pPlane->wing(iw))
			{
				fillWingTreeView(iw, wingFolder);
			}
		}
	}
}



void EditPlaneDlg::fillPlaneMetaData(QStandardItem *item)
{
	QList<QStandardItem*> descriptionDataItem = prepareRow("Description", "Description", m_pPlane->planeDescription());
	item->appendRow(descriptionDataItem);

	QList<QStandardItem*>dataItem = prepareBoolRow("", "hasBody", m_pPlane->hasBody());
	item->appendRow(dataItem);
	dataItem = prepareBoolRow("", "hasElevator", m_pPlane->hasElevator());
	item->appendRow(dataItem);
	dataItem = prepareBoolRow("", "hasSecondWing", m_pPlane->hasSecondWing());
	item->appendRow(dataItem);
	dataItem = prepareBoolRow("", "hasFin", m_pPlane->hasFin());
	item->appendRow(dataItem);


	if(m_pPlane->m_PointMass.size())
	{
		QList<QStandardItem*> PlaneInertiaFolder = prepareRow("Inertia");
		item->appendRow(PlaneInertiaFolder);
		{
			if(m_enumActiveObject==PLANE && m_iActivePointMass>=0)
			{
				m_pStruct->expand(m_pModel->indexFromItem(PlaneInertiaFolder.first()));
			}
			for(int iwm=0; iwm<m_pPlane->m_PointMass.size(); iwm++)
			{
				PointMass *pm = m_pPlane->m_PointMass.at(iwm);
				QList<QStandardItem*> planePointMassFolder = prepareRow(QString("Point_mass_%1").arg(iwm+1));

				PlaneInertiaFolder.first()->appendRow(planePointMassFolder);
				{
					if(m_enumActiveObject==PLANE && m_iActivePointMass==iwm)
					{
						m_pStruct->expand(m_pModel->indexFromItem(planePointMassFolder.first()));
					}
					QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
					dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareRow("", "mass", QString("%1").arg(pm->mass()*Units::kgtoUnit()), Units::weightUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);

					dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
					dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
					planePointMassFolder.first()->appendRow(dataItem);
				}
			}
		}
	}
}



void EditPlaneDlg::fillWingTreeView(int iw, QList<QStandardItem*> &planeRootItem)
{
	Wing *pWing = m_pPlane->wing(iw);

	QList<QStandardItem*> wingFolder = prepareRow("Wing", "Type", wingType(pWing->wingType()));
	wingFolder.at(2)->setData(XFLR5::WINGTYPE, Qt::UserRole);
	planeRootItem.first()->appendRow(wingFolder);

	if(m_pPlane->wing(m_enumActiveWingType)==pWing)
	{
		m_pStruct->expand(m_pModel->indexFromItem(wingFolder.first()));
	}

	QList<QStandardItem*> dataItem = prepareRow("Name", "Name", pWing->wingName());
	wingFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("Symetric", "Symetric", pWing->isSymetric() ? "true": "false");
	dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
	wingFolder.first()->appendRow(dataItem);

	dataItem = prepareDoubleRow("Pitch angle", "Angle", m_pPlane->WingTiltAngle(iw),QString::fromUtf8("°"));
	dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
	wingFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> wingColorFolder = prepareRow("Color");
	wingFolder.first()->appendRow(wingColorFolder);
	{
		dataItem = prepareIntRow("", "red", pWing->wingColor().red());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "green", pWing->wingColor().green());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "blue", pWing->wingColor().blue());
		wingColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "alpha", pWing->wingColor().alpha());
		wingColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> finDataFolder = prepareRow("Fin data");
	wingFolder.first()->appendRow(finDataFolder);
	{
		QList<QStandardItem*> dataItem = prepareRow("", "is Fin:", pWing->isFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Symetric Fin:", pWing->isSymFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);

		dataItem = prepareRow("", "is Double Fin:", pWing->isDoubleFin() ? "true": "false");
		dataItem.at(2)->setData(XFLR5::BOOL, Qt::UserRole);
		finDataFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingPositionFolder = prepareRow("Position");
	wingFolder.first()->appendRow(wingPositionFolder);
	{
		dataItem = prepareDoubleRow("", "x", m_pPlane->WingLE(iw).x*Units::mtoUnit(), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "y", m_pPlane->WingLE(iw).y*Units::mtoUnit(), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "z", m_pPlane->WingLE(iw).z*Units::mtoUnit(), Units::lengthUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> wingInertiaFolder = prepareRow("Inertia");
	wingFolder.first()->appendRow(wingInertiaFolder);
	{
		if(m_enumActiveObject==WING && m_enumActiveWingType==pWing->wingType() && m_iActivePointMass>=0)
		{
			m_pStruct->expand(m_pModel->indexFromItem(wingInertiaFolder.first()));
		}
		QList<QStandardItem*> dataItem = prepareDoubleRow( "", "Volume mass", pWing->volumeMass()*Units::kgtoUnit(), Units::weightUnitLabel());
		dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
		wingInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pWing->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pWing->m_PointMass.at(iwm);
			QList<QStandardItem*> wingPointMassFolder = prepareRow(QString("Point_mass_%1").arg(iwm+1));

			wingInertiaFolder.first()->appendRow(wingPointMassFolder);
			{
				if(m_enumActiveObject==WING && m_enumActiveWingType==pWing->wingType() && m_iActivePointMass==iwm)
				{
					m_pStruct->expand(m_pModel->indexFromItem(wingPointMassFolder.first()));
				}

				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
				dataItem.at(2)->setData(XFLR5::STRING, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "mass", pm->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "x", pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
				dataItem.at(2)->setData(XFLR5::DOUBLE, Qt::UserRole);
				wingPointMassFolder.first()->appendRow(dataItem);
			}
		}
	}

	QList<QStandardItem*> wingSectionFolder = prepareRow("Sections");
	wingFolder.first()->appendRow(wingSectionFolder);
	{
		if(m_pPlane->wing(m_enumActiveWingType)==pWing)
		{
			m_pStruct->expand(m_pModel->indexFromItem(wingSectionFolder.first()));
		}
		for(int iws=0; iws<pWing->m_WingSection.size(); iws++)
		{
			WingSection *wingsec = pWing->m_WingSection.at(iws);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Section_%1").arg(iws+1));
			wingSectionFolder.first()->appendRow(sectionFolder);
			{
				dataItem = prepareDoubleRow("", "span position", wingsec->m_YPosition*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "chord", wingsec->m_Chord*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "offset", wingsec->m_Offset*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "dihedral", wingsec->m_Dihedral, QString::fromUtf8("°"));
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "twist", wingsec->m_Twist, QString::fromUtf8("°"));
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareIntRow("", "x-panels", wingsec->m_NXPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "x-distribution", distributionType(wingsec->m_XPanelDist));
				dataItem.at(2)->setData(XFLR5::PANELDISTRIBUTION, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareIntRow("", "y-panels",wingsec->m_NYPanels);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "y-distribution", distributionType(wingsec->m_YPanelDist));
				sectionFolder.first()->appendRow(dataItem);
				dataItem.at(2)->setData(XFLR5::PANELDISTRIBUTION, Qt::UserRole);

				dataItem = prepareRow("", "Left side foil name", wingsec->m_LeftFoilName.length() ? wingsec->m_LeftFoilName : "No left foil defined");
				dataItem.at(2)->setData(XFLR5::FOILNAME, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);

				dataItem = prepareRow("", "Right side foil name", wingsec->m_LeftFoilName.length() ? wingsec->m_RightFoilName : "No left foil defined");
				dataItem.at(2)->setData(XFLR5::FOILNAME, Qt::UserRole);
				sectionFolder.first()->appendRow(dataItem);
			}
		}
	}

}



void EditPlaneDlg::fillBodyTreeView(QStandardItem*planeRootItem)
{
	Body *pBody = m_pPlane->body();

	QList<QStandardItem*> bodyFolder = prepareRow("Body");
	planeRootItem->appendRow(bodyFolder);

	if(m_enumActiveObject==BODY || m_iActiveFrame>=0)
	{
		m_pStruct->expand(m_pModel->indexFromItem(bodyFolder.first()));
	}

	QList<QStandardItem*> dataItem;


	dataItem = prepareRow("", "Name", m_pPlane->body()->bodyName());
	bodyFolder.first()->appendRow(dataItem);

	dataItem = prepareRow("", "Type", bodyPanelType(pBody->bodyType()));
	dataItem.at(2)->setData(XFLR5::BODYTYPE, Qt::UserRole);
	bodyFolder.first()->appendRow(dataItem);

	QList<QStandardItem*> bodyColorFolder = prepareRow("Color");
	bodyFolder.first()->appendRow(bodyColorFolder);
	{
		QList<QStandardItem*> dataItem = prepareIntRow("", "red", pBody->bodyColor().red());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "green", pBody->bodyColor().green());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "blue", pBody->bodyColor().blue());
		bodyColorFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "alpha", pBody->bodyColor().alpha());
		bodyColorFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyPositionFolder = prepareRow("Position");
	bodyFolder.first()->appendRow(bodyPositionFolder);
	{
		QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", m_pPlane->bodyPos().x*Units::mtoUnit(), Units::lengthUnitLabel());
		bodyPositionFolder.first()->appendRow(dataItem);

		dataItem = prepareDoubleRow("", "z", m_pPlane->bodyPos().z*Units::mtoUnit(), Units::lengthUnitLabel());
		bodyPositionFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> bodyInertiaFolder = prepareRow("Inertia");
	bodyFolder.first()->appendRow(bodyInertiaFolder);
	{
		if(m_enumActiveObject==BODY && m_iActivePointMass>=0)
		{
			m_pStruct->expand(m_pModel->indexFromItem(bodyInertiaFolder.first()));
		}
		QList<QStandardItem*> dataItem = prepareDoubleRow( "Structural (volume) mass", "", pBody->m_VolumeMass*Units::kgtoUnit(), Units::weightUnitLabel());
		bodyInertiaFolder.first()->appendRow(dataItem);

		for(int iwm=0; iwm<pBody->m_PointMass.size(); iwm++)
		{
			PointMass *pm = pBody->m_PointMass.at(iwm);
			QList<QStandardItem*> bodyPointMassFolder = prepareRow(QString("Point_mass_%1").arg(iwm+1));

			bodyInertiaFolder.first()->appendRow(bodyPointMassFolder);
			{
				if(m_enumActiveObject==BODY && m_iActivePointMass==iwm)
				{
					m_pStruct->expand(m_pModel->indexFromItem(bodyPointMassFolder.first()));
				}
				QList<QStandardItem*> dataItem = prepareRow("", "Tag", pm->tag());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "mass", pm->mass()*Units::kgtoUnit(), Units::weightUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "x",pm->position().x*Units::mtoUnit(), Units::lengthUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "y", pm->position().y*Units::mtoUnit(), Units::lengthUnitLabel());;
				bodyPointMassFolder.first()->appendRow(dataItem);

				dataItem = prepareDoubleRow("", "z", pm->position().z*Units::mtoUnit(), Units::lengthUnitLabel());
				bodyPointMassFolder.first()->appendRow(dataItem);
			}
		}
	}

	QList<QStandardItem*> NURBSFolder = prepareRow("NURBS");
	bodyFolder.first()->appendRow(NURBSFolder);
	{
		QList<QStandardItem*> dataItem = prepareIntRow("", "NURBS degree (lengthwise)", pBody->splineSurface()->uDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "NURBS degree (hoop)", pBody->splineSurface()->vDegree());
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (lengthwise)", pBody->m_nxPanels);
		NURBSFolder.first()->appendRow(dataItem);

		dataItem = prepareIntRow("", "Mesh panels (hoop)", pBody->m_nhPanels);
		NURBSFolder.first()->appendRow(dataItem);
	}

	QList<QStandardItem*> hoopFolder = prepareRow("Hoop_panels (FLATPANELS case)");
	bodyFolder.first()->appendRow(hoopFolder);
	{
		for(int isl=0; isl<pBody->sideLineCount(); isl++)
		{
			QList<QStandardItem*> dataItem = prepareIntRow("", QString("Hoop panels in stripe %1").arg(isl+1), pBody->m_hPanels.at(isl));
			hoopFolder.first()->appendRow(dataItem);
		}
	}

	QList<QStandardItem*> bodyFrameFolder = prepareRow("Frames");
	bodyFolder.first()->appendRow(bodyFrameFolder);
	{
		if(m_iActiveFrame>=0)
		{
			m_pStruct->expand(m_pModel->indexFromItem(bodyFrameFolder.first()));
		}
		for(int iFrame=0; iFrame<pBody->splineSurface()->frameCount(); iFrame++)
		{
			Frame *pFrame = pBody->splineSurface()->frameAt(iFrame);

			QList<QStandardItem*> sectionFolder = prepareRow(QString("Frame_%1").arg(iFrame+1));
			bodyFrameFolder.first()->appendRow(sectionFolder);
			{
				dataItem = prepareIntRow("", "Lengthwise panels (FLATPANELS case)", pBody->m_xPanels.at(iFrame));
				sectionFolder.first()->appendRow(dataItem);

				QList<QStandardItem*> dataItem = prepareDoubleRow("x_Position", "x", pFrame->m_Position.x*Units::mtoUnit(), Units::lengthUnitLabel());
				sectionFolder.first()->appendRow(dataItem);

				for(int iPt=0; iPt<pFrame->pointCount(); iPt++)
				{
					QList<QStandardItem*> pointFolder = prepareRow(QString("Point %1").arg(iPt+1));
					sectionFolder.first()->appendRow(pointFolder);
					{
						Vector3d Pt(pFrame->point(iPt));
						QList<QStandardItem*> dataItem = prepareDoubleRow("", "x", Pt.x*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareDoubleRow("", "y", Pt.y*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);

						dataItem = prepareDoubleRow("", "z", Pt.z*Units::mtoUnit(), Units::lengthUnitLabel());
						pointFolder.first()->appendRow(dataItem);
					}
				}
			}
		}
	}
}



void EditPlaneDlg::readPlaneTree()
{
	readViewLevel(m_pModel->index(0,0));
}



void EditPlaneDlg::readViewLevel(QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex, subIndex;

	int iWing = 0;
	int iw=0;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();
		dataIndex = subIndex.sibling(subIndex.row(),2);

		if(indexLevel.child(0,0).isValid())
		{

			if(object.compare("Body", Qt::CaseInsensitive)==0)
				readBodyTree(m_pPlane->body(), indexLevel.child(0,0));
			else if(object.compare("Wing", Qt::CaseInsensitive)==0)
			{
				Wing newWing;
				newWing.clearPointMasses();
				newWing.clearWingSections();
				newWing.clearSurfaces();
				newWing.wingType() = wingType(value);
				Vector3d wingPos;
				double wingTiltAngle;
				readWingTree(&newWing, wingPos, wingTiltAngle, indexLevel.child(0,0));

/*				if(newWing.isFin()) iWing = 3;
				else if(iw==0)      iWing = 0;
				else if(iw==1)      iWing = 2;*/

				switch(newWing.wingType())
				{
					case XFLR5::MAINWING:
					{
						iWing=0;
						break;
					}
					case XFLR5::SECONDWING:
					{
						iWing=1;
						break;
					}
					case XFLR5::ELEVATOR:
					{
						iWing=2;
						break;
					}
					case XFLR5::FIN:
					{
						iWing=3;
						break;
					}
					default:
					{
						iWing=0;
						break;
					}
				}

				m_pPlane->m_Wing[iWing].duplicate(&newWing);
				m_pPlane->WingLE(iWing)        = wingPos;
				m_pPlane->WingTiltAngle(iWing) = wingTiltAngle;
				iw++;
			}
			else if(object.compare("inertia", Qt::CaseInsensitive)==0)
			{
				double volumeMass;
				readInertiaTree(volumeMass, m_pPlane->m_PointMass, indexLevel.child(0,0));
			}

			else readViewLevel(indexLevel.child(0,0));
		}
		else if(field.compare("hasBody", Qt::CaseInsensitive)==0)        m_pPlane->hasBody()       = stringToBool(value);
		else if(field.compare("hasSecondWing", Qt::CaseInsensitive)==0)  m_pPlane->hasSecondWing() = stringToBool(value);
		else if(field.compare("hasElevator", Qt::CaseInsensitive)==0)    m_pPlane->hasElevator()   = stringToBool(value);
		else if(field.compare("hasFin", Qt::CaseInsensitive)==0)         m_pPlane->hasFin()        = stringToBool(value);
		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());
}


void EditPlaneDlg::readWingTree(Wing *pWing, Vector3d &wingLE, double &tiltAngle, QModelIndex indexLevel)
{
	QString object, field, value;

	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();

			if(object.compare("Position", Qt::CaseInsensitive)==0)      readVectorTree(wingLE, indexLevel.child(0,0));
			else if(object.compare("Color", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					QModelIndex dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("red", Qt::CaseInsensitive)==0)         pWing->wingColor().setRed(dataIndex.data().toInt());
					else if(field.compare("green", Qt::CaseInsensitive)==0)  pWing->wingColor().setGreen(dataIndex.data().toInt());
					else if(field.compare("blue", Qt::CaseInsensitive)==0)   pWing->wingColor().setBlue(dataIndex.data().toInt());
					else if(field.compare("alpha", Qt::CaseInsensitive)==0)  pWing->wingColor().setAlpha(dataIndex.data().toInt());

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Fin data", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					if(field.compare("is Fin:", Qt::CaseInsensitive)==0)               pWing->isFin() = stringToBool(value);
					else if(field.compare("is Symetric Fin:", Qt::CaseInsensitive)==0) pWing->isSymFin() = stringToBool(value);
					else if(field.compare("is Double Fin:", Qt::CaseInsensitive)==0)   pWing->isDoubleFin() = stringToBool(value);

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Inertia", Qt::CaseInsensitive)==0)  readInertiaTree(pWing->volumeMass(), pWing->m_PointMass, indexLevel.child(0,0));
			else if(object.compare("Sections", Qt::CaseInsensitive)==0)
			{
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					readWingSectionTree(pWing, subIndex.child(0,0));
					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
		}
		else
		{
			//no more children
			QString object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			QString field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			QString value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			if     (field.compare("Name", Qt::CaseInsensitive)==0)     pWing->rWingName() = value;
			else if(field.compare("Angle", Qt::CaseInsensitive)==0)    tiltAngle = value.toDouble();
			else if(field.compare("Symetric", Qt::CaseInsensitive)==0) pWing->isSymetric() = stringToBool(value);
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}



void EditPlaneDlg::readBodyTree(Body *pBody, QModelIndex indexLevel)
{
	if(!pBody) return;

	QString object, field, value;
	QModelIndex dataIndex, subIndex;

	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			if(object.compare("Position", Qt::CaseInsensitive)==0) readVectorTree(m_pPlane->bodyPos(), indexLevel.child(0,0));
			else if(object.compare("Color", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("red", Qt::CaseInsensitive)==0)         pBody->bodyColor().setRed(dataIndex.data().toInt());
					else if(field.compare("green", Qt::CaseInsensitive)==0)  pBody->bodyColor().setGreen(dataIndex.data().toInt());
					else if(field.compare("blue", Qt::CaseInsensitive)==0)   pBody->bodyColor().setBlue(dataIndex.data().toInt());
					else if(field.compare("alpha", Qt::CaseInsensitive)==0)  pBody->bodyColor().setAlpha(dataIndex.data().toInt());

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}while(subIndex.isValid());
			}
			else if(object.compare("Inertia", Qt::CaseInsensitive)==0) 	readInertiaTree(pBody->volumeMass(), pBody->m_PointMass, indexLevel.child(0,0));
			else if(object.compare("NURBS", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();

					dataIndex = subIndex.sibling(subIndex.row(),2);

					if(field.compare("NURBS degree (lengthwise)", Qt::CaseInsensitive)==0)     pBody->splineSurface()->setuDegree(dataIndex.data().toInt());
					else if(field.compare("NURBS degree (hoop)", Qt::CaseInsensitive)==0)      pBody->splineSurface()->setvDegree(dataIndex.data().toInt());
					else if(field.compare("Mesh panels (lengthwise)", Qt::CaseInsensitive)==0) pBody->m_nxPanels = dataIndex.data().toInt();
					else if(field.compare("Mesh panels (hoop)", Qt::CaseInsensitive)==0)       pBody->m_nhPanels = dataIndex.data().toInt();

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
			else if(object.compare("Hoop_panels (FLATPANELS case)", Qt::CaseInsensitive)==0)
			{
				subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					field = subIndex.sibling(subIndex.row(),1).data().toString();
					value = subIndex.sibling(subIndex.row(),2).data().toString();
					dataIndex = subIndex.sibling(subIndex.row(),2);

					int idx = field.right(field.length()-22).toInt()-1;
					pBody->m_hPanels[idx] =  dataIndex.data().toInt();

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
			else if(object.compare("Frames", Qt::CaseInsensitive)==0)
			{
				pBody->m_SplineSurface.clearFrames();
				QModelIndex subIndex = indexLevel.child(0,0);
				do
				{
					object = subIndex.sibling(subIndex.row(),0).data().toString();
					if(object.indexOf("Frame_")>=0)
					{
						Frame *pFrame = new Frame;
						readBodyFrameTree(pBody, pFrame, subIndex.child(0,0));
						pBody->m_SplineSurface.appendFrame(pFrame);
					}

					subIndex = subIndex.sibling(subIndex.row()+1,0);
				}
				while(subIndex.isValid());
			}
		}
		else
		{
			//no more children
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();

			dataIndex = indexLevel.sibling(indexLevel.row(),2);

			if     (field.compare("Name", Qt::CaseInsensitive)==0) pBody->bodyName() = value;
			else if(field.compare("Type", Qt::CaseInsensitive)==0) pBody->bodyType() = bodyPanelType(value);
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}


void EditPlaneDlg::readBodyFrameTree(Body *pBody, Frame *pFrame, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	double x=0.0;

	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();

		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("Lengthwise panels (FLATPANELS case)", Qt::CaseInsensitive)==0)   pBody->m_xPanels.append(dataIndex.data().toInt());
		else if (object.compare("x_Position", Qt::CaseInsensitive)==0) x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (object.indexOf("Point", Qt::CaseInsensitive)==0)
		{
			Vector3d Pt;
			readVectorTree(Pt, indexLevel.child(0,0));
			pFrame->appendPoint(Pt);
		}
		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());
	pFrame->setuPosition(x);
}


void EditPlaneDlg::readWingSectionTree(Wing *pWing, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;

	// not expecting any more children
	WingSection *pWS = new WingSection;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();


		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("span position", Qt::CaseInsensitive)==0)             pWS->m_YPosition     = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("chord", Qt::CaseInsensitive)==0)                pWS->m_Chord         = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("offset", Qt::CaseInsensitive)==0)               pWS->m_Offset        = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("dihedral", Qt::CaseInsensitive)==0)             pWS->m_Dihedral      = dataIndex.data().toDouble();
		else if (field.compare("twist", Qt::CaseInsensitive)==0)                pWS->m_Twist         = dataIndex.data().toDouble();
		else if (field.compare("x-panels", Qt::CaseInsensitive)==0)             pWS->m_NXPanels      = dataIndex.data().toInt();
		else if (field.compare("y-panels", Qt::CaseInsensitive)==0)             pWS->m_NYPanels      = dataIndex.data().toInt();
		else if (field.compare("x-distribution", Qt::CaseInsensitive)==0)       pWS->m_XPanelDist    = distributionType(value);
		else if (field.compare("y-distribution", Qt::CaseInsensitive)==0)       pWS->m_YPanelDist    = distributionType(value);
		else if (field.compare("Left side foil name", Qt::CaseInsensitive)==0)  pWS->m_LeftFoilName  = value;
		else if (field.compare("Right side foil name", Qt::CaseInsensitive)==0) pWS->m_RightFoilName = value;

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());

	pWing->m_WingSection.append(pWS);

}



void EditPlaneDlg::readInertiaTree(double &volumeMass, QList<PointMass*>&pointMasses, QModelIndex indexLevel)
{
	pointMasses.clear();

	QString object, field, value;
	QModelIndex dataIndex;
	do
	{
		if(indexLevel.child(0,0).isValid())
		{
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			if(object.indexOf("Point_mass_", Qt::CaseInsensitive)>=0)
			{
				PointMass *ppm = new PointMass;
				readPointMassTree(ppm, indexLevel.child(0,0));
				pointMasses.append(ppm);
			}
		}
		else
		{
			//no more children
			object = indexLevel.sibling(indexLevel.row(),0).data().toString();
			field = indexLevel.sibling(indexLevel.row(),1).data().toString();
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();
			dataIndex = indexLevel.sibling(indexLevel.row(),2);

			if     (field.compare("Volume Mass", Qt::CaseInsensitive)==0)   volumeMass = dataIndex.data().toDouble()/Units::kgtoUnit();
		}

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}


void EditPlaneDlg::readPointMassTree(PointMass *ppm, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	// not expecting any more children
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();
		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if      (field.compare("mass", Qt::CaseInsensitive)==0) ppm->mass() = dataIndex.data().toDouble()/Units::kgtoUnit();
		else if (field.compare("tag",  Qt::CaseInsensitive)==0) ppm->tag()  = value;
		else if (field.compare("x",    Qt::CaseInsensitive)==0) ppm->position().x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("y",    Qt::CaseInsensitive)==0) ppm->position().y = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("z",    Qt::CaseInsensitive)==0) ppm->position().z = dataIndex.data().toDouble()/Units::mtoUnit();

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);

	} while(indexLevel.isValid());
}



void EditPlaneDlg::readVectorTree(Vector3d &V, QModelIndex indexLevel)
{
	QString object, field, value;
	QModelIndex dataIndex;
	// not expecting any more children
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();
		field = indexLevel.sibling(indexLevel.row(),1).data().toString();
		value = indexLevel.sibling(indexLevel.row(),2).data().toString();
		dataIndex = indexLevel.sibling(indexLevel.row(),2);

		if (field.compare("x", Qt::CaseInsensitive)==0)      V.x = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("y", Qt::CaseInsensitive)==0) V.y = dataIndex.data().toDouble()/Units::mtoUnit();
		else if (field.compare("z", Qt::CaseInsensitive)==0) V.z = dataIndex.data().toDouble()/Units::mtoUnit();

		indexLevel = indexLevel.sibling(indexLevel.row()+1,0);
	} while(indexLevel.isValid());
}



void EditPlaneDlg::onItemClicked(const QModelIndex &index)
{
	identifySelection(index);
	m_pglPlaneView->update();
}




void EditPlaneDlg::identifySelection(const QModelIndex &indexSel)
{
	// we highlight wing sections and body frames
	// so check if the user's selection is one of these
	m_enumActiveObject = NOOBJECT;
	m_enumActiveWingType = XFLR5::OTHERWING;
	m_iActiveFrame     = -1;
	m_iActiveSection   = -1;
	m_iActivePointMass = -1;

	QModelIndex indexLevel = indexSel;
	QString object, value;
	do
	{
		object = indexLevel.sibling(indexLevel.row(),0).data().toString();

		if(object.indexOf("Section_", 0, Qt::CaseInsensitive)>=0)
		{
			m_iActiveSection = object.right(object.length()-8).toInt() -1;

			//get the wing identification
			indexLevel = indexLevel.parent();
			do
			{
				object = indexLevel.sibling(indexLevel.row(),0).data().toString();

				if(object.compare("Wing", Qt::CaseInsensitive)==0)
				{
					m_enumActiveObject = WING;
					m_enumActiveWingType = wingType(indexLevel.sibling(indexLevel.row(),2).data().toString());
				}
				indexLevel = indexLevel.parent();
			} while(indexLevel.isValid());

			m_iActiveFrame = -1;
			m_iActivePointMass = -1;
			m_bResetglSectionHighlight = true;
			return;
		}
		else if(object.indexOf("Frame_", 0, Qt::CaseInsensitive)>=0)
		{
			m_enumActiveObject = BODY;
			m_iActiveFrame = object.right(object.length()-6).toInt() -1;
			m_iActiveSection = -1;
			m_iActivePointMass = -1;
			m_bResetglSectionHighlight = true;
			return;
		}
		else if(object.indexOf("Point_Mass_", 0, Qt::CaseInsensitive)>=0)
		{
			m_iActivePointMass = object.right(object.length()-11).toInt() -1;
			m_iActiveSection = -1;
			m_iActiveFrame   = -1;
			//the parent object may be a wing, a body or the plane itself
			indexLevel = indexLevel.parent();
			do
			{
				object = indexLevel.sibling(indexLevel.row(),0).data().toString();

				if(object.compare("Wing", Qt::CaseInsensitive)==0)
				{
					m_enumActiveObject = WING;
					m_enumActiveWingType = wingType(indexLevel.sibling(indexLevel.row(),2).data().toString());
					return;
				}
				else if(object.compare("Body", Qt::CaseInsensitive)==0)
				{
					m_enumActiveObject = BODY;
					return;
				}
				else if(object.compare("Plane", Qt::CaseInsensitive)==0)
				{
					m_enumActiveObject = PLANE;
					return;
				}
				indexLevel = indexLevel.parent();
			} while(indexLevel.isValid());
			return;
		}
		else if(object.compare("Wing", Qt::CaseInsensitive)==0)
		{
			m_enumActiveObject = WING;
			value = indexLevel.sibling(indexLevel.row(),2).data().toString();
			m_enumActiveWingType = wingType(value);
			return;
		}
		else if(object.compare("Body", Qt::CaseInsensitive)==0)
		{
			m_enumActiveObject = BODY;
			m_enumActiveWingType = XFLR5::OTHERWING;
			return;
		}

		indexLevel = indexLevel.parent();
	} while(indexLevel.isValid());
}



void EditPlaneDlg::onInsertBefore()
{
	Wing *pWing = m_pPlane->wing(m_enumActiveWingType);

	if(pWing && m_iActiveSection>=0 && m_iActiveSection<pWing->NWingSection() && m_enumActiveWingType!=XFLR5::OTHERWING)
	{
		if(m_iActiveSection==0)
		{
			QMessageBox::warning(this, tr("Warning"), tr("No insertion possible before the first section"));
			return;
		}


		int n = m_iActiveSection;

		pWing->insertSection(m_iActiveSection);

		pWing->YPosition(n) = (pWing->YPosition(n+1) + pWing->YPosition(n-1)) /2.0;
		pWing->Chord(n)     = (pWing->Chord(n+1)     + pWing->Chord(n-1))     /2.0;
		pWing->Offset(n)    = (pWing->Offset(n+1)    + pWing->Offset(n-1))    /2.0;
		pWing->Twist(n)     = (pWing->Twist(n+1)     + pWing->Twist(n-1))     /2.0;
		pWing->Dihedral(n)  = (pWing->Dihedral(n+1)  + pWing->Dihedral(n-1))  /2.0;

		pWing->XPanelDist(n) = pWing->XPanelDist(n-1);
		pWing->YPanelDist(n) = pWing->YPanelDist(n-1);

		pWing->rightFoil(n) = pWing->rightFoil(n-1);
		pWing->leftFoil(n)  = pWing->leftFoil(n-1);

		pWing->NXPanels(n)   = pWing->NXPanels(n-1);


		int ny = pWing->NYPanels(n-1);
		pWing->NYPanels(n)   = (int)(ny/2);
		pWing->NYPanels(n-1) = ny-pWing->NYPanels(n);
		if(pWing->NYPanels(n)==0)   pWing->NYPanels(n)++;
		if(pWing->NYPanels(n-1)==0) pWing->NYPanels(n-1)++;


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_pglPlaneView->update();
	}
	else if(m_pPlane->body() && m_iActiveFrame>=0)
	{
		m_pPlane->body()->insertFrameBefore(m_iActiveFrame);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_bResetglBody   = true;
		m_pglPlaneView->update();
	}
	else if(m_enumActiveObject!=NOOBJECT && m_iActivePointMass>=0)
	{
		if(m_enumActiveObject==PLANE)
		{
			m_pPlane->m_PointMass.insert(m_iActivePointMass, new PointMass);
		}
		else if(m_enumActiveObject==WING)
		{
			m_pPlane->wing(m_enumActiveWingType)->m_PointMass.insert(m_iActivePointMass, new PointMass);
		}
		else if(m_enumActiveObject==BODY && m_pPlane->body())
		{
			m_pPlane->body()->m_PointMass.insert(m_iActivePointMass, new PointMass);
		}


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_pglPlaneView->update();

	}
}



void EditPlaneDlg::onInsertAfter()
{
	Wing *pWing = m_pPlane->wing(m_enumActiveWingType);

	if(pWing && m_iActiveSection>=0 && m_iActiveSection<pWing->NWingSection() && m_enumActiveWingType!=XFLR5::OTHERWING)
	{
		int n = m_iActiveSection;

		if(n<0) n=pWing->NWingSection();

		pWing->insertSection(m_iActiveSection+1);

		if(n<pWing->NWingSection()-2)
		{
			pWing->YPosition(n+1)      = (pWing->YPosition(n)      + pWing->YPosition(n+2))     /2.0;
			pWing->Chord(n+1)    = (pWing->Chord(n)    + pWing->Chord(n+2))   /2.0;
			pWing->Offset(n+1)   = (pWing->Offset(n)   + pWing->Offset(n+2))  /2.0;
			pWing->Twist(n+1)    = (pWing->Twist(n)    + pWing->Twist(n+2))   /2.0;
		}
		else
		{
			pWing->YPosition(n+1)     = pWing->YPosition(n)*1.1;
			pWing->Chord(n+1)   = pWing->Chord(n)/1.1;
			pWing->Offset(n+1)  = pWing->Offset(n) + pWing->Chord(n) - pWing->Chord(n) ;
			pWing->Twist(n+1)     = pWing->Twist(n);
		}

		pWing->Dihedral(n+1)  = pWing->Dihedral(n);
		pWing->NXPanels(n+1)   = pWing->NXPanels(n);
		pWing->NYPanels(n+1)   = pWing->NYPanels(n);
		pWing->XPanelDist(n+1) = pWing->XPanelDist(n);
		pWing->YPanelDist(n+1) = pWing->YPanelDist(n);
		pWing->rightFoil(n+1)  = pWing->rightFoil(n);
		pWing->leftFoil(n+1)   = pWing->leftFoil(n);

		int ny = pWing->NYPanels(n);
		pWing->NYPanels(n+1) = qMax(1,(int)(ny/2));
		pWing->NYPanels(n)   = qMax(1,ny-pWing->NYPanels(n+1));

	//	m_pWing->m_bVLMAutoMesh = true;

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();


		m_iActiveSection++;
		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_pglPlaneView->update();
	}
	else if(m_pPlane->body() && m_iActiveFrame>=0)
	{
		m_pPlane->body()->insertFrameAfter(m_iActiveFrame);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_iActiveFrame++;
		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_bResetglBody   = true;
		m_pglPlaneView->update();
	}
	else if(m_enumActiveObject!=NOOBJECT && m_iActivePointMass>=0)
	{
		if(m_enumActiveObject==PLANE)
		{
			m_pPlane->m_PointMass.insert(m_iActivePointMass+1, new PointMass);
			m_iActivePointMass++;
		}
		else if(m_enumActiveObject==WING)
		{
			m_pPlane->wing(m_enumActiveWingType)->m_PointMass.insert(m_iActivePointMass+1, new PointMass);
			m_iActivePointMass++;
		}
		else if(m_enumActiveObject==BODY && m_pPlane->body())
		{
			m_pPlane->body()->m_PointMass.insert(m_iActivePointMass+1, new PointMass);
			m_iActivePointMass++;
		}


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_pglPlaneView->update();

	}
}


void EditPlaneDlg::onDelete()
{
	if(m_iActiveSection>=0 && m_enumActiveWingType!=XFLR5::OTHERWING)
	{
		if(m_iActiveSection==0)
		{
			QMessageBox::warning(this, tr("Warning"),tr("The wing's first section cannot be deleted"));
			return;
		}

		Wing *pWing = m_pPlane->wing(m_enumActiveWingType);

		if(pWing->NWingSection()<=2)
		{
			QMessageBox::warning(this, tr("Warning"),tr("The wing cannot have less than two sections"));
			return;
		}

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());

		int ny = pWing->NYPanels(m_iActiveSection-1) + pWing->NYPanels(m_iActiveSection);
		pWing->removeWingSection(m_iActiveSection);
		pWing->NYPanels(m_iActiveSection-1) = ny;

		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_pglPlaneView->update();
	}
	else if(m_pPlane->body() && m_iActiveFrame>=0)
	{
		m_pPlane->body()->removeFrame(m_iActiveFrame);

		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_bResetglPlane = true;
		m_bResetglBody   = true;
		m_pglPlaneView->update();
	}
	else if(m_enumActiveObject!=NOOBJECT && m_iActivePointMass>=0)
	{
		if(m_enumActiveObject==PLANE)
		{
			m_pPlane->m_PointMass.removeAt(m_iActivePointMass);
		}
		else if(m_enumActiveObject==WING)
		{
			m_pPlane->wing(m_enumActiveWingType)->m_PointMass.removeAt(m_iActivePointMass);
		}
		else if(m_enumActiveObject==BODY && m_pPlane->body())
		{
			m_pPlane->body()->m_PointMass.removeAt(m_iActivePointMass);
		}


		m_pStruct->closePersistentEditor(m_pStruct->currentIndex());
		fillPlaneTreeView();
		m_pPlane->createSurfaces();

		m_bChanged = true;
		m_bResetglSectionHighlight = true;
		m_pglPlaneView->update();
	}
}




/**
 * Draws the wing legend in the 2D operating point view
 * @param painter a reference to the QPainter object on which the view shall be drawn
 */
void EditPlaneDlg::paintPlaneLegend(QPainter &painter, Plane *pPlane, QRect drawRect)
{
	if(!pPlane) return;
	painter.save();

	QString Result, str, strong;
	QString str1;
	static double Mass;
	static int margin,dheight;

	QPen textPen(Settings::s_TextColor);
	painter.setPen(textPen);
	painter.setFont(Settings::s_TextFont);
	painter.setRenderHint(QPainter::Antialiasing);

	margin = 10;

	QFontMetrics fm(Settings::s_TextFont);
	dheight = fm.height();
	int D = 0;
	int LeftPos = margin;
	int ZPos    = drawRect.height()-14*dheight;

	if(pPlane && pPlane->wing2()) ZPos -= dheight;

	painter.drawText(LeftPos, ZPos, pPlane->planeName());
	D+=dheight;
	QString length, surface;
	Units::getLengthUnitLabel(length);
	Units::getAreaUnitLabel(surface);

	str1 = QString(tr("Wing Span      =")+"%1 ").arg(pPlane->planformSpan()*Units::mtoUnit(),10,'f',3);
	str1 += length;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("xyProj. Span   =")+"%1 ").arg(pPlane->projectedSpan()*Units::mtoUnit(),10,'f',3);
	str1 += length;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Wing Area      =")+"%1 ").arg(pPlane->planformArea() * Units::m2toUnit(),10,'f',3);
	str1 += surface;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("xyProj. Area   =")+"%1 ").arg(pPlane->projectedArea() * Units::m2toUnit(),10,'f',3);
	str1 += surface;
	painter.drawText(LeftPos,ZPos+D, str1);
	D+=dheight;

	Units::getWeightUnitLabel(str);
	Result = QString(tr("Plane Mass     =")+"%1 ").arg(Mass*Units::kgtoUnit(),10,'f',3);
	Result += str;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	Units::getAreaUnitLabel(strong);
	Result = QString(tr("Wing Load      =")+"%1 ").arg(Mass*Units::kgtoUnit()/pPlane->projectedArea()/Units::m2toUnit(),10,'f',3);
	Result += str + "/" + strong;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	if(pPlane && pPlane->wing2())
	{
		str1 = QString(tr("Tail Volume    =")+"%1").arg(pPlane->tailVolume(),10,'f',3);
		painter.drawText(LeftPos, ZPos+D, str1);
		D+=dheight;
	}

	str1 = QString(tr("Root Chord     =")+"%1 ").arg(pPlane->m_Wing[0].rootChord()*Units::mtoUnit(), 10,'f', 3);
	Result = str1+length;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	str1 = QString(tr("MAC            =")+"%1 ").arg(pPlane->mac()*Units::mtoUnit(), 10,'f', 3);
	Result = str1+length;
	painter.drawText(LeftPos, ZPos+D, Result);
	D+=dheight;

	str1 = QString(tr("TipTwist       =")+"%1").arg(pPlane->m_Wing[0].tipTwist(), 10,'f', 3) + QString::fromUtf8("°");
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Aspect Ratio   =")+"%1").arg(pPlane->aspectRatio(),10,'f',3);
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Taper Ratio    =")+"%1").arg(pPlane->taperRatio(),10,'f',3);
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("Root-Tip Sweep =")+"%1").arg(pPlane->m_Wing[0].averageSweep(), 10,'f',3) + QString::fromUtf8("°");
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	str1 = QString(tr("TailVolume     =")+"%1").arg(pPlane->tailVolume(), 10,'f',3);
	painter.drawText(LeftPos, ZPos+D, str1);
	D+=dheight;

	painter.restore();
}



bool EditPlaneDlg::loadSettings(QSettings *pSettings)
{
	pSettings->beginGroup("EditPlaneDlg");
	{
	//  we're reading/loading
		s_WindowSize              = pSettings->value("WindowSize", QSize(1031,783)).toSize();
		s_bWindowMaximized        = pSettings->value("WindowMaximized", false).toBool();
		s_WindowPosition          = pSettings->value("WindowPosition", QPoint(131, 77)).toPoint();
		m_HorizontalSplitterSizes = pSettings->value("HorizontalSplitterSizes").toByteArray();
	}
	pSettings->endGroup();
	return true;
}


bool EditPlaneDlg::saveSettings(QSettings *pSettings)
{
	pSettings->beginGroup("EditPlaneDlg");
	{
		pSettings->setValue("WindowSize", s_WindowSize);
		pSettings->setValue("WindowMaximized", s_bWindowMaximized);
		pSettings->setValue("WindowPosition", s_WindowPosition);
		pSettings->setValue("HorizontalSplitterSizes", m_HorizontalSplitterSizes);
	}
	pSettings->endGroup();

	return true;
}






