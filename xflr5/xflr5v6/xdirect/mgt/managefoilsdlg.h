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


#pragma once

#include <QDialog>

#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

class FoilTableDelegate;
class Foil;

class ManageFoilsDlg : public QDialog
{
    Q_OBJECT
    friend class XDirect;
    friend class MainFrame;

    public:
        ManageFoilsDlg(QWidget *pParent=nullptr);
        ~ManageFoilsDlg();

        void initDialog(QString FoilName);
        void resizeEvent(QResizeEvent *pEvent);
        void keyPressEvent(QKeyEvent *pEvent);
        void showEvent(QShowEvent *pEvent);
        void hideEvent(QHideEvent *pEvent);

        QSize sizeHint() const {return QSize(1200,700);}

    private slots:
        void onDelete();
        void onRename();
        void onExport();
        void onFoilClicked(const QModelIndex& index);
        void onDoubleClickTable(const QModelIndex &index);

    private:

        void fillFoilTable();
        void fillTableRow(int row);

        void setupLayout();

    private:
        QDialogButtonBox *m_pButtonBox;

        QPushButton *m_ppbRename, *m_ppbDelete, *m_ppbSelect, *m_ppbExport;
        QTableView *m_ptvFoils;
        QStandardItemModel *m_pFoilModel;
        FoilTableDelegate *m_pFoilDelegate;

        int m_iSelection;
        Foil *m_pFoil;
        bool m_bChanged;

        static QByteArray s_Geometry;
};

