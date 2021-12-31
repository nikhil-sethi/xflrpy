/****************************************************************************

    Objects2D    Copyright (C) 2016-2019 Andre Deperrois

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



#include "objects2d.h"

#include <xdirect/xdirect.h>
#include <globals/globals.h>
#include <misc/options/settings.h>
#include <QDebug>

#define PI 3.141592654

QVector<Foil*>    Objects2d::s_oaFoil;
QVector<Polar*>   Objects2d::s_oaPolar;
QVector<OpPoint*> Objects2d::s_oaOpp;



Objects2d::Objects2d()
{
}

void Objects2d::deleteAllFoils()
{
    for(int io=0; io<s_oaOpp.size(); io++) delete s_oaOpp.at(io);
    s_oaOpp.clear();

    for(int ip=0; ip<s_oaPolar.size(); ip++) delete s_oaPolar.at(ip);
    s_oaPolar.clear();

    for(int ifoil=0; ifoil<s_oaFoil.size(); ifoil++) delete s_oaFoil.at(ifoil);
    s_oaFoil.clear();
}

/**
 * Deletes the Foil object removes it from the array, and returns a pointer to the next Foil in the array
 * @param pFoil a pointer to the Foil to delete
 * @return a pointer to the next or previous Foil in the array, or NULL if none
 */
Foil * Objects2d::deleteFoil(Foil *pFoil)
{
    if(!pFoil || !pFoil->foilName().length()) return nullptr;

    Foil *pOldFoil=nullptr;
    OpPoint * pOpPoint=nullptr;
    Polar* pPolar=nullptr;

    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        pOpPoint = s_oaOpp[j];
        if(pOpPoint->foilName() == pFoil->foilName())
        {
            if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(nullptr);
            s_oaOpp.removeAt(j);
            delete pOpPoint;
        }
    }


    for (int j=s_oaPolar.size()-1; j>=0; j--)
    {
        pPolar = s_oaPolar.at(j);
        if(pPolar->foilName() == pFoil->foilName())
        {
            if(pPolar==XDirect::curPolar()) XDirect::setCurPolar(nullptr);
            s_oaPolar.removeAt(j);
            delete pPolar;
        }
    }


    Foil *pNewCurFoil = nullptr;

    for (int j=0; j<s_oaFoil.size(); j++)
    {
        pOldFoil = s_oaFoil.at(j);
        if (pOldFoil == pFoil)
        {
            if(j<s_oaFoil.count()-1) pNewCurFoil = s_oaFoil.at(j+1);
            else if(j>0)             pNewCurFoil = s_oaFoil.at(j-1);
            else                     pNewCurFoil = nullptr;

            s_oaFoil.removeAt(j);
            delete pOldFoil;
            break;
        }
    }
    return pNewCurFoil;
}


/**
 * Returns a pointer to the foil with the corresponding nam or NULL if not found.
 * @param strFoilName the name of the Foil to search for in the array
 * @return a pointer to the foil with the corresponding nam or NULL if not found.
 */
Foil* Objects2d::foil(QString strFoilName)
{
    if(!strFoilName.length()) return nullptr;
    Foil* pFoil=nullptr;

    for (int i=0; i<s_oaFoil.size(); i++)
    {
        pFoil = s_oaFoil.at(i);
        if (pFoil->foilName() == strFoilName)
        {
            return pFoil;
        }
    }

    return nullptr;
}


Foil* Objects2d::foilAt(int iFoil)
{
    if(iFoil<0 || iFoil>=foilCount()) return nullptr;
    return s_oaFoil.at(iFoil);
}


