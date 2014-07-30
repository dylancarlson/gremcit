/* -------------------------------------------------------------------- */
/*  MISC2.C                  Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                        Overlayed misc stuff                          */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  systat()        System status                                       */
/*  ringSystemREQ() signals a system request for 2 minutes.             */
/*  dial_out()      dial out to other boards                            */
/*  logo()          prints out logo screen and quote at start-up        */
/*  greeting()      System-entry blurb etc                              */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  ringSystemREQ() signals a system request for 2 minutes.             */
/* -------------------------------------------------------------------- */
void ringSystemREQ(void)
{
    unsigned char row, col;
    char i;
    char answered = FALSE;
    char ringlimit = 120;

    doccr();
    doccr();
    readpos( &row, &col);
    (*stringattr)(row," * System Available * ",(uchar)(cfg.wattr | 128));
    update25();
    doccr();

    Hangup();
    
    answered = FALSE;
    for (i = 0; (i < ringlimit) && !answered; i++)
    {
        outCon(7 /* BELL */);
        pause(80);
        if (KBReady()) answered = TRUE;
    }

    if (!KBReady() && i >= ringlimit)  Initport();

    update25();
}

/* -------------------------------------------------------------------- */
/*  dial_out()      dial out to other boards                            */
/* -------------------------------------------------------------------- */
void dial_out(void)
{
    char con, mod;
    long update = 0L;
    char duplex = 0;    /* 0 = full 1 = half */

    if (!anyEcho) return;

    dowhat = DIALOUT;

    dialout_fkey = 0;

    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    doccr();
    doccr();

    cPrintf(" Now in dial out mode.  Alt-Q to quit,\n"
            " Pg-Up/Pg-Dn for baud rate change,\n" 
            " Alt-E/Alt-S for shell/supershell.\n"
            " Alt-P for duplex toggle.\n");

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    Hangup(); 

    disabled = FALSE;
 
    baud(cfg.initbaud);

    update25();

    outstring(cfg.dialsetup); 
    outstring("\r");

    pause(100);

    callout = TRUE;

    do
    {
        con = 0;  mod = 0;

        if (KBReady())
        {
            con = (char)ciChar();
            if (con)
            {
                outMod(con);
                if (duplex) 
                    oChar(con); /* only to console */
            }
        }

        if (MIReady())
        {
            mod = (char)getMod();

            /* I'm not sure what use this is */
#ifdef GOODBYE
            if (debug)
            {
              mod = (char)( mod & 0x7F );
              mod = tfilter[mod];
            }
#endif

            if (mod == '\n')
              doccr();
            else
              if (mod != '\r')
                  oChar(mod); /* only to console */
        }

        /* handle alt-keys */
        switch(dialout_fkey)
        {
            case ALT_D:
                debug = (char)!debug;
                update25();
                dialout_fkey = 0;
                break;
            case  PGUP:
                speed++;
                if (speed > 7) speed = 0;
                baud(speed);
                update25();
                dialout_fkey = 0;
                break;
            case  PGDN:
                speed--;
                if (speed < 0) speed = 7;
                baud(speed);
                update25();
                dialout_fkey = 0;
                break;
            case  ALT_S:
                if (sysop || !ConLock)
                    shellescape(TRUE);
                dialout_fkey = 0;
                break;
            case  ALT_E:
                if (sysop || !ConLock)
                    shellescape(FALSE);
                dialout_fkey = 0;
                break;
            case  ALT_P:
                duplex = (char)!duplex;
                doccr();
                if (duplex)
                    cPrintf("Half");
                else
                    cPrintf("Full");
                cPrintf(" Duplex.");
                doccr();
                dialout_fkey = 0;
                break;
            case  ALT_Q:
                doccr();
                cPrintf("Quit.");
                doccr();
            default:
                break;
        }

        if (!modStat && !loggedIn)     /* Update time on status line */
            if (time(NULL) != update)      /* Only update if changed */
            {
                update25();
                time(&update);
            }

    } while (dialout_fkey != ALT_Q);
    
    callout = FALSE;

    Initport();

    if (cfg.offhook && loggedIn)
        offhook();

    doCR();

    dowhat = DUNO;
}

