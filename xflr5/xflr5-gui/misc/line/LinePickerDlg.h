/****************************************************************************

    LinePicker Class
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

#ifndef LINEPICKERDLG_H
#define LINEPICKERDLG_H

#include <QDialog>
#include <QComboBox>


class LineBtn;
class LineCbBox;
class LineDelegate;

class LinePickerDlg : public QDialog
{
    Q_OBJECT

public:
    LinePickerDlg(QWidget *pParent);

    void initDialog();
    void initDialog(int pointStyle, int lineStyle, int lineWidth, QColor lineColor, bool bAcceptPointStyle=false);

    void keyPressEvent(QKeyEvent *event);

    int &pointStyle();
    int &lineStyle();
    int &lineWidth();
    QColor &lineColor();

    void setPointStyle(int pointStyle);
    void setLineStyle(int lineStyle);
    void setLineWidth(int width);
    void setLineColor(QColor color);

    void fillBoxes();
    void setupLayout();


protected:
    LineBtn *m_pctrlLineColor;
    LineCbBox *m_pctrlPointStyle, *m_pctrlLineWidth, *m_pctrlLineStyle;
    QPushButton *OKButton, *CancelButton;

private:
    bool m_bAcceptPointStyle;
    int m_PointStyle;
    int m_LineStyle;
    int m_LineWidth;
    QColor m_LineColor;
    LineDelegate *m_pPointStyleDelegate, *m_pLineStyleDelegate, *m_pWidthDelegate;


private slots:
    void onPointStyle(int val);
    void onLineStyle(int val);
    void onLineWidth(int val);
    void onLineColor();

};

#endif // LINEPICKERDLG_H