/**
 * The Foil does not exist yet in the array.
 * Delete any former Foil with the same name, including its children objects.
 * Insert the Foil in sorted order in the array.
 * Questions have been answered previously : which-name, overwrite-or-not-overwrite, etc. Just do it.
*/
void Objects2d::insertThisFoil(Foil *pFoil)
{
    Foil*pOldFoil;
    QString oldFoilName = pFoil->foilName();

    //check that this Foil does not exist in the array
    for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
    {
        if(pFoil == s_oaFoil.at(iFoil))
        {
            //            Trace("This foil "+m_FoilName+" aready exists and has not been inserted");
            return;
        }
    }

    //check if it's an overwrite
    for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
    {
        pOldFoil = s_oaFoil.at(iFoil);
        if(pOldFoil->foilName()==oldFoilName && pOldFoil!=pFoil)
        {
            //copy the old foil's style
            pFoil->m_red          = pOldFoil->m_red;
            pFoil->m_green        = pOldFoil->m_green;
            pFoil->m_blue         = pOldFoil->m_blue;
            pFoil->m_alphaChannel = pOldFoil->m_alphaChannel;
            pFoil->m_FoilStyle = pOldFoil->foilLineStyle();
            pFoil->m_FoilWidth = pOldFoil->foilLineWidth();

            //we overwrite the old foil and delete its children objects
            deleteFoil(pOldFoil);
            //continue loop, shouldn't find any other Foil with the same name, but cleans up former errors eventually
        }
    }

    // no existing former foil with the same name, straightforward insert
    for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
    {
        pOldFoil = s_oaFoil.at(iFoil);
        if(pFoil->foilName().compare(pOldFoil->foilName(), Qt::CaseInsensitive)<0)
        {
            //then insert before
            s_oaFoil.insert(iFoil, pFoil);
            return;
        }
    }

    //not inserted, append
    s_oaFoil.append(pFoil);
}


/**
* Creates a new instance of an OpPoint.
* Loads the data from the XFoil object in that OpPoint
* Adds the data to the active Polar object
* @param pPolarPtr a point to the parent Polar object to which the OpPoint should be attached.
* @return a pointer to the OpPoint which has been created, or NULL if it wasn't stored.
*/
OpPoint* Objects2d::addOpPoint(Foil *pFoil, Polar *pPolar, OpPoint *pOpPoint, bool bStoreOpp)
{
    if(!pFoil || !pOpPoint) return nullptr;

    if(!pPolar) pPolar = XDirect::curPolar();
    if(!pPolar) return nullptr;

    if(Settings::isAlignedChildrenStyle())
    {
        pOpPoint->m_Style = pPolar->m_Style;
        pOpPoint->m_Width = pPolar->m_Width;
        pOpPoint->setColor(pPolar->m_red, pPolar->m_green, pPolar->m_blue, pPolar->alphaChannel());
        pOpPoint->m_PointStyle = pPolar->m_PointStyle;
    }
    else
    {
        QColor clr = randomColor(!Settings::isLightTheme());
        pOpPoint->setColor(clr.red(), clr.green(), clr.black(), clr.alpha());
    }


    if(pOpPoint ==nullptr)
    {
        return nullptr;
    }
    else
    {
        pOpPoint->foilName() = pFoil->foilName();
        pOpPoint->polarName()  = pPolar->polarName();
    }

    if(bStoreOpp)
    {
        //insert the OpPoint in the Operating points array
        Objects2d::insertOpPoint(pOpPoint);
    }

    // Now insert OpPoint in the current Polar object
    if(pPolar->polarType()==XFLR5::FIXEDLIFTPOLAR || pPolar->polarType()==XFLR5::RUBBERCHORDPOLAR)
    {
        if(pOpPoint && pOpPoint->Reynolds()<1.00e8)
        {
            pPolar->addOpPointData(pOpPoint);
        }
    }
    else
    {
        pPolar->addOpPointData(pOpPoint);
    }


    if(!bStoreOpp)
    {
        delete pOpPoint;
        pOpPoint = nullptr;
    }

    return pOpPoint;
}




/**
 * Deletes the Foil and its children Polar and OpPoint objects
 * @return a pointer to the next or previous Foil in the array
 */
