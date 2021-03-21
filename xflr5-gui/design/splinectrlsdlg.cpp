/****************************************************************************

    SplineCtrlsDlg
    Copyright (C) 2009-2016 Andre Deperrois

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


#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>


#include "afoil.h"
#include "splinectrlsdlg.h"
#include <gui_objects/splinefoil.h>
#include <misc/options/settings.h>
#include <misc/text/doubleedit.h>
#include <misc/text/floatrditdelegate.h>

AFoil *SplineCtrlsDlg::s_pAFoil = nullptr;

SplineCtrlsDlg::SplineCtrlsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Spline Parameters"));
    m_pSF = nullptr;
    setupLayout();
}


SplineCtrlsDlg::~SplineCtrlsDlg()
{
    delete [] m_precision;
}


void SplineCtrlsDlg::initDialog()
{
    QString str;
    m_pctrlDegExtrados->clear();
    m_pctrlDegIntrados->clear();
    for (int i=2; i<6; i++)
    {
        str = QString("%1").arg(i);
        m_pctrlDegExtrados->addItem(str);
        m_pctrlDegIntrados->addItem(str);
    }
    m_pctrlDegExtrados->setEnabled(true);
    m_pctrlDegIntrados->setEnabled(true);


    m_pctrlDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
    m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
    m_pctrlOutExtrados->setValue(m_pSF->m_Extrados.m_iRes);
    m_pctrlOutIntrados->setValue(m_pSF->m_Intrados.m_iRes);


    //upper point list
    m_pUpperListModel = new QStandardItemModel(this);
    m_pUpperListModel->setRowCount(10);//temporary
    m_pUpperListModel->setColumnCount(3);

    m_pUpperListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
    m_pUpperListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
    m_pUpperListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

    m_pctrlUpperList->setModel(m_pUpperListModel);

    QHeaderView *HorizontalHeader = m_pctrlUpperList->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pUpperFloatDelegate = new FloatEditDelegate(this);
    m_pctrlUpperList->setItemDelegate(m_pUpperFloatDelegate);

    //Lower point list
    m_pLowerListModel = new QStandardItemModel(this);
    m_pLowerListModel->setRowCount(10);//temporary
    m_pLowerListModel->setColumnCount(3);

    m_pLowerListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
    m_pLowerListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
    m_pLowerListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

    m_pctrlLowerList->setModel(m_pLowerListModel);

    HorizontalHeader = m_pctrlLowerList->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pLowerFloatDelegate = new FloatEditDelegate(this);
    m_pctrlLowerList->setItemDelegate(m_pLowerFloatDelegate);

    m_precision = new int[3];
    m_precision[0] = 0;
    m_precision[1] = 5;
    m_precision[2] = 5;
    m_pUpperFloatDelegate->setPrecision(m_precision);
    m_pLowerFloatDelegate->setPrecision(m_precision);

    connect(m_pUpperFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onUpdate()));
    connect(m_pLowerFloatDelegate, SIGNAL(closeEditor(QWidget *)), this, SLOT(onUpdate()));


    fillPointLists();

    setControls();
}


void SplineCtrlsDlg::showEvent(QShowEvent *event)
{
    int w = m_pctrlUpperList->width();
    m_pctrlUpperList->setColumnWidth(0,(int)(w/3)-20);
    m_pctrlUpperList->setColumnWidth(1,(int)(w/3)-20);
    m_pctrlUpperList->setColumnWidth(2,(int)(w/3)-20);
    w = m_pctrlLowerList->width();
    m_pctrlLowerList->setColumnWidth(0,(int)(w/3)-20);
    m_pctrlLowerList->setColumnWidth(1,(int)(w/3)-20);
    m_pctrlLowerList->setColumnWidth(2,(int)(w/3)-20);
    event->accept();
}



void SplineCtrlsDlg::setupLayout()
{
    QGroupBox *pUpperSideBox = new QGroupBox(tr("Upper side"));
    {
        QVBoxLayout *pUpperSideLayout = new QVBoxLayout;
        {
            QGridLayout *pUpperLayout = new QGridLayout;
            {
                QLabel *labupper1 = new QLabel(tr("Spline degree"));
                QLabel *labupper2 = new QLabel(tr("Output"));
                m_pctrlDegExtrados = new QComboBox;
                m_pctrlOutExtrados = new DoubleEdit;
                m_pctrlOutExtrados->setPrecision(0);
                pUpperLayout->addWidget(labupper1, 1,1);
                pUpperLayout->addWidget(labupper2, 2,1);
                pUpperLayout->addWidget(m_pctrlDegExtrados, 1,2);
                pUpperLayout->addWidget(m_pctrlOutExtrados, 2,2);
            }


            m_pctrlUpperList = new QTableView(this);
            m_pctrlUpperList->setFont(Settings::s_TableFont);
            m_pctrlUpperList->setWindowTitle(QObject::tr("Upper side points"));
            m_pctrlUpperList->setMinimumHeight(200);
            m_pctrlUpperList->setMinimumWidth(250);
            m_pctrlUpperList->setSelectionBehavior(QAbstractItemView::SelectRows);
            pUpperSideLayout->addLayout(pUpperLayout);
            pUpperSideLayout->addStretch(1);
            pUpperSideLayout->addWidget(m_pctrlUpperList);
        }
        pUpperSideBox->setLayout(pUpperSideLayout);
    }

    QGroupBox *pLowerSideBox = new QGroupBox(tr("Lower side"));
    {
        QVBoxLayout *pLowerSideLayout = new QVBoxLayout;
        {
            QGridLayout *pLowerLayout = new QGridLayout;
            {
                QLabel *lablower1 = new QLabel(tr("Spline degree"));
                QLabel *lablower2 = new QLabel(tr("Output"));
                m_pctrlDegIntrados = new QComboBox;
                m_pctrlOutIntrados = new DoubleEdit;
                m_pctrlOutIntrados->setPrecision(0);
                pLowerLayout->addWidget(lablower1, 1,1);
                pLowerLayout->addWidget(lablower2, 2,1);
                pLowerLayout->addWidget(m_pctrlDegIntrados, 1,2);
                pLowerLayout->addWidget(m_pctrlOutIntrados, 2,2);
            }

            m_pctrlLowerList = new QTableView(this);
            m_pctrlLowerList->setFont(Settings::s_TableFont);
            m_pctrlLowerList->setWindowTitle(QObject::tr("Lower side points"));
            m_pctrlLowerList->setMinimumHeight(200);
            m_pctrlLowerList->setMinimumWidth(250);
            m_pctrlLowerList->setSelectionBehavior(QAbstractItemView::SelectRows);
            pLowerSideLayout->addLayout(pLowerLayout);
            pLowerSideLayout->addStretch(1);
            pLowerSideLayout->addWidget(m_pctrlLowerList);
        }
        pLowerSideBox->setLayout(pLowerSideLayout);
    }

    QHBoxLayout *pSideLayout = new QHBoxLayout;
    {
        pSideLayout->addWidget(pUpperSideBox);
        pSideLayout->addWidget(pLowerSideBox);
    }


    QHBoxLayout *pCommandButtons = new QHBoxLayout;
    {
        OKButton        = new QPushButton(tr("OK"));
        CancelButton    = new QPushButton(tr("Cancel"));
        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(OKButton);
        pCommandButtons->addWidget(CancelButton);
        pCommandButtons->addStretch(1);
    }

    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        m_pctrlSymetric = new QCheckBox(tr("Symetric foil"));
        QHBoxLayout *pClosedLayout = new QHBoxLayout;
        {
            m_pctrlCloseLE = new QCheckBox(tr("Force closed LE"));
            m_pctrlCloseTE = new QCheckBox(tr("Force closed TE"));
            pClosedLayout->addWidget(m_pctrlCloseLE);
            pClosedLayout->addWidget(m_pctrlCloseTE);
        }
        pMainLayout->addLayout(pSideLayout);
        pMainLayout->addStretch(1);
        pMainLayout->addWidget(m_pctrlSymetric);
        pMainLayout->addLayout(pClosedLayout);
        pMainLayout->addStretch(1);

        pMainLayout->addLayout(pCommandButtons);
        setLayout(pMainLayout);
    }


    connect(OKButton, SIGNAL(clicked()),this, SLOT(onOK()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    connect(m_pctrlSymetric, SIGNAL(clicked()), this, SLOT(onUpdate()));
    connect(m_pctrlDegExtrados, SIGNAL(activated(int)), this, SLOT(onUpdate()));
    connect(m_pctrlDegIntrados, SIGNAL(activated(int)), this, SLOT(onUpdate()));
    connect(m_pctrlOutExtrados, SIGNAL(editingFinished()), this, SLOT(onUpdate()));
    connect(m_pctrlOutIntrados, SIGNAL(editingFinished()), this, SLOT(onUpdate()));
}



void SplineCtrlsDlg::fillPointLists()
{
    m_pUpperListModel->setRowCount(m_pSF->m_Extrados.m_CtrlPoint.size());
    for (int i=0; i<m_pSF->m_Extrados.m_CtrlPoint.size(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 0, QModelIndex());
        m_pUpperListModel->setData(index, i+1);

        QModelIndex Xindex =m_pUpperListModel->index(i, 1, QModelIndex());
        m_pUpperListModel->setData(Xindex, m_pSF->m_Extrados.m_CtrlPoint[i].x);

        QModelIndex Zindex =m_pUpperListModel->index(i, 2, QModelIndex());
        m_pUpperListModel->setData(Zindex, m_pSF->m_Extrados.m_CtrlPoint[i].y);
    }

    m_pLowerListModel->setRowCount(m_pSF->m_Intrados.m_CtrlPoint.size());
    for (int i=0; i<m_pSF->m_Intrados.m_CtrlPoint.size(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 0, QModelIndex());
        m_pLowerListModel->setData(index, i+1);

        QModelIndex Xindex =m_pLowerListModel->index(i, 1, QModelIndex());
        m_pLowerListModel->setData(Xindex, m_pSF->m_Intrados.m_CtrlPoint[i].x);

        QModelIndex Zindex =m_pLowerListModel->index(i, 2, QModelIndex());
        m_pLowerListModel->setData(Zindex, m_pSF->m_Intrados.m_CtrlPoint[i].y);
    }
}


void SplineCtrlsDlg::readData()
{
    for(int i=0; i<m_pSF->m_Extrados.m_CtrlPoint.size(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 1, QModelIndex());
        m_pSF->m_Extrados.m_CtrlPoint[i].x = index.data().toDouble();

        index = m_pUpperListModel->index(i, 2, QModelIndex());
        m_pSF->m_Extrados.m_CtrlPoint[i].y = index.data().toDouble();
    }
    for (int i=0; i<m_pSF->m_Intrados.m_CtrlPoint.size(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 1, QModelIndex());
        m_pSF->m_Intrados.m_CtrlPoint[i].x = index.data().toDouble();

        index = m_pLowerListModel->index(i, 2, QModelIndex());
        m_pSF->m_Intrados.m_CtrlPoint[i].y = index.data().toDouble();
    }

    int ideg = m_pctrlDegExtrados->currentIndex()+2;
    if(ideg<m_pSF->m_Extrados.m_CtrlPoint.size())
    {
        // there are enough control points for this degree
        m_pSF->m_Extrados.m_iDegree = ideg;
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));
        m_pSF->m_Extrados.m_iDegree = qMax(2,m_pSF->m_Extrados.m_CtrlPoint.size()-1);
        m_pctrlDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
    }

    ideg = m_pctrlDegIntrados->currentIndex()+2;
    if(ideg<m_pSF->m_Intrados.m_CtrlPoint.size())
    {
        // there are enough control points for this degree
        m_pSF->m_Intrados.m_iDegree = ideg;
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));

        m_pSF->m_Intrados.m_iDegree = qMax(2,m_pSF->m_Intrados.m_CtrlPoint.size()-1);
        m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
    }

    m_pSF->m_Extrados.m_iRes = m_pctrlOutExtrados->value();
    m_pSF->m_Intrados.m_iRes = m_pctrlOutExtrados->value();
    m_pSF->m_bSymetric = m_pctrlSymetric->isChecked();

    if(m_pSF->m_bSymetric)
    {
        m_pSF->m_Intrados.copySymetric(&m_pSF->m_Extrados);
    }

    m_pSF->setClosedLE(m_pctrlCloseLE->isChecked());
    m_pSF->setClosedTE(m_pctrlCloseTE->isChecked());
}


void SplineCtrlsDlg::setControls()
{
    m_pctrlSymetric->setChecked(m_pSF->m_bSymetric);
    if(m_pSF->m_bSymetric)
    {
        m_pctrlDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
        m_pctrlOutIntrados->setValue(m_pSF->m_Intrados.m_iRes);
        fillPointLists();
    }
    m_pctrlLowerList->setEnabled(!m_pSF->m_bSymetric);
    m_pctrlDegIntrados->setEnabled(!m_pSF->m_bSymetric);
    m_pctrlOutIntrados->setEnabled(!m_pSF->m_bSymetric);

    m_pctrlCloseLE->setChecked(m_pSF->bClosedLE());
    m_pctrlCloseTE->setChecked(m_pSF->bClosedTE());
}




void SplineCtrlsDlg::keyPressEvent(QKeyEvent *event)
{
    // Prevent Return Key from closing App
    switch (event->key())
    {
        case Qt::Key_Escape:
        {
            reject();
            return;
        }
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!OKButton->hasFocus() && !CancelButton->hasFocus())
            {
                OKButton->setFocus();
            }
            else
            {
                onOK();
            }
            break;
        }
        default:
            event->ignore();
            break;
    }
}

void SplineCtrlsDlg::onOK()
{
    readData();
    accept();
}


void SplineCtrlsDlg::onUpdate()
{
    readData();
    setControls();

    updateSplines();
}


void SplineCtrlsDlg::updateSplines()
{
    m_pSF->m_Extrados.splineKnots();
    m_pSF->m_Extrados.splineCurve();
    m_pSF->m_Intrados.splineKnots();
    m_pSF->m_Intrados.splineCurve();
    m_pSF->updateSplineFoil();
    s_pAFoil->m_p2DWidget->update();
}










