/****************************************************************************

    Plane class
    Copyright (C) André Deperrois

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

#include <QStringList>

#include "plane.h"

#include <xflcore/units.h>
#include <xflcore/xflcore.h>
#include <xflobjects/objects3d/planeopp.h>
#include <xflobjects/objects3d/surface.h>
#include <xflobjects/objects3d/wpolar.h>
#include <xflobjects/objects_global.h>

/** The public constructor. */
Plane::Plane()
{
    m_Wing[0].m_Name   = QObject::tr("Wing");
    m_Wing[0].setWingType(xfl::MAINWING);
    m_Wing[0].computeGeometry();

    m_Wing[1].m_Name   = QObject::tr("2nd Wing");
    m_Wing[1].setWingType(xfl::SECONDWING);
    m_Wing[1].computeGeometry();

    m_Wing[2].m_Name    = QObject::tr("Elevator");
    m_Wing[2].setWingType(xfl::ELEVATOR);
    m_Wing[2].m_bIsFin      = false;
    m_Wing[2].m_Section[0].m_Chord = 0.100;
    m_Wing[2].m_Section[1].m_Chord = 0.080;
    m_Wing[2].m_Section[0].m_YPosition = 0.0;
    m_Wing[2].m_Section[1].m_YPosition = 0.170;
    m_Wing[2].m_Section[0].m_Length   =   0.0;
    m_Wing[2].m_Section[1].m_Length   = 0.150;
    m_Wing[2].m_Section[0].m_Offset   =   0.0;
    m_Wing[2].m_Section[1].m_Offset   = 0.020;
    m_Wing[2].m_Section[0].m_NXPanels = 7;
    m_Wing[2].m_Section[1].m_NYPanels = 7;
    m_Wing[2].m_Section[0].m_XPanelDist = xfl::SINE;
    m_Wing[2].m_Section[0].m_YPanelDist = xfl::UNIFORM;
    m_Wing[2].computeGeometry();

    m_Wing[3].m_Name    = QObject::tr("Fin");
    m_Wing[3].setWingType(xfl::FIN);
    m_Wing[3].m_bIsFin      = true;
    m_Wing[3].m_Section[0].m_Chord      = 0.100;
    m_Wing[3].m_Section[1].m_Chord      = 0.060;
    m_Wing[3].m_Section[0].m_YPosition  = 0.000;
    m_Wing[3].m_Section[1].m_YPosition  = 0.120;
    m_Wing[3].m_Section[0].m_Length     = 0.000;
    m_Wing[3].m_Section[1].m_Length     = 0.120;
    m_Wing[3].m_Section[0].m_Offset     = 0.000;
    m_Wing[3].m_Section[1].m_Offset     = 0.040;
    m_Wing[3].m_Section[0].m_NXPanels   = 7;
    m_Wing[3].m_Section[0].m_NYPanels   = 7;
    m_Wing[3].m_Section[0].m_XPanelDist = xfl::UNIFORM;
    m_Wing[3].m_Section[0].m_YPanelDist = xfl::COSINE;

    m_Wing[3].computeGeometry();

    m_TailVolume       =   0.0;
    m_WingLE[2].x      = 0.600;
    m_WingLE[2].y      =   0.0;
    m_WingLE[2].z      =   0.0;

    m_WingLE[3].x       = 0.650;
    m_WingLE[3].y       =   0.0;
    m_WingLE[3].z       =   0.0;

    for(int iw=0; iw<MAXWINGS; iw++) m_WingTiltAngle[iw] = 0.0;

    m_BodyPos.set(0.0, 0.0, 0.0);

    m_CoG.set(0.0,0.0,0.0);
    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;

    m_bBody         = false;
    m_bFin          = true;
    m_bStab         = true;
    m_bBiplane      = false;

    clearPointMasses();

    m_Name  = QObject::tr("Plane Name");

    m_theStyle.m_Width = 2;
}


Plane::~Plane()
{
    clearPointMasses();
}

bool Plane::hasWPolar(WPolar const*pWPolar) const {return pWPolar->planeName().compare(m_Name)==0;}
bool Plane::hasPOpp(PlaneOpp const*pPOpp)   const {return pPOpp->planeName().compare(m_Name)==0;}


