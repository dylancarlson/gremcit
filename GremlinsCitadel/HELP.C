/* -------------------------------------------------------------------- */
/*  HELP.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/* BLBs, HLP, and MNU files..                                           */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  dump()      does unformatted dump of specified file                 */
/*  dumpf()     does formatted dump of specified file                   */
/*  goodbye()   prints random goodbye blurbs                            */
/*  hello()     prints random hello blurbs                              */
/*  tutorial()  dumps fomatted help files, handles wildcarding          */
/*  nochat()    Prints a nochat BLB, handle rotation                    */
/* -------------------------------------------------------------------- */


/* -------------------------------------------------------------------- */
/*  dump()      does unformatted dump of specified file                 */
/* -------------------------------------------------------------------- */
int dump(char *filename)
{
    FILE *fbuf;
    int c, returnval = TRUE;

    /* last itteration might have been N>exted */
    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR();

    if ( (fbuf = fopen(filename, "r")) == NULL)
    {
        mPrintf(" No file %s", filename);
        return(ERROR);
    }

    /* looks like a kludge, but we need speed!! */

    while ( (c = getc(fbuf) ) != ERROR && (c != 26 /* CPMEOF */ )
    && (outFlag != OUTNEXT) && (outFlag != OUTSKIP) && !mAbort(FALSE) )
    {
        if (c == '\n')  doCR();
        else            oChar((char)c);
    }

    if ( outFlag == OUTSKIP) returnval = ERROR;
    
    fclose(fbuf);

    return  returnval;
}


/* -------------------------------------------------------------------- */
/*  dumpf()     does formatted dump of specified file                   */
/* -------------------------------------------------------------------- */
dumpf(char *filename)
{
    FILE *fbuf;
    char line[MAXWORD];
    int returnval = TRUE;

    /* last itteration might have been N>exted */
    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR();

    if ( (fbuf = fopen(filename, "r")) == NULL)
    {
        mPrintf(" No helpfile %s", filename);
        return(ERROR);
    }
    /* looks like a kludge, but we need speed!! */

    while ( fgets(line, MAXWORD, fbuf) && (outFlag != OUTNEXT)
    && (outFlag != OUTSKIP) && !mAbort(FALSE) )
    {
        mFormat(line);
    }
    if ( outFlag == OUTSKIP) returnval = ERROR;
    
    fclose(fbuf);

    return  returnval;
}

/* -------------------------------------------------------------------- */
/*  tutorial()  dumps fomatted help files, handles wildcarding          */
/* -------------------------------------------------------------------- */
void tutorial(char *filename)
{
    int  i;
    char temp[14];
    char oldverbose;
    
    outFlag     = OUTOK;
    setio(whichIO, echo, outFlag);

    if (!expert)  mPrintf("\n <3J0>ump <3N0>ext <3P0>ause <3S0>top\n");
    /* doCR(); */

    if (changedir(cfg.helppath) == -1 ) return;

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR)
    {
        mPrintf(" No helpfile %s", filename);
        changedir(cfg.homepath);
        return;
    }

    if (ambig(filename))
    {
        /* fill our directory array according to filename */
        oldverbose = verbose;
        verbose = FALSE;
        filldirectory(filename);
        verbose = oldverbose;

        /* print out all the files */
        for (i = 0; filedir[i].entry[0] && 
        ( dumpf(filedir[i].entry) != ERROR) ; i++);

        if ( !i) mPrintf(" No helpfile %s", filename);

        /* free file directory structure */
        if(filedir != NULL)
        _ffree((void *)filedir);
    }
    else
    {
       strcpy(temp, filename);
       temp[strlen(temp)-1] = '@';

       if (filexists(temp) && *term.bold)
         dump(temp);
       else
         dumpf(filename);
    }

    /* go to our home-path */
    changedir(cfg.homepath);
}

/* -------------------------------------------------------------------- */
/*  hello()     Prints a login BLB, handle rotation                     */
/* -------------------------------------------------------------------- */
void hello(void)
{
    BLBRotate("hello", "blb", &cfg.normHello, &cfg.ansiHello);
}

/* -------------------------------------------------------------------- */
/*  goodbye()    Prints a logout BLB, handle rotation                   */
/* -------------------------------------------------------------------- */
void goodbye(void)
{
    BLBRotate("logout", "blb", &cfg.normBye, &cfg.ansiBye);
}

/* -------------------------------------------------------------------- */
/*  nochat()    Prints a nochat BLB, handle rotation                    */
/* -------------------------------------------------------------------- */
void nochat(BOOL reset)
{
    static int ansiChat;
    static int normChat;
    
    if (reset)
    {
        ansiChat = 0;
        normChat = 0;
    }
    else
    {
        BLBRotate("nochat", "blb", &normChat, &ansiChat);
    }
}

/* -------------------------------------------------------------------- */
/*  BLBRotate() prints random blurbs                                    */
/* -------------------------------------------------------------------- */
void BLBRotate(char *base, char *ext, int *reg, int *ansi)
{
    char fn[15];
    char ext2[4];
    int  *num;
    
    strcpy(ext2, ext);
    
    /*
     * Are we doing ansi or normal
     */
    if (changedir(cfg.helppath) == -1 ) return;
    
    sprintf(fn, "%s.%2.2s@", base, ext);
    if(*term.bold && filexists(fn))
    {
        ext2[2] = '@';
        num = ansi;
    }
    else
    {
        num = reg;
    }
        
    if(*num == 0)
    {
        sprintf(fn, "%s.%s", base, ext2);
        dumpf(fn);
    }
    else
    {
        sprintf(fn, "%s%d.%s", base, *num, ext2);
        if (!filexists(fn))
        {
            sprintf(fn, "%s.%s", base, ext2);
            dumpf(fn);
            *num = 0;
        }
        else
        {
            dumpf(fn);
        }
    }
    
    (*num)++;

    changedir(cfg.homepath);
}

/* -------------------------------------------------------------------- */
/*  Usage()     Explain the command line switches.                      */
/* -------------------------------------------------------------------- */
void usage(void)
{
  puts("\n USAGE: CTDL [-D|-C|-NB|-NC|-Mn|-Bn|-A|-E|-Us|-Ps|-V|-!s|-X]");
    puts("    -D  Direct screen writes until config.cit is loaded.");
    puts("    -C  Configure the system.");
    puts("    -NB Turns off bells.");
    puts("    -NC Turns off chat.");
    puts("    -Mn Video mode number, -M1000 for EGA/VGA 43/50 line.");
    puts("    -Bn Baud rate.");
    puts("    -A  Application mode.");
    puts("    -E  Enter CONFIG.CIT file.");
    puts("    -Us 'Name' Log in using Name.");
    puts("    -Ps 'In;Pw' Log in using In;Pw.");
    puts("    -V  Debug mode.");
    puts("    -!s Network mode.");
    puts("    -X  Batch Mode.");
    puts("");
}

