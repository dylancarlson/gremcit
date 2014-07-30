/* -------------------------------------------------------------------- */
/*  CONSOLE.C                Dragon Citadel                   >>IBM<<   */
/* -------------------------------------------------------------------- */
/*  This code handles the console on the IBM PC and compatible          */
/*  computers. This code will need heavy modification for use with      */
/*  other computers.                                                    */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  INPUT:                                                              */
/*  doccr()         Do CR on console, used to not scroll the window     */
/* -------------------------------------------------------------------- */
/*  OUTPUT:                                                             */
/*  fkey()          Deals with function keys from console               */
/*  KBReady()       returns TRUE if a console char is ready             */
/*  outCon()        put a character out to the console                  */
/*  cPrintf()       send formatted output to console                    */
/*  cCPrintf()      send formatted output to console, centered          */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  04/25/90    (PAT)   Created                                         */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data                                                         */
/* -------------------------------------------------------------------- */
BOOL    getkey = FALSE;                 /* key in buffer?               */
extern  char prtf_buff[512];

unsigned int inkey;
char  ungetflag = FALSE;
char  ungetchar;

/* -------------------------------------------------------------------- */
/*  doccr()         Do CR on console, used to not scroll the window     */
/* -------------------------------------------------------------------- */
void doccr(void)
{ 
    unsigned char row, col;

    if (!console) return; 
    if (saver_on) return;

    if (!anyEcho) return;

    readpos( &row, &col);

    if (row == (uchar)(scrollpos + 1) )
    {
        position(0,0);     /* clear screen if we hit our window */
    }

    if (row >= scrollpos)
    {
        scroll( scrollpos, 1, cfg.attr);
        position( scrollpos, 0);
    }
    else 
    {
        col=0;
        row++;
        position(row, col);
    }
}

