/****************************************************************************

    FoilCoordDlg Class
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


#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

class FloatEditDelegate;
class Foil;

class FoilCoordDlg : public QDialog
{
    Q_OBJECT

    friend class XDirect;
    friend class AFoil;

    public:
        FoilCoordDlg(QWidget *pParent);
        ~FoilCoordDlg();

        void initDialog();

    private slots:
        void onDeletePoint();
        void onInsertPoint();
        void onRestore();
        void onApply();
        void onCellChanged(QWidget *);
        void onItemClicked(QModelIndex);
        void onButton(QAbstractButton *pButton);


    private:
        void fillList();
        void setSelection(int sel);
        void setupLayout();
        void readSectionData(int sel, double &X, double &Y);
        void keyPressEvent(QKeyEvent *pEvent) override;
        void showEvent(QShowEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        QSize sizeHint() const override {return QSize(450,750);}

    private:
        QDialogButtonBox *m_pButtonBox;
        QPushButton *m_ppbInsertPoint, *m_ppbDeletePoint;

        QTableView *m_ptvCoordTable;
        QStandardItemModel *m_pCoordModel;
        FloatEditDelegate *m_pFloatDelegate;

        Foil const *m_pMemFoil;
        Foil * m_pBufferFoil;

        bool m_bApplied, m_bModified;
        QWidget *m_pParent;
};


