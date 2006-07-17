/*  _______________________________________________________________________
 
    Surfpack: A Software Library of Multidimensional Surface Fitting Methods
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Surfpack directory.
    _______________________________________________________________________ */

#include "surfpack_config.h"

#ifndef SYSTEM_DEFS_H
#define SYSTEM_DEFS_H

/* C++ headers */

//#ifdef __cplusplus
#include <iostream>
#include <fstream>
/* Migration to sstream will require some source changes from ostrstream
   to ostringstream syntax.  Wait for support in other compilers.
#if defined(__GNUC__) && __GNUC__ >= 3
#include <sstream>
*/
//#include <strstream>
#include <iomanip>
//using namespace std;
//#endif /* __cplusplus */

/* C headers */

#ifdef HAVE_STD
/* C++ compiler using new C headers */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cassert>
#include <csignal>
#include <cerrno>
#include <ctime>
#else
/* C compiler (old style headers are used to avoid std namespace)
   or a C++ compiler which uses old C headers (SGI, TFLOP) */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#endif /* HAVE_STD */

#endif /* SYSTEM_DEFS_H */
