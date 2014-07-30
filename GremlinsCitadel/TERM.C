/* -------------------------------------------------------------------- */
/*  TERM.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                       Terminal emulation code                        */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  setTerm()       Setup the terminal                                  */
/*  termCap()       Does a terminal command                             */
/*  putCode()       Sends the escape code to the modem                  */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
static int iso_clr[] = {0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7};
static void putCode(char *str);
void setOutTrans(BOOL ibmAnsi);

#ifdef GOODBYE

/* Name, Descript, 
   Normal, Bold, Inverse, Blink, Underline, 
   IBM-Extended, IBM-Color 
 */

static  TERMINAL    trm_tty = 
    {"", "", "", "", "", 
     FALSE, FALSE };
     
static  TERMINAL    trm_ansi_bbs = 
    {"[0m", "[1m", "[7m", "[5m", "[4m", 
     TRUE, FALSE };
     
static  TERMINAL    trm_ansi_col = 
    {"[0;1;37;40m", "[1;33;40m", "[1;33;41m", "[5m", "[1;35;40m", 
     TRUE, TRUE };

static  TERMINAL    trm_vt100 = 
    {"[0m", "[1m", "[7m", "[5m", "[4m", 
     FALSE, FALSE };

static TERMINAL *terms[] = {
    &trm_tty,
    &trm_ansi_bbs,
    &trm_ansi_col,
    &trm_vt100,
    NULL
};
#endif

const unsigned char filt_out[256] = {  /* translated character output filter  */
    '\0','\1','\0','\0','\0','\0','\0','\7',       /*0..x7*/
    '\b','\t','\n','\0','\0','\0','\0','N',
    '\0','\0','\0','P', '\0','\0','\0','\0',       /*x10..x17*/
    '\0','\0','\x1a','\x1b','\0','\0','\0','\0',
    ' ', '!', '"', '#', '$', '%', '&', '\'',     /* x20..x27 */
    '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',        /* x30..x37 */
    '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',        /* x40..x47 */
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',        /* x50..x57 */
    'X', 'Y', 'Z', '[', '\\',']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',        /* x60..x67 */
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w',        /* x70..x77 */
    'x', 'y', 'z', '{', '|', '}', '~', '\x7f',
    'C', 'u', 'e', 'a', 'a', 'a', 'a', 'c',        /* x80..x87 */
    'e', 'e', 'e', 'i', 'i', 'i', 'A', 'A',
    'E', ' ', ' ', 'o', 'o', 'o', 'u', 'u',        /* x90..x97 */
    'y', 'O', 'U', 'c', 'L', 'Y', 'P', 'f',
    'a', 'i', 'o', 'u', 'n', 'N', 'a', 'o',        /* xA0..xA7 */
    '?', ' ', ' ', ' ', ' ', '!', '<', '>',
    '%', '*', '#', '|', '+', '+', '+', '\\',       /* xB0..xB7 */
    '\\','+', '|', '\\','/', '/', '/', '\\',
    '\\','+', '+', '+', '-', '+', '+', '+',        /* xC0..xC7 */
    '\\','/', '+', '+', '+', '-', '+', '+',
    '+', '+', '+', '\\','\\','/', '/', '+',        /* xD0..xD7 */
    '+', '/', '/', '#', '#', '#', '#', '#',
    'a', 'B', ' ', ' ', ' ', 'o', 'u', 't',        /* xE0..xE7 */
    ' ', ' ', ' ', 'd', ' ', ' ', 'E', 'n',
    '=', '+', '>', '<', ' ', ' ', '/', '=',        /* xF0..xF7 */
    'o', '.', '.', ' ', 'n', '2', '.', '\xff'
};

