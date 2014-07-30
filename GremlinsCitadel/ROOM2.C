/************************************************************************/
/*                              room2.c                                 */
/*              room code for Citadel bulletin board system             */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      findRoom()              find a free room                        */
/*      formatSummary()         summarizes current room                 */
/*      givePrompt()            gives usual "THISROOM>" prompt          */
/*      indexRooms()            build RAM index to room.cit             */
/*      killempties()           deletes empty rooms                     */
/*      killroom()              aide fn: to kill current room           */
/*      makeRoom()              make new room via user dialogue         */
/*      massdelete()            sysop fn: to kill all msgs from person  */
/*      noteRoom()              enter room into RAM index               */
/*      readbymsgno()           sysop fn: to read by message #          */
/*      renameRoom()            sysop special to rename rooms           */
/*      stepRoom()              goes to next or previous room           */
/************************************************************************/

/************************************************************************/
/*      findRoom() returns # of free room if possible, else ERROR       */
/************************************************************************/
int findRoom(void)
{
    int roomRover;

    for (roomRover = 0;  roomRover < MAXROOMS;  roomRover++)
    {
        if (roomTab[roomRover].rtflags.INUSE == 0) return roomRover;
    }
    return ERROR;
}

/************************************************************************/
/*      formatSummary() formats a summary of the current room           */
/************************************************************************/
void formatSummary(char *buffer)
{
    char line[171];
    char id[NAMESIZE];

    sprintf(line, " Room %s", roomBuf.rbname);

    strcpy(buffer, line);

    if (roomBuf.rbflags.SHARED)
    {
        strcpy(id, roomBuf.rbname);
        NametoId(id);
        if (id[0])
        {
             sprintf(line, ", Net ID %s", id);
             strcat(buffer, line);
        }
    }

    if (roomBuf.rbflags.GROUPONLY)
    {
        sprintf(line, ", owned by group %s",
            grpBuf.group[ roomBuf.rbgrpno ].groupname);

        strcat(buffer, line);
    }
    
    if (roomBuf.rbflags.PRIVGRP)
    {
        sprintf(line, ", privileges group %s",
            grpBuf.group[ roomBuf.rbPgrpno ].groupname);

        strcat(buffer, line);
    }

    if (!roomBuf.rbflags.PUBLIC)
    {
        strcat(buffer, ", hidden");
    }
    
    if (roomBuf.rbflags.ANON)
    {
        strcat(buffer, ", Anonymous");
    }

#ifdef GOODBYE    
    if (roomBuf.rbflags.BIO)
    {
        strcat(buffer, ", BIO");
    }
#endif

    if (roomBuf.rbflags.MODERATED)
    {
        strcat(buffer, ", moderated");
    }

    if (roomBuf.rbflags.READONLY)
    {
        strcat(buffer, ", read only");
    }

    if (roomBuf.rbflags.DOWNONLY)
    {
        strcat(buffer, ", download only");
    }

    if (roomBuf.rbflags.SHARED)
    {
        strcat(buffer, ", networked/shared");
    }

    if (roomBuf.rbflags.APLIC)
    {
        strcat(buffer, ", application");
    }

    if (roomBuf.rbflags.AUTOAPP)
    {
        strcat(buffer, ", autoapp");
    }

    if (roomBuf.rbflags.PERMROOM)
    {
        strcat(buffer, ", permanent room");
    }

    if (aide)
    {
        if (roomBuf.rbflags.MSDOSDIR)
        {
            sprintf(line, "\n Directory room:  path is %s", roomBuf.rbdirname);
            strcat(buffer, line);
        }
    }

    if (sysop && roomBuf.rbflags.APLIC)
    {
        sprintf(line, "\n Application is %s", roomBuf.rbaplic);
        strcat(buffer, line);
    }

    if (roomBuf.rbroomtell[0] && cfg.roomtell && sysop)
    {
        sprintf(line, "\n Room description file is %s", roomBuf.rbroomtell);
        strcat(buffer, line);
    }

    if (roomBuf.descript[0])
    {
        sprintf(line, "\n Room Info-line is: %s", roomBuf.descript);
        strcat(buffer, line);
    }
}

