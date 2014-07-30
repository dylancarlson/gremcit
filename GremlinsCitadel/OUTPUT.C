/* -------------------------------------------------------------------- */
/*  OUTPUT.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  This file contains the output functions                             */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  getWord()       Gets the next word from the buffer and returns it   */
/*  mFormat()       Outputs a string to modem and console w/ wordwrap   */
/*  putWord()       Writes one word to modem and console, w/ wordwrap   */
/*  asciitable()    initializes the ascii translation table             */
/*  doBS()          does a backspace to modem & console                 */
/*  doCR()          does a return to both modem & console               */
/*  dospCR()        does CR for entry of initials & pw                  */
/*  doTAB()         prints a tab to modem & console according to flag   */
/*  oChar()         is the top-level user-output function (one byte)    */
/*  updcrtpos()     updates crtColumn according to character            */
/*  mPrintf()       sends formatted output to modem & console           */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  05/26/89    (PAT)   Created from MISC.C to break that moduel into   */
/*                      more managable and logical peices. Also draws   */
/*                      off MODEM.C and FORMAT.C                        */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static data/functions                                               */
/* -------------------------------------------------------------------- */
char prtf_buff[512];

/* -------------------------------------------------------------------- */
/*  mFormat()       Outputs a string to modem and console w/ wordwrap   */
/* -------------------------------------------------------------------- */
void mFormat(char *string)
{
    static uchar wordBuf[MAXWORD + 8];
    int  i;
    
    for (i = 0;  string[i] && 
    (CANOUTPUT() || outFlag == OUTPARAGRAPH); )
    {
        if (mAbort(FALSE)) return;
        i = getWord(wordBuf, (uchar *)string, i, MAXWORD);
        if (*wordBuf == CTRL_A)
        {
            termCap(wordBuf[1]);
        }
        else
        if (*wordBuf == CTRL_B)
        {
            localTermCap(wordBuf[1]);
        }
        else
        {
            putWord(wordBuf);
        }
    }
}

/* -------------------------------------------------------------------- */
/*  asciitable()    initializes the ascii translation table             */
/* -------------------------------------------------------------------- */
void asciitable(void)
{
    int c;

    /* initialize input character-translation table:  */

    /* control chars -> nulls     */
    for (c = 0;  c < '\40'; c++)  tfilter[c] = '\0';

    /* pass printing chars        */
    for (c = '\40'; c < 128;   c++)  tfilter[c] = (char)c;
    
    /* no IBM extended in most places.. */
    for (c = 128; c < 256;   c++)  tfilter[c] = 0;

    tfilter[1   ]  = 1   ;  /* ctrl-a    = ctrl-a    */
    tfilter[27  ]  = 27  ;  /* special   = special   */
    tfilter[0x7f]  = 8   ;  /* del       = backspace */
    tfilter[8   ]  = 8   ;  /* backspace = backspace */
    tfilter[19  ]  = 'P' ;  /* xoff      = 'P'       */
    tfilter['\r']  = 10  ;  /* '\r'      = NEWLINE   */
    tfilter['\t']  = '\t';  /* '\t'      = '\t'      */
    tfilter[10  ]  = '\0';  /* newline   = null      */
    tfilter[15  ]  = 'N' ;  /* ctrlo     = 'N'       */
    tfilter[26  ]  = 26  ;  /* ctrlz     = ctrlz     */
}

/* -------------------------------------------------------------------- */
/*  doBS()          does a backspace to modem & console                 */
/* -------------------------------------------------------------------- */
void doBS(void)
{
    oChar('\b');
    oChar(' ');
    oChar('\b');
}

