/****************************************************************************

    Wing Class
    Copyright (C) 2005-2016 Andre Deperrois

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


#include <math.h>
#include <QFile>
#include <QtDebug>


#include <objects/objects3d/wing.h>
#include <objects/objects3d/wpolar.h>
#include <objects/objects3d/surface.h>
#include <objects/objects3d/panel.h>
#include <objects/objects3d/pointmass.h>
#include <objects/objects_global.h>
#include <objects/objects2d/polar.h>

double Wing::s_MinPanelSize = 0.0001;
QVector<Foil *> *Wing::s_poaFoil  = nullptr;
QVector<Polar*> *Wing::s_poaPolar = nullptr;

/**
 * The public constructor.
 */
Wing::Wing()
{
    memset(m_Ai,    0, sizeof(m_Ai));
    memset(m_Twist, 0, sizeof(m_Twist));
    memset(m_Cl,    0, sizeof(m_Cl));
    memset(m_PCd,   0, sizeof(m_PCd));
    memset(m_ICd,   0, sizeof(m_ICd));
    memset(m_Cm,    0, sizeof(m_Cm));
    memset(m_CmPressure,  0, sizeof(m_CmPressure));
    memset(m_XCPSpanAbs, 0, sizeof(m_XCPSpanAbs));
    memset(m_XCPSpanRel, 0, sizeof(m_XCPSpanRel));
    memset(m_Re,     0, sizeof(m_Re));
    memset(m_Chord,  0, sizeof(m_Chord));
    memset(m_Offset, 0, sizeof(m_Offset));
    memset(m_XTrTop, 0, sizeof(m_XTrTop));
    memset(m_XTrBot, 0, sizeof(m_XTrBot));
    memset(m_BendingMoment, 0, sizeof(m_BendingMoment));

    memset(m_SpanPos, 0, sizeof(m_SpanPos));
    memset(m_StripArea, 0, sizeof(m_StripArea));

    m_xHinge.clear();
    m_xHinge.insert(0, 1000, 0);
    m_xPanel.clear();
    m_xPanel.insert(0, 1000, 0);

    m_CoG.set(0.0,0.0,0.0);
    m_CoGIxx = m_CoGIyy = m_CoGIzz = m_CoGIxz = 0.0;
    m_VolumeMass = m_TotalMass = 0.0;

    clearPointMasses();

    m_bTextures     = false;
    m_bIsFin        = false;
    m_bDoubleFin    = false;
    m_bSymFin       = false;
    m_bSymetric     = true;
    m_bWingOut      = false;

    m_WingName        = QObject::tr("Wing Name");
    m_WingType        = XFLR5::MAINWING;
    m_WingDescription = "";

    m_WingColor.setRed(  int((double(qrand())/double(RAND_MAX))*155)+100);
    m_WingColor.setGreen(int((double(qrand())/double(RAND_MAX))*155)+100);
    m_WingColor.setBlue( int((double(qrand())/double(RAND_MAX))*155)+100);

    m_QInf0    = 0.0;

    m_pWingPanel     = nullptr;

    m_WingCL            = 0.0;
    m_CDv               = 0.0;
    m_CDi               = 0.0;
    m_GYm               = 0.0;
    m_IYm               = 0.0;
    m_GCm               = 0.0;
    m_GRm               = 0.0;
    m_ICm               = 0.0;
    m_VCm               = 0.0;
    m_VYm               = 0.0;

    m_CP.set(0.0, 0.0, 0.0);

    m_Maxa = 0.0;
    //    m_AVLIndex = -(int)(qrand()/10000);//improbable value...

    m_MatSize   = 0;
    m_NStation  = 0;

    m_AR         = 0.0;// Aspect ratio
    m_TR         = 0.0;// Taper ratio
    m_GChord     = 0.0;// mean geometric chord
    m_MAChord    = 0.0;// mean aero chord
    m_yMac       = 0.0;
    m_ProjectedArea = 0.0;
    m_ProjectedSpan = 0.0;

    m_nFlaps =  0;
    clearWingSections();
    appendWingSection(.180, .0, 0.0, 0.0, 0.000, 13, 19, XFLR5::COSINE, XFLR5::INVERSESINE, "", "");
    appendWingSection(.110, .0, 1.0, 0.0, 0.070, 13, 5,  XFLR5::COSINE,     XFLR5::UNIFORM, "", "");

    computeGeometry();

    double length = Length(0);
    for (int is=0; is<m_WingSection.size(); is++)
    {
        length += Length(is);
        YPosition(is)  = length;
        XPanelDist(is) =  XFLR5::COSINE;
    }
}


/** The public destructor */
Wing::~Wing()
{
    clearWingSections();
    clearPointMasses();
    clearSurfaces();
}


/** Destroys the WingSection objects in good order to avoid memory leaks */
void Wing::clearWingSections()
{
    for(int iws=m_WingSection.size()-1; iws>=0; iws--)
    {
        delete m_WingSection.at(iws);
        m_WingSection.removeAt(iws);
    }
}


/** Destroys the WingSection objects in good order to avoid memory leaks */
void Wing::clearSurfaces()
{
    for(int is=m_Surface.size()-1; is>=0; is--)
    {
        delete m_Surface.at(is);
        m_Surface.removeAt(is);
    }
}

/** Destroys the PointMass objects in good order to avoid memory leaks */
void Wing::clearPointMasses()
{
    for(int ipm=m_PointMass.size()-1; ipm>=0; ipm--)
    {
        delete m_PointMass.at(ipm);
        m_PointMass.removeAt(ipm);
    }
}


/**
 * Imports the wing geometry from a text file.
 * @param path_to_file the path to the filename as a QString
 */
bool Wing::importDefinition(QString path_to_file, QString errorMessage)
{
    QFile fp(path_to_file);
    double ypos;
    double chord;
    double offset;
    double dihedral;
    double twist;
    int nx;
    int ny;
    int px, py;
    XFLR5::enumPanelDistribution x_pan_dist;
    XFLR5::enumPanelDistribution y_pan_dist;
    char right_buff[512];
    char left_buff[512];

    unsigned counter = 0;


    try{
        if (!fp.open(QIODevice::ReadOnly))
        {
            errorMessage = "Could not open the file for reading";
            return false;
        }
        else {
            QTextStream infile(&fp);
            clearWingSections();
            this->m_WingName = infile.readLine();
            while (true)
            {
                counter++;
                infile >> ypos >> chord >> offset >> dihedral >> twist >> nx >> ny;

                infile >> px >> py;

                if(px ==2)         x_pan_dist  = XFLR5::INVERSESINE;
                else if(px ==  1)  x_pan_dist  = XFLR5::COSINE;
                else if(px == -2)  x_pan_dist  = XFLR5::SINE;
                else               x_pan_dist  = XFLR5::UNIFORM;

                if(py ==2)         y_pan_dist  = XFLR5::INVERSESINE;
                else if(py ==  1)  y_pan_dist  = XFLR5::COSINE;
                else if(py == -2)  y_pan_dist  = XFLR5::SINE;
                else               y_pan_dist  = XFLR5::UNIFORM;

                infile >> right_buff >> left_buff;

                if (infile.atEnd())
                {
                    fp.close();
                    break;
                }
                //Append the sections convert from mm to m
                appendWingSection(chord,
                                  twist,
                                  ypos,
                                  dihedral,
                                  offset,
                                  nx,
                                  ny,
                                  x_pan_dist,
                                  y_pan_dist,
                                  QString(QString(QLatin1String(right_buff)).replace(QString("/_/"), QString(" "))),
                                  QString(QString(QLatin1String(left_buff)).replace(QString("/_/"), QString(" ")))
                                  );

            }
        }

        //Build the Geometry
        computeGeometry();
        double length = Length(0);
        for (int is=0; is<m_WingSection.size(); is++)
        {
            length += Length(is);
            YPosition(is)     = length;
            XPanelDist(is) =  XFLR5::COSINE;
        }
    }
    catch (iostream::failure e)
    {
        errorMessage = "Unable to import wing definition\n";
        return false;
    }
    return true;
}


/**
 * Exports the wing geometry to a text file.
 * @param path_to_file the path to the filename as a QString
 */
bool Wing::exportDefinition(QString path_to_file, QString errorMessage)
{
    try{
        QFile fp(path_to_file);
        if (!fp.open(QIODevice::WriteOnly)) {
            errorMessage = "Could not open the file for writing";
            return false;
        } else {
            QTextStream out_file(&fp);
            //Iterate the wing sections are write out the data...
            out_file << this->m_WingName << endl;
            for (int is=0; is<m_WingSection.size(); is++)
            {
                out_file << YPosition(is) << " " << Chord(is) << " " << Offset(is) \
                         << " " << Dihedral(is) << " " << Twist(is) << " " << NXPanels(is) \
                         << " " << NYPanels(is) << " ";

                switch(XPanelDist(is))
                {
                    case XFLR5::COSINE:
                        out_file <<  1;
                        break;
                    case XFLR5::SINE:
                        out_file <<  2;
                        break;
                    case XFLR5::INVERSESINE:
                        out_file << -2;
                        break;
                    default:
                        out_file <<  0; //XFLR5::UNIFORM
                        break;
                }

                out_file << " " ;

                switch(YPanelDist(is))
                {
                    case XFLR5::COSINE:
                        out_file <<  1;
                        break;
                    case XFLR5::SINE:
                        out_file <<  2;
                        break;
                    case XFLR5::INVERSESINE:
                        out_file << -2;
                        break;
                    default:
                        out_file <<  0; //XFLR5::UNIFORM
                        break;
                }


                if(rightFoil(is).isEmpty()){
                    out_file  << " " << "/_/";
                } else {
                    out_file  << " " << rightFoil(is).replace(QString(" "), QString("/_/")).toLatin1().data();
                }
                if(leftFoil(is).isEmpty()) {
                    out_file  << " " << "/_/";
                } else {
                    out_file  << " " << leftFoil(is).replace(QString(" "), QString("/_/")).toLatin1().data();
                }
                out_file << endl;
            }
            fp.close();
        }
    }
    catch (iostream::failure e){
        errorMessage = "Unable to import wing definition\n";
        return false;
    }
    return true;
}


/**
 * Calculates the properties of the wing based on the input data.
 * Stores the results in the member variables.
 * Enables the user to see the properties of the wing in real time as the geometry is modified.
 */
void Wing::computeGeometry()
{
    Foil *pFoilA, *pFoilB;
    double MinPanelSize;
    int is;

    double surface = 0.0;
    double xysurface = 0.0;
    Length(0) = 0.0;
    YProj(0)  = YPosition(0);
    for (is=1; is<NWingSection(); is++)
        Length(is) = YPosition(is) - YPosition(is-1);
    for (is=1; is<NWingSection(); is++)
    {
        YProj(is) = YProj(is-1) + Length(is) * cos(Dihedral(is-1)*PI/180.0);
    }

    m_PlanformSpan  = 2.0 * tipPos();
    m_ProjectedSpan = 0.0;
    m_MAChord = 0.0;
    m_yMac    = 0.0;

    for (is=0; is<NWingSection()-1; is++)
    {
        pFoilA = foil(rightFoil(is));
        pFoilB = foil(rightFoil(is+1));
        surface   += Length(is+1)*(Chord(is)+Chord(is+1))/2.0;//m2
        xysurface += (Length(is+1)*(Chord(is)+Chord(is+1))/2.0)*cos(Dihedral(is)*PI/180.0);
        m_ProjectedSpan += Length(is+1)*cos(Dihedral(is)*PI/180.0);

        m_MAChord += IntegralC2(YPosition(is), YPosition(is+1), Chord(is), Chord(is+1));
        m_yMac    += IntegralCy(YPosition(is), YPosition(is+1), Chord(is), Chord(is+1));
    }

    m_ProjectedSpan *=2.0;
    if(!m_bIsFin || m_bSymFin || m_bDoubleFin)
    {
        m_PlanformArea    = 2.0 * surface;
        m_ProjectedArea = 2.0 * xysurface;
        m_MAChord = m_MAChord * 2.0 / m_PlanformArea;
        m_yMac    = m_yMac    * 2.0 / m_PlanformArea;

        m_GChord  = m_PlanformArea/m_PlanformSpan;
        m_AR      = m_PlanformSpan*m_PlanformSpan/m_PlanformArea;
    }
    else
    {
        m_PlanformArea = surface;
        m_ProjectedArea = xysurface;
        m_MAChord = m_MAChord / m_PlanformArea;
        m_yMac    = m_yMac    / m_PlanformArea;

        m_GChord  = m_PlanformArea/m_PlanformSpan*2.0;
        m_AR      = m_PlanformSpan*m_PlanformSpan/m_PlanformArea/2.0;
    }
    if(tipChord()>0.0)    m_TR = rootChord()/tipChord();
    else                m_TR = 99999.0;

    //calculate the number of flaps
    m_nFlaps = 0;
    if(s_MinPanelSize>0.0) MinPanelSize = s_MinPanelSize;
    else                   MinPanelSize = m_PlanformSpan;

    for (int is=1; is<NWingSection(); is++)
    {
        pFoilA = foil(rightFoil(is-1));
        pFoilB = foil(rightFoil(is));
        if(pFoilA && pFoilB && (!m_bIsFin || (m_bIsFin && m_bSymFin) || (m_bIsFin && m_bDoubleFin)))
        {
            if(pFoilA->m_bTEFlap && pFoilB->m_bTEFlap && qAbs(YPosition(is)-YPosition(is-1))>MinPanelSize)    m_nFlaps++;
        }
        pFoilA = foil(leftFoil(is-1));
        pFoilB = foil(leftFoil(is));
        if(pFoilA && pFoilB)
        {
            if(pFoilA->m_bTEFlap && pFoilB->m_bTEFlap && qAbs(YPosition(is)-YPosition(is-1))>MinPanelSize)    m_nFlaps++;
        }
    }
}


#define NXSTATIONS 20
#define NYSTATIONS 40

