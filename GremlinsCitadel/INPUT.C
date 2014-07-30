/* -------------------------------------------------------------------- */
/*  INPUT.C                  Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  This file contains the input functions                              */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  getNormStr()    gets a string and normalizes it. No default.        */
/*  getNumber()     Get a number in range (top, bottom)                 */
/*  getString()     gets a string from user w/ prompt & default, ext.   */
/*  getYesNo()      Gets a yes/no/abort or the default                  */
/*  BBSCharReady()  Returns if char is avalible from modem or con       */
/*  iChar()         Get a character from user. This also indicated      */
/*                  timeout, carrierdetect, and a host of other things  */
/*  setio()         set io flags according to whicio, echo and outflag  */
/* -------------------------------------------------------------------- */

#ifdef GOODBYE
int rchar2(unsigned timeout, unsigned char *ch)
{
    long t;

    t = time(NULL);

    while ((unsigned int)(time(NULL) - t) < timeout)
    {
        if (!CARRSTATRS()) 
            return(-1);

        if (STATRS())
        {
            *ch = (char)GETRS();
            return(1);
        }

        if (STATCON())
        {
            *ch = (char)GETCON();
            return(1);
        }
    }
    return(-1);
}
#endif

/* -------------------------------------------------------------------- */
/*  getNormStr()    gets a string and normalizes it. No default.        */
/* -------------------------------------------------------------------- */
void getNormStr(char *prompt, char *s, int size, char doEcho)
{
    getString(prompt, s, size, FALSE, doEcho, "");
    normalizeString(s);
}

/* -------------------------------------------------------------------- */
/*  getNumber()     Get a number in range (top, bottom)                 */
/* -------------------------------------------------------------------- */
long getNumber(char *prompt, long bottom, long top, long dfaultnum)
{
    long try;
    label numstring;
    label buffer;
    char *dfault;

    dfault = ltoa(dfaultnum, buffer, 10);

    if (dfaultnum == -1l) dfault[0] = '\0';

    do
    {
        getString(prompt, numstring, NAMESIZE, FALSE, ECHO, dfault);
        try     = atol(numstring);
        if (try < bottom)  mPrintf("Sorry, must be at least %ld\n", bottom);
        if (try > top   )  mPrintf("Sorry, must be no more than %ld\n", top);
    }
    while ((try < bottom ||  try > top) && CARRIER);
    return  (long) try;
}

/* -------------------------------------------------------------------- */
/*  getString()     gets a string from user w/ prompt & default, ext.   */
/* -------------------------------------------------------------------- */
void getString(char *prompt, char *buf, int lim, char QuestIsSpecial, 
               char doEcho, char *dfault)
