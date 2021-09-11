/****************************************************************************

    xflr5 v6
    Copyright (C) Andre Deperrois
    GNU General Public License v3

*****************************************************************************/

#pragma once


#include <QString>

#include <complex>

#include <xflcore/core_enums.h>


void testEigen();

void Legendre(int n, double *a);
double LegendreAssociated( int m, int l, double x);
double Laguerre(int alpha, int k, double x);

std::complex<double> LaplaceHarmonic(int m, int l, double theta, double phi);

int factorial(int n);
int binomial(int n, int k);


int compareComplex(std::complex<double> a, std::complex<double>b);
void sortComplex(std::complex<double>*array, int ub);


bool CubicSplineInterpolation(int n, const double *x, const double *y, double *a, double *b, double *c, double *d);


void getPointDistribution(QVector<double> &fraclist, int nPanels, xfl::enumPanelDistribution DistType=xfl::COSINE);
double getDistribFraction(double tau, xfl::enumPanelDistribution DistType=xfl::COSINE);


double err_func(double x);
double erf_inv(double a);

double interpolateLine(double x, double x0, double y0, double x1, double y1);

double interpolatePolyLine(double x, const QVector<double> &xp, const QVector<double> &yp, bool bExtend);


QString distributionType(xfl::enumPanelDistribution dist);
xfl::enumPanelDistribution distributionType(const QString &Dist);

bool isEven(int n);
bool isOdd(int n);
bool isBetween(int f, int f1, int f2);
bool isBetween(int f, double f1, double f2);

double bunchedParameter(double bunchdist, double bunchamp, double t);

bool linearRegression(int n, double const *x, double const*y, double &a, double &b);

double HH(double x, double t1, double t2, double xmin, double xmax);

void modeProperties(std::complex<double> lambda, double &omegaN, double &omega1, double &zeta);
