/****************************************************************************

    IntEdit Class
    Copyright (C) 2013 Andre Deperrois

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

#include <xflwidgets/text/intedit.h>
#include <misc/options/settings.h>


IntEdit::IntEdit(QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(0);
}


IntEdit::IntEdit(int val, QWidget *pParent) : NumEdit(pParent)
{
    setParent(pParent);
    initialize(val);
}


void IntEdit::initialize(int value)
{
    m_Value = value;
    m_pIV = new QIntValidator(this);
    setValidator(m_pIV);
    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
    formatValue();
}


void IntEdit::readValue()
{
    bool bOK=false;
    QString str = text().trimmed();
    int val = locale().toInt(str, &bOK);

    if(bOK)
    {
        bool bValueChanged = abs(m_Value-val)>0;
        m_Value = val;
        if(bValueChanged) emit valueChanged();
    }
}


void IntEdit::setValue(int val)
{
    m_Value = val;
    formatValue();
}


void IntEdit::keyPressEvent(QKeyEvent *pEvent)
{
    switch (pEvent->key())
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
            readValue();
            formatValue();
            break;
        }
        case Qt::Key_Escape:
        {
            formatValue();
            break;
        }
        default:
        {
            break;
        }
    }
    NumEdit::keyPressEvent(pEvent);
}


void IntEdit::formatValue()
{
    QString str = QString("%L1").arg(m_Value);
    setText(str);
}


void IntEdit::setValueNoFormat(int val)
{
    m_Value = val;
}

