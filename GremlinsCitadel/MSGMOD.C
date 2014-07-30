/****************************************************************************/
/*  MSGMOD.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                       Overlayed message code                         */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  copymessage()   copies specified message # into specified room      */
/*  deleteMessage() deletes message for pullIt()                        */
/*  insert()        aide fn: to insert a message                        */
/*  makeMessage()   is menu-level routine to enter a message            */
/*  markIt()        is a sysop special to mark current message          */
/*  markmsg()       marks a message for insertion and/or visibility     */
/*  pullIt()        is a sysop special to remove a message from a room  */
/*  changeheader()  Alters room# or attr byte in message base & index   */
/* -------------------------------------------------------------------- */

extern ulong copyOf;

/************************************************************************/
/*      markroom()  adds all messages in a room to queue                */
/************************************************************************/
void markroom(void)
{
    ulong          i;
    ulong          num;

    num = sizetable();

    for (i = 0; i < num; ++i)
    {
#ifdef NEWMSGTAB
        if (getRoomNum(i) == thisRoom)
#else
        if (msgTab_mtroomno[(int)i] == thisRoom)
#endif
        {
            if (mayseeindexmsg(i))
                addtomsglist(((ulong)(cfg.mtoldest + i)));
        }
    }
}

/************************************************************************/
/*      printmsglist()  prints out messages currently in list.          */
/************************************************************************/
void printmsglist(void)
{
    int i;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    if (msglistsize) doCR();
    for (i = 0; i < msglistsize && (outFlag == OUTOK); ++i)
    {
        mPrintf(" #%lu\n", msglist[i]);
    }
}

/************************************************************************/
/*      insertmsglist()                                                 */
/************************************************************************/
void insertmsglist(void)
{
    long oldmarkedMId;
    int i;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    oldmarkedMId = markedMId;

    if (msglistsize) doCR();
    for (i = 0; i < msglistsize && (outFlag == OUTOK); ++i)
    {
        markedMId = msglist[i];

        mPrintf(" Inserting message #%lu\n", msglist[i]);
        
        insert();
    }
    markedMId = oldmarkedMId;
}

/************************************************************************/
/*      killmsglist()                                                   */
/************************************************************************/
void killmsglist(void)
{
    long oldoriginalId;
    int i;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    oldoriginalId = originalId;

    if (msglistsize) doCR();
    for (i = 0; i < msglistsize && (outFlag == OUTOK); ++i)
    {
        originalId = msglist[i];

        mPrintf(" Killing message #%lu\n", msglist[i]);

        deleteMessage();
    }
    originalId = oldoriginalId; 
}

/************************************************************************/
/*      sortmsglist()  0 Forward 1 Reverse                              */
/************************************************************************/
void sortmsglist(char direction)
{
    if (!direction)
        qsort(msglist, msglistsize, (unsigned)sizeof(*msglist), 
 (QSORT_CMP_FNP)msglistsort);
    if (direction)
        qsort(msglist, msglistsize, (unsigned)sizeof(*msglist), 
 (QSORT_CMP_FNP)revmsglistsort);
}

/************************************************************************/
/*      msglistsort()
/************************************************************************/
int msglistsort(ulong **s1, ulong **s2)
{
    /* if (*s1 == *s2) return  0; */
    if (*s1 >  *s2) return  1;
    if (*s1 <  *s2) return -1;
    return 0;
}

/************************************************************************/
/*      revmsglistsort()
/************************************************************************/
int revmsglistsort(ulong **s1, ulong **s2)
{
    /* if (*s1 == *s2) return  0; */
    if (*s1 >  *s2) return -1;
    if (*s1 <  *s2) return  1;
    return 0;
}

/************************************************************************/
/*      addtomsglist() adds an element to message queue                 */
/************************************************************************/
void addtomsglist(ulong msgnumber)
{
    if (msgnumber == 100000000l)
    {
        msglistsize = 0;

        msglist = (ulong *)realloc(msglist, 0);

        return;
    }
    else
    {
        msglist = (ulong *)realloc(msglist, (msglistsize + 1) * sizeof(ulong));
        if (msglist == NULL)
        {
            mPrintf("\nError reallocating messsage list\n");
            return;
        }
        else
        {
            msglist[msglistsize] = msgnumber;
            msglistsize++;
        }
    }
}

/************************************************************************/
/*    clearmsglist() clears msg queue                                   */
/************************************************************************/
void clearmsglist(void)
{
    addtomsglist(100000000l);
}               

/************************************************************************/
/*      automark()  automatically mark or display messages              */
/************************************************************************/
void automark(void)
{
    if (auto_mark || auto_kill)
    {
        doCR();
        doCR();
        mPrintf(" Automatic message ");
        if (auto_mark)  mPrintf("mark");
        if (auto_kill)  mPrintf("kill");
        mPrintf(" disabled.");
        auto_mark = 0;
        auto_kill = 0;
        doCR();
        return;
    }
    doCR();
    auto_mark = (char)getYesNo("Enable automatic mark", 0);
    if (auto_mark) return;
    auto_kill = (char)getYesNo("Enable automatic kill", 0);
}

