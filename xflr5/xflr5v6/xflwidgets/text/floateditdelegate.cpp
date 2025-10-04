/****************************************************************************

    FloatEditDelegate Class
    Copyright (C) 2009 Andre Deperrois 

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


#include "floateditdelegate.h"


FloatEditDelegate::FloatEditDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QWidget *FloatEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    if(m_Precision[index.column()]>=0)
    {
        //we have a number
        DoubleEdit *editor = new DoubleEdit(parent);
        editor->setDigits(m_Precision[index.column()]);
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        editor->setValue(value);

        return editor;
    }
    else
    {
        //we have a string
        QLineEdit *editor = new QLineEdit(parent);
        editor->setAlignment(Qt::AlignLeft);
        return editor;
    }
}

void FloatEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(m_Precision[index.column()]>=0)
    {
        double value = index.model()->data(index, Qt::EditRole).toDouble();
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        pDE->setValueNoFormat(value);
    }
    else
    {
        QLineEdit *pLine = static_cast<QLineEdit*>(editor);
        pLine->setText(index.model()->data(index, Qt::EditRole).toString());
    }
}



void FloatEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if(m_Precision[index.column()]>=0)
    {
        DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
        double value = pDE->value();
        model->setData(index, value, Qt::EditRole);
    }
    else
    {
        QLineEdit *pLine = static_cast<QLineEdit*>(editor);
        model->setData(index, pLine->text(), Qt::EditRole);
    }
}


void FloatEditDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    pEditor->setGeometry(option.rect);
}


void FloatEditDelegate::setPrecision(QVector<int> const &Precision)
{
    m_Precision = Precision;
}


void FloatEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strong;
    QStyleOptionViewItem myOption = option;
    if(m_Precision[index.column()]>=0)
    {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        strong = QString("%L1").arg(index.model()->data(index, Qt::DisplayRole).toDouble(),0,'f', m_Precision[index.column()]);

    }
    else
    {
        myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strong = index.model()->data(index, Qt::DisplayRole).toString();
    }
    drawDisplay(painter, myOption, myOption.rect, strong);
    drawFocus(painter, myOption, myOption.rect);
}

