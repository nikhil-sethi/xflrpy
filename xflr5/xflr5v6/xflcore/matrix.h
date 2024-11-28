/****************************************************************************

    Matrix Functions 
    Copyright (C) 2008-2017 André Deperrois 

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


#ifndef MATRIX_H
#define MATRIX_H

#include <xflgeom/geom3d/vector3d.h>
#include <complex>


void transpose33(double *l);
bool invert33(double *l);
void AV33(const double *A, const double *v, double *p);


double det33(const double *aij);
std::complex<double> det33(const std::complex<double> *aij);


double det44(const double *aij);

std::complex<double> det44(const std::complex<double> *aij);
std::complex<double> cofactor44(const std::complex<double> *aij, int &i, int &j);

bool Invert44(const std::complex<double> *ain, std::complex<double> *aout);


bool Gauss(double *A, int n, double *B, int m, bool *pbCancel);


bool Crout_LU_Decomposition_with_Pivoting(double *A, int pivot[], int n, bool *pbCancel, double TaskSize, double &Progress);
bool Crout_LU_with_Pivoting_Solve(const double *LU, double B[], int pivot[], double x[], int n, bool *pbCancel);


void TestEigen();
void CharacteristicPol(double m[][4], double p[5]);
bool LinBairstow(double *p, std::complex<double> *root, int n);
bool Eigenvector(double a[][4], std::complex<double> lambda, std::complex<double> *V);


int Compare(std::complex<double> a, std::complex<double>b);
void ComplexSort(std::complex<double>*array, int ub);


void display_mat(double const *mat, int rows, int cols);
void display_vec(double const *vec, int rows);

void displayDouble(double d0, double d1=-2.0e50, double d2=-2.0e50, double d3=-2.0e50, double d4=-2.0e50, double d5=-2.0e50, double d6=-2.0e50, double d7=-2.0e50, double d8=-2.0e50, double d9=-2.0e50);


#endif // MATRIX_H