/**
* Calculates and returns the inertia properties of the structure based on the object's mass and on the existing geometry
* The mass is assumed to have been set previously.
* Mass = mass of the structure, excluding point masses
* @param  &CoG a reference to the CoG point, as a result of the calculation
* @param  &CoGIxx xx axis component of the inertia tensor, calculated at the CoG
* @param  &CoGIyy yy axis component of the inertia tensor, calculated at the CoG
* @param  &CoGIzz zz axis component of the inertia tensor, calculated at the CoG
* @param  &CoGIxz xz axis component of the inertia tensor, calculated at the CoG
*/
void Wing::computeVolumeInertia(Vector3d &CoG, double &CoGIxx, double &CoGIyy, double &CoGIzz, double &CoGIxz)
{
    QVector<double> ElemVolume(NXSTATIONS*NYSTATIONS*m_Surface.size());
    QVector<Vector3d> PtVolume(NXSTATIONS*NYSTATIONS*m_Surface.size());

    double rho, LocalSpan, LocalVolume;
    double LocalChord,  LocalArea,  tau;
    double LocalChord1, LocalArea1, tau1;
    double xrel, xrel1, yrel, ElemArea;
    Vector3d ATop, ABot, CTop, CBot, PointNormal, Diag1, Diag2;
    Vector3d PtC4, Pt, Pt1, N;
    CoG.set(0.0, 0.0, 0.0);
    CoGIxx = CoGIyy = CoGIzz = CoGIxz = 0.0;

    //sanity check
    //    Vector3d CoGCheck(0.0,0.0,0.0);
    double CoGIxxCheck, CoGIyyCheck, CoGIzzCheck, CoGIxzCheck;
    CoGIxxCheck = CoGIyyCheck = CoGIzzCheck = CoGIxzCheck = 0.0;
    double recalcMass = 0.0;
    double recalcVolume = 0.0;
    double checkVolume = 0.0;

    computeGeometry();

    //the mass density is assumed to be homogeneous

    //the local mass is proportional to the chord x foil area
    //the foil's area is interpolated

    //we consider the whole wing, i.e. all left and right surfaces
    //note : in avl documentation, each side is considered separately

    //first get the CoG - necessary for future application of Huygens/Steiner theorem
    int p = 0;

    for (int j=0; j<m_Surface.size(); j++)
    {
        LocalSpan = m_Surface.at(j)->m_Length/double(NYSTATIONS);
        for (int k=0; k<NYSTATIONS; k++)
        {
            tau  = double(k)   / double(NYSTATIONS);
            tau1 = double(k+1) / double(NYSTATIONS);
            yrel = (tau+tau1)/2.0;

            m_Surface.at(j)->getSection(tau,  LocalChord,  LocalArea,  Pt);
            m_Surface.at(j)->getSection(tau1, LocalChord1, LocalArea1, Pt1);
            LocalVolume = (LocalArea+LocalArea1)/2.0 * LocalSpan;
            PtC4.x = (Pt.x + Pt1.x)/2.0;
            PtC4.y = (Pt.y + Pt1.y)/2.0;
            PtC4.z = (Pt.z + Pt1.z)/2.0;

            //            CoGCheck += LocalVolume * PtC4;
            for(int l=0; l<NXSTATIONS; l++)
            {
                //browse mid-section
                xrel  = 1.0 - 1.0/2.0 * (1.0-cos(double(l)   *PI /double(NXSTATIONS)));
                xrel1 = 1.0 - 1.0/2.0 * (1.0-cos(double(l+1)*PI /double(NXSTATIONS)));

                m_Surface.at(j)->getSurfacePoint(xrel, xrel, yrel,  TOPSURFACE, ATop, N);
                m_Surface.at(j)->getSurfacePoint(xrel, xrel, yrel,  BOTSURFACE, ABot, N);
                m_Surface.at(j)->getSurfacePoint(xrel1, xrel1, yrel, TOPSURFACE, CTop, N);
                m_Surface.at(j)->getSurfacePoint(xrel1, xrel1, yrel, BOTSURFACE, CBot, N);
                PtVolume[p] = (ATop+ABot+CTop+CBot)/4.0;
                Diag1 = ATop - CBot;
                Diag2 = ABot - CTop;
                PointNormal = Diag1 * Diag2;
                ElemArea = PointNormal.VAbs()/2.0;
                //    qDebug("elemarea  %17.7g", ElemArea);
                if(ElemArea>0.0) ElemVolume[p] = ElemArea * LocalSpan;
                else
                {
                    //qDebug("elemarea  %17.7g   %17.7g", ElemArea, PRECISION);
                    //no area, means that the foils have not yet been defined for this surface
                    // so just count a unit volume, temporary
                    ElemVolume[p] = 1.0;

                }
                checkVolume += ElemVolume[p];
                CoG.x += ElemVolume[p] * PtVolume[p].x;
                CoG.y += ElemVolume[p] * PtVolume[p].y;
                CoG.z += ElemVolume[p] * PtVolume[p].z;
                p++;
            }
        }
    }

    if(checkVolume>PRECISION) rho = m_VolumeMass/checkVolume;
    else                      rho = 0.0;

    if(checkVolume>0.0)  CoG *= 1.0/ checkVolume;
    else                 CoG.set(0.0, 0.0, 0.0);


    // CoG is the new origin for inertia calculation
    p=0;
    for (int j=0; j<m_Surface.size(); j++)
    {
        LocalSpan = m_Surface.at(j)->m_Length/double(NYSTATIONS);
        for (int k=0; k<NYSTATIONS; k++)
        {
            tau  = double(k)     / double(NYSTATIONS);
            tau1 = double(k+1) / double(NYSTATIONS);
            m_Surface.at(j)->getSection(tau,  LocalChord,  LocalArea,  Pt);
            m_Surface.at(j)->getSection(tau1, LocalChord1, LocalArea1, Pt1);

            LocalVolume = (LocalArea+LocalArea1)/2.0 * LocalSpan;

            PtC4.x = (Pt.x + Pt1.x)/2.0;
            PtC4.y = (Pt.y + Pt1.y)/2.0;
            PtC4.z = (Pt.z + Pt1.z)/2.0;

            CoGIxxCheck += LocalVolume*rho * ( (PtC4.y-CoG.y)*(PtC4.y-CoG.y) + (PtC4.z-CoG.z)*(PtC4.z-CoG.z) );
            CoGIyyCheck += LocalVolume*rho * ( (PtC4.x-CoG.x)*(PtC4.x-CoG.x) + (PtC4.z-CoG.z)*(PtC4.z-CoG.z) );
            CoGIzzCheck += LocalVolume*rho * ( (PtC4.x-CoG.x)*(PtC4.x-CoG.x) + (PtC4.y-CoG.y)*(PtC4.y-CoG.y) );
            CoGIxzCheck -= LocalVolume*rho * ( (PtC4.x-CoG.x)*(PtC4.z-CoG.z) );

            recalcMass   += LocalVolume*rho;
            recalcVolume += LocalVolume;

            for(int l=0; l<NXSTATIONS; l++)
            {
                //browse mid-section
                CoGIxx += ElemVolume[p]*rho * ( (PtVolume[p].y-CoG.y)*(PtVolume[p].y-CoG.y) + (PtVolume[p].z-CoG.z)*(PtVolume[p].z-CoG.z));
                CoGIyy += ElemVolume[p]*rho * ( (PtVolume[p].x-CoG.x)*(PtVolume[p].x-CoG.x) + (PtVolume[p].z-CoG.z)*(PtVolume[p].z-CoG.z));
                CoGIzz += ElemVolume[p]*rho * ( (PtVolume[p].x-CoG.x)*(PtVolume[p].x-CoG.x) + (PtVolume[p].y-CoG.y)*(PtVolume[p].y-CoG.y));
                CoGIxz -= ElemVolume[p]*rho * ( (PtVolume[p].x-CoG.x)*(PtVolume[p].z-CoG.z) );
                p++;
            }
        }
    }
}


/**
* Calculates the inertia tensor in geometrical (body) axis :
*  - adds the volume inertia AND the inertia of point masses of all components
*  - the body axis is the frame in which the geometry has been defined
*  - the origin is the Wing's CoG, taking into account all masses
*/
void Wing::computeBodyAxisInertia()
{
    Vector3d LA, VolumeCoG;
    double Ixx, Iyy, Izz, Ixz, VolumeMass;
    Ixx = Iyy = Izz = Ixz = VolumeMass = 0.0;

    //Get the volume inertia properties in the volume CoG frame of reference
    computeVolumeInertia(VolumeCoG, Ixx, Iyy, Izz, Ixz);
    m_TotalMass = m_VolumeMass;

    m_CoG = VolumeCoG *m_VolumeMass;

    // add point masses
    for(int im=0; im<m_PointMass.size(); im++)
    {
        m_TotalMass += m_PointMass[im]->mass();
        m_CoG       += m_PointMass[im]->position() * m_PointMass[im]->mass();
    }

    if(m_TotalMass>0.0) m_CoG = m_CoG/m_TotalMass;
    else                m_CoG.set(0.0,0.0,0.0);

    // The CoG position is now available, so calculate the inertia w.r.t the CoG
    // using Huygens theorem
    //LA is the displacement vector from the centre of mass to the new axis
    LA = m_CoG-VolumeCoG;
    m_CoGIxx = Ixx + m_VolumeMass * (LA.y*LA.y+ LA.z*LA.z);
    m_CoGIyy = Iyy + m_VolumeMass * (LA.x*LA.x+ LA.z*LA.z);
    m_CoGIzz = Izz + m_VolumeMass * (LA.x*LA.x+ LA.y*LA.y);
    m_CoGIxz = Ixz - m_VolumeMass *  LA.x*LA.z;

    //add the contribution of point masses to total inertia
    for(int im=0; im<m_PointMass.size(); im++)
    {
        LA = m_PointMass[im]->position() - m_CoG;
        m_CoGIxx += m_PointMass[im]->mass() * (LA.y*LA.y + LA.z*LA.z);
        m_CoGIyy += m_PointMass[im]->mass() * (LA.x*LA.x + LA.z*LA.z);
        m_CoGIzz += m_PointMass[im]->mass() * (LA.x*LA.x + LA.y*LA.y);
        m_CoGIxz -= m_PointMass[im]->mass() * (LA.x*LA.z);
    }
}


int Wing::NYPanels()
{
    double MinPanelSize;

    if(s_MinPanelSize>0.0) MinPanelSize = s_MinPanelSize;
    else                   MinPanelSize = 0.0;

    int ny = 0;
    for(int is=0; is<NWingSection()-1;is++)
    {
        double panelLength = fabs(YPosition(is)-YPosition(is+1));

        if (panelLength < MinPanelSize ||  panelLength<planformSpan()/1000.0/2.0)
        {
        }
        else
        {
            ny += m_WingSection.at(is)->m_NYPanels;
        }
    }
    return ny*2;
}


/**
 * Constructs the surface objects based on the WingSection data.
 * The position and orientation are defined in the plane object
 * The surfaces are constructed from root to tip, and re-ordered from let tip to right tip
 * One surface object for each of the wing's panels
 * A is the surface's left side, B is the right side
 * @param T the translation to be appied to the wing geometry
 * @param XTilt  the rotation in degrees around the x-axis; used in the case of fins
 * @param YTilt  the rotation in degrees arouns the y-axis; used for wing or elevator tilt
 */
