/* -------------------------------------------------------------------- */
/*  MSGREAD.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                       Overlayed message code                         */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  printMessage()  prints message on modem and console                 */
/*  stepMessage()   find the next message in DIR                        */
/*  showMessages()  is routine to print roomful of msgs                 */
/*  printheader()   prints current message header                       */
/*  getMessage()    reads a message off disk into RAM.                  */
/*  getMsgStr()     reads a NULL terminated string from msg file        */
/*  getMsgChar()    reads a character from msg file, curent position    */
/* -------------------------------------------------------------------- */

ulong   copyOf;
char pausemsg = FALSE;
/* char bunga = FALSE; */

/* -------------------------------------------------------------------- */
/*  printMessage()  prints message on modem and console                 */
/* -------------------------------------------------------------------- */
void printMessage(ulong id)
{
    ulong   here;
    long    loc;
    ulong result;

#ifdef NEWMSGTAB
    ulong    slot;
#else
    int     slot;
#endif

    /* char oldlst_rtn; */

    static  level = 0;
    /* bunga = FALSE; */


    result = indexslot(id);
    if (result == ULONG_ERROR) return;




#ifdef NEWMSGTAB
    slot = result;
/*    if ((slot = indexslot(id)) == ERROR) return;  */
#else

    slot = (int)result;
/*   if ((slot = (int)indexslot(id)) == ERROR) return; */
#endif


    if (!level)
    {
        originalId   = id;
    }

#ifdef NEWMSGTAB
    if (getFlags(slot)->COPY)
#else
    if (msgTab_mtmsgflags[slot].COPY)
#endif
        

    {
        copyflag     = TRUE;
        originalattr = 0;

        if (level == 0)

#ifdef NEWMSGTAB
        /*  copyOf = (ulong)(id - (ulong)getOriginID(slot));   */
        /*    copyOf = (ulong)(id - getLocation(slot)); */  
      copyOf = (ulong)(id - long_JOIN(getToHash(slot), getAuthHash(slot)));   
#else
/*           copyOf = (ulong)(id - (ulong)msgTab_mtomesg[slot]);  */

            copyOf = 
(id - long_JOIN(msgTab_mttohash[(int)slot], msgTab_mtauthhash[(int)slot]));

#endif


        /*  copyOf = (ulong)(id - (ulong)msgTab_mtoffset[slot]); */


#ifdef NEWMSGTAB
        originalattr = (uchar)
       (originalattr | (getFlags(slot)->RECEIVED)?ATTR_RECEIVED :0 );

        originalattr = (uchar)
       (originalattr | (getFlags(slot)->REPLY   )?ATTR_REPLY : 0 );

        originalattr = (uchar)
       (originalattr | (getFlags(slot)->MADEVIS )?ATTR_MADEVIS : 0 );

#else
        originalattr = (uchar)
       (originalattr | (msgTab_mtmsgflags[slot].RECEIVED)?ATTR_RECEIVED :0 );

        originalattr = (uchar)
       (originalattr | (msgTab_mtmsgflags[slot].REPLY   )?ATTR_REPLY : 0 );

        originalattr = (uchar)
       (originalattr | (msgTab_mtmsgflags[slot].MADEVIS )?ATTR_MADEVIS : 0 );

#endif
        

        level ++;

        if (level > 20)
        {
            level = 0;
            return;
        }
        /* mtoffset */

#ifdef NEWMSGTAB
/*      if ((int)getOriginID(slot) <= slot)
            printMessage( (ulong)(id - (ulong)getOriginID(slot)));   */

        if (long_JOIN(getToHash(slot), getAuthHash(slot)) <= slot)
            printMessage( (id - long_JOIN(getToHash(slot), getAuthHash(slot))));
#else
/*      if ((int)msgTab_mtomesg[slot] <= slot)
            printMessage( (ulong)(id - (ulong)msgTab_mtomesg[slot]));  */
#ifdef GOODBYE
        if ((int)msgTab_mtomesg[slot] <= slot)
            printMessage( (ulong)(id - (ulong)long_JOIN(msgTab_mtmsgLocLO[(int)slot], msgTab_mtmsgLocHI[(int)slot])));
#endif


   if ((int)long_JOIN(msgTab_mttohash[(int)slot], msgTab_mtauthhash[(int)slot]) <= slot)
     printMessage( (ulong)(id - (ulong)
long_JOIN(msgTab_mttohash[(int)slot], msgTab_mtauthhash[(int)slot])));
#endif


        level = 0;

        return;
    }

    /* in case it returns without clearing buffer */
    msgBuf->mbfwd[  0]  = '\0';
    msgBuf->mbto[   0]  = '\0';


#ifdef NEWMSGTAB
    loc  = getLocation(slot);
#else
    loc  = long_JOIN(msgTab_mtmsgLocLO[slot], msgTab_mtmsgLocHI[slot]); 
#endif

    if (loc == ULONG_ERROR) return;


#ifdef NEWMSGTAB
    if (copyflag)  slot = indexslot(originalId);
#else
    if (copyflag)  slot = (int)indexslot(originalId);
#endif


    if (!mayseeindexmsg(slot) )
    {
        return;
    }

    fseek(msgfl, loc, 0);

    getMessage();

    dotoMessage = NO_SPECIAL;

    sscanf(msgBuf->mbId, "%lu", &here);

    /* cludge to return on dummy msg #1 */
    if (/* (int)here == 1 || */ !mayseemsg())
    {
        return;
    }

    if (here != id )
    {
        mPrintf("Can't find message. Looking for %lu at byte %ld!\n ",
                 id, loc);
        return;
    }
    
    getMsgStr(msgBuf->mbtext, MAXTEXT); 

    if (mf.mfSearch[0])
    {
        if (   !u_match(msgBuf->mbtext, mf.mfSearch) )
            return;
    }
    
    mread++; /* Increment # messages read */

    if(logBuf.MSGCLS && logBuf.IBMANSI)
    {
        numLines = 0;
        mPrintf("[2J");
    }
    
    printheader( id, slot);

    seen = TRUE;

    if (msgBuf->mblink[0])
    {
        dumpf(msgBuf->mblink);
    }
    else
    {
        /* NEW MESSAGE READ */
        mFormat(msgBuf->mbtext);

        doCR();

#ifdef GOODBYE

        /* total cludge, I want to hit a CR for messages that don't */
        /* end with return. hope this works */
        if (!lst_rtn)
        {
            doCR();
            bunga = TRUE;
        }      
#endif
    }
    if ((msgBuf->mbsig[0] || msgBuf->mbusig[0]) && logBuf.SIGNATURES
      /* && cfg.nodeSignature[0] */)
    {
        termCap(TERM_BOLD);

        /* doCR(); */ /* livia nagged */

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
        doCR();
    }
    termCap(TERM_NORMAL);
    /* doCR(); */
    echo = BOTH;
    setio(whichIO, echo, outFlag);

    if (outFlag == OUTOK && 
        logBuf.MSGPAUSE && pausemsg && (!justdidpause || !logBuf.linesScreen))
    {
        mAbort(TRUE); 
        justdidpause = TRUE; 
        numLines = 0;
        pausemsg = FALSE;
    }
}


