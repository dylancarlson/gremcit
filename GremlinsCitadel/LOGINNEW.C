/* -------------------------------------------------------------------- */
/*  LOGINNEW.C               Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                  Overlayed new user login log code                   */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  loginNew()      login a new user, handel diffrent configs..         */
/*  newUser()       prompts for name and password                       */
/*  newUserFile()   Writes new user info out to a file                  */
/*  newlog()        sets up a new log entry for new users returns ERROR */
/*                  if cannot find a usable slot                        */
/*  newslot()       attempts to find a slot for a new user to reside in */
/*                  puts slot in global var  thisSlot                   */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  loginNew()      login a new user, handel diffrent configs..         */
/* -------------------------------------------------------------------- */
void loginNew(char *initials, char *password)
{
    int i; 
    ulong newpointer;
    
    if (getYesNo(cfg.l_verified ? " No record: Enter as new user" 
                                : " No record: Request access", 1))
    {
        if (!CARRIER) return;
        
        if (cfg.l_closedsys && (!sysopNew && !(onConsole && !debug)))
        {
            tutorial("closesys.blb");
            drop_dtr();
            return;
        }
        
        tutorial("userinfo.blb");
        
        if (cfg.l_create || sysopNew || (onConsole && !debug))
        {
            logBuf.VERIFIED = (onConsole && !debug) ? FALSE : !cfg.l_verified;
            newUser(initials, password);
            if (!loggedIn)
                return;
            newaccount();
            update25();
        }

        if (cfg.l_questionare && (!(onConsole && !debug)))
        {
            newUserFile();
        }

        if (cfg.l_application && (!(onConsole && !debug)))
        {
            if (changedir(cfg.aplpath) == ERROR)
            {
                mPrintf("  -- Can't find application directory.\n\n");
                changedir(cfg.homepath);
                return;
            }
            apsystem(cfg.newuserapp);
            changedir(cfg.homepath);
        }

        if (cfg.l_sysop_msg && (!(onConsole && !debug)))
        {
            tutorial("newmsg.blb");
            
            i = loggedIn;       /* force to sysop */
            loggedIn = FALSE; 
            mailFlag  = TRUE; 
            oldFlag   = FALSE;
            limitFlag = FALSE;
            linkMess  = FALSE;
            makeMessage();
            loggedIn = (uchar)i;
        }
        
        if (logBuf.VERIFIED && !sysopNew && loggedIn)
        {
            verbose = FALSE;
            terminate(TRUE);
            mPrintf("\n Thank you, Good Bye.\n");
            Hangup();
        }
    }



/**********************************************************************/
/* Icky Hack designed to make #oldcount functional                    */
/* this was moved here because for some reason making messages        */
/* was causing the pointers to get screwed up                         */
/**********************************************************************/

    if (cfg.oldcount)
    {
        newpointer = (cfg.newest - cfg.oldcount);
        if (newpointer < cfg.oldest)  newpointer = cfg.oldest;

        /* logBuf.lbvisit[0] = newpointer; */ /* pushed down later by setlbvisit() */

        for (i = 0; i < MAXROOMS;  i++)
        {
            logBuf.newpointer[i] = newpointer;

            /* logBuf.lbroom[i].lvisit = 0; */ /* becomes==1 later by setlbvisit() */
        }
    }

/**********************************************************************/
/* ^^^ Disgusting? Wasn't it?  ^^^ Hope it works!                     */
/**********************************************************************/
    
    sysopNew = FALSE;
    
    return;
}

