/************************************************************************/
/*                              confg.c                                 */
/*      configuration program for Citadel bulletin board system.        */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*      buildhalls()            builds hall-table (all rooms in Maint.) */
/*      buildroom()             builds a new room according to msg-buf  */
/*      clearaccount()          sets all group accounting data to zero  */
/*      configcit()             the main configuration for citadel      */
/*      illegal()               abort config.exe program                */
/*      initfiles()             opens & initalizes any missing files    */
/*      logInit()               indexes log.dat                         */
/*      logSort()               Sorts 2 entries in logTab               */
/*      readaccount()           reads grpdata.cit values into grp struct*/
/*      readconfig()            reads config.cit values                 */
/*      RoomTabBld()            builds room.tab, index's room.dat       */
/*      zapGrpFile()            initializes grp.dat                     */
/*      zapHallFile()           initializes hall.dat                    */
/*      zapLogFile()            initializes log.dat                     */
/*      zapRoomFile()           initializes room.dat                    */
/************************************************************************/

/************************************************************************/
/*                External variable declarations in CONFG.C             */
/************************************************************************/
char  *grpFile, *hallFile, *logFile, msgFile[64], *roomFile;

long  newmessagek; 
int   newmaxlogtab;       /* New size values              */

char   resizeMsg = FALSE,               /* Resize Msg-file?             */
       resizeLog = FALSE;               /* Resize Log-file?             */

void checkresize(void);
void resizemsgfile(void);
void resizelogfile(void);
BOOL resize_putMessage(void);
          
/************************************************************************/
/*      buildhalls()  builds hall-table (all rooms in Maint.)           */
/************************************************************************/
void buildhalls(void)
{
    int i;

    doccr(); cPrintf("Building hall file "); doccr();

    for (i = 4; i < MAXROOMS; ++i)
    {
        if (roomTab[i].rtflags.INUSE)
        {
#ifdef GOODBYE
            hallBuf->hall[1].hroomflags[i].inhall = 1;  /* In Maintenance */
            hallBuf->hall[1].hroomflags[i].window = 0;  /* Not a Window   */
#endif
            bit_set(  hallBuf->hall[1].inhall, i);  /* In Maintenance */
            bit_clear(hallBuf->hall[1].window, i);  /* Not a Window */
        }
    }
    putHall();
}

/************************************************************************/
/*      buildroom()  builds a new room according to msg-buf             */
/************************************************************************/
void buildroom(void)
{
    int roomslot;

    if (*msgBuf->mbcopy) return;
    roomslot = msgBuf->mbroomno;

    if (msgBuf->mbroomno < MAXROOMS)
    {
        getRoom(roomslot);

        if ((strcmp(roomBuf.rbname, msgBuf->mbroom) != SAMESTRING)
        || (!roomBuf.rbflags.INUSE))
        {
            if (msgBuf->mbroomno > 3)
            {
                roomBuf.rbflags.INUSE     = TRUE;
                roomBuf.rbflags.PERMROOM  = FALSE;
                roomBuf.rbflags.MSDOSDIR  = FALSE;
                roomBuf.rbflags.GROUPONLY = FALSE;
                roomBuf.rbroomtell[0]     = '\0';
                roomBuf.rbflags.PUBLIC    = TRUE;
            }
            strcpy(roomBuf.rbname, msgBuf->mbroom);

            putRoom(thisRoom);
        }
    }
}

/************************************************************************/
/*      clearaccount()  initializes all group data                      */
/************************************************************************/
void clearaccount(void)
{
    int i;
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++)
    {
        /* init days */
        for ( i = 0; i < 7; i++ )
            accountBuf.group[groupslot].account.days[i] = 1;

        /* init hours & special hours */
        for ( i = 0; i < 24; i++ )
        {
            accountBuf.group[groupslot].account.hours[i]   = 1;
            accountBuf.group[groupslot].account.special[i] = 0;
        }

        accountBuf.group[groupslot].account.have_acc      = FALSE;
        accountBuf.group[groupslot].account.dayinc        = 0.f;
        accountBuf.group[groupslot].account.sp_dayinc     = 0.f;
        accountBuf.group[groupslot].account.maxbal        = 0.f;
        accountBuf.group[groupslot].account.priority      = 0.f;
        accountBuf.group[groupslot].account.dlmult        = -1;
        accountBuf.group[groupslot].account.ulmult        =  1;

    }
}

/************************************************************************/
/*      configcit() the <main> for configuration                        */
/************************************************************************/
void configcit(void)
{
    char oldresizelog = FALSE, oldresizemsg = FALSE;

    fcloseall();

    /* read config.cit */
    readconfig(0);

    /* move to home-path */
    changedir(cfg.homepath);

    /* initialize & open any files */
    initfiles();

    /* allocate the tables here so readconfig() can be called from sysop menu*/
    allocateTables();

    if (msgZap )  zapMsgFile();
    if (roomZap)  zapRoomFile();
    if (logZap )  zapLogFile();
    if (grpZap )  zapGrpFile();
    if (hallZap)  zapHallFile();

    if (roomZap && !msgZap)  roomBuild = TRUE;
    if (hallZap && !msgZap)  hallBuild = TRUE;

    logInit();
    msgInit();
    RoomTabBld();

    if (hallBuild)  buildhalls();

    if (resizeLog)  resizelogfile();
    if (resizeMsg)  resizemsgfile();


    if (resizeLog || resizeMsg)
    {
        oldresizelog = resizeLog;
        oldresizemsg = resizeMsg;
        resizeLog = FALSE;
        resizeMsg = FALSE;

        fcloseall();

        cfg.messagek  = newmessagek; 
        cfg.MAXLOGTAB = newmaxlogtab;

        initfiles();
    }

    if (oldresizelog)
    {
        _ffree((void far *)logTab);

        logTab =  _fcalloc(cfg.MAXLOGTAB+1, sizeof(struct lTable));

        if (logTab == NULL)
            crashout("Error allocating logTab \n");

        logInit();
    }

    if (oldresizemsg)
    {
#ifdef NEWMSGTAB
     destroyMsgTab();
     createMsgTab();
#else

        _ffree((void far *)msgTab_mtmsgflags);
        _ffree((void far *)msgTab_mtmsgLocLO);
        _ffree((void far *)msgTab_mtmsgLocHI);
        _ffree((void far *)msgTab_mtroomno);
        _ffree((void far *)msgTab_mttohash);
        _ffree((void far *)msgTab_mtauthhash);
        _ffree((void far *)msgTab_mtomesg);

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

        msgInit();
    }

    doccr();
    doccr();
    cPrintf("Config Complete.");
    doccr();
    pause(200);

    fcloseall();
}


#ifdef GOODBYE
/***********************************************************************/
/*    illegal() Prints out configur error message and aborts           */
/***********************************************************************/
void illegal(char *errorstring)
{
    doccr();
    cPrintf("%s", errorstring);
    doccr();
    cPrintf("Fatal error. Aborting."); doccr();
    exit(200);
}
#endif

/***********************************************************************/
/*    illegal() Prints out configur error message and aborts           */
/***********************************************************************/
void illegal(const char *fmt, ...)
{
    char buff[256];
    va_list ap;
    
    va_start(ap, fmt);
    vsprintf(buff, fmt, ap);
    va_end(ap);

    /* move to home-path */
    changedir(cfg.homepath);
    
    doccr();
    cPrintf("%s", buff);
    doccr();
    cPrintf("Fatal error in configuration. Aborting."); doccr();
    curson();
    exit(7);
}


