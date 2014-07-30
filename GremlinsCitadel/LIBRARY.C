/************************************************************************/
/*                            library.c                                 */
/*                                                                      */
/*                  Routines used by Ctdl & Confg                       */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              contents                                */
/*                                                                      */
/*      getGroup()              loads groupBuffer                       */
/*      putGroup()              stores groupBuffer to disk              */
/*                                                                      */
/*      getHall()               loads hallBuffer                        */
/*      putHall()               stores hallBuffer to disk               */
/*                                                                      */
/*      getLog()                loads requested CTDLLOG record          */
/*      putLog()                stores a logBuffer into citadel.log     */
/*                                                                      */
/*      getRoom()               load given room into RAM                */
/*      putRoom()               store room to given disk slot           */
/*                                                                      */
/*      writeTables()           writes all system tables to disk        */
/*      readTables()            loads all tables into ram               */
/*                                                                      */
/*      allocateTables()        allocate table space with halloc        */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*      getGrooup() loads group data into RAM buffer                    */
/************************************************************************/
void getGroup(void)
{
    fseek(grpfl, 0L, 0);

    if (fread(&grpBuf, sizeof grpBuf, 1, grpfl) != 1)
    {
        crashout("getGroup-EOF detected!");
    }
}

/************************************************************************/
/*      putGroup() stores group data into grp.cit                       */
/************************************************************************/
void putGroup(void)
{
    fseek(grpfl, 0L, 0);

    if (fwrite(&grpBuf, sizeof grpBuf, 1, grpfl) != 1)
    {
        crashout("putGroup-write fail!");
    }
    fflush(grpfl);
}

/************************************************************************/
/*      getHall() loads hall data into RAM buffer                       */
/************************************************************************/
void getHall(void)
{
    int i;
    
    fseek(hallfl, 0L, 0);

    if (fread(hallBuf, sizeof (struct hallBuffer), 1, hallfl) != 1)
    {
        crashout("getHall-EOF detected!");
    }

#ifdef NEWMSGTAB    
    if (fread(roomPos, MAXROOMS, 2, hallfl) != 2)
#else
    if (fread(roomPos, MAXROOMS, 1, hallfl) != 1)
#endif

    {
        cPrintf("\nCreating room position table.\n");
        
        for (i=0; i<MAXROOMS; i++)
            roomPos[i] = i;
            
        putHall();
    }
}

/************************************************************************/
/*      putHall() stores group data into hall.cit                       */
/************************************************************************/
void putHall(void)
{
    fseek(hallfl, 0L, 0);

    if ( fwrite(hallBuf, sizeof (struct hallBuffer), 1, hallfl) != 1)
    {
        crashout("putHall-write fail!");
    }


#ifdef NEWMSGTAB
    if ( fwrite(roomPos, MAXROOMS, 2, hallfl) != 2)
#else
    if ( fwrite(roomPos, MAXROOMS, 1, hallfl) != 1)
#endif
    {
        crashout("putHall-write fail!");
    }
    
    fflush(hallfl);
}

/************************************************************************/
/*      getLog() loads requested log record into RAM buffer             */
/************************************************************************/
void getLog(struct logBuffer *lBuf, int n)
{
    long int s;

    if (lBuf == &logBuf)  thisLog = n;

    s = (long)n * (long)sizeof logBuf;

    fseek(logfl, s, 0);

    if (fread(lBuf, sizeof logBuf, 1, logfl) != 1)
    {
        crashout("getLog-EOF detected!");
    }
}

/************************************************************************/
/*      putLog() stores given log record into log.cit                   */
/************************************************************************/
void putLog(struct logBuffer *lBuf, int n)
{
    long int s;
    int i;
    
    s = (long)n * (long)(sizeof(struct logBuffer));

    fseek(logfl, s, 0);  

    if (fwrite(lBuf, sizeof logBuf, 1, logfl) != 1)
    {
        crashout("putLog-write fail!");
    }
    fflush(logfl);
    
    for ( i = 0;  i < cfg.MAXLOGTAB;  i++)
    {
        if (n == logTab[i].ltlogSlot)
        {
            log2tab(&logTab[i], lBuf);
            logTab[i].ltlogSlot = n;
            break;
        }
    }
}