/* -------------------------------------------------------------------- */
/*  newUser()       prompts for name and password                       */
/* -------------------------------------------------------------------- */
void newUser(char *initials, char *password)
{
    label fullnm;
    char InitPw[80];
    char Initials[80];
    char passWord[80];
    char *semicolon;
    int  lines;
    
    int abort, good = 0;
    char  firstime = 1;

    if (!CARRIER)  return;

    unlisted = FALSE;  /* default to [Y] for list in userlog for new users */
    roomtell = TRUE;   /* default to [Y] for display of room descriptions  */

    /* this only initializes log.buf, does not write it out */
    if (newlog() == ERROR) return;

    logBuf.linesScreen = cfg.linesScreen;
    /* askTerm(); */
    configure(TRUE);      /* make sure new users configure reasonably     */
    lines = logBuf.linesScreen;
    
    tutorial("password.blb");
    
    do
    {
        do
        {
            /* getNormStr("full name", fullnm, NAMESIZE, ECHO); */
            enterName(cfg.enter_name, fullnm, NULL);
            
            if ( (personexists(fullnm) != ERROR )
            ||   (strcmpi(fullnm, "Sysop") == SAMESTRING)
            ||   (strcmpi(fullnm, "Aide") == SAMESTRING)
            ||   !strlen(fullnm) )
            {
                mPrintf("We already have a %s\n", fullnm);
                good = FALSE;
            }        
            else (good = TRUE);
        }
        while(!good && CARRIER);

        if (!CARRIER)  return;

        if (firstime)
        {
            strcpy(Initials, initials);
        }
        else
        {
            getNormStr("your initials", InitPw, 40, NO_ECHO);
            dospCR();

            semicolon = strchr(InitPw, ';');

            if( semicolon )
            {
               normalizepw(InitPw, Initials, passWord);
            }
            else
            {
                strcpy(Initials, InitPw);
            }

            /* dont allow anything over 19 characters */
            Initials[19] = '\0';
        }

        do
        {
            if (firstime)
            {
                strcpy(passWord, password);
            }
            else if (!semicolon)
            {
                getNormStr("password",  passWord, NAMESIZE, NO_ECHO);
                dospCR();
            }
            
            firstime  = FALSE;  /* keeps from going in infinite loop */
            semicolon = FALSE;

            if ( pwexists(passWord) != ERROR || strlen(passWord) < 2)
            {
                good = FALSE;
                mPrintf("\n Poor password\n ");
            }
            else good = TRUE;
        }
        while( !good  && CARRIER );

        displaypw(fullnm, Initials, passWord);

        abort = getYesNo("OK",2);

        if (abort == 2) return;  /* check for Abort at (Y/N/A)[A]: */
    }
    while ( (!abort) && CARRIER);

    if (CARRIER)
    {
        /* Moved from newlog() */
        strcpy(logBuf.lbname, fullnm);
        strcpy(logBuf.lbin, Initials);
        strcpy(logBuf.lbpw, passWord);

        /* This stuff moved from newlog()
        time(&logBuf.calltime);

        setsysconfig();

        /*
         * trap it 
         */
        sprintf( msgBuf->mbtext, "New user %s", logBuf.lbname);
        if (onConsole)
            strcat(msgBuf->mbtext, " (Console)");
        trap(msgBuf->mbtext, T_LOGIN);

        loggedIn = TRUE;
        slideLTab(thisSlot);
        storeLog();

        /* End of stuff moved from newlog() */
    }

    logBuf.linesScreen = (uchar)lines;
}

