#include "stdafx.h"

#include "define.h"
#include "types.h"
#include "io.h"

//------------------------------------------------------------------------
// read_INT32()
//
// ACTION - Reads a 32 bit word from a file.
//------------------------------------------------------------------------
INT32 read_INT32(FILE *load_file) {
  INT32 return_value=0;
  fread( &return_value, sizeof(INT32), 1, load_file );
  return(return_value);
}

//------------------------------------------------------------------------
// read_INT16()
//
// ACTION - Reads a 16 bit word from a file.
//------------------------------------------------------------------------
INT16 read_INT16(FILE *load_file) {
  INT16 return_value=0;
  fread( &return_value, sizeof(INT16), 1, load_file );
  return(return_value);
}

//------------------------------------------------------------------------
// read_INT8()
//
// ACTION - Reads a 8 bit word from a file.
//------------------------------------------------------------------------
INT8 read_INT8(FILE *load_file) {
  INT8 return_value=0;
  fread( &return_value, sizeof(INT8), 1, load_file );
  return(return_value);
}

//------------------------------------------------------------------------
// read_FIX()
//
//  ACTION - Reads a FIX word from a file.
//------------------------------------------------------------------------
FIX read_FIX(FILE *load_file) {
  FIX return_value=0;
  fread( &return_value, sizeof(FIX), 1, load_file );
  return(return_value);
}

//------------------------------------------------------------------------
// read_FIXANG()
//
// ACTION - Reads a FIXANG word from a file.
//------------------------------------------------------------------------
FIXANG read_FIXANG(FILE *load_file) {
  FIXANG return_value=0;
  fread( &return_value, sizeof(FIXANG), 1, load_file );
  return(return_value);
}

//------------------------------------------------------------------------
// read_matrix()
//
//  ACTION - Reads a matrix structure from a file.
//------------------------------------------------------------------------
vms_matrix *read_matrix(vms_matrix *matrix,FILE *load_file) {
  read_vector(&matrix->rvec,load_file );
  read_vector(&matrix->uvec,load_file );
  read_vector(&matrix->fvec,load_file );
  return(matrix);
}

//------------------------------------------------------------------------
// read_vector()
//
// ACTION - Reads a vector structure from a file.
//------------------------------------------------------------------------
vms_vector *read_vector(vms_vector *vector,FILE *load_file) {
  fread( &vector->x, sizeof(FIX), 1, load_file );
  fread( &vector->y, sizeof(FIX), 1, load_file );
  fread( &vector->z, sizeof(FIX), 1, load_file );
  return(vector);
}

//------------------------------------------------------------------------
// read_angvec()
//
// ACTION - Reads a vector structure from a file.
//
//------------------------------------------------------------------------
vms_angvec *read_angvec(vms_angvec *vector,FILE *load_file) {
  fread( &vector->p, sizeof(FIXANG), 1, load_file );
  fread( &vector->b, sizeof(FIXANG), 1, load_file );
  fread( &vector->h, sizeof(FIXANG), 1, load_file );
  return(vector);
}

//------------------------------------------------------------------------
// write_INT32()
//
// ACTION - Writes a 32 bit word to a file.
//------------------------------------------------------------------------
INT32 write_INT32(INT32 value,FILE *save_file) {
  fwrite(&value, sizeof(INT32), 1, save_file );
  return(value);
}

//------------------------------------------------------------------------
// write_INT16()
//
// ACTION - Writes a 16 bit word to a file.
//------------------------------------------------------------------------
INT16 write_INT16(INT16 value,FILE *save_file) {
  fwrite(&value, sizeof(INT16), 1, save_file );
  return(value);
}

//------------------------------------------------------------------------
// write_INT8()
//
// ACTION - Writes a 8 bit word to a file.
//------------------------------------------------------------------------
INT8 write_INT8(INT8 value,FILE *save_file) {
  fwrite( &value, sizeof(INT8), 1, save_file );
  return(value);
}

//------------------------------------------------------------------------
// write_FIX()
//
// ACTION - Writes a FIX word to a file.
//------------------------------------------------------------------------
FIX write_FIX(FIX value,FILE *save_file) {
  fwrite( &value, sizeof(FIX), 1, save_file );
  return(value);
}

//------------------------------------------------------------------------
// write_FIXANG()
//
// ACTION - Writes a FIXANG word to a file.
//------------------------------------------------------------------------
FIXANG write_FIXANG(FIXANG value,FILE *save_file) {
  fwrite( &value, sizeof(FIXANG), 1, save_file );
  return(value);
}

