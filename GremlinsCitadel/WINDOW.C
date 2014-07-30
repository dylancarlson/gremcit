/* -------------------------------------------------------------------- */
/*                              window.c                                */
/*            Machine dependent windowing routines for Citadel          */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/*      cls()                   clears the screen                       */
/*      connectcls()            clears the screen upon carrier detect   */
/*      clearline()             blanks out specified line with attr     */
/*      cursoff()               turns cursor off                        */
/*      curson()                turns cursor on                         */
/*      gmode()                 checks for monochrome card              */
/*      help()                  toggles help menu                       */
/*      position()              positions the cursor                    */
/*      readpos()               returns current row, col position       */
/*      scroll()                scrolls window up                       */
/*      setscreen()             sets videotype flag                     */
/*      update25()              updates the 25th line                   */
/*      updatehelp()            updates the help window                 */
/*      directchar()            Direct screen write char with attr      */
/*      directstring()          Direct screen write string w/attr at row*/
/*      bioschar()              BIOS print char with attr               */
/*      biosstring()            BIOS print string w/attr at row         */
/*      setscreen()             Set SCREEN to correct address for VIDEO */
/*      ScreenFree()            Handle screen swap between screen/logo  */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data                                                         */
/* -------------------------------------------------------------------- */
#ifdef OLDHELP
static BOOL F10up = FALSE;
static long f10timeout;               /* when was the f10 window opened?*/
#endif /* OLDHELP */
static char far *screen;      /* memory address of RAM for direct screen I/O */
static char far *saveBuffer = NULL;  /* memory buffer for screen saves       */
static uchar row, column;     /* static vars for cursor position             */
static BOOL altF10up = FALSE;
/* static void updatealtF10(void); */
/* static uchar cursorstart, cursorend; */
static int iso_clr[] = {0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7};
static char screenstat = 1;

/* -------------------------------------------------------------------- */
/*      cls()  clears the screen depending on carrier present or not    */
/* -------------------------------------------------------------------- */
void cls(void)
{
    if (saver_on) return;  /* screensaver */

    /* scroll everything but kitchen sink */
    scroll(conRows, 0, cfg.attr);
    position(0,0);
}

/* -------------------------------------------------------------------- */
/*      connectcls()  clears the screen upon carrier detect             */
/* -------------------------------------------------------------------- */
void connectcls(void)
{
    if (saver_on) return;  /* screensaver */

    if (anyEcho)
    {
        cls();
    }
    update25();
}


#ifdef GOODBYE     /* these made little nemo's cursor disappear */
/* -------------------------------------------------------------------- */
/*      cursoff()  make cursor disapear                                 */
/* -------------------------------------------------------------------- */
void cursoff(void)
{
    union REGS REG;


    readcursorsize(&cursorstart, &cursorend);

    REG.h.ah = 01;
    REG.h.bh = 00;
    REG.h.ch = 0x26;
    REG.h.cl = 7;
    int86(0x10, &REG, &REG);
}

/* -------------------------------------------------------------------- */
/*      curson()  Put cursor back on screen checking for adapter.       */
/* -------------------------------------------------------------------- */
void curson(void)
{
    union REGS REG;

    REG.h.ah = 01;
    REG.h.bh = 00;
    REG.h.ch = cursorstart;
    REG.h.cl = cursorend;
    int86(0x10, &REG, &REG);

}
#endif


                /* this are the old routines, may work for nemo */
/* -------------------------------------------------------------------- */
/*      curson()  Put cursor back on screen checking for adapter.       */
/* -------------------------------------------------------------------- */
void curson(void)
{
    union REGS regs;
    uchar st, en;
                          /* Mono ega */
    if ((gmode() == 7) && conMode != 1002)  /* Monocrome adapter */
    {  
        st = 12;
        en = 13;
    }
    else               /*  Color graphics adapter. */
    {                  
        st = 6;
        en = 7;
    }

    regs.h.ah = 0x01;
    regs.h.bh = 0x00;
    regs.h.ch = st;
    regs.h.cl = en;
    int86(0x10, &regs, &regs);
}

