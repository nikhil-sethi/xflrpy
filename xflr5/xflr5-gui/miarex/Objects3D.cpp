/****************************************************************************

	Objects3D    Copyright (C) 2014-2017 Andre Deperrois adeperrois@xflr5.com

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

#include <miarex/Miarex.h>
#include <misc/RenameDlg.h>

#include "Objects3D.h"
#include <objects3d/Surface.h>
#include <QApplication>
#include <QMessageBox>
#include <QLocale>
#include <QProcess>
#include <QtDebug>


QList <void *> Objects3D::s_oaPlane;
QList <void *> Objects3D::s_oaWPolar;
QList <void *> Objects3D::s_oaPOpp;
QList <void *> Objects3D::s_oaBody;


Objects3D::Objects3D()
{
}




/**
 * Creates local pointers in the classes, for programming convenience.
 */
void Objects3D::setStaticPointers()
{
	QMiarex::m_poaPlane  = &s_oaPlane;
	QMiarex::m_poaWPolar = &s_oaWPolar;
	QMiarex::m_poaPOpp   = &s_oaPOpp;
}



/**
 * If the body is associated to a plane, duplicates the body and attaches it to the Plane
 * Else destroys the Body.
 * Used only to import body Objects from files prior to v6.09.06
 * @param a pointer to the Body object to be inserted
 * @return true if the body is associated to a plane, false otherwise
 */
void Objects3D::addBody(Body *pBody)
{
	for(int ip=0; ip<s_oaPlane.size(); ip++)
	{
		Plane *pPlane = (Plane*)s_oaPlane.at(ip);
		if(pPlane->m_BodyName==pBody->m_BodyName)
		{
			// duplicate the body - create one for each plane
			// no more bodies associated to multiple plane
			Body *planeBody = new Body();
			planeBody->duplicate(pBody);

			//attach it and rename it
			pPlane->setBody(planeBody);
		}
	}
/*	for(int ib=0; ib<m_poaBody->size(); ib++)
	{
		Body *pOldBody = (Body*)m_poaBody->at(ib);
		if(pOldBody==pBody)
		{
			m_poaBody->removeAt(ib);
			delete pBody; //discarded
		}
	} */
}

/** Checks the array for an existing plane with the name in the input
 * @param planeName: the plane's name to check
 * @return true if a plane with the same name already exists, false otherwise
*/
bool Objects3D::planeExists(QString planeName)
{
	Plane *pOldPlane;

	for (int i=0; i<s_oaPlane.size(); i++)
	{
		pOldPlane = (Plane*)s_oaPlane.at(i);
		if (pOldPlane->planeName() == planeName)
		{
			return true;
		}
	}
	return false;
}



/**
 * Inserts a plane in the array of plane objects
 * @param pPlane: the plane to be inserted
 */
Plane* Objects3D::addPlane(Plane *pPlane)
{
	int i,j;
	Plane *pOldPlane;

	for (i=0; i<s_oaPlane.size(); i++)
	{
		pOldPlane = (Plane*)s_oaPlane.at(i);
		if (pOldPlane->planeName() == pPlane->planeName())
		{
			//a plane with this name already exists
			// if its the same plane, just return
			if(pOldPlane==pPlane) return pPlane;

			// if its an old plane with the same name, delete and insert at its place
			deletePlane(pOldPlane);
			s_oaPlane.insert(i, pPlane);
			return pPlane;
		}
	}

	// the plane does not exist, just insert in alphabetical order
	for (j=0; j<s_oaPlane.size(); j++)
	{
		pOldPlane = (Plane*)s_oaPlane.at(j);
		if (pPlane->planeName() < pOldPlane->planeName())
		{
			s_oaPlane.insert(j, pPlane);
			return pPlane;
		}
	}

	//could not be inserted, append
	s_oaPlane.append(pPlane);
	return pPlane;
}



/**
* Duplicates the currently selected plane, if any and opens it for edition
* @param pCurPlane a pointer to the Plane object to duplicate
* @return a pointer to the Plane object which has been created
*/
Plane* Objects3D::duplicatePlane(Plane *pPlane)
{
	if(!pPlane) return NULL;
	Plane* pNewPlane= new Plane;
	pNewPlane->duplicate(pPlane);
	return setModPlane(pNewPlane);
}



