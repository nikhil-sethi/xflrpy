/****************************************************************************

	MainFrame  Class
	Copyright (C) 2008-2016 Andre Deperrois adeperrois@xflr5.com

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

#include "scriptconsole.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>

ScriptConsole::ScriptConsole(QWidget *parent) : QWidget(parent)
{
	setWindowTitle("Script Console");
	m_pEngine = new QScriptEngine(this);
	setMinimumWidth(500);
	setupLayout();
}

void ScriptConsole::setXDirect(void *pXDirect)
{
	m_pXDirect = (QXDirect*)pXDirect;
	QScriptValue scriptXDirect = m_pEngine->newQObject(m_pXDirect);
	QScriptValue global = m_pEngine->globalObject();
	global.setProperty("XDirect", scriptXDirect);
}

void ScriptConsole::setupLayout()
{
	QVBoxLayout *pMainLayout = new QVBoxLayout;
	{
		m_pOutput =  new QTextEdit;
		m_pOutput->setReadOnly(true);
		m_pInput = new QLineEdit;
		pMainLayout->addWidget(m_pOutput);
		pMainLayout->addWidget(m_pInput);
	}
	setLayout(pMainLayout);
}


void ScriptConsole::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	m_pInput->setFocus();
}

void ScriptConsole::keyPressEvent(QKeyEvent *event)
{
//	bool bCtrl = (event->modifiers() & Qt::ControlModifier);

	switch (event->key())
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
		{
			executeCommand();
			break;
		}

		default:
			event->ignore();
			return;
	}
	event->accept();
}



void ScriptConsole::executeCommand()
{
	QString scriptCmd = m_pInput->text();
	m_pInput->clear();
	m_pOutput->append(">"+scriptCmd);

	QScriptValue result = m_pEngine->evaluate(scriptCmd);
	QString markup;
	if(result.isError()) markup.append("<font color = \"red\">");
	markup.append(result.toString());
	if(result.isError()) markup.append("</font>");
	m_pOutput->append(markup);
}