/************************************************************************/
/*      makeRoom() constructs a new room via dialogue with user.        */
/************************************************************************/
void makeRoom(void)
{
    label roomname;
    label oldName;
    label groupname;
    int groupslot, testslot, newSlot;
    char line[80];
    int j;

#ifdef NEWMSGTAB
    ulong i;
#else
    int  i;
#endif

    logBuf.lbroom[thisRoom].lbgen  = roomBuf.rbgen;
    /* logBuf.lbroom[thisRoom].lvisit = 0; */
    logBuf.newpointer[thisRoom] = cfg.newest;


    /* zero new count in talleybuffer */
    talleyBuf->room[thisRoom].new  = 0;

    strcpy(oldName, roomBuf.rbname);

    newSlot = findRoom();

    if ( (newSlot) == ERROR )
    {
        mPrintf(" Room table full.");
        return;
    }

    getNormStr("name for new room", roomname, NAMESIZE, ECHO);

    if (!strlen(roomname))
    {
        return;
    }

    testslot = roomExists(roomname);

    if (testslot != ERROR)
    {
        mPrintf(" A \"%s\" room already exists.\n", roomname);
        return;
    }

    memset(&roomBuf, 0, sizeof(roomBuf));
    
    if (limitFlag)
    {
        getString("group for new room", groupname, NAMESIZE, FALSE, ECHO, "");

        groupslot = partialgroup(groupname);
        
        if (!strlen(groupname) || (groupslot == ERROR)
        || !ingroup(groupslot) )
        {
            mPrintf("No such group.");
            getRoom(thisRoom);  /* From official release */
            return;
        }
        roomBuf.rbgrpno  = (unsigned char)groupslot;
        /* roomBuf.rbgrpgen = grpBuf.group[groupslot].groupgen; */
    }
    if (!expert)   tutorial("newroom.blb");

    roomBuf.rbflags.INUSE     = TRUE;
    roomBuf.rbflags.GROUPONLY = limitFlag;

    getNormStr("Description for new room", roomBuf.descript, 80, ECHO);

    roomBuf.rbflags.PUBLIC = getYesNo("Make room public", 1);
    if (aide)
    {
        roomBuf.rbflags.SHARED = getYesNo("Make room shared", 0);
    }
    sprintf(line, "Install \"%s\" as a %s%s room",
    roomname , (roomBuf.rbflags.PUBLIC) ? "public" : "private" , 
               (roomBuf.rbflags.SHARED) ? ", shared" : "" );

    if (!getYesNo( line , 0) )
    {
        getRoom(thisRoom);
        return;
    }

    thisRoom = newSlot;
    
    strcpy(roomBuf.rbname, roomname);

    if (roomBuf.rbflags.SHARED)
    {
        strcpy(line, roomname);
        line[19] = '\0'; /* Cut it down to size */

        testslot = IdExists(line);

        if (testslot != ERROR)
        {
            mPrintf(" A \"%s\" Net ID already exists.\n", line);
        }
        else
        strcpy(roomBuf.netID, line);
    }

#ifdef NEWMSGTAB
    for (i = 0;  i < sizetable();  i++)
#else
    for (i = 0;  i < (int)sizetable();  i++)
#endif

    {

#ifdef NEWMSGTAB
        if (getRoomNum(i) == thisRoom)
#else
        if (msgTab_mtroomno[i] == thisRoom)
#endif

            changeheader(cfg.mtoldest + i,
            3   /* Dump      */,
            255 /* No change */ );
    }
    roomBuf.rbgen = (uchar)((roomTab[thisRoom].rtgen + 1) % MAXGEN);

    noteRoom();                         /* index new room       */
    if (strcmp(roomTab[thisRoom].rtname, roomBuf.rbname) != SAMESTRING)
    {
        cPrintf("Room names changed roomTab = \"%s\", roomBuf = \"%s\"\n",
                roomTab[thisRoom].rtname, roomBuf.rbname);
    }
    
    putRoom(thisRoom);

    /* remove room from all halls */
    for (j = 0; j < MAXHALLS; j++)
    {
        /* remove from halls */
#ifdef GOODBYE
        hallBuf->hall[j].hroomflags[thisRoom].inhall = FALSE;
#endif
        bit_clear(hallBuf->hall[j].inhall, thisRoom);

        /* unwindow */
#ifdef GOODBYE
        hallBuf->hall[j].hroomflags[thisRoom].window = FALSE;
#endif
        bit_clear(hallBuf->hall[j].window, thisRoom);

    }

    /* put room in current hall */
#ifdef GOODBYE
    hallBuf->hall[thisHall].hroomflags[thisRoom].inhall = TRUE;
#endif
    bit_set(hallBuf->hall[thisHall].inhall, thisRoom);

    /* put room in maintenance hall */
#ifdef GOODBYE
    hallBuf->hall[1].hroomflags[thisRoom].inhall = TRUE;
#endif
    bit_set(hallBuf->hall[1].inhall, thisRoom);

    putHall();  /* save it */

    sprintf(msgBuf->mbtext, "%s> created by %s", roomname, logBuf.lbname);

    trap(msgBuf->mbtext, T_NEWROOM);

    aideMessage();

    logBuf.lbroom[thisRoom].lbgen  = roomBuf.rbgen;
    /* logBuf.lbroom[thisRoom].lvisit = 0; */
    logBuf.newpointer[thisRoom] = cfg.newest;

}