Foil * Objects2d::deleteThisFoil(Foil *pFoil)
{
    Foil *pOldFoil=nullptr;
    Polar* pOldPolar=nullptr;
    OpPoint *pOpPoint=nullptr;

    //delete any OpPoints with this FoilName
    for (int jOpp=s_oaOpp.size()-1; jOpp>=0; jOpp--)
    {
        pOpPoint = s_oaOpp[jOpp];
        if(pOpPoint->foilName() == pFoil->foilName())
        {
            if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(nullptr);
            s_oaOpp.removeAt(jOpp);
            delete pOpPoint;
        }
    }


    //delete any Polars  with this FoilName
    for (int jPlr=s_oaPolar.size()-1; jPlr>=0; jPlr--)
    {
        pOldPolar = s_oaPolar.at(jPlr);
        if(pOldPolar->foilName() == pFoil->foilName())
        {
            if(pOldPolar==XDirect::curPolar()) XDirect::setCurPolar(nullptr);
            s_oaPolar.removeAt(jPlr);
            delete pOldPolar;
        }
    }


    //delete the Foil
    Foil *pNewCurFoil= nullptr;

    for (int jFoil=0; jFoil<s_oaFoil.size(); jFoil++)
    {
        pOldFoil = s_oaFoil.at(jFoil);
        if (pOldFoil == pFoil)
        {
            if(jFoil<s_oaFoil.count()-1) pNewCurFoil = s_oaFoil.at(jFoil+1);
            else if(jFoil>0)             pNewCurFoil = s_oaFoil.at(jFoil-1);
            else                         pNewCurFoil = nullptr;

            s_oaFoil.removeAt(jFoil);
            delete pOldFoil;
            break;
        }
    }
    return pNewCurFoil;
}


void Objects2d::deleteOppAt(int index)
{
    if(index<0||index>=s_oaOpp.size()) return;

    OpPoint *pOpp = s_oaOpp.at(index);
    if(pOpp == XDirect::curOpp()) XDirect::setCurOpp(nullptr);
    s_oaOpp.removeAt(index);
    delete pOpp;
}


bool Objects2d::deleteOpp(OpPoint *pOpp)
{
    OpPoint* pOldOpp=nullptr;

    if(!pOpp) return false;
    if(pOpp == XDirect::curOpp()) XDirect::setCurOpp(nullptr);

    for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
    {
        pOldOpp = s_oaOpp.at(iOpp);
        if (pOpp == pOldOpp)
        {
            s_oaOpp.removeAt(iOpp);
            delete pOpp;
            return true;
        }
    }
    return false;
}


void Objects2d::deletePolar(Polar *pPolar)
{
    Polar* pOldPolar=nullptr;

    if(!pPolar) return;
    if(pPolar == XDirect::curPolar()) XDirect::setCurPolar(nullptr);

    for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
    {
        pOldPolar =s_oaPolar.at(iPolar);
        if (pPolar == pOldPolar)
        {
            s_oaPolar.removeAt(iPolar);
            delete pOldPolar;
            break;
        }
    }
}


void Objects2d::deletePolarAt(int index)
{
    if(index<0 || index>=s_oaPolar.size()) return;
    Polar *pPolar = s_oaPolar.at(index);
    if(pPolar == XDirect::curPolar()) XDirect::setCurPolar(nullptr);
    s_oaPolar.removeAt(index);
    delete pPolar;
}

bool Objects2d::foilExists(QString foilName)
{
    Foil *pOldFoil = nullptr;

    for (int i=0; i<s_oaFoil.size(); i++)
    {
        pOldFoil = s_oaFoil.at(i);
        if (pOldFoil->foilName() == foilName)
        {
            return true;
        }
    }
    return false;
}
/**
 * The foil exists in the array.
 * Rename it with the specified new name, and rename its children Polar and OpPoint objects
 * Overwrites any former foil with the same name.
 * Questions have been answered previously : which-name, overwrite-or-not-overwrite, etc. Just do it.
 * @param newFoilName
 */
