/****************************************************************************

	BodyTableDelegate Class
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
#include <misc/text/DoubleEdit.h>


#include "BodyTableDelegate.h"


BodyTableDelegate::BodyTableDelegate(QObject *parent)
 : QItemDelegate(parent)
{
	m_pNRows = nullptr;
}


QWidget *BodyTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
    DoubleEdit *editor = new DoubleEdit(parent);
	editor->setAlignment(Qt::AlignRight);

	editor->setPrecision(m_Precision[index.column()]);
	return editor;
}


void BodyTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	double value = index.model()->data(index, Qt::EditRole).toDouble();
    DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
    pDE->setValue(value);
}


void BodyTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    DoubleEdit *pDE = static_cast<DoubleEdit*>(editor);
    double value = pDE->value();
	model->setData(index, value, Qt::EditRole);
}


void BodyTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QString strong;
	QStyleOptionViewItem myOption = option;
	myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
	strong = QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble(),0,'f', m_Precision[index.column()]);

//	if(index.row()> *m_pNRows) strong=" ";
	drawDisplay(painter, myOption, myOption.rect, strong);
	drawFocus(painter, myOption, myOption.rect);
}


void BodyTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}


void BodyTableDelegate::setPointers(int *PrecisionTable, int *pNRows)
{
	m_Precision = PrecisionTable;
	m_pNRows = pNRows;
}



void BodyTableDelegate::setPointer(int *PrecisionTable)
{
	m_Precision = PrecisionTable;
}