void Wing::createSurfaces(Vector3d const &T, double XTilt, double YTilt)
{
    int nSurf;
    Vector3d PLA, PTA, PLB, PTB, offset, T1;
    Vector3d Trans(T.x, 0.0, T.z);
    Vector3d O(0.0,0.0,0.0);
    double MinPanelSize;

    QVector<Vector3d> VNormal(NWingSection());
    QVector<Vector3d> VNSide(NWingSection());

    if(s_MinPanelSize>0.0) MinPanelSize = s_MinPanelSize;
    else                   MinPanelSize = 0.0;

    m_MatSize = 0;

    //define the normal to each surface
    nSurf=0;
    VNormal[0].set(0.0, 0.0, 1.0);
    VNSide[0].set(0.0, 0.0, 1.0);

    for(int is=0; is<NWingSection()-1;is++)
    {
        double panelLength = qAbs(YPosition(is)-YPosition(is+1));

        if (panelLength < MinPanelSize ||  panelLength<planformSpan()/1000.0/2.0)
        {
        }
        else
        {
            VNormal[nSurf].set(0.0, 0.0, 1.0);
            VNormal[nSurf].rotateX(O, Dihedral(is));
            nSurf++;
        }
    }

    clearSurfaces();
    for(int jss=0; jss<nSurf; jss++)
    {
        m_Surface.append(new Surface);
    }

    if(!m_bIsFin || (m_bIsFin && m_bSymFin) || (m_bIsFin && m_bDoubleFin))
    {
        for(int jss=0; jss<nSurf; jss++)
        {
            m_Surface.append(new Surface);
        }
    }


    if(nSurf<=0)
    {
        return;
    }
    int NSurfaces = nSurf;


    //define the normals at panel junctions : average between the normals of the two connecting sufaces
    for(int jss=0; jss<nSurf; jss++)
    {
        VNSide[jss+1] = VNormal[jss]+VNormal[jss+1];
        VNSide[jss+1].normalize();
    }

    //we start with the center panel, moving towards the left wing tip
    //however, the calculations are written for surfaces ordered from left tip to right tip,
    //so we number them the opposite way
    nSurf = 0;
    int iSurf = NSurfaces-1;


    for(int jss=0; jss<NWingSection()-1; jss++)
    {
        double panelLength = qAbs(YPosition(jss)-YPosition(jss+1));
        if (panelLength < MinPanelSize ||  panelLength<planformSpan()/1000.0/2.0)
        {
        }
        else
        {
            m_Surface[iSurf]->m_pFoilA   = foil(leftFoil(jss+1));
            m_Surface[iSurf]->m_pFoilB   = foil(leftFoil(jss));

            m_Surface[iSurf]->m_Length   =  YPosition(jss+1) - YPosition(jss);

            PLA.x =  Offset(jss+1);         PLB.x =  Offset(jss);
            PLA.y = -YPosition(jss+1);      PLB.y = -YPosition(jss);
            PLA.z =  0.0;                   PLB.z =  0.0;
            PTA.x =  PLA.x+Chord(jss+1);    PTB.x = PLB.x+Chord(jss);
            PTA.y =  PLA.y;                    PTB.y = PLB.y;
            PTA.z =  0.0;                   PTB.z =  0.0;

            m_Surface[iSurf]->setCornerPoints(PLA, PTA, PLB, PTB);
            m_Surface[iSurf]->setNormal(); // is (0,0,1)

            m_Surface[iSurf]->rotateX(m_Surface[iSurf]->m_LB, -Dihedral(jss));
            m_Surface[iSurf]->NormalA.set(VNSide[nSurf+1].x, -VNSide[nSurf+1].y, VNSide[nSurf+1].z);
            m_Surface[iSurf]->NormalB.set(VNSide[nSurf].x,   -VNSide[nSurf].y,   VNSide[nSurf].z);

            m_Surface[iSurf]->m_TwistA   =  Twist(jss+1);
            m_Surface[iSurf]->m_TwistB   =  Twist(jss);
            m_Surface[iSurf]->setTwist();

            if(jss>0 && iSurf<m_Surface.count()-1)
            {
                //translate the surface to the left tip of the previous surface
                T1 = m_Surface[iSurf+1]->m_LA - m_Surface[iSurf]->m_LB;
                m_Surface[iSurf]->translate(0.0,T1.y,T1.z);
                //                m_Surface[is].m_LB = m_Surface[is+1].m_LA;
                //                m_Surface[is].m_TB = m_Surface[is+1].m_TA;
            }

            nSurf++;

            m_Surface[iSurf]->m_NXPanels = NXPanels(jss);
            m_Surface[iSurf]->m_NYPanels = NYPanels(jss);


            //AVL coding + invert XFLR5::SINE and -sine for left wing
            m_Surface[iSurf]->m_XDistType = XPanelDist(jss);
            if(YPanelDist(jss) == XFLR5::SINE)              m_Surface[iSurf]->m_YDistType = XFLR5::INVERSESINE;
            else if(YPanelDist(jss) ==  XFLR5::COSINE)      m_Surface[iSurf]->m_YDistType =  XFLR5::COSINE;
            else if(YPanelDist(jss) == XFLR5::INVERSESINE)  m_Surface[iSurf]->m_YDistType =  XFLR5::SINE;
            else                                            m_Surface[iSurf]->m_YDistType =  XFLR5::UNIFORM;

            m_Surface[iSurf]->createXPoints();
            m_Surface[iSurf]->setFlap();
            m_Surface[iSurf]->init();
            m_Surface[iSurf]->m_bIsLeftSurf   = true;
            m_Surface[iSurf]->m_bIsInSymPlane = false;
            m_Surface[iSurf]->m_innerSection = jss;
            m_Surface[iSurf]->m_outerSection = jss+1;
            --iSurf;
        }
    }
    m_Surface[NSurfaces-1]->m_bIsCenterSurf = true;//previous left center surface

    // we only need a right wing in the following cases
    //   - if it's an 'ordinary wing'
    //   - if it's a fin, symetrical about the fuselage x-axis
    //   - if it's a symetrical double fin
    if(!m_bIsFin || (m_bIsFin && m_bSymFin) || (m_bIsFin && m_bDoubleFin))
    {
        m_Surface[NSurfaces]->m_bIsCenterSurf   = true;//next right center surface
        iSurf = nSurf;
        for (int jss=0; jss<NWingSection()-1; jss++)
        {
            double panelLength = qAbs(YPosition(jss)-YPosition(jss+1));
            if (panelLength < MinPanelSize ||  panelLength<planformSpan()/1000.0/2.0)
            {
            }
            else
            {
                m_Surface[iSurf]->m_pFoilA   = foil(rightFoil(jss));
                m_Surface[iSurf]->m_pFoilB   = foil(rightFoil(jss+1));

                m_Surface[iSurf]->m_Length   =  YPosition(jss+1) - YPosition(jss);

                PLA.x = Offset(jss);        PLB.x = Offset(jss+1);
                PLA.y = YPosition(jss);     PLB.y = YPosition(jss+1);
                PLA.z = 0.0;                PLB.z = 0.0;
                PTA.x = PLA.x+Chord(jss);   PTB.x = PLB.x+Chord(jss+1);
                PTA.y = PLA.y;              PTB.y = PLB.y;
                PTA.z = 0.0;                PTB.z = 0.0;

                m_Surface[iSurf]->setCornerPoints(PLA, PTA, PLB, PTB);
                m_Surface[iSurf]->setNormal(); // is (0,0,1)

                m_Surface[iSurf]->rotateX(m_Surface[iSurf]->m_LA, Dihedral(jss));
                m_Surface[iSurf]->NormalA.set(VNSide[iSurf-nSurf].x,   VNSide[iSurf-nSurf].y,   VNSide[iSurf-nSurf].z);
                m_Surface[iSurf]->NormalB.set(VNSide[iSurf-nSurf+1].x, VNSide[iSurf-nSurf+1].y, VNSide[iSurf-nSurf+1].z);

                m_Surface[iSurf]->m_TwistA   =  Twist(jss);
                m_Surface[iSurf]->m_TwistB   =  Twist(jss+1);
                m_Surface[iSurf]->setTwist();

                if(jss>0 && iSurf>0)
                {
                    //translate the surface to the left tip of the previous surface and merge points
                    T1 = m_Surface[iSurf-1]->m_LB - m_Surface[iSurf]->m_LA ;
                    m_Surface[iSurf]->translate(0.0, T1.y, T1.z);
                    //                    m_Surface[is].m_LA = m_Surface[is-1].m_LB;
                    //                    m_Surface[is].m_TA = m_Surface[is-1].m_TB;
                }

                m_Surface[iSurf]->m_NXPanels = NXPanels(jss);
                m_Surface[iSurf]->m_NYPanels = NYPanels(jss);

                //AVL coding + invert XFLR5::SINE and -sine for left wing
                m_Surface[iSurf]->m_XDistType = XPanelDist(jss);
                m_Surface[iSurf]->m_YDistType = YPanelDist(jss);

                m_Surface[iSurf]->createXPoints();
                m_Surface[iSurf]->setFlap();
                m_Surface[iSurf]->init();
                m_Surface[iSurf]->m_bIsLeftSurf   = false;
                m_Surface[iSurf]->m_bIsRightSurf  = true;
                m_Surface[iSurf]->m_bIsInSymPlane = false;

                m_Surface[iSurf]->m_innerSection = jss;
                m_Surface[iSurf]->m_outerSection = jss+1;

                iSurf++;
            }
        }
    }

    Vector3d Or(0.0,0.0,0.0);
    if(!m_bIsFin || (m_bIsFin && m_bSymFin))
    {
        NSurfaces*=2;
        for (int jSurf=0; jSurf<NSurfaces; jSurf++)
        {
            m_Surface[jSurf]->rotateX(Or, XTilt);
            m_Surface[jSurf]->rotateY(Or, YTilt);
            m_Surface[jSurf]->translate(Trans);
            if(m_bIsFin && m_bSymFin)
            {
                m_Surface[jSurf]->m_bIsInSymPlane = true;
                //                m_Surface[jSurf]->m_bIsLeftSurf   = true;
                //                m_Surface[jSurf]->m_bIsRightSurf  = false;
            }
        }
        m_Surface[NSurfaces-1]->m_bIsTipRight = true;
    }
    else
    {
        if(m_bDoubleFin)
        {
            NSurfaces*=2;
            //rotate surfaces symetrically
            int ns2 = int(NSurfaces/2);
            offset.set(0.0, -T.y, 0.0);
            for(int jSurf=0; jSurf<ns2; jSurf++)
            {
                m_Surface[jSurf]->rotateX(Or, +XTilt);
                m_Surface[jSurf]->rotateZ(Or, YTilt);
                m_Surface[jSurf]->translate(Trans);
                m_Surface[jSurf]->translate(offset);
                m_Surface[jSurf]->m_bIsInSymPlane = false;
            }
            offset.y = -offset.y;
            for(int jSurf=ns2; jSurf< NSurfaces; jSurf++)
            {
                m_Surface[jSurf]->rotateX(Or, -XTilt);
                m_Surface[jSurf]->rotateZ(Or, -YTilt);
                m_Surface[jSurf]->translate(Trans);
                m_Surface[jSurf]->translate(offset);
                m_Surface[jSurf]->m_bIsInSymPlane = false;
            }
            /*            m_Surface[ns2-1]->m_bIsTipRight = true;
            m_Surface[ns2-1]->m_bIsTipLeft  = true;
            m_Surface[0]->m_bIsTipRight   = true;
            m_Surface[0]->m_bIsTipLeft    = true;

            m_Surface[ns2]->m_bIsTipRight        = true;
            m_Surface[ns2]->m_bIsTipLeft         = true;
            m_Surface[NSurfaces-1]->m_bIsTipRight  = true;
            m_Surface[NSurfaces-1]->m_bIsTipLeft   = true;*/
        }
        else
        {
            //Not a double fin, so just a simple single-sided fin
            for (int jSurf=0; jSurf<NSurfaces; jSurf++)
            {
                m_Surface[jSurf]->rotateX(Or, XTilt);
                m_Surface[jSurf]->rotateZ(Or, YTilt);
                m_Surface[jSurf]->translate(Trans);
                m_Surface[jSurf]->m_bIsLeftSurf   = true;
                m_Surface[jSurf]->m_bIsRightSurf  = false;
                m_Surface[jSurf]->m_bIsInSymPlane = true;
            }
        }
    }

    m_Surface[0]->m_bIsTipLeft              = true;
    if(NSurfaces>=1) m_Surface[NSurfaces-1]->m_bIsTipRight = true;

    if(NSurfaces>1)
    {
        m_Surface[int(NSurfaces/2)-1]->m_bJoinRight   = true;
        //check for a center gap greater than 1/10mm
        int nada = int(NSurfaces/2)-1;
        Q_ASSERT(nada>=0);
        if(YPosition(0)>0.0001)     m_Surface[int(NSurfaces/2)-1]->m_bJoinRight   = false;

        if(m_bIsFin && m_bDoubleFin) m_Surface[int(NSurfaces/2)-1]->m_bJoinRight   = false;
    }
}



/**
* Calculates the chord lengths at each position of the NStation defined by the LLT or the Panel analysis
*@param NStation the number of stations required by the analysis
*/
void Wing::computeChords(int NStation)
{
    int j,k,m;
    double y, yob, tau;
    double x0,y0,y1,y2;
    Vector3d C;

    int NSurfaces = m_Surface.size();

    if(NStation !=0)
    {//LLT based
        m_NStation = NStation;

        for (k=0; k<=NStation; k++)
        {
            yob   = cos(k*PI/NStation);
            y = qAbs(yob * m_PlanformSpan/2);
            for (int is=0; is<NWingSection(); is++)
            {
                if(YPosition(is) < y && y <=YPosition(is+1))
                {
                    tau = (y-YPosition(is))/(YPosition(is+1)-YPosition(is));
                    m_Chord[k]  = Chord(is)+(Chord(is+1)-Chord(is)) * tau;
                    m_Offset[k] = Offset(is)+(Offset(is+1)-Offset(is)) * tau;
                    break;
                }
            }
        }
    }
    else
    {
        //VLM Mesh based
        QVector<double> SpanPosition;
        m_NStation = 0;
        m = 0;

        x0 = m_Surface[NSurfaces/2]->m_LA.x;
        y0 = m_Surface[NSurfaces/2]->m_LA.y;

        for (j=NSurfaces/2; j<NSurfaces; j++)
        {
            for (k=0; k<m_Surface.at(j)->m_NYPanels; k++)
            {
                //calculate span positions at each station
                m_Surface.at(j)->getYDist(k, y1, y2);
                SpanPosition.append(y0 + (y1+y2)/2.0*m_Surface.at(j)->m_Length);
                m++;
            }
            y0+=m_Surface.at(j)->m_Length;
        }

        m_NStation = 2*m;
        for (m=0; m<m_NStation/2; m++)
        {
            m_SpanPos[m] = -SpanPosition[m_NStation/2-m-1];
            m_SpanPos[m+m_NStation/2] = SpanPosition[m];
        }

        m=0;
        for (j=0; j<NSurfaces; j++)
        {
            for (k=0; k<m_Surface.at(j)->m_NYPanels; k++)
            {
                //calculate chords and offsets at each station
                m_Chord[m]     = m_Surface.at(j)->chord(k);
                //                m_StripArea[m] = m_Chord[m]* m_Surface.at(j)->Getdl(k);

                m_Surface.at(j)->getLeadingPt(k, C);
                m_Offset[m] = C.x-x0;

                m_Twist[m]  = m_Surface.at(j)->twist(k);
                Q_ASSERT(!std::isnan(m_Twist[m]));
                m++;
            }
        }
        m_NStation = m;
    }
}



/**
* Calculates the chord lengths at each position of the NStation defined by the LLT or the Panel analysis
* Overloaded function
*@param NStation the number of stations required by the analysis
*@param *chord pointer to the array of chords lengths at the span stations
*@param *offset pointer to the array of offset positions at the span stations
*@param *twist pointer to the array of twist values at the span stations
*/
void Wing::computeChords(int NStation, double *chord, double *offset, double *twist)
{
    if(NStation !=0)
    {//LLT based
        m_NStation = NStation;

        for (int k=0; k<=NStation; k++)
        {
            double yob   = cos(k*PI/NStation);
            double y = qAbs(yob * m_PlanformSpan/2);
            for (int is=0; is<NWingSection(); is++)
            {
                if(YPosition(is) < y && y <=YPosition(is+1))
                {
                    double tau = (y-YPosition(is))/(YPosition(is+1)-YPosition(is));
                    chord[k]  = Chord(is)  + (Chord(is+1) -Chord(is))  * tau;
                    offset[k] = Offset(is) + (Offset(is+1)-Offset(is)) * tau;
                    twist[k]  = Twist(is)  + (Twist(is+1) -Twist(is))  * tau;;
                    break;
                }
            }
        }
    }
}


/**
* Copies the gemetrical data from an existing Wing
*@param pWing a pointer to the instance of the source Wing object
*/
void Wing::duplicate(Wing *pWing)
{
    m_NStation      = pWing->m_NStation;
    m_PlanformSpan  = pWing->m_PlanformSpan;
    m_ProjectedSpan = pWing->m_ProjectedSpan;
    m_PlanformArea  = pWing->m_PlanformArea;
    m_ProjectedArea = pWing->m_ProjectedArea;
    m_AR            = pWing->m_AR;
    m_TR            = pWing->m_TR;
    m_GChord        = pWing->m_GChord;
    m_MAChord       = pWing->m_MAChord;
    m_WingName      = pWing->m_WingName;
    m_bSymetric     = pWing->m_bSymetric;
    m_bIsFin        = pWing->m_bIsFin;
    m_bSymFin       = pWing->m_bSymFin;
    m_bDoubleFin    = pWing->m_bDoubleFin;
    m_bTextures     = pWing->m_bTextures;

    clearWingSections();

    for (int is=0; is<pWing->m_WingSection.size(); is++)
    {
        appendWingSection();
        Chord(is)      = pWing->Chord(is);
        YPosition(is)  = pWing->YPosition(is);
        Offset(is)     = pWing->Offset(is);
        Length(is)     = pWing->Length(is);
        NXPanels(is)   = pWing->NXPanels(is) ;
        NYPanels(is)   = pWing->NYPanels(is);
        XPanelDist(is) = pWing->XPanelDist(is);
        YPanelDist(is) = pWing->YPanelDist(is);
        Twist(is)      = pWing->Twist(is);
        Dihedral(is)   = pWing->Dihedral(is);
        ZPosition(is)  = pWing->ZPosition(is);
        YProj(is)      = pWing->YProj(is);

        rightFoil(is)  = pWing->rightFoil(is);
        leftFoil(is)   = pWing->leftFoil(is);
    }

    computeGeometry();

    m_nFlaps = pWing->m_nFlaps;

    m_VolumeMass = pWing->m_VolumeMass;
    m_TotalMass  = pWing->m_TotalMass;
    m_CoG = pWing->m_CoG;
    m_CoGIxx = pWing->m_CoGIxx;
    m_CoGIyy = pWing->m_CoGIyy;
    m_CoGIzz = pWing->m_CoGIzz;
    m_CoGIxz = pWing->m_CoGIxz;

    clearPointMasses();

    for(int im=0; im<pWing->m_PointMass.size();im++)
    {
        m_PointMass.append(new PointMass(pWing->m_PointMass[im]->mass(), pWing->m_PointMass[im]->position(), pWing->m_PointMass[im]->tag()));
    }

    m_WingDescription = pWing->m_WingDescription;
    m_WingColor       = pWing->m_WingColor;
}





/**
* Returns the wing's average sweep from root to tip measured at the quarter chord
* The sweep is calulated as the arctangent of the root and tip quarter-chord points
*@return the value of the average sweep, in degrees
*/
double Wing::averageSweep()
{
    double xroot = rootOffset() + Chord(0)/4.0;
    double xtip  = tipOffset()  + tipChord()/4.0;
    //    double sweep = (atan2(xtip-xroot, m_PlanformSpan/2.0)) * 180.0/PI;
    return (atan2(xtip-xroot, m_PlanformSpan/2.0)) * 180.0/PI;
}


/**
 * Returns the x-position of the quarter-chord point at a given span position, relative to a reference x-value
 *@param yob the span position where the quarter-chord point will be calculated
 *@param xRef the reference position
 *@return the quarter-chord position
 */
double Wing::C4(double yob, double xRef)
{
    double chord, offset, tau;
    double C4 = 0.0;
    double y = qAbs(yob*m_PlanformSpan/2.0);
    for(int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)<= y && y <=YPosition(is+1))
        {
            tau = (y - YPosition(is))/(YPosition(is+1)-YPosition(is));
            chord  = Chord(is)  + tau * (Chord(is+1) - Chord(is));
            offset = Offset(is) + tau * (Offset(is+1) - Offset(is));
            C4 = offset + chord/4.0 - xRef;
            return C4;
        }
    }
    return C4;
}

/**
 * Calculates and returns the chord length at a given relative span position
 * @param yob the relative span position in %, where the chord length will be calculated
 * @return the chord length
 */
double Wing::getChord(double yob)
{
    double chord = 0.0;
    double tau;
    double y;

    y= qAbs(yob*m_PlanformSpan/2.0);//geometry is symetric
    for(int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)<=y && y <=YPosition(is+1))
        {
            tau = (y - YPosition(is))/(YPosition(is+1)-YPosition(is));
            chord = Chord(is) + tau * (Chord(is+1) - Chord(is));
            return chord;
        }
    }
    return -1.0;
}