#ifdef GOOGOODDBYE
        /* this is fucking bullshit */


#ifdef GOODBYE
        if (!((msgBuf->mbsig[0] || msgBuf->mbusig[0]) && logBuf.SIGNATURES)
        && !bunga)
            doCR();
#endif

        if (outFlag == OUTOK && (!justdidpause || !logBuf.linesScreen))  
        { 
            /* oldlst_rtn = lst_rtn; */
            mAbort(TRUE); 
            /* lst_rtn = oldlst_rtn; */
            justdidpause = TRUE; 
        }
            numLines = 0;
            pausemsg = FALSE;
        }


}
#endif


#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  stepMessage()   find the next message in DIR                        */
/* -------------------------------------------------------------------- */
static BOOL stepMessage(ulong *at, int dir)
{
   ulong i = indexslot(*at);
   ulong tablesize = sizetable();


    for ( (1 == dir) ? ++i : --i;
         i < tablesize;      /* if --0, (i = 0xFFFFFFFF) > sizetable() */
        (1 == dir) ? ++i : --i)
    {
        /* skip messages not in this room */
      if (getRoomNum(i) != thisRoom) continue;

        /* skip by special flag */
      if (mf.mfMai && !getFlags(i)->MAIL) continue;
      if (mf.mfLim && !getFlags(i)->LIMITED) continue;
      if (mf.mfPub && (getFlags(i)->LIMITED || getFlags(i)->MAIL)) continue;

        if (mayseeindexmsg(i))
        {
         *at = cfg.mtoldest + i;
         return(TRUE);
        }
    }
   return(FALSE);
}

