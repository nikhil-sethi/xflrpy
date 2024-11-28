/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include  <QStandardItemModel>

class CtrlTableModel: public QStandardItemModel
{
public:
    CtrlTableModel(QObject *parent=nullptr) : QStandardItemModel(parent)  { }

    Qt::ItemFlags flags(const QModelIndex & index) const
    {
        if (index.column() == 0)
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        else
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
};