/* -------------------------------------------------------------------- */
/*  copymessage()   copies specified message # into specified room      */
/* -------------------------------------------------------------------- */
void copymessage(ulong id, int roomno)
{
    unsigned char attr;
    char copy[20];
    ulong slot;

    slot = indexslot(id);

    /* load in message to be inserted */
/*  fseek(msgfl, msgTab_mtmsgLoc[slot], 0);  */


#ifdef NEWMSGTAB
    fseek(msgfl, getLocation(slot), 0);
#else
    fseek(msgfl, long_JOIN(msgTab_mtmsgLocLO[(int)slot], 
                           msgTab_mtmsgLocHI[(int)slot]), 0);
#endif


    getMessage();

    /* retain vital information */
    attr    = msgBuf->mbattr;
    strcpy(copy, msgBuf->mbId);
    
    clearmsgbuf();
    msgBuf->mbtext[0] = '\0';

    strcpy(msgBuf->mbcopy, copy);
    msgBuf->mbattr   = attr;
    msgBuf->mbroomno = roomno;    
    
    putMessage();
    noteMessage();
}

/* -------------------------------------------------------------------- */
/*  deleteMessage() deletes message for pullIt()                        */
/* -------------------------------------------------------------------- */
void deleteMessage(void)
{
    ulong id;

    id = originalId;

#ifdef GOODBYE   /* moved up to pullit() */
    if (!(*msgBuf->mbx))
    {
        markmsg();    /* Mark it for possible insertion elsewhere */
    }
#endif
    
    changeheader(id, DUMP, 255);
    
    if (thisRoom != AIDEROOM && thisRoom != DUMP)
    {
        /* note in Aide): */
        sprintf(msgBuf->mbtext, "Following %s #%lu deleted by %s:",
                cfg.msg_nym, id, logBuf.lbname);

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();

        copymessage(id, AIDEROOM); 
#ifdef GOODBYE
        if (!logBuf.lbroom[AIDEROOM].lvisit)
            talleyBuf->room[AIDEROOM].new--;
#endif

        if (talleyBuf->room[AIDEROOM].visited)
            talleyBuf->room[AIDEROOM].new--;

    }
}

/* -------------------------------------------------------------------- */
/*  insert()        aide fn: to insert a message                        */
/* -------------------------------------------------------------------- */
void insert(void)
{
    if ( thisRoom   == AIDEROOM  ||  markedMId == 0l )
    {
        mPrintf("Not here.");
        return;
    }
    copymessage(markedMId, thisRoom); 
    
    sprintf(msgBuf->mbtext, "Following %s #%lu inserted in %s> by %s",
         cfg.msg_nym, markedMId, roomBuf.rbname, logBuf.lbname );

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    copymessage(markedMId, AIDEROOM); 

#ifdef GOODBYE
    if (!logBuf.lbroom[AIDEROOM].lvisit)
        talleyBuf->room[AIDEROOM].new--;

#endif
        if (talleyBuf->room[AIDEROOM].visited)
            talleyBuf->room[AIDEROOM].new--;

}

/* -------------------------------------------------------------------- */
/*  markIt()        is a sysop special to mark current message          */
/* -------------------------------------------------------------------- */
BOOL markIt(void)
{
    ulong id;
    int yna;
    char oldverbose;

    sscanf(msgBuf->mbId, "%lu", &id);

    /* confirm that we're marking the right one: */
    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);
    oldverbose = verbose;
    verbose = FALSE;
    if (!auto_mark) printMessage( id);
    verbose = oldverbose;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR();

    yna = getYesNo("Mark", (char)((auto_mark) ? 4 : 1));

    if (yna == 1) 
    {
        markmsg();
        return TRUE;
    }

    if (yna == 2) 
    {
        outFlag = OUTSKIP;
        setio(whichIO, echo, outFlag);
    }

    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  markmsg()       marks a message for insertion and/or visibility     */
/* -------------------------------------------------------------------- */
void markmsg(void)
{
    ulong id;
    uchar attr;

    markedMId = originalId;
    id        = originalId;

    /* put in message queue */
    addtomsglist(markedMId);

    if (msgBuf->mbx[0])
    {
        if (!copyflag)  attr = msgBuf->mbattr;
        else            attr = originalattr;

        attr = (uchar)(attr ^ ATTR_MADEVIS);

        if (!copyflag)  msgBuf->mbattr = attr;
        else            originalattr  = attr;

        changeheader(id, /* 255 */ 10000, attr);

        if ((attr & ATTR_MADEVIS) == ATTR_MADEVIS)
            copymessage( id, thisRoom);
    }
}


/* -------------------------------------------------------------------- */
/*  pullIt()        is a sysop special to remove a message from a room  */
/* -------------------------------------------------------------------- */
BOOL pullIt(void)
{
    ulong id;
    int yna;
    char oldverbose;

    id = originalId;
    
    /* confirm that we're removing the right one: */
    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    oldverbose = verbose; 
    verbose = FALSE;
    if (!auto_kill) printMessage( id);
    verbose = oldverbose;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR();

    yna = getYesNo("Pull", (char)((auto_kill) ? 3 : 0));

    if (yna == 1)
    {
        /* moved up from deletemessage() */
        if (!(*msgBuf->mbx))
        {
            markmsg();    /* Mark it for possible insertion elsewhere */
        }

        deleteMessage();
        return TRUE;
    }

    if (yna == 2) 
    {
        outFlag = OUTSKIP;
        setio(whichIO, echo, outFlag);
    }
    return FALSE;
}