/************************************************************************/
/*      killempties() aide fn: to kill empty rooms                      */
/************************************************************************/
void killempties(void)
{
    label oldName;
    int  rm, roomExists();
    int  goodRoom, slot;

#ifdef NEWMSGTAB
    ulong i;
#else
    int i;
#endif

    char    text[LABELSIZE+10];

    sprintf(msgBuf->mbtext, "The following empty rooms deleted by %s: ",
                                                logBuf.lbname);
    trap(msgBuf->mbtext, T_AIDE);

    strcpy(oldName, roomBuf.rbname);

    for (slot = 0;  slot < MAXROOMS;  slot++)
    {
        if (roomTab[slot].rtflags.INUSE)
        {
            goodRoom = FALSE;

            if ( roomTab[slot].rtflags.PERMROOM
              || roomTab[slot].rtflags.MSDOSDIR
              || roomTab[slot].rtflags.SHARED
              || roomTab[slot].rtflags.APLIC
               )
            {
                goodRoom = TRUE;
            } 
            else 
            {


#ifdef NEWMSGTAB
                for (i = 0; i < sizetable(); ++i)
#else
                for (i = 0; i < (int)sizetable(); ++i)
#endif
                {

#ifdef NEWMSGTAB
                    if (getRoomNum(i) == (uchar)slot)
#else
                    if (msgTab_mtroomno[i] == (uchar)slot)
#endif
                   
                         goodRoom = TRUE;
                }
            }

            if (!goodRoom)
            {
                getRoom(slot);
                
                sprintf(text, "Kill %s", roomBuf.rbname);
                if ((i = getYesNo(text, 3)) == TRUE)
                {
                    /* Nuke the room */
                    strcat(msgBuf->mbtext, roomBuf.rbname);
                    strcat(msgBuf->mbtext, "> ");
                    memset(&roomBuf, 0, sizeof(roomBuf));
                    putRoom(slot);
                    noteRoom();
                }
                if (i==2)
                {
                    break;
                }
            }
        }
    }

    if ((rm=roomExists(oldName)) != ERROR)  getRoom(rm);
    else                                    getRoom(LOBBY);

    aideMessage();
}

/************************************************************************/
/*      killroom() aide fn: to kill current room                        */
/************************************************************************/
void killroom(void)
{
#ifdef NEWMSGTAB
    ulong i;
#else
    int i;
#endif
    int j;


    if (thisRoom == LOBBY   || thisRoom == MAILROOM
    || thisRoom == AIDEROOM || thisRoom == DUMP    )
    {
        doCR();
        mPrintf(" Cannot kill %s>, %s>, %s), or %s>",
            roomTab[LOBBY   ].rtname,
            roomTab[MAILROOM].rtname,
            roomTab[AIDEROOM].rtname,
            roomTab[DUMP    ].rtname );
        return;
    }
    if (!getYesNo(confirm, 0))  return;

#ifdef NEWMSGTAB
    for (i = 0;  i < sizetable();  i++)
#else
    for (i = 0;  i < (int)sizetable();  i++)
#endif

    {


#ifdef NEWMSGTAB
        if (getRoomNum(i) == thisRoom)
#else
        if (msgTab_mtroomno[i] == thisRoom)
#endif
        {
            changeheader((ulong)(cfg.mtoldest + i),
                3 /* Dump */, 255 /* no change */);
        }
    }

    /* kill current room from every hall */
    for (j = 0; j < MAXHALLS; j++)
    {
#ifdef GOODBYE
        hallBuf->hall[j].hroomflags[thisRoom].inhall = FALSE;
#endif
        bit_clear(hallBuf->hall[j].inhall, thisRoom);

#ifdef GOODBYE
        hallBuf->hall[j].hroomflags[thisRoom].window = FALSE;
#endif
        bit_clear(hallBuf->hall[j].window, thisRoom);

    }
    putHall();  /* update hall buffer */

    sprintf( msgBuf->mbtext, "%s> killed by %s",
        roomBuf.rbname,  logBuf.lbname );

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    roomBuf.rbflags.INUSE = FALSE;
    putRoom(thisRoom);
    noteRoom();
    getRoom(LOBBY);
}


