/* -------------------------------------------------------------------- */
/*  EDIT.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                Message editor and related code.                      */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  editText()      handles the end-of-message-entry menu.              */
/*  putheader()     prints header for message entry                     */
/*  getText()       reads a message from the user                       */
/*  matchString()   searches for match to given string.                 */
/*  replaceString() corrects typos in message entry                     */
/*  wordcount()     talleys # lines, word & characters message containes*/
/*  fakeFullCase()  converts a message in uppercase-only to a           */
/*  xPutStr()       Put a string to a file w/o trailing blank           */
/*  GetFileMessage()    gets a null-terminated string from a file       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  06/06/89    (PAT)   Made history, cleaned up comments, reformated   */
/*                      icky code.                                      */
/*  06/18/89    (LWE)   Added wordwrap to message entry                 */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */


void cyclesignature(void)
{
    FILE *fBuf;
    char line[256];
    char *words[256];
    int  found = FALSE;
    char path[80];

#ifdef GOODBYE
    mPrintf("cfg.num %d", cfg.sig_num);
#endif

#ifdef GOODBYE
    if (cfg.sig_current_pos == 100000000L)
    {
        mPrintf("Only one");
    }
#endif

    /* there is only one signature line, so get out and use it */
    if (cfg.sig_current_pos == 100000000L)
        return;

    sprintf(path, "%s\\config.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {
        return;
    }

    fseek(fBuf, cfg.sig_current_pos, SEEK_SET);

    for (;!found;)
    {                                      
        /* if end of file cycle back up */
        if (fgets(line, 254, fBuf) == NULL)
        {
            fseek(fBuf, cfg.sig_first_pos, SEEK_SET);
            continue;
        }

        cfg.sig_current_pos = ftell(fBuf);

        if (line[0] != '#')  continue;

        if (strnicmp(line, "#SIGNATURE", 10) != SAMESTRING)
        {

#ifdef GOODBYE
            /* sad;lksdaj;lsadkj;aldfskj */
            mPrintf("Cycle back up\n");
            /* adl;skjdsfal;skj;slfdakjadfs;lfds */
#endif

            /* okay we've gone past the last signature, cycle back up */
            fseek(fBuf, cfg.sig_first_pos, SEEK_SET);
            continue;
        }
        else
            found = TRUE;
       
        parse_it( words, line);

        strcpy(cfg.nodeSignature, words[1]);

        cfg.sig_current_pos = ftell(fBuf);

    }
    fclose(fBuf);
}

