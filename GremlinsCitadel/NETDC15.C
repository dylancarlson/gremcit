/* -------------------------------------------------------------------- */
/*  NETDC15.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*      Networking libs for the Citadel bulletin board system           */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  dc15network()   During network master code                          */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
static void sendRequest(void);
static void reciveRequest(void);
static void makeSendFile(void);
static void sendFiles(void);
static void reciveFiles(void);
static netFailed = FALSE;

/* IdExists() Returns room slot of specified net id or ERROR if not there */
int  IdExists(char *id)
{
    int strhash;
    int i;
    struct aRoom tempRoomBuf = roomBuf;
    int oldThisRoom = thisRoom;

    strhash = hash(id);

    for (i = 0; i < MAXROOMS; i++)
    {       
        if (roomTab[i].rtflags.INUSE  && (roomTab[i].netIDhash == strhash))
        {
            getRoom(i);
            if (strcmpi(id, roomBuf.netID) == SAMESTRING)
            {
                roomBuf = tempRoomBuf;
                thisRoom = oldThisRoom;
                return(i);
            }
        }
    }
    roomBuf = tempRoomBuf;
    thisRoom = oldThisRoom;
    return(ERROR);
}

/* IdtoName() Converts network ID to room name */
void IdtoName(char *string)
{
    int slot;

    slot = IdExists(string);

    if ((slot != ERROR) /* && netcanseeroom(slot) */) 
    {
        strcpy(string,  roomTab[slot].rtname);
    }
    else strcpy(string, "");
}

/* NametoId() Converts Room name to network ID */
void NametoId(char *string)
{
    int slot;
    struct aRoom tempRoomBuf = roomBuf;
    int oldThisRoom = thisRoom;

    slot = roomExists(string);

    if ((slot != ERROR) /* && netcanseeroom(slot) */) 
    {
        getRoom(slot);
        strcpy(string,  roomBuf.netID);
        roomBuf = tempRoomBuf;
        thisRoom = oldThisRoom;
    }
    else strcpy(string, "");
}

/* getId() Gets network id's from room file */
BOOL getId(char first, char *string)
{
    static int counter;
    char foundit = 0;
    struct aRoom tempRoomBuf = roomBuf;
    int oldThisRoom = thisRoom;

    if (first) counter = 0;

    for (; (counter < MAXROOMS) && !foundit; ++counter)
    {
        if (netcanseeroom(counter) && roomTab[counter].netIDhash)
        {
            getRoom(counter);
            strcpy(string, roomBuf.netID);
            foundit = TRUE;
            roomBuf = tempRoomBuf;
            thisRoom = oldThisRoom;
        }
    }
    if (foundit) return(TRUE);
                 return(FALSE);
}