void Objects3D::insertPOpp(PlaneOpp *pPOpp)
{
	PlaneOpp *pOldPOpp = NULL;
	bool bIsInserted = false;

	for (int i=0; i<s_oaPOpp.size(); i++)
	{
		pOldPOpp = (PlaneOpp*)s_oaPOpp.at(i);
		if (pPOpp->planeName() == pOldPOpp->planeName())
		{
			if (pPOpp->polarName() == pOldPOpp->polarName())
			{
				if(pPOpp->polarType()<XFLR5::FIXEDAOAPOLAR)
				{
					if(qAbs(pPOpp->alpha() - pOldPOpp->alpha())<0.005)
					{
						//replace existing point
						pPOpp->color()      = pOldPOpp->color();
						pPOpp->style()      = pOldPOpp->style();
						pPOpp->width()      = pOldPOpp->width();
						pPOpp->isVisible()  = pOldPOpp->isVisible();
						pPOpp->points()     = pOldPOpp->points();

						s_oaPOpp.removeAt(i);
						delete pOldPOpp;
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						break;
					}
					else if (pPOpp->alpha() > pOldPOpp->alpha())
					{
						//insert point
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
				}
				else if (pPOpp->polarType()==XFLR5::FIXEDAOAPOLAR)
				{
					if(qAbs(pPOpp->m_QInf - pOldPOpp->m_QInf)<0.1)
					{
						//replace existing point
						pPOpp->color() = pOldPOpp->color();
						pPOpp->style() = pOldPOpp->style();
						pPOpp->width() = pOldPOpp->width();
						pPOpp->isVisible()  = pOldPOpp->isVisible();
						pPOpp->points() = pOldPOpp->points();

						s_oaPOpp.removeAt(i);
						delete pOldPOpp;
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
					else if (pPOpp->m_QInf > pOldPOpp->m_QInf)
					{
						//insert point
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
				}
				else if (pPOpp->polarType()==XFLR5::BETAPOLAR)
				{
					if(qAbs(pPOpp->m_Beta - pOldPOpp->m_Beta)<0.01)
					{
						//replace existing point
						pPOpp->color() = pOldPOpp->color();
						pPOpp->style() = pOldPOpp->style();
						pPOpp->width() = pOldPOpp->width();
						pPOpp->isVisible()  = pOldPOpp->isVisible();
						pPOpp->points() = pOldPOpp->points();

						s_oaPOpp.removeAt(i);
						delete pOldPOpp;
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
					else if (pPOpp->m_Beta > pOldPOpp->m_Beta)
					{
						//insert point
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
				}
				else if(pPOpp->polarType()==XFLR5::STABILITYPOLAR)
				{
					if(qAbs(pPOpp->m_Ctrl - pOldPOpp->m_Ctrl)<0.001)
					{
						//replace existing point
						pPOpp->color() = pOldPOpp->color();
						pPOpp->style() = pOldPOpp->style();
						pPOpp->width() = pOldPOpp->width();
						pPOpp->isVisible()  = pOldPOpp->isVisible();
						pPOpp->points() = pOldPOpp->points();

						s_oaPOpp.removeAt(i);
						delete pOldPOpp;
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
					else if (pPOpp->m_Ctrl > pOldPOpp->m_Ctrl)
					{
						//insert point
						s_oaPOpp.insert(i, pPOpp);
						bIsInserted = true;
						i = s_oaPOpp.size();// to break
					}
				}
			}
		}
	}

	if (!bIsInserted) 	s_oaPOpp.append(pPOpp);
}









/**
* Adds the WPolar pointed by pWPolar to the m_oaWPolar array.
* Inserts it in alphabetical order using the Plane Name and the polar name
*@param pWPolar a pointer to the instance of the CWPolar object to be inserted
*@return a pointer  to the instance of the CWPolar object which has been inserted, or NULL if the operation failed
*/
void Objects3D::addWPolar(WPolar *pWPolar)
{
	int j;
	WPolar *pOldWPlr;

	for (int ip=0; ip<s_oaWPolar.size(); ip++)
	{
		pOldWPlr = (WPolar*)s_oaWPolar.at(ip);
		if (pOldWPlr->polarName()==pWPolar->polarName() && pOldWPlr->planeName()==pWPolar->planeName())
		{
			s_oaWPolar.removeAt(ip);
			delete pOldWPlr;
			s_oaWPolar.insert(ip, pWPolar);
			return;
		}
	}


	//if it doesn't exist, find its place in alphabetical order and insert it
	for (j=0; j<s_oaWPolar.size(); j++)
	{
		pOldWPlr = (WPolar*)s_oaWPolar.at(j);
		//first key is the Plane name
		if(pWPolar->planeName().compare(pOldWPlr->planeName(), Qt::CaseInsensitive)<0)
		{
			s_oaWPolar.insert(j, pWPolar);
			return;
		}
		else if (pWPolar->planeName() == pOldWPlr->planeName())
		{
			// sort by polar name
			if(pWPolar->polarName().compare(pOldWPlr->polarName(), Qt::CaseInsensitive)<0)
			{
				s_oaWPolar.insert(j, pWPolar);
				return;
			}
		}
	}

	s_oaWPolar.append(pWPolar);
}



/**
 * Deletes a Plane object from the array.
 * @param pPlane a pointer to the Plane object to be deleted
 */
void Objects3D::deletePlane(Plane *pPlane)
{
	if(!pPlane || !pPlane->planeName().length()) return;
	Plane *pOldPlane;
	int i;

	deletePlaneResults(pPlane);

	for (i=s_oaPlane.size()-1; i>=0; i--)
	{
		pOldPlane = (Plane*)s_oaPlane.at(i);
		if (pOldPlane == pPlane)
		{
			s_oaPlane.removeAt(i);
			delete pPlane;
			break;
		}
	}
}


/**
 * Deletes the WPolar and its PlaneOpp objects.
 * @param pWPolar a pointer to the WPolar object which will be deleted
 */
void Objects3D::deleteWPolar(WPolar *pWPolar)
{
	//remove and delete its children POpps from the array
	if(!pWPolar)return;

	for (int l=s_oaPOpp.size()-1;l>=0; l--)
	{
		PlaneOpp *pPOpp = (PlaneOpp*)s_oaPOpp.at(l);
		if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->polarName())
		{
			s_oaPOpp.removeAt(l);
			delete pPOpp;
		}
	}

	for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
	{
		WPolar *pOldWPolar = (WPolar*)s_oaWPolar.at(ipb);
		if(pOldWPolar==pWPolar)
		{
			s_oaWPolar.removeAt(ipb);
			delete pWPolar;
			break;
		}
	}
}



/**
 * Deletes the WPolar and PlaneOpp objects associated to the plane.
 * @param pPlane a pointer to the Plane object for which the results will be deleted
 */
void Objects3D::deletePlaneResults(Plane *pPlane, bool bDeletePolars)
{
	if(!pPlane || !pPlane->planeName().length()) return ;
	WPolar* pWPolar;
	PlaneOpp * pPOpp;

	int i;

	//first remove all POpps associated to the plane
	for (i=s_oaPOpp.size()-1; i>=0; i--)
	{
		pPOpp = (PlaneOpp*)s_oaPOpp.at(i);
		if(pPOpp->planeName() == pPlane->planeName())
		{
			s_oaPOpp.removeAt(i);
			delete pPOpp;
		}
	}

	//next delete all WPolars associated to the plane
	for (i=s_oaWPolar.size()-1; i>=0; i--)
	{
		pWPolar = (WPolar*)s_oaWPolar.at(i);
		if (pWPolar->planeName() == pPlane->planeName())
		{
			if(bDeletePolars)
			{
				s_oaWPolar.removeAt(i);
				delete pWPolar;
				pWPolar = NULL;
			}
			else
			{
				pWPolar->clearData();
			}
		}
	}
}




/**
* Returns a pointer to the body with the name BodyName
*@param BodyName the QString holding the name of the body
*@return a pointer to the body with the requested body name, or NULL if none has been found
*/
Body * Objects3D::getBody(QString BodyName)
{
	Body* pBody;
	for (int ib=0; ib<s_oaBody.size(); ib++)
	{
		pBody = (Body*)s_oaBody.at(ib);
		if (pBody->m_BodyName == BodyName) return pBody;
	}

	Plane *pPlane;
	for(int ip=0; ip<s_oaPlane.size(); ip++)
	{
		pPlane = (Plane*)s_oaPlane.at(ip);
		if(pPlane->body())
		{
			if((pPlane->planeName()+"/Body") == BodyName) return pPlane->body();
		}
	}

	return NULL;
}



/**
* Returns a pointer to the OpPoint with the name of the current plane and current WPolar, and which matches the input parameter
* @param x the aoa, of velocity, or control parameter for which the CPOpp object is requested
* @return a pointer to the instance of the CPOpp object, or NULL if non has been found
*/
PlaneOpp * Objects3D::getPlaneOpp(Plane *pPlane, WPolar* pWPolar, double x)
{
	int i;
	if(!pPlane || !pWPolar) return NULL;

	PlaneOpp* pPOpp;

	for (i=0; i<s_oaPOpp.size(); i++)
	{
		pPOpp = (PlaneOpp*)s_oaPOpp.at(i);
		if ((pPOpp->planeName() == pPlane->planeName()) && (pPOpp->polarName() == pWPolar->polarName()))
		{
			if     (pWPolar->polarType()< XFLR5::FIXEDAOAPOLAR  && qAbs(pPOpp->alpha() - x)<0.005)  return pPOpp;
			else if(pWPolar->polarType()==XFLR5::FIXEDAOAPOLAR  && qAbs(pPOpp->m_QInf - x) <0.005)   return pPOpp;
			else if(pWPolar->polarType()==XFLR5::BETAPOLAR      && qAbs(pPOpp->m_Beta - x) <0.005)   return pPOpp;
			else if(pWPolar->polarType()==XFLR5::STABILITYPOLAR && qAbs(pPOpp->m_Ctrl - x) <0.005)   return pPOpp;
		}
	}
	return NULL;
}



/**
* Returns a pointer to the polar with the name of the input parameter
* @param WPolarName the name of the CWPolar object
* @return a pointer to the instance of the CWPolar object, or NULL if non has been found
*/
WPolar* Objects3D::getWPolar(Plane *pPlane, QString WPolarName)
{
	WPolar *pWPolar;
	int i;
	if(!pPlane) return NULL;

	for (i=0; i<s_oaWPolar.size(); i++)
	{
		pWPolar = (WPolar*)s_oaWPolar.at(i);
		if (pWPolar->planeName()==pPlane->planeName() && pWPolar->polarName()== WPolarName)
			return pWPolar;
	}
	return NULL;
}



/**
* Returns a pointer to the plane with the name PlaneName
*@param PlaneName a QString object with the name of the requested CPlane object
*@return a pointer to the instance of the CPlane object, or NULL if non has been found
*/
Plane * Objects3D::getPlane(QString PlaneName)
{
	int i;
	Plane* pPlane;
	for (i=0; i<s_oaPlane.size(); i++)
	{
		pPlane = (Plane*)s_oaPlane.at(i);
		if (pPlane->planeName() == PlaneName) return pPlane;
	}
	return NULL;
}



/**
 * Returns a pointer to the main wing of the plane defined by its name
 * @param PlaneName the source Plane's name
 * @return a pointer to the Plane's main wing.
 */
Wing* Objects3D::getWing(QString PlaneName)
{
	for(int ip=0; ip<s_oaPlane.size(); ip++)
	{
		Plane *pPlane = (Plane*)s_oaPlane.at(ip);
		if(QString::compare(pPlane->planeName(), PlaneName, Qt::CaseInsensitive)==0) return pPlane->m_Wing;
	}
	return NULL;
}



/**
 * Inserts a modified Plane object in the array, i.a.w. user instructions
 * @param pModPlane a pointer to the instance of the Plane object to be inserted
 * @return a pointer to the Plane object which was successfully inserted, false otherwise
 */
Plane * Objects3D::setModPlane(Plane *pModPlane)
{
	if(!pModPlane) return NULL;
	Plane * pPlane;
	bool bExists = true;
	int resp, k, l;

	QString OldName = pModPlane->planeName();

	QStringList NameList;
	for(k=0; k<s_oaPlane.size(); k++)
	{
		pPlane = (Plane*)s_oaPlane.at(k);
		NameList.append(pPlane->planeName());
	}

	RenameDlg renDlg;
	renDlg.initDialog(&NameList, pModPlane->planeName(),QObject::tr("Enter the new name for the Plane :"));

	while (bExists)
	{
		resp = renDlg.exec();
		if(resp==QDialog::Accepted)
		{
			if (OldName == renDlg.newName()) return pModPlane;

			//Is the new name already used ?
			bExists = false;
			for (k=0; k<s_oaPlane.size(); k++)
			{
				pPlane = (Plane*)s_oaPlane.at(k);
				if (pPlane->planeName() == renDlg.newName())
				{
					bExists = true;
					break;
				}
			}

			if(!bExists)
			{
				// we have a valid name
				// rename the plane
				pModPlane->setPlaneName(renDlg.newName());

				bool bInserted = false;
				for (l=0; l<s_oaPlane.size();l++)
				{
					pPlane = (Plane*)s_oaPlane.at(l);
					if(pPlane == pModPlane)
					{
						// remove the current Plane from the array
						s_oaPlane.removeAt(l);
						// but don't delete it !
						break;
					}
				}
				//and re-insert it
				for (l=0; l<s_oaPlane.size();l++)
				{
					pPlane = (Plane*)s_oaPlane.at(l);
					if(pPlane->planeName().compare(pModPlane->planeName(), Qt::CaseInsensitive) >0)
					{
						//then insert before
						s_oaPlane.insert(l, pModPlane);
						bInserted = true;
						break;
					}
				}
				if(!bInserted)	s_oaPlane.append(pModPlane);
				break;

			}
		}
		else if(resp ==10)
		{
			//the user wants to overwrite the old plane/wing

			pPlane = getPlane(renDlg.newName());
			deletePlane(pPlane);

			pModPlane->setPlaneName(renDlg.newName());

			//place the Plane in alphabetical order in the array
			//remove the current Plane from the array
			for (l=0; l<s_oaPlane.size();l++)
			{
				pPlane = (Plane*)s_oaPlane.at(l);
				if(pPlane == pModPlane)
				{
					s_oaPlane.removeAt(l);
					// but don't delete it !
					break;
				}
			}
			//and re-insert it
			bool bInserted = false;
			for (l=0; l<s_oaPlane.size();l++)
			{
				pPlane = (Plane*)s_oaPlane.at(l);
				if(pPlane->planeName().compare(pModPlane->planeName(), Qt::CaseInsensitive) <0)
				{
					//then insert before
					s_oaPlane.insert(l, pModPlane);
					bInserted = true;
					break;
				}
			}
			if(!bInserted) s_oaPlane.append(pModPlane);
			bExists = false;
		}
		else
		{
			return NULL;//cancelled
		}
	}
	return pModPlane;
}



/**
 * Inserts a new WPolar object in the array.
 * If the WPolar's name already exists, finds a new one, eventually by overwriting an old WPolar.
 * @param pModWPolar a pointer to the instance of the WPolar object to be inserted
 * @return a pointer to the polar which has been set, or NULL if failure
 */
WPolar* Objects3D::insertNewWPolar(WPolar *pNewWPolar, Plane *pCurPlane)
{
	if(!pNewWPolar) return NULL;
	WPolar *pWPolar, *pOldWPolar;

	bool bExists = true;
	int resp, k, l;

	//check if this WPolar is already inserted
	for(int ip=0; ip<s_oaWPolar.size(); ip++)
	{
		pOldWPolar = (WPolar*)s_oaWPolar.at(ip);
		if(pOldWPolar==pNewWPolar)
		{
//			Trace("this WPolar is already in the array, nothing inserted");
			return NULL;
		}
	}

	//make a list of existing names
	QStringList NameList;
	for(k=0; k<s_oaWPolar.size(); k++)
	{
		pWPolar = (WPolar*)s_oaWPolar.at(k);
		if(pCurPlane && pWPolar->planeName()==pCurPlane->planeName()) NameList.append(pWPolar->polarName());
	}

	//Is the new WPolar's name already used ?
	bExists = false;
	for (k=0; k<NameList.count(); k++)
	{
		if(pNewWPolar->polarName()==NameList.at(k))
		{
			bExists = true;
			break;
		}
	}

	if(!bExists)
	{
		//just insert the WPolar in alphabetical order
		for (l=0; l<s_oaWPolar.size();l++)
		{
			pOldWPolar = (WPolar*)s_oaWPolar.at(l);

			if(pOldWPolar->polarName().compare(pNewWPolar->polarName(), Qt::CaseInsensitive) >0)
			{
				//then insert before
				s_oaWPolar.insert(l, pNewWPolar);
				return pNewWPolar;
			}
		}
		//not inserted, append
		s_oaWPolar.append(pNewWPolar);
		return pNewWPolar;
	}

	// an old object with the WPolar's name exists for this Plane, ask for a new one
	RenameDlg dlg;
	dlg.initDialog(&NameList, pNewWPolar->polarName(), QObject::tr("Enter the new name for the Polar:"));
	resp = dlg.exec();

	if(resp==10)
	{
		//user wants to overwrite an existing name
		//so find the existing WPolar with that name
		pWPolar = NULL;
		for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
		{
			pOldWPolar = (WPolar*)s_oaWPolar.at(ipb);
			if(pCurPlane && pOldWPolar->polarName()==pNewWPolar->polarName() && pOldWPolar->planeName()==pCurPlane->planeName())
			{
				pWPolar = pOldWPolar;
				break;
			}
		}

		if(pWPolar)
		{
			//remove and delete its children POpps from the array
			for (l=s_oaPOpp.size()-1;l>=0; l--)
			{
				PlaneOpp *pPOpp = (PlaneOpp*)s_oaPOpp.at(l);
				if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->polarName())
				{
					s_oaPOpp.removeAt(l);
					delete pPOpp;
				}
			}

			for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
			{
				pOldWPolar = (WPolar*)s_oaWPolar.at(ipb);
				if(pOldWPolar==pWPolar)
				{
					s_oaWPolar.removeAt(ipb);
					delete pOldWPolar;
					break;
				}
			}
		}

		//room has been made, insert the new WPolar in alphabetical order
		for (l=0; l<s_oaWPolar.size();l++)
		{
			pOldWPolar = (WPolar*)s_oaWPolar.at(l);

			if(pOldWPolar->polarName().compare(pNewWPolar->polarName(), Qt::CaseInsensitive) >0)
			{
				//then insert before
				s_oaWPolar.insert(l, pNewWPolar);
				return pNewWPolar;
			}
		}
		//not inserted, append
		s_oaWPolar.append(pNewWPolar);
		return pNewWPolar;

	}
	else if(resp==QDialog::Rejected)
	{
		return NULL;
	}
	else if(resp==QDialog::Accepted)
	{
		//not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
		pNewWPolar->polarName()=dlg.newName();
		for (l=0; l<s_oaWPolar.size();l++)
		{
			pOldWPolar = (WPolar*)s_oaWPolar.at(l);

			if(pOldWPolar->polarName().compare(pNewWPolar->polarName(), Qt::CaseInsensitive) >0)
			{
				//then insert before
				s_oaWPolar.insert(l, pNewWPolar);
				return pNewWPolar;
			}
		}
		//not inserted, append
		s_oaWPolar.append(pNewWPolar);
		return pNewWPolar;

	}
	return NULL;//should never gethere
}





/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void Objects3D::renamePlane(QString PlaneName)
{
	QString OldName;
	PlaneOpp *pPOpp;
	int l;
	WPolar *pWPolar;
	Plane *pPlane = getPlane(PlaneName);
	if(pPlane)
	{
		OldName = pPlane->planeName();
		setModPlane(pPlane);

		pPlane->renameWings();

		for (l=s_oaWPolar.size()-1;l>=0; l--)
		{
			pWPolar = (WPolar*)s_oaWPolar.at(l);
			if (pWPolar->planeName() == OldName)
			{
				pWPolar->planeName() = pPlane->planeName();
			}
		}
		for (l=s_oaPOpp.size()-1;l>=0; l--)
		{
			pPOpp = (PlaneOpp*)s_oaPOpp.at(l);
			if (pPOpp->planeName() == OldName)
			{
				pPOpp->planeName() = pPlane->planeName();
			}
		}
	}
}








void Objects3D::deleteObjects()
{
	// clear everything
	int i;
	void *pObj;

	for (i=s_oaPlane.size()-1; i>=0; i--)
	{
		pObj = s_oaPlane.at(i);
		s_oaPlane.removeAt(i);
		delete (Plane*)pObj;
	}

	for (i=s_oaPOpp.size()-1; i>=0; i--)
	{
		pObj = s_oaPOpp.at(i);
		s_oaPOpp.removeAt(i);
		delete (PlaneOpp*)pObj;
	}

	for (i=s_oaWPolar.size()-1; i>=0; i--)
	{
		pObj = s_oaWPolar.at(i);
		s_oaWPolar.removeAt(i);
		delete (WPolar*)pObj;
	}

	for (i=s_oaBody.size()-1; i>=0; i--)
	{
		pObj = s_oaBody.at(i);
		s_oaBody.removeAt(i);
		delete (Body*)pObj;
	}

}