/************************************************************************/
/*      getRoom()                                                       */
/************************************************************************/
void getRoom(int rm)
{
    long int s;

    /* load room #rm into memory starting at buf */
    thisRoom    = rm;
    s = (long)rm * (long)sizeof roomBuf;

    fseek(roomfl, s, 0);
    if (fread(&roomBuf, sizeof roomBuf, 1, roomfl) != 1)  
    {
        crashout("getRoom-EOF detected!");
    }

#ifdef GOODBYE    
    if (roomBuf.rbflags.MSDOSDIR)
    {
        if (changedir(roomBuf.rbdirname) == -1)
        {
            roomBuf.rbflags.MSDOSDIR = FALSE;
            roomBuf.rbflags.DOWNONLY = FALSE;
        
            noteRoom();
            putRoom(rm);
            
            sprintf(msgBuf->mbtext, 
                    "%s>'s directory unfound.\n Directory: %s\n", 
                    roomBuf.rbname, 
                    roomBuf.rbdirname);
            aideMessage();
        }
    }
#endif
}


/* closes invalid directory rooms */
void checkdir(void)
{
    if (roomBuf.rbflags.MSDOSDIR)
    {
        if (changedir(roomBuf.rbdirname) == -1)
        {
            roomBuf.rbflags.MSDOSDIR = FALSE;
            roomBuf.rbflags.DOWNONLY = FALSE;
        
            noteRoom();
            putRoom(thisRoom);
            
            sprintf(msgBuf->mbtext, 
                    "%s>'s directory not found.\n Directory: %s\n", 
                    roomBuf.rbname, 
                    roomBuf.rbdirname);
            aideMessage();
        }
    }
}

/************************************************************************/
/*      putRoom() stores room in buf into slot rm in room.buf           */
/************************************************************************/
void putRoom(int rm)
{
    long int s;

    s = (long)rm * (long)sizeof roomBuf;

    fseek(roomfl, s, 0);
    if (fwrite(&roomBuf, sizeof roomBuf, 1, roomfl) != 1)
    {
        crashout("putRoom-write failed!");
    }
    fflush(roomfl);
}

/************************************************************************/
/*      readTables()  loads all tables into ram                         */
/************************************************************************/
readTables()
{
    FILE  *fd;

    getcwd(etcpath, 64);

    /*
     * ETC.DAT
     */
    if ((fd  = fopen("etc.dat" , "rb")) == NULL)
        return(FALSE);
    if (!fread(&cfg, sizeof cfg, 1, fd))
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("etc.dat");

    changedir(cfg.homepath);

    allocateTables();

    /*
     * LOG.TAB
     */
    if ((fd  = fopen("log.tab" , "rb")) == NULL)
        return(FALSE);
    if (!fread(logTab, sizeof(struct lTable), cfg.MAXLOGTAB, fd))
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("log.tab" );

    /*
     * MSG.TAB
     */
    if (readMsgTab() == FALSE)  return FALSE;

    /*
     * ROOM.TAB
     */
    if ((fd = fopen("room.tab", "rb")) == NULL)
        return(FALSE);
    if (!fread(roomTab, sizeof(struct rTable), MAXROOMS, fd))
    {
        fclose(fd);
        return FALSE;
    }
    fclose(fd);
    unlink("room.tab");


/* new cron.tab stuff */

    readcron();         /* was being read before homepath set! */
    readCrontab();
    unlink("cron.tab");

/* end of new cron.tab stuff */


    return(TRUE);
}

