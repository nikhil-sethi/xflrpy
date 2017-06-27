#ifndef SCRIPTCONSOLE_H
#define SCRIPTCONSOLE_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QKeyEvent>
#include <QScriptEngine>
#include <xdirect/XDirect.h>


class ScriptConsole : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptConsole(QWidget *parent = 0);

signals:

public slots:

public:
	void setXDirect(void *pXDirect);

protected:
	void keyPressEvent(QKeyEvent *event);
	void showEvent(QShowEvent *event);

private:
	void setupLayout();
	void executeCommand();

	QTextEdit *m_pOutput;
	QLineEdit *m_pInput;

	QScriptEngine *m_pEngine;
	QXDirect *m_pXDirect;
};

#endif // SCRIPTCONSOLE_H
