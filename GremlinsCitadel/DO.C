/* -------------------------------------------------------------------- */
/*  DO.C                     Dragon Citadel                             */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*      doChat()                handles C(hat)          command         */
/*      doEnter()               handles E(nter)         command         */
/*      doGoto()                handles G(oto)          command         */
/*      doHelp()                handles H(elp)          command         */
/*      doIntro()               handles I(ntro)         command         */
/*      doKnown()               handles K(nown rooms)   command         */
/*      doLogin()               handles L(ogin)         command         */
/*      doLogout()              handles T(erminate)     command         */
/*      doRead()                handles R(ead)          command         */
/*      doNext()                handles '+' next room                   */
/*      doPrevious()            handles '-' previous room               */
/*      doNextHall()            handles '>' next room                   */
/*      doPreviousHall()        handles '<' previous room               */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/************************************************************************/
/*      doDownload()                                                    */
/************************************************************************/
void doDownload(char ex)
{
    mPrintf("Download ");

    if  (!loggedIn && !cfg.unlogReadOk) 
    {
        mPrintf("\n  --Must log in to download.\n ");
        return;
    }

    /* handle uponly flag! */
    if ( roomTab[thisRoom].rtflags.UPONLY && !pgroupseesroom() )
    {
        doCR();
        mPrintf(" --Room is upload only.");
        doCR();
        return;
    }

    if ( !roomBuf.rbflags.MSDOSDIR )
    {
        if (expert) mPrintf("? ");
        else        mPrintf("\n Not a directory room.");
        return;
    }

    download((char)((ex || !logBuf.protocol) ? '\0' : logBuf.protocol));
}

/************************************************************************/
/*      doUpload()                                                      */
/************************************************************************/
void doUpload(char ex)
{
    mPrintf("Upload ");

    if ( !loggedIn && !cfg.unlogEnterOk )
    {
        mPrintf("\n  --Must log in to upload.\n ");
        return;
    }

    /* handle downonly flag! */
    if ( roomTab[thisRoom].rtflags.DOWNONLY && !pgroupseesroom())
    {
        doCR();
        mPrintf(" --Room is download only.");
        doCR();
        return;
    }

    if ( !roomBuf.rbflags.MSDOSDIR )
    {
        if (expert) mPrintf("? ");
        else        mPrintf("\n Not a directory room.");
        return;
    }
    
    upload((char)((ex || !logBuf.protocol) ? '\0' : logBuf.protocol));
    return;
}

/************************************************************************/
/*      doChat()                                                        */
/************************************************************************/
void doChat(char moreYet, char first)
/* char moreYet; */   /* TRUE to accept following parameters  */
/* char first;   */   /* first parameter if TRUE              */
{
    moreYet = moreYet;  /* -W3 */
    first   = first;    /* -W3 */
    
    chatReq = TRUE;
    
    mPrintf("Chat ");

    trap("Chat request", T_CHAT);

    if (cfg.noChat)
    {
        nochat(FALSE);
        return;
    }

    if (whichIO == MODEM)  ringSysop();
    else                   chat() ;
}