/* -------------------------------------------------------------------- */
/*  doCR()          does a return to both modem & console               */
/* -------------------------------------------------------------------- */
BOOL doCR(void)
{
/*  static int numLines = 0;  */
    BOOL toreturn = FALSE;
/*  char oldlst_rtn; */

    lst_rtn = FALSE;

    if ( !(CANOUTPUT()) )
    {
        return(toreturn);
    }
    
    crtColumn = 1;
    
    /* setio(whichIO, echo, outFlag); */

    domcr();
    doccr();

    if (printing)
        fprintf(printfile, "\n");

    prevChar    = ' ';

    /* pause on full screen */
    if (logBuf.linesScreen && (outFlag == OUTOK || outFlag == NOSTOP))
    {
        numLines++;
        if (numLines == (int)logBuf.linesScreen && (!logBuf.MSGPAUSE || 
!justdidpause))
        {
            /* oldlst_rtn = lst_rtn; */
            toreturn = mAbort(TRUE);
            /* lst_rtn = oldlst_rtn; */
            justdidpause = TRUE;
            numLines = 0;
            /* set it again cause it gets screwed up by the <more> */
            prevChar    = ' ';
            crtColumn = 1;
        }
        else
        {
            justdidpause = FALSE;
        }
    } 
    else 
    {
        numLines = 0;
    }

    return(toreturn);
}

/* -------------------------------------------------------------------- */
/*  dospCR()        does CR for entry of initials & pw                  */
/* -------------------------------------------------------------------- */
void dospCR(void)
{
    char oldecho;
    oldecho = echo;

    echo = BOTH;
    setio(whichIO, echo, outFlag);

    if (cfg.nopwecho == 1)  doCR(); 
    else
    {
        if (onConsole)
        {
            if (gotCarrier()) domcr();
        }
        else  doccr();
    }
    echo = oldecho;
    setio(whichIO, echo, outFlag);

}

/* -------------------------------------------------------------------- */
/*  doTAB()         prints a tab to modem & console according to flag   */
/* -------------------------------------------------------------------- */
void doTAB(void)
{
    int column, column2;

    column  = crtColumn;
    column2 = crtColumn;

    do { outCon(' '); } while ( (++column % 8) != 1);

    if (modem)
    {
        if (termTab)           outMod('\t');
        else
        do { outMod(' '); } while ((++column2 % 8) != 1);
    }
    updcrtpos('\t');
}    

/* -------------------------------------------------------------------- */
/*  echocharacter() echos bbs input according to global flags           */
/* -------------------------------------------------------------------- */
void echocharacter(char c)
{
    if (echo == NEITHER)
    {
        return;
    }
    
    /* setio(whichIO, echo, outFlag); */
    
    if (c == '\b') 
        doBS();
    else
    if (c == '\n') 
        doCR();
    else
        oChar(c);
}

/* -------------------------------------------------------------------- */
/*  oChar()         is the top-level user-output function (one byte)    */
/*        sends to modem port and console both                          */
/*        does conversion to upper-case etc as necessary                */
/*        in "debug" mode, converts control chars to uppercase letters  */
/*      Globals modified:       prevChar                                */
/* -------------------------------------------------------------------- */
void oChar(register char c)
{
    static int UpDoWn=TRUE;   /* You dont want to know */

    prevChar = c;                       /* for end-of-paragraph code    */

    if (c == 1) c = 0;

    if (c == '\t')
    {
        doTAB();
        return;
    }
   
    /* 
     * You don't want to know 
     */
    if (backout)
    {
        if (UpDoWn)
            c = (char)toupper(c);
        else
            c = (char)tolower(c);
        UpDoWn=!UpDoWn;
    }

    if (termUpper)      c = (char)toupper(c);
    
    if (c == 10 /* newline */)  c = ' ';   /* doCR() handles real newlines */

    c = outFilter[(uchar)c];
    
    /* show on console */
    if (console)  outCon(c);

    /* show on printer */
    if (printing)  fputc(c, printfile);

    /* send out the modem  */
    if (gotCarrier() && modem) outMod(c);

    updcrtpos(c);
}

/* -------------------------------------------------------------------- */
/*  updcrtpos()     updates crtColumn according to character            */
/* -------------------------------------------------------------------- */
void updcrtpos(char c)
{
    if (c == '\b') 
        crtColumn--;
    else if (c == '\t')
        while((++crtColumn  % 8) != 1);
    else if ((c == '\n') || (c == '\r')) crtColumn = 1;
    else crtColumn++;
}

/* -------------------------------------------------------------------- */
/*  mPrintf()       sends formatted output to modem & console           */
/* -------------------------------------------------------------------- */
void mPrintf(char *fmt, ... )
{
    va_list ap;

    va_start(ap, fmt);
    vsprintf(prtf_buff, fmt, ap);
    va_end(ap);

    mFormat(prtf_buff);
}