/**
 * Calculates and returns the offste value at a given relative span position
 * @param yob the relative span position in %, where the offset will be calculated
 * @return the offset value
 */
double Wing::getOffset(double yob)
{
    double tau, y;
    double offset = 0.0;

    y= qAbs(yob*m_PlanformSpan/2.0);
    for(int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)<= y && y <=YPosition(is+1))
        {
            tau = (y - YPosition(is))/(YPosition(is+1)-YPosition(is));
            offset = Offset(is) + tau * (Offset(is+1) - Offset(is));
            return offset;
        }
    }

    return -1.0;
}



/**
 * Calculates and returns the twist angle at a given relative span position
 * @param yob the relative position where the twist angle will be calculated
 * @return the twist angle in degrees
 */
double Wing::getTwist(double yob)
{
    double tau;

    double y= qAbs(yob*m_PlanformSpan/2.0);//geometry is symetric

    // calculate twist at each station
    if (y>=0.0)
    {
        //right wing
        for (int is=0; is<NWingSection()-1; is++)
        {
            if(YPosition(is) <= y && y <=YPosition(is+1))
            {
                tau = (y-YPosition(is))/(YPosition(is+1)-YPosition(is));
                return Twist(is)+(Twist(is+1)-Twist(is)) * tau;
            }
        }
    }
    return 0.0;
}



/**
 * Calculates and returns the dihedral angle at a given relative span position
 * @param yob the relative position where the dihedral angle will be calculated
 * @return the dihedral angle in degrees
 */
double Wing::getDihedral(double yob)
{
    double y= qAbs(yob*m_PlanformSpan/2.0);//geometry is symetric
    for(int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)<= y && y <=YPosition(is+1))
        {
            if(yob>=0) return Dihedral(is);
            else  return -Dihedral(is);
        }
    }
    return 0.0;
}

/**
 * Returns pointers to the left and right foils of a given span position, and the relative position of the span position between these two foils
 * @param pFoil0 the pointer to the pointer of the left foil
 * @param pFoil1 the pointer to the pointer of the right foil
 * @param y the reference span position
 * @param t the ratio between the position of the two foils
 */
void Wing::getFoils(Foil **pFoil0, Foil **pFoil1, double y, double &t)
{
    if  (y>0.0)
    {
        //search Right wing
        for (int is=0; is<NWingSection()-1; is++)
        {
            if (YPosition(is)<=y && y<=YPosition(is+1))
            {
                *pFoil0 = foil(rightFoil(is));
                *pFoil1 = foil(rightFoil(is+1));
                t = (y-YPosition(is))/(YPosition(is+1) - YPosition(is));
                return;
            }
        }
    }
    else
    {
        //search left wing
        y = -y;
        for (int is=0; is<NWingSection()-1; is++)
        {
            if (YPosition(is)<=y && y<YPosition(is+1))
            {
                *pFoil0 = foil(leftFoil(is));
                *pFoil1 = foil(leftFoil(is+1));
                t = (y-YPosition(is))/(YPosition(is+1) - YPosition(is));
                return;
            }
        }
    }
    t = 0;
    pFoil0 = nullptr;// use linear
    pFoil1 = nullptr;// use linear
}


/**
 * Returns the total mass of the wing, as the sum of volume and point masses
 * @return the total mass
 */
double Wing::totalMass()
{
    double TotalMass = m_VolumeMass;
    for(int im=0; im<m_PointMass.size(); im++)
        TotalMass += m_PointMass[im]->mass();
    return TotalMass;
}



void Wing::surfacePoint(double xRel, double ypos, enumPanelPosition pos, Vector3d &Point, Vector3d &PtNormal)
{
    Surface *pSurface = nullptr;
    double fy = qAbs(ypos);

    int iSurf = m_Surface.size()/2;

    double yl = 0.0;

    for (int is=0; is<NWingSection()-1; is++)
    {
        if(qAbs(YPosition(is+1)-YPosition(is))>s_MinPanelSize)
        {
            if(YPosition(is)< fy && fy<=YPosition(is+1))
            {
                break;
            }
            yl += m_Surface.at(iSurf)->spanLength();
            iSurf++;
        }
    }

    pSurface = m_Surface.at(iSurf);
    double yRel = (fabs(ypos)-yl)/pSurface->spanLength();
    pSurface->getSurfacePoint(xRel, xRel, yRel, pos, Point, PtNormal);

    if(ypos<0) Point.y = -Point.y;
}


/**
 * Returns the relative position in % of a given absolute span position
 * @param SpanPos the absolute span position
 * @return the relative position, in %
 */
double Wing::yrel(double SpanPos)
{
    double y = qAbs(SpanPos);
    for(int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)<=y && y <YPosition(is+1))
        {
            if(SpanPos>0) return  (y-YPosition(is))/(YPosition(is+1)-YPosition(is));
            else          return  (y-YPosition(is+1))/(YPosition(is)-YPosition(is+1));
        }
    }
    return 1.0;
}

/**
 * The z-position of a specified absolute span position.
 * Used for moment evaluations in LLT, where the wing is defined as a 2D planform
 * @param y the abolute span position
 * @return the absolute z-position
 */
double Wing::ZPosition(double y)
{
    double tau;
    double ZPos =0.0;

    y= qAbs(y);
    if(y<=0.0) return 0.0;
    for (int is=0; is<NWingSection()-1; is++)
    {
        if(YPosition(is)< y && y<=YPosition(is+1))
        {
            for (int iss=0; iss<is; iss++)
            {
                ZPos+=Length(iss+1) * sin(Dihedral(iss)*PI/180.0);
            }
            tau = (y - YPosition(is))/(YPosition(is+1)-YPosition(is));
            ZPos += tau * Length(is+1) * sin(Dihedral(is)*PI/180.0);
            return ZPos;
        }
    }
    return 0.0;
}


/**
 * Computes the bending moment at each span position based on the results of the panel analysis
 * Assumes the array of force vectors has been calculated previously
 * @param bThinSurface true if the calculation has been performed on thin VLM surfaces, false in the case of a 3D-panelanalysis
 */
void Wing::panelComputeBending(bool bThinSurface)
{
    QVector<double> ypos, zpos;
    int j,k,jj,coef,p;
    double bm;
    Vector3d Dist(0.0,0.0,0.0);
    Vector3d Moment;


    if(bThinSurface)
    {
        coef = 1;
        p    = 0;
    }
    else
    {
        coef = 2;
        p= m_Surface[0]->m_NXPanels;
    }

    int NSurfaces = m_Surface.size();

    for (j=0; j<NSurfaces; j++)
    {
        for (k=0; k<m_Surface.at(j)->m_NYPanels; k++)
        {
            if(!bThinSurface)
            {
                ypos.append(m_pWingPanel[p].CollPt.y);
                zpos.append(m_pWingPanel[p].CollPt.z);
            }
            else
            {
                ypos.append(m_pWingPanel[p].VortexPos.y);
                zpos.append(m_pWingPanel[p].Vortex.z);
            }

            p += coef*m_Surface.at(j)->m_NXPanels;
        }
    }

    for (j=0; j<m_NStation; j++)
    {
        bm = 0.0;
        if (ypos[j]<=0)
        {
            for (jj=0; jj<j; jj++)
            {
                Dist.y =  -ypos[jj]+ypos[j];
                Dist.z =  -zpos[jj]+zpos[j];
                Moment = Dist * m_F[jj];
                bm += Moment.x ;
            }
        }
        else
        {
            for (jj=j+1; jj<m_NStation; jj++)
            {
                Dist.y =  ypos[jj]-ypos[j];
                Dist.z =  zpos[jj]-zpos[j];
                Moment = Dist * m_F[jj];
                bm += Moment.x ;
            }
        }
        m_BendingMoment[j] = bm;
    }
}

/**
* Scales the wing chord-wise so that the root chord is set to the NewChord value
*@param NewChord the new value of the root chord
*/
void Wing::scaleChord(double NewChord)
{
    double ratio = NewChord/Chord(0);
    for (int is=0; is<m_WingSection.size(); is++)
    {
        Chord(is)    *= ratio;
        Offset(is)   *= ratio;
    }
    computeGeometry();
}


/**
* Scales the wing span-wise so that the span is set to the NewSpan value
*@param NewSpan the new value of the span
*/
void Wing::scaleSpan(double NewSpan)
{
    for (int is=0; is<m_WingSection.size(); is++)
    {
        YPosition(is)      *= NewSpan/m_PlanformSpan;
        Length(is)   *= NewSpan/m_PlanformSpan;
    }
    computeGeometry();
}


/**
* Scales the wing's sweep so that the sweep is set to the NewSweep value
* @param newSweep the new value of the average quarter-chord sweep, in degrees
*/
void Wing::scaleSweep(double newSweep)
{
    double rootOffset = m_WingSection.first()->m_Offset;
    double rootchord4 = rootOffset + Chord(0)/4.0;

    //scale each panel's offset
    for(int is=1; is<NWingSection(); is++)
    {
        double chord4Offset = rootchord4 + tan(newSweep*PI/180.0) * m_WingSection.at(is)->m_YPosition;
        Offset(is) = chord4Offset - Chord(is)/4.0;
    }
    computeGeometry();
}


/**
* Scales the wing's twist angles so that the tip twist is set to the NewTwist value.
*@param NewTwist the new value of the average quarter-chord twist, in degrees
*/
void Wing::scaleTwist(double NewTwist)
{
    if(fabs(tipTwist())>0.0001)
    {
        //scale each panel's twist
        double ratio = NewTwist/tipTwist();

        for(int is=1; is<NWingSection(); is++)
        {
            Twist(is) *= ratio;
        }
    }
    else
    {
        //Set each panel's twist in the ratio of the span position
        for(int is=1; is<NWingSection(); is++)
        {
            Twist(is) = NewTwist*YPosition(is)/(m_PlanformSpan/2.0);
        }
    }
    computeGeometry();
}


/**
* Scales the wing's area.
* All dimensions scaled proportionally sqrt(2).
* Useful e.g. when the weight is changed or when playing with Cl.
* @param newArea the new value of the wing's area.
*/
void Wing::scaleArea(double newArea)
{
    if(fabs(m_PlanformArea)<PRECISION) return;
    if(newArea<PRECISION) return;

    double ratio = sqrt(newArea/m_PlanformArea);

    for (int is=0; is<m_WingSection.size(); is++)
    {
        YPosition(is) *= ratio;
        Chord(is)     *= ratio;
    }
    computeGeometry();
}




/**
 * Returns the number of mesh panels defined on this Wing's surfaces; the number is given for a double-side mesh of the wing
 * @return the total number of panels
 */
int Wing::VLMPanelTotal(bool bThinSurface)
{
    double MinPanelSize;

    if(s_MinPanelSize>0.0) MinPanelSize = s_MinPanelSize;
    else                              MinPanelSize = m_PlanformSpan/1000.0;
    int total = 0;
    for (int is=0; is<NWingSection()-1; is++)
    {
        //do not create a surface if its length is less than the critical size
        if (qAbs(YPosition(is)-YPosition(is+1)) > MinPanelSize)    total += NXPanels(is)*NYPanels(is);
    }
    if(!m_bIsFin) total *=2;

    if(!bThinSurface)
    {
        total *= 2;
        total += 2*NXPanels(0); // assuming a XFLR5::UNIFORM number of chordwise panels over the wing
    }

    return total;
}


/**
*  Calculates the wing aero coefficients
*  Uses Cp distribution in input for thick surfaces
*  Uses Gamma distribution in input for VLM method
*
*  Input data:
*    Freestream speed Qinf
*    Angle of attack Alpha
*    Cp dstribution for thick wings
*    Mu or Gamma distribution, depending on the analysis type
*    Type of surface :
*        - Thin Surface, i.e. VLM type surfaces, with vortex distribution
*        - Thick Surfaces;, i.e. 3D Panels with source+doublet distribution on panels
*    Type of analysis : viscous or inviscid
*
*  Output
*    centre of pressure position (XCP, YCP)
*    moment coefficients GCm, VCm, ICm, GRm, GYm, VYm, IYm
*/
void Wing::panelComputeOnBody(double QInf, double Alpha, double *Cp, double *Gamma, double &XCP, double &YCP, double &ZCP,
                              double &GCm, double &VCm, double &ICm, double &GRm, double &GYm, double &VYm,double &IYm,
                              WPolar *pWPolar, Vector3d CoG)

