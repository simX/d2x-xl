#ifndef __systime_h
#define __systime_h

#include <time.h>

char *TimeStr (char * szTime);
char *DateStr (char * szTime, bool bMonthNames = false);
char *DateTimeStr (char * szTime, bool bMonthNames = false);
struct tm *GetTimeDate (struct tm *td);

#endif //__systime_h