/* -------------------------------------------------------------------- */
/*      cursoff()  make cursor disapear                                 */
/* -------------------------------------------------------------------- */
void cursoff(void)
{
    union REGS REG;

    REG.h.ah = 01;
    REG.h.bh = 00;
    REG.h.ch = 0x26;
    REG.h.cl = 7;
    int86(0x10, &REG, &REG);
}



/* -------------------------------------------------------------------- */
/*      gmode()  Check for monochrome or graphics.                      */
/* -------------------------------------------------------------------- */
int gmode(void)
{
    union REGS REG;

    REG.h.ah = 0x0f;
    REG.h.bh = 00;
    REG.h.ch = 0;
    REG.h.cl = 0;
    int86(0x10, &REG, &REG);

    return(REG.h.al);
}

#ifdef OLDHELP
/* -------------------------------------------------------------------- */
/*      help()  this toggles our help menu                              */
/* -------------------------------------------------------------------- */
void help(void)
{
    uchar row, col;

    readpos(&row, &col);
 
    /* 
     * small window 
     */
    if (!F10up)     
    {
        /* 
         * Make big window 
         */
        scrollpos = (uchar)(conRows-5);
        if (altF10up) scrollpos--;
                
        if (row > scrollpos)
        {
            scroll(row, (uchar)(row - scrollpos), cfg.wattr);
            row = scrollpos;
        }
 
        if (cfg.bios || conCols > 80)
        {
            if (altF10up)  clearline(conRows-5, cfg.wattr);
            clearline(conRows-4, cfg.wattr);
            clearline(conRows-3, cfg.wattr);
            clearline(conRows-2, cfg.wattr);
            clearline(conRows-1, cfg.wattr);
        }
 
        time(&f10timeout);
        F10up = TRUE;
    }
    else
    {
        /*
         * Make small window 
         */
        if (altF10up)  clearline(conRows-5, cfg.attr);
        clearline(conRows-4, cfg.attr);
        clearline(conRows-3, cfg.attr);
        clearline(conRows-2, cfg.attr);
        clearline(conRows-1, cfg.attr);

        scrollpos = (uchar)(conRows-1);    
        if (altF10up) scrollpos--;
        F10up = FALSE;
    }
    position(row, col);
}
#endif /* OLDHELP */

/* -------------------------------------------------------------------- */
/*  altF10(void)    Extra stat line..                                   */
/* -------------------------------------------------------------------- */
void altF10(void)
{
    uchar row, col;

    readpos(&row, &col);
 
    /* 
     * small window 
     */
    if (!altF10up)     
    {
        /* 
         * Put up the stat line
         */
        scrollpos --;
                
        if (row > scrollpos)
        {
            scroll(row, (uchar)(row - scrollpos), cfg.wattr);
            row = scrollpos;
        }
 
        if (cfg.bios || conCols > 80)
        {
            clearline(scrollpos+1, cfg.wattr);
        }
        
        altF10up = TRUE;
    }
    else
    {
        /*
         * Make small window 
         */
        scrollpos++;
        
        clearline(scrollpos, cfg.attr);

        altF10up = FALSE;
    }
    position(row, col);
}

/* -------------------------------------------------------------------- */
/*      position()  positions the cursor                                */
/* -------------------------------------------------------------------- */
void position(uchar row , uchar column)
{
    union REGS regs;

    regs.h.ah = 0x02;        /* set cursor position interrupt */
    regs.h.dh = row;         /* row                           */
    regs.h.dl = column;      /* column                        */
    regs.h.bh = 0;           /* display page #0               */
    int86( 0x10, &regs,  &regs);
}

