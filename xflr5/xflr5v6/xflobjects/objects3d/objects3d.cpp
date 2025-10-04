/****************************************************************************

    Objects3D    Copyright (C) 2014-2019 André Deperrois

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



#include "objects3d.h"
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/editors/renamedlg.h>


QVector <Plane*>    Objects3d::s_oaPlane;
QVector <WPolar*>   Objects3d::s_oaWPolar;
QVector <PlaneOpp*> Objects3d::s_oaPOpp;
QVector <Body*>     Objects3d::s_oaBody;



/**
 * If the body is associated to a plane, duplicates the body and attaches it to the Plane
 * Else destroys the Body.
 * Used only to import body Objects from files prior to v6.09.06
 * @param a pointer to the Body object to be inserted
 * @return true if the body is associated to a plane, false otherwise
 */
void Objects3d::addBody(Body *pBody)
{
    for(int ip=0; ip<s_oaPlane.size(); ip++)
    {
        Plane *pPlane = s_oaPlane.at(ip);
        if(pPlane->m_BodyName==pBody->m_Name)
        {
            // duplicate the body - create one for each plane
            // no more bodies associated to multiple plane
            Body *planeBody = new Body();
            planeBody->duplicate(pBody);

            //attach it and rename it
            pPlane->setBody(planeBody);
        }
    }
    /*    for(int ib=0; ib<m_poaBody->size(); ib++)
    {
        Body *pOldBody = m_poaBody->at(ib);
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
bool Objects3d::planeExists(QString const &planeName)
{
    Plane *pOldPlane = nullptr;

    for (int i=0; i<s_oaPlane.size(); i++)
    {
        pOldPlane = s_oaPlane.at(i);
        if (pOldPlane->name() == planeName)
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
Plane* Objects3d::addPlane(Plane *pPlane)
{
    Plane *pOldPlane = nullptr;

    for (int i=0; i<s_oaPlane.size(); i++)
    {
        pOldPlane = s_oaPlane.at(i);
        if (pOldPlane->name() == pPlane->name())
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
    for (int j=0; j<s_oaPlane.size(); j++)
    {
        pOldPlane = s_oaPlane.at(j);
        if (pPlane->name() < pOldPlane->name())
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
Plane* Objects3d::duplicatePlane(Plane *pPlane)
{
    if(!pPlane) return nullptr;
    Plane* pNewPlane= new Plane;
    pNewPlane->duplicate(pPlane);
    return setModPlane(pNewPlane);
}


void Objects3d::insertPOpp(PlaneOpp *pPOpp)
{
    bool bIsInserted = false;

    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp *pOldPOpp = s_oaPOpp.at(i);
        if (pPOpp->planeName() == pOldPOpp->planeName())
        {
            if (pPOpp->polarName() == pOldPOpp->polarName())
            {
                if(pPOpp->polarType()<xfl::FIXEDAOAPOLAR)
                {
                    if(qAbs(pPOpp->alpha() - pOldPOpp->alpha())<0.005)
                    {
                        //replace existing point
                        s_oaPOpp.removeAt(i);
                        delete pOldPOpp;
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        break;
                    }
                    else if (pPOpp->alpha() < pOldPOpp->alpha())
                    {
                        //insert point
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                }
                else if (pPOpp->polarType()==xfl::FIXEDAOAPOLAR)
                {
                    if(qAbs(pPOpp->m_QInf - pOldPOpp->m_QInf)<0.1)
                    {
                        //replace the existing point
                        s_oaPOpp.removeAt(i);
                        delete pOldPOpp;
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                    else if (pPOpp->m_QInf < pOldPOpp->m_QInf)
                    {
                        //insert point
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                }
                else if (pPOpp->polarType()==xfl::BETAPOLAR)
                {
                    if(qAbs(pPOpp->beta() - pOldPOpp->beta())<0.01)
                    {
                        //replace existing point
                        s_oaPOpp.removeAt(i);
                        delete pOldPOpp;
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                    else if (pPOpp->beta() < pOldPOpp->beta())
                    {
                        //insert point
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                }
                else if(pPOpp->polarType()==xfl::STABILITYPOLAR)
                {
                    if(qAbs(pPOpp->ctrl() - pOldPOpp->ctrl())<0.001)
                    {
                        //replace existing point
                        s_oaPOpp.removeAt(i);
                        delete pOldPOpp;
                        s_oaPOpp.insert(i, pPOpp);
                        bIsInserted = true;
                        i = s_oaPOpp.size();// to break
                    }
                    else if (pPOpp->ctrl() < pOldPOpp->ctrl())
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

    if (!bIsInserted)     s_oaPOpp.append(pPOpp);
}



/**
* Adds the WPolar pointed by pWPolar to the m_oaWPolar array.
* Inserts it in alphabetical order using the Plane Name and the polar name
*@param pWPolar a pointer to the instance of the CWPolar object to be inserted
*@return a pointer  to the instance of the CWPolar object which has been inserted, or NULL if the operation failed
*/
void Objects3d::addWPolar(WPolar *pWPolar)
{
    int j;
    WPolar *pOldWPlr = nullptr;

    for (int ip=0; ip<s_oaWPolar.size(); ip++)
    {
        pOldWPlr = s_oaWPolar.at(ip);
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
        pOldWPlr = s_oaWPolar.at(j);
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
void Objects3d::deletePlane(Plane *pPlane)
{
    if(!pPlane || !pPlane->name().length()) return;
    Plane *pOldPlane = nullptr;

    deletePlaneResults(pPlane);

    for (int i=s_oaPlane.size()-1; i>=0; i--)
    {
        pOldPlane = s_oaPlane.at(i);
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
void Objects3d::deleteWPolar(WPolar *pWPolar)
{
    //remove and delete its children POpps from the array
    if(!pWPolar)return;

    for (int l=s_oaPOpp.size()-1;l>=0; l--)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(l);
        if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->polarName())
        {
            s_oaPOpp.removeAt(l);
            delete pPOpp;
        }
    }

    for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
    {
        WPolar *pOldWPolar = s_oaWPolar.at(ipb);
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
void Objects3d::deletePlaneResults(Plane *pPlane, bool bDeletePolars)
{
    if(!pPlane || !pPlane->name().length()) return ;
    WPolar* pWPolar = nullptr;
    PlaneOpp * pPOpp = nullptr;


    //first remove all POpps associated to the plane
    for (int i=s_oaPOpp.size()-1; i>=0; i--)
    {
        pPOpp = s_oaPOpp.at(i);
        if(pPOpp->planeName() == pPlane->name())
        {
            s_oaPOpp.removeAt(i);
            delete pPOpp;
        }
    }

    //next delete all WPolars associated to the plane
    for (int i=s_oaWPolar.size()-1; i>=0; i--)
    {
        pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName() == pPlane->name())
        {
            if(bDeletePolars)
            {
                s_oaWPolar.removeAt(i);
                delete pWPolar;
                pWPolar = nullptr;
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
Body * Objects3d::getBody(const QString &BodyName)
{
    Body* pBody = nullptr;
    for (int ib=0; ib<s_oaBody.size(); ib++)
    {
        pBody = s_oaBody.at(ib);
        if (pBody->m_Name == BodyName) return pBody;
    }

    Plane *pPlane;
    for(int ip=0; ip<s_oaPlane.size(); ip++)
    {
        pPlane = s_oaPlane.at(ip);
        if(pPlane->body())
        {
            if((pPlane->name()+"/Body") == BodyName) return pPlane->body();
        }
    }

    return nullptr;
}


/**
* Returns a pointer to the OpPoint with the name of the current plane and current WPolar, and which matches the input parameter
* @param x the aoa, of velocity, or control parameter for which the CPOpp object is requested
* @return a pointer to the instance of the CPOpp object, or NULL if non has been found
*/
PlaneOpp * Objects3d::getPlaneOpp(Plane const*pPlane, WPolar const*pWPolar, double x)
{
    if(!pPlane || !pWPolar) return nullptr;


    for (int i=0; i<s_oaPOpp.size(); i++)
    {
        PlaneOpp*pPOpp = s_oaPOpp.at(i);
        if ((pPOpp->planeName() == pPlane->name()) && (pPOpp->polarName() == pWPolar->polarName()))
        {
            if     (pWPolar->isT12Polar() && qAbs(pPOpp->alpha() - x)<0.005)   return pPOpp;
            else if(pWPolar->isT4Polar()  && qAbs(pPOpp->QInf()  - x) <0.005)  return pPOpp;
            else if(pWPolar->isT5Polar()  && qAbs(pPOpp->beta()  - x) <0.005)  return pPOpp;
            else if(pWPolar->isT7Polar()  && qAbs(pPOpp->ctrl()  - x) <0.005)  return pPOpp;
        }
    }
    return nullptr;
}


/**
* Returns a pointer to the polar with the name of the input parameter
* @param WPolarName the name of the CWPolar object
* @return a pointer to the instance of the CWPolar object, or NULL if non has been found
*/
WPolar* Objects3d::getWPolar(Plane const*pPlane, const QString &WPolarName)
{
    WPolar *pWPolar = nullptr;

    if(!pPlane) return nullptr;

    for (int i=0; i<s_oaWPolar.size(); i++)
    {
        pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName()==pPlane->name() && pWPolar->polarName()== WPolarName)
            return pWPolar;
    }
    return nullptr;
}



/**
* Returns a pointer to the plane with the name PlaneName
*@param PlaneName a QString object with the name of the requested CPlane object
*@return a pointer to the instance of the CPlane object, or NULL if non has been found
*/
Plane * Objects3d::getPlane(QString const &PlaneName)
{
    Plane* pPlane = nullptr;
    for (int i=0; i<s_oaPlane.size(); i++)
    {
        pPlane = s_oaPlane.at(i);
        if (pPlane->name() == PlaneName) return pPlane;
    }
    return nullptr;
}



/**
 * Returns a pointer to the main wing of the plane defined by its name
 * @param PlaneName the source Plane's name
 * @return a pointer to the Plane's main wing.
 */
Wing* Objects3d::getWing(const QString &PlaneName)
{
    for(int ip=0; ip<s_oaPlane.size(); ip++)
    {
        Plane *pPlane = s_oaPlane.at(ip);
        if(QString::compare(pPlane->name(), PlaneName, Qt::CaseInsensitive)==0) return pPlane->m_Wing;
    }
    return nullptr;
}



/**
 * Inserts a modified Plane object in the array, i.a.w. user instructions
 * @param pModPlane a pointer to the instance of the Plane object to be inserted
 * @return a pointer to the Plane object which was successfully inserted, false otherwise
 */
Plane * Objects3d::setModPlane(Plane *pModPlane)
{
    if(!pModPlane) return nullptr;
    Plane * pPlane = nullptr;
    bool bExists = true;
    int resp=0, k=0, l=0;

    QString OldName = pModPlane->name();

    QStringList NameList;
    for(k=0; k<s_oaPlane.size(); k++)
    {
        pPlane = s_oaPlane.at(k);
        NameList.append(pPlane->name());
    }

    RenameDlg renDlg;
    renDlg.initDialog(&NameList, pModPlane->name(),QObject::tr("Enter the new name for the Plane :"));

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
                pPlane = s_oaPlane.at(k);
                if (pPlane->name() == renDlg.newName())
                {
                    bExists = true;
                    break;
                }
            }

            if(!bExists)
            {
                // we have a valid name
                // rename the plane
                pModPlane->setName(renDlg.newName());

                bool bInserted = false;
                for (l=0; l<s_oaPlane.size();l++)
                {
                    pPlane = s_oaPlane.at(l);
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
                    pPlane = s_oaPlane.at(l);
                    if(pPlane->name().compare(pModPlane->name(), Qt::CaseInsensitive) >0)
                    {
                        //then insert before
                        s_oaPlane.insert(l, pModPlane);
                        bInserted = true;
                        break;
                    }
                }
                if(!bInserted)    s_oaPlane.append(pModPlane);
                break;

            }
        }
        else if(resp ==10)
        {
            //the user wants to overwrite the old plane/wing

            pPlane = getPlane(renDlg.newName());
            deletePlane(pPlane);

            pModPlane->setName(renDlg.newName());

            //place the Plane in alphabetical order in the array
            //remove the current Plane from the array
            for (l=0; l<s_oaPlane.size();l++)
            {
                pPlane = s_oaPlane.at(l);
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
                pPlane = s_oaPlane.at(l);
                if(pPlane->name().compare(pModPlane->name(), Qt::CaseInsensitive) <0)
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
            return nullptr;//cancelled
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
WPolar* Objects3d::insertNewWPolar(WPolar *pNewWPolar, Plane *pCurPlane)
{
    if(!pNewWPolar) return nullptr;
    WPolar *pWPolar = nullptr, *pOldWPolar = nullptr;

    bool bExists = true;
    int resp=0, k=0, l=0;

    //check if this WPolar is already inserted
    for(int ip=0; ip<s_oaWPolar.size(); ip++)
    {
        pOldWPolar = s_oaWPolar.at(ip);
        if(pOldWPolar==pNewWPolar)
        {
            //            Trace("this WPolar is already in the array, nothing inserted");
            return nullptr;
        }
    }

    //make a list of existing names
    QStringList NameList;
    for(k=0; k<s_oaWPolar.size(); k++)
    {
        pWPolar = s_oaWPolar.at(k);
        if(pCurPlane && pWPolar->planeName()==pCurPlane->name()) NameList.append(pWPolar->polarName());
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
            pOldWPolar = s_oaWPolar.at(l);

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
        pWPolar = nullptr;
        for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
        {
            pOldWPolar = s_oaWPolar.at(ipb);
            if(pCurPlane && pOldWPolar->polarName()==dlg.newName() && pOldWPolar->planeName()==pCurPlane->name())
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
                PlaneOpp *pPOpp = s_oaPOpp.at(l);
                if (pPOpp->planeName()==pWPolar->planeName() && pPOpp->polarName()==pWPolar->polarName())
                {
                    s_oaPOpp.removeAt(l);
                    delete pPOpp;
                }
            }

            for(int ipb=0; ipb<s_oaWPolar.size(); ipb++)
            {
                pOldWPolar = s_oaWPolar.at(ipb);
                if(pOldWPolar==pWPolar)
                {
                    s_oaWPolar.removeAt(ipb);
                    delete pOldWPolar;
                    break;
                }
            }
        }

        //room has been made, insert the new WPolar in alphabetical order
        pNewWPolar->setPolarName(dlg.newName());
        for (l=0; l<s_oaWPolar.size();l++)
        {
            pOldWPolar = s_oaWPolar.at(l);

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
        return nullptr;
    }
    else if(resp==QDialog::Accepted)
    {
        //not rejected, no overwrite, else the user has selected a non-existing name, rename and insert
        pNewWPolar->setPolarName(dlg.newName());
        for (l=0; l<s_oaWPolar.size();l++)
        {
            pOldWPolar = s_oaWPolar.at(l);

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
    return nullptr;//should never gethere
}


/**
 * Renames the active wing or plane
 * Updates the references in child polars and oppoints
 * @param PlaneName the new name for the wing or plane
 */
void Objects3d::renamePlane(const QString &PlaneName)
{
    QString OldName;
    PlaneOpp *pPOpp;
    int l;
    WPolar *pWPolar;
    Plane *pPlane = getPlane(PlaneName);
    if(pPlane)
    {
        OldName = pPlane->name();
        setModPlane(pPlane);

        pPlane->renameWings();

        for (l=s_oaWPolar.size()-1;l>=0; l--)
        {
            pWPolar = s_oaWPolar.at(l);
            if (pWPolar->planeName() == OldName)
            {
                pWPolar->setPlaneName(pPlane->name());
            }
        }
        for (l=s_oaPOpp.size()-1;l>=0; l--)
        {
            pPOpp = s_oaPOpp.at(l);
            if (pPOpp->planeName() == OldName)
            {
                pPOpp->setPlaneName(pPlane->name());
            }
        }
    }
}








void Objects3d::deleteObjects()
{
    // clear everything


    for (int i=s_oaPlane.size()-1; i>=0; i--)
    {
        Plane *pObj = s_oaPlane.at(i);
        s_oaPlane.removeAt(i);
        delete pObj;
    }

    for (int i=s_oaPOpp.size()-1; i>=0; i--)
    {
        PlaneOpp * pObj = s_oaPOpp.at(i);
        s_oaPOpp.removeAt(i);
        delete pObj;
    }

    for (int i=s_oaWPolar.size()-1; i>=0; i--)
    {
        WPolar *pObj = s_oaWPolar.at(i);
        s_oaWPolar.removeAt(i);
        delete pObj;
    }

    for (int i=s_oaBody.size()-1; i>=0; i--)
    {
        Body *pObj = s_oaBody.at(i);
        s_oaBody.removeAt(i);
        delete pObj;
    }
}


void Objects3d::setWPolarChildrenStyle(WPolar *pWPolar)
{
    if(!pWPolar) return;
    for (int j=s_oaPOpp.size()-1; j>=0; j--)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(j);
        if(pPOpp->planeName() == pWPolar->planeName() && pPOpp->polarName()==pWPolar->polarName())
        {
            pPOpp->setTheStyle(pWPolar->theStyle());
        }
    }
}


void Objects3d::setPlaneStyle(Plane *pPlane, LineStyle const &ls, bool bStipple, bool bWidth, bool bColor, bool bPoints)
{
    if(!pPlane) return;
    pPlane->setTheStyle(ls);
    WPolar *pLastWPolar = nullptr;

    for(int iwp=0; iwp<s_oaWPolar.size(); iwp++)
    {
        WPolar *pWPolar = s_oaWPolar.at(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            if(bStipple) pWPolar->setLineStipple(ls.m_Stipple);
            if(bWidth)   pWPolar->setLineWidth(ls.m_Width);
            if(bColor)
            {
                if(!pLastWPolar) pWPolar->setLineColor(ls.m_Color);
                else             pWPolar->setLineColor(pLastWPolar->lineColor().darker(107));
            }
            if(bPoints) pWPolar->setPointStyle(ls.m_Symbol);

            setWPolarPOppStyle(pWPolar, bStipple, bWidth, bColor, bPoints);

            pLastWPolar = pWPolar;
        }
    }
}


void Objects3d::setWPolarStyle(WPolar *pWPolar, LineStyle const&ls, bool bStyle, bool bWidth, bool bColor, bool bPoints)
{
    if(!pWPolar) return;
    pWPolar->setTheStyle(ls);

    setWPolarPOppStyle(pWPolar, bStyle, bWidth, bColor, bPoints);
}


void Objects3d::setWPolarPOppStyle(WPolar const* pWPolar, bool bStipple, bool bWidth, bool bColor, bool bPoints)
{
    if(!pWPolar) return;
    PlaneOpp *pLastPOpp = nullptr;
    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp)) pLastPOpp = pPOpp;
    }

    if(!pLastPOpp) return;

    if(bStipple)  pLastPOpp->setLineStipple(pWPolar->lineStipple());
    if(bWidth)    pLastPOpp->setLineWidth(pWPolar->lineWidth());
    if(bColor)    pLastPOpp->setLineColor(pWPolar->lineColor());
    if(bPoints)   pLastPOpp->setPointStyle(pWPolar->pointStyle());

    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pWPolar->hasPOpp(pPOpp))
        {
            if(bStipple) pPOpp->setLineStipple(pWPolar->lineStipple());
            if(bWidth)   pPOpp->setLineWidth(pWPolar->lineWidth());
            if(bColor)   pPOpp->setLineColor(pLastPOpp->lineColor().darker(107));
            if(bPoints)  pPOpp->setPointStyle(pWPolar->pointStyle());

            pLastPOpp = pPOpp;
        }
    }
}


Plane * Objects3d::plane(QString const &PlaneName)
{
    Plane* pPlane = nullptr;
    for (int i=0; i<planeCount(); i++)
    {
        pPlane = s_oaPlane.at(i);
        if (pPlane->name() == PlaneName) return pPlane;
    }
    return nullptr;
}


WPolar* Objects3d::wPolar(const Plane *pPlane, QString const &WPolarName)
{
    if(!pPlane) return nullptr;

    for (int i=0; i<polarCount(); i++)
    {
        WPolar *pWPolar = s_oaWPolar.at(i);
        if (pWPolar->planeName()==pPlane->name() && pWPolar->polarName()== WPolarName)
            return pWPolar;
    }
    return nullptr;
}



void Objects3d::setPlaneVisible(const Plane *pPlane, bool bVisible, bool bStabilityPolarsOnly)
{
    if(!pPlane) return;

    for(int iwp=0; iwp<s_oaWPolar.size(); iwp++)
    {
        WPolar *pWPolar = s_oaWPolar.at(iwp);
        if(pWPolar->planeName().compare(pPlane->name())==0)
        {
            /*            if(bStabilityPolarsOnly)
            {
                if(pWPolar->isStabilityPolar()) pWPolar->setVisible(bVisible);
            }
            else*/
            pWPolar->setVisible(bVisible);
        }
    }
    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pPOpp->planeName().compare(pPlane->name())==0)
        {
            if(bStabilityPolarsOnly)
            {
                if(pPOpp->isT7Polar()) pPOpp->setVisible(bVisible);
            }
            else
                pPOpp->setVisible(bVisible);
        }
    }
}


void Objects3d::setWPolarVisible(WPolar *pWPolar, bool bVisible)
{
    if(!pWPolar) return;
    pWPolar->setVisible(bVisible);
    Plane const*pPlane = plane(pWPolar->planeName());
    if(!pPlane) return;

    for(int ipp=0; ipp<s_oaPOpp.size(); ipp++)
    {
        PlaneOpp *pPOpp = s_oaPOpp.at(ipp);
        if(pPOpp->planeName().compare(pPlane->name())==0)
        {
            if(pPOpp->polarName().compare(pWPolar->polarName())==0)
                pPOpp->setVisible(bVisible);
        }
    }
}


void Objects3d::setWPolarColor(Plane const *pPlane, WPolar *pWPolar)
{
    if(!pPlane || !pWPolar) return;
    QColor clr = pPlane->lineColor();
    for(int ip=0; ip<s_oaWPolar.size(); ip++)
    {
        if(s_oaWPolar.at(ip)->planeName().compare(pPlane->name())==0)
        {
            clr = clr.darker(111);
        }
    }
    pWPolar->setLineColor(clr);
}
