/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois 
    GNU General Public License v3

*****************************************************************************/



#pragma once

#include <xflgeom/geom2d/vector2d.h>

class CartesianFrame2d
{

    friend class Panel;
    friend class Panel3;
    friend class Panel4;

    public:
        CartesianFrame2d()
        {
            O.set(0.0,0.0);
            I.set(1.0,0.0);

            lij[0]=1.0;  lij[1]=0.0;
            lij[2]=0.0;  lij[3]=1.0;
        }

        CartesianFrame2d(Vector2d const &origin, Vector2d const &_I)
        {
            O = origin;
            I=_I;
            makeRotationMatrix();
        }


        void setFrame(Vector2d const &origin, Vector2d const &_I)
        {
            O = origin;
            I=_I.normalized();
            makeRotationMatrix();
        }

        void setI(Vector2d const &_I)
        {
            I=_I.normalized();
            makeRotationMatrix();
        }

        void setOrigin(Vector2d origin) {O=origin;}


        void makeRotationMatrix()
        {
            Vector2d J(-I.y, I.x);

            //create the transformation matrix
            double det = I.x*J.y-I.y*J.x;
            if(fabs(det)<1.e-20)
            {
                lij[0] = lij[3] = 1.0;
                lij[1] = lij[2] = 0.0;
            }
            else
            {
                lij[0] =  J.y/det;
                lij[1] = -J.x/det;
                lij[2] = -I.y/det;
                lij[3] =  I.x/det;
            }
        }

        void localToGlobal(Vector2d const &local, Vector2d &global) const
        {
            Vector2d J(-I.y, I.x);

            global.x = local.x * I.x + local.y * J.x;
            global.y = local.x * I.y + local.y * J.y;
        }

        Vector2d localToGlobal(Vector2d const &local) const
        {
            Vector2d L;
            Vector2d J(-I.y, I.x);

            L.x = local.x * I.x + local.y * J.x;
            L.y = local.x * I.y + local.y * J.y;
            return L;
        }


        void globalToLocal(Vector2d const &V, Vector2d &VLocal) const
        {
            VLocal.x = lij[0]*V.x +lij[1]*V.y;
            VLocal.y = lij[2]*V.x +lij[3]*V.y;
        }

        Vector2d globalToLocal(Vector2d const &global) const
        {
            Vector2d L;
            L.x = lij[0]*global.x +lij[1]*global.y;
            L.y = lij[2]*global.x +lij[3]*global.y;

            return L;
        }

        void localToGlobalPosition(double const&xl, double const &yl, double &XG, double &YG) const
        {
            Vector2d J(-I.y, I.x);

            XG = O.x + xl*I.x + yl*J.x;
            YG = O.y + xl*I.y + yl*J.y;
        }

        Vector2d localToGlobalPosition(Vector2d const &Pl) const
        {
            Vector2d L;
            Vector2d J(-I.y, I.x);

            L.x = O.x + Pl.x*I.x + Pl.y*J.x;
            L.y = O.y + Pl.x*I.y + Pl.y*J.y;
            return L;
        }


        void globalToLocalPosition(Vector2d const &globalpos, Vector2d &localpos) const
        {
            double Vx=globalpos.x-O.x;
            double Vy=globalpos.y-O.y;
            localpos.x = lij[0]*Vx +lij[1]*Vy;
            localpos.y = lij[2]*Vx +lij[3]*Vy;
        }

        void globalToLocalPosition(double const&XG, double const&YG, double &xl, double &yl) const
        {
            double Vx=XG-O.x;
            double Vy=YG-O.y;
            xl = lij[0]*Vx +lij[1]*Vy;
            yl = lij[2]*Vx +lij[3]*Vy;
        }


        Vector2d globalToLocalPosition(Vector2d const &P) const
        {
            double Vx=P.x-O.x;
            double Vy=P.y-O.y;
            Vector2d L;
            L.x = lij[0]*Vx +lij[1]*Vy;
            L.y = lij[2]*Vx +lij[3]*Vy;
            return L;
        }


    private:
        Vector2d O;
        Vector2d I; // x-axis unit vector in global coordinates

        double lij[4];
};

