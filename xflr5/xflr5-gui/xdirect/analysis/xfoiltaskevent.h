/****************************************************************************

	XFoilTaskEvent Class
	   Copyright (C) 2016-2016 Andre Deperrois adeperrois@xflr5.com

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

#ifndef XFOILTASKEVENT_H
#define XFOILTASKEVENT_H

#include <XFoil.h>
#include <QEvent>
#include <QString>

// Custom event identifier
const QEvent::Type XFOIL_END_TASK_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);
const QEvent::Type XFOIL_END_OPP_EVENT = static_cast<QEvent::Type>(QEvent::User + 2);


class XFoilTaskEvent : public QEvent
{

public:
	XFoilTaskEvent(void * pFoil, void *pPolar): QEvent(XFOIL_END_TASK_EVENT),
		m_pFoil(pFoil),
		m_pPolar(pPolar)
	{
	}

	void * foilPtr() const	{return m_pFoil;}
	void * polarPtr() const	{return m_pPolar;}

private:
	void *m_pFoil=NULL;
	void *m_pPolar=NULL;
};



class XFoilOppEvent : public QEvent
{

public:
	XFoilOppEvent(void * pFoil, void *pPolar, XFoil *pXFoilRef): QEvent(XFOIL_END_OPP_EVENT),
		m_pFoil(pFoil),
		m_pPolar(pPolar)
	{
//		memcpy(&m_XFoil, pXFoilPtr, sizeof(XFoil));
		m_pXFoil = pXFoilRef; /** use the copy constructor and = operator defined implicitly by the compiler */
	}

	~XFoilOppEvent()
	{
		if(m_pXFoil) delete m_pXFoil;
	}

	void * foilPtr() const	{return m_pFoil;}
	void * polarPtr() const	{return m_pPolar;}
	XFoil * XFoilPtr() {return m_pXFoil;}

private:
	void *m_pFoil=NULL;
	void *m_pPolar=NULL;
	XFoil *m_pXFoil=NULL;    /** need to store current XFoil results */
};

#endif // XFOILTASKEVENT_H