/* char *prompt; */          /* Enter PROMPT */
/* char *buf; */             /* Where to put it */
/* char doEcho; */           /* To echo, or not to echo, that is the question */
/* int  lim; */              /* max # chars to read */
/* char QuestIsSpecial; */   /* Return immediately on '?' input? */
/* char *dfault;*/           /* Default for the lazy. */
{
    char c, d, oldEcho, errors = 0;
    int  i;

    if (!CARRIER)
    {
        buf[0] = '\0';
        return;
    }
    
    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    if ( strlen(prompt) )
    {
        doCR();

        if (strlen(dfault))
        {
            sprintf(gprompt, "2Enter %s [%s]:0 ", prompt, dfault);  
        }
        else
        {
            sprintf(gprompt, "2Enter %s:0 ", prompt);
        }

        mPrintf("%s", gprompt);

        dowhat = PROMPT;    
    }
    
    /* setio(whichIO, echo, outFlag); */

    oldEcho = echo;
    echo     = NEITHER;
    setio(whichIO, echo, outFlag); 

    
    if (!doEcho)
    {
        if (!cfg.nopwecho)
        {
            echoChar = 1;
        }
        else if (cfg.nopwecho == 1)
        {
            echoChar = '\0';
        }
        else 
        {
            echoChar = cfg.nopwecho;
        }
    }

    i   = 0;

    for (i =  0, c = (char)iChar(); 
         c != 10 /* NEWLINE */ && CARRIER;
         c = (char)iChar()
        )
    {
        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);

        /*
         * handle delete chars: 
         */
        if (c == '\b')
        {
            if (i != 0)
            {

                echo = oldEcho;
                setio(whichIO, echo, outFlag);

                doBS();

                echo = NEITHER;
                setio(whichIO, echo, outFlag);

                i--;

                if ( (echoChar >= '0') && (echoChar <= '9'))
                {
                    echoChar--;
                    if (echoChar < '0') echoChar = '9';
                }
            }
            else 
            {
                echo = oldEcho;
                setio(whichIO, echo, outFlag);

                oChar(7 /* bell */);

                echo = NEITHER;
                setio(whichIO, echo, outFlag);

            }
        }
        else
        if (c == 0)
        {
            if (CARRIER)
            {
                continue;
            }
            else    
            {
                i = 0;
                break;
            }
        }
        else
        {
            if (c == CTRL_A && (i < lim-1) && cfg.colors)
            {
                /* CTRL-A>nsi   */
               /*  d = (char)iChar(); */
                d = (char)iCharNE();

                if (
                      (d >= '0' && d <= '4') 
                   || (d >= 'a' && d <= 'h')
                   || (d >= 'A' && d <= 'H')
                   )
                {
                    echo = oldEcho;
                    setio(whichIO, echo, outFlag);
 
                    termCap(d);
                    echo = NEITHER;
                    setio(whichIO, echo, outFlag);

                    buf[i++]   = 0x01;
                    buf[i++]   = d;
                }
                else 
                {
                    echo = oldEcho;
                    setio(whichIO, echo, outFlag);
 
                   oChar(7);

                    echo = NEITHER;
                    setio(whichIO, echo, outFlag);

                }
            }
            else

            if (i < lim && c != '\t')
            {
                if ( (echoChar >= '0') && (echoChar <= '9'))
                {
                    echoChar++;
                    if (echoChar > '9') echoChar = '0';
                }

                buf[i] = c;

                if (doEcho || cfg.nopwecho == 0)
                {
                    echo = oldEcho;
                    setio(whichIO, echo, outFlag);

                    oChar(c);

                    echo = NEITHER;
                    setio(whichIO, echo, outFlag);

                }
                else
                {
                    echo = oldEcho;
                    setio(whichIO, echo, outFlag);

                    oChar(echoChar);   

                    echo = NEITHER;
                    setio(whichIO, echo, outFlag);

                }

                i++;
            }
            else
            {
                echo = oldEcho;
                setio(whichIO, echo, outFlag);

                oChar(7 /* bell */);

                echo = NEITHER;
                setio(whichIO, echo, outFlag);

                errors++;
                if (errors > 15 && !onConsole)
                {
                    drop_dtr();
                }
             }
        }

        /* kludge to return immediately on single '?': */
        if (QuestIsSpecial && *buf == '?')  
        {
            echo = oldEcho;
            setio(whichIO, echo, outFlag);

            doCR();

            echo = NEITHER;
            setio(whichIO, echo, outFlag);

            break;
        }
    }

    echo     = oldEcho;
    setio(whichIO, echo, outFlag);

    buf[i]   = '\0';
    echoChar = '\0';

    if ( strlen(dfault) && !strlen(buf) ) strcpy(buf,dfault);

    dowhat = DUNO;

    doCR();
}

