/* -------------------------------------------------------------------- */
/*  LOGOUT.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                      Overlayed logout log code                       */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  terminate()     is menu-level routine to exit system                */
/*  setalloldrooms()    set all rooms to be old.                        */
/*  initroomgen()   initializes room gen# with log gen                  */
/*  setdefaultconfig()  this sets the global configuration variables    */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  terminate()     is menu-level routine to exit system                */
/* -------------------------------------------------------------------- */
void terminate(char discon)
{
    float balance;
    char  doStore;
    int   traptype;

    backout = FALSE;
    
    chatReq = FALSE;
    
    doStore = (BOOL)(CARRIER);

    if (discon || !doStore)
    {
        sysopNew = FALSE;
    }
      
    balance = logBuf.credits;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);


    if (doStore && verbose == 2)
    {
        doCR();
        mPrintf(" You were caller %s", ltoac(cfg.callno));
        doCR();                               
        mPrintf(" You were logged in for: %s", diffstamp(logtimestamp)); 
        doCR();
        mPrintf(" You entered %d %s", entered, cfg.msgs_nym);
        doCR();
        mPrintf(" and read %d.", mread);
        doCR();
        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT)
        {
            mPrintf(" %.0f %s used this is call",startbalance - logBuf.credits,
              ( (int)(startbalance - logBuf.credits) == 1)?"minute":"minutes" );
            doCR();
            mPrintf(" Your balance is %.0f %s", logBuf.credits,
                 ( (int)logBuf.credits == 1 ) ? "minute" : "minutes" );
            doCR();
        }
    }

    if (doStore && verbose) goodbye();

    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    if (loggedIn) mPrintf(" %s logged out\n ", logBuf.lbname);

    /* if (slv_door)  ExitToMsdos = TRUE; */

    thisHall = 0;    /* go to ROOT hallway */

    if (discon) 
    {
        if (gotCarrier())
        {
            Hangup();
        }
         whichIO = MODEM; /* I really don't know */
         setio(whichIO, echo, outFlag);

        /* onConsole = FALSE; */

         /* whichIO = CONSOLE; */  /* DUNNO */
       /*  onConsole = TRUE; */   /* DUNNO */

#ifdef GOODBYE
        if (!slv_slave)
        {
            Initport();
        }
#endif
    }
    if (!slv_door && !gotCarrier())
    {
        Initport();
    }

    if  (!doStore)  /* if carrier dropped */
    {
        /* trap it */
        sprintf(msgBuf->mbtext, "Carrier dropped");
        trap(msgBuf->mbtext, T_CARRIER);
    }   

    /* update new pointer only if carrier not dropped */
    if (loggedIn && doStore)
    {
        logBuf.lbroom[thisRoom].lbgen    = roomBuf.rbgen;
        /* logBuf.lbroom[thisRoom].lvisit   = 0; */
        logBuf.newpointer[thisRoom] = cfg.newest;


        /* logBuf.lbroom[thisRoom].mail  = 0; */
        talleyBuf->room[thisRoom].hasmail = 0;
    }

    if (loggedIn)
    {
        logBuf.callno      = cfg.callno;
        logBuf.calltime    = logtimestamp;
        /* logBuf.lbvisit[0]  = cfg.newest; */
        /* for the Minibin() function to calculate #new messages */
        logBuf.lastpointer = cfg.newest;

        logTab[0].ltcallno = cfg.callno;

        storeLog();
        loggedIn = FALSE;

        /* trap it */
        if (!logBuf.lbflags.NODE) 
        {
            sprintf(msgBuf->mbtext, "Logout %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_LOGIN);
        }
        else
        {
            sprintf(msgBuf->mbtext, "NetLogout %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_NETWORK);
        }

        if (!logBuf.lbflags.NODE)
            traptype = T_ACCOUNT;
        else
            traptype = T_NETWORK;

        sprintf(msgBuf->mbtext, "  ----- %4d messages entered", entered);
        trap(msgBuf->mbtext, traptype);

        sprintf(msgBuf->mbtext, "  ----- %4d messages read",  mread);
        trap(msgBuf->mbtext, traptype);

        if (logBuf.lbflags.NODE)
        {
           sprintf(msgBuf->mbtext, "  ----- %4d messages expired",  expired);
           trap(msgBuf->mbtext, T_NETWORK);

           sprintf(msgBuf->mbtext, "  ----- %4d messages duplicate",  duplicate);
           trap(msgBuf->mbtext, T_NETWORK);
        }    

        sprintf(msgBuf->mbtext, "Cost was %ld", (long)startbalance - (long)balance);
        trap(msgBuf->mbtext, T_ACCOUNT);

        if (cfg.accounting)  unlogthisAccount();
        heldMessage = FALSE;
        cleargroupgen();
        initroomgen();

        logBuf.lbname[0] = 0;

        setalloldrooms();
    }

    /* setTerm(""); */
    setdefaultTerm(0); /* TTY */

    
    update25();

    setdefaultconfig(); /* erases logbuf, rethink this shit */
    if (cfg.accounting)  unlogthisAccount();
    initroomgen();

    roomtalley();
    getRoom(LOBBY);

#ifdef GOODBYE
    if (!logBuf.lbflags.NODE)
        traptype = T_ACCOUNT;
    else
        traptype = T_NETWORK;
#endif

    clearmsglist();
    auto_mark = 0;
    auto_kill = 0;
    markedMId   = 0L;
    originalId  = 0L;

#ifdef GOODBYE
    sprintf(msgBuf->mbtext, "  ----- %4d messages entered", entered);
    trap(msgBuf->mbtext, traptype);

    sprintf(msgBuf->mbtext, "  ----- %4d messages read",  mread);
    trap(msgBuf->mbtext, traptype);

    if (logBuf.lbflags.NODE)
    {
       sprintf(msgBuf->mbtext, "  ----- %4d messages expired",  expired);
       trap(msgBuf->mbtext, T_NETWORK);

       sprintf(msgBuf->mbtext, "  ----- %4d messages duplicate",  duplicate);
       trap(msgBuf->mbtext, T_NETWORK);
    }    

    sprintf(msgBuf->mbtext, "Cost was %ld", (long)startbalance - (long)balance);
    trap(msgBuf->mbtext, T_ACCOUNT);
#endif
}


/* -------------------------------------------------------------------- */
/*  setalloldrooms()    set all rooms to be old.                        */
/* -------------------------------------------------------------------- */
void setalloldrooms(void)
{
    int i;

    for (i = 0; i < MAXROOMS; i++)
        logBuf.newpointer[i] = cfg.newest;

#ifdef GOODBYE
    logBuf.lbvisit[0] = cfg.newest;
#endif
}

/* -------------------------------------------------------------------- */
/*  initroomgen()   initializes room gen# with log gen                  */
/* -------------------------------------------------------------------- */
void initroomgen(void)
{
    int i;

    for (i = 0; i < MAXROOMS;  i++)
    {
        /* Clear mail and xclude flags in logbuff for every room */

        /* logBuf.lbroom[i].mail    = FALSE;  */
        talleyBuf->room[i].hasmail = 0;

        logBuf.lbroom[i].xclude  = FALSE;

        if (roomTab[i].rtflags.PUBLIC == 1)
        {
            /* make public rooms known: */
            logBuf.lbroom[i].lbgen  = roomTab[i].rtgen;
            /* logBuf.lbroom[i].lvisit = MAXVISIT - 1; */
              logBuf.newpointer[i] = cfg.oldest;


        } else
        {
            /* make private rooms unknown: */
            logBuf.lbroom[i].lbgen =
                (uchar)((roomTab[i].rtgen + (MAXGEN-1)) % MAXGEN);

           /* logBuf.lbroom[i].lvisit = MAXVISIT - 1; */
             logBuf.newpointer[i] = cfg.oldest;

        }
    }
}

/* -------------------------------------------------------------------- */
/*  setdefaultconfig()  this sets the global configuration variables    */
/* -------------------------------------------------------------------- */
void setdefaultconfig(void)
{
    prevChar    = ' ';
    termWidth   = cfg.width;
    termLF      = (BOOL)cfg.linefeeds;
    termUpper   = (BOOL)cfg.uppercase;
    termNulls   = cfg.nulls;
    expert      = FALSE;

/*    aide        = FALSE; */
/*    sysop       = FALSE; */

    aide = cfg.user[D_AIDE];  
    sysop = cfg.user[D_SYSOP];

    twit        = cfg.user[D_PROBLEM];
    unlisted    = FALSE;
    termTab     = (BOOL)cfg.tabs;
    oldToo      = cfg.readOld;   /* later a cfg.lastold */
    roomtell    = FALSE;
    
    memset(&logBuf, 0, sizeof(logBuf));

    /* peter commented that out */
    logBuf.linesScreen = cfg.linesScreen; 

    /* put user into group NULL */
    logBuf.groups[0] = grpBuf.group[0].groupgen;
    
    /* strcpy(logBuf.tty, "TTY"); */
    setdefaultTerm(0); /* TTY */
}


