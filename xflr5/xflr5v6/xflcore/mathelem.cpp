/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#include <QVector>
#include <QDebug>

#include "mathelem.h"
#include "constants.h"
#include <xflcore/matrix.h>

#include <cstring>

/**
* Bubble sort algorithm for complex numbers
*@param array the array of complex numbers to sort
*@param ub the size of the array
*/
void sortComplex(std::complex<double>*array, int n)
{
    std::complex<double> temp(0), temp2(0);
    int flipped(0);

    if (n<=1) return;

    int indx = 1;
    do
    {
        flipped = 0;
        for (int indx2 = n - 1; indx2 >= indx; --indx2)
        {
            temp  = array[indx2];
            temp2 = array[indx2 - 1];
            if (compareComplex(temp2, temp) > 0)
            {
                array[indx2 - 1] = temp;
                array[indx2] = temp2;
                flipped = 1;
            }
        }
    } while ((++indx < n) && flipped);
}


/**
* Method for the comparison of two complex number
*@param a first complex number
*@param b second complex number
*@return 1 if Real(a) > Real(b), -1 if Real(a)<Real(b); if Real(a)=Real(b), returns 1 if Imag(a)>Image(b), -1 otherwise.
*/
int compareComplex(std::complex<double> a, std::complex<double>b)
{
    if(a.real()>b.real())       return  1;
    else if (a.real()<b.real()) return -1;
    else
    {	//same real part
        if(a.imag()>b.imag())         return  1;
        else if (a.imag()<b.imag())   return -1;
        else return 0;
    }
}


/**
// Given an array of n+1 pairs (x[i], y[i]), with i ranging from 0 to n,
// this function calculates the 3rd order cubic spline which interpolate the pairs.
//
// The spline is defined for each interval [x[j], x[j+1]) by n third order polynomial functions
//              p_j(x) = ax3 + bx2 + cx + d
//
// The equations to determine the coefficients a,b,c,d are
//
// Interpolation : 2n conditions
//    p_j(x[j])   = y[j];
//    p_j(x[j+1]) = y[j+1];
//
// Continuity of 1st and 2nd order derivatives at internal points: 2(n-1) conditions
//    p_j'(x[j]) = p_j+1'(x[j])
//    p_j"(x[j]) = p_j+1"(x[j])
//
// Second order derivative is zero at the end points : 2 conditions
//    p_j"(x[0]) =  p_j"(x[n]) =0
//
//
// This sets a linear system of size 4n which is solved by the Gauss algorithm for coefs a,b,c and d
// The RHS vector is
//	  a[0]
//	  b[0]
//	  c[0]
//	  d[0]
//	  a[1]
//    ...
//	  d[n-1]
*/
bool CubicSplineInterpolation(int n, double const *x, double const *y, double *a, double *b, double *c, double *d)
{
    if(n>50) return false;
    int size(0);

    QVector<double>M(16*n*n,0);
    QVector<double>RHS(4*n,0);


    size = 4*n;
//	Interpolation conditions
    for (int i=0; i<n; i++)
    {
        //pj(x[i]) = y[i]
        M[2*i*size +4*i]     = x[i]*x[i]*x[i];
        M[2*i*size +4*i + 1] = x[i]*x[i];
        M[2*i*size +4*i + 2] = x[i];
        M[2*i*size +4*i + 3] = 1.0;

        //pj(x[i+1]) = y[i+1]
        M[(2*i+1)*size +4*i]     = x[i+1]*x[i+1]*x[i+1];
        M[(2*i+1)*size +4*i + 1] = x[i+1]*x[i+1];
        M[(2*i+1)*size +4*i + 2] = x[i+1];
        M[(2*i+1)*size +4*i + 3] = 1.0;

        RHS[2*i]   = y[i];
        RHS[2*i+1] = y[i+1];
    }

//  Derivation conditions
    for (int i=1; i<n; i++)
    {
        //continuity of 1st order derivatives

        M[(2*n+i)*size + 4*(i-1)]     =  3.0*x[i]*x[i];
        M[(2*n+i)*size + 4*(i-1)+1]   =  2.0     *x[i];
        M[(2*n+i)*size + 4*(i-1)+2]   =  1.0;

        M[(2*n+i)*size + 4*i]   = -3.0*x[i]*x[i];
        M[(2*n+i)*size + 4*i+1] = -2.0     *x[i];
        M[(2*n+i)*size + 4*i+2] = -1.0;

        RHS[2*n+i]   = 0.0;

        //continuity of 2nd order derivatives
        M[(3*n+i)*size + 4*(i-1)]     =  6.0*x[i];
        M[(3*n+i)*size + 4*(i-1)+1]   =  2.0     ;

        M[(3*n+i)*size + 4*i]   = -6.0*x[i];
        M[(3*n+i)*size + 4*i+1] = -2.0     ;

        RHS[3*n+i]   = 0.0;
    }

//	second order derivative is zero at end points = "natural spline"
    M[2*n*size]     = 6.0*x[0];
    M[2*n*size+1]   = 2.0;
    RHS[2*n]        = 0.0;

    M[3*n*size + size-4]   = 6.0*x[n];
    M[3*n*size + size-3]   = 2.0;
    RHS[3*n+1]             = 0.0;

    bool bCancel = false;
    if(!Gauss(M.data(), 4*n, RHS.data(), 1, &bCancel)) return false;

    for(int i=0; i<n; i++)
    {
        a[i] = RHS[4*i];
        b[i] = RHS[4*i+1];
        c[i] = RHS[4*i+2];
        d[i] = RHS[4*i+3];
    }

    return true;
}


