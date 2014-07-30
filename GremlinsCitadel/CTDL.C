/************************************************************************/
/*                              ctdl.c                                  */
/*              Command-interpreter code for Citadel                    */
/************************************************************************/

#define MAIN
#define EXTERN 

#include "ctdl.h"

unsigned _stklen    = 1024*12;          /* set up our stackspace */
unsigned _ovrbuffer = 0x2E00 / 0x0F;    /* 0x2D7A last check, config.c */

/*  int _far _nullcheck(void); */  /* Undocumented function for MSC 6.0 */

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      doRegular()             fanout for above commands               */
/*      getCommand()            prints prompt and gets command char     */
/*      main()                  has the central menu code               */
/************************************************************************/

/************************************************************************/
/*              External function definitions for CTDL.C                */
/************************************************************************/

/*----------------------------------------------------------------------*/
/*      logo()  prints out system logo at startup                       */
/*----------------------------------------------------------------------*/
void logo()
{
    int i;

    cls();

    getScreenSize((&conCols), (&conRows));
 
    for (i = 0; i < ((conRows/2) -1); i++) doccr(); 

    cCPrintf("%s Version %s (%s)", programName, version, compilerName);  doccr();
    cCPrintf("%s", copyright);  doccr();  doccr();
}

/************************************************************************/
/*      greeting() gives system-entry blurb etc                         */
/************************************************************************/
void greeting()
{
    char dtstr[80];
    ulong messages;

    verbose = FALSE;
    if (loggedIn) terminate(FALSE);

    echo  =  BOTH;
    setio(whichIO, echo, outFlag);


    cleargroupgen();
    setdefaultconfig();
    initroomgen();
/*    cleargroupgen();  */
    
    if (cfg.accounting) unlogthisAccount();
    
    pause(10);
    
    cls();
    
    doccr();
    
    update25();

    if (modStat)
    {
       pause(cfg.connectwait * 100);
       Mflush();
    }

    /* set terminal */
    autoansi();

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    if (modStat || debug) 
    {
        hello();
        doCR();
    }

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    
    /* doCR(); */
    mPrintf("Welcome to %s, %s, %s", 
            cfg.nodeTitle, cfg.nodeRegion, cfg.nodeContry);
    doCR();
    mPrintf("%s Version %s (%s)", programName, version, compilerName);
    doCR();

     mPrintf("%s", copyright);


    doCR();  
    doCR();

    strftime(dtstr, 79, cfg.vdatestamp, 0l);
    mPrintf("%s", dtstr);


    if (!cfg.forcelogin)
    {
        mPrintf("\n H for Help");
        mPrintf("\n ? for Menu");
        mPrintf("\n L to Login");
    }


    getRoom(LOBBY);
    checkdir();


    messages = talleyBuf->room[thisRoom].messages;

    doCR();

    mPrintf("  %lu %s.", messages, 
        (messages == 1) ? cfg.msg_nym: cfg.msgs_nym);

    doCR();

    Mflush();
}