/* -------------------------------------------------------------------- */
/*  changeheader()  Alters room# or attr byte in message base & index   */
/* -------------------------------------------------------------------- */
void changeheader(ulong id, int roomno, uchar attr)
{
    long loc;

#ifdef NEWMSGTAB
    ulong  slot;
#else
    int  slot;
#endif

    int  c;
    long pos;
    int  room;

#ifdef NEWMSGTAB
    struct messagetable *lmt;

    char newstyle = FALSE;
    uchar roomno_lo;
    uchar roomno_hi;
    long startloc;

    pos = ftell(msgfl);
    slot = indexslot(id);
    lmt = getMsgTab(slot);

#else

    pos = ftell(msgfl);
    slot = (int)indexslot(id);
#endif

    /*
     * Change the room # for the message
     */
    if (roomno !=  /* 255 */ 10000)
    {
        /* determine room # of message to be changed */

#ifdef NEWMSGTAB
        room = lmt->mtroomno;
#else
        room = msgTab_mtroomno[slot];
#endif


        /* fix the message tallys from */
        talleyBuf->room[room].total--;
        if (mayseeindexmsg(slot))
        {
            talleyBuf->room[room].messages--;
            if  ((ulong)(cfg.mtoldest + slot) >

                logBuf.newpointer[room])
                /* logBuf.lbvisit[ logBuf.lbroom[room].lvisit ]) */
                talleyBuf->room[room].new--;
        }

        /* fix room tallys to */
        talleyBuf->room[roomno].total++;
        if (mayseeindexmsg(slot))
        {
            talleyBuf->room[roomno].messages++;
            if  ((ulong)(cfg.mtoldest + slot) >
                logBuf.newpointer[room])

                /* logBuf.lbvisit[ logBuf.lbroom[roomno].lvisit ]) */
                talleyBuf->room[room].new++;
        }
    }

/*  loc  = msgTab2[slot].mtmsgLoc; */

#ifdef NEWMSGTAB
    loc  = lmt->mtmsgLoc;
#else
    loc  = long_JOIN(msgTab_mtmsgLocLO[slot], msgTab_mtmsgLocHI[slot]); 
#endif


    if (loc == ERROR) return;

    fseek(msgfl, loc, SEEK_SET);

    /* find start of message */
    do c = getMsgChar(); while (c != 0xFF);


#ifdef NEWMSGTAB
    /* record exact position of start of message */
    startloc  = ftell(msgfl);



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
        cPrintf("\nold style message\n");
#endif

    }
    else
    {
        newstyle = TRUE;
#ifdef GOODBYE
        cPrintf("\nnew style message\n");
#endif
    }
    fseek(msgfl, startloc , 0);

    /* end of stupid cludginess */

#endif




    if (roomno != /* 255 */ 10000)
    {


#ifdef NEWMSGTAB
        if (newstyle)
        {
            roomno_lo = int_LO(roomno * 2);
            roomno_hi = int_HI(roomno * 2);

            overwrite(1);
            putMsgChar(roomno_lo);

            overwrite(1);
            putMsgChar(roomno_hi);
        }
        else
        {
            overwrite(1);
            /* write room #    */
            putMsgChar(roomno);
        }
#else
            overwrite(1);
            /* write room #    */
            putMsgChar(roomno);
#endif



#ifdef NEWMSGTAB
        lmt->mtroomno = roomno;
#else
        msgTab_mtroomno[slot] = roomno;
#endif

    }
    else
    {
        /* step over room number */
#ifdef NEWMSGTAB
        getMsgChar();
        if (newstyle)
            getMsgChar();
#else

        getMsgChar();

#endif

    }

    if (attr != 255)
    {
        overwrite(1);
        /* write attribute */
        putMsgChar(attr);  


#ifdef NEWMSGTAB
        lmt->mtmsgflags.RECEIVED
            = ((attr & ATTR_RECEIVED) == ATTR_RECEIVED);

        lmt->mtmsgflags.REPLY
            = ((attr & ATTR_REPLY)    == ATTR_REPLY   );

        lmt->mtmsgflags.MADEVIS
            = ((attr & ATTR_MADEVIS)  == ATTR_MADEVIS );
#else
        msgTab_mtmsgflags[slot].RECEIVED
            = ((attr & ATTR_RECEIVED) == ATTR_RECEIVED);

        msgTab_mtmsgflags[slot].REPLY
            = ((attr & ATTR_REPLY)    == ATTR_REPLY   );

        msgTab_mtmsgflags[slot].MADEVIS
            = ((attr & ATTR_MADEVIS)  == ATTR_MADEVIS );
#endif

    }

    fseek(msgfl, pos, SEEK_SET);
}

