/* -------------------------------------------------------------------- */
/*  LOGIN.C                  Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                       Overlayed login log code                       */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  login()         is the menu-level routine to log someone in         */
/*  loginNew()      login a new user, handel diffrent configs..         */
/*  setgroupgen()   sets unmatching group generation #'s                */
/*  setroomgen()    sets room gen# with log gen                         */
/*  setlbvisit()    sets lbvisit at log-in                              */
/*  slideLTab()     crunches up slot, then frees slot at beginning,     */
/*                  it then copies information to first slot            */
/*  minibin()       minibin log-in stats                                */
/*  pwslot()        returns logtab slot password is in, else ERROR      */
/*  pwexists()      returns TRUE if password exists in logtable         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  login()         is the menu-level routine to log someone in         */
/* -------------------------------------------------------------------- */
void doLogin(char moreYet)
{
    int foundIt;
    char InitPw[NAMESIZE+NAMESIZE+2];
    char password[NAMESIZE+NAMESIZE+2];
    char initials[NAMESIZE+NAMESIZE+2];
    char *semicolon;

    Mflush();

    if (!CARRIER) return;

    if (login_user || login_pw) /* handle command line log-ins */
    {
        if (!modStat) 
            if (cfg.offhook)  offhook();

        /* login using initials and pw */
        if (login_pw)
        {
            normalizepw(cmd_login, initials, password);
            login_pw = FALSE;
        }
        else

        if (login_user)
        {
            normalizeString(cmd_login);
            if (findPerson(cmd_login, &logBuf) != ERROR)
            {
                strcpy(initials, logBuf.lbin);
                strcpy(password, logBuf.lbpw);
            }
            login_user = FALSE;
        }

    }
    else   /* ask user for initials and password */
    {


    if (moreYet == 2)
        moreYet = FALSE;
    else
    {
        /* dont print Login when hitting 'L' from console mode */
        if (!(!moreYet && !loggedIn && !modStat))
        {
            mPrintf("Login ");
        }
    }



    if (loggedIn)  
    {
        mPrintf("\n Already logged in!\n ");
        return;
    }

    if (!modStat) 
        if (cfg.offhook)  offhook();


  getNormStr((moreYet) ? "" : "your initials", InitPw, NAMESIZE+NAMESIZE+1, NO_ECHO);
    if (!CARRIER) return;

        dospCR();

        semicolon = strchr(InitPw, ';');

        if (!semicolon)
        {
            strcpy(initials, InitPw);
            getNormStr( "password",  password, NAMESIZE, NO_ECHO);
            dospCR();
        }     
        else  
        {
            normalizepw(InitPw, initials, password);
        }

        /* dont allow anything over 19 characters */
        initials[NAMESIZE] = '\0';
    }
    
    /* reset transmitted & received */
    transmitted = 0l;
    received    = 0l;

    /* reset read & entered */
    mread   = 0;
    entered = 0;

    foundIt = ((pwslot(initials, password)) != ERROR);

    if (foundIt && *password)
    {
        loggedIn    = TRUE;
        update25();

        /* trap it */
        if (!logBuf.lbflags.NODE) 
        {
            sprintf( msgBuf->mbtext, "Login %s", logBuf.lbname);
            if (onConsole)
                strcat(msgBuf->mbtext, " (Console)");

            trap(msgBuf->mbtext, T_LOGIN);
        }
        else
        {
            sprintf( msgBuf->mbtext, "NetLogin %s", logBuf.lbname);
            trap(msgBuf->mbtext, T_NETWORK);
        }
    }
    else
    {
        loginNew(initials, password);
    }

    if (!loggedIn)
        return;

    heldMessage = FALSE;

    setsysconfig();
    setgroupgen();
    setroomgen();
    setlbvisit();

    slideLTab(thisSlot);

    /* cant log in now. */
    if (cfg.accounting && !logBuf.lbflags.NOACCOUNT)
    {
        negotiate();
        logincrement();
        if (!logincheck()) 
        {
            Hangup();
            return;
        }
    }

    /* can't log in now. */
    if (logBuf.VERIFIED && !onConsole)
    {
        tutorial("verified.blb");
        Hangup();
        return;
    }

    if (logBuf.lbflags.NODE)
    {
#ifdef  TRASH       
        if (debug)
        {
            readnode();

            cPrintf("Node:  \"%s\" \"%s\"", node.ndname, node.ndregion);  doccr();
            cPrintf("Phone: \"%s\" %d", node.ndphone, node.nddialto);     doccr();
            cPrintf("Login: \"%s\" %d", node.ndlogin, node.ndwaitto);     doccr();
            cPrintf("Baud:  %d    Protocol: \"%s\"\n ", node.ndbaud, node.ndprotocol);
            cPrintf("Expire:%d    Waitout:  %d", node.ndexpire, node.ndwaitto); doccr();
            cPrintf("Network: %d  ZIP: %s UNZIP: %s", node.network, node.zip, node.unzip); doccr();
        }
#endif        
        
        time(&logtimestamp);
        return;
    }

    if (logBuf.PSYCHO)
    {
        backout = TRUE;
    }
    
    /* reverse engineering Minibin?!?! */
    if (logBuf.MINIBIN)
    {
        minibin();
    }
    
    changedir(cfg.helppath); 

    if ( filexists("bulletin.blb") )
    {
        tutorial("bulletin.blb");
    }
    
    gotodefaulthall();

    roomtalley();

    mf.mfLim = 0;   /* just to make sure. */
    mf.mfMai = 0;
    mf.mfPub = 0;
    mf.mfUser[0]=0;

    nochat(TRUE);       /* reset chats */
    
    /* verbose = FALSE; */
    verbose = logBuf.VERBOSE;

    /* hmmm... where to put this */
    if (roomBuf.rbflags.APLIC && roomBuf.rbflags.AUTOAPP )
        ExeAplic();

    showMessages(NEWoNLY, FALSE);

    verbose = FALSE;
    if (expert) listRooms(NEWRMS, FALSE);
    else        listRooms(OLDNEW, FALSE);

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);
    
    /* record login time, date */
    time(&logtimestamp);

    cfg.callno++;

    storeLog();
}