{
    int  j, k, l, p, m, nFlap, coef;
    double CPStrip, tau, NForce, cosa, sina;
    Vector3d HingeLeverArm,  PtC4Strip, PtLEStrip, ForcePt, SurfaceNormal, LeverArmC4CoG, LeverArmPanelC4, LeverArmPanelCoG;
    Vector3d Force, panelforce, StripForce, viscousDragVector, panelmoment, HingeMoment, viscousDragMoment, GeomMoment;
    Vector3d WindNormal, WindDirection;
    Vector3d Origin(0.0,0.0,0.0);

    //initialize
    m_GRm =0.0;
    m_GCm = m_VCm = m_ICm = 0.0;
    m_GYm = m_VYm = m_IYm = 0.0;

    // Define the number of panels to consider on each strip
    if(pWPolar->bThinSurfaces()) coef = 1;    // only mid-surface
    else                         coef = 2;    // top and bottom surfaces

    // Define the wind axis
    cosa = cos(Alpha*PI/180.0);
    sina = sin(Alpha*PI/180.0);
    WindDirection.set( cosa, 0.0, sina);
    WindNormal.set(   -sina, 0.0, cosa);


    // Calculate the Reynolds number on each strip
    for (m=0; m< m_NStation; m++) m_Re[m] = m_Chord[m] * QInf /pWPolar->m_Viscosity;

    m = p = nFlap = 0;
    m_FlapMoment.clear();

    // For each of the wing's surfaces, calculate the coefficients on each strip
    // and sum them up to get the wing's overall coefficients

    int NSurfaces = m_Surface.size();

    for (j=0; j<NSurfaces; j++)
    {
        Surface *pSurf = m_Surface.at(j);
        //do not consider left tip patch, if any
        if(!pWPolar->bThinSurfaces() && pSurf->m_bIsTipLeft) p += pSurf->m_NXPanels;

        if(pSurf->m_bTEFlap) m_FlapMoment.append(0.0);

        SurfaceNormal = pSurf->Normal;

        // consider each strip in turn
        for (k=0; k<pSurf->m_NYPanels; k++)
        {
            //initialize
            viscousDragVector.set(0.0,0.0,0.0);
            StripForce.set(0.0,0.0,0.0);
            GeomMoment.set(0.0,0.0,0.0);

            m_CmPressure[m]    = 0.0;
            CPStrip        = 0.0;

            pSurf->getLeadingPt(k, PtLEStrip);
            pSurf->getC4(k, PtC4Strip, tau);
            if(fabs(pWPolar->m_BetaSpec)>0.0)
            {
                PtC4Strip.rotateZ(Origin, pWPolar->m_BetaSpec);
                PtLEStrip.rotateZ(Origin, pWPolar->m_BetaSpec);
            }

            LeverArmC4CoG = PtC4Strip - CoG;

            for (l=0; l<coef*pSurf->m_NXPanels; l++)
            {
                // Get the force acting on the panel
                if(m_pWingPanel[p].m_Pos!=MIDSURFACE)
                {
                    ForcePt = m_pWingPanel[p].CollPt;
                    panelforce = m_pWingPanel[p].Normal * (-Cp[p]) * m_pWingPanel[p].Area;      // Newtons/q
                }
                else
                {
                    // for each panel along the chord, add the lift coef
                    ForcePt = m_pWingPanel[p].VortexPos;
                    panelforce  = WindDirection * m_pWingPanel[p].Vortex;
                    panelforce *= 2.0 * Gamma[p] /QInf;                                 //Newtons/q

                    if(!pWPolar->bVLM1() && !m_pWingPanel[p].m_bIsLeading)
                    {
                        Force       = WindDirection * m_pWingPanel[p].Vortex;
                        Force      *= 2.0 * Gamma[p+1] /QInf;                          //Newtons/q
                        panelforce -= Force;
                    }
                    Cp[p] = panelforce.dot(m_pWingPanel[p].Normal)/m_pWingPanel[p].Area;    //
                }
                StripForce += panelforce;                                           // Newtons/q
                NForce = panelforce.dot(SurfaceNormal);                             // Newtons/q

                LeverArmPanelC4    = ForcePt - PtC4Strip;                           // m
                LeverArmPanelCoG   = ForcePt - CoG;                                 // m


                panelmoment = LeverArmPanelC4 * panelforce;                             // N.m/q
                m_CmPressure[m]  += panelmoment.y;                                      // N.m/q, w.r.t. quarter chord point

                GeomMoment += LeverArmPanelCoG * panelforce;                        // N.m/q

                XCP       += ForcePt.x * panelforce.dot(WindNormal); //global center of pressure
                YCP       += ForcePt.y * panelforce.dot(WindNormal);
                ZCP       += ForcePt.z * panelforce.dot(WindNormal);
                CPStrip   += ForcePt.x * NForce;

                if(pSurf->m_bTEFlap)
                {
                    if(pSurf->isFlapPanel(m_pWingPanel[p].m_iElement))
                    {
                        //then p is on the flap, so add its contribution
                        HingeLeverArm = ForcePt - pSurf->m_HingePoint;
                        HingeMoment = HingeLeverArm * panelforce;                   //N.m/q
                        m_FlapMoment[nFlap] += HingeMoment.dot(pSurf->m_HingeVector)* pWPolar->density() * QInf * QInf/2.0;  //N.m
                    }
                }
                p++;
            }

            // calculate center of pressure position
            NForce = StripForce.dot(SurfaceNormal);
            m_XCPSpanRel[m]    = (CPStrip/NForce - PtLEStrip.x)/m_Chord[m];
            m_XCPSpanAbs[m]    =  CPStrip/NForce ;

            // add viscous properties, if required
            if(pWPolar->bViscous()) viscousDragVector = WindDirection * (m_PCd[m]) * m_StripArea[m];   // N/q
            else                    viscousDragVector.set(0.0,0.0,0.0);

            // global moments
            viscousDragMoment =  LeverArmC4CoG * viscousDragVector;         // N.m/q, w.r.t. CoG

            m_GRm += GeomMoment.dot(WindDirection);

            m_VYm += viscousDragMoment.dot(WindNormal);                     // N.m/q

            //            m_IYm += -m_ICd[m] * m_StripArea[m] * PtC4Strip.y ;
            m_IYm += GeomMoment.dot(WindNormal);                            // N.m/q

            m_VCm += viscousDragMoment.y;                                   // N.m/q
            m_ICm += GeomMoment.y;                                          // N.m/q

            m_CmPressure[m] *= 1.0 /m_Chord[m]/m_StripArea[m];              // N.m/q, w.r.t. quarter chord point
            m_Cm[m] = (GeomMoment.y + viscousDragMoment.y)/m_Chord[m]/m_StripArea[m];   // N.m/q, w.r.t. CoG
            m++;
        }
        //do not consider right tip patch
        if(!pWPolar->bThinSurfaces() && pSurf->m_bIsTipRight) p += pSurf->m_NXPanels;
        if(pSurf->m_bTEFlap) nFlap++;
    }


    //global plane dimensionless coefficients
    GCm += m_VCm + m_ICm; // Pitching moment, sum of Viscous and Induced parts
    VCm += m_VCm;
    //qDebug("  VCm=%13.7f   %s", m_VCm, wingName().toStdString().c_str());

    ICm += m_ICm;

    //sign convention for rolling and yawing is opposite to algebric results
    GRm -= m_GRm; // Rolling moment, no induced contribution
    GYm -= (m_VYm + m_IYm); // Yawing moment, sum of Viscous and Induced parts
    VYm -= m_VYm;
    IYm -= m_IYm;
}



/**
* In input, takes the speed QInf and the distribution of lift coefficients m_Cl[] along the span
* In output, returns for each span station
*     - The Reynolds number m_Re[]
*    - The viscous drag coefficient m_PCd[]
*      - The top and bottom transition points m_XTrtop[] and m_XTrBot[]
*/
void Wing::panelComputeViscous(double QInf, WPolar *pWPolar, double &WingVDrag, bool bViscous, QString &OutString)
{
    QString string, strong, strLength;
    int m;
    bool bPointOutRe, bPointOutCl, bOutRe, bError;
    double tau = 0.0;
    Vector3d PtC4;

    OutString.clear();

    WingVDrag = 0.0;

    bOutRe = bError = bPointOutRe = bPointOutCl = false;

    strLength = "m";

    // Calculate the Reynolds number on each strip
    for (m=0; m<m_NStation; m++)  m_Re[m] = m_Chord[m] * QInf /pWPolar->m_Viscosity;

    if(!bViscous)
    {
        for(m=0; m <m_NStation; m++)
        {
            m_PCd[m] = m_XTrTop[m] = m_XTrBot[m] = 0.0;
        }
        return;
    }

    //Interpolate the viscous properties from the foil's type 1 polar mesh
    m=0;
    for (int j=0; j<m_Surface.size(); j++)
    {
        for(int k=0; k<m_Surface.at(j)->m_NYPanels; k++)
        {
            bOutRe = bPointOutRe = false;
            bPointOutCl = false;
            m_Surface.at(j)->getC4(k, PtC4, tau);

            m_PCd[m]    = getInterpolatedVariable(2, m_Surface.at(j)->m_pFoilA, m_Surface.at(j)->m_pFoilB, m_Re[m], m_Cl[m], tau, bOutRe, bError);
            bPointOutRe = bOutRe || bPointOutRe;
            if(bError) bPointOutCl = true;

            m_XTrTop[m] = getInterpolatedVariable(5, m_Surface.at(j)->m_pFoilA, m_Surface.at(j)->m_pFoilB, m_Re[m], m_Cl[m], tau, bOutRe, bError);
            bPointOutRe = bOutRe || bPointOutRe;
            if(bError) bPointOutCl = true;

            m_XTrBot[m] = getInterpolatedVariable(6, m_Surface.at(j)->m_pFoilA, m_Surface.at(j)->m_pFoilB, m_Re[m], m_Cl[m], tau, bOutRe, bError);
            bPointOutRe = bOutRe || bPointOutRe;
            if(bError) bPointOutCl = true;

            if(bPointOutCl)
            {
                strong = QString(QObject::tr("           Span pos = %1 ")).arg(m_SpanPos[m], 9,'f',2);
                strong += strLength;
                strong += ",  Re = ";
                string.sprintf("%.0f", m_Re[m]);
                strong += string;

                strong+= QString(QObject::tr(",  Cl = %1 could not be interpolated")+"\n").arg(m_Cl[m],6,'f',2);
                OutString += strong;

                m_bWingOut = true;
            }
            else if(bPointOutRe)
            {
                strong = QString(QObject::tr("           Span pos = %1 ")).arg(m_SpanPos[m],9,'f',2);
                strong += strLength;
                strong += ",  Re = ";
                string.sprintf("%.0f", m_Re[m]);
                strong += string;
                strong += QString(QObject::tr(",  Cl = %1 is outside the flight envelope")+"\n").arg(m_Cl[m],6,'f',2);
                OutString += strong;

                m_bWingOut = true;
            }

            // Sum the total viscous drag of this wing
            WingVDrag  += m_PCd[m] * m_StripArea[m];
            m++;
        }
    }
}


/**
 * Identifies if a given index of a panel belongs to this wing or not
 * @param nPanel the index of the panel
 * @return true if the panel belongs to the wing, false otherwise
 */
bool Wing::isWingPanel(int nPanel)
{
    for(int p=0; p<m_MatSize; p++)
    {
        if(nPanel==m_pWingPanel[p].m_iElement) return true;
    }
    return false;
}


/**
 * Identifies if a given index of a node belongs to this wing or not
 * @param nNode the index of a node
 * @return true if the node belongs to the wing, false otherwise
 */
bool Wing::isWingNode(int nNode)
{
    for(int p=0; p<m_MatSize; p++)
    {
        if(nNode==m_pWingPanel[p].m_iLA) return true;
        if(nNode==m_pWingPanel[p].m_iLB) return true;
        if(nNode==m_pWingPanel[p].m_iTA) return true;
        if(nNode==m_pWingPanel[p].m_iTB) return true;
    }
    return false;
}


/** Returns the offset at a span section identified by its index
*@param iSection the index of the section
*@return the value of the offset
*/
double & Wing::Offset(const int &iSection)    {return m_WingSection[iSection]->m_Offset;}

/** Returns the dihedral angle at a span section identified by its index
*@param iSection the index of the section
*@return the value of the dihedral angle, in degrees
*/
double & Wing::Dihedral(const int &iSection)  {return m_WingSection[iSection]->m_Dihedral;}

/** Returns the chord length at a span section identified by its index
*@param iSection the index of the section
*@return the value of the chord length
*/
double & Wing::Chord(const int &iSection)     {return m_WingSection[iSection]->m_Chord;}

/** Returns the twist angle at a span section identified by its index
*@param iSection the index of the section
*@return the value of the twist angle, in degrees
*/
double & Wing::Twist(const int &iSection)     {return m_WingSection[iSection]->m_Twist;}

/** Returns the span position at a span section identified by its index
*@param iSection the index of the section
*@return the value of the span position
*/
double & Wing::YPosition(const int &iSection) {return m_WingSection[iSection]->m_YPosition;}

/** Returns the length between a span section identified by its index and the next spanwise section
*@param iSection the index of the section
*@return the value of the length of the panel
*/
double & Wing::Length(const int &iSection)    {return m_WingSection[iSection]->m_Length;}

/** Returns the span position of a span section identified by its index, projected on the x-y plane
*@param iSection the index of the section
*@return the value of the projected span position
*/
double & Wing::YProj(const int &iSection)     {return m_WingSection[iSection]->m_YProj;}

/** Returns the z-position at a span section identified by its index
*@param iSection the index of the section
*@return the value of the z-position
*/
double & Wing::ZPosition(const int &iSection) {return m_WingSection[iSection]->m_ZPos;}


/** Returns the number of chordwise panels at a span section identified by its index
*@param iSection the index of the section
*@return the number of chordwise panels
*/
int & Wing::NXPanels(const int &iSection)   {return m_WingSection[iSection]->m_NXPanels;}

/** Returns the number of spanwise panels at a span section identified by its index
*@param iSection the index of the section
*@return the number of spanwise panels
*/
int & Wing::NYPanels(const int &iSection)   {return m_WingSection[iSection]->m_NYPanels;}

/** Returns the type of distribution of chordwise panels at a span section identified by its index - always XFLR5::COSINE type
*@param iSection the index of the section
*@return the type of distribution of chordwise panels - always XFLR5::COSINE type
*/
XFLR5::enumPanelDistribution & Wing::XPanelDist(const int &iSection) {return m_WingSection[iSection]->m_XPanelDist;}

/** Returns the type of distribution of spanwise panels at a span section identified by its index
*@param iSection the index of the section
*@return the type of distribution of spanwise panels
*/
XFLR5::enumPanelDistribution & Wing::YPanelDist(const int &iSection) {return m_WingSection[iSection]->m_YPanelDist;}

/**
 * Returns the name of the foil on the right side of a span section
 * @param iSection the index of the section
 * @return the name of the foil on the right side of the section
 */
QString & Wing::rightFoil(const int &iSection) {return m_WingSection[iSection]->m_RightFoilName;}

/**
 * Returns the name of the foil on the left side of a span section
 * @param iSection the index of the section
 * @return the name of the foil on the left side of the section
 */
QString & Wing::leftFoil(const int &iSection)  {return m_WingSection[iSection]->m_LeftFoilName;}


/**
 * Removes the section in the geometry of the wing identified by its index
 * @param iSection the index of the section
 */
void Wing::removeWingSection(int const iSection)
{
    if(iSection<0 || iSection>=m_WingSection.size()) return;
    m_WingSection.removeAt(iSection);
}


/**
 * Inserts a section in the geometry of the wing at a postion identified by its index
 * @param iSection the index of the section
 */
void Wing::insertSection(int iSection)
{
    if(iSection==0)                          m_WingSection.prepend(new WingSection);
    else if(iSection>=m_WingSection.size())  m_WingSection.append(new WingSection);
    else                                     m_WingSection.insert(iSection, new WingSection);
}


/**
 * Appends a new section at the tip of the wing, with default values
 *@
 */
bool Wing::appendWingSection()
{
    m_WingSection.append(new WingSection());
    return true;
}


/**
 * Appends a new section at the tip of the wing, with values specified as input parameters
 */
bool Wing::appendWingSection(double Chord, double Twist, double Pos, double Dihedral, double Offset,
                             int NXPanels, int NYPanels, XFLR5::enumPanelDistribution XPanelDist, XFLR5::enumPanelDistribution YPanelDist,
                             QString RightFoilName, QString LeftFoilName)
{
    WingSection *pWS = new WingSection();
    m_WingSection.append(pWS);
    pWS->m_Chord      = Chord;
    pWS->m_Twist      = Twist;
    pWS->m_YPosition  = Pos ;
    pWS->m_Dihedral   = Dihedral;
    pWS->m_Offset     = Offset ;

    pWS->m_NXPanels   = NXPanels ;
    pWS->m_NYPanels   = NYPanels;
    pWS->m_XPanelDist = XPanelDist;
    pWS->m_YPanelDist = YPanelDist;

    pWS->m_RightFoilName  = RightFoilName;
    pWS->m_LeftFoilName   = LeftFoilName;

    return true;
}


bool Wing::isWingFoil(Foil *pFoil)
{
    if(!pFoil) return false;

    for (int iws=0; iws<NWingSection(); iws++)
    {
        if(pFoil->foilName() == m_WingSection.at(iws)->m_RightFoilName)
        {
            return true;
        }
    }

    if(!m_bSymetric)
    {
        for (int iws=0; iws<NWingSection(); iws++)
        {
            if(pFoil->foilName() == m_WingSection.at(iws)->m_LeftFoilName)
            {
                return true;
            }
        }
    }
    return false;
}







/** Finds the intersection point of a line originating at point O and with unit vector U
 * @param O the origin of the line
 * @param U the unit vector on the lie
 * @param I the intersection point, if any, otherwise returns an unchanged value
 * @return true if an intersection point was found, false otherwise
 */
bool Wing::intersectWing(Vector3d O,  Vector3d U, Vector3d &I)
{
    double dist=0.0;

    for(int j=0; j<m_Surface.count(); j++)
    {
        if(Intersect(m_Surface.at(j)->m_LA, m_Surface.at(j)->m_LB,
                     m_Surface.at(j)->m_TA, m_Surface.at(j)->m_TB,
                     m_Surface.at(j)->Normal,
                     O, U, I, dist)) return true;
    }
    return false;
}