/* -------------------------------------------------------------------- */
/*  editText()      handles the end-of-message-entry menu.              */
/*      return TRUE  to save message to disk,                           */
/*             FALSE to abort message, and                              */
/*             ERROR if user decides to continue                        */
/* -------------------------------------------------------------------- */
int editText(char *buf, int lim)
{
    char ch, x;
    FILE *fd;
    char stuff[100];
    label tmp1, tmp2;
    char ich;
    int i;

    strcpy(gprompt, "2Entry cmd:0 ");
    dowhat = PROMPT;

    do
    {
        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);

        while (MIReady()) getMod();
        doCR();
        mPrintf("2Entry cmd:0 ");
        switch ( ch = (char)toupper(ich = (char)iCharNE()) )
        {
        case 'A':
            mPrintf("Abort\n ");
            if  (!strlen(buf)) return FALSE;
            if (getYesNo(confirm, 0))
            {
                heldMessage = TRUE;

                memcpy( msgBuf2, msgBuf, sizeof(struct msgB) );

                dowhat = DUNO;
                return FALSE;
            }
            break;

#ifdef GOODBYE
        case 'C':
            mPrintf("Continue");
            doCR();
            return ERROR;
#endif
        case 'C':
            mPrintf("Continue");
            for( i = strlen(buf); i > 0 && buf[i-1] == 10; i--)
                buf[i-1] = 0;
            doCR();
            doCR();
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);

            putheader(FALSE);
            doCR();
            mFormat(buf);
            outFlag = IMPERVIOUS;
            setio(whichIO, echo, outFlag);

            return ERROR;

        case 'F':
            mPrintf("Find & Replace text\n ");
            replaceString(buf, lim, TRUE);
            break;

        case 'P':
            mPrintf("Print formatted\n ");
            doCR();
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);

            putheader(FALSE);
            doCR();
            mFormat(buf);
            if ((msgBuf->mbsig[0] || msgBuf->mbusig[0]) && cfg.nodeSignature[0])
            {
                termCap(TERM_BOLD);
                doCR(); /* livia nagged */
                doCR(); mPrintf("------");
                termCap(TERM_NORMAL);
                if (*msgBuf->mbusig)
                {
                    doCR();
                    mPrintf(msgBuf->mbusig);
                }
                if (*msgBuf->mbsig)
                {
                    doCR();
                    mPrintf(msgBuf->mbsig);
                }
            }
            termCap(TERM_NORMAL);
            outFlag = IMPERVIOUS;
            setio(whichIO, echo, outFlag);

            doCR();
            break;

        case 'R':
            mPrintf("Replace text\n ");
            replaceString(buf, lim, FALSE);
            break;

        case 'S':
            if (cfg.msgNym)
            {
                mPrintf("%s %s", cfg.msg_done, cfg.msg_nym);
                doCR();
            }
            else
            {
                mPrintf("Save");
                wordcount(buf);
            }
            entered++;  /* increment # messages entered */
            dowhat = DUNO;
            cyclesignature(); /* this loads a new signature from config.cit */
            return TRUE;

        case 'W':
            mPrintf("Word count\n ");
            wordcount(buf);
            break;

        case '?':
            oChar('?');
            tutorial("edit.mnu");
            break;

        case '!':
            mPrintf("Subject / Signature\n ");
            getString("subject",  stuff,  NAMESIZE, FALSE, ECHO, "");
            if (*stuff)
            {
                strcpy(msgBuf->mbsub, stuff);
                normalizeString(msgBuf->mbsub);
            }

            getString("signature", stuff, NAMESIZE, FALSE, ECHO, "");
            if (*stuff)
            {
                strcpy(msgBuf->mbusig, stuff);
                normalizeString(msgBuf->mbusig);
            }

            break;

        case 'U':
            mPrintf("Subject\n ");
            getString("subject",   stuff,  NAMESIZE, FALSE, ECHO, "");
            if (*stuff)
            {
                strcpy(msgBuf->mbsub, stuff);
                normalizeString(msgBuf->mbsub);
            }
            break;

        case 'I':
            mPrintf("Signature\n ");
            getString("signature", stuff, NAMESIZE, FALSE, ECHO, "");
            if (*stuff)
            {
                strcpy(msgBuf->mbusig, stuff);
                normalizeString(msgBuf->mbusig);
            }
            break;

        case '@':
            mPrintf("Address\n ");
            getString("destinaton user", stuff,  NAMESIZE, FALSE, ECHO, "");
            if (*stuff)
            {
                strcpy(msgBuf->mbto, stuff);
                normalizeString(msgBuf->mbto);
            }

            getString("node", stuff, NAMESIZE, FALSE, ECHO, "");

            if (*stuff)
            {
                strcpy(msgBuf->mbzip, stuff);
                normalizeString(msgBuf->mbzip);
            }

            getString("region", stuff, NAMESIZE, FALSE, ECHO, "");

            if (*stuff)
            {
                strcpy(msgBuf->mbrzip, stuff);
                normalizeString(msgBuf->mbrzip);
            }

            break;

        case '<':
            if (sysop && onConsole)
            {
                mPrintf("File Input"); doCR();

                getString("Filename", stuff, 
                          64, FALSE, ECHO, "Message.apl");
                normalizeString(stuff);
                if (!strlen(stuff)) break;
                    
                if ((fd = fopen(stuff, "rb")) != NULL)
                {
                    GetFileMessage(fd, msgBuf->mbtext, MAXTEXT -1);
                    fclose(fd);
                    /* unlink("message.apl"); */
                }
                else
                {
                    mPrintf("File not found"); doCR();
                }
            }
            else        /* fix this later */
            {
                oChar(ich);
                if (!expert) tutorial("edit.mnu");
                else mPrintf("\n '?' for menu.\n \n");
            }
            break;
        case '*':
            if (sysop)
            {
                mPrintf("Name Change"); doCR();

                getString("title", stuff, 
                          NAMESIZE, FALSE, ECHO, msgBuf->mbtitle);

                if (*stuff)
                {
                    strcpy(msgBuf->mbtitle, stuff);
                    normalizeString(msgBuf->mbtitle);
                }


                strcpy(stuff, msgBuf->mbauth);
                getString("name", stuff, 
                          NAMESIZE, FALSE, ECHO, msgBuf->mbauth);
                if (*stuff)
                {
                    strcpy(msgBuf->mbauth, stuff);
                    normalizeString(msgBuf->mbauth);
                }


                strcpy(stuff, msgBuf->mbsur);
                getString("surname", stuff, 
                          NAMESIZE, FALSE, ECHO, msgBuf->mbsur);
                if (*stuff)
                {
                    strcpy(msgBuf->mbsur, stuff);
                    normalizeString(msgBuf->mbsur);
                }
                
                break;
            }
            /* fall through */

        default:
            if ( (x = (char)strpos((char)tolower(ch), editcmd)) != 0 )
            {
                x--;
                mPrintf("%s", edit[x].ed_name); doCR();
                if (edit[x].ed_local && !onConsole)
                {
                    mPrintf("\n Local editor only!\n ");
                }
                else
                {
                    changedir(cfg.aplpath);
                    if ((fd = fopen("message.apl", "wb")) != NULL)
                    {
                        xPutStr(fd, msgBuf->mbtext);
                        fclose(fd);
                    }
                    sprintf(tmp1, "%d", cfg.mdata);
                    sprintf(tmp2, "%u", bauds[speed]);
                    if (onConsole)
                    {
                        strcpy(tmp1, "LOCAL");
                    }
                    sformat(stuff, edit[x].ed_cmd, "fpsa", "message.apl",
                            tmp1, tmp2, cfg.aplpath);
                    readMessage = FALSE;
                    
                    apsystem(stuff);
                    
                    
                    if ((fd = fopen("message.apl", "rb")) != NULL)
                    {
                        GetFileMessage(fd, msgBuf->mbtext, MAXTEXT -1);
                        fclose(fd);
                        unlink("message.apl");
                    }
                    if (debug) cPrintf("(%s)", stuff);
                    changedir(cfg.homepath);
                }
                break;
            }
            oChar(ich);
            if (!expert) tutorial("edit.mnu");
            else mPrintf("\n '?' for menu.\n \n");
            break;
        }
    }
    while (CARRIER);
    dowhat = DUNO;
    return FALSE;
}