/***********************************************************************/
/*     doEnter() handles E(nter) command                               */
/***********************************************************************/
void doEnter(char moreYet,char first)
{
    char done;
    char letter;
    char ich;

    if (moreYet)  first = '\0';

    done      = TRUE ;
    mailFlag  = FALSE;
    oldFlag   = FALSE;
    limitFlag = FALSE;
    linkMess  = FALSE;

    mPrintf("Enter ");

    do  
    {
        done    = TRUE;

  letter = (char)(toupper( first ? (char)first : (char)(ich=(char)iCharNE())));

        /* allow exclusive mail entry only */
        if ( !loggedIn && !cfg.unlogEnterOk && (letter != 'E') )
        {
            mPrintf("\n  --Must log in to enter.\n ");
            break;
        }

        switch (letter)
        {
        case '\r':
        case '\n':
        case 'M':
            mPrintf("%s ", cfg.msg_nym);

            /* handle readonly flag! */
            if ( roomTab[thisRoom].rtflags.READONLY && !pgroupseesroom())
            {
                mPrintf("\n\n  --Room is readonly.\n ");
                return;
            }

            /* handle steeve */
            if(MessageRoom[thisRoom]==(char)cfg.MessageRoom && !(sysop || aide))
            {
                mPrintf("\n\n  --Only %d %s per room.\n ",cfg.MessageRoom, 
                    cfg.msgs_nym);
                return;
            }

            doCR();
            makeMessage();
            break;
            
        case 'B':
            mPrintf("Border Line");
            if (!logBuf.BOARDERS && !sysop)
            {
                mPrintf("\n\n  --You can't enter a border.\n ");
                return;
            }
            editBorder();
            break;
            
        case 'C':
            mPrintf("Configuration ");
            configure(FALSE);
            break;
            
        case 'D':
            mPrintf("Door ");
            if (!execDoor(letter = (char)iCharNE()))
            {
                if (letter == '?')
                {
                    oChar('?');
                    tutorial("DOORS.MNU");
                }
                else
                {
                    oChar(letter);
                    mPrintf(" ? for list of doors");
                    doCR();
                }
            }
            break;
            
        case 'E':
            mPrintf("Exclusive %s ", cfg.msg_nym);

            /* handle readonly flag! */
            if ( roomTab[thisRoom].rtflags.READONLY && !pgroupseesroom())
            {
                mPrintf("\n\n  --Room is readonly.\n ");
                return;
            }

            /* handle steeve */
            if(MessageRoom[thisRoom]==(char)cfg.MessageRoom && !(sysop || aide))
            {
                mPrintf("\n\n  --Only %d %s per room.\n ",cfg.MessageRoom, 
                    cfg.msgs_nym);
                return;
            }

            /* handle nomail flag! */
            if (logBuf.lbflags.NOMAIL)
            {
                doCR();
                doCR();
                mPrintf("  --You can't enter exclusive messages."); doCR();
                return;
            }
        

#ifdef GOODBYE
            /* no mail in anon rooms! */
            if ( roomTab[thisRoom].rtflags.ANON)
            {
                doCR();
                doCR();
                mPrintf("  --Not in Anon rooms."); doCR();
                return;
            }
#endif

            doCR();
            if (whichIO != CONSOLE) 
            { 
                echo = CALLER;     
                setio(whichIO, echo, outFlag);  
            }

            limitFlag = FALSE; 
            mailFlag = TRUE;
            makeMessage();
            echo = BOTH;
            setio(whichIO, echo, outFlag);

            break;
        
        case 'H':
            mPrintf("Hallway ");
            doCR();
            enterhall();
            break;
        case 'L':
            mPrintf("Limited-access ");
            done      = FALSE;
            limitFlag = TRUE;
            break;
        case '*':
            mPrintf("File-linked ");

            if ( !sysop && (letter == '*'))
            {
                mPrintf("\n\n  --You can't enter a file-linked %s.\n ", 
                    cfg.msg_nym);
                return;
            }

            done      = FALSE;
            linkMess  = TRUE;
            break;
        case 'G':
            mPrintf("Group-only ");
            done      = FALSE;
            limitFlag = TRUE;
            break;


#ifdef GOODBYE
            mPrintf("Group-only %s ", cfg.msg_nym);

            /* handle readonly flag! */
            if ( roomTab[thisRoom].rtflags.READONLY && !pgroupseesroom())
            {
                mPrintf("\n\n  --Room is readonly.\n ");
                return;
            }

            /* handle steeve */
            if(MessageRoom[thisRoom]==(char)cfg.MessageRoom && !(sysop || aide))
            {
                mPrintf("\n\n  --Only %d %s per room.\n ",cfg.MessageRoom, 
                    cfg.msgs_nym);
                return;
            }

            doCR();
            limitFlag = TRUE;
            makeMessage();
            break;
#endif

        case 'O':
            mPrintf("Old ");
            done    = FALSE;
            oldFlag = TRUE;
            break;
        case 'P':
            mPrintf("Password ");
            doCR();

            if (!loggedIn)
            {
                mPrintf("\n  --Must be logged in.\n ");
                return;
            }

            newPW();
            break;

#ifdef GOODBYE
        case 'R':
            mPrintf("Room ");
            if (   !cfg.nonAideRoomOk 
                && !aide 
                && !hallBuf->hall[thisHall].enterRoom)
            {
                 doCR(); 
                 mPrintf(" --Must be aide to create room.");
                 doCR(); 
                 break;
            }
            doCR();
            makeRoom();
            break;
#endif
        case 'R':
            mPrintf("Room ");

            if (!loggedIn)
            {
                 mPrintf("\n  --Must log in to create new room.\n ");
                 break;
            }
            if (!(hallBuf->hall[thisHall].enterRoom || cfg.nonAideRoomOk
                || aide))
            {
                mPrintf("\n  --Must be aide to create room.\n ");
                 break;
            }
            doCR();
            makeRoom();
            break;

        case 'T':
            mPrintf("Textfile ");

            if (roomBuf.rbflags.MSDOSDIR != 1)
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
                return;
            }

            /* handle downonly flag! */
            if ( roomTab[thisRoom].rtflags.DOWNONLY && !pgroupseesroom())
            {
                mPrintf("\n\n  --Room is download only.\n ");
                return;
            }
            if (!loggedIn)
            {
                mPrintf("\n  --Must be logged in.\n ");
                return;
            }

            entertextfile();
            break;
        case 'W':
            mPrintf("WC-protocol upload ");
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
                return;
            }

            /* handle downonly flag! */
            if ( roomTab[thisRoom].rtflags.DOWNONLY && !pgroupseesroom())
            {
                mPrintf("\n\n  --Room is download only.\n ");
                return;
            }

            if (!loggedIn)
            {
                mPrintf("\n  --Must be logged in.\n ");
                return;
            }

            enterwc();
            /* doCR(); */
            break;
#ifdef GOODBYE
        case 'A':
            mPrintf("Application");
            ExeAplic();
            break;
#endif
        case 'A':
            mPrintf("Application");
            if (!loggedIn)
            {
                mPrintf("\n  --Must be logged in.\n ");
            }
            else ExeAplic();
            break;

        case 'X':
            mPrintf("Exclude Room ");
            exclude();
            break;
        case 'S':
            if (cfg.surnames || cfg.titles)
            {
                label tempsur;

                mPrintf("Surname / Title"); 
  
                if ( !sysop && !cfg.entersur )
                {
             mPrintf("\n\n  --Users can't enter their title and surname.\n ");
                    return;
                }

                if ( !sysop && logBuf.SURNAMLOK )
                {
                mPrintf("\n\n  --Your title and surname has been locked!\n ");
                     return;
                }

                doCR();
                
                if (cfg.titles)
                {
                    getString("title", tempsur, NAMESIZE, 0, ECHO, 
                              logBuf.title);
                    if (*tempsur)
                    {
                        strcpy(logBuf.title, tempsur);
                        normalizeString(logBuf.title);
                    }
                }
                
                if (cfg.surnames)
                {
                    getString("surname", tempsur, NAMESIZE, 0, ECHO, 
                              logBuf.surname);
                    if (*tempsur)
                    {
                        strcpy(logBuf.surname, tempsur);
                        normalizeString(logBuf.surname);
                    }
                }

                /* putLog(&logBuf, thisLog);  */
                storeLog();

                break;
            }
        default:
            oChar(ich);
            mPrintf("? ");
            if (expert)  break;
        case '?':
            if (letter == '?') oChar('?');
            tutorial("entopt.mnu");
            break;
        }
    }
    while (!done && moreYet);

    oldFlag   = FALSE;
    mailFlag  = FALSE;
    limitFlag = FALSE;

}