/************************************************************************/
/*      initfiles() -- initializes files, opens them                    */
/************************************************************************/
void initfiles(void)
{
    chdir(cfg.homepath);

    if (cfg.msgpath[ (strlen(cfg.msgpath) - 1) ]  == '\\')
        cfg.msgpath[ (strlen(cfg.msgpath) - 1) ]  =  '\0';

    sprintf(msgFile, "%s\\%s", cfg.msgpath, "msg.dat");

    grpFile     = "grp.dat" ;
    hallFile    = "hall.dat";
    logFile     = "log.dat" ;
    roomFile    = "room.dat";

    checkresize();

    /* open group file */
    if ((grpfl = fopen(grpFile, "r+b")) == NULL)
    {
        cPrintf(" %s not found. ", grpFile);  doccr();
        if ((grpfl = fopen(grpFile, "w+b")) == NULL)
            illegal("Can't create the group file!");
        {
            cPrintf(" It will be initialized."); doccr();
            grpZap = TRUE;
        }
    }

    /* open hall file */
    if ((hallfl = fopen(hallFile, "r+b")) == NULL)
    {
        cPrintf(" %s not found. ", hallFile); doccr();
        if ((hallfl = fopen(hallFile, "w+b")) == NULL)
            illegal("Can't create the hall file!");
        {
            cPrintf(" It will be initialized.");  doccr();
            hallZap = TRUE;
        }
    }

    /* open log file */
    if ((logfl = fopen(logFile, "r+b")) == NULL)
    {
        cPrintf(" %s not found. ", logFile); doccr();
        if ((logfl = fopen(logFile, "w+b")) == NULL)
            illegal("Can't create log file!");
        {
            cPrintf(" It will be initialized.");  doccr();
            logZap = TRUE;
        }
    }

    /* open message file */
    if ((msgfl = fopen(msgFile, "r+b")) == NULL)
    {
        cPrintf(" %s not found. ", "msg.dat");  doccr();
        if ((msgfl = fopen(msgFile, "w+b")) == NULL)
            illegal("Can't create message file!");
        {
            cPrintf(" It will be initialized.");  doccr();
            msgZap = TRUE;
        }
    }

    /* open room file */
    if ((roomfl = fopen(roomFile, "r+b")) == NULL)
    {
        cPrintf(" %s not found. ", roomFile);  doccr();
        if ((roomfl = fopen(roomFile, "w+b")) == NULL)
            illegal("Can't create room file!");
        {
            cPrintf(" It will be initialized."); doccr();
            roomZap = TRUE;
        }
    }
}

/************************************************************************/
/*      logInit() indexes log.dat                                       */
/************************************************************************/
void logInit(void)
{
    int i;
    int count = 0;

    doccr();  doccr();
    cPrintf("Building log table "); doccr();

    cfg.callno = 0l;

    rewind(logfl);
    
    /* clear logTab */
    for (i = 0; i < cfg.MAXLOGTAB; i++) 
        memset(&logTab[i], 0, sizeof(&logTab[i]));

    /* load logTab: */
    for (thisLog = 0;  thisLog < cfg.MAXLOGTAB;  thisLog++)
    {
  
        cPrintf("log#%d\r",thisLog);

        getLog(&logBuf, thisLog);

        if (logBuf.callno > cfg.callno)  cfg.callno = logBuf.callno;

        /* 
         * count valid entries:             
         */
        if (logBuf.lbflags.L_INUSE == 1)  count++;
          
        /* 
         * copy relevant info into index:   
         */
        log2tab(&logTab[thisLog], &logBuf);
        logTab[thisLog].ltlogSlot= thisLog;
        
#ifdef OLD        
        logTab[thisLog].ltcallno = logBuf.callno;
        logTab[thisLog].permanent = logBuf.lbflags.PERMANENT;
        if (logBuf.lbflags.L_INUSE == 1)
        {
            logTab[thisLog].ltnmhash = hash(logBuf.lbname);
            logTab[thisLog].ltinhash = hash(logBuf.lbin  );
            logTab[thisLog].ltpwhash = hash(logBuf.lbpw  );
        }
        else
        {
            logTab[thisLog].ltnmhash = 0;
            logTab[thisLog].ltinhash = 0;
            logTab[thisLog].ltpwhash = 0;
        }
#endif        
    
    }
    doccr();
    cPrintf("%lu calls.", cfg.callno);
    doccr();
    cPrintf("%d valid log entries.", count);  

    qsort(logTab, 
          (size_t)cfg.MAXLOGTAB, 
          (unsigned)sizeof(*logTab), 
          (QSORT_CMP_FNP)logSort);

/*  qsort(logTab, (size_t)cfg.MAXLOGTAB, (unsigned)sizeof(*logTab), logSort); 
*/
}

/************************************************************************/
/*      logSort() Sorts 2 entries in logTab                             */
/************************************************************************/
int logSort(struct lTable *s1, struct lTable *s2)
{
    if (s1->ltnmhash == 0 && s2->ltnmhash == 0)
        return 0;
    if (s1->ltnmhash == 0 && s2->ltnmhash != 0)
        return 1;
    if (s1->ltnmhash != 0 && s2->ltnmhash == 0)
        return -1;
    if (s1->ltcallno < s2->ltcallno)
        return 1;
    if (s1->ltcallno > s2->ltcallno)
        return -1;
    return 0;
}

/************************************************************************/
/*      readaccount()  reads grpdata.cit values into group structure    */
/************************************************************************/
void readaccount(void)
{                          
    FILE *fBuf;
    char line[90];
    char *words[256];
    int  i, j, k, l, count;
    int groupslot = ERROR;
    int hour;
   
    clearaccount();   /* initialize all accounting data */

    /* move to home-path */
    changedir(cfg.homepath);

    if ((fBuf = fopen("grpdata.cit", "r")) == NULL)  /* ASCII mode */
    {  
        cPrintf("Can't find Grpdata.cit!"); doccr();
        exit(200);
    }

    while (fgets(line, 90, fBuf) != NULL)
    {
        if (line[0] != '#')  continue;

        count = parse_it( words, line);

        for (i = 0; grpkeywords[i] != NULL; i++)
        {
            if (strcmpi(words[0], grpkeywords[i]) == SAMESTRING)
            {
                break;
            }
        }

        switch(i)
        {
            case GRK_DAYS:              
                if (groupslot == ERROR)  break;

                /* init days */
                for ( j = 0; j < 7; j++ )
                    accountBuf.group[groupslot].account.days[j] = 0;

                for (j = 1; j < count; j++)
                {
                    for (k = 0; daykeywords[k] != NULL; k++)
                    {
                        if (strcmpi(words[j], daykeywords[k]) == SAMESTRING)
                        {
                            break;
                        }
                    }
                    if (k < 7)
                        accountBuf.group[groupslot].account.days[k] = TRUE;
                    else if (k == 7)  /* any */
                    {
                        for ( l = 0; l < MAXGROUPS; ++l)
                            accountBuf.group[groupslot].account.days[l] = TRUE;
                    }
                    else
                    {
                        doccr();
                   cPrintf("Grpdata.Cit - Warning: Unknown day %s ", words[j]);
                        doccr();
                    }
                }
                break;

            case GRK_GROUP:             
                groupslot = groupexists(words[1]);
                if (groupslot == ERROR)
                {
                    doccr();
                    cPrintf("Grpdata.Cit - Warning: Unknown group %s", words[1]);
                    doccr();
                }
                accountBuf.group[groupslot].account.have_acc = TRUE;
                break;

            case GRK_HOURS:             
                if (groupslot == ERROR)  break;

                /* init hours */
                for ( j = 0; j < 24; j++ )
                    accountBuf.group[groupslot].account.hours[j]   = 0;

                for (j = 1; j < count; j++)
                {
                    if (strcmpi(words[j], "Any") == SAMESTRING)
                    {
                        for (l = 0; l < 24; l++)
                            accountBuf.group[groupslot].account.hours[l] = TRUE;
                    }
                    else
                    {
                        hour = atoi(words[j]);

                        if ( hour > 23 ) 
                        {
                            doccr();
                            cPrintf("Grpdata.Cit - Warning: Invalid hour %d ",
                            hour);
                            doccr();
                        }
                        else
                       accountBuf.group[groupslot].account.hours[hour] = TRUE;
                    }
                }
                break;

            case GRK_DAYINC:
                if (groupslot == ERROR)  break;

                if (count > 1)
                {
                    sscanf(words[1], "%f ",
                    &accountBuf.group[groupslot].account.dayinc);   /* float */
                }
                break;

            case GRK_DLMULT:
                if (groupslot == ERROR)  break;

                if (count > 1)
                {
                    sscanf(words[1], "%f ",
                    &accountBuf.group[groupslot].account.dlmult);   /* float */
                }
                break;

            case GRK_ULMULT:
                if (groupslot == ERROR)  break;

                if (count > 1)
                {
                    sscanf(words[1], "%f ",
                    &accountBuf.group[groupslot].account.ulmult);   /* float */
                }
                break;

            case GRK_PRIORITY:
                if (groupslot == ERROR)  break;

                if (count > 1)
                {
                    sscanf(words[1], "%f ",
                    &accountBuf.group[groupslot].account.priority);  /* float */
                }

                break;

            case GRK_MAXBAL:
                if (groupslot == ERROR)  break;

                if (count > 1)
                {
                    sscanf(words[1], "%f ",
                    &accountBuf.group[groupslot].account.maxbal);   /* float */
                }

                break;



            case GRK_SPECIAL:           
                if (groupslot == ERROR)  break;

                /* init hours */
                for ( j = 0; j < 24; j++ )
                    accountBuf.group[groupslot].account.special[j]   = 0;

                for (j = 1; j < count; j++)
                {
                    if (strcmpi(words[j], "Any") == SAMESTRING)
                    {
                        for (l = 0; l < 24; l++)
                            accountBuf.group[groupslot].account.special[l] = TRUE;
                    }
                    else
                    {
                        hour = atoi(words[j]);

                        if ( hour > 23 )
                        {
                            doccr();
                            cPrintf("Grpdata.Cit - Warning: Invalid special hour %d ", hour);
                            doccr();
                        }
                        else
                       accountBuf.group[groupslot].account.special[hour] = TRUE;
                    }

                }
                break;
        }

    }
    fclose(fBuf);
}