/* -------------------------------------------------------------------- */
/*  putheader()     prints header for message entry                     */
/* -------------------------------------------------------------------- */
void putheader(BOOL first)
{
    char dtstr[80];
    int oldEcho = echo;
    
    if (first) 
    {
        echo = BOTH;
        setio(whichIO, echo, outFlag);
    }
    
    strftime(dtstr, 79, (loggedIn) ? logBuf.dstamp : cfg.datestamp, 0l);

    termCap(TERM_BOLD);
    mPrintf("    %s", dtstr);
    
    mPrintf(" From ");
    
    if (!roomBuf.rbflags.ANON)
    {
        if (msgBuf->mbtitle[0] && logBuf.DISPLAYTS
           && (
                (cfg.titles && !(msgBuf->mboname[0])) 
                || cfg.nettitles
              )
           )
        {
             mPrintf( "[%s03] ", msgBuf->mbtitle);
        }
        
        mPrintf("4%s03", msgBuf->mbauth);
        
        if (msgBuf->mbsur[0] && logBuf.DISPLAYTS
           && (
                (cfg.surnames && !(msgBuf->mboname[0])) 
                || cfg.netsurname
              )
           )
        {
             mPrintf( " [%s03]", msgBuf->mbsur);
        }
    }
    else
    {
        /* mPrintf("****"); */
        mPrintf("4%s03", cfg.anonauthor);
    }
        
    if (msgBuf->mbto[0])    mPrintf(" To %s03", msgBuf->mbto);
    if (msgBuf->mbfwd[0])   mPrintf(" Forwarded to %s03", msgBuf->mbfwd);
    if (msgBuf->mbzip[0])   mPrintf(" @ %s", msgBuf->mbzip);
    if (msgBuf->mbrzip[0])  mPrintf(", %s", msgBuf->mbrzip);
    if (msgBuf->mbczip[0])  mPrintf(", %s", msgBuf->mbczip);
    if (msgBuf->mbgroup[0]) mPrintf(" (%s Only)", msgBuf->mbgroup);
    if (msgBuf->mbsub[0])
    {
        doCR();
        mPrintf("    Subject: 4%s0", msgBuf->mbsub);
    }
    termCap(TERM_NORMAL);
    
    echo = (uchar)oldEcho;
    setio(whichIO, echo, outFlag);

}