/* -------------------------------------------------------------------- */
/*  dc15master()    During network master code                          */
/* -------------------------------------------------------------------- */
BOOL dc15network(BOOL master)
{
    char line[100], line2[100];
    label here, there;
    FILE *file;
    int i, rms;
    time_t t, t2=0;
    BOOL    done = FALSE;
    
    netFailed = FALSE;
    
    if (!gotCarrier()) return FALSE;

    sprintf(line, "%s\\mesg.tmp", cfg.temppath);
    unlink(line);
    
    sprintf(line, "%s\\mailin.tmp", cfg.temppath);
    unlink(line);

    sprintf(line, "%s\\roomreq.in", cfg.temppath);
    unlink(line);
    
    sprintf(line, "%s\\roomreq.out", cfg.temppath);
    unlink(line);

    if((file = fopen(line, "ab")) == NULL)
    {
        perror("Error opening roomreq.out");
        return FALSE;
    }

    for (i=(node.network == NET_1_69) ? getId(1, there) : get_first_room(here, there), rms=0;
         i;
         i=(node.network == NET_1_69) ? getId(0, there) : get_next_room(here, there), rms++)
    {
        PutStr(file, there);
    }
    
    PutStr(file, "");
    fclose(file);

    if (master)
    {
        sendRequest();
        if (!gotCarrier()) return FALSE;
        reciveRequest();
    }
    else
    {
        reciveRequest();
        if (!gotCarrier()) return FALSE;
        sendRequest();
    }
    
    if (!gotCarrier() || netFailed) return FALSE;
    
    if (master)
    {
        /* clear the buffer */
        while (gotCarrier() && MIReady())
        {
            getMod();
        }
    }
    
    makeSendFile();
    
    if (!gotCarrier() || netFailed) return FALSE;
        
    /*
     * wait for them to get their shit together 
     */
    cPrintf(" Waiting for transfer.");
    
    outMod('X');
    t2 = 0;
    t = time(NULL); 
    while (gotCarrier() && !done)
    {
        if (time(NULL) > (t + (35 * 60))) /* only wait 35 minutes */
        {
            drop_dtr();
            netFailed = TRUE;
        }
        
        if (MIReady())
        {
            i = getMod();
            if (i == 'X' || ((node.network != NET_DCIT16) && (node.network != 
  NET_1_69)))
            {
                done = TRUE;
            }
            else
            {
                if (debug)
                {
                    cPrintf("<%c>", i);
                }
            }
                
        }
        
        /* wake them up! (every second) */
        if (time(NULL) != t2)
        {
            outMod('X');
            t2 = time(NULL);
        }
    }
    
    /* wake them up! */
    for (i=0; i<10; i++)
        outMod('X');
    
    doccr();

    if (!gotCarrier() || netFailed) return FALSE;

    if (master)
    {
        reciveFiles();
        if (!gotCarrier() || netFailed) return FALSE;
        sendFiles();
    }
    else
    {
        sendFiles();
        if (!gotCarrier() || netFailed) return FALSE;
        reciveFiles();
    }
    
    if (netFailed) return FALSE;
    
    cPrintf(" Hangup.");
    doccr();
    
    drop_dtr();

    cPrintf(" Uncompressing message files.");
    doccr();
         
    sformat(line, node.unzip, "d", roomdatain);
    apsystem(line);
         
    unlink(roomdatain);
    
    for (i=0; i<rms; i++)
    {
        sprintf(line,  "room.%d",   i);
        sprintf(line2, "roomin.%d", i);
        rename(line, line2);
    }
        
    sprintf(line,  "%s\\mesg.tmp",   cfg.temppath);
    sprintf(line2, "%s\\mailin.tmp", cfg.temppath);
    rename(line, line2);

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  sendRequest()   Send the room request file                          */
/* -------------------------------------------------------------------- */
static void sendRequest(void)
{
    cPrintf(" Sending room request file.");
    doccr();

    wxsnd(cfg.temppath, roomreqout, 
         (char)strpos((char)tolower(node.ndprotocol[0]), extrncmd));
    
    if (node.network != NET_1_69)
        unlink(roomreqout); 
}

/* -------------------------------------------------------------------- */
/*  reciveRequest() Recive the room request file                        */
/* -------------------------------------------------------------------- */
static void reciveRequest(void)
{
    cPrintf(" Receiving room request file.");
    doccr();

    wxrcv(cfg.temppath, roomreqin, 
            (char)strpos((char)tolower(node.ndprotocol[0]), extrncmd));
            
    if (!filexists(roomreqin))
    {
        drop_dtr();
        netFailed = TRUE;
    }
}


/* checks NETID.CIT for existance of a net id */
BOOL checkidlist(char *id)
{
    FILE *fBuf;
    char line[256];
    char *words[256];
    int  found = FALSE;
    char path[80];

    sprintf(path, "%s\\netid.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {
        return(FALSE);
    }

    while ((fgets(line, 254, fBuf) != NULL) && !found)
    {
        if (strnicmp(line, "#NETID", 6) != SAMESTRING)
        {
            continue;
        }
        
        parse_it( words, line);

        if (strcmpi(id,  words[1]) == SAMESTRING)
        {
            found = TRUE;
        }
    }
    fclose(fBuf);

    return((BOOL)found);
}

/* autoroom, auto-creates a net room */
BOOL autoroom(char *roomname)
{
    int   testslot, newSlot;

#ifdef NEWMSGTAB
    ulong i;
#else
    int  i;
#endif
    char string[40];
    FILE *file;
    char path[80];
    int j;


    newSlot = findRoom();

    if ( (newSlot) == ERROR )
    {
        return(FALSE);
    }

    if (!strlen(roomname))
    {
        return(FALSE);
    }

    testslot = roomExists(roomname);

    if (testslot != ERROR)
    {
        return(FALSE);
    }

    testslot = IdExists(roomname);

    if (testslot != ERROR)
    {
        return(FALSE);
    }

    memset(&roomBuf, 0, sizeof(roomBuf));

    roomBuf.rbflags.INUSE     = TRUE;
    roomBuf.rbflags.PUBLIC    = TRUE;
    roomBuf.rbflags.SHARED    = TRUE;

    thisRoom = newSlot;

    strcpy(roomBuf.netID, roomname);
    strcpy(roomBuf.rbname, roomname);

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

    /* put room in maintenance hall */
#ifdef GOODBYE
    hallBuf->hall[1].hroomflags[thisRoom].inhall = TRUE;
#endif
    bit_set(hallBuf->hall[1].inhall, thisRoom);


    putHall();  /* save it */

    getRoom(LOBBY);

    /* add net id to netid.cit */
    sprintf(path, "%s\\netid.cit", cfg.homepath);

    if((file = fopen(path, "a")) == NULL)
    {
        perror("Error opening netid.cit");
    }
    else
    {
        sprintf(string, "\n#NETID    \"%s\"", roomname);
        PutStr(file, string);
        fclose(file);
    }

    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  makeSendFile()  Make the file to send to remote..                   */
/* -------------------------------------------------------------------- */
static void makeSendFile(void)
{
    char line[100], line2[100];
    label troo, id;
    label fn;
    FILE *file;
    int i = 0, rm;
    char maderoom;
    char idinlist;
    int crcounter = 0;
    
    if ((file = fopen(roomreqin, "rb")) == NULL)
    {
        perror("Error opening roomreq.in");
        return;   /* ok */
    }

    doccr();
    cPrintf(" Fetching:");
    doccr();

    GetStr(file, troo, LABELSIZE);
    
    while(strlen(troo) && !feof(file))
    {
        /* convert Net Id's into actual room names */
        if (node.network == NET_1_69)
        {
            strcpy(id, troo);
            id[19] = '\0'; /* cut it down to size */
            IdtoName(troo);
        }

        if ((rm = roomExists(troo)) != ERROR)
        {
            if (netcanseeroom(rm))
            {
                sprintf(fn, "room.%d", i);
                cPrintf(" %-20s  ", troo);
                if( !((crcounter+1) % 3) ) doccr();
                crcounter++;
                NewRoom(rm, fn);
            }
            else
            {
                if (node.network == NET_1_69)
                {
                    if (node.verbose)  /* Is 1 or 2 */
                    {
                        doccr();
                        cPrintf(" No access to %s room.", troo);
                        doccr();
                        amPrintf(" '%s' room not available to remote.\n", troo);
                        netError = TRUE;
                    }
                }
                else
                {
                    doccr();
                    cPrintf(" No access to %s room.", troo);
                    doccr();
                    amPrintf(" '%s' room not available to remote.\n", troo);
                    netError = TRUE;
                }
            }
        }
        else
        {
            if (node.network == NET_1_69)
            {
                if (node.autoroom)
                {
                    idinlist = checkidlist(id);
                    maderoom = FALSE;
                    if (!idinlist)
                    {
                        maderoom = autoroom(id);
                    }
                } 

                if (node.verbose) /* Is 1 or 2 */
                {
                    if (node.autoroom)
                    {
                        if (maderoom)
                        {
                            doccr();
                            cPrintf(" Room %s created.", id);
                            doccr();
                            amPrintf(" '%s' Room created.\n", id);
                            netError = TRUE;
                        } 
                        else
                        {
                            if (idinlist && node.verbose == 2)
                            {
                                doccr();
                                cPrintf(" No %s Net ID on system.", id);
                                doccr();
                                amPrintf(" '%s' Net ID not found for remote.\n", id);
                                netError = TRUE;
                            }
                            else
                            {
                                if (node.verbose == 2)
                                {
                                    doccr();
                                    cPrintf(" Room %s not created.", id);
                                    doccr();
                                    amPrintf(" '%s' Room not created.\n", id);
                                    netError = TRUE;
                                }
                            }
                        } 
                    }
                    else
                    {
                        doccr();
                        cPrintf(" No %s Net ID on system.", id);
                        doccr();
                        amPrintf(" '%s' Net ID not found for remote.\n", id);
                        netError = TRUE;
                    }
                }
            }
            else
            {
                doccr();
                cPrintf(" No %s room on system.", troo);
                doccr();
                amPrintf(" '%s' room not found for remote.\n", troo);
                netError = TRUE;
            }
        }

        i++;
        GetStr(file, troo, LABELSIZE);
    }
    doccr();
    fclose(file);
    unlink(roomreqin);

    cPrintf(" Copying mail file.");
    doccr();
    sprintf(line,  "%s\\%s",         cfg.transpath, node.ndmailtmp);
    sprintf(line2, "%s\\mesg.tmp",   cfg.temppath);
    if ((file = fopen(line2, "wb")) != NULL)
    {
        fclose(file);
    }
    copyfile(line, line2);
    
    cPrintf(" Compressing message files.");
    doccr();
    
    /* 
     * Zip them up
     */
    sformat(line, node.zip, "df", roomdataout, "mesg.tmp room.*");
    apsystem(line);
    
    /* 
     * Remove them.
     */
    ambigUnlink("room.*",   FALSE);
    unlink("mesg.tmp");
}

/* -------------------------------------------------------------------- */
/*  sendFiles()     Send the data files                                 */
/* -------------------------------------------------------------------- */
static void sendFiles(void)
{
    cPrintf(" Sending mail and rooms.");
    doccr();

    wxsnd(cfg.temppath, roomdataout, 
         (char)strpos((char)tolower(node.ndprotocol[0]), extrncmd));
    
    unlink(roomdataout);
}

/* -------------------------------------------------------------------- */
/*  reciveFiles()   Recive the date files                               */
/* -------------------------------------------------------------------- */
static void reciveFiles(void)
{
    cPrintf(" Receiving mail and rooms.");
    doccr();

    wxrcv(cfg.temppath, roomdatain, 
            (char)strpos((char)tolower(node.ndprotocol[0]), extrncmd));

    if (!filexists(roomdatain))
    {
        drop_dtr();
        netFailed = TRUE;
    }
}