#endif

/* -------------------------------------------------------------------- */
/*  stepMessage()   find the next message in DIR                        */
/* -------------------------------------------------------------------- */
BOOL stepMessage(ulong *at, int dir)
{
   ulong i; 
   ulong tablesize; 

   if (*at < cfg.oldest) i = ULONG_ERROR;
   else                  i = indexslot(*at);

   tablesize = sizetable();

    for ( (1 == dir) ? ++i : --i;
         i < tablesize;      /* if --0, (i = 0xFFFFFFFF) > sizetable() */
        (1 == dir) ? ++i : --i)
    {

#ifdef GOODBYE
    for (i = indexslot(*at), i += dir; i > -1 && i < sizetable(); i += dir)
    {
#endif

        /* skip messages not in this room */

#ifdef NEWMSGTAB
        if (getRoomNum(i) != thisRoom) continue;
#else
        if (msgTab_mtroomno[(int)i] != thisRoom) continue;
#endif


#ifdef NEWMSGTAB
        /* skip by special flag */
        if (mf.mfMai && !getFlags(i)->MAIL) continue;
        if (mf.mfLim && !getFlags(i)->LIMITED) continue;
        if (mf.mfPub && 
           (getFlags(i)->LIMITED || getFlags(i)->MAIL ))
#else
        /* skip by special flag */
        if (mf.mfMai && !msgTab_mtmsgflags[(int)i].MAIL) continue;
        if (mf.mfLim && !msgTab_mtmsgflags[(int)i].LIMITED) continue;
        if (mf.mfPub && 
           (msgTab_mtmsgflags[(int)i].LIMITED || msgTab_mtmsgflags[(int)i].MAIL ))

#endif

           continue;

        if (mayseeindexmsg(i))
        {
            *at = (ulong)(cfg.mtoldest + i);
            return TRUE;
        }
    }
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  showMessages()  is routine to print roomful of msgs                 */
/* -------------------------------------------------------------------- */
void showMessages(char whichMess, char revOrder)
{
    int   increment; /* i for message save to file */
    ulong lowLim, highLim, msgNo, start;
    unsigned char attr;
    BOOL  done;
    char  save[64];

    int dummyincrement;
    ulong dummymsgno;
    BOOL dummydone;

    if (mf.mfLim)
    {
        getgroup();
        if (!mf.mfLim)
            return;
    }
    else 
    {
      doCR();
    }

    if (mf.mfUser[0])
        getNormStr("user", mf.mfUser, NAMESIZE, ECHO);
    
    if (mf.mfSearch[0])
    {
        getNormStr("search text", mf.mfSearch, NAMESIZE-2, ECHO);
        if (mf.mfSearch[0] != '*')
        {
            sprintf(save, "*%s", mf.mfSearch);
            strcpy(mf.mfSearch, save);
        }
        if (mf.mfSearch[strlen(mf.mfSearch)-1] != '*')
        {
            strcat(mf.mfSearch, "*");
        }
        mPrintf("Searching for %s.", mf.mfSearch); doCR();
    }

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    if (!expert )  mPrintf("\n <3J0>ump <3N0>ext <3P0>ause <3S0>top");

    switch (whichMess)  
    {
    case NEWoNLY:
        if (loggedIn)
        /* lowLim  = logBuf.lbvisit[ logBuf.lbroom[thisRoom].lvisit ]  + 1 ;*/

        lowLim = logBuf.newpointer[thisRoom] + 1;
        else
           lowLim  = cfg.oldest;
        highLim = cfg.newest;

        /* print out last new message */
        if (!revOrder && oldToo && (highLim >= lowLim))
            stepMessage(&lowLim, -1);
        break;

    case OLDaNDnEW:
        lowLim  = cfg.oldest;
        highLim = cfg.newest;
        break;

    case OLDoNLY:
        lowLim  = cfg.oldest;
        if (loggedIn)
            /* highLim = logBuf.lbvisit[ logBuf.lbroom[thisRoom].lvisit];*/
            highLim = logBuf.newpointer[thisRoom];

        else
            highLim=cfg.newest;
        break;
    }

    /* stuff may have scrolled off system unseen, so: */
    if (cfg.oldest  > lowLim)  lowLim = cfg.oldest;

    /* Allow for reverse retrieval: */
    if (!revOrder)
    {
        start       = lowLim;
        increment   = 1;
    }else{
        start       = highLim;
        increment   = -1;
    }

#ifdef GOODBYE
   if (start != cfg.oldest)
   {
       start -= (long)increment; 
       done = (BOOL)(!stepMessage(&start, increment));
   }
   else
   {
       start = cfg.oldest + 1;  
       start -= (long)increment; 
       done = (BOOL)(!stepMessage(&start, increment));
   }
#endif

    start -= (long)increment; 
    done = (BOOL)(!stepMessage(&start, increment));


    for (msgNo = start;
         !done 
         && msgNo >= lowLim 
         && msgNo <= highLim 
         && (CARRIER);
         done = (BOOL)(!stepMessage(&msgNo, increment)) )
    {

        /* all this trouble just to find out if this is going to */
        /* be the last message we're displaying */
        if (logBuf.MSGPAUSE)
        {
            dummymsgno = msgNo;
            dummyincrement = increment;

            dummydone = (BOOL)(!stepMessage(&dummymsgno, dummyincrement));

            if (!dummydone
             && dummymsgno >= lowLim 
             && dummymsgno <= highLim 
             && (CARRIER))
                 pausemsg = TRUE;
        }

        /*i = indexslot(msgNo);   for save message to file??? */

        if (BBSCharReady()) mAbort(FALSE);

        if (outFlag != OUTOK)
        {
            if (outFlag == OUTNEXT || outFlag == OUTPARAGRAPH)
            {
                outFlag = OUTOK;
                setio(whichIO, echo, outFlag);

            }
            else if (outFlag == OUTSKIP)  
            {
                echo = BOTH;
                setio(whichIO, echo, outFlag);

                memset(&mf, 0, sizeof(mf));
                pausemsg = FALSE;
                return;
            }
        }

        copyflag = FALSE;
        seen = FALSE;

        dowhat = READMESSAGE;
        printMessage( msgNo );
        dowhat = DUNO;

        if (outFlag != OUTSKIP)
        {

            /* New Stuff */
            if (auto_mark)
                dotoMessage = MARK_IT;

            if (auto_kill)
                dotoMessage = PULL_IT;
          
            /* End of new stuff */

            switch(dotoMessage)
            {
                case COPY_IT:
                    if (!sysop || !onConsole) break;
                    getNormStr("save path", save, 64, ECHO);
                    if (*save)
                    {
                        /*copyMessage(i, save); save message to file */
                    }
                    break;
    
                case PULL_IT:
                    /* Pull current message from room if flag set */
                    pullIt();
                    /* outFlag = OUTOK; */
                    break;
    
                case MARK_IT:
                    /* Mark current message from room if flag set */
                    markIt();
                    /* outFlag = OUTOK; */
                    break;

#ifdef GOODBYE    
                case REVERSE_READ:
                    increment = (increment == 1) ? -1 : 1;
                    doCR();
                    mPrintf("0<3Reversed0>");
                    doCR();
                    break;
#endif
                case REVERSE_READ:
                    outFlag = OUTOK;
                    setio(whichIO, echo, outFlag);

                    increment = -increment;
                    doCR();
                    mPrintf("0<3Reversed %c0>", (increment == 1) ? '+' : '-');
                    lowLim = cfg.oldest;
                    highLim= cfg.newest;     /* reevaluate for Livia */
                    doCR();
                    break;
    
                case NO_SPECIAL:
                    /* Release (Y/N)[N] */
                    if (outFlag == OUTNEXT)
                        break;
                    
                    if (   *msgBuf->mbx 
                        && CAN_MODERATE() 
                        && seen 
                        && ( msgBuf->mbattr & ATTR_MADEVIS) != ATTR_MADEVIS
                        && CANOUTPUT() 
                       )
                    if (getYesNo("Release", 0))
                    {
                        markmsg();
                        outFlag = OUTOK;
                        setio(whichIO, echo, outFlag);

                    }
    
                    /* reply to mail */
                    if ( whichMess == NEWoNLY 
                      && ( strcmpi(msgBuf->mbto,  logBuf.lbname) == SAMESTRING
                      ||   ( strcmpi(msgBuf->mbto, "Aide")   == SAMESTRING
                             && aide)
                      ||   ( strcmpi(msgBuf->mbto, "Sysop")  == SAMESTRING
                             && sysop)
                      ||   strcmpi(msgBuf->mbfwd, logBuf.lbname) == SAMESTRING )
                      && loggedIn )
                    {
                       outFlag = OUTOK;
                       setio(whichIO, echo, outFlag);

                       doCR(); 
                       if (getYesNo("Respond", 1)) 
                       {
                           replyFlag = 1;
                           mailFlag  = 1;
                           linkMess  = FALSE;
    
                           if (!copyflag)  attr = msgBuf->mbattr;
                           else            attr = originalattr;
    
                           if (whichIO != CONSOLE)
                           {
                               echo = CALLER;
                               setio(whichIO, echo, outFlag);
                           }
    
                           if  (makeMessage()) 
                           {
                               attr = (uchar)(attr | ATTR_REPLY);
    
                               if (!copyflag)  msgBuf->mbattr = attr;
                               else            originalattr  = attr;
    
                               if (!copyflag)  changeheader(msgNo,      /* 255 */ 10000, attr);
                               else            changeheader(originalId, /* 255 */ 10000, attr);
                           }
    
                           replyFlag = 0;
                           mailFlag  = 0;
    
                           /* Restore privacy zapped by make... */
                           if (whichIO != CONSOLE)
                           {
                               echo = BOTH;
                               setio(whichIO, echo, outFlag);
                           }    
                           outFlag = OUTOK;
                           setio(whichIO, echo, outFlag);

    
                           if (cfg.oldest  > lowLim)
                           {
                               lowLim = cfg.oldest;
                               if (msgNo < lowLim) msgNo = lowLim;
                           }
                       }
                    }
                    break;
                    
                default:
                    break;
            }
        }

        copyflag     = FALSE;
        originalId   = 0;
        originalattr = 0;

/*    if (!done && logBuf.MSGPAUSE) mAbort(TRUE);  */

    }

#ifdef GOODBYE
    /* this is fucking bullshit */
    if (done && !((msgBuf->mbsig[0] || msgBuf->mbusig[0]) && logBuf.SIGNATURES)
        && !bunga)
        doCR();
#endif

    echo = BOTH;
    setio(whichIO, echo, outFlag);

    memset(&mf, 0, sizeof(mf));
}
/* -------------------------------------------------------------------- */
/*  getMessage()    reads a message off disk into RAM.                  */
/* -------------------------------------------------------------------- */
BOOL getMessage(void)
{
    struct unkLst *lul;  /* brent I dunno */

    char c;
    int i;

#ifdef NEWMSGTAB
    char newstyle = FALSE;
    uchar roomno_lo;
    uchar roomno_hi;

#endif

    /* clear message buffer out */
    clearmsgbuf();

    /* look 10000 chars for start message */
    for (i = 0; i < 10000; ++i)
    {
        c = (uchar)getMsgChar();
        if (c == -1) break;
    }
    if (i == 10000) return(FALSE);

#ifdef GOODBYE
    /* find start of message */
    do
    {
        c = (uchar)getMsgChar();
    } while (c != -1);
#endif

    /* record exact position of start of message */
    msgBuf->mbheadLoc  = (long)(ftell(msgfl) - (long)1);


#ifdef NEWMSGTAB

    /* here is a kludge, Its looking ahead to see of this message has */
    /* a two byte room number */
    getMsgChar();
    getMsgChar();
    c = getMsgChar();
    if (isdigit(c))
    {
        /* newstyle = TRUE; */
        /* kill this debug stuff later */
#ifdef GOODBYE
        if (debug)
        {
            doccr();
            cPrintf("Old Style Message");
            doccr();
        }
#endif

    }
    else
    {

        newstyle = TRUE;

#ifdef GOODBYE
        if (debug)
        {
            /* kill this debug stuff later */
            doccr();
            cPrintf("New Style Message");
            doccr();
        }
#endif

    }
    fseek(msgfl, msgBuf->mbheadLoc  + 1L , 0);

/* end of stupid cludgyness */

#endif



    /* get message's room #         */
#ifdef NEWMSGTAB
    if (newstyle)
    {
        roomno_lo = (uchar)getMsgChar();
        roomno_hi = (uchar)getMsgChar();

        msgBuf->mbroomno = int_JOIN(roomno_lo, roomno_hi);

        msgBuf->mbroomno = msgBuf->mbroomno / 2;
    }
    else
    {
        msgBuf->mbroomno   = (uchar)getMsgChar();
    }

#else
        msgBuf->mbroomno   = (uchar)getMsgChar();
#endif

    /* get message's attribute byte */
    msgBuf->mbattr     = (uchar)getMsgChar();

    getMsgStr(msgBuf->mbId, LABELSIZE);
    
    do 
    {
        c = (char)getMsgChar();
        
        switch (c)
        {
        case 'A':     getMsgStr(msgBuf->mbauth,  LABELSIZE);    break;
        case 'B':     getMsgStr(msgBuf->mbsub,   80       );    break;
        case 'C':     getMsgStr(msgBuf->mbcopy,  LABELSIZE);    break;
        case 'D':     getMsgStr(msgBuf->mbtime,  LABELSIZE);    break;
        case 'F':     getMsgStr(msgBuf->mbfwd,   LABELSIZE);    break;
        case 'G':     getMsgStr(msgBuf->mbgroup, LABELSIZE);    break;
        case 'I':     getMsgStr(msgBuf->mbreply, LABELSIZE);    break;
        case 'J':     getMsgStr(msgBuf->mbcreg,  LABELSIZE);    break;
        case 'j':     getMsgStr(msgBuf->mbccont, LABELSIZE);    break;
        case 'L':     getMsgStr(msgBuf->mblink,  63);           break;
        case 'M':     /* will be read off disk later */         break;
        case 'N':     getMsgStr(msgBuf->mbtitle, LABELSIZE);    break;
        case 'n':     getMsgStr(msgBuf->mbsur,   LABELSIZE);    break;
        case 'O':     getMsgStr(msgBuf->mboname, LABELSIZE);    break;
        case 'o':     getMsgStr(msgBuf->mboreg,  LABELSIZE);    break;
        case 'P':     getMsgStr(msgBuf->mbfpath, 128     );     break;
        case 'p':     getMsgStr(msgBuf->mbtpath, 128     );     break;
        case 'Q':     getMsgStr(msgBuf->mbocont, LABELSIZE);    break;
        case 'q':     getMsgStr(msgBuf->mbczip,  LABELSIZE);    break;
        case 'R':     getMsgStr(msgBuf->mbroom,  LABELSIZE);    break;
        case 'S':     getMsgStr(msgBuf->mbsrcId, LABELSIZE);    break;
        case 's':     getMsgStr(msgBuf->mbsoft,  LABELSIZE);    break;
        case 'T':     getMsgStr(msgBuf->mbto,    LABELSIZE);    break;
        case 'X':     getMsgStr(msgBuf->mbx,     LABELSIZE);    break;
        case 'Z':     getMsgStr(msgBuf->mbzip,   LABELSIZE);    break;
        case 'z':     getMsgStr(msgBuf->mbrzip,  LABELSIZE);    break;
        case '.':     getMsgStr(msgBuf->mbsig,   90       );    break;
        case '_':     getMsgStr(msgBuf->mbusig,  90       );    break;
        /* nodephone */
        case 'H':     getMsgStr(msgBuf->mbophone,LABELSIZE);    break;
        case 'h':     getMsgStr(msgBuf->mbzphone,LABELSIZE);    break;
        case '\0':     break;

#ifdef GOODBYE
        default:
            getMsgStr(msgBuf->mbtext, cfg.maxtext); /* discard unknown field  */
            msgBuf->mbtext[0]    = '\0';
            break;
#endif

        default: 
            { 
            /* try to store unknown field */ 
            if ((lul = addUnknownList()) != NULL) 
                { 
                lul->whatField = c; 
                getMsgStr(lul->theValue, MAXUNKLEN); 
                } 
            else 
                { 
                /* cannot save it - discard unknown field  */ 
                getMsgStr(msgBuf->mbtext, MAXTEXT); 
                msgBuf->mbtext[0]    = '\0'; 
                } 
            } 

        }
    } while (c != 'M');

    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  getMsgStr()     reads a NULL terminated string from msg file        */
/* -------------------------------------------------------------------- */
void getMsgStr(char *dest, int lim)
{
    char c;

    while ((c = (char)getMsgChar()) != 0)    /* read the complete string     */
    {
        if (lim)                        /* if we have room then         */
        {
            lim--;
            *dest++ = c;                /* copy char to buffer          */
        }
    }
    *dest = '\0';                       /* tie string off with null     */
}

/* -------------------------------------------------------------------- */
/*  getMsgChar()    reads a character from msg file, curent position    */
/* -------------------------------------------------------------------- */
int getMsgChar(void)
{
    int c;

    c = fgetc(msgfl);

    if (c == ERROR)
    {
        /* check for EOF */
        if (feof(msgfl))
        {
            clearerr(msgfl);
            fseek(msgfl, 0l, SEEK_SET);
            c = fgetc(msgfl);
        }
    }
    return c;
}

/* -------------------------------------------------------------------- */
/*  printheader()   prints current message header                       */
/* -------------------------------------------------------------------- */
void printheader(ulong id, ulong slot)
{
    char dtstr[80];
    uchar attr;
    long timestamp;

    struct unkLst *lul;  /* brent I dunno */

    if (outFlag == OUTNEXT) 
    {
        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);
    }

    if (verbose && sysop && debug)
    {
        /* print any unknowns ... */
        for(lul = firstUnk; lul != NULL; lul = lul->next) 
        { 
            doCR();
            mPrintf("    Unknown field: (3%c0) %s", lul->whatField, lul->theValue); 
        } 
    }

    if (*msgBuf->mbtime)
    {
        sscanf(msgBuf->mbtime, "%ld", &timestamp);

        if (verbose)
        {
        strftime(dtstr, 79, 
                 (loggedIn) ? logBuf.vdstamp : cfg.vdatestamp, timestamp);
        }
        else
        {
        strftime(dtstr, 79, 
                 (loggedIn) ? logBuf.dstamp : cfg.datestamp, timestamp);
        }
    }


    if (verbose && (strcmpi(msgBuf->mbauth, "****") != SAMESTRING || sysop)) 
    {
        doCR();
        termCap(TERM_BOLD);
        mPrintf("    # %lu of %lu", originalId, cfg.newest);

        if (debug)
        mPrintf(" (Slot #%lu, Room #%d)", slot, thisRoom); 

        if (copyflag && aide)
            mPrintf(" (Duplicate id # %lu, %s)", copyOf, msgBuf->mbId);
        if (*msgBuf->mbsrcId) 
        {
            doCR();
            mPrintf("    Source id # is:  %s", msgBuf->mbsrcId);
        }              
        if (*msgBuf->mblink && sysop) 
        {
            doCR();
            mPrintf("    Linked file is:  %s", msgBuf->mblink);
        }
        if (*msgBuf->mbfpath)
        {
            doCR();
            mPrintf("    Path followed:   %s!%s", msgBuf->mbfpath, cfg.nodeTitle);
        }
        if (*msgBuf->mbsoft)
        {
            doCR();
            mPrintf("    Source software: %s", msgBuf->mbsoft);
        }
        if (*msgBuf->mbtpath)
        {
            doCR();
            mPrintf("    Forced path:     %s", msgBuf->mbtpath);
        }
    }
    
    doCR();
    termCap(TERM_BOLD);
    mPrintf("    %s", dtstr);
    
    if (msgBuf->mbauth[ 0])
    {
        mPrintf(" From ");
        
        if (!roomBuf.rbflags.ANON 
          && strcmpi(msgBuf->mbauth, "****") != SAMESTRING)
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
            
            
            termCap(TERM_UNDERLINE);
            mPrintf("%s", msgBuf->mbauth);
            termCap(TERM_NORMAL);
            termCap(TERM_BOLD);
            
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

#ifdef GOODBYE   /* Marty told me to trash this */         
            if (sysop && strcmpi(msgBuf->mbauth, "****") != SAMESTRING)
            {
                mPrintf(" (%s03)", msgBuf->mbauth);
            }
#endif

        }
    }
    
    termCap(TERM_BOLD);

    if (msgBuf->mboname[0]
        && (strcmpi(msgBuf->mboname, cfg.nodeTitle) != SAMESTRING
          || strcmpi(msgBuf->mboreg, cfg.nodeRegion) != SAMESTRING)
            && strcmpi(msgBuf->mbauth, msgBuf->mboname) != SAMESTRING)
             mPrintf(" @ %s", msgBuf->mboname);


    if (msgBuf->mboreg[0] &&
        strcmpi(msgBuf->mboreg, cfg.nodeRegion) != SAMESTRING)
        {
           mPrintf(", %s", msgBuf->mboreg);

           if (verbose && *msgBuf->mbcreg)
               mPrintf(" {%s}", msgBuf->mbcreg);
        }
    
    if (msgBuf->mbocont[0] && verbose &&
        strcmpi(msgBuf->mbocont, cfg.nodeContry) != SAMESTRING)
        {
           mPrintf(", %s", msgBuf->mbocont);

           if (verbose && *msgBuf->mbccont)
               mPrintf(" {%s}", msgBuf->mbccont);
        }

    if (msgBuf->mbophone[0] && verbose) mPrintf(", %s",  msgBuf->mbophone);

    if (msgBuf->mbto[0])
    {
        mPrintf(" To %s03", msgBuf->mbto);

        if (msgBuf->mbfwd[0])
            mPrintf(" Forwarded to %s03", msgBuf->mbfwd );

        if (msgBuf->mbzip[0]
              && strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING)
                 mPrintf(" @ %s", msgBuf->mbzip);

        if (msgBuf->mbrzip[0] &&
            strcmpi(msgBuf->mbrzip, cfg.nodeRegion))
              mPrintf(", %s", msgBuf->mbrzip);

        if (msgBuf->mbreply[0])
        {
            if (verbose)
                mPrintf(" [reply to %s]", msgBuf->mbreply);
            else
                mPrintf(" [reply]");
        }

#ifdef NEWMSGTAB
        if ( getFlags(slot)->RECEIVED)  mPrintf(" [received]");
        if ( getFlags(slot)->REPLY)     mPrintf(" [reply sent]");
#else
        if ( msgTab_mtmsgflags[(int)slot].RECEIVED)  mPrintf(" [received]");
        if ( msgTab_mtmsgflags[(int)slot].REPLY)     mPrintf(" [reply sent]");
#endif


        if ( (msgBuf->mbto[0])
           && !(strcmpi(msgBuf->mbauth, logBuf.lbname) == SAMESTRING ))
        {

            if (!copyflag)  attr = msgBuf->mbattr;
            else            attr = originalattr;

            if (!(attr & ATTR_RECEIVED))
            {
                attr = (uchar)(attr | ATTR_RECEIVED);

                if (!copyflag)  msgBuf->mbattr = attr;
                else            originalattr  = attr;

                if (!copyflag)  changeheader(id,         /* 255 */ 10000, attr);
                else            changeheader(originalId, /* 255 */ 10000, attr);

            }
        }
    }

    if (strcmpi(msgBuf->mbroom, roomBuf.rbname) != SAMESTRING)
    {
        mPrintf(" In %s03>",  msgBuf->mbroom );
    }

    if (msgBuf->mbgroup[0])
    {
        mPrintf(" (%s only)", msgBuf->mbgroup);
    }

    if (  CAN_MODERATE() 
          && msgBuf->mbx[0])
    {
#ifdef NEWMSGTAB
        if (!getFlags(slot)->MADEVIS)
#else
        if (!msgTab_mtmsgflags[(int)slot].MADEVIS)
#endif

        {
          if (msgBuf->mbx[0] == 'Y')
              mPrintf(" 1[problem user]03");
          else
              mPrintf(" [moderated]");
        }
        else  mPrintf(" [viewable %s]", msgBuf->mbx[0] == 'Y' ?
              "problem user" : "moderated" );
    }

    if ((aide || sysop) && msgBuf->mblink[0])
        mPrintf(" [file-linked]");

    doCR();

    if (msgBuf->mbsub[0] && logBuf.SUBJECTS)
    {
        mPrintf("    Subject: 4%s0", msgBuf->mbsub);
        doCR();
    }
    
    termCap(TERM_NORMAL);
    
    if (*msgBuf->mbto && whichIO != CONSOLE) 
    {
        echo = CALLER;
        setio(whichIO, echo, outFlag);
    }
}