/**
 * Calculates and returns the inertia properties of the structure based on the Body and Wing masses and on the existing geometry.
 * The inertia is calculated in the CoG referential.
 * @param Mass = mass of the structure, excluding point masses
 * @param  &CoG a reference to the CoG point, as a result of the calculation
 * @param  &CoGIxx xx axis component of the inertia tensor, calculated at the Plane's CoG
 * @param  &CoGIyy yy axis component of the inertia tensor, calculated at the Plane's CoG
 * @param  &CoGIzz zz axis component of the inertia tensor, calculated at the Plane's CoG
 * @param  &CoGIxz xz axis component of the inertia tensor, calculated at the Plane's CoG
*/
void Plane::computeVolumeInertia(double &Mass, Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz) const
{
    double Ixx(0), Iyy(0), Izz(0), Ixz(0);

    Vector3d Pt;
    Vector3d CoGBody;
    Vector3d CoGWing[MAXWINGS];
    Wing const *pWing[]{nullptr,nullptr,nullptr,nullptr};

    pWing[0] = m_Wing;
    if(m_bBiplane) pWing[1] = m_Wing+1;
    if(m_bStab)    pWing[2] = m_Wing+2;
    if(m_bFin)     pWing[3] = m_Wing+3;

    CoG.set(0.0, 0.0, 0.0);
    CoGIxx = CoGIyy = CoGIzz = CoGIxz = 0.0;
    double PlaneMass = 0.0;

    //get the wing's inertias
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pWing[iw] && pWing[iw]->m_VolumeMass>PRECISION)
        {
            //the inertia of the wings are base on the surface geometry;
            //these surfaces have been translated to the LE position as they were created
//            pWing[iw]->computeGeometry();
            pWing[iw]->computeVolumeInertia(CoGWing[iw], Ixx, Iyy, Izz, Ixz);
            CoG += CoGWing[iw] * pWing[iw]->m_VolumeMass;// so we do not add again the LE position
            PlaneMass += pWing[iw]->m_VolumeMass;
            CoGIxx += Ixx;
            CoGIyy += Iyy;
            CoGIzz += Izz;
            CoGIxz += Ixz;
        }
    }

    if(m_bBody)
    {
        if(m_Body.m_VolumeMass>PRECISION)
        {
            m_Body.computeVolumeInertia(CoGBody, Ixx, Iyy, Izz, Ixz);
            CoG += (CoGBody+m_BodyPos) * m_Body.m_VolumeMass;
            PlaneMass += m_Body.m_VolumeMass;
            CoGIxx += Ixx;
            CoGIyy += Iyy;
            CoGIzz += Izz;
            CoGIxz += Ixz;
        }
    }

    if(PlaneMass>0.0) CoG *= 1.0/ PlaneMass;
    else              CoG.set(0.0, 0.0, 0.0);


    // Deduce inertia tensor in the Plane's CoG frame from Huygens/Steiner theorem
    // we transfer the inertia of each component, defined in its own CG,
    // to the new origin which is the plane's Volume CoG, excluding point masses

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pWing[iw])
        {
            Pt = CoGWing[iw] - CoG;
            CoGIxx +=  pWing[iw]->m_VolumeMass * (Pt.y*Pt.y + Pt.z*Pt.z);
            CoGIyy +=  pWing[iw]->m_VolumeMass * (Pt.x*Pt.x + Pt.z*Pt.z);
            CoGIzz +=  pWing[iw]->m_VolumeMass * (Pt.x*Pt.x + Pt.y*Pt.y);
            CoGIxz +=  pWing[iw]->m_VolumeMass *  Pt.x*Pt.z;
        }
    }

    if(m_bBody)
    {
        Pt = CoGBody - CoG;
        CoGIxx += m_Body.m_VolumeMass * (Pt.y*Pt.y + Pt.z*Pt.z);
        CoGIyy += m_Body.m_VolumeMass * (Pt.x*Pt.x + Pt.z*Pt.z);
        CoGIzz += m_Body.m_VolumeMass * (Pt.x*Pt.x + Pt.y*Pt.y);
        CoGIxz += m_Body.m_VolumeMass *  Pt.x*Pt.z;
    }
    Mass = PlaneMass;
}


