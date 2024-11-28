/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/


#pragma once
#include <xflgeom/geom3d/vector3d.h>
#include <xflcore/matrix.h>

class CartesianFrame
{
    friend class Panel;
    friend class Panel3;
    friend class Panel4;

    public:
        CartesianFrame()
        {
            memset(lij, 0, 9*sizeof(double));
        }

        CartesianFrame(Vector3d const &origin, Vector3d const &i, Vector3d const &j, Vector3d const &k)
        {
            O.x=origin.x; O.y=origin.y; O.z=origin.z;
            I.x=i.x;     I.y=i.y;     I.z=i.z;
            J.x=j.x;     J.y=j.y;     J.z=j.z;
            K.x=k.x;     K.y=k.y;     K.z=k.z;
            makeRotationMatrix();
        }

        void setFrame(Vector3d const &origin, Vector3d const &i, Vector3d const &j, Vector3d const &k)
        {
            O.x=origin.x; O.y=origin.y; O.z=origin.z;
            I.x=i.x;     I.y=i.y;     I.z=i.z;
            J.x=j.x;     J.y=j.y;     J.z=j.z;
            K.x=k.x;     K.y=k.y;     K.z=k.z;
            makeRotationMatrix();
        }

        void setOrigin(Vector3d origin) {O.x=origin.x; O.y=origin.y; O.z=origin.z;}

        void setIJK(Vector3d const &I_, Vector3d const &J_, Vector3d const &K_)
        {
            I.x=I_.x;     I.y=I_.y;     I.z=I_.z;
            J.x=J_.x;     J.y=J_.y;     J.z=J_.z;
            K.x=K_.x;     K.y=K_.y;     K.z=K_.z;
            makeRotationMatrix();
        }

        void makeRotationMatrix()
        {
            //create the transformation matrix
            lij[0]=I.x;       lij[1]=J.x;     lij[2]=K.x;
            lij[3]=I.y;       lij[4]=J.y;     lij[5]=K.y;
            lij[6]=I.z;       lij[7]=J.z;     lij[8]=K.z;

            invert33(lij);
        }

        void localToGlobal(Vector3d const &local, Vector3d &global) const
        {
            global.x = local.x * I.x + local.y * J.x + local.z * K.x;
            global.y = local.x * I.y + local.y * J.y + local.z * K.y;
            global.z = local.x * I.z + local.y * J.z + local.z * K.z;
        }

        Vector3d localToGlobal(Vector3d const &local) const
        {
            Vector3d L;
            L.x = local.x * I.x + local.y * J.x + local.z * K.x;
            L.y = local.x * I.y + local.y * J.y + local.z * K.y;
            L.z = local.x * I.z + local.y * J.z + local.z * K.z;
            return L;
        }

        void globalToLocal(Vector3d const &V, Vector3d &VLocal) const
        {
            VLocal.x = lij[0]*V.x +lij[1]*V.y +lij[2]*V.z;
            VLocal.y = lij[3]*V.x +lij[4]*V.y +lij[5]*V.z;
            VLocal.z = lij[6]*V.x +lij[7]*V.y +lij[8]*V.z;
        }

        /**
     * Converts the global coordinates of the input vector in local panel coordinates.
     * @param  V the vector in the global reference frame
     * @return The vector in the local reference frame
     */
        Vector3d globalToLocal(Vector3d const &global) const
        {
            Vector3d L;
            L.x = lij[0]*global.x +lij[1]*global.y +lij[2]*global.z;
            L.y = lij[3]*global.x +lij[4]*global.y +lij[5]*global.z;
            L.z = lij[6]*global.x +lij[7]*global.y +lij[8]*global.z;
            return L;
        }


        void localToGlobalPosition(double const&xl, double const &yl, double const &zl, double &XG, double &YG, double &ZG) const
        {
            XG = O.x + xl*I.x + yl*J.x + zl*K.x;
            YG = O.y + xl*I.y + yl*J.y + zl*K.y;
            ZG = O.z + xl*I.z + yl*J.z + zl*K.z;
        }

        Vector3d localToGlobalPosition(Vector3d const &Pl) const
        {
            Vector3d L;
            L.x = O.x + Pl.x*I.x + Pl.y*J.x + Pl.z*K.x;
            L.y = O.y + Pl.x*I.y + Pl.y*J.y + Pl.z*K.y;
            L.z = O.z + Pl.x*I.z + Pl.y*J.z + Pl.z*K.z;
            return L;
        }


        void globalToLocalPosition(Vector3d const &globalpos, Vector3d &localpos) const
        {
            double Vx=globalpos.x-O.x;
            double Vy=globalpos.y-O.y;
            double Vz=globalpos.z-O.z;
            localpos.x = lij[0]*Vx +lij[1]*Vy +lij[2]*Vz;
            localpos.y = lij[3]*Vx +lij[4]*Vy +lij[5]*Vz;
            localpos.z = lij[6]*Vx +lij[7]*Vy +lij[8]*Vz;
        }

        void globalToLocalPosition(double const&XG, double const&YG, double const&ZG, double &xl, double &yl, double &zl) const
        {
            double Vx=XG-O.x;
            double Vy=YG-O.y;
            double Vz=ZG-O.z;
            xl = lij[0]*Vx +lij[1]*Vy +lij[2]*Vz;
            yl = lij[3]*Vx +lij[4]*Vy +lij[5]*Vz;
            zl = lij[6]*Vx +lij[7]*Vy +lij[8]*Vz;
        }

        Vector3d globalToLocalPosition(Vector3d const &P) const
        {
            double Vx=P.x-O.x;
            double Vy=P.y-O.y;
            double Vz=P.z-O.z;
            Vector3d L;
            L.x = lij[0]*Vx +lij[1]*Vy +lij[2]*Vz;
            L.y = lij[3]*Vx +lij[4]*Vy +lij[5]*Vz;
            L.z = lij[6]*Vx +lij[7]*Vy +lij[8]*Vz;
            return L;
        }

    private:
        Vector3d O;
        Vector3d I, J, K;

        double lij[9];
};

