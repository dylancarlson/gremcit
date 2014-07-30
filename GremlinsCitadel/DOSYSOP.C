/* -------------------------------------------------------------------- */
/*  DOSYSOP.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*        Code for dosysop() and some function implemetations.          */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  doSysop()       Privlaged Fn: menu breakdown                        */
/*  do_SysopGroup() handles doSysop() Group functions                   */
/*  do_SysopHall()  handles the doSysop hall functions                  */
/*  sysopunlink()   unlinks ambiguous filenames, sysop only             */
/*  globalverify()  does global sweep to verify any un-verified         */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*  doSysop()       Privileged Fn: menu breakdown                       */
/* -------------------------------------------------------------------- */
char doSysop(void)
{
/*  char  oldIO;  */
    int   c;
#ifdef NEWMSGTAB
    int j;
    struct msgTabList *lml;
#endif

    char  ich;

    reverse = FALSE;

/*   oldIO = whichIO;  */
    
#ifdef GOODBYE
    /* we want to be in console mode when we go into sysop menu */
    if (!gotCarrier() || !sysop)
    {
        whichIO = CONSOLE;
      /*  onConsole = (char)(whichIO == CONSOLE); */
    }
#endif

    sysop = TRUE;

    update25();
 
    while (!ExitToMsdos)
    {
        amZap();
        
        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);

        doCR();
        mPrintf("2Privileged function:0 ");
       
        dowhat = SYSOPMENU;
        c = iCharNE();
        ich = (char)c;
        dowhat = DUNO;
        
        switch (toupper(c))
        {
        case 'A':
            mPrintf("Abort");
            doCR();
        case 0:
            /* restore old mode */
           /*  if (modStat) whichIO = oldIO; */
            if (loggedIn)
            {
                sysop = (uchar)logBuf.lbflags.SYSOP;
            } else {
                sysop = FALSE;
            }
         /*   onConsole = (char)(whichIO == CONSOLE); */
#ifdef GOODBYE            
            if (!onConsole)
                modStat = TRUE;
#endif                
            update25();
            return FALSE;

        case 'C':
            mPrintf("Cron special: ");
            cron_commands();
            break;

        case 'D':
            mPrintf("Date change\n ");
            changeDate();
            break;

        case 'E':
            mPrintf("Enter EXTERNAL.CIT and GRPDATA.CIT files.\n ");
            readaccount();
            readprotocols();
            break;

        case 'F':
            doAide( 1, 'E');
            break;

        case 'G':
            mPrintf("Group special: ");
            do_SysopGroup();
            break;

        case 'H':
            mPrintf("Hallway special: ");
            do_SysopHall();
            break;

        case 'K':
            mPrintf("Kill account\n ");
            killuser();
            break;

        case 'L':
            mPrintf("Login enabled\n ");
            sysopNew = TRUE;
            break;

        case 'M':
            mPrintf("Mass delete\n ");
            massdelete();
            break;

        case 'N':
            mPrintf("NewUser Verification\n ");
            globalverify();
            break;
        
        case 'R':
            mPrintf("Reset file info\n ");
            if (roomBuf.rbflags.MSDOSDIR != 1)
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }
            else updateinfo();
            break;

        case 'S':
            mPrintf("Show user\n ");
            showuser();
            break;

        case 'U':
            mPrintf("Userlog edit\n ");
            userEdit();
            break;

        case 'V':
            mPrintf("View Help Text File\n ");
            tutorial("sysop.hlp");
            break;

        case 'X':
            mPrintf("Exit to MS-DOS");
            doCR();
            if (!onConsole) break;
            if (!getYesNo(confirm, 0))   break;
            if (loggedIn)
            {
                sysop = (uchar)logBuf.lbflags.SYSOP;
            } else {
                sysop = FALSE;
            }
            ExitToMsdos = TRUE;
            return FALSE;

        case 'Z': 
            mPrintf("Zap empty rooms\n ");
            killempties(); 
            break; 

        case '!':        
            mPrintf("Shell");
            doCR();
            if (!onConsole) break;
            shellescape(FALSE);
            break;

        case '@':        
            mPrintf("Super Shell");
            doCR();
            if (!onConsole) break;
            shellescape(TRUE);
            break;


#ifdef  NEWMSGTAB

        case '^':

        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);

        mPrintf("Newmsgtab debug info"); 

            doCR();
            rmPrintf(" usexms: %d  xmshand: %u  xmssize: %uK",
                usexms, xmshand, xmssize);
            doCR();
            rmPrintf(" emsframe: %u  emshand: %u  emssize: %u (16K pages)",
                emsframe, emshand, emssize);
            doCR();
            rmPrintf(" vrthand: %u, vrtpags: %u", vrthand, vrtpags);

