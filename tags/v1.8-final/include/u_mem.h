/* $Id: u_mem.h,v 1.7 2003/11/27 00:21:04 btb Exp $ */
/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#include <stdlib.h>
#include "pstypes.h"

extern int bShowMemInfo;

#ifdef _DEBUG
#	define DBG_MALLOC	1
#else
#	define DBG_MALLOC 0
#endif

#ifdef DBG_MALLOC	

#define D2X_MEM_HANDLER

void _CDECL_ mem_display_blocks(void);
void * mem_malloc( unsigned int size, char * var, char * file, int line, int fill_zero );
void * mem_realloc( void * buffer, unsigned int size, char * var, char * file, int line );
void mem_free( void * buffer );
char * mem_strdup(char * str, char * var, char * file, int line );
void mem_init();

/* DPH: Changed malloc, etc. to d_malloc. Overloading system calls is very evil and error prone */
#define d_malloc(size)      mem_malloc((size),"Unknown", __FILE__,__LINE__, 0 )
#define d_calloc(n,size)    mem_malloc((n*size),"Unknown", __FILE__,__LINE__, 1 )
#define d_realloc(ptr,size) mem_realloc((ptr),(size),"Unknown", __FILE__,__LINE__ )
#define d_free(ptr)         { mem_free(ptr); ptr=NULL; } 
#define d_strdup(str)       mem_strdup((str),"Unknown",__FILE__,__LINE__)

#define MALLOC( var, nType, count )   (var=(nType *)mem_malloc((count)*sizeof(nType),#var, __FILE__,__LINE__,0 ))

// Checks to see if any blocks are overwritten
void mem_validate_heap();

#else

#define d_malloc(size)      malloc(size)
#define d_calloc(n, size)   calloc(n, size)
#define d_realloc(ptr,size) realloc(ptr,size)
#define d_free(ptr)         { free(ptr); ptr=NULL; }
#define d_strdup(str)       strdup(str)

#define MALLOC( var, nType, count )   (var=(nType *)malloc((count)*sizeof(nType)))

#endif
