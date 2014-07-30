/************************************************************************/
/*                              account.c                               */
/*        time accounting code for Citadel bulletin board system        */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      clearthisAccount()      initializes all current user group data */
/*      logincrement()          increments balance according to data    */
/*      logincheck()            logs-out a user if can't log in now     */
/*      negotiate()             determines lowest cost, highest time    */
/*      newaccount()            sets up accounting balance for new user */
/*      unlogthisAccount()      NULL group accounting to thisAccount    */
/*      updatebalance()         updates user's accounting balance       */
/*      calc_trans()            adjust time after a transfer            */
/*                                                                      */
/************************************************************************/
static ulong last_received;             /* #received last update        */
static ulong last_transmitted;          /* #received last update        */

/************************************************************************/
/*      clearthisAccount()  initializes all current user group data     */
/************************************************************************/
void clearthisAccount()
{
    int i;

    /* init days */
    for ( i = 0; i < 7; i++ )
        thisAccount.days[i] = 0;

    /* init hours & special hours */
    for ( i = 0; i < 24; i++ )
   {
        thisAccount.hours[i]   = 0;
        thisAccount.special[i] = 0;
    }

    thisAccount.have_acc      = FALSE;
    thisAccount.dayinc       = 0.f;
    thisAccount.sp_dayinc    = 0.f;
    thisAccount.maxbal       = 0.f;
    thisAccount.dlmult       = -1.f; /* charge full time */
    thisAccount.ulmult       = 1.f;  /* credit full time (xtra)  */
}

/************************************************************************/
/*      logincrement()          increments balance according to data    */
/*                              give em' more minutes if new day!       */
/************************************************************************/
void logincrement(void)
{
    long diff, timestamp;
    long day = 86400l;
    float numdays, numcredits;

    time(&timestamp);

    diff = timestamp - logBuf.calltime;
                           /* how many days since last call(1st of day)*/
    numdays = (float)( (float)diff / (float)day);

    if (numdays < 0)        /* date was wrong..             */
    {
        numdays = 1;        /* give em something for sysops mistake..   */
        time(&lasttime);
        logBuf.calltime    = lasttime;
    }

    numcredits = numdays * thisAccount.dayinc;

    /* If they have negative minutes this will bring them up MAYBE  */
    logBuf.credits = logBuf.credits + numcredits;

    if (logBuf.credits > thisAccount.maxbal)
        logBuf.credits = thisAccount.maxbal;

    /* Credits/Minutes to start with.   */
    startbalance = logBuf.credits;

    last_received    = 0l;
    last_transmitted = 0l;
    time(&lasttime);        /* Now, this is the last time we were on.   */
}
 

/************************************************************************/
/*      logincheck()            logs-out a user if can't log in now     */
/************************************************************************/
logincheck()
{
    specialTime = thisAccount.special[ hour() ];

    /* Local and bad calls get no accounting.   */
    if (!gotCarrier() || onConsole) return(TRUE);

    if (  !thisAccount.days[ dayofweek() ]
    ||    !thisAccount.hours[ hour() ]
    ||     logBuf.credits <= 0 )
    {
        tutorial("nologin.blb");

        verbose = FALSE;
        terminate(TRUE);

        return (FALSE);
    }

    if ( thisAccount.special[ hour() ] )  /* Is is free time?     */
    {
        specialTime = TRUE;
    }

    return (TRUE);
}

/************************************************************************/
/*      negotiate()  determines lowest cost, highest time for user      */
/************************************************************************/
void negotiate(void)
{
    int   groupslot, i;
    int   firstime = TRUE;
    float priority = 0;

    clearthisAccount();

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++)
    {
        if (ingroup(groupslot)
            && accountBuf.group[groupslot].account.have_acc)
        {
            /* is in a group with accounting */
            thisAccount.have_acc = TRUE;

            if (accountBuf.group[groupslot].account.priority >= priority)
            {

                if (accountBuf.group[groupslot].account.priority > priority)
                {                       /********************************/
                    clearthisAccount(); /* if it's more imp, start over */
                    thisAccount.have_acc = TRUE;/*otherwise, compromise!*/
                    firstime = TRUE;                
                }                               /************************/

                priority = accountBuf.group[groupslot].account.priority;

                for ( i = 0; i < 7; ++i)
                    if (accountBuf.group[groupslot].account.days[i])
                        thisAccount.days[i] = 1;

                for ( i = 0; i < 24; ++i)
                {
                    if (accountBuf.group[groupslot].account.hours[i])
                        thisAccount.hours[i]   = 1;

                    if (accountBuf.group[groupslot].account.special[i])
                        thisAccount.special[i] = 1;
                }

                if  (accountBuf.group[groupslot].account.dayinc >
                        thisAccount.dayinc  || firstime)
                    thisAccount.dayinc
                        = accountBuf.group[groupslot].account.dayinc;

                if  (accountBuf.group[groupslot].account.sp_dayinc >
                    thisAccount.sp_dayinc  || firstime)
                    thisAccount.sp_dayinc =
                    accountBuf.group[groupslot].account.sp_dayinc;

                if  (accountBuf.group[groupslot].account.maxbal >
                    thisAccount.maxbal)
                    thisAccount.maxbal
                        = accountBuf.group[groupslot].account.maxbal;

    /* these are */ thisAccount.dlmult =
    /* special   */ accountBuf.group[groupslot].account.dlmult;
    /*           */
    /*           */ thisAccount.ulmult =
    /*-----------*/ accountBuf.group[groupslot].account.ulmult;

                firstime = FALSE;
            }  /*  if  */
        }  /*  if  */
    }  /*  for  */
}

