/****************************************************************************

	DirectDesignView Class
	Copyright (C) 2015 Andre Deperrois 

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

#ifndef DIRECT2DDESIGN_H
#define DIRECT2DDESIGN_H

#include "section2dwidget.h"
#include <gui_objects/SplineFoil.h>
#include <objects/objects2d/Foil.h>

class Direct2dDesign : public Section2dWidget
{
public:
	Direct2dDesign(QWidget *pParent=NULL);

	void setObjects(Foil *pBufferFoil, SplineFoil *pSF, QList<Foil *> *poaFoil);

	void setScale();
	void paintEvent(QPaintEvent *event);
	void resizeEvent (QResizeEvent *event);

	int highlightPoint(Vector3d real);
	int selectPoint(Vector3d real);
	void dragSelectedPoint(double x, double y);

	void setNeutralLineColor(QColor clr){m_NeutralColor = clr;}
	QColor neutralLineColor(){return m_NeutralColor;}

private slots:
	void onInsertPt();
	void onRemovePt();

private:
	void paintSplines(QPainter &painter);
	void paintFoils(QPainter &painter);
	void paintLegend(QPainter &painter);
	void paintLECircle(QPainter &painter);


private:
	QList<Foil*> *m_poaFoil;   /**< a pointer to the array of Foil objects */
	SplineFoil *m_pSF;          /**< a pointer to the SplineFoil object */
	Foil *m_pBufferFoil;

public:
	bool m_bLECircle;           /**< true if the leading edge circle should be displayed >*/
	double m_LERad;             /**< the radius of the leading edge circle to draw >*/

};

#endif // DIRECT2DDESIGN_H
