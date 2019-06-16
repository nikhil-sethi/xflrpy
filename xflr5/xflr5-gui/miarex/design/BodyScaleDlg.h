/****************************************************************************

    BodyScaleDlg Class
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


#ifndef BODYSCALEDLG_H
#define BODYSCALEDLG_H

#include <QDialog>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>

class DoubleEdit;
class GL3dBodyDlg;

class BodyScaleDlg : public QDialog
{
    Q_OBJECT

    friend class GL3dBodyDlg;
    friend class EditBodyDlg;
    friend class Body;
    friend class BodyFrameWt;
    friend class BodyLineWt;

public:
    BodyScaleDlg(QWidget *pParent);

private slots:
    void onOK();
    void onRadio();

private:
    void setupLayout();
    void initDialog(bool bFrameOnly=false);
    void enableControls();
    void keyPressEvent(QKeyEvent *event);

private:

    QPushButton *m_pOKButton, *m_pCancelButton;
    QRadioButton *m_pctrlBody, *m_pctrlFrame;
    DoubleEdit *m_pctrlXScaleFactor;
    DoubleEdit *m_pctrlYScaleFactor;
    DoubleEdit *m_pctrlZScaleFactor;
    DoubleEdit *m_pctrlFrameID;


private:

    double m_XFactor, m_YFactor, m_ZFactor;
    bool m_bFrameOnly;
    int m_FrameID;
};

#endif // BODYGRIDDLG_H