/* -------------------------------------------------------------------- */
/*  autoansi()  this sets term at according to cfg.autoansi or does     */
/*              actual auto ansi detect.                                */
/* -------------------------------------------------------------------- */
void autoansi(void)
{
     char x;
     char inch;
     char detected = 0;
     long t;

     if (cfg.autoansi == 0)
     {
        setdefaultTerm(0);
     }
     else
     if (cfg.autoansi == 2)
     {
        setdefaultTerm(2);
     }
     else
     {
         if (modStat)
         {
             tfilter[27] = 27;
             mPrintf("Auto Ansi test . . . ");

             Mflush();
     
             for (x = 0; x < 3 && !MIReady(); x++)
             {
                 outstring("[6n");
                 pause(50);
             }

             if (MIReady())
             {
                 inch = (char)getMod();

                 if (inch == 27)
                 {
                     detected = TRUE;
                     mPrintf(" Ansi Detected . . .");
                     doCR();
                     doCR();
                     setdefaultTerm(2);
                 }
             }

             if (!detected)
             {
                 mPrintf(" Ansi NOT Detected . . .");

                 doCR();
                 doCR();
                 mPrintf("Wait 5 seconds, or hit (space), (ESC), (A)nsi, or (S)top . . .");

                 t = time(NULL);

                 Mflush();

                 inch = 0;

                 do
                 {
                     if (MIReady())
                     {
                         inch = (char)getMod();
                     }

                     if (KBReady())
                     {
                         inch = (char)ciChar();
                     }

                     inch = toupper(inch);
                 }   while( gotCarrier() &&  (inch != ' ') && (inch != 'A') && 
                                             (inch != 'S') && (inch != 27)
                      &&
                  ((unsigned int)(time(NULL) - t) < 5) );

                 doCR();

                 if (toupper(inch) == 'A')
                     setdefaultTerm(2);
                 else
                     setdefaultTerm(0);
             }
             tfilter[27] = 0;
         }
         else
         {
             setdefaultTerm(2);
         }    
     }
     Mflush();
}

/* -------------------------------------------------------------------- */
/*  putCode()       Sends the escape code to the modem                  */
/* -------------------------------------------------------------------- */
static void putCode(char *str)
{
    if (printing && debug)
        fprintf(printfile, str);

    while(*str)
    {
        outMod(*str);
        str++;
    }
}


/* -------------------------------------------------------------------- */
/*  localTermCap()  Does a local (non-networked) terminal command       */
/* -------------------------------------------------------------------- */
void localTermCap(char c)
{
    /* setio(whichIO, echo, outFlag); */
    
    switch (toupper(c))
    {
    case TERM_BS:
        doBS();
        pause(5);
        break;
        
    case TERM_IMPERV:
        outFlag = NOSTOP;
        setio(whichIO, echo, outFlag);
        break;
        
    case TERM_PAUSE:
        pause(100);
        break;
    
    case TERM_HANGUP:
        Hangup();
        break;
    
    default:
        break;  
    }
}

/* -------------------------------------------------------------------- */
/*  setdefaultcolors() sets default colors                              */
/* -------------------------------------------------------------------- */
void setdefaultcolors(void)
{
    logBuf.attributes[ATTR_NORMAL]    = (uchar)(logBuf.IBMCOLOR ? 15  : 7);
    logBuf.attributes[ATTR_BLINK]     = (uchar)(logBuf.IBMCOLOR ? 128 : 128);
    logBuf.attributes[ATTR_REVERSE]   = (uchar)(logBuf.IBMCOLOR ? 78  : 112);
    logBuf.attributes[ATTR_BOLD]      = (uchar)(logBuf.IBMCOLOR ? 14  : 8);
    logBuf.attributes[ATTR_UNDERLINE] = (uchar)(logBuf.IBMCOLOR ? 13  : 1);
}

/* -------------------------------------------------------------------- */
/*  setdefaultTerm()  Setup the terminal                                */
/*                    0=tty, 1=mono, 2=color, 3=vt100                   */
/* -------------------------------------------------------------------- */
void setdefaultTerm(char t)
{
    switch(t)          
    {                   
    case 0:  /* TTY */
    strcpy(term.normal,  "");
    strcpy(term.bold,    "");
    strcpy(term.inverse, "");
    strcpy(term.blink,   "");
    strcpy(term.under,   "");
    term.ibmAnsi  =      0;
    term.ibmColor =      0;
    break;

    case 1: /* MONO */
    strcpy(term.normal,  "[0m");
    strcpy(term.bold,    "[1m");
    strcpy(term.inverse, "[7m");
    strcpy(term.blink,   "[5m");
    strcpy(term.under,   "[4m");
    term.ibmAnsi  =      1;
    term.ibmColor =      0;
    break;

    case 2: /* COLOR */
    strcpy(term.normal,  "[0;1;37;40m");
    strcpy(term.bold,    "[1;33;40m");
    strcpy(term.inverse, "[1;33;41m");
    strcpy(term.blink,   "[5m");
    strcpy(term.under,   "[1;35;40m");
    term.ibmAnsi  =      1;
    term.ibmColor =      1;
    break;

    case 3: /* VT100 */
    strcpy(term.normal,  "[0m");
    strcpy(term.bold,    "[1m");
    strcpy(term.inverse, "[7m");
    strcpy(term.blink,   "[5m");
    strcpy(term.under,   "[4m");
    term.ibmAnsi  =      0;
    term.ibmColor =      0;
    break;
    }
    setOutTrans(term.ibmAnsi);
}