/* -------------------------------------------------------------------- */
/*  main()          Call init, general system loop, then shutdown       */
/* -------------------------------------------------------------------- */
void main(int argc, char *argv[])
{
    char c, x = FALSE; /* , floppy = FALSE; */
    int  trys = 0;
    char modStatMain = FALSE; /* newCarrier && JustLostCarrier */
    char prompt = TRUE;    
    char didterminate = FALSE; /* nodes log in only on the first log-in */
    char oldIO;
    char auto_login = FALSE; /* this is so it will go to login when run in door mode */

    tzset();
    
    parseArgs(argc, argv);

    initCitadel();

    /* this is my main citadel loop */

    /* Set system to a known state */
    echo      = BOTH;                
    whichIO   = CONSOLE;
    outFlag   = IMPERVIOUS;
    modStat   = FALSE;
    
    setio(whichIO, echo, outFlag);

    if (login_pw || login_user || (slv_door && cfg.forcelogin))
        auto_login = TRUE;

    if (!auto_login)
    {
        Mflush();
        greeting();
    }

    started = time(NULL);

    if (slv_net)
    {
        /* cls(); */
        
        doccr();
        /* ansiattr = cfg.cattr; */

        cPrintf("Network: with \"%s\"", slv_node);

        /* ansiattr = cfg.attr; */
        
        if (net_callout(slv_node))
        {
            did_net(slv_node);
        }

        ExitToMsdos = TRUE;
    }

    if (slv_door) /* set according to carrier */
    {
        /* set baud rate even if carrier not present */
        if (slv_baud != UINT_ERROR)
            baud(slv_baud);
        else
            baud(cfg.initbaud);

        if (gotCarrier())
        {
#ifdef GOODBYE
            if (slv_baud != -1)
                baud(slv_baud);
            else
                baud(cfg.initbaud);
#endif
            
            carrdetect();
                
            whichIO     = MODEM;
            setio(whichIO, echo, outFlag);

        }
        else
        {
            whichIO   = CONSOLE;
            setio(whichIO, echo, outFlag);

        }
    }

    setdefaultTerm(2); /* Ansi-CLR */

    
    while (!ExitToMsdos)
    {
#ifdef GOODBYE
        if (_nullcheck()) /* undocumented MSC function */
            cPrintf("\n");
#endif

        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);

        update25();

        /* Carrier Detect */
        if (!loggedIn)
        {
        if ((modStat && !modStatMain) || auto_login)
        {
            setdefaultTerm(0);  /* TTY */

            modStatMain = TRUE;
            if (modStat) 
            {
                whichIO = MODEM;
                setio(whichIO, echo, outFlag);
            }
            Mflush();
            greeting();

            /* doCR(); */

            if (cfg.forcelogin || auto_login)
            {
                do
                {
                    doLogin(2); /* login modified to handle command line login */
                }
                while (!loggedIn && CARRIER && trys++<3);
                trys = 0;

                if (!loggedIn)
                {
                    if (!onConsole)
                        Hangup();
                    else
                    { whichIO = MODEM; /* simulated hangup */
                       setio(whichIO, echo, outFlag);
                    }
                }
            }
            auto_login = FALSE;
            started = time(NULL);
        }
        }

        if (logBuf.lbflags.NODE && loggedIn && modStat && !didterminate)
        {
             dowhat = NETWORKING;
             callout = TRUE;
             net_slave();
             callout = FALSE;
             dowhat = DUNO;

             pause(200);
        
             carrloss();
      
             cfg.callno++;
             verbose = FALSE;
             terminate(FALSE);

             started = time(NULL);
             setdefaultTerm(2); /* Ansi-CLR */
        }

        /* Carrier Loss */
        if (!CARRIER)
        {
            didterminate = FALSE; /* node can log in now */

            if (slv_door)  ExitToMsdos = TRUE; 

            carrloss();
            if (loggedIn)
            {
                verbose = FALSE;
                terminate(FALSE);
            }
            modStatMain = FALSE;

            echo      = BOTH;                
            whichIO   = CONSOLE;
            outFlag   = IMPERVIOUS;
            modStat   = FALSE;
    
            setio(whichIO, echo, outFlag);

            if (!gotCarrier()   /* slv_door */)
            {
                Initport();
            }

            started = time(NULL);
            setdefaultTerm(2); /* Ansi-CLR */
        }


        if (logBuf.VERIFIED)   /* I'm not sure if this really belongs here */
        {
            verbose = FALSE;
            terminate(TRUE);
            started = time(NULL);
            setdefaultTerm(2); /* Ansi-CLR */
        }

        x       = getCommand(&c, prompt);
        prompt = TRUE;

        if (c) started = time(NULL); 

        if ((toupper(c) == 'T'))
        {
            didterminate = TRUE; /* node cannot log in */
        }


        /* do greeting from console login */
        if ((toupper(c) == 'L') && !x && !loggedIn && !modStat)
        {
            greeting();
            /* doCR(); */
        }

        if (chatkey) 
        {
            chat();
            started = time(NULL);  
        }

        if (eventkey && !modStat && !loggedIn)
        { 
            do_cron(CRON_TIMEOUT);
            eventkey = FALSE;
            started = time(NULL);  
            setdefaultTerm(2); /* Ansi-CLR */

        }

        if ( ((int)((time(NULL) - started)/(time_t)60) >= cfg.idle)
          && !modStat && !loggedIn )
        {
            if (!do_cron(CRON_TIMEOUT))
            {
                /* started = time(NULL); */
                prompt = FALSE;
            }
            started = time(NULL);  
            setdefaultTerm(2); /* Ansi-CLR */
        }

        if (sysopkey)
        {
           oldIO = whichIO;
           whichIO = CONSOLE;
           setio(whichIO, echo, outFlag);

           doSysop();
           if (modStat)
           {
               whichIO = oldIO;
               setio(whichIO, echo, outFlag);
           }
           started = time(NULL);
           if (!modStat) setdefaultTerm(2); /* Ansi-CLR */
        }

        doRegular(x, c);

        /* unsuccessful login */
        if ((toupper(c) == 'L') && !x && !loggedIn && !modStat)
        {
            whichIO = MODEM; /* simulated hangup */
            setio(whichIO, echo, outFlag);
        }

        if (c) started = time(NULL); 


        if ((sysReq == TRUE) && !loggedIn && !modStat && !ExitToMsdos)
        {
            sysReq=FALSE;
            if (cfg.offhook)
            {
                offhook();
            }
            else
            {
                drop_dtr();
            }
            ringSystemREQ();
            started = time(NULL);
        }
    }
    /* return_code = 1; */ /* might as well set it to something */
    exitcitadel();
}