/* -------------------------------------------------------------------- */
/*  getText()       reads a message from the user                       */
/*                  Returns TRUE if user decides to save it, else FALSE */
/* -------------------------------------------------------------------- */
BOOL getText(void)
{
    char temp, done, d, c = 0, *buf, beeped = FALSE;
    int  i, toReturn, lim, wsize = 0,j;
    char /* lastCh, */ word[50];

    if (!expert)
    {
        tutorial("entry.blb");
        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);

        doCR();
        mPrintf(" You have up to %d characters.", MAXTEXT );
        mPrintf("\n Enter %s (end with empty line).", cfg.msg_nym);
    }

    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    doCR();
    putheader(TRUE);
    doCR();

    buf     = msgBuf->mbtext;
    
    if (oldFlag) 
    {
        int i;

        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);
      
        for( i = strlen(buf); i > 0 && buf[i-1] == 10; i--)
            buf[i-1] = 0;
        mFormat(msgBuf->mbtext);

        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);

    }

    /* lastCh  = 10 */ /* NEWLINE */;

    lim     = MAXTEXT - 1;

    done = FALSE;

    asciitable();
    for (i=127; i<256; i++)
        tfilter[i] = (char)i;
    
    do
    {
        i = strlen(buf);
        /* if (i) lastCh = buf[i]; */

        while (!done && i < lim && CARRIER )
        {
            /* if (i) lastCh = c; */
            c = (char)iChar();

            switch(c)  /* Analyse what they typed       */
            {
            case CTRL_A:                /* CTRL-A>nsi   */
                temp = echo;
                echo = NEITHER;
                setio(whichIO, echo, outFlag);

                d = (char)iChar();
                echo = temp;
                setio(whichIO, echo, outFlag);


                if (d == '?')
                {
                    tutorial("ansi.hlp");
                    outFlag = OUTOK;
                    setio(whichIO, echo, outFlag);
            
                    for( i = strlen(buf); i > 0 && buf[i-1] == 10; i--)
                    buf[i-1] = 0;
                    doCR();
                    doCR();
                    outFlag = OUTOK;
                    setio(whichIO, echo, outFlag);

                    putheader(FALSE);
                    doCR();
                    mFormat(buf);
                    outFlag = IMPERVIOUS;
                    setio(whichIO, echo, outFlag);
                }
                else if (((d >= '0' && d <= '4') ||
                          (d >= 'A' && d <= 'H') ||
                          (d >= 'a' && d <= 'h'))  /* && ansiOn */)
                {
                    termCap(d);
                    buf[i++]   = 0x01;
                    buf[i++]   = d;
                }
                else 
                {
                    oChar(7);
                }
                break;
                
             case  10:                  /* NEWLINE      */
                if ( /* (lastCh == 10) || */ (i == 0) ||
                     (i && buf[i-1] == 10 )) done = TRUE;
                if (!done) buf[i++] = 10; /* A CR might be nice   */
                break;
                
             case  ESC:                 /* An ESC? no, No, NO!  */
                oChar(7);    
                break;
                
             case  26:                  /* CTRL-Z       */
                done = TRUE;
                entered++;  /* increment # messages entered */
                break;  
                
             case '\b':                 /* Backspace */
                if (i > 0 && buf[i-1] == '\t')  /* TAB  */
                {
                    i--;
                    while ( (crtColumn % 8) != 1)  doBS();
                }
                else
                    if (i > 0 && buf[i-1] != 10)
                    {
                        i--;
                        if(wsize > 0)  wsize--;
                    }
                    else
                    {
                        oChar(32);
                        oChar(7);
                    }
                break;
             
             default:
                if (c == '\r' || c == '\n' || c == ' ' || c == '\t' || wsize == 50)
                {
                    wsize = 0;
                }
                else
                {
                    if (crtColumn >= (uchar)(termWidth-1))
                    {
                        if (wsize)
                        {
                            for (j = 0; j < (wsize+1); j++)
                                doBS();
                            doCR();
                            for (j = 0; j < wsize; j++)
                                echocharacter(word[j]);
                            echocharacter(c);
                        }
                        else
                        {
                            doBS();
                            doCR();
                            echocharacter(c);
                        }

                        wsize = 0;
                    }
                    else
                    {
                        word[wsize] = (char)c;
                        wsize ++;
                    }
                }

                if (c != 0) buf[i++]   = c;
                if (i > MAXTEXT - 80 && !beeped)
                {
                    beeped = TRUE;
                    oChar(7 /* BELL */);
                }
                break;
            }
           
            buf[i] = 0x00;              /* null to terminate message     */
            /* if (i) lastCh = buf[i-1]; */

            if (i == lim)   mPrintf(" Buffer overflow.\n ");
        }
        done = FALSE;                   /* In case they Continue         */
        
        termCap(TERM_NORMAL);

        if (c == 26 && i != lim)        /* if was CTRL-Z        */
        {
           buf[i++] = 10;               /* end with NEWLINE     */
           buf[i] = 0x00;
           toReturn = TRUE;             /* yes, save it         */
           doCR();

           if (strlen(buf) > 1)
               cyclesignature();  /* this loads a new signature from config.cit */

           if (cfg.msgNym)
           {
               mPrintf(" %s %s", cfg.msg_done, cfg.msg_nym);
               doCR();
           }
           else
           {
               mPrintf(" Saved ");
               wordcount(buf);
           }
        }
        else                           /* done or lost carrier */
        {
            asciitable();
            toReturn = editText(buf, lim);
            asciitable();
            for (i=127; i<256; i++)
                tfilter[i] = (char)i;
        }

    }   
    while ((toReturn == ERROR)  &&  CARRIER);
    /* ERROR returned from editor means continue    */

    asciitable();
    
    if (toReturn == TRUE || toReturn == ERROR)     /* Filter null messages */
    {   
        toReturn = FALSE;
        for (i = 0; buf[i] != 0 && !toReturn; i++)
            toReturn = (buf[i] > ' ');
    }
    return  (BOOL)toReturn;
}

