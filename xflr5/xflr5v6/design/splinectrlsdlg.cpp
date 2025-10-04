/****************************************************************************

    SplineCtrlsDlg
    Copyright (C) 2009-2016 André Deperrois

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


#include "splinectrlsdlg.h"
#include <design/afoil.h>
#include <gui_objects/splinefoil.h>

#include <xflcore/displayoptions.h>
#include <xflwidgets/customwts/floateditdelegate.h>
#include <xflwidgets/customwts/intedit.h>

AFoil *SplineCtrlsDlg::s_pAFoil = nullptr;

SplineCtrlsDlg::SplineCtrlsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Spline Parameters"));
    m_pSF = nullptr;
    setupLayout();
}


SplineCtrlsDlg::~SplineCtrlsDlg()
{
}


void SplineCtrlsDlg::initDialog()
{
    QString str;

    m_pleSFName->setText(m_pSF->splineFoilName());

    m_pcbDegExtrados->clear();
    m_pcbDegIntrados->clear();
    for (int i=2; i<6; i++)
    {
        str = QString("%1").arg(i);
        m_pcbDegExtrados->addItem(str);
        m_pcbDegIntrados->addItem(str);
    }
    m_pcbDegExtrados->setEnabled(true);
    m_pcbDegIntrados->setEnabled(true);

    m_pcbDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
    m_pcbDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
    m_pieOutExtrados->setValue(m_pSF->m_Extrados.m_iRes);
    m_pieOutIntrados->setValue(m_pSF->m_Intrados.m_iRes);


    //upper point list
    m_pUpperListModel = new QStandardItemModel(this);
    m_pUpperListModel->setRowCount(10);//temporary
    m_pUpperListModel->setColumnCount(3);

    m_pUpperListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
    m_pUpperListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
    m_pUpperListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

    m_ptvUpperList->setModel(m_pUpperListModel);

    QHeaderView *HorizontalHeader = m_ptvUpperList->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pUpperFloatDelegate = new FloatEditDelegate(this);
    m_ptvUpperList->setItemDelegate(m_pUpperFloatDelegate);

    //Lower point list
    m_pLowerListModel = new QStandardItemModel(this);
    m_pLowerListModel->setRowCount(10);//temporary
    m_pLowerListModel->setColumnCount(3);

    m_pLowerListModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Point"));
    m_pLowerListModel->setHeaderData(1, Qt::Horizontal, QObject::tr("x"));
    m_pLowerListModel->setHeaderData(2, Qt::Horizontal, QObject::tr("y"));

    m_ptvLowerList->setModel(m_pLowerListModel);

    HorizontalHeader = m_ptvLowerList->horizontalHeader();
    HorizontalHeader->setStretchLastSection(true);

    m_pLowerFloatDelegate = new FloatEditDelegate(this);
    m_ptvLowerList->setItemDelegate(m_pLowerFloatDelegate);

    QVector<int> precision = {0,5,5};
    m_pUpperFloatDelegate->setPrecision(precision);
    m_pLowerFloatDelegate->setPrecision(precision);

    connect(m_pUpperFloatDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onUpdate()));
    connect(m_pLowerFloatDelegate, SIGNAL(closeEditor(QWidget*)), SLOT(onUpdate()));

    fillPointLists();
    setControls();
}


void SplineCtrlsDlg::showEvent(QShowEvent *)
{
    int w = m_ptvUpperList->width();
    m_ptvUpperList->setColumnWidth(0,int(w/3)-20);
    m_ptvUpperList->setColumnWidth(1,int(w/3)-20);
    m_ptvUpperList->setColumnWidth(2,int(w/3)-20);
    w = m_ptvLowerList->width();
    m_ptvLowerList->setColumnWidth(0,int(w/3)-20);
    m_ptvLowerList->setColumnWidth(1,int(w/3)-20);
    m_ptvLowerList->setColumnWidth(2,int(w/3)-20);
}


void SplineCtrlsDlg::setupLayout()
{
    QVBoxLayout *pMainLayout = new QVBoxLayout;
    {
        QHBoxLayout *pNameLayout = new QHBoxLayout;
        {
            QLabel *pLabName = new QLabel(tr("Name"));
            m_pleSFName = new QLineEdit;
            pNameLayout->addWidget(pLabName);
            pNameLayout->addWidget(m_pleSFName);
        }

        QHBoxLayout *pSideLayout = new QHBoxLayout;
        {
            QGroupBox *pUpperSideBox = new QGroupBox(tr("Upper side"));
            {
                QVBoxLayout *pUpperSideLayout = new QVBoxLayout;
                {
                    QGridLayout *pUpperLayout = new QGridLayout;
                    {
                        QLabel *labupper1 = new QLabel(tr("Spline degree"));
                        QLabel *labupper2 = new QLabel(tr("Output"));
                        m_pcbDegExtrados = new QComboBox;
                        m_pieOutExtrados = new IntEdit;
                        pUpperLayout->addWidget(labupper1, 1,1);
                        pUpperLayout->addWidget(labupper2, 2,1);
                        pUpperLayout->addWidget(m_pcbDegExtrados, 1,2);
                        pUpperLayout->addWidget(m_pieOutExtrados, 2,2);
                    }

                    m_ptvUpperList = new QTableView(this);
                    m_ptvUpperList->setFont(DisplayOptions::tableFont());
                    m_ptvUpperList->setWindowTitle(QObject::tr("Upper side points"));
                    m_ptvUpperList->setMinimumHeight(200);
                    m_ptvUpperList->setMinimumWidth(250);
                    m_ptvUpperList->setSelectionBehavior(QAbstractItemView::SelectRows);
                    pUpperSideLayout->addLayout(pUpperLayout);
                    pUpperSideLayout->addStretch(1);
                    pUpperSideLayout->addWidget(m_ptvUpperList);
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
                        m_pcbDegIntrados = new QComboBox;
                        m_pieOutIntrados = new IntEdit;
                        pLowerLayout->addWidget(lablower1, 1,1);
                        pLowerLayout->addWidget(lablower2, 2,1);
                        pLowerLayout->addWidget(m_pcbDegIntrados, 1,2);
                        pLowerLayout->addWidget(m_pieOutIntrados, 2,2);
                    }

                    m_ptvLowerList = new QTableView(this);
                    m_ptvLowerList->setFont(DisplayOptions::tableFont());
                    m_ptvLowerList->setWindowTitle(QObject::tr("Lower side points"));
                    m_ptvLowerList->setMinimumHeight(200);
                    m_ptvLowerList->setMinimumWidth(250);
                    m_ptvLowerList->setSelectionBehavior(QAbstractItemView::SelectRows);
                    pLowerSideLayout->addLayout(pLowerLayout);
                    pLowerSideLayout->addStretch(1);
                    pLowerSideLayout->addWidget(m_ptvLowerList);
                }
                pLowerSideBox->setLayout(pLowerSideLayout);
            }

            pSideLayout->addWidget(pUpperSideBox);
            pSideLayout->addWidget(pLowerSideBox);
        }

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        {
            connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onButton(QAbstractButton*)));
        }

        m_pchSymetric = new QCheckBox(tr("Symetric foil"));
        QHBoxLayout *pClosedLayout = new QHBoxLayout;
        {
            m_pchCloseLE = new QCheckBox(tr("Force closed LE"));
            m_pchCloseTE = new QCheckBox(tr("Force closed TE"));
            pClosedLayout->addWidget(m_pchCloseLE);
            pClosedLayout->addWidget(m_pchCloseTE);
        }

        pMainLayout->addLayout(pNameLayout);
        pMainLayout->addLayout(pSideLayout);
        pMainLayout->addWidget(m_pchSymetric);
        pMainLayout->addLayout(pClosedLayout);

        pMainLayout->addWidget(m_pButtonBox);
        setLayout(pMainLayout);
    }

    connect(m_pchSymetric,    SIGNAL(clicked()),         SLOT(onUpdate()));
    connect(m_pcbDegExtrados, SIGNAL(activated(int)),    SLOT(onUpdate()));
    connect(m_pcbDegIntrados, SIGNAL(activated(int)),    SLOT(onUpdate()));
    connect(m_pieOutExtrados, SIGNAL(editingFinished()), SLOT(onUpdate()));
    connect(m_pieOutIntrados, SIGNAL(editingFinished()), SLOT(onUpdate()));
}


void SplineCtrlsDlg::onButton(QAbstractButton *pButton)
{
    if      (m_pButtonBox->button(QDialogButtonBox::Ok)     == pButton)  onOK();
    else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)  reject();
}


void SplineCtrlsDlg::fillPointLists()
{
    m_pUpperListModel->setRowCount(m_pSF->m_Extrados.m_CtrlPt.size());
    for (int i=0; i<m_pSF->m_Extrados.m_CtrlPt.size(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 0, QModelIndex());
        m_pUpperListModel->setData(index, i+1);

        QModelIndex Xindex =m_pUpperListModel->index(i, 1, QModelIndex());
        m_pUpperListModel->setData(Xindex, m_pSF->m_Extrados.m_CtrlPt[i].x);

        QModelIndex Zindex =m_pUpperListModel->index(i, 2, QModelIndex());
        m_pUpperListModel->setData(Zindex, m_pSF->m_Extrados.m_CtrlPt[i].y);
    }

    m_pLowerListModel->setRowCount(m_pSF->m_Intrados.m_CtrlPt.size());
    for (int i=0; i<m_pSF->m_Intrados.m_CtrlPt.size(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 0, QModelIndex());
        m_pLowerListModel->setData(index, i+1);

        QModelIndex Xindex =m_pLowerListModel->index(i, 1, QModelIndex());
        m_pLowerListModel->setData(Xindex, m_pSF->m_Intrados.m_CtrlPt[i].x);

        QModelIndex Zindex =m_pLowerListModel->index(i, 2, QModelIndex());
        m_pLowerListModel->setData(Zindex, m_pSF->m_Intrados.m_CtrlPt[i].y);
    }
}


void SplineCtrlsDlg::readData()
{
    m_pSF->setSplineFoilName(m_pleSFName->text());

    for(int i=0; i<m_pSF->m_Extrados.m_CtrlPt.size(); i++)
    {
        QModelIndex index = m_pUpperListModel->index(i, 1, QModelIndex());
        m_pSF->m_Extrados.m_CtrlPt[i].x = index.data().toDouble();

        index = m_pUpperListModel->index(i, 2, QModelIndex());
        m_pSF->m_Extrados.m_CtrlPt[i].y = index.data().toDouble();
    }
    for (int i=0; i<m_pSF->m_Intrados.m_CtrlPt.size(); i++)
    {
        QModelIndex index = m_pLowerListModel->index(i, 1, QModelIndex());
        m_pSF->m_Intrados.m_CtrlPt[i].x = index.data().toDouble();

        index = m_pLowerListModel->index(i, 2, QModelIndex());
        m_pSF->m_Intrados.m_CtrlPt[i].y = index.data().toDouble();
    }

    int ideg = m_pcbDegExtrados->currentIndex()+2;
    if(ideg<m_pSF->m_Extrados.m_CtrlPt.size())
    {
        // there are enough control points for this degree
        m_pSF->m_Extrados.m_iDegree = ideg;
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));
        m_pSF->m_Extrados.m_iDegree = qMax(2,m_pSF->m_Extrados.m_CtrlPt.size()-1);
        m_pcbDegExtrados->setCurrentIndex(m_pSF->m_Extrados.m_iDegree-2);
    }

    ideg = m_pcbDegIntrados->currentIndex()+2;
    if(ideg<m_pSF->m_Intrados.m_CtrlPt.size())
    {
        // there are enough control points for this degree
        m_pSF->m_Intrados.m_iDegree = ideg;
    }
    else
    {
        // too few control points, adjust the degree
        QMessageBox::warning(this,tr("Warning"), tr("The spline degree must be less than the number of control points"));

        m_pSF->m_Intrados.m_iDegree = qMax(2,m_pSF->m_Intrados.m_CtrlPt.size()-1);
        m_pcbDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
    }

    m_pSF->m_Extrados.m_iRes = m_pieOutExtrados->value();
    m_pSF->m_Intrados.m_iRes = m_pieOutExtrados->value();
    m_pSF->m_bSymetric = m_pchSymetric->isChecked();

    if(m_pSF->m_bSymetric)
    {
        m_pSF->m_Intrados.copySymetric(&m_pSF->m_Extrados);
    }

    m_pSF->setClosedLE(m_pchCloseLE->isChecked());
    m_pSF->setClosedTE(m_pchCloseTE->isChecked());
}


void SplineCtrlsDlg::setControls()
{
    m_pchSymetric->setChecked(m_pSF->m_bSymetric);
    if(m_pSF->m_bSymetric)
    {
        m_pcbDegIntrados->setCurrentIndex(m_pSF->m_Intrados.m_iDegree-2);
        m_pieOutIntrados->setValue(m_pSF->m_Intrados.m_iRes);
        fillPointLists();
    }
    m_ptvLowerList->setEnabled(!m_pSF->m_bSymetric);
    m_pcbDegIntrados->setEnabled(!m_pSF->m_bSymetric);
    m_pieOutIntrados->setEnabled(!m_pSF->m_bSymetric);

    m_pchCloseLE->setChecked(m_pSF->bClosedLE());
    m_pchCloseTE->setChecked(m_pSF->bClosedTE());
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
            if(!m_pButtonBox->hasFocus())
            {
                m_pButtonBox->setFocus();
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
    s_pAFoil->m_p2dWidget->update();
}










