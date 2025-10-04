/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include <QApplication>

#include "ctrltabledelegate.h"

#include <xflcore/xflcore.h>
#include <xflwidgets/wt_globals.h>

CtrlTableDelegate::CtrlTableDelegate(QObject *parent) : QItemDelegate(parent)
{
}


QWidget *CtrlTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
	if(index.column()==0)
	{
		QLineEdit *pEditor = new QLineEdit(parent);
		pEditor->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		return pEditor;
	}
	else if(index.column()==1 || index.column()==2)
	{
        if(index.column()<m_bEditable.size() && m_bEditable.at(index.column()))
        {
            DoubleEdit *pEditor = new DoubleEdit(parent);
            pEditor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            pEditor->setDigits(m_Precision[index.column()]);
            return pEditor;
        }
        else return nullptr;
	}

	return nullptr;
}


void CtrlTableDelegate::setEditorData(QWidget *pEditor, const QModelIndex &index) const
{
	if(index.column()==0)
	{
		QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *pLineEdit = dynamic_cast<QLineEdit*>(pEditor);
		pLineEdit->setText(strong);
	}
	else if(index.column()==1 || index.column()==2)
	{
		double value = index.model()->data(index, Qt::EditRole).toDouble();
		DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
        pDE->setValue(value);
	}
	else if(index.column()==3)
	{
		QString strong = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = dynamic_cast<QLineEdit*>(pEditor);
		lineEdit->setText(strong);
	}
}


void CtrlTableDelegate::setModelData(QWidget *pEditor, QAbstractItemModel *pAbstractItemModel, const QModelIndex &index) const
{
	if(index.column()==0)
	{
		QString strong;
		QLineEdit *pLineEdit = static_cast<QLineEdit*>(pEditor);
		strong = pLineEdit->text();

		pAbstractItemModel->setData(index, strong, Qt::EditRole);
	}
	else if(index.column()==1 || index.column()==2)
	{
		DoubleEdit *pDE = static_cast<DoubleEdit*>(pEditor);
        pDE->readValue();
        pAbstractItemModel->setData(index, pDE->value(), Qt::EditRole);
	}
	else if(index.column()==3)
	{
/*		QString strong;
		QLineEdit *pLineEdit = static_cast<QLineEdit*>(editor);
		strong = pLineEdit->text();
		model->setData(index, strong, Qt::EditRole);*/
	}
}
 

void CtrlTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString strange;
//	int NCtrls = 2;
	QStyleOptionViewItem myOption = option;

	int col = index.column();

	if(col==0)
	{
		myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strange = index.model()->data(index, Qt::DisplayRole).toString();
	}
	else if(col==1 || col==2)
	{
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        double dble = index.model()->data(index, Qt::DisplayRole).toDouble();
        if(xfl::g_bLocalize)
            strange = QString("%L1").arg(dble ,0,'f',  m_Precision.at(index.column()));
        else
            strange = QString("%1").arg(dble ,0,'f',  m_Precision.at(index.column()));
    }
	else if(col==3)
	{
		myOption.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        strange = index.model()->data(index, Qt::DisplayRole).toString();
	}
    drawDisplay(painter, myOption, myOption.rect, strange);
    drawFocus(painter, myOption, myOption.rect);
}


void CtrlTableDelegate::updateEditorGeometry(QWidget *pEditor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	pEditor->setGeometry(option.rect);
}


void CtrlTableDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &option, const QRect &, Qt::CheckState state) const
{
	const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;

	QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
										  option.rect.size(),
										  QRect(option.rect.x() + textMargin, option.rect.y(),
												option.rect.width() - (textMargin * 2), option.rect.height()));

    QItemDelegate::drawCheck(painter, option, checkRect, state);
}




bool CtrlTableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
						 const QModelIndex &index)
{
	// make sure that the item is checkable
	Qt::ItemFlags flags = model->flags(index);
	if (!(flags & Qt::ItemIsUserCheckable) || !(flags & Qt::ItemIsEnabled))
		return false;

	// make sure that we have a check state
	QVariant value = index.data(Qt::CheckStateRole);
	if (!value.isValid())
		return false;

	// make sure that we have the right event type
	if (event->type() == QEvent::MouseButtonRelease)
	{
		const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
		QRect checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
											  option.rect.size(),
											  QRect(option.rect.x() + textMargin, option.rect.y(),
													option.rect.width() - (2 * textMargin), option.rect.height()));

		if (!checkRect.contains(static_cast<QMouseEvent*>(event)->pos())) return false;
	}
	else if (event->type() == QEvent::KeyPress)
	{
		if (   static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space
			&& static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
			return false;
	}
	else
	{
		return false;
	}

	Qt::CheckState state = (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked
						? Qt::Unchecked : Qt::Checked);
	return model->setData(index, state, Qt::CheckStateRole);
}