/* -------------------------------------------------------------------- */
/*  rmPrintf()       sends formatted output to modem & console          */
/* -------------------------------------------------------------------- */
void rmPrintf(char *fmt, ... )
{
    va_list ap;

    va_start(ap, fmt);
    vsprintf(prtf_buff, fmt, ap);
    va_end(ap);

    if (reverse)
    {
        stripansi(prtf_buff);
        strrev(prtf_buff);
    }

    mFormat(prtf_buff);
}

/* -------------------------------------------------------------------- */
/*  prtList()   Print a list of rooms, ext.                             */
/* -------------------------------------------------------------------- */
void prtList(char *item)
{
    static int  listCol;
    static int  first;
    static int  num;
    
    if (item == LIST_START || item == LIST_END)
    {
        if (item == LIST_END)
        {
            if (num)
            {
                rmPrintf("3.0");
                doCR();
            }
        }
        listCol = 0;
        num     = 0;
        first   = TRUE;
    }
    else
    {
        num++;
        
        if (first)
        {
            first = FALSE;
        }
        else
        {
            rmPrintf("3,0 ");
        }

        if (strlen(item) + 2 + crtColumn > termWidth)
        {
            doCR();
        }

        /* putWord((uchar *)item); */
        rmPrintf(item);

    }
}

/* -------------------------------------------------------------------- */
/*  getWord()       Gets the next word from the buffer and returns it   */
/* -------------------------------------------------------------------- */
int getWord(uchar *dest, register uchar *source, int offset, int lim)
{
    register int i = offset;

    if (source[i] == '\r' || source[i] == '\n')
    {
        i+=1;
    }
    else
    if (isspace(source[i]))
    {
        /* step over spaces */
        for (; 
                isspace(source[i]) 
             && !(source[i] == '\r' || source[i] == '\n') 
             && ((i - offset) < lim) ; i++)
            ;
    }
    else
    if (source[i] == CTRL_A || source[i] == CTRL_B)
    {
        i += 2;
    }
    else
    {
        /* step over word */
        for (;  !(isspace(source[i]))
             && ((i - offset) < lim) 
             && source[i] 
             && source[i] != CTRL_A
             && source[i] != CTRL_B; 
             i++)
            ;
    }
    
    strncpy((char *)dest, (char *)(source + offset), i - offset);

    dest[i - offset] = '\0' /*NULL*/;
    
    return(i);
}

/* -------------------------------------------------------------------- */
/*  putWord()       Writes one word to modem and console, w/ wordwrap   */
/* -------------------------------------------------------------------- */
void putWord(uchar *st)
{
    register uchar *s;
    register int  newColumn;
    
    /* setio(whichIO, echo, outFlag); */
    
    /*
     * Handle returns..
     */
    if (*st == '\r' || *st == '\n')
    {
        lst_rtn   = TRUE;
    }
    else
    if (lst_rtn)
    {
        if (isspace(*st))                   /* a space.. */
        {
            if (lst_rtn)
            {
                if (outFlag == OUTPARAGRAPH)
                {
                    outFlag = OUTOK;
                    setio(whichIO, echo, outFlag);
                }
                if (doCR()) return;
            }
            lst_rtn   = FALSE;
        }
        else                                /* a word.. */
        {
            if (!lst_space) /* turn return to space to sperate words.. */
            {
                oChar(' '); 
            }
        }
        
        lst_space = TRUE;
        lst_rtn   = FALSE;
    }
        
    /*
     * Do we need to wrap? 
     */
    for (newColumn = crtColumn, s = st;  *s; s++)  
    {
        if (*s == '\t')      while ((++newColumn % 8) != 1);
        else                 ++newColumn;
    }
    
    if (newColumn > (int)termWidth)
    {
        if (doCR()) return;

        if (isspace(*st))
        {
            return;
        }
    }
    
    /*
     * Now print it 
     */
    if (isspace(*st))                   /* a space.. */
    {
        for ( ; *st; st++)
        {
            oChar(*st);
        }
        
        lst_space = TRUE;
    }
    else                                /* a word.. */
    {
        for ( ; *st; st++)
        {
            /* worry about words longer than a line:   */
            if (crtColumn >= termWidth)
            {
                if (doCR()) return;
            }
    
            oChar(*st);
            
            lst_space = FALSE;
        }
    }
}


