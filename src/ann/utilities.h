/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.

    Surfpack: A Software Library of Multidimensional Surface Fitting Methods

    Surfpack is distributed under the DAKOTA GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */
#include "surfpack_config.h"

/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2001, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * File: utilities.h
 * -----------------
 * This file provides the header file for utilities.C
 */

#ifndef _utilities_h_
#define _utilities_h_


/*
 * Function: my_atanh
 * Usage: double my_atanh(x)
 * -------------------------
 * This function will return the arctanh of argument x.
 */
double my_atanh(double x);

/*
 * Function: multiplyMM 
 * Usage: DakotaRealMatix = multiplyMM(std::vector< std::vector< double > > m1, std::vector< std::vector< double > > m2)
 * ------------------------------------------------------------------------------
 * This function will multiply two matrices (result = m1*m2). 
 */
std::vector< std::vector< double > > multiplyMM(std::vector< std::vector< double > >& m1, std::vector< std::vector< double > >& m2);

/*
 * Function: FindMax
 * Usage: result = FindMax(std::vector< std::vector< double > > m)
 * -------------------------------------------
 * This function will return the maximum value for each
 * column of the matrix m in the form of a vector.
 */
std::vector<double> FindMax(std::vector< std::vector< double > > m);

/*
 * Function: FindMin
 * Usage: result = FindMin(std::vector< std::vector< double > > m)
 * -------------------------------------------
 * This function will return the minimum value for each
 * column of the matrix m in the form of a vector.
 */
std::vector<double> FindMin(std::vector< std::vector< double > > m);

#endif