/* -------------------------------------------------------------------- */
/*  doRegular()         High level command menu.                        */
/* -------------------------------------------------------------------- */
char doRegular(char x, char c)
{
    char toReturn;

    toReturn = FALSE;
    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    
    switch (toupper(c))
    {

    case 'S':
        if (sysop && x)
        {
            mPrintf("\bSysop Menu");
            doCR();
            doSysop();
        } 
        else 
        {
            /* oChar(c); */
            toReturn=TRUE;
        }
        break;

#ifdef GOODBYE
    case 'A':
        if (aide && x)
        {
            doAide(x, 'E');
        } else {
            doEnter(FALSE, 'a');
        }
        break;
#endif

    case 'A':
        if (aide)
        {
            doAide(x, 'E');
        } else 
        {
            /* oChar(c); */
            toReturn=TRUE;
        }
        break;

    case 'C': doChat(x, '\0');                    break;
    case 'D': doDownload(x);                      break;
    case 'E': doEnter(x, 'm');                    break;
    case 'F': doRead(x, 'f');                     break;
    case 'G': doGoto(x, FALSE);                   break;
    case 'H': doHelp(x);                          break;
    case 'I': doIntro();                          break;
    case 'J': unGotoRoom();                       break;
    case 'K': doKnown(x, 'r');                    break;
/*  case 'L': doList(x, 'i');                     break; */
    case 'L': doLogin( x      );                  break;    

    case 'M': doEnter(FALSE, 'e');                break;
    case 'N': doRead(x, 'n');                     break;
    case 'O': doRead(x, 'o');                     break;
    case 'R': doRead(x, 'r');                     break;
    case 'B': doGoto(x, TRUE);                    break;
    case 'T': doLogout(x, 'q');                   break;
    case 'U': doUpload(x);                        break;
    case 'Q': mPrintf("Quack!\n");                break;
    case 'P': mPrintf("Poop!\n");                 break;
    case 'X':
        if (!x)
        {
            doEnter( x, 'x' );
        }else{
            doXpert();
        }
        break;
    case 'V':
        if (x) 
            doverbose();
        break;
    case '=':
    case '+': doNext()         ;                    break;
    case '-': doPrevious()     ;                    break;

    case ']':
    case '>': doNextHall()     ;                    break;
    case '[':
    case '<': doPreviousHall() ;                    break;

    case ';': doSmallChat()    ;                    break;
    
    case '?':
        oChar('?');
        if (!x)
        {
            tutorial("mainopt.mnu");
        }
        else
        {
            tutorial("maindot.mnu");
        }
        break;
    
    case 0:
        break;  /* irrelevant value */
        
    default:
        toReturn = (char)!execDoor(c);
        break;
    }

    if (toReturn)
    {
        oChar(c);
        if (!expert)  mPrintf("\n '?' for menu, 'H' for help.\n \n" );
        else          mPrintf(" ?\n \n" );
    }
    
    return toReturn;
}