/************************************************************************/
/*      writeTables()  stores all tables to disk                        */
/************************************************************************/
void writeTables(void)
{
    FILE  *fd;

    changedir(etcpath);

    if ((fd     = fopen("etc.dat" , "wb")) == NULL)
    {
        crashout("Can't make Etc.dat");
    }
    /* write out Etc.dat */
    fwrite(&cfg, sizeof cfg, 1, fd);
    fclose(fd);

    changedir(cfg.homepath);

    if ((fd  = fopen("log.tab" , "wb")) == NULL)
    {
        crashout("Can't make Log.tab");
    }
    /* write out Log.tab */
    fwrite(logTab, sizeof(struct lTable), cfg.MAXLOGTAB, fd);
    fclose(fd);
 
    writeMsgTab();

    if ((fd = fopen("room.tab", "wb")) == NULL)
    {
        crashout("Can't make Room.tab");
    }
    /* write out Room.tab */
    fwrite(roomTab, sizeof(struct rTable), MAXROOMS, fd);
    fclose(fd);

/* new cron.tab stuff */

    writeCrontab();

/* end of new cron.tab stuff */


    changedir(etcpath);
}


/************************************************************************/
/*    allocateTables()   allocate msgTab and logTab                     */
/************************************************************************/
void allocateTables(void)
{
    logTab =  _fcalloc(cfg.MAXLOGTAB+1, sizeof(struct lTable));

    if (logTab == NULL)
        crashout("Error allocating logTab \n");

#ifdef NEWMSGTAB
    createMsgTab();
#else

    msgTab_mtmsgflags = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtmsgflags));
    msgTab_mtmsgLocLO = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtmsgLocLO));
    msgTab_mtmsgLocHI = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtmsgLocHI));
    msgTab_mtroomno   = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtroomno));
    msgTab_mttohash   = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mttohash));
    msgTab_mtauthhash = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtauthhash));
    msgTab_mtomesg    = _fcalloc(cfg.nmessages+1, sizeof(*msgTab_mtomesg));

    if (msgTab_mtmsgflags == NULL || 
        msgTab_mtmsgLocLO == NULL ||
        msgTab_mtmsgLocHI == NULL ||
        msgTab_mtroomno   == NULL || 
        msgTab_mttohash   == NULL || 
        msgTab_mtauthhash == NULL ||
        msgTab_mtomesg    == NULL )
        crashout("Error allocating msgTab \n");
#endif

    if (
          ((lBuf2   = _fcalloc(1,          sizeof(struct logBuffer ))) == NULL)
       || ((talleyBuf=_fcalloc(1,          sizeof(struct talleyBuffer))) == NULL)
       || ((doors   = _fcalloc(MAXDOORS,   sizeof(struct ext_door  ))) == NULL)
       || ((edit    = _fcalloc(MAXEXTERN,  sizeof(struct ext_editor))) == NULL)
       || ((hallBuf = _fcalloc(1,          sizeof(struct hallBuffer))) == NULL)
       || ((extrn   = _fcalloc(MAXEXTERN,  sizeof(struct ext_prot  ))) == NULL)
       || ((roomTab = _fcalloc(MAXROOMS,   sizeof(struct rTable    ))) == NULL)
       || ((msgBuf  = _fcalloc(1,          sizeof(struct msgB      ))) == NULL)
       || ((msgBuf2 = _fcalloc(1,          sizeof(struct msgB      ))) == NULL)

#ifdef NEWMSGTAB
       || ((roomPos = _fcalloc(MAXROOMS,   2                        )) == NULL)
#else
       || ((roomPos = _fcalloc(MAXROOMS,   1                        )) == NULL)
#endif

       )
    {
        crashout("Can not allocate space for tables");
    }
}

/************************************************************************/
/*    freeTables()   allocate msgTab and logTab                         */
/************************************************************************/
void freeTables(void)
{
    _ffree((void far *)logTab);

#ifdef NEWMSGTAB
    destroyMsgTab();
#else
    
    _ffree((void far *)msgTab_mtmsgflags);
    _ffree((void far *)msgTab_mtmsgLocLO);
    _ffree((void far *)msgTab_mtmsgLocHI);
    _ffree((void far *)msgTab_mtroomno);
    _ffree((void far *)msgTab_mttohash);
    _ffree((void far *)msgTab_mtauthhash);
    _ffree((void far *)msgTab_mtomesg);
#endif


    _ffree((void far *)roomPos  );
    _ffree((void far *)msgBuf2  );
    _ffree((void far *)msgBuf   );
    _ffree((void far *)roomTab  );
    _ffree((void far *)extrn    );
    _ffree((void far *)hallBuf  );
    _ffree((void far *)edit     );
    _ffree((void far *)doors    );
    _ffree((void far *)talleyBuf);
    _ffree((void far *)lBuf2    );
}