/************************************************************************/
/*      exclude() handles X>clude room,  toggles the bit                */
/************************************************************************/
void exclude(void)
{
    if  (!logBuf.lbroom[thisRoom].xclude)
    {
         mPrintf("\n \n Room now excluded from G)oto loop.\n ");
         logBuf.lbroom[thisRoom].xclude = TRUE;  
    }else{
         mPrintf("\n \n Room now in G)oto loop.\n ");
         logBuf.lbroom[thisRoom].xclude = FALSE;
    }
}

#ifdef GOODBYE
/************************************************************************/
/*      exclude() handles X>clude room,  toggles the bit                */
/************************************************************************/
void exclude(void)
{
    doCR();
    
    if (getYesNo("Exclude room from G)oto loop", logBuf.lbroom[thisRoom].xclude))
        logBuf.lbroom[thisRoom].xclude = TRUE;  
    else
        logBuf.lbroom[thisRoom].xclude = FALSE;
}
#endif

/************************************************************************/
/*      doGoto() handles G(oto) command                                 */
/************************************************************************/
void doGoto(char expand, char skip)
/* char expand; */   /* TRUE to accept following parameters  */
{
    label roomName;

    if (!skip)
    {
        mPrintf("Goto ");
        skiproom = FALSE;
    } 
    else 
    {
        mPrintf("Bypass to ");
        skiproom = TRUE;
    }

    if (!expand)
    {
        gotoRoom("");
        return;
    }

    getString("", roomName, NAMESIZE, 1, ECHO, "");
    normalizeString(roomName);

    if (roomName[0] == '?')
    {
        verbose = FALSE;
        listRooms(OLDNEW, FALSE);
    }
    else 
    {
        gotoRoom(roomName);
    }
}

