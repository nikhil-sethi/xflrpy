/****************************************************************************

    UFOTableDelegate Class
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


#include "planetabledelegate.h"
#include <xflobjects/objects3d/objects3d.h>
#include <xflwidgets/customwts/doubleedit.h>


PlaneTableDelegate::PlaneTableDelegate(QObject *parent)
 : QItemDelegate(parent)
{
}


QWidget *PlaneTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    return nullptr;//No edition possible - display only

    if(index.column()==0)
    {
        QLineEdit *editor = new QLineEdit(parent);
        editor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        return editor;
    }
    else
    {
        DoubleEdit *editor = new DoubleEdit(parent);
        editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        editor->setDigits(m_Precision[index.column()]);
        return editor;
    }

    return nullptr;
}


bool PlaneTableDelegate::editorEvent(QEvent *, QAbstractItemModel *, const QStyleOptionViewItem &, const QModelIndex &)
{
    return false;//don't edit anything!

}


void PlaneTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strong;
    QStyleOptionViewItem myOption = option;
    int NUFOs  = Objects3d::planeCount();

    if(index.row()> NUFOs)
    {
        strong=" ";
        drawDisplay(painter, myOption, myOption.rect, strong);
        drawFocus(painter, myOption, myOption.rect);
    }
    else if(index.column()==0)
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
        drawDisplay(painter, myOption, myOption.rect, strong);
        drawFocus(painter, myOption, myOption.rect);
    }
    else
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        strong = QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble(), 0,'f',m_Precision[index.column()]);
        drawDisplay(painter, myOption, myOption.rect, strong);
        drawFocus(painter, myOption, myOption.rect);
    }
}



void PlaneTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = (QLineEdit*)editor;
        lineEdit->setText(strong);
    }
    else
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        pDE->setValue(value);
    }
}


void PlaneTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(index.column()==0)
    {
        QString strong;
        QLineEdit *pLineEdit = static_cast<QLineEdit*>(editor);
        strong = pLineEdit->text();
        model->setData(index, strong, Qt::EditRole);
    }
    else
    {
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        double value = pDE->value();
        model->setData(index, value, Qt::EditRole);
    }
}


void PlaneTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