/* -------------------------------------------------------------------- */
/*      clearline()  clears specified line to attr                      */
/* -------------------------------------------------------------------- */
void clearline(unsigned int row, uchar attr)
{
    union REGS regs;

    position((uchar)row, (uchar)0); /* set cursor on the bottom line   */
    regs.h.ah = 0x09;        /* service 9, write character # attribute */
    regs.h.bl = attr;        /* character attribute                    */
    regs.h.al = 32;          /* write spaces     0x0900                */
    regs.x.cx = conCols;     /* clear whole line                       */
    regs.h.bh = 0;           /* display page                           */
    int86( 0x10, &regs,  &regs);          
}


/* -------------------------------------------------------------------- */
/*      readpos()  returns current cursor position                      */
/* -------------------------------------------------------------------- */
void readpos(uchar *row, uchar *column)
{
    union REGS regs;

    regs.h.ah = 0x03;        /* set cursor position interrupt */
    regs.h.bh = 0;           /* display page #0               */
    int86( 0x10, &regs,  &regs);

    *row    = regs.h.dh;     /* row                           */
    *column = regs.h.dl;     /* column                        */
}

/* -------------------------------------------------------------------- */
/*      readcursorsize()  returns current cursor position               */
/* -------------------------------------------------------------------- */
void readcursorsize(uchar *cursorstart, uchar *cursorend)
{
    union REGS regs;

    regs.h.ah = 0x03;        /* set cursor position interrupt */
    regs.h.bh = 0;           /* display page #0               */
    int86( 0x10, &regs,  &regs);

    *cursorstart   = regs.h.ch;     /* row                           */
    *cursorend     = regs.h.cl;     /* column                        */
}


/* -------------------------------------------------------------------- */
/*      scroll()  scrolls window up from specified line                 */
/* -------------------------------------------------------------------- */
void scroll( uchar row, uchar howmany, uchar attr)
{
    union REGS regs;
    uchar rw, col;

    readpos( &rw, &col);

    regs.h.al = howmany;     /* scroll how many lines                */

    regs.h.cl = 0;           /* row # of upper left hand corner      */
    regs.h.ch = 0;           /* col # of upper left hand corner      */
    regs.h.dh = row;         /* row # of lower left hand corner      */
    regs.h.dl = (uchar)
                (conCols-1); /* col # of lower left hand corner      */
    
    regs.h.ah = 0x06;        /* scroll window up interrupt           */
    regs.h.bh = attr;        /* char attribute for blank lines       */

    int86( 0x10, &regs,  &regs);

    /* put cursor back! */
    position( rw, col);
}





#ifdef OLDHELP
/* -------------------------------------------------------------------- */
/*      updatehelp()  updates the help menu according to global vars    */
/* -------------------------------------------------------------------- */
void updatehelp(void)
{
    long time(), l;
    char bigline[81];
    uchar row, col;

    if ( f10timeout < (time(&l) - (long)(60 * 2)) ) 
    {
        help();
        return;
    }

    if(cfg.bios)  cursoff();

    strcpy(bigline,
    "ษอออออออออออออออัอออออออออออออออัออออออออออออออัอออออออออออออออัอออออออออออออออป");

    readpos( &row, &col);

    (*stringattr)(conRows-4, bigline, cfg.wattr);

    sprintf(bigline, "บ%sณ%sณ%sณ%sณ%sบ",
          " F1 þ Shutdown ", " F2 þ Startup  " , " F3 þ Request ",
                 (anyEcho) ? " F4 þ Echo-Off " : " F4 þ Echo-On  ",
      (whichIO == CONSOLE) ? " F5  þ Modem   " : " F5  þ Console ");

    (*stringattr)(conRows-3, bigline, cfg.wattr);

    sprintf(bigline, "บ%sณ%sณ%sณ%sณ%sบ",
    " F6 þ Sysop Fn ", (cfg.noBells) ? " F7 þ Bells-On " : " F7 þ Bells-Off" ,
    " F8 þ ChatMode",  (cfg.noChat)  ? " F9 þ Chat-On  " : " F9 þ Chat-Off ",
    " F10 þ Help    ");

    (*stringattr)(conRows-2, bigline, cfg.wattr);

    strcpy(bigline,
    "ศอออออออออออออออฯอออออออออออออออฯออออออออออออออฯอออออออออออออออฯอออออออออออออออผ");


    (*stringattr)(conRows-1, bigline, cfg.wattr);

    position( (uchar)(row >= scrollpos ? scrollpos : row), col);

    if(cfg.bios)  curson();
}
#endif /* OLDHELP */

