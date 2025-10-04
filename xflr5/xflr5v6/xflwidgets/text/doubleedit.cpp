/****************************************************************************

    DoubleEdit Class
    Copyright (C) 2014 Andre Deperrois 

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

#include <xflwidgets/text/doubleedit.h>


#include <QFontMetrics>
#include <cmath>

DoubleEdit::DoubleEdit(QWidget *pParent)  : NumEdit(pParent)
{
    setParent(pParent);
    initialize(0.0, -1);
}


DoubleEdit::DoubleEdit(double val, int decimals, QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(val, decimals);
}


void DoubleEdit::initialize(double value, int decimals)
{
//    setAutoFillBackground(true);
    m_Value = value;
    m_Notation = QDoubleValidator::StandardNotation;
    setRange(-1.e10, 1.e10);
    m_Digits = decimals;

    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
    formatValue();
}


void DoubleEdit::readValue()
{
    QString strange=text().trimmed();

/*    strange = "   language:          "+QLocale::languageToString(locale().language());
    strange = "   decimal separator: "+QString(locale().decimalPoint());
    strange = "   group separator:   "+QString(locale().groupSeparator());
*/

    bool bOK=true;
    double val = strange.toDouble(&bOK);
    val = locale().toDouble(strange, &bOK);

    if(bOK)
    {
        bool bValueChanged = fabs(m_Value-val)>1.e-10;
        m_Value = val;

        if(bValueChanged) emit valueChanged();
    }
}


void DoubleEdit::setValue(double val)
{
    m_Value = val;
    formatValue();
}


void DoubleEdit::setValuef(float val)
{
    m_Value = double(val);
    formatValue();
}


void DoubleEdit::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            readValue();
            formatValue();

            QLineEdit::keyPressEvent(pEvent);
            break;
        }
        case Qt::Key_Escape:
        {
            formatValue();
            QLineEdit::keyPressEvent(pEvent);
            break;
        }
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_E:
        case Qt::Key_Comma:
        case Qt::Key_Period:
        case Qt::Key_Minus:
        case Qt::Key_Plus:
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_End:
        case Qt::Key_Home:
        case Qt::Key_Tab:
        {
            NumEdit::keyPressEvent(pEvent);
//            readValue();
            break;
        }
        default:
        {
            NumEdit::keyPressEvent(pEvent);
            pEvent->ignore();
            break;
        }
    }
}

/**
 * Checks that the value is within bounds and formats it
 */
void DoubleEdit::formatValue()
{
    if(m_Value<m_MinValue) m_Value=m_MinValue;
    if(m_Value>m_MaxValue) m_Value=m_MaxValue;

    QString str;

    QString format = "%L1";
    if(m_Digits<0)
    {
        str=QString(format).arg(m_Value, 0, 'g');
    }
    else
    {
        if ((fabs(m_Value)<=1.e-30 || fabs(m_Value)>=pow(10.0, -m_Digits)) && m_Value <10000000.0)
        {
            str=QString(format).arg(m_Value,0,'f', m_Digits);
        }
        else
        {
            str=QString(format).arg(m_Value, 0,'g', m_Digits+1);
        }
    }

    setText(str);
}


void DoubleEdit::setValueNoFormat(double val)
{
    m_Value = val;
}


