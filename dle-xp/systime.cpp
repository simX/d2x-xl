#include <stdio.h>
#include "systime.h"

static char *szMonths [] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

                        /*--------------------------*/

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

                        /*--------------------------*/

char *DateTimeStr (char * szTime, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf (szTime, "%d %s %d %d:%02d",
				 td.tm_mday, szMonths [td.tm_mon], td.tm_year + 1900,
				 td.tm_hour, td.tm_min);
else
	sprintf (szTime, "%d/%d/%d %d:%02d",
				 td.tm_mon, td.tm_mday, td.tm_year,
				 td.tm_hour, td.tm_min);
return szTime;
} 

                        /*--------------------------*/

char * TimeStr (char * szTime)
{
	struct tm td;

GetTimeDate (&td);
sprintf (szTime, "%d:%02d.%02d", td.tm_hour, td.tm_min, td.tm_sec);
return szTime;
} 

                        /*--------------------------*/

char *DateStr (char * szTime, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf (szTime, "%d %s %d", td.tm_mday, szMonths [td.tm_mon], td.tm_year);
else
	sprintf (szTime, "%d/%d/%d", td.tm_mon, td.tm_mday, td.tm_year);
return szTime;
} 

                        /*--------------------------*/