/* -------------------------------------------------------------------- */
/*      directstring() print a string with attribute at row             */
/* -------------------------------------------------------------------- */
void directstring(unsigned int row, char *str, uchar attr)
{
    register int i, j, l;

    l = strlen(str);

    for(i=row*(conCols*2), j=0; j<l; i +=2, j++)
    {
        screen[i] = str[j];
        screen[i+1] = attr;
    }
}

/* -------------------------------------------------------------------- */
/*      directchar() print a char directly with attribute at row        */
/* -------------------------------------------------------------------- */
void directchar(char ch, uchar attr)
{
    int i;
    uchar row, col;

    readpos( &row, &col);

    i = (row*(conCols*2))+(col*2);

    screen[i] = ch;
    screen[i+1] = attr;

    position( row, (uchar)(col+1));
}


/* -------------------------------------------------------------------- */
/*      biosstring() print a string with attribute                      */
/* -------------------------------------------------------------------- */
void biosstring(unsigned int row, char *str, uchar attr)
{
    union REGS regs;
    union REGS temp_regs;
    register int i=0;

    regs.h.ah = 9;           /* service 9, write character # attribute */
    regs.h.bl = attr;        /* character attribute                    */
    regs.x.cx = 1;           /* number of character to write           */
    regs.h.bh = 0;           /* display page                           */

    while(str[i])
    {
        position((uchar)row, (uchar)i);/* Move cursor to the correct position */
        regs.h.al = str[i];            /* set character to write     0x0900   */
        int86( 0x10, &regs, &temp_regs);
        i++;
    }
}


/* -------------------------------------------------------------------- */
/*      bioschar() print a char with attribute                          */
/* -------------------------------------------------------------------- */
void bioschar(char ch, uchar attr)
{
    uchar row, col;
    union REGS regs;

    regs.h.ah = 9;           /* service 9, write character # attribute */
    regs.h.bl = attr;        /* character attribute                    */
    regs.h.al = ch;          /* write 0x0900                           */
    regs.x.cx = 1;           /* write 1 character                      */
    regs.h.bh = 0;           /* display page                           */ 
    int86( 0x10, &regs, &regs);

    readpos( &row, &col);
    position( row, (uchar)(col+1));
}