#ifdef GOODBYE
/************************************************************************/
/*      doHelp() handles H(elp) command                                 */
/************************************************************************/
void doHelp(char expand)
/* char expand; */   /* TRUE to accept following parameters  */
{
    mPrintf("Help"); doCR(); doCR();
    
    Help(expand);
}
#endif

/************************************************************************/
/*      doHelp() handles H(elp) command                                 */
/************************************************************************/
void doHelp(char expand)
{
    label fileName;

    mPrintf("Help ");
    if (!expand)
    {
        doCR();
        tutorial("dohelp.hlp");
        return;
    }

    getString("", fileName, 9, 1, ECHO, "");
    normalizeString(fileName);

    if (strlen(fileName) == 0)  strcpy(fileName, "dohelp");

    if (fileName[0] == '?')
    {
        tutorial("topics.hlp");
    } else {
        /* adding the extention makes things look simpler for           */
        /* the user... and restricts the files which can be read        */
        strcat(fileName, ".hlp");

        tutorial(fileName);
    }
}



/************************************************************************/
/*      doIntro() handles Intro to ....  command.                       */
/************************************************************************/
void doIntro()
{
    mPrintf("Intro to %s\n ", cfg.nodeTitle);
    tutorial("intro.hlp");
}


/***********************************************************************/
/*      doKnown() handles K(nown rooms) command.                       */
/***********************************************************************/
void doKnown(char moreYet, char first)
/* char moreYet; */          /* TRUE to accept following parameters */
/* char first;  */           /* first parameter if true             */
{
    char letter;
/*  char verbose = FALSE; */
    char numMess = FALSE;
    char done;
    char ich;

    verbose = FALSE;
    reverse = FALSE;

    if (moreYet)  first = '\0';

    mPrintf("Known ");

    do  
    {
        done    = TRUE;

  letter = (char)(toupper( first ? (char)first : (char)(ich=(char)iCharNE())));
        switch (letter)
        {
            case 'A':
                mPrintf("Application Rooms ");
                mPrintf("\n ");
                listRooms(APLRMS, numMess);
                break;
            case 'D':
                mPrintf("Directory Rooms ");
                mPrintf("\n ");
                listRooms(DIRRMS, numMess);
                break;
            case 'H':
                mPrintf("Hallways ");
                knownhalls();
                break;
            case 'G':
                mPrintf("Group only Rooms ");
                mPrintf("\n ");
                listRooms(LIMRMS, numMess);
                break;
            case 'L':
                mPrintf("Local Rooms ");
                mPrintf("\n ");
                listRooms(NOTSHRDRM, numMess);
                break;
            case 'N':
                mPrintf("New Rooms ");
                mPrintf("\n ");
                listRooms(NEWRMS, numMess);
                break;
            case 'O':
                mPrintf("Old Rooms ");
                mPrintf("\n ");
                listRooms(OLDRMS, numMess);
                break;
            case 'E':
            case 'M':
                mPrintf("Exclusive Message Rooms ");
                mPrintf("\n ");
                listRooms(MAILRM, numMess);
                break;
            case 'S':
                mPrintf("Shared Rooms ");
                mPrintf("\n ");
                listRooms(SHRDRM, numMess);
                break;
            case 'I':
                mPrintf("Room Info");
                mPrintf("\n ");
                RoomStatus();
                break;
            
            case '\r':
            case '\n':
            case 'R':
                mPrintf("Rooms"); doCR();
                listRooms(OLDNEW, numMess);
                break;
            case 'V':
                mPrintf("Verbose ");
                done    = FALSE;
                verbose = TRUE;
                break;
            case 'W':
                mPrintf("Windows "); doCR();
                if (cfg.floors)
                {
                    doCR();
                    mPrintf("-- System in floor mode, no effect."); doCR();
                    return;
                }
                listRooms(WINDWS, numMess);
                break;
            case 'X':
                mPrintf("Xcluded Rooms ");
                mPrintf("\n ");
                listRooms(XCLRMS, numMess);
                break;
            case '#':
                mPrintf("Number of %s ", cfg.msgs_nym);
                done    = FALSE;
                numMess = TRUE;
                break;
            default:
                oChar(ich);
                mPrintf("? ");
                if (expert)  break;
            case '?':
                if (letter == '?') oChar('?');
                tutorial("known.mnu");
                break;
        }
    }
    while (!done && moreYet);
}