/* -------------------------------------------------------------------- */
/*  matchString()   searches for match to given string.                 */
/*                  Runs backward  through buffer so we get most recent */
/*                  error first. Returns loc of match, else ERROR       */
/* -------------------------------------------------------------------- */
char *matchString(char *buf, char *pattern, char *bufEnd, char ver)
{
    char *loc, *pc1, *pc2;
    char subbuf[10];
    char foundIt;

    for (loc = bufEnd, foundIt = FALSE;  !foundIt && --loc >= buf;)
    {
        for (pc1 = pattern, pc2 = loc,  foundIt = TRUE ;  *pc1 && foundIt;)
        {
            if (! (tolower(*pc1++) == tolower(*pc2++)))   foundIt = FALSE;
        }
        if (ver && foundIt)
        {
          doCR();
          strncpy(subbuf,
             buf + 10 > loc ? buf : loc - 10,
             (unsigned)(loc - buf) > 10 ? 10 : (unsigned)(loc - buf));
          subbuf[(unsigned)(loc - buf) > 10 ? 10 : (unsigned)(loc - buf)] = 0;
          mPrintf("%s", subbuf);
          if (*term.bold)
              termCap(TERM_BOLD);
          else
              mPrintf(">");
          mPrintf("%s", pattern);
          if (*term.bold)
              termCap(TERM_NORMAL);
          else
              mPrintf("<");
          strncpy(subbuf, loc + strlen(pattern), 10 );
          subbuf[10] = 0;
          mPrintf("%s", subbuf);
          if (!getYesNo("Replace", 0))
            foundIt = FALSE;
        }
    }
    return   foundIt  ?  loc  :  NULL;
}