/* -------------------------------------------------------------------- */
/* Handle ansi escape sequences                                         */
/* -------------------------------------------------------------------- */
char ansi(char c)
{
    static char args[20], first = FALSE;
    static uchar c_x = 0, c_y = 0;
    uchar argc, a[5];
    int i;
    uchar x, y;
    char *p;
    static BOOL hilight = FALSE,    /* hilight is on */
                blink = FALSE;      /* blinking is on */
        /*        dirty = TRUE; */      /* the hilight (remote) is dirty */
    char str[80];
    char temp[20];  


    if (c == 27 /* ESC */)
    {
        strcpy(args, "");
        first = TRUE;
        return TRUE;
    }

    if (first && c != '[')
    {
        first = FALSE;
        return FALSE;
    }

    if (first && c == '[')
    {
        first = FALSE;
        return TRUE;
    }

    for (i = 0; i < 5; ++i)
    {
        a[i] = 0;
    }
    
    if (isalpha(c))
    {
        p = args; 
        argc = 0;

        while(*p && (argc < 5))
        {
            if (isdigit(*p))
            {
                a[argc]=(uchar)atoi(p);
                if (a[argc] > 9) p++;
                p++;
                argc++;
            }
            else
                p++;
        }

        switch(c)
        {
        case 'J': /* cls */
                cls();
                update25();
                break;
        case 'K': /* del to end of line */
                clreol();
                break;
        /* auto ansi detect */
        case 'n':

                if (a[0] == 6)
                {
                    outstring("[Z");
                }
                break;
        case 'm':
                for (i = 0; i < (int)argc; i++)
                {
                    switch(a[i])
                    {
                    case 5:
                        ansiattr = (uchar)(ansiattr | 128); /* Blink */
                        blink = TRUE;
                        break;
                    case 4:
                        ansiattr = (uchar)(ansiattr | 1);   /* Underline */
                        break;
                    case 7:
                        ansiattr = cfg.wattr;               /* Reverse Video */
                        break;
                    case 0:
                        ansiattr = cfg.attr;                /* Default */
                        blink = FALSE;
                        hilight = FALSE;
                        break;
                    case 1:
                        ansiattr = cfg.cattr;               /* Bold */
                        hilight = TRUE;
                        break;
                    default:
    
                       /*
                        * Set the background
                        */
                        if (a[i] >= 40 && a[i] <= 47)
                        {
                            ansiattr = (uchar)((ansiattr & 0x0F) 
                            | ((iso_clr[a[i]-40]) << 4) | (blink ? 0x80 : 0));
                        }
    
                       /*
                        * Set the foreground 
                        */
                        if (a[i] >= 30 && a[i] <= 37)
                        {
                            ansiattr = (uchar)((ansiattr & 0xF0) 
                            | ((iso_clr[a[i]-30]) | (hilight ? 0x08 : 0)));
                        }

                        break;
                    }
                }
            break;
        case 's': /* save cursor */
                readpos(&c_x, &c_y);
                break;
        case 'u': /* restore cursor */
                position(c_x, c_y);
                break;
        case 'A':
                readpos(&x, &y);
                i = x - (argc ? a[0] : 1);
                if (i) 
                    x = (uchar)i;
                position(x, y);
                break;
        case 'B':
                readpos(&x, &y);
                i = x + (argc ? a[0] : 1);
                if (i<(int)scrollpos) 
                    x = (uchar)i;
                position(x, y);
                break;
        case 'D':
                readpos(&x, &y);
                i = y - (argc ? a[0] : 1);
                if (i) 
                    y = (uchar)i;
                position(x, y);
                break;
        case 'C':
                readpos(&x, &y);
                i = y + (argc ? a[0] : 1);
                if (i<(int)conCols) 
                    y = (uchar)i;
                position(x, y);
                break;
        case 'f':
        case 'H':
                if (!argc)
                {
                    position(0,0);
                    break;
                }
                if (argc == 1)
                {
                    if (args[0] == ';')
                    {
                        a[1] = a[0];
                        a[0] = 1;
                    }else{
                        a[1] = 1;
                    }
                    argc = 2;
                }
                if (argc == 2 && a[0] < (uchar)(conRows+1) && a[1] < conCols)
                {
                    position((uchar)(a[0]-1), (uchar)(a[1]-1));
                    break;
                }
        default:
                {

                sprintf(temp, "%s%c", args, c);

       sprintf(str, "Ansi Code: [%-15s  Args: (%d) [%02d %02d %02d %02d %02d]",
       temp, argc, a[0], a[1], a[2], a[3], a[4]);


                    (*stringattr)(0, str, cfg.wattr);
                }
                break;
        }
        if (debug)
        {

                sprintf(temp, "%s%c", args, c);

       sprintf(str, "Ansi Code: [%-15s  Args: (%d) [%02d %02d %02d %02d %02d]",
       temp, argc, a[0], a[1], a[2], a[3], a[4]);

            (*stringattr)(0, str, cfg.wattr);
        }
        return FALSE;
    }else{
        {
            i = strlen(args);
            args[i]=c;
            args[i+1]='\0' /*NULL*/;
        }
        return TRUE;
    }
}       