void Wing::getTextureUV(int iSurf, double *leftV, double *rightV, double &leftU, double &rightU, int nPoints)
{
    double xRelA, xRelB, xA, xB, yA, yB;
    double xMin=100000, xMax=-100000, yMin, yMax;
    int iSectionA=0, iSectionB=1;

    Surface const *pSurf = m_Surface[iSurf];

    if(pSurf->isLeftSurf())
    {
        iSectionB = pSurf->innerSection();
        iSectionA = pSurf->outerSection();
    }
    else
    {
        iSectionA = pSurf->innerSection();
        iSectionB = pSurf->outerSection();
    }

    for(int is=0; is<m_WingSection.count(); is++)
    {
        xMin = std::min(xMin, m_WingSection.at(is)->m_Offset);
        xMax = std::max(xMax, m_WingSection.at(is)->m_Offset + m_WingSection.at(is)->m_Chord);
    }

    for(int i=0; i<nPoints; i++)
    {
        if(m_Surface[iSurf]->m_NXFlap>0 && m_Surface[iSurf]->m_pFoilA && m_Surface[iSurf]->m_pFoilB)
        {
            int nPtsTr = nPoints/3;
            int nPtsLe = nPoints-nPtsTr;

            if(i<nPtsTr)
            {
                xRelA = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsTr-1)))* (pSurf->m_pFoilA->m_TEXHinge/100.);
                xRelB = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPtsTr-1)))* (pSurf->m_pFoilB->m_TEXHinge/100.);
            }
            else
            {
                int j = i-nPtsTr;
                xRelA = pSurf->m_pFoilA->m_TEXHinge/100. + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsLe-1))) * (1.-pSurf->m_pFoilA->m_TEXHinge/100.);
                xRelB = pSurf->m_pFoilB->m_TEXHinge/100. + 1.0/2.0*(1.0-cos(PI* double(j)/double(nPtsLe-1))) * (1.-pSurf->m_pFoilB->m_TEXHinge/100.);
            }
        }
        else
        {
            xRelA  = 1.0/2.0*(1.0-cos(PI * double(i)/double(nPoints-1)));
            xRelB  = xRelA;
        }


        //        xRel  = 1.0/2.0*(1.0-cos( double(i)*PI   /(double)(nPoints-1)));
        xA = m_WingSection.at(iSectionA)->m_Offset + m_WingSection.at(iSectionA)->m_Chord*xRelA;
        xB = m_WingSection.at(iSectionB)->m_Offset + m_WingSection.at(iSectionB)->m_Chord*xRelB;

        leftV[i]  = (xA-xMin)/(xMax-xMin);
        rightV[i] = (xB-xMin)/(xMax-xMin);
    }


    yMin = m_WingSection.first()->m_YPosition;
    yMax = m_WingSection.last()->m_YPosition;

    yA = m_WingSection.at(iSectionA)->m_YPosition;
    yB = m_WingSection.at(iSectionB)->m_YPosition;
    if(pSurf->isLeftSurf())
    {
        leftU = 1.0-(yA-yMin)/(yMax-yMin);
        rightU  = 1.0-(yB-yMin)/(yMax-yMin);
    }
    else
    {
        leftU  = (yA-yMin)/(yMax-yMin);
        rightU = (yB-yMin)/(yMax-yMin);
    }
}


/**
 * Loads or Saves the data of this Wing to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Wing::serializeWingWPA(QDataStream &ar, bool bIsStoring)
{
    int ArchiveFormat;// identifies the format of the file

    if(bIsStoring)
    {
        //not storing to .wpa format anymore
        return true;
    }
    else
    {
        // loading code
        float f,g,h;
        int k;
        //1010 : added storage of alpha channel + added a provision for ints and floats
        //1009 : QFLR5 v0.03 : added mass properties for inertia calculations
        //1008 : QFLR5 v0.02 : Added wing description field
        //1007 : Changed length units to m
        //1006 : Added Wing Color v2.99-15
        //1005 : Added Chordwise spacing (v2.99-00)
        //1004 : corrected NXPanels, NYPanels, YPanelDist to int (v1.99-33)
        //1003 : AVL Format (v1.99-18)
        //1002 : save VLM Mesh (v1.99-12)
        //1001 : initial format

        ar >> ArchiveFormat;

        if (ArchiveFormat <1001 || ArchiveFormat>1100) {
            m_WingName = "";
            return false;
        }

        readCString(ar,m_WingName);
        if (m_WingName.length() ==0) return false;

        if (ArchiveFormat >=1008)
        {
            readCString(ar, m_WingDescription);
        }

        ar >> k;
        if(k!=0){
            m_WingName = "";
            return false;
        }

        ar >> k;
        if (k==1) m_bSymetric = true;
        else if (k==0) m_bSymetric = false;
        else{
            m_WingName = "";
            return false;
        }
        //        m_bVLMSymetric = m_bSymetric;

        int NPanel;
        ar >> NPanel;
        if(NPanel <0 || NPanel>1000) return false;

        clearWingSections();
        for(int is=0; is<=NPanel; is++) m_WingSection.append(new WingSection);

        QString strFoil;

        for (int is=0; is<=NPanel; is++)
        {
            readCString(ar, strFoil);
            rightFoil(is) = strFoil;
        }
        for (int is=0; is<=NPanel; is++)
        {
            readCString(ar, strFoil);
            leftFoil(is) = strFoil;
        }

        for (int is=0; is<=NPanel; is++)
        {
            ar >> f; Chord(is)=double(f);
            if (qAbs(Chord(is)) <0.0)
            {
                m_WingName = "";
                return false;
            }
        }

        for (int is=0; is<=NPanel; is++)
        {
            ar >> f; YPosition(is)=double(f);
            if (qAbs(YPosition(is)) <0.0)
            {
                m_WingName = "";
                return false;
            }
        }
        for (int is=0; is<=NPanel; is++)
        {
            ar >> f; Offset(is)=double(f);
        }

        if(ArchiveFormat<1007)
        {
            //convert mm to m
            for (int is=0; is<=NPanel; is++)
            {
                YPosition(is)    /= 1000.0;
                Chord(is)  /= 1000.0;
                Offset(is) /= 1000.0;
            }

        }
        for (int is=0; is<=NPanel; is++)
        {
            ar >> f; Dihedral(is)=double(f);
        }
        for (int is=0; is<=NPanel; is++)
        {
            ar >> f; Twist(is)=double(f);
        }

        ar >> f; //m_XCmRef=f;

        ar >> k;

        for (int is=0; is<=NPanel; is++)
        {
            if(ArchiveFormat<=1003)
            {
                ar >> f;
                NXPanels(is) = int(f);
            }
            else
                ar >> NXPanels(is);
        }

        for (int is=0; is<=NPanel; is++)
        {
            if(ArchiveFormat<=1003)
            {
                ar >> f;
                NYPanels(is) = int(f);
            }
            else     ar >> NYPanels(is);
        }
        int total = 0;
        for (int is=0; is<NPanel; is++)
        {
            total += NYPanels(is);
        }


        int spanpos = 0;
        int vlmpanels = 0;
        for (int is=0; is<NPanel; is++)
        {
            spanpos  += NYPanels(is);
            vlmpanels +=NXPanels(is)*NYPanels(is);
        }


        if (ArchiveFormat >=1005)
        {
            for(int is=0; is<=NPanel; is++)
            {
                ar >> k;
                if(k==1)       XPanelDist(is) = XFLR5::COSINE;
                else if(k==2)  XPanelDist(is) = XFLR5::SINE;
                else if(k==-2) XPanelDist(is) = XFLR5::INVERSESINE;
                else           XPanelDist(is) = XFLR5::UNIFORM;  //case 0
            }
        }

        for (int is=0; is<=NPanel; is++)
        {
            ar >> k;
            if(k==1)       YPanelDist(is) = XFLR5::COSINE;
            else if(k==2)  YPanelDist(is) = XFLR5::SINE;
            else if(k==-2) YPanelDist(is) = XFLR5::INVERSESINE;
            else           YPanelDist(is) = XFLR5::UNIFORM;  //case 0
        }

        if(ArchiveFormat>=1006)
        {
            int r,g,b;
            readCOLORREF(ar, r,g,b);
        }

        if(ArchiveFormat>=1009)
        {
            ar >> f;  m_VolumeMass = double(f);
            int nMass;

            ar >> nMass;
            QVarLengthArray<double> mass;
            QVarLengthArray<Vector3d> position;
            QVarLengthArray<QString> tag;

            for(int im=0; im<nMass; im++)
            {
                ar >> f;
                mass.append(double(f));
            }
            for(int im=0; im<nMass; im++)
            {
                ar >> f >> g >> h;
                position.append(Vector3d(double(f),double(g),double(h)));
            }
            for(int im=0; im<nMass; im++)
            {
                tag.append("");
                readCString(ar, tag[im]);
            }

            clearPointMasses();
            for(int im=0; im<nMass; im++)
            {
                m_PointMass.append(new PointMass(mass[im], position[im], tag[im]));
            }
        }

        if(ArchiveFormat>=1010)
        {
            ar >> k; m_WingColor.setAlpha(k);
            for(int i=0; i<20; i++) ar>>f;
            for(int i=0; i<20; i++) ar>>k;
        }

        computeGeometry();
        return true;
    }
}




/**
 * Loads or Saves the data of this Wing to a binary file.
 * @param ar the QDataStream object from/to which the data should be serialized
 * @param bIsStoring true if saving the data, false if loading
 * @return true if the operation was successful, false otherwise
 */
bool Wing::serializeWingXFL(QDataStream &ar, bool bIsStoring)
{
    QString tag;
    QString rightfoil, leftfoil;
    int nx, ny;
    int i, k, n, is;
    int ArchiveFormat;// identifies the format of the file
    double dble, dm, px, py, pz;
    double chord, twist, pos, dihedral, offset;
    XFLR5::enumPanelDistribution xDist, yDist;

    if(bIsStoring)
    {
        ar << 100001;
        ar << m_WingName;
        ar << m_WingDescription;

        writeQColor(ar, m_WingColor.red(), m_WingColor.green(), m_WingColor.blue(), m_WingColor.alpha());

        ar << m_bSymetric;

        ar << NWingSection();

        for (is=0; is<NWingSection(); is++)
        {
            ar << rightFoil(is);
            ar << leftFoil(is);
            ar << Chord(is);
            ar << YPosition(is);
            ar << Offset(is);
            ar << Dihedral(is);
            ar << Twist(is);
            ar << NXPanels(is);
            ar << NYPanels(is);

            switch(XPanelDist(is))
            {
                case XFLR5::COSINE:
                    ar <<  1;
                    break;
                case XFLR5::SINE:
                    ar <<  2;
                    break;
                case XFLR5::INVERSESINE:
                    ar << -2;
                    break;
                default:
                    ar <<  0; //XFLR5::UNIFORM
                    break;
            }

            switch(YPanelDist(is))
            {
                case XFLR5::COSINE:
                    ar <<  1;
                    break;
                case XFLR5::SINE:
                    ar <<  2;
                    break;
                case XFLR5::INVERSESINE:
                    ar << -2;
                    break;
                default:
                    ar <<  0; //XFLR5::UNIFORM
                    break;
            }
        }

        ar << m_VolumeMass;
        ar << m_PointMass.size();
        for(i=0; i<m_PointMass.size(); i++)
        {
            ar << m_PointMass.at(i)->mass();
            ar << m_PointMass.at(i)->position().x << m_PointMass.at(i)->position().y << m_PointMass.at(i)->position().z;
            ar << m_PointMass.at(i)->tag();
        }

        if(m_bTextures) ar<<1 ; else ar<<0;
        // space allocation for the future storage of more data, without need to change the format
        for (int i=1; i<19; i++) ar << 0;
        switch (wingType()) {
            case XFLR5::MAINWING:
                ar<<0;
                break;
            case XFLR5::SECONDWING:
                ar<<1;
                break;
            case XFLR5::ELEVATOR:
                ar<<2;
                break;
            case XFLR5::FIN:
                ar<<3;
                break;
            case XFLR5::OTHERWING:
                ar<<4;
                break;
        }

        for (int i=0; i<50; i++) ar << 0.0;

        return true;
    }
    else
    {
        ar >> ArchiveFormat;
        if(ArchiveFormat<100000 || ArchiveFormat>100001) return false;

        ar >> m_WingName;
        ar >> m_WingDescription;

        int a,r,g,b;
        readQColor(ar, r, g, b, a);
        m_WingColor.setColor(r,g,b,a);

        ar >> m_bSymetric;

        clearWingSections();
        ar >> n;
        for (i=0; i<n; i++)
        {
            ar >> rightfoil;
            ar >> leftfoil;
            ar >> chord;
            ar >> pos;
            ar >> offset;
            ar >> dihedral;
            ar >> twist;
            ar >> nx;
            ar >> ny;

            ar >> k;
            if(k==1)       xDist = XFLR5::COSINE;
            else if(k== 2) xDist = XFLR5::SINE;
            else if(k==-2) xDist = XFLR5::INVERSESINE;
            else           xDist = XFLR5::UNIFORM;

            ar >> k;
            if(k==1)       yDist = XFLR5::COSINE;
            else if(k== 2) yDist = XFLR5::SINE;
            else if(k==-2) yDist = XFLR5::INVERSESINE;
            else           yDist = XFLR5::UNIFORM;

            appendWingSection(chord, twist, pos, dihedral, offset, nx, ny, xDist, yDist, rightfoil, leftfoil);
        }

        ar >> m_VolumeMass;
        clearPointMasses();
        ar >> n;
        for(i=0; i<n; i++)
        {
            ar >> dm >> px >> py >> pz;
            ar >> tag;
            m_PointMass.append(new PointMass(dm, Vector3d(px, py, pz), tag));
        }

        ar>>k; if(k) m_bTextures=true; else m_bTextures=false;
        // space allocation
        for (int i=1; i<19; i++) ar >> k;
        ar >>k;
        switch (k) {
            case 0:
                m_WingType=XFLR5::MAINWING;
                break;
            case 1:
                m_WingType=XFLR5::SECONDWING;
                break;
            case 2:
                m_WingType=XFLR5::ELEVATOR;
                break;
            case 3:
                m_WingType=XFLR5::FIN;
                break;
            case 4:
                m_WingType=XFLR5::OTHERWING;
                break;
            default:
                break;
        }

        for (int i=0; i<50; i++) ar >> dble;

        computeGeometry();
        return true;
    }
}





/**
* Scales the wing's Aspect Ratio.
* Chords and span are scaled accordingly but the wing area remains unchanged.
* Good for general optimisation.
* @param newAR the new value of the aspect ratio.
*/
void Wing::scaleAR(double newAR)
{
    if(m_AR<PRECISION)  return;
    if(newAR<PRECISION) return;

    double ratio = sqrt(newAR/m_AR);

    for (int is=0; is<m_WingSection.size(); is++)
    {
        YPosition(is) *= ratio;
        Chord(is)     /= ratio;
    }
    computeGeometry();
}


/**
* Scales the wing's Taper Ratio.
* Root chord is unchanged, all other chords are scale proportionnally to their span position.
* @param newTR the new value of the taper ratio.
*/
void Wing::scaleTR(double newTR)
{
    if(m_TR<PRECISION)  return;
    if(newTR<PRECISION) return;

    double Ratio = m_TR/newTR;
    for (int is=0; is<m_WingSection.size(); is++)
    {
        double yRel = YPosition(is)/m_PlanformSpan *2.0;
        double cRatio = 1.0 +  yRel * (Ratio-1.0);
        Chord(is)     *= cRatio;
    }
    computeGeometry();
}



/**
 * Export the wing geometry to a binary file in STL Format.
 * @param out the instance of the QTextStream to which the output will be directed
 */