/* -------------------------------------------------------------------- */
/*  replaceString() corrects typos in message entry                     */
/* -------------------------------------------------------------------- */
void replaceString(char *buf, int lim, char ver)
{
    char oldString[260];
    char newString[260];
    char *loc, *textEnd;
    char *pc;
    int  incr, length;
    char *matchString();
                                                  /* find terminal null */
    for (textEnd = buf, length = 0;  *textEnd;  length++, textEnd++);

    getString("text",      oldString, 256, FALSE, ECHO, "");
    if (!*oldString)
    {
        mPrintf(" Text not found.\n");
        return;
    }

    if ((loc=matchString(buf, oldString, textEnd, ver)) == NULL)
    {
        mPrintf(" Text not found.\n ");
        return;
    }

    getString("replacement text", newString, 256, FALSE, ECHO, "");

#ifdef GOODBYE
    if    (   strlen(newString) > strlen(oldString)
    && (((int)strlen(newString) - strlen(oldString))  >=  (lim - length)))
    {
        mPrintf(" Buffer overflow.\n ");
        return;
    }
#endif

    if  ((length + (strlen(newString) - strlen(oldString))) >= lim)
    {
        mPrintf(" Buffer overflow.\n ");
        return;
    }

    /* delete old string: */
    for (pc=loc, incr=strlen(oldString); (*pc=*(pc+incr)) != 0;  pc++);
    textEnd -= incr;

    /* make room for new string: */
    for (pc=textEnd, incr=strlen(newString);  pc>=loc;  pc--)
    {
        *(pc+incr) = *pc;
    }

    /* insert new string: */
    for (pc=newString;  *pc;  *loc++ = *pc++);
}

/* -------------------------------------------------------------------- */
/*  wordcount()     talleys # lines, word & characters message containes*/
/* -------------------------------------------------------------------- */
void wordcount(char *buf)
{
    char *counter;
    int lines = 0, words = 0, chars;

    counter = buf;

    chars = strlen(buf);

    while(*counter++)
    {
         if (*counter == ' ')
         {
             if ( ( *(counter - 1) != ' ') && ( *(counter - 1) != '\n') ) 
             words++;
         }

         if (*counter == '\n')
         {
             if ( ( *(counter - 1) != ' ') && ( *(counter - 1) != '\n') ) 
             words++;
             lines++;
         } 

    }
    mPrintf(" %d lines, %d words, %d characters", lines, words, chars);
    doCR();
}