/************************************************************************/
/*      newaccount()  sets up accounting balance for new user           */
/*      extra set-up stuff for new user                                 */
/************************************************************************/
void newaccount(void)
{

    logBuf.credits   = cfg.newbal;
    last_received    = 0l;
    last_transmitted = 0l;
    time(&lasttime);
    logBuf.calltime  = lasttime;
    
}

/************************************************************************/
/*      unlogthisAccount()  sets up NULL group accounting to thisAccount*/
/************************************************************************/
void unlogthisAccount(void)
{
    int i;

    /* set up unlogged balance */
    logBuf.credits = cfg.unlogbal;

    /* reset transmitted & received */
    transmitted = 0l;
    received    = 0l;

    last_received    = 0l;
    last_transmitted = 0l;
    time(&lasttime);
    logBuf.calltime  = lasttime;

    /* init days */
    for ( i = 0; i < 7; i++ )
        thisAccount.days[i]    =  accountBuf.group[0].account.days[i];

    /* init hours & special hours */
    for ( i = 0; i < 24; i++ )
    {
        thisAccount.hours[i]   =  accountBuf.group[0].account.hours[i];
        thisAccount.special[i] =  accountBuf.group[0].account.special[i];
    }

    thisAccount.dayinc        =  accountBuf.group[0].account.dayinc;
    thisAccount.sp_dayinc     =  accountBuf.group[0].account.sp_dayinc;
    thisAccount.maxbal        =  accountBuf.group[0].account.maxbal;
    thisAccount.dlmult        =  accountBuf.group[0].account.dlmult;
    thisAccount.ulmult        =  accountBuf.group[0].account.ulmult;

}

/************************************************************************/
/*      updatebalance()  updates user's accounting balance              */
/*      This routine will warn the user of excessive use, and terminate */
/*      user when warnings have run out                                 */
/************************************************************************/
void updatebalance(void)
{
    float drain;
    long timestamp, diff;

    if ( thisAccount.special[ hour() ] && !specialTime)
    {
        specialTime = TRUE;
        doCR();

        if (loggedIn)
        {
            mPrintf("Special time is now beginning, you have no time limit.");
            doCR();
        }
    }

    if (specialTime &&                  /* if it's no longer special    */
       !thisAccount.special[ hour() ] ) /* time....                     */
    {
        doCR();

        if (loggedIn)
        {
            mPrintf("Special time is over. You have %.0f %s left today.",
                    logBuf.credits, (logBuf.credits == 1)?"minute":"minutes" );
            doCR();
        }
        specialTime = FALSE;

        time(&lasttime);
    }

    if (specialTime)    /* don't charge them for FREE time!             */
        return;


    /* get current time stamp */
    time(&timestamp);

    diff = timestamp - lasttime;

    /* If the time was set wrong..... */
    if (diff < 0)
      diff = 0;
    
    drain = (float)((float)diff / (float)60);

    logBuf.credits = logBuf.credits - drain;

    time(&lasttime);

    if (!gotCarrier() || onConsole) return;

    if (logBuf.credits < (float)5)
    {
        doCR();
        mPrintf(" Only %.0f %s left today!", logBuf.credits,
            (logBuf.credits == 1)?"minute":"minutes" );
        doCR();
    }

    if (  !thisAccount.days[ dayofweek() ]   /* if times up of it's no  */
    ||    !thisAccount.hours[ hour()     ]   /* login time for them..   */
    ||     logBuf.credits <= (float)0    )
    {
        tutorial("goodbye.blb");

        verbose = TRUE;
        terminate(TRUE);
    }
}

/************************************************************************/
/*      calc_trans()        Calculate and adjust time after a transfer  */
/*                          (trans == 1) ? Upload : Download            */
/************************************************************************/
void calc_trans(long time1, long time2, char trans)
{
    long    diff;                       /* # secs trans took            */
    float   credcost;                   /* their final credit/cost      */
    float   c;                          /* # minutes transfer took      */
    float   change;                     /* how much we change their bal */
    float   mult;                       /* the multiplyer for trans     */
    char    neg;                        /* is credcost negative?        */

    if (trans) mult = thisAccount.ulmult;
        else mult = thisAccount.dlmult;

    diff = time2 - time1;  /* how many secs did ul take                 */

    c = (float)diff / (float)60;       /* how many minutes did it take  */

    change = c * mult;                 /* take care of multiplyer...    */

    logBuf.credits = logBuf.credits + change;

    credcost = change;

    if ( credcost < 0 )
    {
        neg = TRUE;
    } else {
        credcost = credcost + c;    /* Make less confusion, add time used */
    }                               /* when telling them about credit     */

    doCR();
    mPrintf("Your transfer took %.2f %s. You have been %s %.2f %s.",
        c,
        (c   == 1)      ?  "minute"    : "minutes",
        (neg == 1)      ?  "charged"   : "credited",
        (neg == 1)      ? (-credcost)  :  credcost,
        (credcost == 1) ?  "minute"    : "minutes"
        );
    doCR();

    time(&lasttime);    /* don't charge twice...    */
}

