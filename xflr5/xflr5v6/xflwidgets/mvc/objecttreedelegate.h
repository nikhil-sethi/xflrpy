/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/

#pragma once

#include <QItemDelegate>
#include <xflcore/fontstruct.h>

class ObjectTreeDelegate : public QItemDelegate
{
    public:
        ObjectTreeDelegate(QObject *pParent);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void setPaintStyle(bool bPaint) {m_bPaintStyle=bPaint;}

        bool m_bPaintStyle;

        static void setTreeFontStruct(FontStruct const &treefontstruct) {s_TreeFontStruct=treefontstruct;}

    protected:
        static FontStruct s_TreeFontStruct;
};