/************************************************************************/
/*     massdelete()  sysop fn: to kill all msgs from person             */
/************************************************************************/
void massdelete(void)
{
    label who;
    char string[80];
    int namehash, killed = 0;
#ifdef NEWMSGTAB
    ulong i;
#else
    int i;
#endif

    getNormStr("who", who, NAMESIZE, ECHO);

    if (!strlen(who)) return;

    sprintf(string, "Delete all %s from %s", cfg.msgs_nym, who);

    namehash = hash(who);
    
    if (getYesNo(string, 0))
    {
#ifdef NEWMSGTAB
        for (i = 0; i < sizetable(); ++i)
#else
        for (i = 0; i < (int)sizetable(); ++i)
#endif
        {

#ifdef NEWMSGTAB
            if (getAuthHash(i) == namehash)
#else
            if (msgTab_mtauthhash[i] == namehash)
#endif
            {


#ifdef NEWMSGTAB
                if (getRoomNum(i) != 3 /* DUMP */ )
#else
                if (msgTab_mtroomno[i] != 3 /* DUMP */ )
#endif
                {
                    ++killed;

                    changeheader((ulong)(cfg.mtoldest + i),
                        3 /* Dump */, 255 /* no change */);
                }
            }
        }

        mPrintf("%d %s killed.", killed, cfg.msgs_nym);
        doCR();

        if (killed)
        {
            sprintf(msgBuf->mbtext, "All messages from %s deleted", who);
            trap(msgBuf->mbtext, T_SYSOP);
            
            roomtalley();
        }
    }
}

/************************************************************************/
/*      readbymsgno()  sysop fn: to read by message #                   */
/************************************************************************/
void readbymsgno(void)
{
    ulong msgno;
    ulong slot;
    char olddowhat;

    /* doCR(); */
    /* doCR(); */
    
    mPrintf("Range is %ld - %ld",(long)cfg.mtoldest, (long)cfg.newest );

    for(;;)
    {
        msgno = getNumber("Message number to read",
            (long)0, (long)10000000L, -1l);

        if (!msgno) return;

        if ( (msgno < cfg.mtoldest) ||
             (msgno > cfg.newest  ))
        {
            mPrintf("Value out of range.");
            doCR();
            return;
        }

        slot = indexslot(msgno);

        if (!mayseeindexmsg(slot)) 
        {
            mPrintf("%s not viewable.", cfg.msg_nym);

            /* mPrintf("Message not viewable."); */

            /* doCR(); */
        }
        else
        {
            verbose = TRUE;
            copyflag = FALSE;


            dotoMessage = NO_SPECIAL;

            olddowhat = dowhat;
            dowhat = READMESSAGE;
            dotoMessage = NO_SPECIAL;

            printMessage(msgno);
            dowhat = olddowhat;

            if (dotoMessage == MARK_IT)
                markIt();
            if (dotoMessage == PULL_IT)
                pullIt();
        }

        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);
        doCR();

    }
}


#ifdef GOODBYE
/************************************************************************/
/*      readbymsgno()  sysop fn: to read by message #                   */
/************************************************************************/
void readbymsgno(void)
{
    ulong msgno;
    int slot;

    doCR();
    
    msgno = (ulong) getNumber("Message number to read",
        (long)cfg.mtoldest, (long)cfg.newest, -1l);

    slot = indexslot(msgno);

    if (!mayseeindexmsg(slot)) 
    {
        mPrintf("%s not viewable.", cfg.msg_nym);
    }
    else
    {
        printMessage(msgno, (char)1);
    }

    doCR();
}
#endif

/************************************************************************/
/************************************************************************/
void moveRoom(int offset)
{
    int  rp1, rp2;
    
    mPrintf("Move room"); doCR(); doCR();
    rp1 = roomPosSlot(thisRoom);
    if  (   
            (rp1 > (LOBBY+1)  || offset ==  1)    
         && (rp1 < (MAXROOMS) || offset == -1)
         && (thisRoom != LOBBY)
        )
    {
        rp2 = rp1 + offset;
    
        roomPos[rp1] = roomPos[rp2];
        roomPos[rp2] = thisRoom;
        
        mPrintf("Room moved to just after %s.",
                roomTab[roomPos[rp2-1]].rtname);
        doCR();    
    }
}


