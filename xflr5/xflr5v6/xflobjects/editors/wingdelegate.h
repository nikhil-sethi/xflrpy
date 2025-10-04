/****************************************************************************

    WingDelegate Class
    Copyright (C) 2009 André Deperrois 

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


#include <QVector>
#include <QPainter>
#include <QStyledItemDelegate>

class Foil;

class DoubleEdit;

class WingDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    friend class WingDlg;
    friend class GL3dWingDlg;

    public:
        WingDelegate(QObject *parent = nullptr);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void setPrecision(QVector<int> PrecisionTable) {m_Precision = PrecisionTable;}

    private:
        void *m_pWingDlg;
        QVector<int> m_Precision; ///table of float precisions for each column
};

