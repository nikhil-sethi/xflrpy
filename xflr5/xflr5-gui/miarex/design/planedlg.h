/****************************************************************************

    PlaneDlg Class
    Copyright (C) 2009-2019 Andre Deperrois

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

#ifndef PLANEDLG_H
#define PLANEDLG_H

#include <QDialog>
#include <QLabel>
#include <QAction>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>

class Plane;
class DoubleEdit;

/** The class to define and edit planes. SUes */

class PlaneDlg : public QDialog
{
    Q_OBJECT
    friend class Miarex;
    friend class MainFrame;
    friend class EditPlaneDlg;

public:
    PlaneDlg(QWidget *parent);
    void initDialog();

    static void setWindowPos(QPoint pos){s_WindowPosition = pos;}

private slots:
    void onOK();
    void onFin();
    void onStab();
    void onBodyCheck();
    void onDefineWing();
    void onDefineStab();
    void onDefineFin();
    void onDefineBody();
    void onDefineBodyObject();
    void onChanged();
    void onDescriptionChanged();
    void onImportWing();
    void onDefaultBody();
    void onPlaneName();
    void onSymFin();
    void onDoubleFin();
    void onBiplane();
    void onDefineWing2();
    void onImportWing2();
    void onImportPlaneBody();
    void onImportXMLBody();
    void onInertia();
    void onButton(QAbstractButton *pButton);

private:
    void setupLayout();
    void setResults();
    void readParams();
    void setParams();
    void keyPressEvent(QKeyEvent *event);
    void reject();

private:

    Plane *m_pPlane;   /**< A pointer to the plane which is currently edited in this dialog window */
    bool m_bChanged;   /**< Set to true whenever the data in the window has been changed */
    bool m_bDescriptionChanged;
    bool m_bAcceptName;


private:
    QLabel *m_pctrlSurf1;
    QLabel *m_pctrlSurf2;
    QLabel *m_pctrlSurf3;
    //    QLabel *m_pctrlVolume;
    QLabel *m_pctrlLen1;
    QLabel *m_pctrlLen2;
    QLabel *m_pctrlLen3;
    QLabel *m_pctrlLen4;
    QLabel *m_pctrlLen5;
    QLabel *m_pctrlLen6;
    QLabel *m_pctrlLen7;
    QLabel *m_pctrlLen8;
    QLabel *m_pctrlLen9;
    QLabel *m_pctrlLen10;
    QLabel *m_pctrlLen11;
    QLabel *m_pctrlLen12;
    QLabel *m_pctrlLen13;
    QLabel *m_pctrlWingSpan;
    QLabel *m_pctrlWingSurface;
    QLabel *m_pctrlStabVolume;
    QLabel *m_pctrlFinSurface;
    QLabel *m_pctrlStabLeverArm;
    QLabel *m_pctrlStabSurface;
    QLabel *m_pctrlPlaneVolume;
    QLabel *m_pctrlVLMTotalPanels;
    DoubleEdit  *m_pctrlXBody;
    DoubleEdit  *m_pctrlZBody;
    DoubleEdit  *m_pctrlXLEFin;
    DoubleEdit  *m_pctrlYLEFin;
    DoubleEdit  *m_pctrlZLEFin;
    DoubleEdit  *m_pctrlZLEStab;
    DoubleEdit  *m_pctrlXLEStab;
    DoubleEdit  *m_pctrlXLEWing;
    DoubleEdit  *m_pctrlZLEWing;
    DoubleEdit  *m_pctrlXLEWing2;
    DoubleEdit  *m_pctrlZLEWing2;
    DoubleEdit  *m_pctrlStabTilt;
    DoubleEdit  *m_pctrlFinTilt;
    DoubleEdit  *m_pctrlWingTilt;
    DoubleEdit  *m_pctrlWingTilt2;
    QLineEdit *m_pctrlPlaneName;
    QTextEdit *m_pctrlPlaneDescription;
    QCheckBox *m_pctrlBiplane;
    QCheckBox *m_pctrlBody;
    QCheckBox *m_pctrlStabCheck;
    QCheckBox *m_pctrlFinCheck;
    QCheckBox *m_pctrlDoubleFin;
    QCheckBox    *m_pctrlSymFin;
    QPushButton *m_pctrlDefineWing;
    QPushButton *m_pctrlImportWing;
    QPushButton *m_pctrlDefineWing2;
    QPushButton *m_pctrlImportWing2;
    QPushButton    *m_pctrlDefineFin;
    QPushButton    *m_pctrlVTail;
    QPushButton    *m_pctrlDefineStab;
    QPushButton *m_pctrlBodyActions;
    QPushButton *m_pctrlPlaneInertia;

    QDialogButtonBox *m_pButtonBox;


    QAction *m_pImportXMLBody, *m_pImportPlaneBody;

    static bool s_bWindowMaximized;
    static QPoint s_WindowPosition;   /**< the position on the client area of the dialog's topleft corner */
    static QSize s_WindowSize;     /**< the window size in the client area */

};

#endif // PLANEDLG_H
