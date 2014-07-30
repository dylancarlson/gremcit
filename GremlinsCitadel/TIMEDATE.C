/* -------------------------------------------------------------------- */
/*  TIMEDATE.C               Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  This file contains functions relating to the time and date          */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  changedate()    Menu-level change date rutine                       */
/*  dayofweek()     returns current day of week 0 - 6  sun = 0          */
/*  diffstamp()     display the diffrence from timestamp to present     */
/*  getdstamp()     get datestamp (text format)                         */
/*  gettstamp()     get timestamp (text format)                         */
/*  hour()          returns hour of day  0 - 23                         */
/*  set_date()       Gets the date from the user and sets it             */
/*  pause()         busy-waits N/100 seconds                            */
/*  systimeout()    returns 1 if time has been exceeded                 */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  05/26/89    (PAT)   Created from MISC.C to break that moduel into   */
/*                      more managable and logical peices. Also draws   */
/*                      off MODEM.C                                     */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */
extern char *monthTab[12];

/* -------------------------------------------------------------------- */
/*  Static data/functions                                               */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  changedate()    Menu-level change date rutine                       */
/* -------------------------------------------------------------------- */
void changeDate(void)
{
    char dtstr[20];

    doCR();

    strftime(dtstr, 19, "%x", 0l);
    mPrintf(" Current date is now: %s", dtstr);

    doCR();

    strftime(dtstr, 19, "%X", 0l);
    mPrintf(" Current time is now: %s", dtstr);

    doCR();

    if(!getYesNo(" Enter a new date & time", 0)) return;

    set_date();
}