/**
* Calculates the inertia tensor in geometrical (body) axis :
*  - adds the volume inertia AND the inertia of point masses of all components
*  - the body axis is the frame in which the geometry has been defined
*  - the origin is the plane's CoG, taking into account all masses
*/
void Plane::computeBodyAxisInertia()
{
    Vector3d VolumeCoG, MassPos;
    Wing const *pWing[]{nullptr, nullptr, nullptr, nullptr};
    double Ixx(0), Iyy(0), Izz(0), Ixz(0), VolumeMass(0);

    pWing[0] = m_Wing;
    if(m_bBiplane) pWing[1] = m_Wing+1;
    if(m_bStab)    pWing[2] = m_Wing+2;
    if(m_bFin)     pWing[3] = m_Wing+3;

    computeVolumeInertia(VolumeMass, VolumeCoG, Ixx, Iyy, Izz, Ixz);
    double TotalMass = VolumeMass;

    m_CoG = VolumeCoG *VolumeMass;

    //add the wing's point masses
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pWing[iw])
        {
            for(int im=0; im<pWing[iw]->m_PointMass.size(); im++)
            {
                PointMass const &pm = pWing[iw]->m_PointMass.at(im);
                TotalMass +=  pm.mass();
                m_CoG     += (pm.position()+ m_WingLE[iw]) * pm.mass();
            }
        }
    }

    //add the body's point masses
    if(m_bBody)
    {
        for(int im=0; im<m_Body.m_PointMass.size(); im++)
        {
            PointMass const &pm = m_Body.m_PointMass.at(im);
            TotalMass +=  pm.mass();
            m_CoG     += (pm.position()+m_BodyPos) * pm.mass();
        }
    }

    // add the plane's point masses
    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);
        TotalMass += pm.mass();
        m_CoG     += pm.position() * pm.mass();
    }


    if(TotalMass>PRECISION) m_CoG = m_CoG/TotalMass;
    else                      m_CoG.set(0.0,0.0,0.0);


    // The CoG position is now available, so calculate the inertia w.r.t the Total CoG, including point masses
    // The total CoG is the new origin for this calculation, so we transfer the other inertias using Huygens/Steiner theorem
    MassPos = m_CoG - VolumeCoG;
    m_CoGIxx = Ixx + VolumeMass * (MassPos.y*MassPos.y+ MassPos.z*MassPos.z);
    m_CoGIyy = Iyy + VolumeMass * (MassPos.x*MassPos.x+ MassPos.z*MassPos.z);
    m_CoGIzz = Izz + VolumeMass * (MassPos.x*MassPos.x+ MassPos.y*MassPos.y);
    m_CoGIxz = Ixz + VolumeMass *  MassPos.x*MassPos.z;

    for(int im=0; im<m_PointMass.size(); im++)
    {
        PointMass const &pm = m_PointMass.at(im);

        MassPos = m_CoG - pm.position();
        m_CoGIxx += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
        m_CoGIyy += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
        m_CoGIzz += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
        m_CoGIxz += pm.mass() * (MassPos.x*MassPos.z);
    }

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(pWing[iw])
        {
            for(int im=0; im<pWing[iw]->m_PointMass.size(); im++)
            {
                PointMass const &pm = pWing[iw]->m_PointMass.at(im);
                MassPos = m_CoG - (pm.position() + m_WingLE[iw]);
                m_CoGIxx += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
                m_CoGIyy += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
                m_CoGIzz += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
                m_CoGIxz -= pm.mass() * (MassPos.x*MassPos.z);
            }
        }
    }

    if(m_bBody)
    {
        for(int im=0; im<m_Body.m_PointMass.size(); im++)
        {
            PointMass const &pm = m_Body.m_PointMass.at(im);
            MassPos = m_CoG - (pm.position() + m_BodyPos);
            m_CoGIxx += pm.mass() * (MassPos.y*MassPos.y + MassPos.z*MassPos.z);
            m_CoGIyy += pm.mass() * (MassPos.x*MassPos.x + MassPos.z*MassPos.z);
            m_CoGIzz += pm.mass() * (MassPos.x*MassPos.x + MassPos.y*MassPos.y);
            m_CoGIxz -= pm.mass() * (MassPos.x*MassPos.z);
        }
    }
}


