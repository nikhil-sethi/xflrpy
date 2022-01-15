/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/



#pragma once

#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QSize>
#include <QColor>
#include <QAbstractItemModel>

#include <xflcore/linestyle.h>


class LineCbBox;

class LineDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

    public:
        LineDelegate (LineCbBox *pCbBox = nullptr);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

        void setLineStyle(QVector<LineStyle> const&ls){m_LineStyle = ls;}
        void setLineColor(QColor const &clr);

    private:
        LineCbBox *m_pCbBox; //pointer to the parent LineComboBox
        QSize m_Size;

        QVector<LineStyle> m_LineStyle;
};