void Objects2d::renameThisFoil(Foil *pFoil, QString newFoilName)
{
    Foil *pOldFoil=nullptr;
    Polar* pOldPolar=nullptr;
    OpPoint *pOpPoint=nullptr;
    QString oldFoilName = pFoil->foilName();

    //check that this Foil exists in the array
    bool bFound = false;
    for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
    {
        if(pFoil == s_oaFoil.at(iFoil))
        {
            bFound = true;
            break;
        }
    }
    if(!bFound)
    {
        //        Trace("The foil "+m_FoilName+" could not be found in the array and has not been renamed");
        return;
    }

    //rename it
    pFoil->setFoilName(newFoilName);

    //delete any former Foil with the new name
    for(int iFoil=0; iFoil<s_oaFoil.count(); iFoil++)
    {
        pOldFoil = s_oaFoil.at(iFoil);
        if(pOldFoil->foilName() == oldFoilName)
        {
            deleteThisFoil(pOldFoil);
            //continue loop to purge old duplicates, who knows.
        }
    }

    //ready to rename

    //rename its children objects
    for (int iPolar=0; iPolar<s_oaPolar.size(); iPolar++)
    {
        pOldPolar = s_oaPolar.at(iPolar);
        if(pOldPolar->foilName() == oldFoilName)
        {
            pOldPolar->setFoilName(newFoilName);
        }
    }

    for (int iOpp=0; iOpp<s_oaOpp.size(); iOpp++)
    {
        pOpPoint = s_oaOpp.at(iOpp);
        if(pOpPoint->foilName() == oldFoilName)
        {
            pOpPoint->setFoilName(newFoilName);
        }
    }


    //remove the Foil from its current position in the array
    for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
    {
        if(pFoil == s_oaFoil.at(iFoil))
        {
            s_oaFoil.removeAt(iFoil);
            break;
        }
    }


    //re-insert and we're done
    for(int iFoil=0; iFoil<s_oaFoil.size(); iFoil++)
    {
        pOldFoil = s_oaFoil.at(iFoil);
        if(pFoil->foilName().compare(pOldFoil->foilName(), Qt::CaseInsensitive)<0)
        {
            //then insert before
            s_oaFoil.insert(iFoil, pFoil);
            return;
        }
    }

    //Not inserted, append
    s_oaFoil.append(pFoil);
}




OpPoint *Objects2d::getOpp(Foil *pFoil, Polar *pPolar, double Alpha)
{
    OpPoint* pOpPoint = nullptr;
    if(!pPolar) return nullptr;

    for (int i=0; i<s_oaOpp.size(); i++)
    {
        if(!pPolar) return nullptr;
        pOpPoint = s_oaOpp.at(i);
        //since alphas are calculated at 1/100th
        if (pOpPoint->foilName() == pFoil->foilName())
        {
            if (pOpPoint->polarName() == pPolar->polarName())
            {
                if(pPolar->polarType() != XFLR5::FIXEDAOAPOLAR)
                {
                    if(qAbs(pOpPoint->aoa() - Alpha) <0.001)
                    {
                        return pOpPoint;
                    }
                }
                else
                {
                    if(qAbs(pOpPoint->Reynolds() - Alpha) <0.1)
                    {
                        return pOpPoint;
                    }
                }
            }
        }
    }
    return nullptr;// if no OpPoint has a matching alpha
}


/**
 * Inserts a new OpPoint in the array. The OpPoints are sorted by FoilName first, then by Re number, then by aoa.
 * If an OpPoint already exists with the same combination of (FoilName, Re, aoa), it is overwritten.
 * @param pNewPoint
 */