/**
* Calculates the Plane's tail volume = lever_arm_elev x Area_Elev / MAC_Wing / Area_Wing
*/
void Plane::computePlane(void)
{
    if(m_bStab)
    {
        double SLA = m_WingLE[2].x + m_Wing[2].Chord(0)/4.0 - m_WingLE[0].x - m_Wing[0].Chord(0)/4.0;
        double area = m_Wing[0].m_ProjectedArea;
        if(m_bBiplane) area += m_Wing[1].m_ProjectedArea;

        double ProjectedArea = 0.0;
        for (int i=0;i<m_Wing[2].NWingSection()-1; i++)
        {
            ProjectedArea += m_Wing[2].Length(i+1)*(m_Wing[2].Chord(i)+m_Wing[2].Chord(i+1))/2.0
                    *cos(m_Wing[2].Dihedral(i)*PI/180.0)*cos(m_Wing[2].Dihedral(i)*PI/180.0);//m2

        }
        ProjectedArea *=2.0;
        m_TailVolume = ProjectedArea * SLA / area/m_Wing[0].m_MAChord ;
    }
    else m_TailVolume = 0.0;
}



/**
* Copies the data from an existing Plane
*@param pPlane a pointer to the instance of the source Plane object
*/
void Plane::duplicate(Plane const *pPlane)
{
    m_Name        = pPlane->m_Name;
    m_PlaneDescription = pPlane->m_PlaneDescription;
    renameWings();

    m_bFin          = pPlane->m_bFin;
    m_bStab         = pPlane->m_bStab;
    m_bBiplane      = pPlane->m_bBiplane;

    m_TailVolume    = pPlane->m_TailVolume;

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        m_WingTiltAngle[iw] = pPlane->m_WingTiltAngle[iw];
        m_WingLE[iw]        = pPlane->m_WingLE[iw];

        m_Wing[iw].duplicate(pPlane->m_Wing+iw);
    }

    m_BodyPos.copy(pPlane->m_BodyPos);

    m_CoG = pPlane->m_CoG;
    m_CoGIxx = pPlane->m_CoGIxx;
    m_CoGIyy = pPlane->m_CoGIyy;
    m_CoGIzz = pPlane->m_CoGIzz;
    m_CoGIxz = pPlane->m_CoGIxz;

    clearPointMasses();
    for(int im=0; im<pPlane->m_PointMass.size();im++)
    {
        m_PointMass.append(pPlane->m_PointMass.at(im));
    }

    m_bBody = pPlane->m_bBody;

    m_Body.duplicate(&pPlane->m_Body);

    setAutoBodyName();
}


void Plane::setAutoBodyName()
{
    if(!m_bBody) m_BodyName.clear();
    else
    {
        m_BodyName = m_Name+"_body";
        m_Body.m_Name = m_Name+"_body";
    }
}


void Plane::setWings(bool bWing2, bool bStab, bool bFin)
{
    m_bBiplane = bWing2;
    m_bStab = bStab;
    m_bFin = bFin;
}


/**
* Returns the plane's tail volume
* @return the plane's tail volume
*/
double Plane::tailVolume() const
{
    if(m_bStab) return m_TailVolume;
    else        return 0.0;
}


/**
* Returns the Plane's total mass, i.e. the sum of Volume and Point masses of all its components.
* @return the Plane's total mass.
*/
double Plane::totalMass() const
{
    double Mass = m_Wing[0].totalMass();
    if(m_bBiplane) Mass += m_Wing[1].totalMass();
    if(m_bStab)    Mass += m_Wing[2].totalMass();
    if(m_bFin)     Mass += m_Wing[3].totalMass();
    if(m_bBody)    Mass += m_Body.totalMass();

    for(int i=0; i<m_PointMass.size(); i++)
        Mass += m_PointMass.at(i).mass();

    return Mass;
}


/** Destroys the PointMass objects in good order to avoid memory leaks */
void Plane::clearPointMasses()
{
    m_PointMass.clear();
}


/**
* Renames each of the Plane's Wing objects with an automatic name.
*/
void Plane::renameWings()
{
    m_Wing[0].m_Name = QObject::tr("Main Wing");
    m_Wing[1].m_Name = QObject::tr("Second Wing2");
    m_Wing[2].m_Name = QObject::tr("Elevator");
    m_Wing[3].m_Name = QObject::tr("Fin");
}


