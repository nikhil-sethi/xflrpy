/****************************************************************************

    PertDlg class
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

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItem>

#include <xfoil_params.h>

class DoubleEdit;
class Foil;
class FloatEditDelegate;

class PertDlg : public QDialog
{
    Q_OBJECT

    friend class MainFrame;
    friend class XInverse;

    public:
        PertDlg(QWidget *pParent);
        ~PertDlg();


    private slots:
        void onCellChanged(QWidget *);
        void onRestore();
        void onApply();
        void onOK();
        void onButton(QAbstractButton *pButton);

    private:
        void setupLayout();
        void initDialog();
        void fillCnModel() ;
        void readData();

    private:
        QDialogButtonBox *m_pButtonBox;

        QTableView *m_ptvCn;
        QStandardItemModel m_CnModel;
        FloatEditDelegate *m_pFloatDelegate;

    protected:
        void keyPressEvent(QKeyEvent *pEvent) override;


    private:
        void * m_pXInverse;
        int   m_nc;
        double m_cnr[IMX+1];
        double m_cni[IMX+1];
        double m_backr[IMX+1];
        double m_backi[IMX+1];
};