#ifndef NEWMSGTAB
/* -------------------------------------------------------------------- */
/*  readMsgTab()     Avoid the 64K limit. (stupid segments)             */
/* -------------------------------------------------------------------- */
int readMsgTab(void)
{
    FILE *fd;
    char temp[80];

    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    if ((fd  = fopen(temp , "rb")) == NULL)
        return(FALSE);

    if (!fread(msgTab_mtmsgflags, sizeof(*msgTab_mtmsgflags), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mtmsgLocLO, sizeof(*msgTab_mtmsgLocLO), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mtmsgLocHI, sizeof(*msgTab_mtmsgLocHI), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mtroomno, sizeof(*msgTab_mtroomno), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mttohash, sizeof(*msgTab_mttohash), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mtauthhash, sizeof(*msgTab_mtauthhash), cfg.nmessages, fd)) return(FALSE);
    if (!fread(msgTab_mtomesg, sizeof(*msgTab_mtomesg), cfg.nmessages, fd)) return(FALSE);
    
    fclose(fd);
    unlink(temp);

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  writeMsgTab()       Avoid the 64K limit. (stupid segments)          */
/* -------------------------------------------------------------------- */
void writeMsgTab(void)
{
    FILE *fd;
    char temp[80];

    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    if ((fd  = fopen(temp , "wb")) == NULL)
        return;

    fwrite(msgTab_mtmsgflags, sizeof(*msgTab_mtmsgflags), cfg.nmessages , fd);
    fwrite(msgTab_mtmsgLocLO, sizeof(*msgTab_mtmsgLocLO), cfg.nmessages , fd);
    fwrite(msgTab_mtmsgLocHI, sizeof(*msgTab_mtmsgLocHI), cfg.nmessages , fd);
    fwrite(msgTab_mtroomno, sizeof(*msgTab_mtroomno), cfg.nmessages , fd);
    fwrite(msgTab_mttohash, sizeof(*msgTab_mttohash), cfg.nmessages , fd);
    fwrite(msgTab_mtauthhash, sizeof(*msgTab_mtauthhash), cfg.nmessages , fd);
    fwrite(msgTab_mtomesg, sizeof(*msgTab_mtomesg), cfg.nmessages , fd);

    fclose(fd);
}
#else

/* -------------------------------------------------------------------- */
/*  readMsgTab()     Avoid the 64K limit. (stupid segments)             */
/* -------------------------------------------------------------------- */
int readMsgTab(void)
{
    FILE *fd;
    char temp[80];
    struct messagetable *lmt;
    ulong i;

    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    if ((fd  = fopen(temp , "rb")) == NULL)
        return(FALSE);

    for (i = 0; i < cfg.nmessages; i += MSGtABpERbLK)
        {
        lmt = getMsgTab(i);
        if (!fread(lmt, 16384, 1, fd)) return(FALSE);
        }
    
    fclose(fd);
    unlink(temp);

    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  writeMsgTab()     Avoid the 64K limit. (stupid segments)            */
/* -------------------------------------------------------------------- */
void writeMsgTab(void)
{
    FILE *fd;
    char temp[80];
    struct messagetable *lmt;
    ulong i;

    sprintf(temp, "%s\\%s", cfg.homepath, "msg.tab");

    if ((fd  = fopen(temp , "wb")) == NULL)
        return;

    for (i = 0; i < cfg.nmessages; i += MSGtABpERbLK)
        {
        lmt = getMsgTab(i);
        fwrite(lmt, 16384, 1 , fd);
        }

    fclose(fd);
}

#endif