/* -------------------------------------------------------------------- */
/*  systat()        System status                                       */
/* -------------------------------------------------------------------- */
void systat(void)
{
    union REGS r;
    int i;
    int percentfull;

#ifdef NEWMSGTAB
    ulong j;
/*  struct msgTabList *lml;  */
    struct msgflags *lmf;
    ulong tablesize;
#else
    int j;
    int tablesize;
#endif

    long average, work;
    char dtstr[80];
    int  public    = 0,     /* talleys.. */
         active    = 0,
         directory = 0,
         shared    = 0,
         private   = 0,
         anon      = 0,
         group     = 0,
         problem   = 0,
         perm      = 0,
         aides     = 0,
         sysops    = 0,
         nodes     = 0,
         moderated = 0; 

#ifdef GOODBYE
,
         copies    = 0,
         networked = 0;
#endif

    ulong public_mess     = 0,
          networked_mess  = 0,
          private_mess    = 0,
          moderated_mess  = 0,
          group_mess      = 0,
          copies_mess     = 0,
          problem_mess    = 0;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    
    /*
     * On...
     */
    doCR();
    rmPrintf(" 3You are on:0\n %s, %s, %s", 
            cfg.nodeTitle, cfg.nodeRegion, cfg.nodeContry); doCR();
    
    doCR();

    rmPrintf(" 3Running:0\n %s Version %s (%s)", programName, version,  
compilerName); 

doCR();

  rmPrintf(" %s", copyright);                       doCR();

    if (verbose)
    {

        rmPrintf(" Compiled on %s at %s", cmpDate, cmpTime); doCR();
    }
    
    
    /*
     * times.. 
     */
    doCR();
    strftime(dtstr, 79, (loggedIn) ? logBuf.vdstamp : cfg.vdatestamp, 0l);
    rmPrintf(" 3It is:    0%s", dtstr); doCR();
    if (verbose)
    {
        rmPrintf(" 3Up for:   %s0", diffstamp(uptimestamp)); doCR();
    }
    if (gotCarrier())
    {
        rmPrintf(" 3Online:   %s0", diffstamp(conntimestamp)); doCR();
    }

    if (loggedIn)
    {
        rmPrintf(" 3Loggedin: %s0",  diffstamp(logtimestamp)); doCR();
    }
    
    /*
     * Userlog info.. 
     */
    doCR();
    rmPrintf(" 3Userlog stats:0"); doCR();
    rmPrintf(" %d log entries", cfg.MAXLOGTAB); doCR();
    active = aides = sysops = problem = perm = nodes = 0;
    for (i=0; i < cfg.MAXLOGTAB; i++)
    {
        if (logTab[i].ltflags.L_INUSE)
        {
            active++;
            if (logTab[i].ltflags.AIDE)         aides++;
            if (logTab[i].ltflags.SYSOP)        sysops++;
            if (logTab[i].ltflags.PROBLEM)      problem++;
            if (logTab[i].ltflags.PERMANENT)    perm++;
            if (logTab[i].ltflags.NODE)         nodes++;
        }
    }
    rmPrintf(" %d active", active); doCR();
    rmPrintf(" %s calls", ltoac(cfg.callno));   doCR();
    if (verbose)
    {
        rmPrintf(" %d nodes", nodes);
        if (aide || sysop)
            rmPrintf(", %d aides, %d sysops, %d problem users, and %d permanent",
                    aides, sysops, problem, perm);  doCR();
    }
    
    /*
     * Room file info.. 
     */
    doCR();
    rmPrintf(" 3Room stats:0");            doCR();
    rmPrintf(" %d rooms", MAXROOMS);         doCR();
                                            
    active = public = private = directory = 
    group = shared = moderated = anon = 0;
    
    for (i=0; i<MAXROOMS; i++)
    {
        if (roomTab[i].rtflags.INUSE)
        {
            active++;
            if (roomTab[i].rtflags.PUBLIC)        public++;
            else                                    private++;
            if (roomTab[i].rtflags.MSDOSDIR)      directory++;
            if (roomTab[i].rtflags.GROUPONLY)     group++;
            if (roomTab[i].rtflags.SHARED)        shared++;
            if (roomTab[i].rtflags.MODERATED)     moderated++;
            if (roomTab[i].rtflags.ANON)          anon++;
        }
    }
    
    rmPrintf(" %d active", active);          doCR(); 
    
    if (verbose)
    {
        rmPrintf(" %d public, %d private, %d directory,"
                " %d moderated, %d group only, %d networked, and %d anon.", 
                public, private, directory, moderated, group, shared, anon);
        doCR();
    
    }


    /*
     * Hall info.. 
     */
    doCR();
    rmPrintf(" 3Hall stats:0");            doCR();
    rmPrintf(" %d halls", MAXHALLS);         doCR();



    active =0;
    group = 0;
    anon = 0;


    for (i = 0; i < MAXHALLS; ++i)
    {
        if (hallBuf->hall[i].h_inuse)
        {
            active++;

            if (hallBuf->hall[i].owned)
                group++;

            if (hallBuf->hall[i].enterRoom)
                anon++;
        }
    }

    rmPrintf(" %d active", active);          doCR(); 

    if (verbose)
    {

        rmPrintf(" %d group only, %d roomok", group, anon);
        doCR();
    }
   
    /*
     * Message(s) status.. 
     */
    doCR();
    strcpy(dtstr, cfg.msg_nym);
    dtstr[0] = (char)toupper(dtstr[0]);
    rmPrintf(" 3%s stats:0", dtstr); doCR();
    
    /* stop before the message count if already aborted.. */
    if (outFlag != OUTOK)
        return;
        
    if (verbose)
    {

          public_mess     = 
          networked_mess  = 
          private_mess    = 
          moderated_mess  = 
          group_mess      = 
          copies_mess     = 
          problem_mess    = 0;

/*  public_mess = private_mess = group_mess = moderated_mess = problem_ = 0; */
        
#ifdef NEWMSGTAB
        tablesize = sizetable();
#else
        tablesize = (int)sizetable();
#endif

        for( j = 0; j < tablesize; ++j)
        {

#ifdef NEWMSGTAB

                 lmf = getFlags(j);


                 if (lmf->COPY     ) copies_mess++;
                 if (lmf->PROBLEM  ) problem_mess++;
                 if (lmf->MODERATED) moderated_mess++;
                 if (lmf->LIMITED  ) group_mess++  ;
           else  if (lmf->MAIL     ) private_mess++;
           else                        public_mess++ ;
                  if (lmf->NETWORKED)  networked_mess++;
             /*   if (msgTab8[j].mtomesg)               networked++; */


#else
                 if (msgTab_mtmsgflags[j].COPY     ) copies_mess++;
                 if (msgTab_mtmsgflags[j].PROBLEM  ) problem_mess++;
                 if (msgTab_mtmsgflags[j].MODERATED) moderated_mess++;
                 if (msgTab_mtmsgflags[j].LIMITED  ) group_mess++  ;
           else  if (msgTab_mtmsgflags[j].MAIL     ) private_mess++;
           else                                      public_mess++ ;
                  if (msgTab_mtmsgflags[j].NETWORKED)  networked_mess++;
             /*   if (msgTab8[j].mtomesg)               networked++; */

#endif
        }
    }


    
    rmPrintf(" %s entered", ltoac(cfg.newest)); doCR();
    rmPrintf(" %s online, #%lu to %lu",
    ltoac(cfg.newest - cfg.oldest + 1), cfg.oldest, cfg.newest); 
     doCR();

    if ( (cfg.mtoldest - cfg.oldest) > 0 && aide)
    {
        rmPrintf(" %lu missing", (cfg.mtoldest - cfg.oldest)); doCR();
    }

    /* if oldest message on system is 1 or 1st message in table is located */
    /* at very beginning of message file then use current position to */
    /* compute average message length. */

#ifdef NEWMSGTAB
    if (cfg.oldest == 1L  || (getLocation(0) == 0)) 
#else
    if (cfg.oldest == 1L  || (long_JOIN(msgTab_mtmsgLocLO[0], 
        msgTab_mtmsgLocHI[0] == 0))) 
#endif

    {
        work = cfg.catLoc;
percentfull = (int)(((cfg.catLoc * 100L) / ((long)cfg.messagek * 1024L)));
    }
    else
    {
        work = (long)((long)cfg.messagek * 1024l);
        percentfull = 100;
    }

    average = (work) / (cfg.newest - cfg.oldest + 1);




    if (verbose)
    {
        rmPrintf(" %lu public, %lu networked, %lu private, %lu moderated, ", 
                public_mess, networked_mess, private_mess, moderated_mess);
    
        if (!aide) rmPrintf("and ");
    
        rmPrintf("%lu group only", group_mess);
    
        if (aide)  rmPrintf(", %lu copies, and %lu problem user", copies_mess, 
problem_mess  /* , cfg.msgs_nym */);
        doCR();


#ifdef NEWMSGTAB
        rmPrintf(" %luK %s space, %ld %s slots", cfg.messagek, cfg.msg_nym, 
cfg.nmessages, cfg.msg_nym); 

#else
        rmPrintf(" %luK %s space, %d %s slots", cfg.messagek, cfg.msg_nym, 
cfg.nmessages, cfg.msg_nym); 
#endif

        if (percentfull < 100)
        rmPrintf(", base is %d%% full", percentfull);
        
        doCR();

    }                         


    rmPrintf(" %s bytes average %s length", ltoac(average), cfg.msg_nym); doCR();


    
    /*
     * System and debugging.. 
     */
    if (/* sysop && */ verbose)
    {
        r.h.ah = 0x48;   /* allocate memory              */
        r.h.al = 0;
        r.x.bx = 0xffff; /* ask for an impossible amount */

        intdos(&r, &r);

        doCR();
        rmPrintf(" 3System stats / debugging:0"); doCR();
        rmPrintf(" Host system is an IBM or compatible.");  doCR();  
        rmPrintf(" DOS version %d.%d", _osmajor, _osminor); doCR();
        
     /* rmPrintf(" %u bytes stack space", _stklen );        doCR(); */
        rmPrintf(" %u bytes stack space", stackavail() );        doCR();

        rmPrintf(" %uK system memory", _bios_memsize());       doCR();
        rmPrintf(" %ld bytes free system memory", (long)r.x.bx*16L); doCR();
/* rmPrintf(" %s bytes free system memory", ltoac(farcoreleft()) ); doCR(); */
        strcpy(dtstr, ltoac(received));
        rmPrintf(" %s characters transmitted, %s characters received",
                ltoac(transmitted), dtstr ); doCR();
    }



#ifdef GOODBYE

    if (sysop || verbose)
    {
        if (debug)
        {

        doCR();
        rmPrintf(" 3Newmsgtab stats:0"); 

            doCR();
            rmPrintf(" usexms: %d  xmshand: %u  xmssize: %uK",
                usexms, xmshand, xmssize);
            doCR();
            rmPrintf(" emsframe: %u  emshand: %u  emssize: %u (16K pages)",
                emsframe, emshand, emssize);
            doCR();
            rmPrintf(" vrthand: %u, vrtpags: %u", vrthand, vrtpags);

            lml = mtList;
            j = 1;
            while (lml != NULL)
                {
                doCR();
                rmPrintf(" mtChunk %2d is in ", j);
                if (lml->whatMem == mtHEAP) rmPrintf("heap");
                if (lml->whatMem == mtXMS)  rmPrintf("XMS ");
                if (lml->whatMem == mtEMS)  rmPrintf("EMS ");
                if (lml->whatMem == mtVIRT) rmPrintf("virt");
                rmPrintf(" memory at %p. It hold slots %7lu to %7lu.",
                           lml->where,
                           lml->firstMsg, lml->firstMsg + MSGtABpERbLK - 1);
                j++;
                lml = lml->next;
            }
            doCR();
            lml = freeChunk;
            rmPrintf(" The free chunk is in ");
            if (lml->whatMem == mtHEAP) rmPrintf("heap");
            if (lml->whatMem == mtXMS)  rmPrintf("XMS ");
            if (lml->whatMem == mtEMS)  rmPrintf("EMS ");
            if (lml->whatMem == mtVIRT) rmPrintf("virt");
            rmPrintf(" memory at %p.", lml->where);

            doCR();
        }
    }
#endif


}


