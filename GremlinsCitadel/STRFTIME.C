/* -------------------------------------------------------------------- */
/*  STRFTIME.C               Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  This file contains functions relating to the time and date          */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  strftime()      formats a custom time and date string using formats */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data/functions                                               */
/* -------------------------------------------------------------------- */
       char *monthTab[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" } ;
static char *fullmnts[12] = {"January",   "February", "March",    "April",
                             "May",       "June",     "July",     "August",
                             "September", "October",  "November", "December" };
static char *days[7]      = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *fulldays[7]  = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday" } ;




/* extern char *tzname[2]; */
/* -------------------------------------------------------------------- */
/*  sstrftime()     formats a custom time and date string using formats */
/* -------------------------------------------------------------------- */
void strftime(char *outstr, int maxsize, char *formatstr, long tnow)
{
    int i, k;
    char temp[30];
    struct tm *tmnow;      

    if (tnow == 0l) time(&tnow);

    tmnow = localtime(&tnow);

    outstr[0] = '\0';

    for(i=0; formatstr[i]; i++)
    {
        if(formatstr[i] != '%')
            sprintf(temp, "%c", formatstr[i]);
        else
        {
            i++;
            temp[0] = '\0';
            if(formatstr[i])
            switch(formatstr[i])
            {
            case 'a': /* %a  abbreviated weekday name */
                    sprintf(temp, "%s", days[tmnow->tm_wday]);
                    break;
            case 'A': /*  %A  full weekday name */
                    sprintf(temp, "%s", fulldays[tmnow->tm_wday]);
                    break;
            case 'b': /*  %b  abbreviated month name */
                    sprintf(temp, "%s", monthTab[tmnow->tm_mon]);
                    break;
            case 'B': /*  %B  full month name */
                    sprintf(temp, "%s", fullmnts[tmnow->tm_mon]);
                    break;
            case 'c': /*  %c  standard date and time string */
                    sprintf(temp, "%s", ctime(&tnow));
                    temp[strlen(temp)-1] = '\0';
                    break;
            case 'd': /*  %d  day-of-month as decimal (1-31) */
                    sprintf(temp, "%d", tmnow->tm_mday);
                    break;
            case 'D': /*  %D  day-of-month as decimal (01-31) */
                    sprintf(temp, "%02d", tmnow->tm_mday);
                    break;
            case 'H': /*  %H  hour, range (00-23) */
                    sprintf(temp, "%02d", tmnow->tm_hour);
                    break;
            case 'I': /*  %I  hour, range (01-12) */
                    if(tmnow->tm_hour)
                        sprintf(temp, "%02d", tmnow->tm_hour > 12 ?
                                              tmnow->tm_hour - 12 :
                                              tmnow->tm_hour );
                    else
                        sprintf(temp, "%d", 12);
                    break;
            case 'j': /*  %j  day-of-year as a decimal (001-366) */
                    sprintf(temp, "%03d", tmnow->tm_yday + 1);
                    break;
            case 'm': /*  %m  month as decimal (01-12) */
                    sprintf(temp, "%02d", tmnow->tm_mon + 1);
                    break;
            case 'M': /*  %M  minute as decimal (00-59) */
                    sprintf(temp, "%02d", tmnow->tm_min);
                    break;
            case 'p': /*  %p  locale's equivaent af AM or PM */
                    sprintf(temp, "%s", tmnow->tm_hour > 11 ? "PM" : "AM");
                    break;
            case 'S': /*  %S  second as decimal (00-59) */
                    sprintf(temp,"%02d", tmnow->tm_sec);
                    break;
            case 'U': /*  %U  week-of-year, Sunday being first day (00-52) */
                    k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                    if(k<0) k += 7;
                    if(k != 0)
                    {
                        k = tmnow->tm_yday - (7-k);
                        if(k<0) k = 0;
                    }
                    else
                        k = tmnow->tm_yday;
                    sprintf(temp, "%02d", k/7);
                    break;
            case 'W': /*  %W  week-of-year, Monday being first day (00-52) */
                    k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                    if(k<0) k += 7;
                    if(k != 1)
                    {
                        if(k==0) k = 7;
                        k = tmnow->tm_yday - (8-k);
                        if(k<0) k = 0;
                    }
                    else
                        k = tmnow->tm_yday;
                    sprintf(temp, "%02d", k/7);
                    break;
            case 'w': /*  %w  weekday as a decimal (0-6, sunday being 0) */
                    sprintf(temp, "%d", tmnow->tm_wday);
                    break;
            case 'x': /*  %x  standard date string */
#if 1
                *temp = '\0';
                k = (int)(tmnow->tm_mday-1) / 7 + 1;  /* week of the month */

                switch (tmnow->tm_mon + 1)
                {

                case 1:                     /** January **/
                    if (1 == tmnow->tm_mday)          /* 1st */
                      strcpy(temp,                    "New Years Day");
                    else
                    if (1 == tmnow->tm_wday &&        /* Monday */
                        3 == k)                       /* 3rd Monday */
                      strcpy(temp,                    "M. L. King, Jr. Day");
                    break;

                case 2:                     /** February **/
                    if (14 == tmnow->tm_mday)         /* 14th */
                      strcpy(temp,                    "Valentine's Day");
                    else 
                    if (18 == tmnow->tm_mday)         /* 18th */
                      strcpy(temp,                    "President's Day");
                    break;

                case 3:                     /** March **/
                    if (17 == tmnow->tm_mday)         /* 17th */
                      strcpy(temp,                    "St. Patrick's Day");
                    else
                    if (24 == tmnow->tm_mday)         /* 24th */
                      strcpy(temp,                    "Err Head's Birthday");
                    break;

                case 4:                     /** April **/
                    if (1 == tmnow->tm_mday)          /* 1st */
                      strcpy(temp,                    "April Fool's Day");
                    break;

                case 5:                     /** May **/
                    if (!(tmnow->tm_wday) &&          /* Sunday */
                        2 == k)                       /* 2nd Sunday */
                      strcpy(temp,                    "Mother's Day");
                    else
                    if (6 == tmnow->tm_wday &&        /* Saturday */
                        3 == k)                       /* 3rd Saturday */
                      strcpy(temp,                    "Armed Forces Day");
                    else
                    if (30 == tmnow->tm_mday)         /* 30th */
                      strcpy(temp,                    "Memorial Day");
                    break;

                case 6:                     /** June **/
                    if (14 == tmnow->tm_mday)         /* 14th */
                      strcpy(temp,                    "Flag Day");
                    else
                    if (!(tmnow->tm_wday) &&          /* Sunday */
                        3 == k)                       /* 3rd Sunday */
                      strcpy(temp,                    "Father's Day");
                    break;

                case 7:                     /** July **/
                    if (4 == tmnow->tm_mday)          /* 4th */
                      strcpy(temp,                    "Independence Day");
                    break;

                case 9:                     /** September **/
                    if (1 == tmnow->tm_wday &&        /* Monday */
                        1 == k)                       /* 1st Monday */
                      strcpy(temp,                    "Labor Day");
                    break;

                case 10:                    /** October **/
                    if (12 == tmnow->tm_mday)         /* 12th */
                      strcpy(temp,                    "Columbus Day");
                    else
                    if (31 == tmnow->tm_mday)         /* 31st */
                      strcpy(temp,                    "Halloween");

                case 11:                    /** November **/
                    if (2 == tmnow->tm_wday &&        /* Tuesday */
                        1 == k)                       /* 1st Tuesday */
                      strcpy(temp,                    "Election Day");
                    else
                    if (11 == tmnow->tm_mday)         /* 11th */
                      strcpy(temp,                    "Veteran's Day");
                    else
                    if (4 == tmnow->tm_wday &&        /* Thursday */
                        4 == k)                       /* 4th Thursday */
                      strcpy(temp,                    "Thanksgiving Day");
                    break;

                case 12:                    /** December **/
                    if (24 == tmnow->tm_mday)         /* 24th */
                      strcpy(temp,                    "Christmas Eve");
                    else
                    if (25 == tmnow->tm_mday)         /* 25th */
                      strcpy(temp,                    "Christmas Day");
                    else
                    if (31 == tmnow->tm_mday)         /* 31st */
                      strcpy(temp,                    "New Years Eve");
                    break;
                }
                if (*temp)
                    sprintf(temp+strlen(temp), " '%02d", tmnow->tm_year);
#endif
                else
                    sprintf(temp, "%02d%s%02d", tmnow->tm_year,
                                                monthTab[tmnow->tm_mon],
                                                tmnow->tm_mday);
                    break;
            case 'X': /*  %X  standard time string */
                    sprintf(temp, "%02d:%02d:%02d", tmnow->tm_hour,
                                                    tmnow->tm_min,
                                                    tmnow->tm_sec);
                    break;
            case 'y': /*  %y  year in decimal without century (00-99) */
#if 1            
                    sprintf(temp, "%02d", tmnow->tm_year % 100);
#else
                    sprintf(temp, "%02d", tmnow->tm_year);
#endif
                    break;
            case 'Y': /*  %Y  year including century as decimal */
#if 1                    
                    sprintf(temp, "%d", 1900+tmnow->tm_year);
#else

                    if(tmnow->tm_year > 99)
                    {
                        tmnow->tm_year -= 100;
                        sprintf(temp, "20%02d", tmnow->tm_year);
                    }
                    else
                        sprintf(temp, "19%02d", tmnow->tm_year);
#endif
                    break;
            case 'Z': /*  %Z  timezone name */
                    tzset();
                    sprintf(temp, "%s", tzname[0]);
                    break;
            case '%': /*  %%  the percent sign */
                    sprintf(temp, "%%");
                    break;
            default:
                    temp[0] = '\0';
                    break;
            }  /* end of switch */

        }  /* end of if */

        if( (int)(strlen(temp) + strlen(outstr)) > maxsize)
            break;
        else
            if(strlen(temp))
                strcat(outstr, temp);

    } /* end of for loop */
}