/************************************************************************/
/*      readprotocols() reads protocol.cit values into ext   structure  */
/************************************************************************/
void readprotocols(void)
{                          
    FILE *fBuf;
    char line[128];
    char *words[256];
    int  j, count;

    numDoors    = 0;
    extrncmd[0] = '\0' /* NULL */;
    editcmd[0]  = '\0' /* NULL */;
   
    /* move to home-path */
    changedir(cfg.homepath);

    if ((fBuf = fopen("external.cit", "r")) == NULL)  /* ASCII mode */
    {  
        cPrintf("Can't find external.cit!"); doccr();
        exit(200);
    }

    while (fgets(line, 125, fBuf) != NULL)
    {
        if (line[0] != '#')  continue;

        count = parse_it( words, line);

        if (strcmpi("#PROTOCOL", words[0]) == SAMESTRING)
        {
            j = strlen(extrncmd);

            if (strlen( words[1] ) > 19 )
              illegal("Protocol name too long; must be less than 20");
            if (strlen( words[3] ) > 39 )
              illegal("Receive command too long; must be less than 40");
            if (strlen( words[4] ) > 39 )
              illegal("Send command too long; must be less than 40");
            if (atoi(words[2]) < 0 || atoi(words[2]) > 1)
              illegal("Batch field bad; must be 0 or 1");
            if (atoi(words[3]) < 0 || atoi(words[3]) > 10 * 1024)
              illegal("Block field bad; must be 0 to 10K");
            if (j >= MAXEXTERN)
              illegal("Too many external protocols");
    
            strcpy(extrn[j].ex_name, words[1]);
            extrn[j].ex_batch = (char)atoi(words[2]);
            extrn[j].ex_block = atoi(words[3]);
            strcpy(extrn[j].ex_rcv,  words[4]);
            strcpy(extrn[j].ex_snd,  words[5]);
            extrncmd[j]   = (char)tolower(*words[1]);
            extrncmd[j+1] = '\0';
        }
        if (strcmpi("#EDITOR", words[0]) == SAMESTRING)
        {
            j = strlen(editcmd);

            if (strlen( words[1] ) > 19 )
              illegal("Protocol name too long; must be less than 20");
            if (strlen( words[3] ) > 29 )
              illegal("Command line too long; must be less than 30");
            if (atoi(words[2]) < 0 || atoi(words[2]) > 1)
              illegal("Local field bad; must be 0 or 1");
            if (j > 19)
              illegal("Only 20 external editors");
    
            strcpy(edit[j].ed_name,  words[1]);
            edit[j].ed_local  = (char)atoi(words[2]);
            strcpy(edit[j].ed_cmd,   words[3]);
            editcmd[j]    = (char)tolower(*words[1]);
            editcmd[j+1]                = '\0';
        }
        if (strcmpi("#DOOR", words[0]) == SAMESTRING)
        {
            if (count < 8)
              illegal("Too few arguments for #DOOR keyword");
            if (strlen( words[1] ) > NAMESIZE )
              illegal("Door name too long, must be less than 31");
            if (strlen( words[2] ) > 40 )
              illegal("Door command too long, must be less than 41");
            if (strlen( words[3] ) > NAMESIZE )
              illegal("Door group too long, must be less than 31");
            if (numDoors >= (MAXDOORS) )
              illegal("Too many #DOORs");
    
            strcpy(doors[numDoors].name,   words[1]);
            strcpy(doors[numDoors].cmd ,   words[2]);
            strcpy(doors[numDoors].group,  words[3]);
            doors[numDoors].CON     = atoi(words[4]);
            doors[numDoors].SYSOP   = atoi(words[5]);
            doors[numDoors].AIDE    = atoi(words[6]);
            doors[numDoors].DIR     = atoi(words[7]);
            numDoors++;
        }
    }
    fclose(fBuf);
}

/*
 * count the lines that start with keyword...
 *
int keyword_count(key, filename)
char *key;
char *filename;
{
    FILE *fBuf;
    char line[90];
    char *words[256];
    int  count = 0;
   
    changedir(cfg.homepath);

    if ((fBuf = fopen(filename, "r")) == NULL) 
    {  
        cPrintf("Can't find %s!", filename); doccr();
        exit(200);
    }

    while (fgets(line, 90, fBuf) != NULL)
    {
        if (line[0] != '#')  continue;

        parse_it( words, line);

        if (strcmpi(key, words[0]) == SAMESTRING)
          count++;
   }

   fclose(fBuf);

   return (count == 0 ? 1 : count);
} */