/* -------------------------------------------------------------------- */
/*  save_screen() allocates a buffer and saves the screen               */
/* -------------------------------------------------------------------- */
void save_screen(void)
{
    if (!screenstat) return;

    if (saver_on) return;  /* screensaver */

    if (saveBuffer == NULL)
    {
        saveBuffer = _fcalloc(((conRows+1)*conCols*2), sizeof(char));

        if (saveBuffer == NULL)
        {
            mPrintf("Cannot allocate screen\n");
        }
        else
        {
            memcpy(saveBuffer, screen, ((conRows+1)*conCols*2));
            screenstat = 0;
        }
        readpos( &row , &column);
    }
}


/* -------------------------------------------------------------------- */
/*   restore_screen() restores screen and free's buffer                 */
/* -------------------------------------------------------------------- */
void restore_screen(void)
{
    if (screenstat) return;

    if (saver_on) return;  /* screensaver */

    if (saveBuffer != NULL)
    {
        setscreen();
        
        memcpy(screen, saveBuffer, ((conRows+1)*conCols*2));
        screenstat = 1;
        _ffree((void *)saveBuffer);
        position(row , column);
        saveBuffer = NULL;
    }
}

/* -------------------------------------------------------------------- */
/* ScreenFree() either saves the screen and displays the opening logo   */
/*      or restores, depending on anyEcho                               */
/* -------------------------------------------------------------------- */
void ScreenFree(void)
{
    static uchar row, col, helpVal = 0;

    if (anyEcho)
    {
        if (scrollpos == 19)        /* the help window is open */
        {
            helpVal = 1;
        }
        else
        {
            helpVal = 0;
        }

        save_screen();
        readpos( &row, &col);
        cursoff();
        logo();

        if (helpVal)
        {
#ifdef OLDHELP
            updatehelp();
#endif /* OLDHELP */        
        }
    }
    else
    {
        restore_screen();
        if (helpVal == 0 && scrollpos == 19)   /* window opened while locked */
        {
            if (row > 19)
            {
                scroll( 23, (uchar)(row - 19), cfg.wattr);
                position(19, col);
            }
#ifdef OLDHELP
            updatehelp();
#endif /* OLDHELP */
        }

        if (helpVal == 1 && scrollpos == 23)   /* window closed while locked */
        {
            clearline(20, cfg.attr);
            clearline(21, cfg.attr);
            clearline(22, cfg.attr);
            clearline(23, cfg.attr);
        }
        position(row, col);
        curson();
    }
}

/* -------------------------------------------------------------------- */
/*      updatehelp()  updates the help menu according to global vars    */
/* -------------------------------------------------------------------- */
void updatealtF10(void)
{
    char str[200];
    int i;
        
    sprintf(str, " %-30.30s ณ %-30.30s ณ ",
        hallBuf->hall[thisHall].hallname,
        roomBuf.rbname);
        
    for (i = strlen(str); i < (int)conCols; i++)
    {
        strcat(str, " ");
    }
    
    (*stringattr)(scrollpos+1, str, cfg.wattr);
}