void Objects2d::insertOpPoint(OpPoint *pNewPoint)
{
    if(!pNewPoint) return;

    OpPoint* pOpPoint = nullptr;
    Polar *pPolar = getPolar(pNewPoint->foilName(), pNewPoint->polarName());

    if(!pPolar)
    {
        delete pNewPoint;
        return;
    }

    // first add the OpPoint to the OpPoint Array for the current FoilName
    for (int i=0; i<s_oaOpp.size(); i++)
    {
        pOpPoint = s_oaOpp.at(i);
        if (pNewPoint->foilName().compare(pOpPoint->foilName())<0)
        {
            //insert point
            s_oaOpp.insert(i, pNewPoint);
            return;
        }
        else if (pNewPoint->foilName() == pOpPoint->foilName())
        {
            if (pNewPoint->Reynolds() < pOpPoint->Reynolds())
            {
                //insert point
                s_oaOpp.insert(i, pNewPoint);
                return;
            }
            else if (fabs(pNewPoint->Reynolds()-pOpPoint->Reynolds())<1.0)
            {
                if(fabs(pNewPoint->aoa() - pOpPoint->aoa())<0.005 &&
                        fabs(pNewPoint->ACrit-pOpPoint->ACrit)<0.1 &&
                        fabs(pNewPoint->Xtr1-pOpPoint->Xtr1)<0.001 &&
                        fabs(pNewPoint->Xtr2-pOpPoint->Xtr2)<0.001)
                {

                    //replace existing point
                    XDirect::setCurOpp(nullptr);
                    s_oaOpp.removeAt(i);
                    delete pOpPoint;
                    s_oaOpp.insert(i, pNewPoint);
                    return;
                }
                else if (pNewPoint->m_Alpha > pOpPoint->aoa())
                {
                    //insert point
                    s_oaOpp.insert(i, pNewPoint);
                    return;
                }
            }
        }
    }

    s_oaOpp.append(pNewPoint);
}


/**
 * Inserts a polar in the array, using the foil name, the polar type, the Re number and the a.o.a. as sorting keys.
 * If a Polar with identical foilname and polar name exists, deletes the old and replaces it.
 * Note : sorting of Type 1 polars is necessary for the interpolation process in Plane analysis.
 * @param pPolar a pointer to the Polar object to insert in the array;
 */
void Objects2d::addPolar(Polar *pPolar)
{
    if(!pPolar) return;

    bool bExists   = false;
    bool bInserted = false;
    Polar *pOldPlr = nullptr;
    int ip,j;

    for (ip=0; ip<s_oaPolar.size(); ip++)
    {
        pOldPlr = s_oaPolar.at(ip);
        if (pOldPlr->polarName().compare(pPolar->polarName())==0 &&
                pOldPlr->foilName().compare(pPolar->foilName())==0)
        {
            bExists = true;
            s_oaPolar.removeAt(ip);
            delete pOldPlr;
            s_oaPolar.insert(ip, pPolar);
            break;
        }
    }

    if(!bExists)
    {
        for (j=0; j<s_oaPolar.size(); j++)
        {
            pOldPlr = s_oaPolar.at(j);

            //first index is the parent foil name
            if (pPolar->foilName().compare(pOldPlr->foilName(), Qt::CaseInsensitive)<0)
            {
                s_oaPolar.insert(j, pPolar);
                bInserted = true;
                break;
            }
            else if (pPolar->foilName() == pOldPlr->foilName())
            {
                //second index is the polar type
                if(pPolar->polarType() < pOldPlr->polarType())
                {
                    s_oaPolar.insert(j, pPolar);
                    bInserted = true;
                    break;
                }
                else if(pPolar->polarType() == pOldPlr->polarType())
                {
                    if (pPolar->polarType() != XFLR5::FIXEDAOAPOLAR)
                    {
                        //sort by re Nbr
                        if(pPolar->Reynolds() < pOldPlr->Reynolds())
                        {
                            s_oaPolar.insert(j, pPolar);
                            bInserted = true;
                            break;
                        }
                    }
                    else
                    {
                        //Type 4, sort by Alphas
                        if(pPolar->m_ASpec < pOldPlr->m_ASpec)
                        {
                            s_oaPolar.insert(j, pPolar);
                            bInserted = true;
                            break;
                        }
                    }
                }
            }
        }
        if(!bInserted)
        {
            s_oaPolar.append(pPolar);
            bInserted = true;
        }
    }
}