/* -------------------------------------------------------------------- */
/*  fkey()          Deals with function keys from console               */
/* -------------------------------------------------------------------- */
void fkey(void)
{            
    char key;
    int oldIO; 
    label string;

    key = (inkey >> 8);

    /* key = (char)getch(); */

    /* this handles the keys for dialout mode */
    if (dowhat == DIALOUT)
    {
        dialout_fkey = key;
        return;
    }

    if (strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)    
    if (ConLock == TRUE && key == ALT_L &&
        strcmpi(cfg.f6pass, "disabled") != SAMESTRING)
    {

        /* silly */
        if (logBuf.TWIRLY)
        {
            doBS();
        }

        ConLock = FALSE;

        oldIO = whichIO;
        whichIO = CONSOLE;
        setio(whichIO, echo, outFlag);

     /*   onConsole = TRUE;  */
        update25();
        string[0] = 0;
        getNormStr("System Password", string, NAMESIZE, NO_ECHO);
        if (strcmpi(string, cfg.f6pass) != SAMESTRING)
            ConLock = TRUE;
        whichIO = (BOOL)oldIO;
        setio(whichIO, echo, outFlag);

     /*   onConsole = (BOOL)(whichIO == CONSOLE);  */
        update25();
        givePrompt();
        return;
    }

    if (ConLock && !sysop && strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)
        return;

    switch(key)
    {
    case F1:
        drop_dtr();
        break;

    case F2:
        Initport();
        break;

    case F3:
        sysReq = (BOOL)(!sysReq);
        break;

    case F4:
        ScreenFree();
        anyEcho = (BOOL)(!anyEcho);
        break;

    case F5: 
        if  (whichIO == CONSOLE) whichIO = MODEM;
        else                     whichIO = CONSOLE;

        setio(whichIO, echo, outFlag);

    /*    onConsole = (BOOL)(whichIO == CONSOLE); */
        break;

    case SFT_F6:
        if (!ConLock)
            aide = (BOOL)(!aide);
        break;

    case ALT_F6:
        if (!ConLock)
            sysop = (BOOL)(!sysop);
        break;

    case F6:
        if (sysop || !ConLock)
            sysopkey = TRUE;
        break;

    case F7:
        cfg.noBells = (uchar)!cfg.noBells;
        break;

    case ALT_C:
    case F8:
        chatkey = (BOOL)(!chatkey);   /* will go into chat from main() */
        break;

    case F9:
        cfg.noChat = (uchar)!cfg.noChat;
        chatReq = FALSE;
        break;
    
    case F10:
#ifdef OLDHELP        
        help();
#else
        altF10();
#endif /* OLDHELP */
        break;

    case ALT_B:
        backout = (BOOL)(!backout);
        break;

    case ALT_D:
        debug = (BOOL)(!debug);
        break;

    case ALT_E:
        eventkey = TRUE;
        break;

    case ALT_L:
        if (cfg.f6pass[0] && strcmpi(cfg.f6pass, "f6disabled") != SAMESTRING)
          ConLock = (BOOL)(!ConLock);
        break;

    case ALT_P:
        if (printing)
        {
            printing=FALSE;
            fclose(printfile);
        }else{
            printfile=fopen(cfg.printer, "a");
            if (printfile)
            {
                printing=TRUE;
            } else {
                printing=FALSE;
                fclose(printfile);
            }
        }
        break;

    case ALT_T:
        twit = (BOOL)(!twit);
        break;

    case ALT_X:
        if (dowhat == MAINMENU)
        {
            echo = BOTH;
            setio(whichIO, echo, outFlag);

            /* silly */
            if (logBuf.TWIRLY)
            {
                doBS();
            }

            mPrintf("Exit to MS-DOS"); doCR();
            ExitToMsdos = TRUE;
        }
        break;
    
    case ALT_V:
        logBuf.VERIFIED = (BOOL)(!logBuf.VERIFIED);
        break;

    case ALT_F10:
        altF10();
        break;
        
    case ALT_A:
        logBuf.lbflags.NOACCOUNT = !logBuf.lbflags.NOACCOUNT;
        break;

    case ALT_S:
        if (cfg.screensave)
        {
            save_screen();
            setscreen();
            cls();
            saver_on = TRUE;
        }
        break;

    case ALT_Z:
        sleepkey = TRUE;
        break;
    
    case PGUP:
        logBuf.credits += 5;
        break;
    
    case PGDN:
        logBuf.credits -= 5;
        break;
        
    default:
        break;
    }

    /* clear out keyboard buffer, I dunno about this */
    while (STATCON()) GETCON();

    update25();
}

/************************************************************************/
/*      KBReady() returns TRUE if a console char is ready               */
/************************************************************************/
BOOL KBReady(void)
{
    int c;

    if (getkey) return(TRUE);
  
    if (kb_hit())
    {
        c = get_ch();

        if (!c)
        {
            fkey();
            return(FALSE);
        }
        else unget_ch(c);

        getkey = 1;
       
        return(TRUE);
    }
    else return(FALSE);

}

/* -------------------------------------------------------------------- */
/*  ciChar()        Get character from console                          */
/* -------------------------------------------------------------------- */
int ciChar(void)
{
    int c;
    
    c = get_ch();
    getkey = 0;
    ++received;  /* increment received char count */

    return c;
}