/* -------------------------------------------------------------------- */
/*  getCommand()    prints menu prompt and gets command char            */
/*      Returns: char via parameter and expand flag as value  --        */
/*               i.e., TRUE if parameters follow else FALSE.            */
/* -------------------------------------------------------------------- */
char getCommand(char *c, char prompt)
{
    char expand;

    /*
     * Carrier loss!
     */
    if (!CARRIER)
    {
        *c = 0;
        return 0;
    }
    
    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    /* update user's balance */
    if( cfg.accounting && !logBuf.lbflags.NOACCOUNT )
    {
        updatebalance();
    }

    if (prompt)
    {
#ifdef DRAGON
        dragonAct();    /* user abuse rutine :-) */
#endif

        if (cfg.borders)
        {
            doBorder();
        }

        givePrompt();
    }

    dowhat = MAINMENU;

        *c = (char)iCharNE();

#ifdef GOODBYE
    do
    {
        *c = (char)iCharNE();
    }
    while( ((char)toupper(*c)) == 'P'); 
#endif

    expand  = (char)
              ( (*c == ' ') || (*c == '.') || (*c == ',') || (*c == '/') );

    if (expand)
    {
        mPrintf("%c", *c);
        *c = (char)iCharNE();
    }
    dowhat = DUNO;

    return expand;
}

/* -------------------------------------------------------------------- */
/*  parseArgs()     sets global flags baised on command line            */
/* -------------------------------------------------------------------- */
void parseArgs(int argc, char *argv[])
{
   int i;   /* , i2; */ 
  /* long l; */
    
    cfg.bios = 1;
    cfg.attr = 7;   /* logo gets white letters */
    
    for(i = 1; i < argc; i++)
    {
        if (   argv[i][0] == '/'
            || argv[i][0] == '-')
        {
            switch(toupper((int)argv[i][1]))
            {

            case 'X':
                batchmode = TRUE;
                break;

            case 'D':
                cfg.bios = 0;
                break;

#ifdef GOODBYE                
            case 'B':
                l = atol(argv[i]+2);
                for (i2 = 0; i2<7; i2++)
                {
                    if (l == bauds[i2])
                        slv_baud = i2;
                }
                /* fall into */
#endif

            case 'B':
                if (argv[i][2] == '\0')
                    slv_baud = digitbaud(atoi(argv[++i]));
                else
                    slv_baud = digitbaud(atoi(&(argv[i][2])));

                /* fall into */

        /*      break;   */


#ifdef GOODBYE                
            case 'S':
                slv_slave = TRUE;
                start_p = FALSE;
                break;
            
            case 'L':
                slv_local = TRUE;
                slv_slave = TRUE;
                start_p = FALSE;
                break;
#endif
            case 'S':
            case 'L':
            case 'A':
                slv_door = TRUE;
                break;
                
            case 'C':
                unlink("etc.dat");

                unlink("log.tab");
                unlink("msg.tab");
                unlink("room.tab");
                unlink("cron.tab");

                reconfig = TRUE;
                break;

            case 'E':
                readconfigcit = TRUE;
                break;

            case 'N':
                if (toupper(argv[i][2]) == 'B')
                {
                    cfg.noBells = TRUE;
                }
                if (toupper(argv[i][2]) == 'C')
                {
                    cfg.noChat = TRUE;
                }
                break;

            case 'M':
                conMode = atoi(argv[i]+2);
                break;
#ifdef GOODBYE                
            case '*':
                start_p = FALSE;
                break;
#endif                
            case '!':
                slv_net = TRUE;
                /* start_p = FALSE; */
                if (argv[i][2])
                    strcpy(slv_node, argv[i]+2);
                else
                {
                    i++;
                    strcpy(slv_node, argv[i]);
                }
                break;

            case 'P':               /* log in user */
                login_pw = TRUE;
                if (argv[i][2])
                    strcpy(cmd_login, argv[i]+2);
                else
                {
                    i++;
                    strcpy(cmd_login, argv[i]);
                }
                break;

            case 'U':               /* log in user with password */
                login_user = TRUE;
                if (argv[i][2])
                    strcpy(cmd_login, argv[i]+2);
                else
                {
                    i++;
                    strcpy(cmd_login, argv[i]);
                }
                break;

            case 'V':               /* debug (verbose) mode */
                debug = TRUE;
                break;
               
            default:
                printf("\nUnknown command line switch '%s'.\n", argv[i]);
            case  '?':    
            case  'H':    
                usage();
                exit(200);
            }
        }
    }
}