/**
 *Renames the plane and sets the automatic default name for its wings .
 *@param planeName the Plane's new name.
*/
void Plane::setName(QString const &planeName)
{
    m_Name = planeName;
    renameWings();
}


/**
* Creates the Surface objects associated to each of the Plane's Wing objects.
*/
void Plane::createSurfaces()
{
    m_Wing[0].createSurfaces(m_WingLE[0],   0.0, m_WingTiltAngle[0]);
    if(wing(1)) m_Wing[1].createSurfaces(m_WingLE[1],   0.0, m_WingTiltAngle[1]);
    if(wing(2)) m_Wing[2].createSurfaces(m_WingLE[2],   0.0, m_WingTiltAngle[2]);
    if(wing(3)) m_Wing[3].createSurfaces(m_WingLE[3], -90.0, m_WingTiltAngle[3]);

    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(wing(iw))
        {
            for (int j=0; j<m_Wing[iw].m_Surface.size(); j++)
                m_Wing[iw].m_Surface[j].setMeshSidePoints(body(), bodyPos().x, bodyPos().z);
            m_Wing[iw].computeBodyAxisInertia();
        }
    }
}


/**
* Initiliazes the pointer to an existing Body object
* @param pBody the pointer to the existing Body object
*/
void Plane::setBody(Body *pBody)
{
    if(!pBody)
    {
        m_bBody = false;
        m_BodyName.clear();
    }
    else
    {
        m_bBody = true;
        m_Body.duplicate(pBody);
        setAutoBodyName();
    }
}


/**
 * Returns the number of mesh panels defined on this Plane's surfaces.
 * Assumes thin surfaces for the wings.
 * @return the number of mesh panels
 */
int Plane::VLMPanelTotal()
{
    int total = wing()->VLMPanelTotal(true);

    if(wing2()) total += wing2()->VLMPanelTotal(true);

    if(stab())  total += stab()->VLMPanelTotal(true);

    if(fin())
    {
        if(fin()->m_bDoubleFin || fin()->m_bSymFin)
            total += 2*fin()->VLMPanelTotal(true);
        else
            total +=   fin()->VLMPanelTotal(true);
    }

    if(m_bBody) total += body()->m_nxPanels * body()->m_nhPanels * 2;

    return total;
}


/**
 * Returns a pointer to the wing with index iw, or NULL if this plane's wing is not active
 *  @param iw the index of the wing
 *  @return a pointer to the wing, or NULL if none;
 */
Wing *Plane::wing(xfl::enumWingType wingType)
{
    switch(wingType)
    {
        case xfl::MAINWING:     return wing();
        case xfl::SECONDWING:   return wing2();
        case xfl::ELEVATOR:     return stab();
        case xfl::FIN:          return fin();
        default:                return nullptr;
    }
}


/** Returns a pointer to the Plane's wing with index iw, or NULL if none has been defined.  */
Wing *Plane::wing(int iw)
{
    if(iw==0)    return m_Wing;
    else if (iw==1)
    {
        return m_bBiplane ? m_Wing+1 : nullptr;
    }
    else if (iw==2)
    {
        return m_bStab    ? m_Wing+2 : nullptr;
    }
    else if (iw==3)
    {
        return m_bFin     ? m_Wing+3 : nullptr;
    }
    return nullptr;
}


/** Returns a pointer to the Plane's wing with index iw, or NULL if none has been defined.  */
Wing const *Plane::wingAt(int iw) const
{
    if(iw==0)    return m_Wing;
    else if (iw==1)
    {
        return m_bBiplane ? m_Wing+1 : nullptr;
    }
    else if (iw==2)
    {
        return m_bStab    ? m_Wing+2 : nullptr;
    }
    else if (iw==3)
    {
        return m_bFin     ? m_Wing+3 : nullptr;
    }
    return nullptr;
}


