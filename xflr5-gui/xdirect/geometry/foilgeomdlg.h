/****************************************************************************

    FoilGeomDlg Class
    Copyright (C) 2008 Andre Deperrois 

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


#ifndef FOILGEOMDLG_H
#define FOILGEOMDLG_H

#include <QDialog>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class XFoil;
class Foil;
class DoubleEdit;
class FoilGeomDlg : public QDialog
{
    Q_OBJECT
    friend class AFoil;
    friend class XDirect;
    friend class MainFrame;

public:
    FoilGeomDlg(QWidget *pParent);
    void initDialog();

private slots:
    void onRestore();
    void onOK();

    void onCamberSlide(int pos);
    void onXCamberSlide(int pos);
    void onThickSlide(int pos);
    void onXThickSlide(int pos);
    void onCamber();
    void onXCamber();
    void onThickness();
    void onXThickness();


private:
    void keyPressEvent(QKeyEvent *event);
    void setupLayout();
    void apply();

private:
    QSlider    *m_pctrlCamberSlide, *m_pctrlThickSlide, *m_pctrlXThickSlide, *m_pctrlXCamberSlide;
    DoubleEdit *m_pctrlXCamber;
    DoubleEdit    *m_pctrlXThickness;
    DoubleEdit    *m_pctrlThickness;
    DoubleEdit    *m_pctrlCamber;

    QPushButton *m_OKButton, *m_CancelButton, *m_pRestoreButton;


private:
    static XFoil* s_pXFoil;

    double m_fCamber;
    double m_fThickness;
    double m_fXCamber;
    double m_fXThickness;
    Foil* m_pBufferFoil;
    Foil* m_pMemFoil;

    QWidget *m_pParent;

    bool  m_bApplied,m_bAppliedX, m_bModified;

};

#endif // FOILGEOMDLG_H