/* -------------------------------------------------------------------- */
/*  dayofweek()     returns current day of week 0 - 6  sun = 0          */
/* -------------------------------------------------------------------- */
int dayofweek(void)
{
    long stamp;
    struct tm *timestruct;

    time(&stamp);

    timestruct = localtime(&stamp);

    return(timestruct->tm_wday);
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  diffstamp()     display the diffrence from timestamp to present     */
/* -------------------------------------------------------------------- */
void diffstamp(long oldtime)
{
    long currentime, diff;
    char tstr[10], dstr[10];
    int days;

    time(&currentime);

    diff = currentime - oldtime;

    diff = diff + 315561600L;
    
    strftime(dstr, 9, "%j", diff);
    strftime(tstr, 9, "%X", diff);

    /* convert it to integer */
    days = atoi(dstr) - 1;

    if (days)
        mPrintf(" %d %s ",  days, days == 1 ? "day" : "days");

    mPrintf(" %s", tstr);
}
#endif


/* -------------------------------------------------------------------- */
/*  diffstamp()     display the diffrence from timestamp to present     */
/* -------------------------------------------------------------------- */
char *diffstamp(long oldtime)
{
    long currentime, diff;
    char tstr[10];
    static char dstr[20];
    int days;

    time(&currentime);

    diff = currentime - oldtime + 315561600L;
    
    strftime(dstr, 9, "%j", diff);
    strftime(tstr, 9, "%X", diff);

    /* convert it to integer */
    days = atoi(dstr) - 1;
    if (days)
    {
        sprintf(dstr, "%d day%s ",  days, (days == 1) ? "" : "s");
        strcat(dstr, tstr);
    }
    else
        strcpy(dstr, tstr);
    
    return(dstr);
}

/* -------------------------------------------------------------------- */
/*  getdstamp()     get datestamp (text format)                         */
/* -------------------------------------------------------------------- */
void getdstamp(char *buffer, unsigned stamp)
{
    int  day, year, mth;

    char month[4];

    year = (( stamp >> 9) & 0x7f) + 80;
    mth  = ( stamp >> 5) & 0x0f;
    day = stamp & 0x1f;

    if(mth < 1 || mth > 12 || day < 1 || day > 31)
    {
        strcpy(buffer, "       ");
        return;
    }

    strcpy(month, monthTab[mth-1]);

    sprintf(buffer, "%d%s%02d", year, month, day);
}

/* -------------------------------------------------------------------- */
/*  gettstamp()     get timestamp (text format)                         */
/* -------------------------------------------------------------------- */
void gettstamp(char *buffer, unsigned stamp)
{
    int hours, minutes, seconds;

    hours   =  (stamp >> 11) & 0x1f;
    minutes  = (stamp >> 5)  & 0x3f;
    seconds  = (stamp & 0x1f) * 2;

    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
}

/* -------------------------------------------------------------------- */
/*  hour()          returns hour of day  0 - 23                         */
/* -------------------------------------------------------------------- */
int hour(void)
{
    long stamp;
    struct tm *timestruct;

    time(&stamp);

    timestruct = localtime(&stamp);

    return(timestruct->tm_hour);
}

/* -------------------------------------------------------------------- */
/*  set_date()       Gets the date from the user and sets it             */
/* -------------------------------------------------------------------- */
void set_date(void)
{
    union REGS inregs, outregs;
    int yr, mth, dy, hr, mn, sc;
    char dtstr[20];
    struct tm *timestruct;
    long stamp;

    time(&stamp);

    timestruct = localtime(&stamp);

    yr  = timestruct->tm_year;
    mth = timestruct->tm_mon + 1;
    dy  = timestruct->tm_mday;
    hr  = timestruct->tm_hour;
    mn  = timestruct->tm_min;
    sc  = timestruct->tm_sec;

    inregs.x.cx  = (uint) getNumber("Year",  85l, 99l,(long)yr ) + 1900;
    inregs.h.dh  = (uchar) getNumber("Month", 1l,  12l,(long)mth);
    inregs.h.dl  = (uchar) getNumber("Day",   1l,  31l,(long)dy);
    inregs.h.ah = 0x2b;
    intdos(&inregs, &outregs);

    inregs.h.ch  = (uchar) getNumber("Hour",   0l, 23l,(long)hr);
    inregs.h.cl  = (uchar) getNumber("Minute", 0l, 59l,(long)mn);
    inregs.h.dh  = (uchar) getNumber("Second", 0l, 59l,(long)sc);
    inregs.h.ah = 0x2d;
    intdos(&inregs, &outregs);

    doCR();

    strftime(dtstr, 19, "%x", 0l);
    mPrintf(" Current date is now: %s", dtstr);

    doCR();

    strftime(dtstr, 19, "%X", 0l);
    mPrintf(" Current time is now: %s", dtstr);

    doCR();
}

/* -------------------------------------------------------------------- */
/*  pause()         busy-waits N/100 seconds                            */
/* -------------------------------------------------------------------- */
void pause(register int ptime)
{
    union REGS in, out;
    register int i, j=0;
    in.h.ah=0x2C;
    intdos(&in, &out);
    i = out.h.dl;
    while(j < ptime)
    {
        in.h.ah=0x2C;
        intdos(&in, &out);
        if(out.h.dl < (uchar)i)
            j += (100+out.h.dl)-i;
        else
            j += out.h.dl-i;
        i = out.h.dl;
    }
}

/* -------------------------------------------------------------------- */
/*  netpause()         busy-waits N/100 seconds                         */
/* -------------------------------------------------------------------- */
void netpause(register int ptime)
{
    union REGS in, out;
    register int i, j=0;
    int ch;
    in.h.ah=0x2C;
    intdos(&in, &out);
    i = out.h.dl;
    while(j < ptime)
    {
        in.h.ah=0x2C;
        intdos(&in, &out);
        if(out.h.dl < (uchar)i)
            j += (100+out.h.dl)-i;
        else
            j += out.h.dl-i;
        i = out.h.dl;

        if (MIReady())
        {
            ch = (char)getMod();
            if (debug) outCon(ch);
        }
    }
}

/* -------------------------------------------------------------------- */
/*  twirlypause()         busy-waits N/100 seconds                      */
/* -------------------------------------------------------------------- */
void twirlypause(register int ptime)
{
    union REGS in, out;
    register int i, j=0;
    in.h.ah=0x2C;
    intdos(&in, &out);
    i = out.h.dl;
    while(j < ptime && !KBReady() && !(MIReady() && gotCarrier() && modStat
    && (whichIO == MODEM)))
    {
        in.h.ah=0x2C;
        intdos(&in, &out);
        if(out.h.dl < (uchar)i)
            j += (100+out.h.dl)-i;
        else
            j += out.h.dl-i;
        i = out.h.dl;
    }
}

/* -------------------------------------------------------------------- */
/*  systimeout()    returns 1 if time has been exceeded                 */
/* -------------------------------------------------------------------- */
int systimeout(time_t timer)
{
    time_t currentime;

    time(&currentime);

    if  ( 
          (
            (loggedIn && (((currentime - timer) / 60 ) >= cfg.timeout))
          ||(!loggedIn && (((currentime - timer) / 60 ) >= cfg.unlogtimeout))
          )
        )
        return(TRUE);

    return(FALSE);
}