/* -------------------------------------------------------------------- */
/*  setgroupgen()   sets unmatching group generation #'s                */
/* -------------------------------------------------------------------- */
void setgroupgen(void)
{
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++)
    {
        if (logBuf.groups[groupslot] != grpBuf.group[groupslot].groupgen)
        {
            logBuf.groups[groupslot] =
                (uchar)((grpBuf.group[groupslot].groupgen
                + (MAXGROUPGEN - 1)) % MAXGROUPGEN);
        }
    }
}

/* -------------------------------------------------------------------- */
/*  setroomgen()    sets room gen# with log gen                         */
/* -------------------------------------------------------------------- */
void setroomgen(void)
{
    int i;

    /* set gen on all unknown rooms  --  INUSE or no: */
    for (i = 0;  i < MAXROOMS;  i++)
    {
        /* Clear mail and xclude flags in logbuff for any  */
        /* rooms created since last call                   */
    
        if (logBuf.lbroom[i].lbgen != roomTab[i].rtgen)
        {
            talleyBuf->room[i].hasmail = FALSE;
          /*logBuf.lbroom[i].mail    = FALSE;*/

            logBuf.lbroom[i].xclude  = FALSE;
        }

        /* if not a public room */
        if (roomTab[i].rtflags.PUBLIC == 0)
        {
            /* if you don't know about the room */
            if (((logBuf.lbroom[i].lbgen) != roomTab[i].rtgen) ||
               (!aide && i == AIDEROOM))
            {
                /* mismatch gen #'s properly */
                logBuf.lbroom[i].lbgen 
                    =  (uchar)((roomTab[i].rtgen + (MAXGEN-1)) % MAXGEN);

                /* logBuf.lbroom[i].lvisit =  MAXVISIT - 1; */
                 logBuf.newpointer[i] = cfg.oldest;


            }
        }
        else if ((logBuf.lbroom[i].lbgen) != roomTab[i].rtgen) 
        {
            /* newly created public room -- remember to visit it; */
            logBuf.lbroom[i].lbgen  = roomTab[i].rtgen;

            /* logBuf.lbroom[i].lvisit = 1; */
             logBuf.newpointer[i] = cfg.oldest; 
        }
    }
}

/* -------------------------------------------------------------------- */
/*  setlbvisit()    sets lbvisit at log-in                              */
/* -------------------------------------------------------------------- */
void setlbvisit(void)
{
    int i;

    /* see if the message base was cleared since last call */
    for (i = 0; i < MAXROOMS; i++)
    {
        if (logBuf.newpointer[i] > cfg.newest)
        {
            for (i = 0; i < MAXROOMS; i++)
                logBuf.newpointer[i] = cfg.oldest;

#ifdef GOODBYE
            logBuf.lbvisit[ 0            ]= cfg.newest;
            logBuf.lbvisit[ (MAXVISIT-1) ]= cfg.oldest;
#endif

            doCR();
            mPrintf("%s base destroyed since last call!", cfg.msg_nym); doCR();
            mPrintf("All %s pointers reset.", cfg.msg_nym); doCR();
            return;
        }
    }
    
    /* slide lbvisit array down and change lbgen entries to match: */

#ifdef GOODBYE
    for (i = (MAXVISIT - 2);  i;  i--)
    {
        logBuf.lbvisit[i] = logBuf.lbvisit[i-1];
    }
    logBuf.lbvisit[(MAXVISIT - 1)] = cfg.oldest;
    logBuf.lbvisit[0             ] = cfg.newest;
#endif

    for (i = 0;  i < MAXROOMS;  i++)
    {
#ifdef GOODBYE
        if ((logBuf.lbroom[i].lvisit)  <  (MAXVISIT-2))
        {
            logBuf.lbroom[i].lvisit++;
        }
#endif

        logBuf.lbroom[i].bypass = FALSE;
        
        MessageRoom[i] = 0;
    } 
}