/* -------------------------------------------------------------------- */
/*  outCon()        put a character out to the console                  */
/* -------------------------------------------------------------------- */
void outCon(char c)
{
    unsigned char row, col;
    static   char escape = FALSE;

    if (!console) return; 

    /* still do bells in screensaver mode */
    if (saver_on && c != 7) return;

    if (c == 7   /* BELL */  && cfg.noBells)  return;
    if (c == 27 || escape) /* ESC || ANSI sequence */
    {
        escape = ansi(c);
        return;
    }
    if (c == 26) /* CT-Z */                   return;

    if (!anyEcho)  return;

    /* if we dont have carrier then count what goes to console */
    if (!gotCarrier()) transmitted++;

    if (c == '\n')                                      /* Newline */
    {
        doccr();
    }
    else
    if (c == '\r')                                      /* Return */
    {
        readpos(&row, &col);
        position(row, 0);
    }
    else 
    if (c == 7)                                         /* Bell */
    {
        putchar(c);
    }
    else
    if (c == '\b')                                      /* Backspace */
    {
        readpos(&row, &col);
        if (col == 0 && prevChar != 10)
        {
            row--;
            col = conCols;
        }
        position(row, (uchar)(col-1));
        (*charattr)(' ', ansiattr);
        position(row, (uchar)(col-1));
    } 
    else                                                /* Other Character */
    {
        readpos(&row, &col);
        (*charattr)(c, ansiattr);
        if (col == (uchar)(conCols-1))
        {
            position(row,col);
            doccr();
        }
    }
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void cPrintf(char *fmt, ... )
{
    register char *buf = prtf_buff;
    va_list ap;
    /* char oldconsole; */

    /* oldconsole = console; */
   /*  console = TRUE; */

    /* for screenblanker */
    kb_hit();
    while(STATCON())  GETCON(); 

    va_start(ap, fmt);
    vsprintf(prtf_buff, fmt, ap);
    va_end(ap);

    while(*buf) {
        outCon(*buf++);
    }

    /* console = oldconsole; */
}

/* -------------------------------------------------------------------- */
/*  cCPrintf()      send formatted output to console, centered          */
/* -------------------------------------------------------------------- */
void cCPrintf(char *fmt, ... )
{
    va_list ap;
    int i;
    uchar row, col;

    va_start(ap, fmt);
    vsprintf(prtf_buff, fmt, ap);
    va_end(ap);

    i = (conCols - strlen(prtf_buff)) / 2;

    strrev(prtf_buff);

    while(i--)
    {
        strcat(prtf_buff, " ");
    }

    strrev(prtf_buff);

    readpos(&row, &col);
    (*stringattr)(row, prtf_buff, cfg.attr);
    /* (*stringattr)(wherey()-1, prtf_buff, cfg.attr); */
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  ctrl_c()        Used to catch CTRL-Cs                               */
/* -------------------------------------------------------------------- */
void ctrl_c(void)
{
    uchar row, col;

    signal(SIGINT, ctrl_c);
    readpos( &row, &col);
    position((uchar)(row-1), 19);
    ungetch('\r');
    getkey = TRUE;
}
#endif 
/* -------------------------------------------------------------------- */
/*  cGets()     get a string from the console.                          */
/* -------------------------------------------------------------------- */
void cGets(char *buff)
{
    cgets(buff);
}

/************************************************************************/
/*      kb_hit() returns TRUE if character waiting                      */
/*      calls assembly language routine STATCON()                       */
/************************************************************************/
BOOL kb_hit(void)
{
    unsigned char c;

    if (!STATCON()) 
        return(FALSE);

    /* screensaver junk */
    time(&keyboard_timer);
    if (cfg.screensave && saver_on)
    {
        saver_on = FALSE;
        restore_screen();
        curson();
        update25();
    }

    c = (inkey = GETCON());

    unget_ch(c);
    return(TRUE);
}

/************************************************************************/
/*      unget_ch()  ungets a character from keyboard                    */
/************************************************************************/
void unget_ch(char c)
{
    ungetflag = TRUE;
    ungetchar = c;
}

/************************************************************************/
/*      get_ch()  returns character from keyboard                       */
/************************************************************************/
int get_ch(void)
{
    /* union REGS REG; */
    int c;

    if (ungetflag)
    {
        ungetflag = FALSE;
        c = ungetchar;
    }
    else
    {
#ifdef GOODBYE
        REG.h.ah = 7;
        intdos(&REG, &REG);
        c = REG.h.al;
#endif
    c = GETCON();

    }
    return(c);
}