/* -------------------------------------------------------------------- */
/*  newUserFile()   Writes new user info out to a file                  */
/* -------------------------------------------------------------------- */
void newUserFile(void)
{
    FILE           *fl;
    char      name[40];
    char     phone[30];
    label      surname;
    label        title;
    char      temp[85];
    char     dtstr[85];
/*  int    tempmaxtext; */ 
    int          clm=0;
    int            l=0;

    *name     ='\0';
    *phone    ='\0';
    *surname  ='\0';
    *title    ='\0';
    
    tutorial("newquest.blb");
    
    if (cfg.titles && cfg.entersur)
    {
        getNormStr("the title you desire",   title,  NAMESIZE, ECHO);
    }
    
    if (cfg.surnames && cfg.entersur)
    {
        getNormStr("the surname you desire", surname,    NAMESIZE, ECHO);
    }
    
    strcpy(logBuf.title,   title);
    strcpy(logBuf.surname, surname);

    strcpy(msgBuf->mbsur,   logBuf.surname);
    strcpy(msgBuf->mbtitle, logBuf.title);

    getNormStr("your full real name",        name,     40,       ECHO);

    if (name[0])
    {
        getNormStr("your phone number [(xxx)xxx-xxxx]", phone, 30, ECHO);
    }


    strcpy(msgBuf->mbto, "Sysop");
    strcpy(msgBuf->mbauth, logBuf.lbname);
    msgBuf->mbtext[0] = 0;

#ifdef GOODBYE
    tempmaxtext = cfg.maxtext;
    cfg.maxtext = 1024;
#endif

    getText();
    stripansi(msgBuf->mbtext);

#ifdef GOODBYE
    cfg.maxtext = tempmaxtext;
#endif
              
    if (changedir(cfg.homepath) == ERROR)  return;

    if ((fl = fopen("newuser.log", "at")) == NULL)
    {
        return;
    }
    strftime(dtstr, 79, (loggedIn) ? logBuf.vdstamp : cfg.vdatestamp, 0l);

    sprintf(temp, "\n %s\n", dtstr);
    fwrite(temp, strlen(temp), 1, fl);

    stripansi(title); 
    stripansi(surname);
    stripansi(name);
    stripansi(phone);


    if (surname[0] || title[0])
    {
        sprintf(temp, " Nym:       [%s] %s [%s]\n", 
                title, deansi(logBuf.lbname), surname);
    }
    else
    {
        sprintf(temp, " Nym:       %s\n",    deansi(logBuf.lbname) );
    }
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Real name: %s\n",             name );
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Phone:     %s\n",            phone );
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, " Baud:      %u\n",     bauds[speed] );
    fwrite(temp, strlen(temp), 1, fl);

    sprintf(temp, "\n");

    if(msgBuf->mbtext[0])   /* xPutStr(fl, msgBuf->mbtext); */
    {
        do
        {
            if((msgBuf->mbtext[l] == 32 || msgBuf->mbtext[l] == 9) && clm > 73)
            {
                fwrite(temp, strlen(temp), 1, fl);
                clm = 0;
                l++;
            }
            else
            {
                fputc(msgBuf->mbtext[l], fl);
                clm++;
                if(msgBuf->mbtext[l] == 10)
                    clm = 0;
                if(msgBuf->mbtext[l] == 9)
                    clm = clm + 7;
                l++;
            }
        } while(msgBuf->mbtext[l]);
    }

    fclose(fl);
    doCR();
}



