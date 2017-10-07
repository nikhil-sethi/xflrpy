/****************************************************************************

	FoilTableDelegate Class
	Copyright (C) 2009-2016 Andre Deperrois adeperrois@xflr5.com

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

#include <QApplication>

#include "FoilTableDelegate.h"
#include "AFoil.h"
#include <xdirect/objects2d.h>
#include <globals.h>
#include <graph_globals.h>
#include <misc/Settings.h>



FoilTableDelegate::FoilTableDelegate(QObject *pParent)
 : QItemDelegate(pParent)
{
	m_pManageFoils = NULL;
	m_pAFoil = NULL;
}


QWidget *FoilTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & index ) const
{
	return NULL;//No edition possible - display only

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
		editor->setPrecision(m_Precision[index.column()]);
		return editor;
	}

	return NULL;
}


bool FoilTableDelegate::editorEvent(QEvent *event, QAbstractItemModel *pModel, const QStyleOptionViewItem &option,
							 const QModelIndex &index)
{
//	if(index.column()<12) return false;
	if(m_pAFoil)
	{
		QAFoil *pAFoil = (QAFoil*)m_pAFoil;
		QMouseEvent *pEvent = (QMouseEvent*)event;
		if(pEvent->buttons() & Qt::LeftButton) pAFoil->onFoilClicked(index);
		event->accept();
		return true;
	}
	else return QItemDelegate::editorEvent(event, pModel, option, index);
}


void FoilTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QString strong;
	QStyleOptionViewItem myOption = option;
	QAFoil *pAFoil = (QAFoil*)m_pAFoil;
	int NFoils = Objects2D::s_oaFoil.size();

	if(index.row()> NFoils)
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
	else if(index.column()==5)
	{
		myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		strong = QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toInt());
		drawDisplay(painter, myOption, myOption.rect, strong);
		drawFocus(painter, myOption, myOption.rect);
	}
	else if(index.column()==6 || index.column()==9)
	{
		myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		strong = QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble(), 0,'f',m_Precision[index.column()]);
		drawDisplay(painter, myOption, myOption.rect, strong);
		drawFocus(painter, myOption, myOption.rect);
	}
	else if(index.column()==12)
	{
		if(pAFoil)
		{
			if(index.row()==0)	drawCheckBox(painter, myOption.rect, pAFoil->m_pSF->m_bVisible);
			else
			{
				Foil *pFoil = Objects2D::s_oaFoil.at(index.row()-1);
				drawCheckBox(painter, myOption.rect, pFoil->isVisible());
			}
		}
	}
	else if(index.column()==13)
	{
		if(pAFoil)
		{
			if(index.row()==0)	drawCheckBox(painter, myOption.rect, pAFoil->m_pSF->m_bCenterLine);
			else
			{
				Foil *pFoil = Objects2D::s_oaFoil.at(index.row()-1);
				drawCheckBox(painter, myOption.rect, pFoil->showCenterLine());
			}
		}
	}
	else if(index.column()==14)
	{
		if(pAFoil)
		{
			QColor color;
			int pointStyle, lineStyle, lineWidth;


			if(index.row()==0)
			{
				color = pAFoil->m_pSF->splineFoilColor();
				pointStyle = pAFoil->m_pSF->splinePointStyle();
				lineStyle = pAFoil->m_pSF->splineFoilStyle();
				lineWidth = pAFoil->m_pSF->splineFoilWidth();
			}
			else
			{
				Foil *pFoil = Objects2D::s_oaFoil.at(index.row()-1);
				color = colour(pFoil);
				pointStyle = pFoil->foilPointStyle();
				lineStyle = pFoil->foilLineStyle();
				lineWidth = pFoil->foilLineWidth();
			}
			QRect r = option.rect;
			r = pAFoil->m_pctrlFoilTable->visualRect(index);;

			painter->save();

			QPen LinePen(color);
			LinePen.setStyle(getStyle(lineStyle));
			LinePen.setWidth(lineWidth);
			painter->setPen(LinePen);
			painter->drawLine(r.left()+5, r.top()+r.height()/2, r.right()-5, r.top()+r.height()/2);

			LinePen.setStyle(Qt::SolidLine);
			painter->setPen(LinePen);
			QColor backColor;
			if (option.state & QStyle::State_Selected) backColor = option.palette.highlight().color();
			else                                       backColor = option.palette.base().color();

			drawPoint(*painter, pointStyle, backColor, r.center());

			painter->restore();
		}
//		drawFocus(painter, myOption, myOption.rect);
	}
	else
	{
		myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		strong = QString("%1").arg(index.model()->data(index, Qt::DisplayRole).toDouble()*100.0, 0,'f', m_Precision[index.column()]);
		drawDisplay(painter, myOption, myOption.rect, strong);
		drawFocus(painter, myOption, myOption.rect);
	}
}



void FoilTableDelegate::drawCheckBox(QPainter *painter, QRect const & r, bool bChecked) const
{
	QFontMetrics fm(Settings::s_TextFont);
	int h23 = (int)((double)fm.height()*3./5.);//pixels
	int h3 = (int)((double)fm.height()/3.5);//pixels
	double h4 = (double)fm.height()/5.0;

	QPoint center = r.center();

	QRect sR2 = QRect(center.x()-h23, center.y()-h23, 2*h23, 2*h23);
	QRect sR3 = QRect(center.x()-h3, center.y()-h3, 2*h3, 2*h3);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setBrush(QApplication::palette().button());
	painter->setBackgroundMode(Qt::TransparentMode);

	QPen checkPen;

	QColor drawClr = QApplication::palette().color(QPalette::Button);
	QColor textClr = QApplication::palette().color(QPalette::ButtonText);

	checkPen.setColor(drawClr);
	checkPen.setWidth(1);
	painter->setPen(checkPen);

	painter->drawRoundedRect(sR2, h4, h4);

	checkPen.setColor(textClr);
	checkPen.setWidth(2);
	painter->setPen(checkPen);

	if(bChecked)
	{
		painter->drawLine(sR3.left(), sR3.bottom(), sR3.right(), sR3.top());
		painter->drawLine(sR3.left(), sR3.top(),    sR3.right(), sR3.bottom());
	}
	else
	{

	}
	painter->restore();
}


void FoilTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
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


void FoilTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
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
        double value = pDE->value()/100.0;
		model->setData(index, value, Qt::EditRole);
	}
}


void FoilTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