/**
 * Returns an array of (nPanels+1) double values spaced between 0 and 1, i.a.w. the specified distribution.
 */
void getPointDistribution(QVector<double> &fraclist, int nPanels, xfl::enumPanelDistribution DistType)
{
    fraclist.clear();
    fraclist.reserve(nPanels+1);
    double dN = double(nPanels);
    for(int i=0; i<=nPanels; i++)
    {
        double di = double(i)/dN;
        double d  = getDistribFraction(di, DistType);
        fraclist.push_back(d);
    }
}


double getDistribFraction(double tau, xfl::enumPanelDistribution DistType)
{
    switch (DistType)
    {
        case xfl::COSINE:
        {
            return 1.0/2.0*(1.0-cos(tau*PI));
        }
        case xfl::SINE:
        {
            return 1.0*(1.-cos(tau*PI/2.0));
        }
        case xfl::INVERSESINE:
        {
            return  1.0*(sin( tau*PI/2.0));
        }

        default: //UNIFORM
        case xfl::UNIFORM:
        {
            return tau;
        }
    }
}


/**
 * returns the ordinate y corresponding to coordinate x
 * on a line defined by points (x0,y0) and (x1,y1)
 */
double interpolateLine(double x, double x0, double y0, double x1, double y1)
{
    if(fabs(x1-x0)<PRECISION) return 0.0;
    double a = (y1-y0)/(x1-x0);
    return y0+a*(x-x0);
}


/** if bExtend is true, then interpolations will be made outside the interval defined by the x array
 * based on the end points
 * if false, then interpolation will return the first or last y-value */
double interpolatePolyLine(double x, QVector<double> const &xp, QVector<double> const &yp, bool bExtend)
{
    if(xp.size() != yp.size()) return 0.0;

    int n = xp.size();
    if(n<2) return 0.0; // cannot interpolate


    if(x<=xp[0])
    {
        if(bExtend) return interpolateLine(x, xp[0], yp[0], xp[1], yp[1]);
        else        return yp.front();
    }
    if(x>=xp[n-1])
    {
        if(bExtend) return interpolateLine(x, xp[n-2], yp[n-2], xp[n-1], yp[n-1]);
        else        return yp.back();
    }

    for(int i=1; i<xp.size()-1; i++)
    {
        if(xp[i]<=x && x<=xp[i+1]) return interpolateLine(x, xp[i], yp[i], xp[i+1], yp[i+1]);
    }
    return 0.0; // never reached
    Q_ASSERT(true);
}