#ifdef GOODBYE
            doCR();
            mPrintf(" usexms: %d  xmshand: %u  xmssize: %dK",
                usexms, xmshand, xmssize);
            doCR();
            mPrintf(" emsframe: %d  emshand: %d  emssize: %d (16K pages)",
                emsframe, emshand, emssize);
            doCR();
            mPrintf(" vrthand: %d, vrtpags: %d", vrthand, vrtpags);
#endif

            lml = mtList;
            j = 1;
            while (lml != NULL)
                {
                doCR();
                mPrintf(" mtChunk %2d is in ", j);
                if (lml->whatMem == mtHEAP) mPrintf("heap");
                if (lml->whatMem == mtXMS)  mPrintf("XMS ");
                if (lml->whatMem == mtEMS)  mPrintf("EMS ");
                if (lml->whatMem == mtVIRT) mPrintf("virt");
                mPrintf(" memory at %p. It holds slots %7lu to %7lu.",
                           lml->where,
                           lml->firstMsg, lml->firstMsg + MSGtABpERbLK - 1);
                j++;
                lml = lml->next;
            }
            doCR();
            lml = freeChunk;
            mPrintf(" The free chunk is in ");
            if (lml->whatMem == mtHEAP) mPrintf("heap");
            if (lml->whatMem == mtXMS)  mPrintf("XMS ");
            if (lml->whatMem == mtEMS)  mPrintf("EMS ");
            if (lml->whatMem == mtVIRT) mPrintf("virt");
            mPrintf(" memory at %p.", lml->where);

            doCR();
            break;

#endif

        case '#':
            mPrintf("Read by %s number\n ", cfg.msg_nym);
            readbymsgno();
            break;

        case '$':
            mPrintf("System status"); doCR();
            verbose = TRUE;
            systat();
            break;
            
        case '*':
            mPrintf("Unlink file(s)\n ");
            if (roomBuf.rbflags.MSDOSDIR != 1)
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }
            else sysopunlink();
            break;

        case '&':
            mPrintf("Enter CONFIG.CIT file.\n ");
            /* just in case */
            changedir(cfg.homepath);
            readconfig(2);
            break;

        case '?':
            oChar('?');
            tutorial("sysop.mnu");
            break;

        default:
            oChar(ich);
            if (!expert)  mPrintf("\n '?' for menu.\n "  );
            else          mPrintf(" ?\n "                );
            break;
        }
    }
    if (loggedIn)
    {
        sysop = (uchar)logBuf.lbflags.SYSOP;
    } else {
        sysop = FALSE;
    }
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  do_SysopGroup() handles doSysop() Group functions                   */
/* -------------------------------------------------------------------- */
void do_SysopGroup()
{
    char ich;
    switch(toupper(ich=(char)iCharNE()))
    {
    case 'G':
        mPrintf("Global Group membership ");
        globalgroup();
        break;
    case 'K':
        mPrintf("Kill group");
        killgroup();
        break;
    case 'N':
        mPrintf("New group");
        newgroup();
        break;
    case 'U':
        mPrintf("Global user membership\n  \n");
        globaluser();
        break;
    case 'R':
        mPrintf("Rename group");
        renamegroup();
        break;
    case '?':
        oChar('?');
        doCR();
        tutorial("sysgroup.mnu");
        break;
        
    default:
        oChar(ich);
        if (!expert)  mPrintf("\n '?' for menu.\n "  );
        else          mPrintf(" ?\n "                );
        break;
    }
}

