
#include "graph_globals.h"

void drawPoint(QPainter &painter, int pointStyle, QPoint pt, QColor backColor)
{
	QBrush backBrush(backColor);
	painter.setBrush(backBrush);
	switch(pointStyle)
	{
		case 0: break;
		case 1:
		{
			int ptSide = 2;
			painter.drawEllipse(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 2:
		{
			int ptSide = 4;
			painter.drawEllipse(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 3:
		{
			int ptSide = 2;
			painter.drawRect(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		case 4:
		{
			int ptSide = 4;
			painter.drawRect(pt.x()-ptSide, pt.y()-ptSide, 2*ptSide, 2*ptSide );
			break;
		}
		default: break;
	}
}



/**
* Returns the index of a Qt-style based on the index of the style in the array
*@param s the index of the style
*@return The index of the Qt-style
*/
Qt::PenStyle getStyle(int s)
{
	if(s==0)      return Qt::SolidLine;
	else if(s==1) return Qt::DashLine;
	else if(s==2) return Qt::DotLine;
	else if(s==3) return Qt::DashDotLine;
	else if(s==4) return Qt::DashDotDotLine;
	return Qt::SolidLine;
}