xfl::enumPanelDistribution distributionType(QString const &Dist)
{
    QString strDist = Dist.trimmed();
    if     (strDist.compare("COSINE",      Qt::CaseInsensitive)==0) return xfl::COSINE;
    else if(strDist.compare("UNIFORM",     Qt::CaseInsensitive)==0) return xfl::UNIFORM;
    else if(strDist.compare("SINE",        Qt::CaseInsensitive)==0) return xfl::SINE;
    else if(strDist.compare("INVERSESINE", Qt::CaseInsensitive)==0) return xfl::INVERSESINE;
    else
    {
        qDebug()<<"String distrib not found"<<strDist;
        return xfl::UNIFORM;
    }
}


QString distributionType(xfl::enumPanelDistribution dist)
{
    switch(dist)
    {
        default:
        case xfl::UNIFORM:
            return "UNIFORM";
        case xfl::COSINE:
            return "COSINE";
        case xfl::SINE:
            return "SINE";
        case xfl::INVERSESINE:
            return "INVERSESINE";
    }
}


/**
* Tests if a given integer is between two other integers
*@param f the integer to test
*@param f1 the first bound
*@param f2 the second bound
*@return true if f1<f<f2 or f2<f<f1
*/
bool isBetween(int f, int f1, int f2)
{
    if (f2 < f1)
    {
        int tmp = f2;
        f2 = f1;
        f1 = tmp;
    }
    if     (f<f1) return false;
    else if(f>f2) return false;
    return true;
}


/**
 * Tests if a given integer is between two double values
 * @param f the integer to test
 * @param f1 the first bound
 * @param f2 the second bound
 * @return true if f1<f<f2 or f2<f<f1
 */
bool isBetween(int f, double f1, double f2)
{
    double ff = f;
    if (f2 < f1)
    {
        double tmp = f2;
        f2 = f1;
        f1 = tmp;
    }
    if     (ff<f1) return false;
    else if(ff>f2) return false;
    return true;
}


/** input uniformly spaced in [0,1], ouput bunched in [0,1]
    BunchAmp:  k=0.0 --> uniform bunching, k=1-->full varying bunch
    BunchDist: k=0.0 --> uniform bunching, k=1 weigth on endpoints
*/
double bunchedParameter(double bunchdist, double bunchamp, double t)
{
    // complex mix of three functions
    // linear, tanh, and atanh
    double flin = -1.0+t*2.0;

    if(bunchdist<0.5)
    {
        //mix linear and tanh
        double mix = 2.0*bunchdist;
        double a0 = (0.001+tanh(bunchamp))*3.5;
        double fth = tanh(a0*(2.0*t-1.0))/tanh(a0);
        return (1.0+mix*flin + (1.0-mix)*fth)/2.0;
    }
    else
    {
        //mix linear and atanh
        double mix = 2.0*(bunchdist-0.5);
        double a1 = (0.001+tanh(3.0*bunchamp))*0.99;
        double fath = atanh(a1*(2.0*t-1.0))/atanh(a1);
        return (1.0+(1.0-mix)*flin + mix*fath)/2.0;
    }
}

/** Performs a linear regression of the array of n points (x_i, y_i) and
 * calculater the coefficients of line y = ax+b.
 * Returns true if the line could be calculated, false if not */
bool linearRegression(int n, double const *x, double const*y, double &a, double &b)
{
    a = b = 0;

    double sum_x(0), sum_y(0);
    double sum_x2(0);
    double sum_xy(0);

    for(int i=0; i<n; i++)
    {
        sum_x  += x[i];
        sum_y  += y[i];
        sum_xy += x[i]*y[i];
        sum_x2 += x[i]*x[i];
    }
    double dn = double(n);
    double denom = dn*sum_x2 - sum_x*sum_x;
    if(fabs(denom)<1.e-6) return false;

    a = (dn*sum_xy-sum_x*sum_y)/denom;
    b = (sum_y - a*sum_x)/dn;

    return true;
}