/* -------------------------------------------------------------------- */
/*  do_SysopHall()  handles the doSysop hall functions                  */
/* -------------------------------------------------------------------- */
void do_SysopHall()
{
    char ich;
    switch(toupper(ich=(char)iCharNE()))
    {
    case 'F':
        mPrintf("Force access");
        force();
        break;
    
    case 'K':
        mPrintf("Kill hallway");
        killhall();
        break;
    
    case 'L':
        mPrintf("List halls");
        listhalls();
        break;
    
    case 'N':
        mPrintf("New hall");
        newhall();
        break;
    
    case 'R':
        mPrintf("Rename hall");
        renamehall();
        break;
    
    case 'G':
        mPrintf("Global Hall func"); doCR();
        globalhall();
        break;

    case '-':
        moveHall(-1);
        break;
        
    case '+':
        moveHall(1);
        break;
    
    case '?':
        oChar('?');
        doCR();
        tutorial("syshall.mnu");
        break;
        
    default:
        oChar(ich);
        if (!expert)  mPrintf("\n '?' for menu.\n "  );
        else          mPrintf(" ?\n "                );
        break;
    }
}

/* -------------------------------------------------------------------- */
/*  sysopunlink()   unlinks ambiguous filenames, sysop only             */
/* -------------------------------------------------------------------- */
void sysopunlink(void)
{
    label files;
    int i;

    getString("file(s) to unlink", files, NAMESIZE, FALSE, ECHO, "");

    if(files[0])
    {
        i = ambigUnlink(files, TRUE);
        if(i)
            updateinfo();
        doCR();
        mPrintf("(%d) file(s) unlinked", i);
        doCR();

        sprintf(msgBuf->mbtext, "File(s) %s unlinked in room %s]",
                    files, roomBuf.rbname);

        trap(msgBuf->mbtext, T_SYSOP);
    }
}

/*----------------------------------------------------------------------*/
/*  globalverify()  does global sweep to verify any un-verified         */
/*----------------------------------------------------------------------*/
void globalverify(void)
{
    int    logNo, i, yn;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    for (i=0;( (i < cfg.MAXLOGTAB) && (outFlag != OUTSKIP) && !mAbort(FALSE) );i++)
    {
        if (logTab[i].ltpwhash != 0 && logTab[i].ltnmhash !=0)
        {
            logNo=logTab[i].ltlogSlot;
            getLog(&lBuf, logNo);
            if (lBuf.VERIFIED == TRUE)
            {
                mPrintf("\n 3%s0", lBuf.lbname);
                doCR();
                yn=getYesNo("Verify", 0+3);
                if (yn == 2)
                {
                    SaveAideMess();
                    return;
                }
                if (yn)
                {
                    lBuf.VERIFIED = FALSE;
                    if (strcmpi(logBuf.lbname, lBuf.lbname) == SAMESTRING)
                        logBuf.VERIFIED = FALSE;
                    sprintf(msgBuf->mbtext, "%s verified by %s",
                                           lBuf.lbname, logBuf.lbname );
                    trap(msgBuf->mbtext, T_SYSOP);
                    amPrintf(" %s\n", msgBuf->mbtext);
                }
                else
                if (strcmpi(logBuf.lbname, lBuf.lbname) != SAMESTRING)
                {
                    if (getYesNo("Kill account", 0))
                    {
                        mPrintf( "\n \'%s\' terminated.\n ", lBuf.lbname);
                            /* trap it */
                        sprintf(msgBuf->mbtext,
                            "Un-verified user %s terminated", lBuf.lbname);
                        trap(msgBuf->mbtext, T_SYSOP);
                        lBuf.lbname[0] = '\0';
                        lBuf.lbin[  0] = '\0';
                        lBuf.lbpw[  0] = '\0';
                        lBuf.lbflags.L_INUSE   = FALSE;
                        lBuf.lbflags.PERMANENT = FALSE;
                    }
                }
                putLog(&lBuf, logNo);
            }
        }
    }
    
    SaveAideMess();
}