/**
 * Loads or Saves the data of this Plane to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Plane::serializePlaneWPA(QDataStream &ar, bool bIsStoring)
{
    int nMass=0;
    float f=0,g=0,h=0;

    QString strong = "";
    int ArchiveFormat;// identifies the format of the file
    if (bIsStoring)
    {
        //to storing anymore to wpa format
        return true;
    }
    else
    {    // loading code
        int k;
        //1013 : v6.09.06 added pbody serialization
        //1012 : QFLR5 v0.03 : added mass properties for inertia calculations
        //1011 : QFLR5 v0.02 : added Plane description field
        //1010 : added body LE x and z position
        //1009 : added Main wing LE x and z position
        //1008 : added body data
        //1007 : added second wing data, CheckPanel
        //1006 : Converted lengths to m
        //1005 : stored double sym fin
        //1004 : suppressed colors
        //1003 : Added fin tilt;
        //1002 : Added doublefin;
        ar >> ArchiveFormat;
        if (ArchiveFormat <1001 || ArchiveFormat>1100)
        {
            m_Name = "";
            return false;
        }

        xfl::readCString(ar,m_Name);
        if (m_Name.length() ==0) return false;

        if(ArchiveFormat>=1011) xfl::readCString(ar, m_PlaneDescription);

        m_Wing[0].serializeWingWPA(ar, false);
        if(ArchiveFormat>=1007) m_Wing[1].serializeWingWPA(ar, false);
        m_Wing[2].serializeWingWPA(ar, false);
        m_Wing[3].serializeWingWPA(ar, false);

        ar >>k;
        if(k) m_bStab = true; else m_bStab = false;
        ar >>k;
        if(k) m_bFin = true;  else m_bFin = false;

        if(ArchiveFormat>=1002)
        {
            ar >>k;
            if(k) m_Wing[3].m_bDoubleFin = true;  else m_Wing[3].m_bDoubleFin = false;

            ar >>k;
            if(k) m_Wing[3].m_bSymFin = true;  else m_Wing[3].m_bSymFin = false;
        }
        if(ArchiveFormat>=1005)
        {
            ar >>k;
            //            if(k) m_bDoubleSymFin = true;  else m_bDoubleSymFin = false;
            //            m_Wing[3].m_bDoubleSymFin = m_bDoubleSymFin;
        }
        if(ArchiveFormat>=1007)
        {
            ar >>k;
            //            if(k) m_bCheckPanels = true;  else m_bCheckPanels = false;
            ar >>k;
            if(k) m_bBiplane = true;  else m_bBiplane = false;
            ar >> m_WingLE[1].x >> m_WingLE[1].y >> m_WingLE[1].z >> m_WingTiltAngle[1];
        }
        ar >>  m_WingLE[2].x >>  m_WingLE[2].y >>  m_WingLE[2].z;
        ar >> m_WingTiltAngle[0] >> m_WingTiltAngle[2];
        if(ArchiveFormat>=1003){
            ar >> m_WingTiltAngle[3];
        }
        ar >> m_WingLE[3].x >> m_WingLE[3].y >> m_WingLE[3].z;

        if(ArchiveFormat>=1010)
        {
            ar >> m_BodyPos.x >> m_BodyPos.z;
        }
        if(ArchiveFormat>=1009)
        {
            ar>> m_WingLE[0].x >> m_WingLE[0].z;
        }

        if(ArchiveFormat<1004)
        {
            int r,g,b;
            xfl::readCOLORREF(ar,r,g,b);
            xfl::readCOLORREF(ar,r,g,b);
            xfl::readCOLORREF(ar,r,g,b);
        }

        if(ArchiveFormat<1006)
        {
            //convert to m
            m_WingLE[2].x /= 1000.0;
            m_WingLE[2].y /= 1000.0;
            m_WingLE[2].z /= 1000.0;
            m_WingLE[3].x  /= 1000.0;
            m_WingLE[3].y  /= 1000.0;
            m_WingLE[3].z  /= 1000.0;
        }
        if(ArchiveFormat>=1008)
        {
            ar >> k;
            if(k)  m_bBody=true; else m_bBody=false;
            xfl::readCString(ar,strong);
            m_BodyName = strong;
        }
        else m_bBody = false;

        if(ArchiveFormat>=1012)
        {
            ar >> nMass;
            QVector<double> mass(nMass, 0);
            QVector<Vector3d> position(nMass);
            QStringList tag;


            for(int im=0; im<nMass; im++)
            {
                ar >> f;
                mass[im] = double(f);
            }
            for(int im=0; im<nMass; im++)
            {
                ar >> f >> g >> h;
                position[im] = Vector3d(double(f),double(g),double(h));
            }
            for(int im=0; im<nMass; im++)
            {
                tag << "";
                xfl::readCString(ar, tag[im]);
            }

            clearPointMasses();
            for(int im=0; im<nMass; im++)
            {
                m_PointMass.append(PointMass(mass.at(im), position.at(im), tag.at(im)));
            }
            tag.clear();
        }

        if(ArchiveFormat>=1013)
        {
            if(m_bBody)
            {
                m_Body.serializeBodyWPA(ar, false);
            }
        }

        computePlane();

        return true;
    }
}


/**
 * Loads or Saves the data of this Plane to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Plane::serializePlaneXFL(QDataStream &ar, bool bIsStoring)
{
    bool bl(false);
    int k(0);
    double dble(0), mass(0), px(0), py(0), pz(0);
    QString str;


    int ArchiveFormat=100002;// identifies the format of the file
    // 1000001: v648
    // 1000002: v649, added the style
    if (bIsStoring)
    {
        // storing code
        ar << ArchiveFormat;

        ar << m_Name;
        ar << m_PlaneDescription;

        m_theStyle.serializeFl5(ar, bIsStoring);

        m_Wing[0].serializeWingXFL(ar, bIsStoring);
        m_Wing[1].serializeWingXFL(ar, bIsStoring);
        m_Wing[2].serializeWingXFL(ar, bIsStoring);
        m_Wing[3].serializeWingXFL(ar, bIsStoring);

        bool bl = false;
        ar << m_bBiplane<< m_bStab <<m_bFin << m_Wing[3].m_bDoubleFin << m_Wing[3].m_bSymFin << bl;//m_bDoubleSymFin;

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            ar << m_WingLE[iw].x << m_WingLE[iw].y<< m_WingLE[iw].z << m_WingTiltAngle[iw];
        }

        ar << m_bBody;
        ar << m_BodyPos.x << m_BodyPos.z;
        if(m_bBody)
        {
            ar << m_BodyName;
            m_Body.serializeBodyXFL(ar, true);
        }

        ar << m_PointMass.size();
        for(int i=0; i<m_PointMass.size(); i++)
        {
            ar << m_PointMass.at(i).mass();
            ar << m_PointMass.at(i).position().x << m_PointMass.at(i).position().y << m_PointMass.at(i).position().z;
            ar << m_PointMass.at(i).tag();
        }

        // space allocation for the future storage of more data, without need to change the format
        k=0;
        for (int i=0; i<20; i++) ar << k;
        dble=0;
        for (int i=0; i<50; i++) ar << dble;

        return true;
    }
    else
    {    // loading code
        ar >> ArchiveFormat;
        if (ArchiveFormat <100001 || ArchiveFormat>110000)
        {
            return false;
        }

        ar >> m_Name;
        ar >> m_PlaneDescription;

        if(ArchiveFormat>=100002)  m_theStyle.serializeFl5(ar, bIsStoring);

        m_Wing[0].serializeWingXFL(ar, bIsStoring);
        m_Wing[1].serializeWingXFL(ar, bIsStoring);
        m_Wing[2].serializeWingXFL(ar, bIsStoring);
        m_Wing[3].serializeWingXFL(ar, bIsStoring);

        m_Wing[0].setWingType(xfl::MAINWING);
        m_Wing[1].setWingType(xfl::SECONDWING);
        m_Wing[2].setWingType(xfl::ELEVATOR);
        m_Wing[3].setWingType(xfl::FIN);

        ar >> m_bBiplane>> m_bStab >>m_bFin >> m_Wing[3].m_bDoubleFin >> m_Wing[3].m_bSymFin >> bl; // m_bDoubleSymFin;

        for(int iw=0; iw<MAXWINGS; iw++)
        {
            ar >> px >> py >> pz >> dble;
            // correcting past errors
/*            qDebug()<<qIsFinite(px)<<qIsFinite(py)<<qIsFinite(pz);
            qDebug()<<qIsInf(px)<<qIsInf(py)<<qIsInf(pz);
            qDebug()<<qIsNaN(px)<<qIsNaN(py)<<qIsNaN(pz);*/
            if(std::isnan(px))   px = 0.0;
            if(std::isnan(py))   py = 0.0;
            if(std::isnan(pz))   pz = 0.0;
            if(std::isnan(dble)) dble = 0.0;
            if(fabs(px)  <LENGTHPRECISION) px = 0.0;
            if(fabs(py)  <LENGTHPRECISION) py = 0.0;
            if(fabs(pz)  <LENGTHPRECISION) pz = 0.0;
            if(fabs(dble)<LENGTHPRECISION) dble = 0.0;
            if(fabs(px)  >1000.0) px = 0.0;
            if(fabs(py)  >1000.0) py = 0.0;
            if(fabs(pz)  >1000.0) pz = 0.0;
            if(fabs(dble)>1000.0) dble = 0.0;

            m_WingLE[iw].set(px, py, pz);
            m_WingTiltAngle[iw] = dble;
        }

        ar >> m_bBody;
        ar >> m_BodyPos.x >> m_BodyPos.z;
        if(m_bBody)
        {
            ar >> m_BodyName;
            m_Body.serializeBodyXFL(ar, bIsStoring);
        }

        clearPointMasses();
        ar >> k;
        for(int i=0; i<k; i++)
        {
            ar >> mass >> px >>py >> pz;
            ar >> str;
            m_PointMass.append({mass, Vector3d(px, py, pz), str});
        }

        // space allocation
        for (int i=0; i<20; i++) ar >> k;
        for (int i=0; i<50; i++) ar >> dble;

        computePlane();

        return true;
    }
}


