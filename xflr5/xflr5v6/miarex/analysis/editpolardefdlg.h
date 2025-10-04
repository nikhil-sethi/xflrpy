/****************************************************************************

    EditPolarDefDlg Class
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
#include <QTreeView>
#include <QStandardItemModel>
#include <QDialogButtonBox>

class WPolar;
class Plane;
class EditObjectDelegate;

class EditPolarDefDlg : public QDialog
{
    Q_OBJECT

    friend class Wing;
    friend class Plane;
    friend class Miarex;
    friend class WPolar;

    private slots:
        void accept() override;
        void onItemChanged();
        void onButton(QAbstractButton *pButton);

    public:
        EditPolarDefDlg(QWidget *pParent=nullptr);

        void showEvent(QShowEvent *pEvent) override;
        void hideEvent(QHideEvent *pEvent) override;
        void resizeEvent(QResizeEvent *pEvent) override;
        void keyPressEvent(QKeyEvent *pEvent) override;

        QSize sizeHint() const override {return QSize(700,900);}

        void initDialog(Plane *pPlane, WPolar *pWPolar);
        void setupLayout();
        void showWPolar();
        void fillInertiaData(QList<QStandardItem *> inertiaFolder);
        void fillControlFields(QList<QStandardItem *> stabControlFolder);
        void readData();
        void readViewLevel(QModelIndex indexLevel);
        void readControlFields(QModelIndex indexLevel);

        QList<QStandardItem *> prepareRow(const QString &object, const QString &field="", const QString &value="",  const QString &unit="");
        QList<QStandardItem *> prepareBoolRow(const QString &first, const QString &second, const bool &third);
        QList<QStandardItem *> prepareIntRow(const QString &object, const QString &field, const int &value);
        QList<QStandardItem *> prepareDoubleRow(const QString &object, const QString &field, const double &value,  const QString &unit);


    private:
        WPolar * m_pWPolar;
        Plane * m_pPlane;
        QTreeView * m_pStruct;
        QStandardItemModel *m_pModel;
        EditObjectDelegate *m_pDelegate;
        QDialogButtonBox *m_pButtonBox;

        static QByteArray s_Geometry;
};