void Wing::exportSTLBinary(QDataStream &outStream, int CHORDPANELS, int SPANPANELS, float unit)
{
    /***
     *  UINT8[80] – Header
     *     UINT32 – Number of triangles
     *
     *     foreach triangle
     *     REAL32[3] – Normal vector
     *     REAL32[3] – Vertex 1
     *     REAL32[3] – Vertex 2
     *     REAL32[3] – Vertex 3
     *     UINT16 – Attribute byte count
     *     end
    */

    Vector3d N, Pt;
    QVector<Vector3d> NormalA(CHORDPANELS+1);
    QVector<Vector3d> NormalB(CHORDPANELS+1);
    QVector<Vector3d> PtLeft(CHORDPANELS+1);
    QVector<Vector3d> PtRight(CHORDPANELS+1);
    QVector<Vector3d> PtBotLeft(CHORDPANELS+1);
    QVector<Vector3d> PtBotRight(CHORDPANELS+1);

    memset(NormalA.data(), 0, ulong(CHORDPANELS+1) * sizeof(Vector3d));
    memset(NormalB.data(), 0, ulong(CHORDPANELS+1) * sizeof(Vector3d));


    //    80 character header, avoid word "solid"
    //                       0123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789
    QString strong =     "binary STL file                                                                ";
    writeCString(outStream, strong);

    //Number of triangles
    // nSurfaces
    //   *CHORDPANELS*SPANPANELS   quads
    //   *2                        2 triangles/quad
    //   *2                        top and bottom surfaces
    // 2 Tip patches
    //   1 LE triangle
    //   1 TE triangle
    //   CHORDPANELS-1  quads
    //   *2 triangles/quad

    int nTriangles = m_Surface.count() * CHORDPANELS * SPANPANELS * 2 *2
                     + 2* ((CHORDPANELS-2)*2 + 2);
    outStream << nTriangles;

    short zero = 0;

    N.set(0.0, 0.0, 0.0);
    int iTriangles = 0;

    char buffer[12];

    for (int j=0; j<m_Surface.size(); j++)
    {
        //top surface
        for(int is=0; is<SPANPANELS; is++)
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(), PtRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);

            double tauA = double(is)   /double(SPANPANELS);
            double tauB = double(is+1) /double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //left side vertices
                N = NormalA[ic] * (1.0-tau) + NormalB[ic] * tau;
                //1st triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);


                //2nd triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                iTriangles +=2;
            }
        }


        //bottom surface
        for(int is=0; is<SPANPANELS; is++)
        {
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtLeft.data(), PtRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);

            double tauA = double(is)   / double(SPANPANELS);
            double tauB = double(is+1) / double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                N = NormalA[ic] * (1.0-tau) + NormalB[ic] * tau;

                //1st triangle
                writeFloat(outStream, N.xf()*unit);
                writeFloat(outStream, N.yf()*unit);
                writeFloat(outStream, N.zf()*unit);
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                //2nd triangle
                writeFloat(outStream, N.xf()*unit);
                writeFloat(outStream, N.yf()*unit);
                writeFloat(outStream, N.zf()*unit);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                writeFloat(outStream, Pt.xf()*unit);
                writeFloat(outStream, Pt.yf()*unit);
                writeFloat(outStream, Pt.zf()*unit);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                iTriangles +=2;
            }
        }
    }

    Q_ASSERT(iTriangles==m_Surface.count() * CHORDPANELS * SPANPANELS * 2 *2);

    //TIP PATCHES

    for (int j=0; j<m_Surface.size(); j++)
    {
        if(m_Surface.at(j)->isTipLeft())
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(), PtRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtBotLeft.data(), PtBotRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);

            N = m_Surface.at(j)->Normal;
            N.rotateX(90.0);

            //L.E. triangle
            writeFloat(outStream, N.xf());
            writeFloat(outStream, N.yf());
            writeFloat(outStream, N.zf());
            writeFloat(outStream, PtBotLeft[0].xf()*unit);
            writeFloat(outStream, PtBotLeft[0].yf()*unit);
            writeFloat(outStream, PtBotLeft[0].zf()*unit);
            writeFloat(outStream, PtLeft[1].xf()*unit);
            writeFloat(outStream, PtLeft[1].yf()*unit);
            writeFloat(outStream, PtLeft[1].zf()*unit);
            writeFloat(outStream, PtBotLeft[1].xf()*unit);
            writeFloat(outStream, PtBotLeft[1].yf()*unit);
            writeFloat(outStream, PtBotLeft[1].zf()*unit);
            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                writeFloat(outStream, PtBotLeft[ic].xf()*unit);
                writeFloat(outStream, PtBotLeft[ic].yf()*unit);
                writeFloat(outStream, PtBotLeft[ic].zf()*unit);
                writeFloat(outStream, PtLeft[ic].xf()*unit);
                writeFloat(outStream, PtLeft[ic].yf()*unit);
                writeFloat(outStream, PtLeft[ic].zf()*unit);
                writeFloat(outStream, PtLeft[ic+1].xf()*unit);
                writeFloat(outStream, PtLeft[ic+1].yf()*unit);
                writeFloat(outStream, PtLeft[ic+1].zf()*unit);
                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                //2nd triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                writeFloat(outStream, PtBotLeft[ic].xf()*unit);
                writeFloat(outStream, PtBotLeft[ic].yf()*unit);
                writeFloat(outStream, PtBotLeft[ic].zf()*unit);
                writeFloat(outStream, PtLeft[ic+1].xf()*unit);
                writeFloat(outStream, PtLeft[ic+1].yf()*unit);
                writeFloat(outStream, PtLeft[ic+1].zf()*unit);
                writeFloat(outStream, PtBotLeft[ic+1].xf()*unit);
                writeFloat(outStream, PtBotLeft[ic+1].yf()*unit);
                writeFloat(outStream, PtBotLeft[ic+1].zf()*unit);
                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                iTriangles +=2;
            }

            //T.E. triangle
            int ic = CHORDPANELS-1;
            writeFloat(outStream, N.xf());
            writeFloat(outStream, N.yf());
            writeFloat(outStream, N.zf());
            writeFloat(outStream, PtBotLeft[ic].xf()*unit);
            writeFloat(outStream, PtBotLeft[ic].yf()*unit);
            writeFloat(outStream, PtBotLeft[ic].zf()*unit);
            writeFloat(outStream, PtLeft[ic].xf()*unit);
            writeFloat(outStream, PtLeft[ic].yf()*unit);
            writeFloat(outStream, PtLeft[ic].zf()*unit);
            writeFloat(outStream, PtBotLeft[ic+1].xf()*unit);
            writeFloat(outStream, PtBotLeft[ic+1].yf()*unit);
            writeFloat(outStream, PtBotLeft[ic+1].zf()*unit);
            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            iTriangles +=1;
        }

        if(m_Surface.at(j)->isTipRight())
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(), PtRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtBotLeft.data(), PtBotRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);

            N = m_Surface.at(j)->Normal;
            N.rotateX(-90.0);

            //L.E. triangle
            writeFloat(outStream, N.xf());
            writeFloat(outStream, N.yf());
            writeFloat(outStream, N.zf());
            writeFloat(outStream, PtBotRight[0].xf()*unit);
            writeFloat(outStream, PtBotRight[0].yf()*unit);
            writeFloat(outStream, PtBotRight[0].zf()*unit);
            writeFloat(outStream, PtRight[1].xf()*unit);
            writeFloat(outStream, PtRight[1].yf()*unit);
            writeFloat(outStream, PtRight[1].zf()*unit);
            writeFloat(outStream, PtBotRight[1].xf()*unit);
            writeFloat(outStream, PtBotRight[1].yf()*unit);
            writeFloat(outStream, PtBotRight[1].zf()*unit);
            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);
            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                writeFloat(outStream, PtBotRight[ic].xf()*unit);
                writeFloat(outStream, PtBotRight[ic].yf()*unit);
                writeFloat(outStream, PtBotRight[ic].zf()*unit);
                writeFloat(outStream, PtRight[ic].xf()*unit);
                writeFloat(outStream, PtRight[ic].yf()*unit);
                writeFloat(outStream, PtRight[ic].zf()*unit);
                writeFloat(outStream, PtRight[ic+1].xf()*unit);
                writeFloat(outStream, PtRight[ic+1].yf()*unit);
                writeFloat(outStream, PtRight[ic+1].zf()*unit);
                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);

                //2nd triangle
                writeFloat(outStream, N.xf());
                writeFloat(outStream, N.yf());
                writeFloat(outStream, N.zf());
                writeFloat(outStream, PtBotRight[ic].xf()*unit);
                writeFloat(outStream, PtBotRight[ic].yf()*unit);
                writeFloat(outStream, PtBotRight[ic].zf()*unit);
                writeFloat(outStream, PtRight[ic+1].xf()*unit);
                writeFloat(outStream, PtRight[ic+1].yf()*unit);
                writeFloat(outStream, PtRight[ic+1].zf()*unit);
                writeFloat(outStream, PtBotRight[ic+1].xf()*unit);
                writeFloat(outStream, PtBotRight[ic+1].yf()*unit);
                writeFloat(outStream, PtBotRight[ic+1].zf()*unit);

                memcpy(buffer, &zero, sizeof(short));
                outStream.writeRawData(buffer, 2);
                iTriangles +=2;
            }

            //T.E. triangle
            int ic = CHORDPANELS-1;
            writeFloat(outStream, N.xf());
            writeFloat(outStream, N.yf());
            writeFloat(outStream, N.zf());
            writeFloat(outStream, PtBotRight[ic].xf()*unit);
            writeFloat(outStream, PtBotRight[ic].yf()*unit);
            writeFloat(outStream, PtBotRight[ic].zf()*unit);
            writeFloat(outStream, PtRight[ic].xf()*unit);
            writeFloat(outStream, PtRight[ic].yf()*unit);
            writeFloat(outStream, PtRight[ic].zf()*unit);
            writeFloat(outStream, PtBotRight[ic+1].xf()*unit);
            writeFloat(outStream, PtBotRight[ic+1].yf()*unit);
            writeFloat(outStream, PtBotRight[ic+1].zf()*unit);
            memcpy(buffer, &zero, sizeof(short));
            outStream.writeRawData(buffer, 2);

            iTriangles +=1;
        }
    }
    Q_ASSERT(iTriangles==nTriangles);
}



/**
 * Export the wing geometry to a binary file in STL Format.
 * @param out the instance of the QTextStream to which the output will be directed
 */
void Wing::exportSTLText(QTextStream &outStream, int CHORDPANELS, int SPANPANELS)
{
    /***
     * solid name
     *
     *       facet normal ni nj nk
     *         outer loop
     *           vertex v1x v1y v1z
     *           vertex v2x v2y v2z
     *           vertex v3x v3y v3z
     *      endloop
     *   endfacet
     *
     * endsolid name
    */
    QString name = wingName();
    name.replace(" ","");
    QString strong = "solid " + name + "\n";
    outStream << strong;

    Vector3d N, Pt;
    QVector<Vector3d> NormalA(CHORDPANELS+1);
    QVector<Vector3d> NormalB(CHORDPANELS+1);
    QVector<Vector3d> PtLeft(CHORDPANELS+1);
    QVector<Vector3d> PtRight(CHORDPANELS+1);
    QVector<Vector3d> PtBotLeft(CHORDPANELS+1);
    QVector<Vector3d> PtBotRight(CHORDPANELS+1);

    memset(NormalA.data(), 0, ulong(CHORDPANELS+1) * sizeof(Vector3d));
    memset(NormalB.data(), 0, ulong(CHORDPANELS+1) * sizeof(Vector3d));

    //Number of triangles
    // nSurfaces
    //   *CHORDPANELS*SPANPANELS   quads
    //   *2                        2 triangles/quad
    //   *2                        top and bottom surfaces
    // 2 Tip patches
    //   1 LE triangle
    //   1 TE triangle
    //   CHORDPANELS-1  quads
    //   *2 triangles/quad

    int nTriangles = m_Surface.count() * CHORDPANELS * SPANPANELS * 2 *2
                      + 2* ((CHORDPANELS-2)*2 + 2);
    N.set(0.0, 0.0, 0.0);
    int iTriangles = 0;

    for (int j=0; j<m_Surface.size(); j++)
    {
        //top surface
        for(int is=0; is<SPANPANELS; is++)
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(), PtRight.data(), NormalA.data(), NormalB.data(), CHORDPANELS+1);

            double tauA = double(is)   / double(SPANPANELS);
            double tauB = double(is+1) / double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                N = (NormalA[ic]+NormalA[ic+1]) * (1.0-tau) + (NormalB[ic]+NormalB[ic+1]) * tau;
                N.normalize();

                //1st triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outStream << "    endloop\n  endfacet\n";

                //2nd triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outStream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
        }

        //bottom surface
        for(int is=0; is<SPANPANELS; is++)
        {
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtLeft.data(), PtRight.data(),
                                           NormalA.data(), NormalB.data(), CHORDPANELS+1);

            double tauA = double(is)   /double(SPANPANELS);
            double tauB = double(is+1) /double(SPANPANELS);
            double tau = (tauA+tauB)/2.0;
            for(int ic=0; ic<CHORDPANELS; ic++)
            {
                //left side vertices
                N = (NormalA[ic]+NormalA[ic+1]) * (1.0-tau) + (NormalB[ic]+NormalB[ic+1]) * tau;
                N.normalize();

                //1st triangle
                outStream << strong.sprintf("facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                Pt = PtLeft[ic]   * (1.0-tauA) + PtRight[ic]   * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outStream << "    endloop\n  endfacet\n";

                //2nd triangle
                outStream << strong.sprintf("facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                Pt = PtLeft[ic+1] * (1.0-tauA) + PtRight[ic+1] * tauA;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic+1] * (1.0-tauB) + PtRight[ic+1] * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                Pt = PtLeft[ic]   * (1.0-tauB) + PtRight[ic]   * tauB;
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  Pt.x, Pt.y, Pt.z);
                outStream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
        }
    }

    Q_ASSERT(iTriangles==m_Surface.count() * CHORDPANELS * SPANPANELS * 2 *2);

    //TIP PATCHES

    for (int j=0; j<m_Surface.size(); j++)
    {
        if(m_Surface.at(j)->isTipLeft())
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(),    PtRight.data(),    NormalA.data(), NormalB.data(), CHORDPANELS+1);
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtBotLeft.data(), PtBotRight.data(), NormalA.data(), NormalB.data(), CHORDPANELS+1);

            N = m_Surface.at(j)->Normal;
            N.rotateX(90.0);

            //L.E. triangle
            outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outStream << "    outer loop\n";
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[0].x, PtBotLeft[0].y, PtBotLeft[0].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[1].x,    PtLeft[1].y,    PtLeft[1].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[1].x, PtBotLeft[1].y, PtBotLeft[1].z);
            outStream << "    endloop\n  endfacet\n";
            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic].x,   PtBotLeft[ic].y,   PtBotLeft[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x,      PtLeft[ic].y,      PtLeft[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
                outStream << "    endloop\n  endfacet\n";
                //2nd triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x,      PtLeft[ic].y,      PtLeft[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic+1].x,    PtLeft[ic+1].y,    PtLeft[ic+1].z);
                outStream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
            //T.E. triangle
            int ic = CHORDPANELS-1;
            outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outStream << "    outer loop\n";
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic].x,   PtBotLeft[ic].y,   PtBotLeft[ic].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtLeft[ic].x,      PtLeft[ic].y,      PtLeft[ic].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotLeft[ic+1].x, PtBotLeft[ic+1].y, PtBotLeft[ic+1].z);
            outStream << "    endloop\n  endfacet\n";
            iTriangles +=1;
        }

        if(m_Surface.at(j)->isTipRight())
        {
            m_Surface.at(j)->getSidePoints(TOPSURFACE, nullptr, PtLeft.data(),    PtRight.data(),    NormalA.data(), NormalB.data(), CHORDPANELS+1);
            m_Surface.at(j)->getSidePoints(BOTSURFACE, nullptr, PtBotLeft.data(), PtBotRight.data(), NormalA.data(), NormalB.data(), CHORDPANELS+1);

            N = m_Surface.at(j)->Normal;
            N.rotateX(-90.0);

            //L.E. triangle
            outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outStream << "    outer loop\n";
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[0].x, PtBotRight[0].y, PtBotRight[0].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[1].x,    PtRight[1].y,    PtRight[1].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[1].x, PtBotRight[1].y, PtBotRight[1].z);
            outStream << "    endloop\n  endfacet\n";
            iTriangles +=1;

            for(int ic=1; ic<CHORDPANELS-1; ic++)
            {
                //1st triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic].x,   PtBotRight[ic].y,   PtBotRight[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x,      PtRight[ic].y,      PtRight[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
                outStream << "    endloop\n  endfacet\n";
                //2nd triangle
                outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
                outStream << "    outer loop\n";
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x,      PtRight[ic].y,      PtRight[ic].z);
                outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic+1].x,    PtRight[ic+1].y,    PtRight[ic+1].z);
                outStream << "    endloop\n  endfacet\n";
                iTriangles +=2;
            }
            //T.E. triangle
            int ic = CHORDPANELS-1;
            outStream << strong.sprintf("  facet normal %13.7f  %13.7f  %13.7f\n",  N.x, N.y, N.z);
            outStream << "    outer loop\n";
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic].x,   PtBotRight[ic].y,   PtBotRight[ic].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtRight[ic].x,      PtRight[ic].y,      PtRight[ic].z);
            outStream << strong.sprintf("      vertex %13.7f  %13.7f  %13.7f\n",  PtBotRight[ic+1].x, PtBotRight[ic+1].y, PtBotRight[ic+1].z);
            outStream << "    endloop\n  endfacet\n";
            iTriangles +=1;
        }
    }

    Q_ASSERT(iTriangles==nTriangles);

    strong = "endsolid " + name + "\n";
    outStream << strong;
}