int Plane::spanStationCount()
{
    // check the number of span spations
    int nSpanStations = 0;
    for(int iw=0; iw<MAXWINGS; iw++)
    {
        if(wing(iw))    nSpanStations = wing()->m_NStation;
    }
    return nSpanStations;
}


QString Plane::planeData(bool) const
{
    QString props;
    QString str1, str, strong;

    QString length, surface;
    Units::getLengthUnitLabel(length);
    Units::getAreaUnitLabel(surface);

    str1 = QString(QObject::tr("Wing Span      =")+"%1 ").arg(planformSpan()*Units::mtoUnit(),10,'f',3);
    str1 += length;
    props += str1+"\n";

    str1 = QString(QObject::tr("xyProj. Span   =")+"%1 ").arg(projectedSpan()*Units::mtoUnit(),10,'f',3);
    str1 += length;
    props += str1+"\n";

    str1 = QString(QObject::tr("Wing Area      =")+"%1 ").arg(planformArea() * Units::m2toUnit(),10,'f',3);
    str1 += surface;
    props += str1+"\n";

    str1 = QString(QObject::tr("xyProj. Area   =")+"%1 ").arg(projectedArea() * Units::m2toUnit(),10,'f',3);
    str1 += surface;
    props += str1+"\n";

    Units::getMassUnitLabel(str);
    str1 = QString(QObject::tr("Plane Mass     =")+"%1 ").arg(totalMass()*Units::kgtoUnit(),10,'f',3);
    str1 += str;
    props += str1+"\n";

    Units::getAreaUnitLabel(strong);
    str1 = QString(QObject::tr("Wing Load      =")+"%1 ").arg(totalMass()*Units::kgtoUnit()/projectedArea()/Units::m2toUnit(),10,'f',3);
    str1 += str + "/" + strong;
    props += str1+"\n";

    if(m_bBiplane) str1 = QString(QObject::tr("Tail Volume    =")+"%1").arg(tailVolume(),10,'f',3);


    str1 = QString(QObject::tr("Root Chord     =")+"%1 ").arg(m_Wing[0].rootChord()*Units::mtoUnit(), 10,'f', 3);
    str1 = str1+length;
    props += str1+"\n";

    str1 = QString(QObject::tr("MAC            =")+"%1 ").arg(mac()*Units::mtoUnit(), 10,'f', 3);
    str1 = str1+length;
    props += str1+"\n";

    str1 = QString(QObject::tr("TipTwist       =")+"%1").arg(m_Wing[0].tipTwist(), 10,'f', 3) + QChar(0260);
    props += str1+"\n";

    str1 = QString(QObject::tr("Aspect Ratio   =")+"%1").arg(aspectRatio(),10,'f',3);
    props += str1+"\n";

    str1 = QString(QObject::tr("Taper Ratio    =")+"%1").arg(taperRatio(),10,'f',3);
    props += str1+"\n";

    str1 = QString(QObject::tr("Root-Tip Sweep =")+"%1").arg(m_Wing[0].averageSweep(), 10,'f',3) + QChar(0260);
    props += str1;

    return props;
}