Polar *Objects2d::getPolar(Foil *pFoil, QString PolarName)
{
    if (!PolarName.length()) return nullptr;

    for (int i=0; i<s_oaPolar.size(); i++)
    {
        Polar *pPolar =  s_oaPolar.at(i);
        if (pPolar->foilName() == pFoil->foilName() &&  pPolar->polarName() == PolarName)
        {
            return pPolar;
        }
    }
    return nullptr;
}


Polar *Objects2d::getPolar(QString m_FoilName, QString PolarName)
{
    if (!PolarName.length())
    {
        return nullptr;
    }

    for (int i=0; i<s_oaPolar.size(); i++)
    {
        Polar *pPolar =  s_oaPolar.at(i);
        if (pPolar->foilName() == m_FoilName &&  pPolar->polarName() == PolarName)
        {
            return pPolar;
        }
    }
    return nullptr;
}


Polar* Objects2d::polarAt(int index)
{
    if(index<0 || index>=s_oaPolar.size()) return nullptr;
    return s_oaPolar.at(index);
}


OpPoint*  Objects2d::oppAt(int index)
{
    if(index<0 || index>=s_oaOpp.size()) return nullptr;
    return s_oaOpp.at(index);
}


void Objects2d::deleteFoilResults(Foil *pFoil, bool bDeletePolars)
{
    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        OpPoint *pOpPoint = s_oaOpp[j];
        if(pOpPoint->foilName() == pFoil->foilName())
        {
            if(pOpPoint==XDirect::curOpp()) XDirect::setCurOpp(nullptr);
            s_oaOpp.removeAt(j);
            delete pOpPoint;
        }
    }

    for (int j=s_oaPolar.size()-1; j>=0; j--)
    {
        Polar *pPolar = s_oaPolar.at(j);
        if(pPolar->foilName() == pFoil->foilName())
        {
            if(bDeletePolars)
            {
                if(pPolar==XDirect::curPolar()) XDirect::setCurPolar(nullptr);
                s_oaPolar.removeAt(j);
                delete pPolar;
            }
            else
            {
                pPolar->resetPolar();
            }
        }
    }
}


void Objects2d::setFoilChildrenStyle(Foil *pFoil)
{
    if(!pFoil) return;
    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        OpPoint *pOpPoint = s_oaOpp[j];
        if(pOpPoint->foilName() == pFoil->foilName())
        {
            pOpPoint->m_Style = pFoil->m_FoilStyle;
            pOpPoint->m_Width = pFoil->m_FoilWidth;
            pOpPoint->setColor(pFoil->m_red, pFoil->m_green, pFoil->m_blue, pFoil->alphaChannel());
            pOpPoint->m_PointStyle = pFoil->m_PointStyle;
        }
    }

    for (int j=s_oaPolar.size()-1; j>=0; j--)
    {
        Polar *pPolar = s_oaPolar.at(j);
        if(pPolar->foilName() == pFoil->foilName())
        {
            pPolar->m_Style = pFoil->m_FoilStyle;
            pPolar->m_Width = pFoil->m_FoilWidth;
            pPolar->setColor(pFoil->m_red, pFoil->m_green, pFoil->m_blue, pFoil->alphaChannel());
            pPolar->m_PointStyle = pFoil->m_PointStyle;
        }
    }
}


void Objects2d::setPolarChildrenStyle(Polar *pPolar)
{
    if(!pPolar) return;
    for (int j=s_oaOpp.size()-1; j>=0; j--)
    {
        OpPoint *pOpPoint = s_oaOpp[j];
        if(pOpPoint->foilName() == pPolar->foilName() && pOpPoint->polarName()==pPolar->polarName())
        {
            pOpPoint->m_Style = pPolar->m_Style;
            pOpPoint->m_Width = pPolar->m_Width;
            pOpPoint->setColor(pPolar->m_red, pPolar->m_green, pPolar->m_blue, pPolar->alphaChannel());
            pOpPoint->m_PointStyle = pPolar->m_PointStyle;
        }
    }
}