/**
 * Returns a pointer to the foil with the corresponding nam or NULL if not found.
 * @param strFoilName the name of the Foil to search for in the array
 * @return a pointer to the foil with the corresponding nam or NULL if not found.
 */
Foil* Wing::foil(QString strFoilName)
{
    if(!strFoilName.length()) return nullptr;
    Foil* pFoil=nullptr;
    for (int i=0; i<s_poaFoil->size(); i++)
    {
        pFoil = s_poaFoil->at(i);
        if (pFoil->foilName() == strFoilName)
        {
            return pFoil;
        }
    }

    return nullptr;
}

/**
*Interpolates a variable on the polar mesh, based on the geometrical position of a point between two sections on a wing.
*@param m_poaPolar the pointer to the array of polars.
*@param nVar the index of the variable to interpolate.
*@param pFoil0 the pointer to the left foil  of the wing's section.
*@param pFoil1 the pointer to the left foil  of the wing's section.
*@param Re the Reynolds number at the point's position.
*@param Cl the lift coefficient at the point's position, used as the input parameter.
*@param Tau the relative position of the point between the two foils.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double Wing::getInterpolatedVariable(int nVar, Foil *pFoil0, Foil *pFoil1, double Re, double Cl, double Tau, bool &bOutRe, bool &bError)
{
    bool IsOutRe = false;
    bool IsError  = false;
    bOutRe = false;
    bError = false;
    double Var0, Var1;
    if(!pFoil0)
    {
        Cl = 0.0;
        Var0 = 0.0;
    }
    else Var0 = getPlrPointFromCl(pFoil0, Re, Cl,nVar, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;


    if(!pFoil1)
    {
        Cl = 0.0;
        Var1 = 0.0;
    }
    else Var1 = getPlrPointFromCl(pFoil1, Re, Cl,nVar, IsOutRe, IsError);
    if(IsOutRe) bOutRe = true;
    if(IsError) bError = true;

    if (Tau<0.0) Tau = 0.0;
    if (Tau>1.0) Tau = 1.0;

    return ((1-Tau) * Var0 + Tau * Var1);
}




/**
* Returns the value of an aero coefficient, interpolated on a polar mesh, and based on the value of the Reynolds Number and of the lift coefficient.
* Proceeds by identifiying the two polars surronding Re, then interpolating both with the value of Alpha,
* last by interpolating the requested variable between the values measured on the two polars.
*@param m_poaPolar the pointer to the array of polars.
*@param pFoil the pointer to the foil
*@param Re the Reynolds number .
*@param Cl the lift coefficient, used as the input parameter for interpolation.
*@param PlrVar the index of the variable to interpolate.
*@param bOutRe true if Cl is outside the min or max Cl of the polar mesh.
*@param bError if Re is outside the min or max Reynolds number of the polar mesh.
*@return the interpolated value.
*/
double Wing::getPlrPointFromCl(Foil *pFoil, double Re, double Cl, int PlrVar, bool &bOutRe, bool &bError)
{
    /*    Var
    0 =    m_Alpha;
    1 = m_Cl;
    2 = m_Cd;
    3 = m_Cdp;
    4 = m_Cm;
    5, 6 = m_XTr1, m_XTr2;
    7, 8 = m_HMom, m_Cpmn;
    9,10 = m_ClCd, m_Cl32Cd;
*/
    double Clmin=0, Clmax=0;
    Polar *pPolar;
    double Var1=0, Var2=0, u=0, dist=0;
    Var1 = Var2 = u = dist = 0.0;
    int pt=0;
    int n=0;

    bOutRe = false;
    bError = false;

    if(!pFoil)
    {
        bOutRe = true;
        bError = true;
        return 0.000;
    }

    n=0;
    // Are there any Type 1 polars available for this foil ?
    for (int i = 0; i<s_poaPolar->size(); i++)
    {
        pPolar = s_poaPolar->at(i);
        if((pPolar->polarType()== XFLR5::FIXEDSPEEDPOLAR) && (pPolar->foilName() == pFoil->foilName()))
        {
            n++;
            if(n>=2) break;
        }
    }

    //more than one polar - interpolate between  - tough job

    //First Find the two polars with Reynolds number surrounding wanted Re
    Polar * pPolar1 = nullptr;
    Polar * pPolar2 = nullptr;
    int nPolars = s_poaPolar->size();
    //Type 1 Polars are sorted by crescending Re Number

    //if Re is less than that of the first polar, use this one
    for (int i=0; i<nPolars; i++)
    {
        pPolar = s_poaPolar->at(i);
        if((pPolar->polarType()==XFLR5::FIXEDSPEEDPOLAR) && (pPolar->foilName()==pFoil->foilName()) && pPolar->m_Cl.size()>0)
        {
            // we have found the first type 1 polar for this foil
            if (Re < pPolar->Reynolds())
            {
                bOutRe = true;
                //interpolate Cl on this polar
                QVector<double> const &pX = pPolar->getPlrVariable(PlrVar);
                int size = pPolar->m_Cl.size();
                if(Cl < pPolar->m_Cl[0])
                {
                    return pX.front();
                }
                if(Cl > pPolar->m_Cl[size-1])
                {
                    return pX.back();
                }
                for (i=0; i<size-1; i++)
                {
                    if(pPolar->m_Cl[i] <= Cl && Cl < pPolar->m_Cl[i+1])
                    {
                        //interpolate
                        if(pPolar->m_Cl[i+1]-pPolar->m_Cl[i] < 0.00001)//do not divide by zero
                            return pX.at(i);
                        else {
                            u = (Cl - pPolar->m_Cl[i])  /(pPolar->m_Cl[i+1]-pPolar->m_Cl[i]);
                            return pX.at(i) + u * (pX.at(i+1)-pX.at(i));
                        }
                    }
                }
                break;
            }
            break;
        }
    }

    // if not Find the two polars
    for (int i=0; i< nPolars; i++)
    {
        pPolar = s_poaPolar->at(i);
        if((pPolar->polarType()== XFLR5::FIXEDSPEEDPOLAR) && (pPolar->foilName() == pFoil->foilName())  && pPolar->m_Cl.size()>0)
        {
            // we have found the first type 1 polar for this foil
            pPolar->getClLimits(Clmin, Clmax);
            if (pPolar->Reynolds() <= Re)
            {
                if(Clmin <= Cl && Cl <= Clmax)
                {
                    pPolar1 = pPolar;
                }
            }
            else
            {
                if(Clmin <= Cl && Cl <= Clmax)
                {
                    pPolar2 = pPolar;
                    break;
                }
            }
        }
    }

    if (!pPolar2)
    {
        //then Re is greater than that of any polar
        // so use last polar and interpolate Cls on this polar
        bOutRe = true;
        if(!pPolar1)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }
        int size = pPolar1->m_Cl.size();
        if(!size)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }

        QVector<double> const &pX = pPolar1->getPlrVariable(PlrVar);
        if(Cl < pPolar1->m_Cl[0])       return pX.front();
        if(Cl > pPolar1->m_Cl[size-1]) return pX.back();
        for (int i=0; i<size-1; i++)
        {
            if(pPolar1->m_Cl[i] <= Cl && Cl < pPolar1->m_Cl[i+1])
            {
                //interpolate
                if(pPolar1->m_Cl[i+1]-pPolar1->m_Cl[i] < 0.00001)
                {//do not divide by zero
                    return pX.at(i);
                }
                else
                {
                    u = (Cl - pPolar1->m_Cl[i])
                            /(pPolar1->m_Cl[i+1]-pPolar1->m_Cl[i]);
                    return pX.at(i) + u * (pX.at(i+1)-pX.at(i));
                }
            }
        }
        //Out in Re, out in Cl...
        return pX.back();
    }
    else
    {
        // Re is between that of polars 1 and 2
        // so interpolate Cls for each
        if(!pPolar1)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }
        int size = pPolar1->m_Cl.size();
        if(!size)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }

        QVector<double> const &pX = pPolar1->getPlrVariable(PlrVar);
        pPolar1->getClLimits(Clmin, Clmax);
        if(Cl < Clmin)
        {
            Var1 = pX.front();
            bOutRe = true;
        }
        else if(Cl > Clmax)
        {
            Var1 = pX.back();
            bOutRe = true;
        }
        else
        {
            //first Find the point closest to Cl=0
            pt = 0;
            dist = fabs(pPolar1->m_Cl[0]);
            for (int i=1; i<size;i++)
            {
                if (fabs(pPolar1->m_Cl[i])< dist)
                {
                    dist = fabs(pPolar1->m_Cl[i]);
                    pt = i;
                }
            }
            if(Cl<pPolar1->m_Cl[pt])
            {
                for (int i=pt; i>0; i--)
                {
                    if(Cl<= pPolar1->m_Cl[i] && Cl > pPolar1->m_Cl[i-1])
                    {
                        //interpolate
                        if(fabs(pPolar1->m_Cl[i]-pPolar1->m_Cl[i-1]) < 0.00001)
                        {
                            //do not divide by zero
                            Var1 = pX.at(i);
                            break;
                        }
                        else
                        {
                            u = (Cl - pPolar1->m_Cl[i-1])
                                    /(pPolar1->m_Cl[i]-pPolar1->m_Cl[i-1]);
                            Var1 = pX.at(i-1) + u * (pX.at(i)-pX.at(i-1));
                            break;
                        }
                    }
                }
            }
            else
            {
                for (int i=pt; i<size-1; i++)
                {
                    if(pPolar1->m_Cl[i] <=Cl && Cl < pPolar1->m_Cl[i+1])
                    {
                        //interpolate
                        if(fabs(pPolar1->m_Cl[i+1]-pPolar1->m_Cl[i]) < 0.00001){//do not divide by zero
                            Var1 = pX.at(i);
                            break;
                        }
                        else
                        {
                            u = (Cl - pPolar1->m_Cl[i])
                                    /(pPolar1->m_Cl[i+1]-pPolar1->m_Cl[i]);
                            Var1 = pX.at(i) + u * (pX.at(i+1)-pX.at(i));
                            break;
                        }
                    }
                }
            }
        }
        size = pPolar2->m_Cl.size();
        if(!size)
        {
            bOutRe = true;
            bError = true;
            return 0.000;
        }

        QVector<double> const &pX2 = pPolar2->getPlrVariable(PlrVar);
        pPolar2->getClLimits(Clmin, Clmax);

        if(Cl < Clmin)
        {
            Var2 = pX2.front();
            bOutRe = true;
        }
        else if(Cl > Clmax)
        {
            Var2 = pX2.back();
            bOutRe = true;
        }
        else
        {
            //first Find the point closest to Cl=0
            pt = 0;
            dist = qAbs(pPolar2->m_Cl[0]);
            for (int i=1; i<size;i++)
            {
                if (fabs(pPolar2->m_Cl[i])< dist)
                {
                    dist =fabs(pPolar2->m_Cl[i]);
                    pt = i;
                }
            }
            if(Cl<pPolar2->m_Cl[pt])
            {
                for (int i=pt; i>0; i--)
                {
                    if(Cl<= pPolar2->m_Cl[i] && Cl > pPolar2->m_Cl[i-1])
                    {
                        //interpolate
                        if(fabs(pPolar2->m_Cl[i]-pPolar2->m_Cl[i-1]) < 0.00001)
                        {//do not divide by zero
                            Var2 = pX2.at(i);
                            break;
                        }
                        else
                        {
                            u = (Cl - pPolar2->m_Cl[i-1])
                                    /(pPolar2->m_Cl[i]-pPolar2->m_Cl[i-1]);
                            Var2 = pX2.at(i-1) + u * (pX2.at(i)-pX2.at(i-1));
                            break;
                        }
                    }
                }
            }
            else
            {
                for (int i=pt; i<size-1; i++)
                {
                    if(pPolar2->m_Cl[i] <=Cl && Cl < pPolar2->m_Cl[i+1])
                    {
                        //interpolate
                        if(fabs(pPolar2->m_Cl[i+1]-pPolar2->m_Cl[i]) < 0.00001)
                        {
                            //do not divide by zero
                            Var2 = pX2.at(i);
                            break;
                        }
                        else
                        {
                            u = (Cl - pPolar2->m_Cl[i])
                                    /(pPolar2->m_Cl[i+1]-pPolar2->m_Cl[i]);
                            Var2 = pX2.at(i) + u * (pX2.at(i+1)-pX2.at(i));
                            break;
                        }
                    }
                }
            }
        }

        // then interpolate Variable

        double v =   (Re - pPolar1->Reynolds()) / (pPolar2->Reynolds() - pPolar1->Reynolds());
        double Var = Var1 + v * (Var2-Var1);
        return Var;
    }
}




/**
 Auxiliary integral used in LLT calculations
*/
double Wing::IntegralC2(double y1, double y2, double c1, double c2)
{
    double res = 0.0;

    if (qAbs(y2-y1)<1.e-5) return 0.0;
    double g = (c2-c1)/(y2-y1);

    res = (c1-g*y1)*(c1-g*y1)*(y2-y1) +
            g * (c1-g*y1)      *(y2*y2-y1*y1)+
            g*g/3.0            *(y2*y2*y2-y1*y1*y1);

    return res;
}


/**
 Auxiliary integral used in LLT calculations
*/
double Wing::IntegralCy(double y1, double y2, double c1, double c2)
{
    double res = 0.0;
    if (qAbs(y2-y1)<1.e-5) return (y1+y2)/2.0 * (c1+c2)/2.0;

    double g = (c2-c1)/(y2-y1);
    res = (c1-g*y1)/2.0 *(y2*y2 - y1*y1) + g/3.0 * (y2*y2*y2-y1*y1*y1);
    return res;
}