/* -------------------------------------------------------------------- */
/*  setlogTerm()       Setup the terminal according to log.buf          */
/* -------------------------------------------------------------------- */
void setlogTerm(void)
{
    strcpy(term.normal,  logBuf.IBMANSI ? 
                         attrtoansi(logBuf.attributes[ATTR_NORMAL],1)    : "");
    strcpy(term.bold,    logBuf.IBMANSI ? 
                         attrtoansi(logBuf.attributes[ATTR_BOLD],0)      : "");
    strcpy(term.inverse, logBuf.IBMANSI ? 
                         attrtoansi(logBuf.attributes[ATTR_REVERSE],0)   : "");
    strcpy(term.blink,   logBuf.IBMANSI ? 
                         attrtoansi(logBuf.attributes[ATTR_BLINK],0)     : "");
    strcpy(term.under,   logBuf.IBMANSI ? 
                         attrtoansi(logBuf.attributes[ATTR_UNDERLINE],0) : "");

    term.ibmAnsi  =     (uchar)logBuf.IBMGRAPH;
    term.ibmColor =     (uchar)logBuf.IBMCOLOR;

    setOutTrans(term.ibmAnsi);
}

/* -------------------------------------------------------------------- */
/*  attrtoansi()       converts attribute byte into ansi sequence       */
/* -------------------------------------------------------------------- */
char *attrtoansi(uchar attr, uchar normal)
{
    char ansi_attribute;


/*
       7 6 5 4 3 2 1 0
       | \   / | \   /
       B   B   I   F
       L   A   N   O
       I   C   T   R
       N   K   E   E
       K       N   
*/                   

    static char str[15];


    union
    {

        struct {
        unsigned foreground  : 3;
        unsigned intensity   : 1;
        unsigned background  : 3;
        unsigned blink       : 1;
        } breakdown;

        uchar attr;
    } character;


    character.attr  = attr;

    if (logBuf.IBMCOLOR) /* color */
    {
        if (character.breakdown.blink)
        {
            ansi_attribute = '5';
            sprintf(str, "%c[%s%cm", 27,
            (normal) ? "0;" : "",
            ansi_attribute);
        }
        else
        if (character.breakdown.intensity) 
        {
            ansi_attribute = '1';

            sprintf(str, "%c[%s%c;%d;%dm", 27,
                (normal) ? "0;" : "",
                ansi_attribute,
                iso_clr[character.breakdown.foreground] + 30,
                iso_clr[character.breakdown.background] + 40);
        }
        else
        {
            ansi_attribute = '0';

            sprintf(str, "%c[%s%c;%d;%dm", 27, 
                (normal) ? "0;" : "",
                ansi_attribute,
                iso_clr[character.breakdown.foreground] + 30,
                iso_clr[character.breakdown.background] + 40);

        }

    }
    else /* monochrome */
    {
        if (character.breakdown.blink)     /* Blink */
            ansi_attribute = '5';
        else
        if (character.breakdown.intensity) /* Boldface */
            ansi_attribute = '1';
        else                               
        if (character.attr == 7)           /* Normal */
            ansi_attribute = '0';
        else                               
        if (character.attr == 1)           /* Underline */
            ansi_attribute = '4';
        else
        if (character.attr == 112)         /* Reverse Video */
            ansi_attribute = '7';

        sprintf(str, "%c[%cm", 27, ansi_attribute);

    }


    return str;    


#ifdef GOODBYE
    if (dirty && ((c >= 'A' && c <= 'H') || (c >= 'a' && c <= 'h')))
    {
        sprintf(str, "%c[%dm", 27, hilight ? 1 : 0);
        putCode(str);
        if (blink && !hilight)
            putCode("[5m");
        dirty = FALSE;
    }
#endif
}


#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  setTerm()       Setup the terminal                                  */
/* -------------------------------------------------------------------- */
void setTerm(char *t)
{
    int i;
    
    for (i=0; terms[i] != NULL; i++)
    {
        if (strcmpi(terms[i]->name, t) == SAMESTRING)
        {
            term = *terms[i];
            setOutTrans(term.ibmAnsi);
            return;
        }
    }
    
    term = *terms[0];
    setOutTrans(term.ibmAnsi);
}

#endif

