/****************************************************************************

    xflr5 v6
    Copyright (C) André Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>


#include "gridsettingsdlg.h"
#include <xflwidgets/line/linemenu.h>
#include <xflwidgets/line/linebtn.h>
#include <xflwidgets/customwts/doubleedit.h>
#include <xflcore/units.h>

QByteArray GridSettingsDlg::s_WindowGeometry;


GridSettingsDlg::GridSettingsDlg(QWidget *pParent): QDialog(pParent)
{
    setWindowTitle(tr("Grid options"));
	m_bWithUnit = true;
	setupLayout();
}


void GridSettingsDlg::hideEvent(QHideEvent *)
{
    s_WindowGeometry = saveGeometry();
}


void GridSettingsDlg::showEvent(QShowEvent *)
{
    restoreGeometry(s_WindowGeometry);
}


void GridSettingsDlg::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
		case Qt::Key_Escape:
		{
			done(0);
			break;
		}
		case Qt::Key_Return:
		case Qt::Key_Enter:
		{
			if(!m_pButtonBox->hasFocus())
			{
				m_pButtonBox->setFocus();
			}
			else
			{
				QDialog::accept();
			}
			break;
		}
		default:
			event->ignore();
	}
}


void GridSettingsDlg::initDialog(Grid const &grid, bool bWithUnit)
{
	m_Grid = grid;
	m_bWithUnit = bWithUnit;

    m_pGridControl->initControls(&m_Grid);
}


void GridSettingsDlg::setupLayout()
{
    m_pGridControl = new GridControl;

	m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	{
		connect(m_pButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButton(QAbstractButton*)));
	}

	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
        pMainLayout->addWidget(m_pGridControl);
		pMainLayout->addWidget(m_pButtonBox);
	}
	setLayout(pMainLayout);
}


void GridSettingsDlg::onButton(QAbstractButton *pButton)
{
	if (m_pButtonBox->button(QDialogButtonBox::Ok) == pButton)
	{
        m_Grid.showYAxis(m_pGridControl->m_pchYAxisShow->isChecked());
		accept();
	}
	else if (m_pButtonBox->button(QDialogButtonBox::Cancel) == pButton)     reject();
	else if (m_pButtonBox->button(QDialogButtonBox::Discard) == pButton)    reject();
}







