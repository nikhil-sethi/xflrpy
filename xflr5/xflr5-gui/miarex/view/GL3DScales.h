/****************************************************************************

	GL3DScales Class
	Copyright (C) 2009-2016 Andre Deperrois 
	
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

#ifndef GL3DSCALES_H
#define GL3DSCALES_H

#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>
#include <misc/exponentialslider.h>
#include <misc/text/DoubleEdit.h>

class GL3DScales : public QWidget
{
	Q_OBJECT
	friend class MainFrame;
	friend class Miarex;

public:
	GL3DScales(QWidget *);
	void initDialog();

private slots:
	void onCpScale();
	void onApply();
	void onLiftEdit();
	void onDragEdit();
	void onVelocityEdit();
	void onLiftScale();
	void onDragScale();
	void onVelocityScale();


private:
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
	void keyPressEvent(QKeyEvent *event);

	void setupLayout();
	void readStreamParams();

	static bool loadSettings(QSettings *pSettings);
	static bool saveSettings(QSettings *pSettings);

	ExponentialSlider *m_pctrlLiftScaleSlider, *m_pctrlDragScaleSlider, *m_pctrlVelocityScaleSlider;
	DoubleEdit *m_pctrlLiftScale, *m_pctrlDragScale, *m_pctrlVelocityScale;
	QPushButton *ApplyButton;
	QCheckBox *m_pctrlAutoCpScale;
	DoubleEdit	*m_pctrlLegendMin, *m_pctrlLegendMax;

	DoubleEdit *m_pctrlNXPoint, *m_pctrlDeltaL, *m_pctrlXFactor, *m_pctrlXOffset, *m_pctrlZOffset;
	QRadioButton *m_pctrlLE, *m_pctrlTE, *m_pctrlLine;

	QLabel *m_pctrlLengthUnit1, *m_pctrlLengthUnit2, *m_pctrlLengthUnit3;

	static void *s_pMiarex;

	QWidget *m_pParent;
	
public:
	static int s_pos;
	static int s_NX;
	static double s_DeltaL;
	static double s_XFactor;
	static double s_XOffset, s_ZOffset;
};

#endif // GL3DSCALES_H
