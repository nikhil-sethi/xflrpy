/****************************************************************************

    ManagePlanesDlg Class
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
#include <QTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

class Plane;
class PlaneTableDelegate;

class ManagePlanesDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;

    public:
        ManagePlanesDlg(QWidget *pParent);
        ~ManagePlanesDlg();

        void initDialog(QString &UFOName);

    private slots:
        void onDelete();
        void onRename();
        void onTableRowChanged(QModelIndex index);
        void onDoubleClickTable(const QModelIndex &index);
        void onDescriptionChanged();
        void onButton(QAbstractButton *pButton);

    private:
        void resizeEvent(QResizeEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;

        void fillPlaneTable();
        void fillPlaneRow(int row);

        void setupLayout();
        void selectPlane();

    private:

        QDialogButtonBox *m_pButtonBox;

        QPushButton *m_ppbRename, *m_ppbDelete;
        QTextEdit *m_pteDescription;
        QTableView *m_ptvPlanes;
        QStandardItemModel *m_pPlaneModel;
        PlaneTableDelegate *m_pPlaneDelegate;
        //    QItemSelectionModel *m_pSelectionModel;

        Plane *m_pPlane;
        bool m_bChanged;
};


