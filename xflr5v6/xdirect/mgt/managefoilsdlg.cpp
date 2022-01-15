/****************************************************************************

    ManageFoilsDlg Class
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


#include <QHBoxLayout>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStringList>


#include "managefoilsdlg.h"

#include <design/foiltabledelegate.h>
#include <xflobjects/editors/renamedlg.h>

#include <xflobjects/objects2d/objects2d.h>
#include <xflcore/displayoptions.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects2d/foil.h>

QByteArray ManageFoilsDlg::s_Geometry;


ManageFoilsDlg::ManageFoilsDlg(QWidget *pParent) : QDialog(pParent)
{
    setWindowTitle(tr("Foil Management"));

    m_bChanged = false;
    m_iSelection = 0;
    m_pFoil = nullptr;
    setupLayout();

    connect(m_ppbDelete, SIGNAL(clicked()),this, SLOT(onDelete()));
    connect(m_ppbRename, SIGNAL(clicked()),this, SLOT(onRename()));
    connect(m_ppbExport, SIGNAL(clicked()),this, SLOT(onExport()));
    connect(m_ptvFoils, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onDoubleClickTable(const QModelIndex &)));
}


void ManageFoilsDlg::initDialog(QString FoilName)
{
    fillFoilTable();
    QString strong;

    if(m_pFoilModel->rowCount())
    {
        if(FoilName.length())
        {
            QModelIndex ind;
            for(int i=0; i< m_pFoilModel->rowCount(); i++)
            {
                ind = m_pFoilModel->index(i, 0, QModelIndex());
                strong = ind.model()->data(ind, Qt::EditRole).toString();

                if(strong == FoilName)
                {
                    m_ptvFoils->selectRow(i);
                    break;
                }
            }
        }
        else
        {
            m_ptvFoils->selectRow(0);
            QStandardItem *pItem = m_pFoilModel->item(0,0);
            FoilName = pItem->text();
        }
        m_pFoil = Objects2d::foil(FoilName);
    }
    else
    {
        m_pFoil = nullptr;
    }
}


void ManageFoilsDlg::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            if(!m_pButtonBox->hasFocus()) m_pButtonBox->setFocus();
            else                          accept();

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


void ManageFoilsDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_Geometry);
}



void ManageFoilsDlg::hideEvent(QHideEvent*)
{
    s_Geometry = saveGeometry();
}


void ManageFoilsDlg::setupLayout()
{
    QVBoxLayout *pCommandButtons = new QVBoxLayout;
    {
        m_ppbDelete     = new QPushButton(tr("Delete"));
        m_ppbRename     = new QPushButton(tr("Rename"));
        m_ppbExport     = new QPushButton(tr("Export Foil"));

        m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        {
            connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        }

        pCommandButtons->addStretch(1);
        pCommandButtons->addWidget(m_ppbDelete);
        pCommandButtons->addWidget(m_ppbRename);
        pCommandButtons->addWidget(m_ppbExport);
        pCommandButtons->addStretch(2);
        pCommandButtons->addWidget(m_pButtonBox);
        pCommandButtons->addStretch(1);
    }

    m_ptvFoils   = new QTableView(this);
    m_ptvFoils->setFont(DisplayOptions::tableFont());
    m_ptvFoils->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ptvFoils->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ptvFoils->horizontalHeader()->setStretchLastSection(true);

    QSizePolicy szPolicyExpanding;
    szPolicyExpanding.setHorizontalPolicy(QSizePolicy::MinimumExpanding);
    szPolicyExpanding.setVerticalPolicy(QSizePolicy::Expanding);
    m_ptvFoils->setSizePolicy(szPolicyExpanding);

    QHBoxLayout * pMainLayout = new QHBoxLayout(this);
    {
        pMainLayout->addWidget(m_ptvFoils);
        pMainLayout->addLayout(pCommandButtons);

        pMainLayout->setStretchFactor(m_ptvFoils,1);
    }
    setLayout(pMainLayout);


    connect(m_ptvFoils, SIGNAL(clicked(const QModelIndex &)), SLOT(onFoilClicked(const QModelIndex&)));
    connect(m_ptvFoils, SIGNAL(pressed(const QModelIndex &)), SLOT(onFoilClicked(const QModelIndex&)));


    m_pFoilModel = new QStandardItemModel(this);
    m_pFoilModel->setRowCount(10);//temporary
    m_pFoilModel->setColumnCount(12);

    m_pFoilModel->setHeaderData(0, Qt::Horizontal, tr("Name"));
    m_pFoilModel->setHeaderData(1, Qt::Horizontal, tr("Thickness (%)"));
    m_pFoilModel->setHeaderData(2, Qt::Horizontal, tr("at (%)"));
    m_pFoilModel->setHeaderData(3, Qt::Horizontal, tr("Camber (%)"));
    m_pFoilModel->setHeaderData(4, Qt::Horizontal, tr("at (%)"));
    m_pFoilModel->setHeaderData(5, Qt::Horizontal, tr("Points"));
    m_pFoilModel->setHeaderData(6, Qt::Horizontal, tr("TE Flap (")+QChar(0260)+")");
    m_pFoilModel->setHeaderData(7, Qt::Horizontal, tr("TE XHinge"));
    m_pFoilModel->setHeaderData(8, Qt::Horizontal, tr("TE YHinge"));
    m_pFoilModel->setHeaderData(9, Qt::Horizontal, tr("LE Flap (")+QChar(0260)+")");
    m_pFoilModel->setHeaderData(10, Qt::Horizontal, tr("LE XHinge"));
    m_pFoilModel->setHeaderData(11, Qt::Horizontal, tr("LE YHinge"));

    m_ptvFoils->setModel(m_pFoilModel);
    m_ptvFoils->setWindowTitle(tr("Foils"));

    m_pFoilDelegate = new FoilTableDelegate(this);
    m_pFoilDelegate->m_pManageFoils = this;
    m_ptvFoils->setItemDelegate(m_pFoilDelegate);
    m_pFoilDelegate->m_pFoilModel = m_pFoilModel;

    QVector<int> m_precision(12,2);
    m_pFoilDelegate->m_Precision = m_precision;
}


ManageFoilsDlg::~ManageFoilsDlg()
{
}


void ManageFoilsDlg::fillFoilTable()
{
    int i;
    m_pFoilModel->setRowCount(Objects2d::foilCount());

    for(i=0; i< Objects2d::foilCount(); i++)
    {
        fillTableRow(i);
    }
}


void ManageFoilsDlg::fillTableRow(int row)
{
    QModelIndex ind;
    Foil *pFoil = Objects2d::foilAt(row);

    ind = m_pFoilModel->index(row, 0, QModelIndex());
    m_pFoilModel->setData(ind,pFoil->name());

    if(pFoil->description().length()) m_pFoilModel->setData(ind, pFoil->description(), Qt::ToolTipRole);

    ind = m_pFoilModel->index(row, 1, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->thickness());

    ind = m_pFoilModel->index(row, 2, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->xThickness());

    ind = m_pFoilModel->index(row, 3, QModelIndex());
    m_pFoilModel->setData(ind, pFoil->camber());

    ind = m_pFoilModel->index(row, 4, QModelIndex());
    m_pFoilModel->setData(ind,pFoil->xCamber());

    ind = m_pFoilModel->index(row, 5, QModelIndex());
    m_pFoilModel->setData(ind,pFoil->m_n);


    if(pFoil && pFoil->m_bTEFlap)
    {
        ind = m_pFoilModel->index(row, 6, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEFlapAngle);

        ind = m_pFoilModel->index(row, 7, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEXHinge/100.0);

        ind = m_pFoilModel->index(row, 8, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_TEYHinge/100.0);

    }
    if(pFoil && pFoil->m_bLEFlap)
    {
        ind = m_pFoilModel->index(row, 9, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEFlapAngle);

        ind = m_pFoilModel->index(row, 10, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEXHinge/100.0);

        ind = m_pFoilModel->index(row, 11, QModelIndex());
        m_pFoilModel->setData(ind,pFoil->m_LEYHinge/100.0);
    }
}


void ManageFoilsDlg::onRename()
{
    if (m_pFoil)
    {
        QStringList NameList;
        for(int k=0; k< Objects2d::foilCount(); k++)
        {
            Foil *pOldFoil= Objects2d::foilAt(k);
            NameList.append(pOldFoil->name());
        }

        RenameDlg renDlg(this);
        renDlg.initDialog(&NameList, m_pFoil->name(), tr("Enter the foil's new name"));

        if(renDlg.exec()!=QDialog::Rejected)
        {
            Objects2d::renameThisFoil(m_pFoil, renDlg.newName());
            fillFoilTable();
            m_bChanged = true;
        }
    }
}


void ManageFoilsDlg::onExport()
{
    if(!m_pFoil)    return;

    QString FileName;

    FileName = m_pFoil->name();
    FileName.replace("/", " ");

    FileName = QFileDialog::getSaveFileName(this, tr("Export Foil"),
                                            xfl::s_LastDirName+"/"+FileName+".dat",
                                            tr("Foil File (*.dat)"));

    if(!FileName.length()) return;
    int pos = FileName.lastIndexOf("/");
    if(pos>0) xfl::s_LastDirName = FileName.left(pos);

    QFile XFile(FileName);

    if (!XFile.open(QIODevice::WriteOnly | QIODevice::Text)) return ;

    QTextStream out(&XFile);

    m_pFoil->exportFoil(out);
    XFile.close();
}


void ManageFoilsDlg::onDelete()
{
    if(!m_pFoil) return;

    Objects2d::deleteFoil(m_pFoil);

    QModelIndex index = m_ptvFoils->currentIndex();
    int sel = qMax(index.row()-1,0);

    fillFoilTable();

    if(m_pFoilModel->rowCount()>0)
    {
        m_ptvFoils->selectRow(sel);

        QStandardItem *pItem = m_pFoilModel->item(sel,0);
        QString FoilName = pItem->text();

        m_pFoil = Objects2d::foil(FoilName);
    }
    else m_pFoil = nullptr;

    m_bChanged = true;
}



void ManageFoilsDlg::onDoubleClickTable(const QModelIndex &index)
{
    if(index.row()>=0) accept();
}


void ManageFoilsDlg::onFoilClicked(const QModelIndex& index)
{
    if(index.row()>=Objects2d::foilCount()) return;

    QStandardItem *pItem = m_pFoilModel->item(index.row(),0);
    QString FoilName =pItem->text();

    m_pFoil= Objects2d::foil(FoilName);
}


void ManageFoilsDlg::resizeEvent(QResizeEvent *pEvent)
{
    int w = m_ptvFoils->width();
    int w12 = int(double(w)/13.0);
    int w14 = int(double(w)/15.0);

    m_ptvFoils->setColumnWidth(1,w12);
    m_ptvFoils->setColumnWidth(2,w12);
    m_ptvFoils->setColumnWidth(3,w12);
    m_ptvFoils->setColumnWidth(4,w12);
    m_ptvFoils->setColumnWidth(5,w14);//points

    m_ptvFoils->setColumnWidth(6,w14);//TE Flap
    m_ptvFoils->setColumnWidth(7,w12);//TE XHinge
    m_ptvFoils->setColumnWidth(8,w12);//TE YHinge

    m_ptvFoils->setColumnWidth(9, w14);//LE Flap
    m_ptvFoils->setColumnWidth(10,w12);//LE XHinge
    m_ptvFoils->setColumnWidth(11,w12);//LE YHinge

    m_ptvFoils->setColumnWidth(0,w-8*w12-3*w14-40);
    pEvent->accept();
}