/* -------------------------------------------------------------------- */
/*  getYesNo()      Gets a yes/no/abort or the default                  */
/* -------------------------------------------------------------------- */
int getYesNo(char *prompt, char dfault)
{
    int  toReturn;
    char  c;
    char oldEcho;
    
    if (!CARRIER)
    {
        switch(dfault)
        {
        case 0:
        case 3:
            return FALSE;
            
        case 1:
        case 4:
            return TRUE;
            
        default:
            return 2;
        }
    }

    doCR();
    toReturn = ERROR;

    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    sprintf(gprompt, "2%s? ", prompt);

    switch(dfault)
    {
    case 0: strcat(gprompt, "(Yes/No)[No]");           break;
    case 1: strcat(gprompt, "(Yes/No)[Yes]");          break;
    case 2: strcat(gprompt, "(Yes/No/Abort)[Abort]");  break;
    case 3: strcat(gprompt, "(Yes/No/Abort)[No]");     break;
    case 4: strcat(gprompt, "(Yes/No/Abort)[Yes]");    break;
    default:                   
            strcat(gprompt, "(Yes/No)[No]");
            dfault = 0;
            break;
    }
    
    strcat(gprompt, ":0 ");

    mPrintf(gprompt);

    dowhat = PROMPT;    
    
    do {
        oldEcho = echo;
        echo    = NEITHER;
        setio(whichIO, echo, outFlag);

        c       = (char)iChar();
        echo    = oldEcho;
        setio(whichIO, echo, outFlag);
 
        if ( (c == '\n') || (c == '\r') )
        {
            if (dfault == 1 || dfault == 4)  c = 'Y';
            if (dfault == 0 || dfault == 3)  c = 'N';
            if (dfault == 2)                 c = 'A';
        }

        switch (toupper(c))
        {
            case 'Y': mPrintf("Yes"  ); doCR(); toReturn   = 1;  break;
            case 'N': mPrintf("No"   ); doCR(); toReturn   = 0;  break;
            case 'A': 
                if (dfault > 1) 
                {
                    mPrintf("Abort");  doCR();

                    toReturn   = 2; 
                }
                break;
        }
    } while( toReturn == ERROR && CARRIER );

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    dowhat = DUNO;
    return   toReturn;
}

/* -------------------------------------------------------------------- */
/*  BBSCharReady()  Returns if char is avalible from modem or con       */
/* -------------------------------------------------------------------- */
int BBSCharReady(void)
{
    return (   (gotCarrier() && (whichIO == MODEM) && MIReady()) 
            || KBReady()  
           );
}

/* -------------------------------------------------------------------- */
/*  setio()         set io flags according to whicio, echo and outflag  */
/* -------------------------------------------------------------------- */
void setio(char whichio, char echo, char outflag)
{
    if ( !(outflag == OUTOK || outflag == IMPERVIOUS || outflag == NOSTOP))
    {
        modem   = FALSE;
        console = FALSE;
    }
    else if (echo == BOTH)
    {
        modem   = TRUE;
        console = TRUE;
    }  
    else if (echo == CALLER)
    {
        if (whichio == MODEM)
        {
           modem   = TRUE;
           console = FALSE;
        } 
        else if (whichio == CONSOLE)
        {
           modem   = FALSE;
           console = TRUE;
        }
    }
    else if (echo == NEITHER)
    {
        modem   = FALSE; 
        console = FALSE; 
    }

    if (!gotCarrier() || !modStat)  modem = FALSE;
}

/* -------------------------------------------------------------------- */
/*  iCharNE()   don't echo it..                                         */
/* -------------------------------------------------------------------- */
int iCharNE(void)
{
    char  c;
    char oldEcho;
    
    oldEcho = echo;
    echo    = NEITHER;
    setio(whichIO, echo, outFlag);
    c       = (char)iChar();
    echo    = oldEcho;
    setio(whichIO, echo, outFlag);

    return c;
}