/************************************************************************/
/*      readconfig() reads config.cit values                            */
/*      ignore == 0 normal call before reconfiguring system             */
/*      ignore == 1 force read at startup with CTDL -E cmd line option  */
/*      ignore == 2 force read from sysop menu  '&'                     */
/************************************************************************/
void readconfig(char ignore) 
{
    FILE *fBuf;
    char line[256];
    char *words[256];
    int  i, j, k, l, count, att;
    char notkeyword[20];
    char valid = FALSE;
    char found[K_NWORDS+2];
    int  lineNo = 0;
    long file_pos;  /* for multi sigs */
    char bordercount = 0;  /* for borderlines */

    cfg.sig_first_pos = 100000000L;    /* multi sigs */
    cfg.sig_current_pos = 100000000L;  /* multi sigs */
/*   cfg.sig_num = 0;  */

    /* have optional keywords default to something */

    cfg.forward = 0;
    cfg.connectwait = 0;
    cfg.hangupdelay = 0;
    
    #define nochange  "%s invalid: can't change to '%s' directory"

    strcpy(cfg.msg_nym,  "message");
    strcpy(cfg.msgs_nym, "messages");
    strcpy(cfg.msg_done, "Saving");
    cfg.version = cfgver;

    cfg.downshift[0] = 0;

    cfg.checkCTS = FALSE;
    cfg.autoansi = 0;

    /* clear all borders */
    for (i=0; i < MAXBORDERS; i++)
        cfg.border[i][0] = '\0';
    
    for (i=0; i <= K_NWORDS; i++)
        found[i] = FALSE;

    found[K_BORDER]      = TRUE; /* don't bitch about no borders */
    found[K_MESSAGE_NYM] = TRUE; /* don't bitch about no message_nyms */

    found[K_CHECKCTS] = TRUE;
    found[K_FORWARD] = TRUE;

/* optional keywords */

#ifndef NEWMSGTAB
    found [K_MSGHEAPPAGES] = TRUE;
    found [K_VIRTMEM]      = TRUE;
#endif

    found [K_CONNECTWAIT]   = TRUE;
    found [K_DOWNSHIFT]    = TRUE;
    found [K_AUTOANSI]    = TRUE;

    found [K_HANGUPDELAY]  = TRUE;
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
                /* remove this as soon as i implement configurable prompts */


    found[K_PROMPT] = TRUE;     /* don't bitch about configurable prompt */


    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/


    if ((fBuf = fopen("config.cit", "r")) == NULL)  /* ASCII mode */
    {  
        cPrintf("Can't find Config.cit!"); doccr();
        exit(200);
    }


    for(;;)
    {
        file_pos = ftell(fBuf);

        if  (fgets(line, 254, fBuf) == NULL)
            break;

        lineNo++;

        if (line[0] != '#')  continue;

        count = parse_it( words, line);

        for (i = 0; keywords[i] != NULL; i++)
        {
            if (strcmpi(words[0], keywords[i]) == SAMESTRING)
            {
                break;
            }
        }

        if (keywords[i] == NULL)
        {
            cPrintf("CONFIG.CIT (%d) Warning: Unknown variable %s ", lineNo, 
                words[0]);
            doccr();
            continue;
        }
        else
        {                          /* allow multi sigs */
            if (found[i] == TRUE &&
            (i != K_BORDER) && (i != K_MESSAGE_NYM) && (i != K_NODESIG)

#ifndef NEWMSGTAB
          && (i != K_MSGHEAPPAGES)
          && (i != K_VIRTMEM)
#endif

          && (i != K_DOWNSHIFT)

          && (i != K_HANGUPDELAY)

          && (i != K_CHECKCTS)
          && (i != K_AUTOANSI)

/* optional keywords */
/*****************************/
/*****************************/
/*****************************/
/*****************************/
/*****************************/
/* remove this later */
&& (i != K_PROMPT)
&& (i != K_FORWARD)
&& (i != K_CONNECTWAIT)


/*****************************/
/*****************************/
/*****************************/
/*****************************/
/*****************************/


            )
            {
                cPrintf("CONFIG.CIT (%d) Warning: %s multiply defined!", lineNo, 
                    words[0]);
                doccr();
            }else{
                found[i] = TRUE;
            }
        }

        switch(i)
        {

#ifdef NEWMSGTAB

           case K_MSGHEAPPAGES:
                if (ignore == 2) break;

                cfg.msgHeapPages = (unsigned) atoi(words[1]);
                if (cfg.msgHeapPages < 2) cfg.msgHeapPages = 2;
                break;

            case K_VIRTMEM:
                if (ignore == 2) break;

                if (strlen(words[1]) > 63)
                    illegal("Virtmem filename too long; must be less than 64");

                strcpy(cfg.vmemfile, words[1]);
                break;
#else
            case K_MSGHEAPPAGES:
                break;
            case K_VIRTMEM:
                break;
#endif

            case K_CHECKCTS:
                cfg.checkCTS = atoi(words[1]);
                break;

            case K_ENTEROK:
                cfg.unlogEnterOk = (uchar)atoi(words[1]);
                break;

            case K_FORCELOGIN: 
                cfg.forcelogin   = (uchar)atoi(words[1]);
                break;

            case K_READOK:
                cfg.unlogReadOk = (uchar)atoi(words[1]);
                break;


            case K_ACCOUNTING:
                cfg.accounting = (uchar)atoi(words[1]);
                break;

            case K_IDLE_WAIT:
                cfg.idle = atoi(words[1]);
                break;

            case K_SCREENSAVE:

                cfg.fucking_stupid        = FALSE;
                cfg.really_fucking_stupid = FALSE;

                cfg.screensave = atoi(words[1]);
                if (count > 2)
                    cfg.fucking_stupid = atoi(words[2]);
                if (count > 3)
                    cfg.really_fucking_stupid = atoi(words[3]);
                break;

            case K_ATTR:
                sscanf(words[1], "%x ", &att); /* hex! */
                cfg.attr = (uchar)att;
                break;

            case K_WATTR:
                sscanf(words[1], "%x ", &att); /* hex! */
                cfg.wattr = (uchar)att;
                break;

            case K_CATTR:
                sscanf(words[1], "%x ", &att); /* hex! */
                cfg.cattr = (uchar)att;
                break;

            case K_BATTR:
                sscanf(words[1], "%x ", &att);    /* hex! */
                cfg.battr = (uchar)att;
                break;

            case K_UTTR:
                sscanf(words[1], "%x ", &att);     /* hex! */
                cfg.uttr = (uchar)att;
                break;

            case K_INIT_BAUD:
                cfg.initbaud = (uchar)atoi(words[1]);
                break;
            
            case K_READOLD:
                cfg.readOld = (uchar)atoi(words[1]);
                break;
            
            case K_LINES_SCREEN:
                cfg.linesScreen = (char)atoi(words[1]);
                break;

            case K_CONNECTWAIT:
                cfg.connectwait = (char)atoi(words[1]);
                break;

            case K_BIOS:
                cfg.bios = (uchar)atoi(words[1]);
                break;

            case K_DUMB_MODEM:
                cfg.dumbmodem    = (uchar)atoi(words[1]);
                break;

            case K_READLLOG:
                cfg.readluser    = (uchar)atoi(words[1]);
                break;

            case K_DATESTAMP:
                if (strlen( words[1] ) > 63 )
                    illegal("#DATESTAMP too long; must be less than 64");

                strcpy( cfg.datestamp, words[1] );
                break;

            case K_VDATESTAMP:
                if (strlen( words[1] ) > 63 )
                    illegal("#VDATESTAMP too long; must be less than 64");

                strcpy( cfg.vdatestamp, words[1] );
                break;

            case K_PROMPT:
                if (strlen( words[1] ) > 63 )
                    illegal("#PROMPT too long; must be less than 64");

                strcpy( cfg.prompt, words[1] );
                break;
            
            case K_ENTER_NAME:
                if (strlen( words[1] ) > NAMESIZE )
                {
                    illegal("#ENTER_NAME too long; must be less than 30");
                }
                strcpy( cfg.enter_name, words[1] );
                break;

#ifdef GOODBYE
            case K_VDATESTAMP:
                if (strlen( words[1] ) > 63 )
                    illegal("#VDATESTAMP too long; must be less than 64");

                strcpy( cfg.vdatestamp, words[1] );
                break;
#endif

            case K_MODERATE: 
                cfg.moderate     = (uchar)atoi(words[1]);
                break;

            case K_NETMAIL:
                cfg.netmail = (uchar)atoi(words[1]);
                break;

            case K_HELPPATH:  
                if (strlen( words[1] ) > 63 )
                    illegal("helppath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy( cfg.helppath, words[1] );  
                break;
            
            case K_NET_PREFIX:  
                if (strlen( words[1] ) > 30 )
                    illegal("#NET_PREFIX too long; must be less than 30");

                strcpy( cfg.netPrefix, words[1] );  
                break;
            
            case K_TRANSPATH:  
                if (strlen( words[1] ) > 63 )
                    illegal("#TRANSPATH too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy( cfg.transpath, words[1] );  
                break;

            case K_TEMPPATH:
                if (strlen( words[1] ) > 63 )
                illegal("temppath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);


                strcpy( cfg.temppath, words[1] );
                break;


            case K_HOMEPATH:
                if (strlen( words[1] ) > 63 )
                    illegal("homepath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy(cfg.homepath, words[1] );  
                break;

            case K_KILL:
                cfg.kill = (uchar)atoi(words[1]);
                break;

            case K_LINEFEEDS:
                cfg.linefeeds = (uchar)atoi(words[1]);
                break;

            case K_HANGUPDELAY:
                cfg.hangupdelay = atoi(words[1]);
                break;

#ifdef GOODBYE            
            case K_LOGINSTATS:
                cfg.loginstats = (uchar)atoi(words[1]);
                break;
#endif

            case K_MAXBALANCE:
                sscanf(words[1], "%f ", &cfg.maxbalance); /* float */
                break;

            case K_MAXLOGTAB:
                if (ignore) break;

                cfg.MAXLOGTAB    = atoi(words[1]);

                break;

            case K_MESSAGE_ROOM:
                cfg.MessageRoom = (char)atoi(words[1]);
                break;

            case K_AUTOANSI:
                cfg.autoansi = (char)atoi(words[1]);
                break;

            case K_FORWARD:
                cfg.forward = (char)atoi(words[1]);
                break;

            case K_NEWUSERAPP:
                if (strlen( words[1] ) > 12 )
                illegal("NEWUSERAPP too long; must be less than 13");

                strcpy( cfg.newuserapp, words[1] );
                break;

#ifdef GOODBYE
            case K_MAXTEXT:
                cfg.maxtext = atoi(words[1]);
                break;
#endif
#ifdef GOODBYE
            case K_MAX_WARN:
                cfg.maxwarn = (char)atoi(words[1]);
                break;
#endif

            case K_MDATA:

                if (ignore == 2) break;

                cfg.mdata   = atoi(words[1]);

                if ( (cfg.mdata < 1) || (cfg.mdata > 4) )
                {
                    illegal("MDATA port can only currently be 1, 2, 3 or 4");
                }
                break;

            case K_MAXFILES:
                cfg.maxfiles = atoi(words[1]);
                break;

            case K_MSGPATH:
                if (strlen(words[1]) > 63)
                    illegal("msgpath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy(cfg.msgpath, words[1]);  
                break;

            case K_F6PASSWORD:
                if (strlen(words[1]) > 19)
                    illegal("f6password too long; must be less than 20");

                strcpy(cfg.f6pass, words[1]);  
                break;

            case K_APPLICATIONS:
                if (strlen(words[1]) > 63)
                    illegal("applicationpath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy(cfg.aplpath, words[1]);  
                break;

            case K_MESSAGEK:
                if (ignore) break;
                cfg.messagek = atol(words[1]);
                break;

            case K_MODSETUP:
                if (strlen(words[1]) > 63)
                    illegal("Modsetup too long; must be less than 64");

                strcpy(cfg.modsetup, words[1]);  
                break;

            case K_DOWNSHIFT:
                if (strlen(words[1]) > 31)
                    illegal("Downshift too long; must be less than 64");

                strcpy(cfg.downshift, words[1]);  
                break;
                
            case K_DIAL_INIT:
                if (strlen(words[1]) > 63)
                    illegal("Dial_Init too long; must be less than 64");

                strcpy(cfg.dialsetup, words[1]);  
                break;
                
            case K_DIAL_PREF:
                if (strlen(words[1]) > 20)
                    illegal("Dial_Prefix too long; must be less than 20");

                strcpy(cfg.dialpref, words[1]);  
                break;

            case K_NEWBAL:
                sscanf(words[1], "%f ", &cfg.newbal);  /* float */
                break;

            case K_AIDEHALL:
                cfg.aidehall = (uchar)atoi(words[1]);
                break;




            case K_NMESSAGES:
                if (ignore) break;


#ifdef NEWMSGTAB
                cfg.nmessages  = atol(words[1]);
#else
                cfg.nmessages  = atoi(words[1]);
#endif


                break;

            case K_NODENAME:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("nodeName too long; must be less than 30");

                strcpy(cfg.nodeTitle, words[1]);  
                break;

            case K_NODEPHONE:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("nodePhone too long; must be less than 30");

                strcpy(cfg.nodephone, words[1]);  
                break;

            case K_SYSOP:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("sysop too long; must be less than 30");

                strcpy(cfg.sysop, words[1]);  
                break;
            
            case K_NODESIG:
                if (strlen(words[1]) > 90)
                    illegal("Signature too long; must be less than 90");

                /* found[i] = FALSE; */ /* disable multiply defined warnings */

                if (cfg.sig_first_pos == 100000000L)  /* multi sigs */
                {
                    cfg.sig_first_pos = file_pos;
                    strcpy(cfg.nodeSignature, words[1]);  
                }
                else 
                cfg.sig_current_pos = ftell(fBuf);
                /* cfg.sig_num++; */
                break;

            case K_NODEREGION:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("nodeRegion too long; must be less than 30");

                strcpy(cfg.nodeRegion, words[1]);
                break;
            
            case K_NODECONTRY:
                if (strlen(words[1]) > NAMESIZE)
                {
                    illegal("#nodecontry too long; must be less than 30");
                }

                strcpy(cfg.nodeContry, words[1]);
                break;

            case K_NOPWECHO:
                cfg.nopwecho = (unsigned char)atoi(words[1]);
                break;

            case K_NULLS:
                cfg.nulls = (uchar)atoi(words[1]);
                break;

            case K_OFFHOOK:
                cfg.offhook = (uchar)atoi(words[1]);
                break;

            case K_OLDCOUNT:
                cfg.oldcount = atoi(words[1]);
                break;

            case K_PRINTER:
                if (ignore == 2) break;

                if (strlen(words[1]) > 63)
                    illegal("printer too long; must be less than 64");

                strcpy(cfg.printer, words[1]);  
                break;

            case K_ROOMOK:
                cfg.nonAideRoomOk = (uchar)atoi(words[1]);
                break;

            case K_ROOMTELL:
                cfg.roomtell = (uchar)atoi(words[1]);
                break;

            case K_ROOMPATH:
                if (strlen(words[1]) > 63)
                    illegal("roompath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy(cfg.roompath, words[1]);  
                break;
            
            case K_FLOORS:
                cfg.floors = (uchar)atoi(words[1]);
                break;

            case K_SUBHUBS:
                cfg.subhubs = (char)atoi(words[1]);
                break;

            case K_TABS:
                cfg.tabs = (uchar)atoi(words[1]);
                break;
            
            case K_TIMEOUT:
                cfg.timeout = (char)atoi(words[1]);
                break;

            case K_TRAP:
                for (l = 0; l < 17; ++l) cfg.trapit[l] = FALSE;

                for (j = 1; j < count; j++)
                {
                    valid = FALSE;

                    for (k = 0; trapkeywords[k] != NULL; k++)
                    {
                        sprintf(notkeyword, "!%s", trapkeywords[k]);

                        if (strcmpi(words[j], trapkeywords[k]) == SAMESTRING)
                        {
                            valid = TRUE;

                            if ( k == 0)  /* ALL */
                            {
                                for (l = 0; l < 17; ++l) cfg.trapit[l] = TRUE;
                            }
                            else cfg.trapit[k] = TRUE;
                        }
                        else if (strcmpi(words[j], notkeyword) == SAMESTRING)
                        {
                            valid = TRUE;

                            if ( k == 0)  /* ALL */
                            {
                                for (l = 0; l < 17; ++l) cfg.trapit[l] = FALSE;
                            }
                            else cfg.trapit[k] = FALSE; 
                        }
                    }

                    if ( !valid )
                    {
                        doccr();
                        cPrintf("Config.Cit - Warning:"
                                " Unknown #TRAP parameter %s ", words[j]);
                        doccr();
                    }
                }
                break;

            case K_TRAP_FILE:

                if (ignore == 2) break;

                if (strlen(words[1]) > 63)
                    illegal("Trap filename too long; must be less than 64");
  
                strcpy(cfg.trapfile, words[1]);  

                break;

            case K_UNLOGGEDBALANCE:
                sscanf(words[1], "%f ", &cfg.unlogbal);  /* float */
                break;

            case K_UNLOGTIMEOUT:
                cfg.unlogtimeout = (char)atoi(words[1]);
                break;

            case K_UPPERCASE:
                cfg.uppercase = (uchar)atoi(words[1]);
                break;

            case K_USER:
                for ( j = 0; j < 8; ++j)  cfg.user[j] = 0;

                for (j = 1; j < count; j++)
                {
                    valid = FALSE;

                    for (k = 0; userkeywords[k] != NULL; k++)
                    {
                        if (strcmpi(words[j], userkeywords[k]) == SAMESTRING)
                        {
                           valid = TRUE;

                           cfg.user[k] = TRUE;
                        }
                    }

                    if (!valid)
                    {
                        doccr();
                   cPrintf("Config.Cit - Warning: Unknown #USER parameter %s ",
                        words[j]);
                        doccr();
                    }
                }
                break;

            case K_WIDTH:
                cfg.width = (uchar)atoi(words[1]);
                break;

            case K_TWIT_FEATURES:
                cfg.msgNym     = FALSE;
                cfg.borders    = FALSE;
                cfg.titles     = FALSE;
                cfg.nettitles  = FALSE;
                cfg.surnames   = FALSE;
                cfg.netsurname = FALSE;
                cfg.entersur   = FALSE;
                cfg.colors     = FALSE;

                for (j = 1; j < count; j++)
                {
                    valid = FALSE;

                    for (k = 0; twitfeatures[k] != NULL; k++)
                    {
                        if (strcmpi(words[j], twitfeatures[k]) == SAMESTRING)
                        {
                            valid = TRUE;

                            switch (k)
                            {
                            case 0:     /* MESSAGE NYMS */
                                cfg.msgNym = TRUE;
                                break;

                            case 1:     /* BOARDERS */
                                cfg.borders = TRUE;
                                break;
                            
                            case 2:     /* TITLES */
                                cfg.titles = TRUE;
                                break;
                            
                            case 3:     /* NET_TITLES */
                                cfg.nettitles = TRUE;
                                break;
                            
                            case 4:     /* SURNAMES */
                                cfg.surnames = TRUE;
                                break;
                            
                            case 5:     /* NET_SURNAMES */
                                cfg.netsurname = TRUE;
                                break;
                            
                            case 6:     /* ENTER_TITLES */
                                cfg.entersur = TRUE;
                                break;

                            case 7:     /* COLORS */
                                cfg.colors = TRUE;
                                break;

                            default:
                                break;
                            }
                        }
                    }

                    if ( !valid )
                    {
                        doccr();
                        cPrintf("Config.Cit - Warning:"
                                " Unknown #TWIT_FEATURES parameter %s ",
                                words[j]);
                        doccr();
                    }
                }
                break;
            
            case K_LOGIN:
                cfg.l_closedsys   = FALSE;
                cfg.l_verified    = TRUE;
                cfg.l_questionare = FALSE;
                cfg.l_application = FALSE;
                cfg.l_sysop_msg   = FALSE;
                cfg.l_create      = TRUE;
                
                for (j = 1; j < count; j++)
                {
                    for (k = 0, valid = FALSE; newuserkeywords[k] != NULL; k++)
                    {
                        if (strcmpi(words[j], newuserkeywords[k]) == SAMESTRING)
                        {
                            valid = TRUE;
                            l = TRUE;
                        }
                        else
                        if (words[j][0] == '!' &&
                            strcmpi((words[j])+1, newuserkeywords[k]) == SAMESTRING)
                        {
                            valid = TRUE;
                            l = FALSE;
                        }
                            
                        if (valid)    
                        {
                            switch (k)
                            {
                            case L_CLOSED:
                                cfg.l_closedsys = (uchar)l;
                                break;
                            
                            case L_VERIFIED:
                                cfg.l_verified = (uchar)l;
                                break;
                            
                            case L_QUESTIONS:
                                cfg.l_questionare = (uchar)l;
                                break;
                            
                            case L_APPLICATION:
                                cfg.l_application = (uchar)l;
                                break;
                            
                            case L_SYSOP_MESSAGE:
                                cfg.l_sysop_msg = (uchar)l;
                                break;
                            
                            case L_NEW_ACCOUNTS:
                                cfg.l_create = (uchar)l;
                                break;
                                
                            default:
                                doccr();
                                cPrintf("Config.Cit - Warning:"
                                        " Unknown #LOGIN parameter %s ",
                                        words[j]);
                                doccr();
                                break;
                            }
                            break;
                        }
                    }

                    if ( !valid )
                    {
                        doccr();
                        cPrintf("Config.Cit - Warning:"
                                " Unknown #LOGIN parameter %s ",
                                words[j]);
                        doccr();
                    }
                }
                break;

/* New stuff */

            case K_DIRPATH:
                if (strlen(words[1]) > 63)
                    illegal("dirpath too long; must be less than 64");

                if (changedir(words[1]) == ERROR)
                    illegal(nochange, words[0], words[1]);

                strcpy(cfg.dirpath, words[1]);
                break;


            case K_DIAL_RING:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("dial_ring too long; must be less than 30");

                strcpy(cfg.dialring, words[1]);  
                break;

            case K_UP_DAYS:              
                /* init days */
                for ( j = 0; j < 7; j++ )
                    cfg.updays[j] = 0;

                for (j = 1; j < count; j++)
                {
                    for (k = 0; daykeywords[k] != NULL; k++)
                    {
                        if (strcmpi(words[j], daykeywords[k]) == SAMESTRING)
                        {
                            break;
                        }
                    }
                    if (k < 7)
                        cfg.updays[k] = TRUE;
                    else if (k == 7)  /* any */
                    {
                        for ( l = 0; l < 7; ++l)
                            cfg.updays[l] = TRUE;
                    }
                    else
                    {
                        doccr();
                            cPrintf("Invalid CONFIG.CIT %s: %s",
                                    words[0], words[j]);
                        doccr();
                    }
                }
                break;

            case K_UP_HOURS:
                /* init hours */
                for ( j = 0; j < 24; j++ )
                    cfg.uphours[j] = FALSE;

                for (j = 1; j < count; j++)
                {
                    if (strcmpi(words[j], "Any") == SAMESTRING)
                    {
                        for (l = 0; l < 24; l++)
                            cfg.uphours[l] = TRUE;
                    }
                    else
                    {
                        l = atoi(words[j]);

                        if ( l > 23 ) 
                        {
                            doccr();
                            cPrintf("Invalid CONFIG.CIT %s: %s",
                                    words[0], words[j]);
                            doccr();
                        }
                        else
                            cfg.uphours[l] = TRUE;
                    }
                }
                break;

            case K_ALIAS:
                if (strlen(words[1]) > 3)
                    illegal("alias too long; must be less than 4");

                strcpy(cfg.alias, words[1]);
                break;


            case K_MESSAGE_NYM:
                if (strlen(words[1]) > NAMESIZE ||
                    strlen(words[2]) > NAMESIZE ||
                    strlen(words[3]) > NAMESIZE )
               illegal("message_nym too long; must be less than 31");

                strcpy(cfg.msg_nym,  words[1]);
                strcpy(cfg.msgs_nym, words[2]);
                strcpy(cfg.msg_done, words[3]);
                break;

            case K_BORDER:
                if (bordercount >= MAXBORDERS) 
                    illegal("Too many borders");

                if (strlen(words[1]) > 80)
                    illegal("border too long; must be less than 81");

                strcpy(cfg.border[bordercount],  words[1]);
                bordercount++;
                break;

            case K_TWITREGION:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("twitregion too long; must be less than 30");

                strcpy(cfg.twitRegion, words[1]);
                break;

            case K_TWITCOUNTRY:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("twitcountry too long; must be less than 30");

                strcpy(cfg.twitCountry, words[1]);
                break;

            case K_ANONAUTHOR:
                if (strlen(words[1]) > NAMESIZE)
                    illegal("anonauthor too long; must be less than 30");

                strcpy(cfg.anonauthor, words[1]);
                break;

/* End of New stuff */


            default:
                cPrintf("Config.Cit - Warning: Unknown variable %s", words[0]);
                doccr();
                break;
        }
    }
    fclose(fBuf);

    for (i = 0, valid = TRUE; i <= K_NWORDS; i++)
    {
        if (!found[i])
        {
            cPrintf("CONFIG.CIT : ERROR: can not find %s keyword!\n",
                keywords[i]);
            valid = FALSE;
        }
    }

    if (!valid)
        illegal("");

    if (strcmpi(cfg.homepath, cfg.temppath) == SAMESTRING)    
    {
        illegal("#HOMEPATH may not equal #TEMPPATH");
    }

    cyclesignature();  /* back to first signature for multi sigs */
}

/************************************************************************/
/*      RoomTabBld() -- build RAM index to ROOM.DAT, displays stats.    */
/************************************************************************/
void RoomTabBld(void)
{
    int  slot = 0;
    int  roomCount = 0;

    doccr(); doccr();
    cPrintf("Building room table"); doccr();

#ifdef NEWMSGTAB
    cPrintf("Room No: %d\r", slot);
#endif

    for (slot = 0;  slot < MAXROOMS;  slot++)
    {
        getRoom(slot);

#ifdef NEWMSGTAB
        if (!(slot % 10))
#endif
            cPrintf("Room No: %d\r", slot);

        if (roomBuf.rbflags.INUSE)  ++roomCount;
   
        noteRoom();
        putRoom(slot);
    }

#ifdef NEWMSGTAB
    cPrintf("Room No: %d\r", slot);
#endif

    doccr();
    cPrintf(" %d of %d rooms in use", roomCount, MAXROOMS); doccr();

}

/************************************************************************/
/*      zapGrpFile(), erase & reinitialize group file                   */
/************************************************************************/
void zapGrpFile(void)
{
    doccr();
    cPrintf("Writing group table."); doccr();

    memset(&grpBuf, 0, sizeof grpBuf);

    strcpy( grpBuf.group[0].groupname, "Null");
    grpBuf.group[0].g_inuse  = 1;
    grpBuf.group[0].groupgen = 1;      /* Group Null's gen# is one      */
                                       /* everyone's a member at log-in */

    strcpy( grpBuf.group[1].groupname, "Reserved_2");
    grpBuf.group[1].g_inuse   = 1;
    grpBuf.group[1].groupgen  = 1;

    putGroup();
}

/************************************************************************/
/*      zapHallFile(), erase & reinitialize hall file                   */
/************************************************************************/
void zapHallFile(void)
{
    int i;
    
    doccr();
    cPrintf("Writing hall table.");  doccr();

    strcpy( hallBuf->hall[0].hallname, "Root");
    hallBuf->hall[0].owned = 0;                 /* Hall is not owned     */

    hallBuf->hall[0].h_inuse = 1;

#ifdef GOODBYE
    hallBuf->hall[0].hroomflags[0].inhall = 1;  /* Lobby> in Root        */
    hallBuf->hall[0].hroomflags[1].inhall = 1;  /* Mail>  in Root        */
    hallBuf->hall[0].hroomflags[2].inhall = 1;  /* Aide)  in Root        */
#endif

    bit_set(hallBuf->hall[0].inhall, 0);  /* Lobby> in Root */
    bit_set(hallBuf->hall[0].inhall, 1);  /* Mail>  in Root */
    bit_set(hallBuf->hall[0].inhall, 2);  /* Aide)  in Root */

    strcpy( hallBuf->hall[1].hallname, "Maintenance");
    hallBuf->hall[1].owned = 0;                 /* Hall is not owned     */

    hallBuf->hall[1].h_inuse = 1;

#ifdef GOODBYE
    hallBuf->hall[1].hroomflags[0].inhall = 1;  /* Lobby> in Maintenance */
    hallBuf->hall[1].hroomflags[1].inhall = 1;  /* Mail>  in Maintenance */
    hallBuf->hall[1].hroomflags[2].inhall = 1;  /* Aide)  in Maintenance */
#endif

    bit_set(hallBuf->hall[1].inhall, 0);  /* Lobby> in Maintenance */
    bit_set(hallBuf->hall[1].inhall, 1);  /* Mail>  in Maintenance */
    bit_set(hallBuf->hall[1].inhall, 2);  /* Aide)  in Maintenance */

#ifdef GOODBYE
    hallBuf->hall[0].hroomflags[2].window = 1;  /* Aide) is the window   */    
    hallBuf->hall[1].hroomflags[2].window = 1;  /* Aide) is the window   */
#endif

    bit_set(hallBuf->hall[0].window, 2);  /* Aide) is the window   */
    bit_set(hallBuf->hall[1].window, 2);  /* Aide) is the window   */
    

    /*
     * Init the room position table..
     */
    for (i=0; i<MAXROOMS; i++)
        roomPos[i] = i;
    
    putHall();
}

/************************************************************************/
/*      zapLogFile() erases & re-initializes userlog.buf                */
/************************************************************************/
zapLogFile()
{
    int  i;

    /* clear RAM buffer out:                    */
    memset(&logBuf, 0, sizeof(logBuf));
 
    doccr();  
    doccr();
    cPrintf("MAXLOGTAB=%d",cfg.MAXLOGTAB);  doccr();

    /* write empty buffer all over file;        */
    for (i = 0; i < cfg.MAXLOGTAB;  i++)
    {
        cPrintf("Clearing log entry %d\r", i);
        /* logTab[i].ltlogSlot = i; */
        putLog(&logBuf, i);
    }
    doccr();
    return TRUE;
}

/************************************************************************/
/*      zapRoomFile() erases and re-initailizes ROOM.DAT                */
/************************************************************************/
zapRoomFile()
{
    int i;

    memset(&roomBuf, 0, sizeof(roomBuf));
    
    doccr();  doccr();
    cPrintf("MAXROOMS=%d", MAXROOMS); doccr();

    for (i = 0;  i < MAXROOMS;  i++)
    {
        cPrintf("Clearing room %d\r", i);
        putRoom(i);
        noteRoom();
    }

    /* Lobby> always exists -- guarantees us a place to stand! */
    thisRoom            = 0          ;
    strcpy(roomBuf.rbname, "Lobby")  ;
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC   = TRUE;
    roomBuf.rbflags.INUSE    = TRUE;

    putRoom(LOBBY);
    noteRoom();

    /* Mail> is also permanent...       */
    thisRoom            = MAILROOM      ;
    strcpy(roomBuf.rbname, "Mail");
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC   = TRUE;
    roomBuf.rbflags.INUSE    = TRUE;

    putRoom(MAILROOM);
    noteRoom();

    /* Aide) also...                    */
    thisRoom            = AIDEROOM;
    strcpy(roomBuf.rbname, "Aide");
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC   = FALSE;
    roomBuf.rbflags.INUSE    = TRUE;

    putRoom(AIDEROOM);
    noteRoom();

    /* Dump> also...                    */
    thisRoom            = DUMP;
    strcpy(roomBuf.rbname, "Dump");
    roomBuf.rbflags.PERMROOM = TRUE;
    roomBuf.rbflags.PUBLIC   = TRUE;
    roomBuf.rbflags.INUSE    = TRUE;

    putRoom(DUMP);
    noteRoom();

    return TRUE;
}

/************************************************************************/
/**************************  Resize Stuff *******************************/
/************************************************************************/

/************************************************************************/
/*      resizelogfile() -- resizes log file                             */
/************************************************************************/
void resizelogfile(void)
{
    int i;
    int result;
    int dummy;
    char *logFile2;

    FILE *oldlog;
    FILE *newlog;

    chdir(cfg.homepath);

    fclose(logfl);

    logFile2 = "log.tmp";
    
    if ((logfl = fopen(logFile2, "w+b")) == NULL)
        illegal("Can't create temp log file!");

    dummy = cfg.MAXLOGTAB;
    cfg.MAXLOGTAB = newmaxlogtab;
    zapLogFile();
    cfg.MAXLOGTAB = dummy;
    doccr();

    fclose(logfl);

    /* open first log file */
    if ((oldlog = fopen(logFile, "r+b")) == NULL)
    {
        cPrintf("Can't open %s\n", logFile);
        return;
    }

    /* open temp log file */
    if ((newlog = fopen(logFile2, "r+b")) == NULL)
    {
        cPrintf("Can't open %s\n", logFile2);
        fclose(oldlog);
        return;
    }

    for (i = 0;  ((i < cfg.MAXLOGTAB) && (i < newmaxlogtab)); i++)
    {
        cPrintf("Copying log entry %d  \r", logTab[i].ltlogSlot);

        logfl = oldlog;

        /* get source log entry  */
        getLog(&logBuf, logTab[i].ltlogSlot);

        logfl = newlog;

        /* write destination log entry */ 
        putLog(&logBuf, i);
    }
    doccr();

    fclose(oldlog);
    fclose(newlog);

    /* clear RAM buffer out: */
    memset(&logBuf, 0, sizeof(logBuf));

    result = unlink(logFile);
    if (result == -1)
    {
        cPrintf("Cannot delete log.dat");
        doccr();
    }

    result = rename(logFile2, logFile);
    if (result == -1)
    {
        cPrintf("Cannot rename log.tmp");
        doccr();
    }
}

/************************************************************************/
/*      resizemsgfile() -- resizes message file                         */
/************************************************************************/
void resizemsgfile(void)
{
    int i;
    int result;
    char msgFile2[64];
    /* int dummy; */
    long loc;
    ulong tablesize;
    ulong here;
    ulong oldest;

    FILE *oldmsg;
    FILE *newmsg;

    oldest    = cfg.mtoldest;
    tablesize = sizetable();

    sprintf(msgFile2, "%s\\%s", cfg.msgpath, "msg.tmp");

    fclose(msgfl);
    
    if ((msgfl = fopen(msgFile2, "w+b")) == NULL)
        illegal("Can't create the message file!");

    /* dummy = cfg.messagek;  */
    cfg.messagek = newmessagek;
    zapMsgFile();
    cfg.catLoc = 0l;

/*  cfg.messagek = dummy; */  /*  taken out */
    doccr();
    doccr();

    fclose(msgfl);

    /* open first message file */
    if ((oldmsg = fopen(msgFile, "r+b")) == NULL)
    {
        cPrintf("Can't open %s\n", msgFile);
        return;
    }
    /* open temp message file */
    if ((newmsg = fopen(msgFile2, "r+b")) == NULL)
    {
        cPrintf("Can't open %s\n", msgFile2);
        fclose(oldmsg);
        return;
    }

    for (i = 0; i < tablesize; i++)
    {
        /* loc = msgTab2[i].mtmsgLoc; */

#ifdef NEWMSGTAB
        loc = getLocation(i);
#else

        loc = long_JOIN(msgTab_mtmsgLocLO[i], msgTab_mtmsgLocHI[i]); 
#endif

        msgfl = oldmsg;

        fseek(msgfl, loc, 0);

        getMessage();
        getMsgStr(msgBuf->mbtext, MAXTEXT); 

        sscanf(msgBuf->mbId, "%ld", &here);

#ifdef GOODBYE
        /* Don't bother with the null message */
        if (here == 1L)
            continue;
#endif

        if (here != (oldest + (ulong)i))
            continue;

        cPrintf("Copying Message #%lu\r", here);

        msgfl = newmsg;

        resize_putMessage();
    }
    doccr();

    fclose(oldmsg);
    fclose(newmsg);

    result = unlink(msgFile);
    if (result == -1)
    {
        cPrintf("Cannot delete msg.dat");
        doccr();
    }

    result = rename(msgFile2, msgFile);
    if (result == -1)
    {
        cPrintf("Cannot rename msg.tmp");
        doccr();
    }
}

/************************************************************************/
/*      checkresize() -- checks to see if message file and/or log       */
/*      need to be resized.                                             */
/************************************************************************/
void checkresize(void)
{
    int fh;
    long result;

    /* save old values for later */
    newmessagek  = cfg.messagek;
    newmaxlogtab = cfg.MAXLOGTAB;

    fh = open(msgFile, O_RDONLY);

    if (fh != -1)
    {
        result = filelength(fh);

        if (result != ( (long)cfg.messagek * 1024l))
        {
            resizeMsg = TRUE;
            cPrintf("Must resize msg.dat");
            doccr();

            /* set messagek to actual value */
            cfg.messagek = (long)(result / 1024l);
        }

        close(fh);
    }

    fh = open(logFile, O_RDONLY);

    if (fh != -1)
    {
        result = filelength(fh);

        if (result != ( (long)cfg.MAXLOGTAB *  (long)sizeof logBuf))
        {
            resizeLog = TRUE;
            cPrintf("Must resize log.dat");
            doccr();

            /* set MAXLOGTAB to actual value */
            cfg.MAXLOGTAB = (int)(result / (long)sizeof logBuf);
        }
        close(fh);
    }
    if (resizeMsg || resizeLog)  pause(200);
}

/* -------------------------------------------------------------------- */
/*  resize_putMessage()    stores a message to disk                     */
/* -------------------------------------------------------------------- */
BOOL resize_putMessage(void)
{
    struct unkLst *lul;  /* Brent, I dunno */ 

#ifdef NEWMSGTAB
    uchar roomno_lo;
    uchar roomno_hi;
#endif

    /* record start of message to be noted */
    msgBuf->mbheadLoc = (long)cfg.catLoc;

    /* tell putMsgChar where to write   */
    fseek(msgfl, cfg.catLoc, 0);
 
    /* start-of-message              */
    overwrite(1);
    putMsgChar((char)0xFF);









    /* write room #                  */

#ifdef NEWMSGTAB
    if (msgBuf->mbroomno > 254)
    {

#ifdef GOODBYE
        if (debug)
        {
            doccr();
            cPrintf("Writing new style");
            doccr();
        }
#endif

        roomno_lo = int_LO(msgBuf->mbroomno * 2);
        roomno_hi = int_HI(msgBuf->mbroomno * 2);

        overwrite(1);
        putMsgChar(roomno_lo);

        overwrite(1);
        putMsgChar(roomno_hi);
    }
    else
    {

#ifdef GOODBYE
        if (debug)
        {
            doccr();
            cPrintf("Writing old style");
            doccr();
        }
#endif

        overwrite(1);
        putMsgChar(msgBuf->mbroomno);
    }
#else
    overwrite(1);
    putMsgChar(msgBuf->mbroomno);
#endif

    /* write attribute byte  */
    overwrite(1);
    putMsgChar(msgBuf->mbattr);  

    /* write message ID */
    dPrintf("%s", msgBuf->mbId);         

    if (msgBuf->mbauth[0])   { dPrintf("A%s", msgBuf->mbauth);      }
    if (msgBuf->mbsub[0])    { dPrintf("B%s", msgBuf->mbsub);       }
    if (msgBuf->mbcopy[0])   { dPrintf("C%s", msgBuf->mbcopy);      }
    if (msgBuf->mbtime[0])   { dPrintf("D%s", msgBuf->mbtime);      }
    if (msgBuf->mbfwd[0])    { dPrintf("F%s", msgBuf->mbfwd);       }
    if (msgBuf->mbgroup[0])  { dPrintf("G%s", msgBuf->mbgroup);     }
    if (msgBuf->mbreply[0])  { dPrintf("I%s", msgBuf->mbreply);     }
    if (msgBuf->mbcreg[0])   { dPrintf("J%s", msgBuf->mbcreg);      }
    if (msgBuf->mbccont[0])  { dPrintf("j%s", msgBuf->mbccont);     }
    if (msgBuf->mblink[0])   { dPrintf("L%s", msgBuf->mblink);      }
    if (msgBuf->mbtitle[0])  { dPrintf("N%s", msgBuf->mbtitle);     }
    if (msgBuf->mbsur[0])    { dPrintf("n%s", msgBuf->mbsur);       }
    if (msgBuf->mboname[0])  { dPrintf("O%s", msgBuf->mboname);     }
    if (msgBuf->mboreg[0])   { dPrintf("o%s", msgBuf->mboreg);      }
    if (msgBuf->mbfpath[0])  { dPrintf("P%s", msgBuf->mbfpath);     }
    if (msgBuf->mbtpath[0])  { dPrintf("p%s", msgBuf->mbtpath);     }
    if (msgBuf->mbocont[0])  { dPrintf("Q%s", msgBuf->mbocont);     }
    if (msgBuf->mbczip[0])   { dPrintf("q%s", msgBuf->mbczip);      }
    if (msgBuf->mbroom[0])   { dPrintf("R%s", msgBuf->mbroom);      }
    if (msgBuf->mbsrcId[0])  { dPrintf("S%s", msgBuf->mbsrcId);     }
    if (msgBuf->mbsoft[0])   { dPrintf("s%s", msgBuf->mbsoft);      }
    if (msgBuf->mbto[0])     { dPrintf("T%s", msgBuf->mbto);        }
    if (msgBuf->mbx[0])      { dPrintf("X%s", msgBuf->mbx);         }
    if (msgBuf->mbzip[0])    { dPrintf("Z%s", msgBuf->mbzip);       }
    if (msgBuf->mbrzip[0])   { dPrintf("z%s", msgBuf->mbrzip);      }
    if (msgBuf->mbsig[0])    { dPrintf(".%s", msgBuf->mbsig);       }
    if (msgBuf->mbusig[0])   { dPrintf("_%s", msgBuf->mbusig);      }
    /* nodephone */
    if (msgBuf->mbophone[0]) { dPrintf("H%s", msgBuf->mbophone);    }
    if (msgBuf->mbzphone[0]) { dPrintf("h%s", msgBuf->mbzphone);    }


    /* put any unknowns... */ 
    for(lul = firstUnk; lul != NULL; lul = lul->next) 
        { 
        dPrintf("%c%s", lul->whatField, lul->theValue); 
        } 


    /* M-for-message. */
    overwrite(1);
    putMsgChar('M'); putMsgStr(msgBuf->mbtext);

    /* now finish writing */
    fflush(msgfl);

    /* record where to begin writing next message */
    cfg.catLoc = ftell(msgfl);

    return  TRUE;
}