/************************************************************************/
/*      doLogout() handles T(erminate) command                          */
/************************************************************************/
void doLogout(char expand, char first)
/* char expand; */   /* TRUE to accept following parameters  */
/* char first; */    /* first parameter if TRUE              */
{
    char done = FALSE; /* , verbose = FALSE; */
    char ich;

    verbose = FALSE;

    if (expand)   first = '\0';

    mPrintf("Terminate ");

    if (first == 'q')
        verbose = 1;
    
    while(!done && CARRIER)
    {
        done = TRUE;

        switch (toupper( first ? (int)first : (int)(ich=(char)iCharNE())))
        {
        case '?':
            oChar('?');
            mPrintf("\n Logout options:\n \n ");
    
            mPrintf("3Q0>uit-also\n " );
            mPrintf("3S0>tay\n "      );
            mPrintf("3V0>erbose\n "   );
            mPrintf("3?0> -- this menu\n "  );
            break;
        
        case 'Y':
        case 'Q':
            mPrintf("Quit-also\n ");
            if (!expand)  
            {
                if (!getYesNo(confirm, 0))   break;
            }
            if (!CARRIER) break;
            terminate( /* hangUp == */ TRUE);

            /* dont exit when doing .TS */
            if (slv_door)  ExitToMsdos = TRUE; 

            break;
            
        case 'S':
            mPrintf("Stay\n ");
            terminate( /* hangUp == */ /* FALSE */ (uchar)(!modStat && 
!slv_door));
            break;
            
        case 'V':
            mPrintf("Verbose ");
            verbose = 2;
            done = FALSE;
            break;
        
        default:
            oChar(ich);
            if (expert)
                mPrintf("? ");
            else
                mPrintf("? for help");
            break;
        }
        first = '\0';
    }
}