/* -------------------------------------------------------------------- */
/*  newlog()        sets up a new log entry for new users returns ERROR */
/*                  if cannot find a usable slot                        */
/*  #oldcount implemented!                                              */
/* -------------------------------------------------------------------- */
int newlog(void)
{
    int  ourSlot, i, v;
 /*   ulong newpointer; */

    /* 
     * get a new slot for this user 
     */
    thisSlot = newslot();

    if (thisSlot == ERROR)
    {
        thisSlot = 0;
        return(ERROR);
    }

    ourSlot = logTab[thisSlot].ltlogSlot;

    /*
     * Fill in the account.
     */
    v = logBuf.VERIFIED;
    getLog(&logBuf, ourSlot);
    memset(&logBuf, 0, sizeof(logBuf));
    
    setlogconfig();

#ifdef GOODBYE  /* Moved to new user */
    strcpy(logBuf.lbname, fullnm);
    strcpy(logBuf.lbin, in);
    strcpy(logBuf.lbpw, pw);
#endif

    logBuf.surname[0] = '\0';     /* no starting surname */
    logBuf.title  [0] = '\0';     /* no starting title   */
    logBuf.forward[0] = '\0';     /* no starting forwarding */

    logBuf.lbflags.L_INUSE   = TRUE;
    logBuf.lbflags.PROBLEM   = cfg.user[D_PROBLEM];
    logBuf.lbflags.PERMANENT = cfg.user[D_PERMANENT];
    logBuf.lbflags.NOACCOUNT = cfg.user[D_NOACCOUNT];
    logBuf.lbflags.NETUSER   = cfg.user[D_NETWORK];
    logBuf.lbflags.NOMAIL    = cfg.user[D_NOMAIL];
    logBuf.lbflags.AIDE      = cfg.user[D_AIDE];
/*  aide = cfg.user[D_AIDE]; */ 
    logBuf.lbflags.SYSOP     = cfg.user[D_SYSOP];
/*  sysop = cfg.user[D_SYSOP];  */

    logBuf.BOARDERS          = cfg.user[D_BOARDER];
    logBuf.VERIFIED          = v;
    /* logBuf.IBMGRAPH          = FALSE; */
    logBuf.DISPLAYTS         = TRUE;
    logBuf.SUBJECTS          = TRUE;
    logBuf.SIGNATURES        = TRUE;
    logBuf.TWIRLY            = FALSE;
    logBuf.VERBOSE           = FALSE;
    logBuf.MSGPAUSE          = FALSE;
    logBuf.MSGCLS            = FALSE;

    /* should this be a cfg.user thing? */
    logBuf.MINIBIN           = TRUE;

    logBuf.ROOMINFO = TRUE;
    logBuf.HALLTELL = TRUE;

    /* elegia */
    strcpy(logBuf.prompt, cfg.prompt);
    strcpy(logBuf.dstamp, cfg.datestamp);
    strcpy(logBuf.vdstamp, cfg.vdatestamp);

    strcpy(logBuf.netPrefix, cfg.netPrefix);

    strcpy(logBuf.msg_header, cfg.msg_header);
    strcpy(logBuf.vmsg_header, cfg.vmsg_header);

#ifdef GOODBYE
    for (i = 1; i < MAXVISIT; i++)
    {
        logBuf.lbvisit[i] = cfg.oldest;
    }

    logBuf.lbvisit[ 0            ]= cfg.newest;
    logBuf.lbvisit[ (MAXVISIT-1) ]= cfg.oldest;
#endif

    for (i = 0; i < MAXROOMS; i++)
    {
        logBuf.newpointer[i] = cfg.oldest;
    }


    initroomgen();

/**********************************************************************/
/* Icky Hack designed to make #oldcount functional                    */
/**********************************************************************/
#ifdef GOODBYE
    if (cfg.oldcount)
    {
        newpointer = (cfg.newest - cfg.oldcount);
        if (newpointer < cfg.oldest)  newpointer = cfg.oldest;

        logBuf.lbvisit[0] = newpointer; /* pushed down later by setlbvisit() */

        for (i = 0; i < MAXROOMS;  i++)
        {
            logBuf.lbroom[i].lvisit = 0; /* becomes==1 later by setlbvisit() */
        }
    }
#endif

/**********************************************************************/
/* ^^^ Disgusting? Wasn't it?  ^^^ Hope it works!                     */
/**********************************************************************/

    cleargroupgen();

    /*
     * put user into group NULL 
     */
    logBuf.groups[0] = grpBuf.group[0].groupgen;
    
    /* 
     * put user into auto-add groups 
     */
    for (i=0; i<MAXGROUPS; i++)
    {
        if (grpBuf.group[i].autoAdd)
        {
            logBuf.groups[i] = grpBuf.group[i].groupgen;
        }
    }

    /*
     * accurate read-userlog for first time call 
     */
    logBuf.callno   = cfg.callno + 1;
    logBuf.credits  = (float)0;

#ifdef GOODBYE          /* This stuff moved to newUser()
    time(&logBuf.calltime);

    setsysconfig();

    /*
     * trap it 
     */
    sprintf( msgBuf->mbtext, "New user %s", logBuf.lbname);
    trap(msgBuf->mbtext, T_LOGIN);

    loggedIn = TRUE;
    slideLTab(thisSlot);
    storeLog();

#endif          /* End of stuff moved to newUser() */

    return(TRUE);
}


/* -------------------------------------------------------------------- */
/*  newslot()       attempts to find a slot for a new user to reside in */
/*                  puts slot in global var  thisSlot                   */
/* -------------------------------------------------------------------- */
int newslot(void)
{
    int i;
    int foundit = ERROR;

    for ( i = cfg.MAXLOGTAB - 1; ((i > -1) && (foundit == ERROR)) ; --i)
    {
        if (!logTab[i].ltflags.PERMANENT) foundit = i;
    }
    if (foundit == ERROR)
    {
        mPrintf("\n All log slots taken.\n");
    }
    return foundit;
}



