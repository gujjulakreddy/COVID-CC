#pragma once
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3F81279C_3B02_44F9_B5AB_4886E077D54B__INCLUDED_)
#define AFX_STDAFX_H__3F81279C_3B02_44F9_B5AB_4886E077D54B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_WARNINGS

// Bring in the CPLEX function declarations and the C library 
// header file stdio.h with the following single include 
//#include "stdafx.h"
#include <cplex.h>
#include <iostream>
#include <sstream> 
#include <cstdlib>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cmath>
#include <assert.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>

#define BUFLEN 1024			// Line length
#define FLEN  32			// Word length for input file
#define NLEN  48			// Max length of name
#define ZERO_CONST  1e-9	// Epsilon
#define NCOLS	3			// Number of columns in client input data: Client ID #, Client Pathway ID #, Client Start Date
#define NyVars 4000
//#define NxVARS  4000		// Max number of x decision variables: for convenience 
//#define MAX_REQS	200		// Max number of client requests per day
//#define MAX_REPLICS 100		// Max number of replications


//#define TIGHTNING 0
//#define BIGM_FACTOR  100
//#define BIGM_FACTOR2  10
//#define ZERO 1e-3
//#define ZERO_PROB 1e-4
//#define SEED_INCREM 100




//Added for load balancing
//struct ResAssign {
//	//char resAssignPathwayName;
//	//int assignSteps;
//	char resAssignType[4];				// The resource type assigned 
//	int resAssignID;	// Resource types 
//	int resAssignDays;			// Resource array indices corresponding to resourceType[NLEN] arrray
//	int resAssignTimeSlots;			// Resource array indices corresponding to resourceType[NLEN] arrray
//};

using namespace std;

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__3F81279C_3B02_44F9_B5AB_4886E077D54B__INCLUDED_)

int main(int argc, char* argv[]);

// METHODS
/***
* Rounding method
*
*/
//int myround(double num);

/***
* This method generates a bernoulli (with p = 0.5) random number (0 or 1)
*
* Author" L. Ntaimo
*/
//int bernoulli(double p, long *seed1, long *seed2);

/***
* This method generates a number from unif(lb, ub)
*
* Author" L. Ntaimo
*/
//int uniform(int lb, int ub, long *seed1, long *seed2);


/**********************************************************************
*	RANDOM.C -- Random variate generation routines for the harbor
*		simulation.  The uniform generator is pretty good, and is
*		used by all the other routines.  The African tanker
*		problem is described on p. 112 of Law & Kelton, and uses
*		uniform, multinomial, and exponential RV's.  I have
*		included a normal approximation to test distribution
*		shape effects.  A true normal generator is not needed
*		to test the effect of shape differences, and besides,
*		the approximation isn't at all bad.
*		Last modified 10/11/86 -- PJS
**********************************************************************/

/*********************************************************************
*
*  NOTE: To use u0_1, you need to pass pointers to 2 seeds, as in:
*                               u0_1(&seed1, &seed2)
*  These will be changed at the end of u0_1, so be sure to retain
*  a copy of your initial seed if you need to control for a common
*  sample.
*
*  JLH 12/9/03
**********************************************************************/

//#include <math.h>		/* for exponential, log and power fns	 */

//double u0_1(long *seed1, long *seed2);

/* This generator XOR's the result of an LCG	 */
/* and a Tausworth generator to produce a	     */
/* value.  It passed the Kolmogorov-Smirnov,	 */
/* chi-square, birthday, and runs tests.	     */
/* Since the periods of the two components are	 */
/* relatively prime, it has a period of ~2^60	 */

//long lcg(long int_x);

/* Multiplicative Congruential, actually.	 */
/* Period is 2^31 -2.  Algorithm is Schrage's	 */
/* portable pmmlcg, translated from FORTRAN.	 */

//long tausworthe(long x);

/* Tausworth generator.  Period is (2^31)-1.	 */
/* Algorithm is from Bratley, Fox & Schrage.	 */