/* -------------------------------------------------------------------- */
/*  fakeFullCase()  converts a message in uppercase-only to a           */
/*      reasonable mix.  It can't possibly make matters worse...        */
/*      Algorithm: First alphabetic after a period is uppercase, all    */
/*      others are lowercase, excepting pronoun "I" is a special case.  */
/*      We assume an imaginary period preceding the text.               */
/* -------------------------------------------------------------------- */
void fakeFullCase(char *text)
{
    char *c;
    char lastWasPeriod;
    char state;

    for(lastWasPeriod=TRUE, c=text;   *c;  c++)
    {
        if ( (*c != '.') && (*c != '?') && (*c != '!') )
        {
            if (isalpha(*c))
            {
                if (lastWasPeriod)  *c = (char)toupper(*c);
                else                *c = (char)tolower(*c);
                lastWasPeriod          = FALSE;
            }
        } else
        {
            lastWasPeriod       = TRUE ;
        }
    }

    /* little state machine to search for ' i ': */
    #define NUTHIN          0
    #define FIRSTBLANK      1
    #define BLANKI          2
    for (state=NUTHIN, c=text;  *c;  c++)
    {
        switch (state)
        {
        case NUTHIN:
            if (isspace(*c))  state   = FIRSTBLANK;
            else              state   = NUTHIN    ;
            break;
        case FIRSTBLANK:
            if (*c == 'i')    state   = BLANKI    ;
            else              state   = NUTHIN    ;
            break;
        case BLANKI:
            if (isspace(*c))  state   = FIRSTBLANK;
            else              state   = NUTHIN    ;

            if (!isalpha(*c)) *(c-1)  = 'I';
            break;
        }
    }
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  xPutStr()       Put a string to a file w/o trailing blank           */
/* -------------------------------------------------------------------- */
void xPutStr(FILE *fl, char *str)
{
    while(*str)
    {
        fputc(*str, fl);
        str++;
    }
}
#endif

/* -------------------------------------------------------------------- */
/*  xPutStr()       Put a string to a file w/o trailing blank           */
/* -------------------------------------------------------------------- */
void xPutStr(FILE *fl, char *str)
{
    uint len = 0;
    char *ptr;

    while(*str)
    {
        if ((ptr = strstr(str, "\n ")) != NULL)
        {
            len = (uint)(ptr - str);
            if (len < termWidth)
            {
                xPutLine(fl, str, len);
                fputc(' ', fl);      /* put space after \n */
                str = ptr + 2;       /* +2 = strlen("\n ") */
                continue;            /* quite vital here */
            }
        }
        if (strlen(str) < termWidth)
        {
             if (*str)
                 xPutLine(fl, str, strlen(str)-1); /* -1 for last \n */
             return;
        }
        else
        {
             ptr = str + termWidth;
             while (*ptr != ' ' && *ptr != '\n' && *ptr != '\t')
                 ptr--;                 /* go back to start of word */
             len = (uint)(ptr - str);           /*        ^, not  ^ or ^    */
             xPutLine(fl, str, len);
             str = ptr + 1;             /* move to next line, +1 for space */
        }
    }
}

/* -------------------------------------------------------------------- */
/*  xPutLine()      Put a line of a string to a file w/o trailing blank */
/* -------------------------------------------------------------------- */
void xPutLine(FILE *fl, const char *str, int len)
{
    for (; len; len--, str++)
    {
        if (*str == '\n')      /* any "\n " pair would be done outside */
            fputc(' ', fl);    /* of this, so change '\n' to ' ' */
        else
            fputc(*str, fl);
    }
    fputc('\r', fl);                    /* DOS wants returns at EOL */
    fputc('\n', fl);                    /* everyone likes linfeeds  */
}

/* -------------------------------------------------------------------- */
/*  GetFileMessage()    gets a null-terminated string from a file       */
/* -------------------------------------------------------------------- */
void GetFileMessage(FILE *fl, char *str, int mlen)
{
    int l = 0, i;
    char ch;
 
    asciitable();
    for (i=127; i<256; i++)
        tfilter[i] = (char)i;
    tfilter['\r']  = '\0';
    tfilter[10  ]  = 10;
    tfilter[0xFF]  = '\0';
 
    while(!feof(fl) && l < mlen)
    {
        if ((ch = tfilter[(uchar)fgetc(fl)]) != '\0' && !feof(fl))
        {
            str[l] = ch;
            l++;
        }
    }
 
    str[l]='\0';
 
    asciitable();
}