//------------------------------------------------------------------------
// write_matrix()
//
// ACTION - Writes a matrix structure to a file.
//------------------------------------------------------------------------
vms_matrix *write_matrix(vms_matrix *matrix,FILE *save_file) {
  write_vector(&matrix->rvec,save_file );
  write_vector(&matrix->uvec,save_file );
  write_vector(&matrix->fvec,save_file );
  return(matrix);
}

//------------------------------------------------------------------------
// write_vector()
//
// ACTION - Writes a vector structure to a file.
//------------------------------------------------------------------------
vms_vector *write_vector(vms_vector *vector,FILE *save_file) {
  fwrite( &vector->x, sizeof(FIX), 1, save_file );
  fwrite( &vector->y, sizeof(FIX), 1, save_file );
  fwrite( &vector->z, sizeof(FIX), 1, save_file );
  return(vector);
}

//------------------------------------------------------------------------
// write_angvec()
//
// ACTION - Writes a vector structure to a file.
//------------------------------------------------------------------------
vms_angvec *write_angvec(vms_angvec *vector,FILE *save_file) {
  fwrite( &vector->p, sizeof(FIXANG), 1, save_file );
  fwrite( &vector->b, sizeof(FIXANG), 1, save_file );
  fwrite( &vector->h, sizeof(FIXANG), 1, save_file );
  return(vector);
}

                        /*---------------------------*/

static char *CopyIoName (char *dest, char *src, UINT16 srcLen, UINT16 destSize)
{
if (dest) {
   if (srcLen > --destSize)
      srcLen = destSize;
   strncpy (dest, src, srcLen);
   dest [srcLen] = '\0';
	}
return dest;
}

                        /*---------------------------*/

char *FSplit (char *fullName, char *pathName, char *fileName, char *extName)
{
   char *s;
   char	fn [256];
	INT32	l;

l = strlen (fullName);
memcpy (fn, fullName, l + 1);
if (pathName)
   *pathName = 0;
for (s = fn + l; (s != fn); --s) {
   if ((*s == ':') || (*s == '\\')) {
      CopyIoName (pathName, fn, (UINT16) (s - fn) + 1, 256);
      memmove (fn, s + 1, strlen (s));
      break;
      }
   }

for (s = fn + strlen (fn); (s != fn); --s)
   if ((*s == '.') || (*s == ':') || (*s == '\\'))
      break;
if (*s == '.') {
   CopyIoName (extName, s, (UINT16) strlen (s), 256);
   if (fileName) {
      *s = 0;
      CopyIoName (fileName, fn, (UINT16) strlen (fn), 256);
      }
   }
else {
   CopyIoName (fileName, fn, (UINT16) strlen (fn), 256);
   if (extName)
      *extName = 0;
   }
return pathName ? pathName : fileName ? fileName : extName;
}

/*----------------------------------------------------------------------------+
|                                                                             |
+----------------------------------------------------------------------------*/

#include <stdio.h>

static char *szMonths [] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

struct tm *GetTimeDate (struct tm *td)
{
   time_t t;
   struct tm *h;

time (&t);
h = localtime (&t);
h->tm_mon++;
if (!td)
   return h;
*td = *h;
td->tm_year += (td->tm_year < 80 ? 2000 : 1900);
return td;
}


char *TimeStr (char *pszTime)
{
	struct tm td;

GetTimeDate (&td);
sprintf (pszTime, "%d:%02d.%02d", td.tm_hour, td.tm_min, td.tm_sec);
return pszTime;
} 


char *DateStr (char *pszTime, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf (pszTime, "%d %s %d", td.tm_mday, szMonths [td.tm_mon - 1], td.tm_year);
else
	sprintf (pszTime, "%d/%d/%d", td.tm_mon, td.tm_mday, td.tm_year);
return pszTime;
} 


char *TimeDateStr (char *pszTime, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf (pszTime, "%d %s %d %d:%02d",
		       td.tm_mday, szMonths [td.tm_mon - 1], td.tm_year,
			    td.tm_hour, td.tm_min);
else
	sprintf (pszTime, "%d/%d/%d %d:%02d",
		       td.tm_mon, td.tm_mday, td.tm_year,
			    td.tm_hour, td.tm_min);
return pszTime;
} 