/* -------------------------------------------------------------------- */
/*  iChar()         Get a character from user. This also indicated      */
/*                  timeout, carrierdetect, and a host of other things  */
/* -------------------------------------------------------------------- */
int iChar(void)
{
    char c = 0;
    long timer, update = 0L; 
    static char twirlpos = 0;
/*  char twirl[4] = "/|\\-"; */
    char twirl[] = "/-\\|"; 
/*  char subliminal[] = "Run Gremcit!"; */
    char oldEcho;

    oldEcho = echo;
    echo = BOTH; 
    setio(whichIO, echo, outFlag);

    sysopkey = FALSE; /* go into sysop mode from main()? */
    eventkey = FALSE; /* for an event? */

    if (keyboard_timer == 0L) time(&keyboard_timer); 

    time(&timer);

    /* silly */
    if (logBuf.TWIRLY)
    {
        oChar(' ');
    }
    
    for (;;) /* while(TRUE) */
    {
    if (logBuf.TWIRLY && !KBReady() && !(MIReady() && gotCarrier() && modStat 
    && (whichIO == MODEM)  ))
        {
            twirlypause(10);
            twirlpos ++;
            if (twirlpos == 4) twirlpos = 0;
            oChar('\b');
            oChar(twirl[twirlpos]);
        }


        /*
         * Exit to MS-DOS key hit
         */
        if (ExitToMsdos)
        {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            return 0;
        }    


        /*
         * Carrier lost state
         */
        if (!CARRIER)
        {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            return 0;
        }

        /*
         * Carrier gained
         */
        if (!loggedIn && !modStat)
        {
        if (carrier())
        {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            return 0;
        }
        }
        
        /*
         * Keyboard press
         */
        if (KBReady())
        {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            c = (char)ciChar();
            break;
        }

        /*
         *  Get character from modem..
         */
        if (MIReady() && gotCarrier() && modStat)
        {
            /* silly */
            if (logBuf.TWIRLY && (whichIO == MODEM))
            {
                doBS();
            }

            c = (char)getMod();

            if (whichIO == MODEM)
                break;
        }

        /*
         *  Request for sysop menu at main..
         */
        if ((sysopkey || chatkey) && dowhat == MAINMENU)  
        {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            return(0);
        }
        /*
         *  Event key..
         */
         if (eventkey && (dowhat == MAINMENU))
         {
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            return(0);
         }

        /*
         *  Sysop initiated chat..
         */
        if (chatkey && dowhat == PROMPT)
        {

            char oldEcho;

            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            oldEcho = echo;
            echo    = BOTH;
            setio(whichIO, echo, outFlag);

            doCR();
            chat();
            doCR();
            mPrintf("%s", gprompt);

            /* silly */
            if (logBuf.TWIRLY)
            {
                oChar(' ');
            }

            echo   = oldEcho;
            setio(whichIO, echo, outFlag);

            time(&timer);

            chatkey = FALSE;
        }

        /*
         *  Keypress timeout
         */
        if ((sleepkey || systimeout(timer)) && (loggedIn || (modStat || dowhat != MAINMENU))) 
        { 
            sleepkey = FALSE;
            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            echo = BOTH;
            setio(whichIO, echo, outFlag);

            mPrintf("Sleeping? Call again! :-) \n "); 
            if (onConsole)
            {
                whichIO = MODEM;
                setio(whichIO, echo, outFlag);
            }

            Hangup(); 
        } 
        else sleepkey = FALSE;

        /*
         *  Screensaver timeout   
         */
    if ( (!(modStat && cfg.really_fucking_stupid))  && 


                 cfg.screensave && !saver_on && ((int)((time(NULL) -
                 keyboard_timer)/(time_t)60) >= cfg.screensave))
         {


            /* silly */
#ifdef GOODBYE
            if (logBuf.TWIRLY)
            {
                doBS();
            }
#endif

             save_screen();
             setscreen();
             cls();
             saver_on = TRUE;
         }

        /*
         *  Cron timeout   
         */
        if ( ((int)((time(NULL) - started)/(time_t)60) >= cfg.idle)
              && dowhat == MAINMENU && !modStat && !loggedIn )
        {

            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }
            break;

        }


        /*
         *  Update Clock
         */

        if (!modStat && !loggedIn)     /* Update time on status line */
            if (time(NULL) != update)      /* Only update if changed */
            {
                update25();
                time(&update);
            }

    }

    c = tfilter[(uchar)c];

    if (c != 1    /* don't print ^A's          */
        && ((c != 'p' && c != 'P') || dowhat != MAINMENU)
        /* dont print out the P at the main menu... */
       )
      {
            echo = oldEcho;
            setio(whichIO, echo, outFlag);
            echocharacter(c);  
      }

    return(c);
}