/** Hicks-Henne bump function
 * parameter t1 controls the bump's position and t2 its width
 */
double HH(double x, double t1, double t2)
{
    if(x<=0.0 || x>=1.0) return 0.0;
    return pow(sin(PI*pow(x, log(0.5)/log(t1))), t2);
}


/**
 * Calculates the coefficients of Legendre's polynomial of degree n
 * Uses the recursive formula n.Pnp1(x) = (2n-1).x.Pnm1(x) - (n-1) Pnm2(x)
 * P0(x) = 1; P1(x) = x;
 */
void Legendre(int n, double *a)
{
    if(n==0)
    {
        a[0] = 1.0;
    }
    else if(n==1)
    {
        a[0] = 0.0;
        a[1] = 1.0;
    }
    else
    {
        QVector<double> a_nm1(n,0);
        QVector<double> a_nm2(n-1,0);

        Legendre(n-1, a_nm1.data());
        for(int i=0; i<n; i++)
            a[i+1]  = double(2*n-1)/double(n) * a_nm1[i];

        Legendre(n-2, a_nm2.data());
        for(int i=0; i<n-1; i++)
            a[i] += -double(n-1)/double(n) * a_nm2[i];
    }
}


double Laguerre(int alpha, int k, double x)
{
    if(k==0) return 1.0;
    if(k==1) return double(1+alpha)-x;
    return (double(alpha+1)-x)/double(k) * Laguerre(alpha+1, k-1, x) - x/double(k) * Laguerre(alpha+2, k-2, x);
}


int factorial(int n)
{
    switch(n)
    {
        case 0: return 1;
        case 1: return 1;
        case 2: return 2;
        case 3: return 6;
        case 4: return 24;
        case 5: return 120;
        case 6: return 750;
        case 7: return 5040;
        default: break;
    }
    return n*factorial(n-1);
}


int binomial(int n, int k)
{
    if(k==0) return 1;
    if(k==n) return 1;
    return binomial(n-1, k-1) + binomial(n-1, k);
}


double LegendreAssociated(int m, int l, double x)
{
    if(m<0)
        return -1.0* double(factorial(l+m))/double(factorial(l-m))*LegendreAssociated(-m, l, x);

    if(m==0  && l==0) return 1;
    if(m==0  && l==1) return x;
    if(m==1  && l==1) return -sqrt(1.0-x*x);
    if(m==-1 && l==1) return 0.5*sqrt(1.0-x*x);

    if(l==m)   return - double(2*l-1) * sqrt(1.0-x*x) * LegendreAssociated(m-1, l-1, x);
    if(m==l-1) return x*double(2*l+1) * LegendreAssociated(m,m,x);

    return 1.0/double(l-m+1) * ( double(2*l+1) * x * LegendreAssociated(m, l-1, x)
                                -double(l+m)       * LegendreAssociated(m, l-2, x));
}


/** theta and phi in radians */
std::complex<double> LaplaceHarmonic(int m, int l, double theta, double phi)
{
    std::complex<double> harmonic(0,0);
    if(abs(m)>l) return harmonic;

    double N = sqrt(double(2*l+1)/4.0/PI* factorial(l-m)/factorial(l+m));
    double LA = LegendreAssociated(m, l, cos(theta));

    harmonic.real(N * LA * cos(m*phi));
    harmonic.imag(N * LA * sin(m*phi));

    return harmonic;
}


void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &zeta)
{
    omega1 = fabs(lambda.imag());

    if(omega1 > PRECISION)
    {
        omegaN = sqrt(lambda.real()*lambda.real()+omega1*omega1);
        zeta = -lambda.real()/omegaN;
    }
    else
    {
        omegaN = 0.0;
        zeta = 0.0;
    }
}




