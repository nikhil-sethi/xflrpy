/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QTreeView>



class XflTreeView : public QTreeView
{
    public:
        XflTreeView(QWidget *pParent=nullptr);

    private:
         QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
};