/* -------------------------------------------------------------------- */
/*  slideLTab()     crunches up slot, then frees slot at beginning,     */
/*                  it then copies information to first slot            */
/* -------------------------------------------------------------------- */
void slideLTab(int slot)    /* slot is current tabslot being moved */
{
    int ourSlot, i;

    people = slot; /* number of people since last call */

    if (!slot) return;

    ourSlot = logTab[slot].ltlogSlot;

    /* Gee, this works.. */
    for (i=slot; i>0; i--)
        logTab[i] = logTab[i-1];

    thisSlot = 0;

    /* copy info to beginning of table */
    log2tab(&logTab[0], &logBuf);
    logTab[0].ltlogSlot = ourSlot;
}

/* -------------------------------------------------------------------- */
/*  minibin()       minibin log-in stats                                */
/* -------------------------------------------------------------------- */
void minibin(void)
{
    int calls, messages;
    char dtstr[80];
    char minutes[5];

/*    messages = (int)(cfg.newest - logBuf.lbvisit[1]); */
    messages = (int)(cfg.newest - logBuf.lastpointer);
    calls    = (int)(cfg.callno - logBuf.callno);

    /* special hack to kill mangled surnames beacuse of the
       3.10.05 to 3.11.00 conversion program... */
    if (!tfilter[logBuf.surname[0]])
    {
        logBuf.surname[0] = '\0' /*NULL*/;
    }
    
    if (!expert) mPrintf(" \n \n <3J0>ump <3N0>ext <3P0>ause <3S0>top");
    
    doCR();
    mPrintf("0Welcome back ");
    if (cfg.titles && logBuf.title[0] && logBuf.DISPLAYTS)
    {
        mPrintf("[%s] ", logBuf.title);
    }
    mPrintf("3%s0",logBuf.lbname);
    if (cfg.surnames && logBuf.surname[0] && logBuf.DISPLAYTS)
    {
        mPrintf(" [%s]", logBuf.surname);
    }    
    mPrintf("!");
    doCR();
    
    if (calls == 0)
    {
        mPrintf("You were just here.");
        doCR();
    }
    else
    if (calls == -1)
    {
        ;
    }
    else
    {
        strftime(dtstr, 79, (loggedIn) ? logBuf.vdstamp : cfg.vdatestamp, logBuf.calltime);
        mPrintf("You last called on: %s", dtstr);
        doCR();
        mPrintf("You are caller %s", ltoac(cfg.callno + 1l));
        doCR();
        mPrintf("%d %s made", people,
            (people == 1)?"person has":"people have");
        doCR();
        mPrintf("%d %s and left",calls, (calls == 1)?"call":"calls");
        doCR();
        mPrintf("%s new %s since you were last here.",ltoac((long)messages),
            (messages==1)? cfg.msg_nym: cfg.msgs_nym);
        doCR();
    }

    if (cfg.accounting && !logBuf.lbflags.NOACCOUNT)
    {
        if (!specialTime)
        {
            sprintf(minutes, "%.0f", logBuf.credits);

            mPrintf("You have %s minute%s left today.", minutes,
                (strcmp(minutes, "1") == SAMESTRING) ? "" : "s");
        }
        else 
        {
            mPrintf("You have unlimited time.");
        }

        doCR();
    }

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);
}

/* -------------------------------------------------------------------- */
/*  pwslot()        returns logtab slot password is in, else ERROR      */
/* -------------------------------------------------------------------- */
int pwslot(char *in, char *pw)
{
    int slot;

    if (strlen(pw) < 2)  return ERROR;  /* Don't search for these pwds */

    slot = pwexists(pw);

    if (slot == ERROR) return ERROR;

    /* initials must match too */
    if ( (logTab[slot].ltinhash) != (int)hash(in) ) return ERROR;

    getLog(&lBuf, logTab[slot].ltlogSlot);

    if (  (strcmpi(pw, lBuf.lbpw) == SAMESTRING)
    &&    (strcmpi(in, lBuf.lbin) == SAMESTRING) )
    {
        memcpy(&logBuf, &lBuf, sizeof logBuf);
        thisSlot = slot;
        thisLog  = logTab[slot].ltlogSlot;
        return(slot);
    }
    else
    {
        return ERROR;
    }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
int pwexists(char *pw)
{
    int i, pwhash;
    
    pwhash = hash(pw);

    for ( i = 0;  i < cfg.MAXLOGTAB;  i++)
    {
        if (pwhash == logTab[i].ltpwhash)
        return(i);
    }
    return(ERROR);
}