/************************************************************************/
/*      doRead() handles R(ead) command                                 */
/************************************************************************/
void doRead(char moreYet, char first)
/* char moreYet; */          /* TRUE to accept following parameters */
/* char first;   */          /* first parameter if TRUE             */
{
    char abort, done, letter;
    char whichMess, revOrder; /* , verbose;  */
    char ich;
    char textverbose = FALSE;
    reverse = FALSE; /* for .RRI, .RRD */

    if (moreYet)   first = '\0';

    mPrintf("Read ");

    abort      = FALSE;
    revOrder   = FALSE;
    verbose    = logBuf.VERBOSE;   /* FALSE;  */
    whichMess  = NEWoNLY;
    memset(&mf, 0, sizeof(mf));


    if  (!loggedIn && !cfg.unlogReadOk) 
    {
        mPrintf("\n  --Must log in to read.\n ");
        return;
    }

    do
    {
        done    = TRUE;

   letter = (char)(toupper(first ? (int)first : (int)(ich = (char)iCharNE())));

        switch (letter)
        {
        case '\n':
        case '\r':
        case 'M':
            mPrintf("Messages"); /* doCR(); */
            moreYet = FALSE;
            break;
        
        case 'K':
        case '*':
            mPrintf("Keyword Search ");
            mf.mfSearch[0] = TRUE;
            done           = FALSE;
            break;
        
        case 'B':
            mPrintf("By-User ");
            mf.mfUser[0] = TRUE;
            done         = FALSE;
            break;
        
        case 'C':
            mPrintf("Configuration ");
            showconfig(&logBuf);
            abort     = TRUE;
            break;
        

        case 'D':
            mPrintf("Directory ");
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }
            else readdirectory();
            abort      = TRUE;
            reverse = FALSE; /* for .RRI, .RRD */
            break;

        case 'I':
            mPrintf("Info file ");
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }
            else readinfofile(moreYet);
            abort      = TRUE;
            reverse = FALSE; /* for .RRI, .RRD */
            break;

        case 'L':
            mPrintf("Limited-access ");
            mf.mfLim     = TRUE;
            done         = FALSE;
            break;


#ifdef GOODBYE
            doBS();
            doCR();
            doCR();
            mPrintf("  -- Command now avalible from '.List' menu.");
            doCR();
            return;
#endif
        
        case 'E':
            mPrintf("Exclusive ");
            mf.mfMai     = TRUE;
            done         = FALSE;
            break;
        
        case 'F':
            mPrintf("Forward ");
            revOrder     = FALSE;
            whichMess    = OLDaNDnEW;
            done         = FALSE;
            break;
        
        case 'G':
            mPrintf("Group-only ");
            mf.mfLim     = TRUE;
            done         = FALSE;
            break;
        
        case 'H':
            mPrintf("Hallways ");
            reverse = FALSE;
            readhalls();
            abort     = TRUE;
            break;
        
        case 'N':
            mPrintf("New ");
            whichMess  = NEWoNLY;
            done       = FALSE;
            break;
        
        case 'O':
            mPrintf("Old ");
            revOrder   = TRUE;
            whichMess  = OLDoNLY;
            done       = FALSE;
            break;
        
        case 'P':
            mPrintf("Public ");
            mf.mfPub     = TRUE;
            done         = FALSE;
            break;
        
        case 'R':
            mPrintf("Reverse ");
            revOrder   = TRUE;
            reverse    = TRUE; /* for .RRI, .RRD */
            whichMess  = OLDaNDnEW;
            done       = FALSE;
            break;
        
        case 'S':
            mPrintf("Status\n ");
            systat();
            abort         = TRUE;
            break;

        case 'T':
            mPrintf("Textfile ");
            verbose = textverbose;
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
                return;
            }

            /* handle uponly flag! */
            if ( roomTab[thisRoom].rtflags.UPONLY && !pgroupseesroom() )
            {
                doCR();
                mPrintf(" --Room is upload only.");
                doCR();
                return;
            }
            readtextfile();
            abort         = TRUE;
            break;
        
        case 'U':
            mPrintf("Userlog ");
            Readlog(revOrder);
            abort         = TRUE;
            break;
            
        case 'V':
            mPrintf("Verbose ");
            verbose       = TRUE;
            textverbose   = TRUE;
            done          = FALSE;
            break;

        case 'W':
            mPrintf("WC-protocol download ");
            if (!roomBuf.rbflags.MSDOSDIR)
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
                return;
            }

            /* handle uponly flag! */
            if ( roomTab[thisRoom].rtflags.UPONLY && !pgroupseesroom() )
            {
                doCR();
                mPrintf(" --Room is upload only.");
                doCR();
                return;
            }
            readwc();
            /* doCR(); */
            abort    = TRUE;
            break;


        case 'Z':
            mPrintf("ZIP-file ");
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }else readzip();
            abort     = TRUE;
            break;

