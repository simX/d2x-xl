/*
 * $Revision: 1.1 $
 * $Author: bradleyb $
 * $Date: 2001/11/11 23:39:22 $
 *
 * Macro to make file signatures
 *
 * $Log: makesig.h,v $
 * Revision 1.1  2001/11/11 23:39:22  bradleyb
 * Created header for MAKE_SIG macro
 *
 *
 */

#ifndef _MAKESIG_H
#define _MAKESIG_H

#define MAKE_SIG(a,b,c,d) (((long)(a)<<24)+((long)(b)<<16)+((c)<<8)+(d))

#endif
