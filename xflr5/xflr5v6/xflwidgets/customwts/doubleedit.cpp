/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois 
    GNU General Public License v3

*****************************************************************************/


#include <cmath>

#include <QFontMetrics>

#include <xflcore/xflcore.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflwidgets/wt_globals.h>


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
    m_Value = value;
    m_Notation = QDoubleValidator::StandardNotation;
    setRange(-1.e10, 1.e10);
    m_Digits = decimals;

    setAlignment(Qt::AlignRight);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum));
}


void DoubleEdit::readValue()
{
    QString strange=text().trimmed();

    bool bOK=true;
    double val = strange.toDouble(&bOK);
    if(xfl::g_bLocalize)
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

    QString str = formatDouble(m_Value, m_Digits, xfl::g_bLocalize);
    setText(str);
}


void DoubleEdit::setValueNoFormat(double val)
{
    m_Value = val;
}


