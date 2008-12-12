/* Maths.h library header file */

#ifndef _MATHS_H
#define _MATHS_H

#define NO_FIX_INLINE 0
#define ALT_SQRT 2

#include <stdlib.h>
#include "pstypes.h"

#define D_RAND_MAX 32767

void d_srand (unsigned int seed);
int d_rand ();			// Random number function which returns in the range 0-0x7FFF


//=============================== FIXED POINT ===============================

#ifndef Pi
#  define  Pi    3.141592653589793240
#endif

typedef int32_t fix;		//16 bits int, 16 bits frac
typedef int16_t fixang;		//angles

typedef struct quadint // integer 64 bit, previously called "quad"
  {
    u_int32_t low;
    int32_t high;
  }
quadint;

//Convert an int to a fix
#define i2f(i) (((fix) (i)) << 16)

//Get the int part of a fix
#define f2i(f) ((f)>>16)

//Get the int part of a fix, with rounding
#define f2ir(f) (((f)+f0_5)>>16)

//Convert fix to double and double to fix
#define f2fl(f) (((float)  (f)) / (float) 65536)
#define f2db(f) (((float) (f)) / (float) 65536)
#define fl2f(f) ((fix) ((f) * 65536))

//Some handy constants
#define f0_0	0
#define f1_0	0x10000
#define f2_0	0x20000
#define f3_0	0x30000
#define f10_0	0xa0000

#define f0_5 0x8000
#define f0_1 0x199a

#define F0_0	f0_0
#define F1_0	f1_0
#define F2_0	f2_0
#define F3_0	f3_0
#define F10_0	f10_0

#define F0_5 	f0_5
#define F0_1 	f0_1

#ifdef _MSC_VER
#	define QLONG __int64
#else
#	define QLONG long long
#endif


//#if defined(NO_FIX_INLINE) || (!defined(__GNUC__) && !defined(__WATCOMC__))
#if NO_FIX_INLINE
//multiply two fixes, return a fix
fix FixMul (fix a, fix b);

//divide two fixes, return a fix
fix FixDiv (fix a, fix b);

//multiply two fixes, then divide by a third, return a fix
fix FixMulDiv (fix a, fix b, fix c);

#else

#define FixMul(_a, _b)	((fix) ((((QLONG) (_a)) * (_b)) / 65536))
#define FixDiv(_a, _b)	((fix) ((_b) ? ((((QLONG) (_a)) * 65536) / (_b)) : 1))
#define FixMulDiv(_a, _b, _c) ((fix) ((_c) ? ((((QLONG) (_a)) * (_b)) / (_c)) : 1))

#endif

//extract a fix from a quadint product
fix fixquadadjust (quadint * q);

//divide a quadint by a long
int32_t fixdivquadlong (u_int32_t qlow, u_int32_t qhigh, u_int32_t d);

//negate a quadint
void fixquadnegate (quadint * q);

//computes the square root of a long, returning a short
ushort long_sqrt (int32_t a);

//computes the square root of a quadint, returning a long
extern int nMathFormat;
extern int nDefMathFormat;

unsigned int sqrt64 (unsigned QLONG a);

#define mul64(_a,_b)	((QLONG) (_a) * (QLONG) (_b))

//multiply two fixes, and add 64-bit product to a quadint
void fixmulaccum (quadint * q, fix a, fix b);

u_int32_t quad_sqrt (u_int32_t low, int32_t high);
//unsigned long quad_sqrt (long low, long high);

//computes the square root of a fix, returning a fix
fix fix_sqrt (fix a);

//compute sine and cosine of an angle, filling in the variables
//either of the pointers can be NULL
void FixSinCos (fix a, fix * s, fix * c);	//with interpolation

void FixFastSinCos (fix a, fix * s, fix * c);	//no interpolation

//compute inverse sine & cosine
fixang fix_asin (fix v);

fixang fix_acos (fix v);

//given cos & sin of an angle, return that angle.
//parms need not be normalized, that is, the ratio of the parms cos/sin must
//equal the ratio of the actual cos & sin for the result angle, but the parms 
//need not be the actual cos & sin.  
//NOTE: this is different from the standard C atan2, since it is left-handed.
fixang fix_atan2 (fix cos, fix sin);

//for passed value a, returns 1/sqrt(a) 
fix fix_isqrt (fix a);

#endif