#ifdef GOODBYE    /* readlzh() is buggy and will crash the bbs */
        case '!':
            mPrintf("LZH-file ");
            if ( !roomBuf.rbflags.MSDOSDIR )
            {
                if (expert) mPrintf("? ");
                else        mPrintf("\n Not a directory room.");
            }else readlzh();
            abort     = TRUE;
            break;
#endif 
        
        default:
            oChar(ich);
            mPrintf("? ");
            abort    = TRUE;
            if(expert) break;
        case '?':
            if (letter == '?') oChar('?');
            tutorial("readopt.mnu");
            abort    = TRUE;
            break;
        }
        first = '\0';

    }
    while (!done && moreYet && !abort);

    if (abort) return;


    /* no by-user read in anon rooms! */
    if ( roomTab[thisRoom].rtflags.ANON && mf.mfUser[0])
    {
        doCR();
        doCR();
        mPrintf("  --Not in Anon rooms."); doCR();
        return;
    }
    showMessages(whichMess, revOrder);
}


/************************************************************************/
/*      doXpert                                                         */
/************************************************************************/
void doXpert()
{
    mPrintf("Xpert %s", (expert) ? "Off" : "On");
    doCR();
    expert = (char)(!expert);
}

/************************************************************************/
/*      doverbose                                                       */
/************************************************************************/
void doverbose()
{
    mPrintf("Auto Verbose %s", (logBuf.VERBOSE) ? "Off" : "On");
    doCR();
    logBuf.VERBOSE = (char)(!logBuf.VERBOSE);
}

/************************************************************************/
/*     doNext() handles the '+' for next room                           */
/************************************************************************/
void doNext()
{
    mPrintf("Next Room: ");
    stepRoom(1);
}

/************************************************************************/
/*     doPrevious() handles the '-' for previous room                   */
/************************************************************************/
void doPrevious()
{
    mPrintf("Previous Room: ");
    stepRoom(0);
}

/************************************************************************/
/*     doNextHall() handles the '>' for next hall                       */
/************************************************************************/
void doNextHall()
{
    mPrintf("Next Hall: ");
    stephall(1);
}

/************************************************************************/
/*     doPreviousHall() handles the '<' for previous hall               */
/************************************************************************/
void doPreviousHall()
{
    mPrintf("Previous Hall: ");
    stephall(0);
}

/************************************************************************/
/*      exclude() handles X>clude room,  toggles the bit                */
/************************************************************************/
void doSmallChat(void)
{
    char str[256];
    
    oChar(';');
    
    getString("", str, 255, 1, ECHO, "");
    normalizeString(str);

    /* to keep strcmpi from crashing */
    str[5] = 0;
    
    if (strcmpi(str, "xyzzy") == SAMESTRING)
    {
        if (
               (sysop || aide)
           &&  groupseesroom(AIDEROOM) 
           )
        {
            oldroom   = thisRoom;
            ug_hall   = thisHall;
            /* ug_lvisit = logBuf.lbroom[thisRoom].lvisit; */
            ug_newpointer = logBuf.newpointer[thisRoom];
            ug_bypass = logBuf.lbroom[thisRoom].bypass;
            ug_new    = talleyBuf->room[thisRoom].new;
    
            logBuf.lbroom[thisRoom].lbgen    = roomBuf.rbgen;
           /* logBuf.lbroom[thisRoom].lvisit   = 0; */
           logBuf.newpointer[thisRoom] = cfg.newest;


            talleyBuf->room[thisRoom].hasmail  = 0;
         /* logBuf.lbroom[thisRoom].mail     = 0; */
        
            /* zero new count in talleybuffer */
            talleyBuf->room[thisRoom].new  = 0;
            
            getRoom(AIDEROOM);
            checkdir();
            thisHall = 1 /* maintenance */;
            
            dumpRoom();
        }
        else
        {
            doCR();
            mPrintf("Nothing happens"); doCR();
        }   
    }
}