/* -------------------------------------------------------------------- */
/*      update25()  updates the 25th line according to global variables */
/* -------------------------------------------------------------------- */
void update25(void)
{
    char string[200];
    char str2[200];
    char name[50];
    char flags[10];
    char carr[5];
    char tmleft[6];
    uchar row, col;
    int i;
    char displaybaud[10];

    if (saver_on) return;
    
#ifdef OLDHELP
    if (F10up)      updatehelp();
#endif /* OLDHELP */
    if (altF10up)   updatealtF10();

    if (cfg.bios)  cursoff();
    
    readpos(&row, &col);


    if (loggedIn)  
    {
         strcpy( name, deansi(logBuf.lbname));
         for (i = 0; i < ((30 - strlen(logBuf.lbname)) / 2); i++)
           strcat(name, " ");
    }
    else
    {
         strftime( name, 30, "ออออ %y%b%D  %H:%M:%S ออออ ", 0l);
    }

#ifdef GOODBYE
    strcpy(string, deansi(logBuf.lbname));
    string[23] = 0;
    
    if (loggedIn)  
    {
        name[0] = 0;
        for (i = 0; i < ((24 - strlen(string)) / 2); i++)
        {
            strcat(name, "๗");
        }
        strcat( name, "๗ ");
        strcat( name, string);
        strcat( name, " ๗");
        for (i = 0; i < ((24 - strlen(string)) / 2); i++)
        {
            strcat(name, "๗");
        }
    }
    else
    {
         strcpy( name, "ออออออ Not logged in ออออออ");
    }
#endif

    if (gotCarrier())           strcpy( carr, "CD");
    else                        strcpy( carr, "NC");

    strcpy(flags, "       ");

    if ( aide )                     flags[0] = 'A';
    if ( twit )                     flags[1] = 'T';
    if ( logBuf.lbflags.PERMANENT ) flags[2] = 'P';
    if ( logBuf.lbflags.NETUSER )   flags[3] = 'N';
    if ( logBuf.lbflags.UNLISTED )  flags[4] = 'U';
    if ( sysop )                    flags[5] = 'S';
    if ( logBuf.lbflags.NOMAIL )    flags[6] = 'M';

    if (!cfg.accounting || logBuf.lbflags.NOACCOUNT || !loggedIn)
    {
        strcpy(tmleft, " NA ");
    }
    else
    {
        if (specialTime)
            strcpy(tmleft, "SpTm");
        else
            sprintf(tmleft, "%4.0f", logBuf.credits);
    }

    if (bauds[speed] == 19200) 
        strcpy (displaybaud, "19.2");
    else
    if (bauds[speed] == 38400U) 
        strcpy (displaybaud, "38.4");
    else
    if (bauds[speed] == 57600U) 
        strcpy (displaybaud, "57.6");
    else
        sprintf(displaybaud, "%4u", bauds[speed]);
    
    sprintf( string, "%30.30sณ%sณ%sณ%sณ%sณ%sณ%c%c%c%cณ%sณ%sณ%sณ%s",
      name,
      (whichIO == CONSOLE) ? "Console" : " Modem ",
      carr,
      displaybaud,
      tmleft,
      (disabled)    ? "DS" : "EN",
      (cfg.noBells) ? ' ' : '',
      (backout)     ? '' : ' ',
      (debug)       ? '่' : ' ',
      (ConLock)     ? '' : ' ',
      (cfg.noChat)  ? ((chatReq) ? "rcht" : "    " ) : 
                      ((chatReq) ? "RCht" : "Chat" ),
      (printing)    ? "Prt"  : "   ",
      (sysReq)      ? "REQ"  : "   ",
      flags
    );
 
    if (conCols > 99)
    {
        sprintf(str2, "ณ%-20.20s", roomBuf.rbname);
        strcat(string, str2);
    }
    
    if (conCols > 130)
    {
        sprintf(str2, "ณ%-20.20s", hallBuf->hall[thisHall].hallname);
        strcat(string, str2);
    }
    
    for (i = strlen(string); i < (int)conCols; i++)
    {
        strcat(string, " ");
    }
 
    (*stringattr)(conRows, string, cfg.wattr);

    position(row,col);

    if(cfg.bios)  curson();
}