/* -------------------------------------------------------------------- */
/*  askAttributes()   Lets the user config his own colors               */
/* -------------------------------------------------------------------- */
void askAttributes(void)
{

    doCR();

    logBuf.attributes[ATTR_NORMAL]    = 
        (uchar)getNumber("Normal",    0L, 255L, logBuf.attributes[ATTR_NORMAL]);

    logBuf.attributes[ATTR_BLINK]     =
        (uchar)getNumber("Blinking",  0L, 255L, logBuf.attributes[ATTR_BLINK]);

    logBuf.attributes[ATTR_REVERSE]   =
        (uchar)getNumber("Inverted",  0L, 255L, logBuf.attributes[ATTR_REVERSE]);

    logBuf.attributes[ATTR_BOLD]      =
        (uchar)getNumber("Boldface",  0L, 255L, logBuf.attributes[ATTR_BOLD]);

    logBuf.attributes[ATTR_UNDERLINE] =
        (uchar)getNumber("Underline", 0L, 255L, logBuf.attributes[ATTR_UNDERLINE]);


    setlogTerm();
}


#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  askTerm()       Setup the terminal                                  */
/* -------------------------------------------------------------------- */
void askTerm(void)
{
    int i, i2;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);
    
    for (i=0; terms[i] != NULL; i++)
    {
        mPrintf(" %d - %-20s %s", i+1, terms[i]->name, terms[i]->descript); 
        doCR();
    }

    i2 = (int)getNumber("Terminal", 0L, (long)i, 0L);

    if (i2)
    {
        term = *terms[i2-1];
        setOutTrans(term.ibmAnsi);
    }
}
#endif

/* -------------------------------------------------------------------- */
/*  setOutTrans()       Setup translation table                         */
/* -------------------------------------------------------------------- */
void setOutTrans(BOOL ibmAnsi)
{
    uint i;
    
    for (i=0; i<0xFF; i++)
    {
        outFilter[i] = (ibmAnsi) ? (uchar)i : filt_out[i];
    }
}

/* -------------------------------------------------------------------- */
/*  termCap()       Does a terminal command                             */
/* -------------------------------------------------------------------- */
    
void termCap(char c)
{
    char str[10];
    static BOOL hilight = FALSE,    /* hilight is on */
                blink = FALSE,      /* blinking is on */
                dirty = TRUE;       /* the hilight (remote) is dirty */
    
    /* setio(whichIO, echo, outFlag); */
    
    /*
     *  ISO COLOR support, should handle damn near any case.. 
     */
    
    /* 
     * Since the ^A0 may actualy be hilighted (or the ^A3 may not!) 
     * we need to reset the hilight state if the system is flagged
     * 'dirty'.
     *
     * Also if we reset to non-hilighted we need to reoutput the
     * blinking if it was set.
     */
    if (dirty && ((c >= 'A' && c <= 'H') || (c >= 'a' && c <= 'h')))
    {
        sprintf(str, "%c[%dm", 27, hilight ? 1 : 0);
        putCode(str);
        if (blink && !hilight)
            putCode("[5m");
        dirty = FALSE;
    }
    
    /*
     * Send the background
     */
    if (c >= 'A' && c <= 'H')
    {
        if (term.ibmColor)
        {
            sprintf(str, "%c[4%cm", ESC, '0' + (c - 'A'));
            putCode(str);
            ansiattr = (uchar)((ansiattr & 0x0F) 
                       | (iso_clr[(c - 'A')] << 4) | (blink ? 0x80 : 0));
        }
        return;
    }
    
    /*
     * Send the forground 
     */
    if (c >= 'a' && c <= 'h')
    {
        if (term.ibmColor)
        {
            sprintf(str, "%c[3%cm", ESC, '0' + (c - 'a'));
            putCode(str);
            ansiattr = (uchar)((ansiattr & 0xF0) 
                       | (iso_clr[(c - 'a')]) | (hilight ? 0x08 : 0));
        }
        return;
    }
    
    /*
     * Normal DragCit colors.. 
     */
    switch (toupper(c))
    {
    case TERM_BLINK:
        if (*term.blink)
        {
            putCode(term.blink);
            ansiattr = (uchar)(ansiattr | 128);
            blink = TRUE;
        }
        break;
        
    case TERM_REVERSE:
        if (*term.inverse)
        {
            putCode(term.inverse);
            ansiattr = cfg.wattr;
            dirty = TRUE;
        }
        break;
        
    case TERM_BOLD:
        if (*term.bold)
        {
            putCode(term.bold);
            ansiattr = cfg.cattr;
            hilight = TRUE;
            dirty = FALSE;
        }
        break;
        
    case TERM_UNDERLINE:
        if (*term.under)
        {
            putCode(term.under);
            ansiattr = cfg.uttr;
            dirty = TRUE;
        }
        break;
        
    case TERM_NORMAL:
    default:
        if (*term.normal)
        {
            putCode(term.normal);
            ansiattr = cfg.attr;
            hilight = FALSE;
            blink = FALSE;
            dirty = TRUE;
        }
        break;
    }
}