/* -------------------------------------------------------------------- */
/*      setscreen() set video mode flag 0 mono 1 cga                    */
/* -------------------------------------------------------------------- */
void setscreen(void)
{
    int mode;
    union REGS REG;
    static unsigned char heightmode = 0;
    static unsigned char scanlines = 0;
    char mono = 0;
    char far *CrtHite    = (char  far *) 0x00400085L;

    if (gmode() == 7) mono = TRUE;
       
    if (!heightmode /* && !mono */)
    {
        if      (*CrtHite == 8)       heightmode = 0x012;
        else if (*CrtHite == 14)      heightmode = 0x011;
        else if (*CrtHite == 16)      heightmode = 0x014;
    }

    if (scanlines /* && !mono */)
    {
        REG.h.ah = 0x12;          /* Video function: */
        REG.h.bl = 0x30;          /* Set scan lines  */
        REG.h.al = scanlines;     /* Num scan lines  */
        int86(0x10, &REG, &REG);
    }

    if (conMode != -1)
    {
        /*
         * conMode 1000 --> EGA 43 line, or VGA 50 line
         */

     /*  if (!mono)  */
    /*  { */
            REG.h.ah = 0x00;
            REG.h.al = (uchar)((conMode >= 1000) ? 3 : conMode);
            int86(0x10, &REG, &REG);
      /*  } */
    
        /*
         * Set to character set 18, (EGA 43 line, or VGA 50 line)
         */
        if (conMode == 1000)
        {
            REG.h.ah = 0x11;
            REG.h.al = 18;
            REG.h.bl = 0x00;
            int86(0x10, &REG, &REG);
        } 
        else /* if (!mono) */
        { 
            REG.h.ah = 0x11;
            REG.h.al = heightmode;
            REG.h.bl = 0x00;
            int86(0x10, &REG, &REG);
        } 
    }

    if (!scanlines /* && !mono */)
    {
        getScreenSize((&conCols), (&conRows));

        if (!mono)
        {
            if (conRows == 24)
            {
                if (*CrtHite == 14)  scanlines = 1;   /* Old char set */
                if (*CrtHite == 16)  scanlines = 2;   /* Vga char set */
            }
            if (conRows == 42)  scanlines = 1;
            if (conRows == 49)  scanlines = 2;
        }
        else
        {
            if (conRows == 24) scanlines = 1;
            if (conRows == 27) scanlines = 2;  /* herc only */
            if (conRows == 42) scanlines = 1;
        }
    }
    
    mode = gmode();
    conMode = (mode == 3 && conMode >= 1000) ? conMode : mode;
    
    if(mode == 7)
    {
        screen = (char far *)0xB0000000L;    /* mono */
    }
    else
    {
        screen = (char far *)0xB8000000L;    /* cga */
        /* outp(0x03d9, cfg.battr); */       /* set border color */
    }

    getScreenSize((&conCols), (&conRows));
    
    scrollpos = (uchar)(conRows-1);
#ifdef OLDHELP
    if (F10up)
        scrollpos = (uchar)(conRows-5);
#endif
    if (altF10up) scrollpos--;

    if(cfg.bios)
    {
        charattr = bioschar;
        stringattr = biosstring;
    }
    else
    {
        charattr = directchar;
        stringattr = directstring;
    }
    ansiattr = cfg.attr;
}

void getScreenSize(uchar *Cols, uchar *Rows)
{
    *Cols = *(unsigned char far *)(0x0000044A);
    *Rows = *(unsigned char far *)(0x00000484);
    
    /* 
     * sanity checks, some cards don't set these locations.
     * (Heather's CGA for example. =] ) 
     */
    if (*Cols < 40) *Cols = 80;
    if (*Rows < 15) *Rows = 24;
    
    /* 
     * now an overide for stupid CGA's (like Krissy's) who
     * insist on writing to this location.. 
     */
    if (conMode == 1001)
    {
        *Cols = 80; 
        *Rows = 24; 
    }
}
/* -------------------------------------------------------------------- */
/* clreol() delete to end of line                                       */
/* -------------------------------------------------------------------- */
void clreol(void)
{
    uchar row, col;
    int i;

    readpos( &row, &col);

    for (i=col; i < 80; i++)
        putch(' ');

    position( row, col);
}

