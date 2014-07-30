/* -------------------------------------------------------------------- */
/*                              aplic.c                                 */
/*                    Aplication code for Citadel                       */
/* -------------------------------------------------------------------- */
#include "ctdl.h"
#include "applic.h"
#include "apstruct.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/*                                                                      */
/*      aplreadmess()           read message in from application        */
/*      apsystem()              turns off interupts and makes           */
/*                              a system call                           */
/*      ExeAplic()              gets name of aplication and executes    */
/*      readuserin()            reads userdati.apl from disk            */
/*      shellescape()           handles the sysop '!' shell command     */
/*      tableIn()               allocates RAM and reads log and msg     */
/*                              and tab files into RAM                  */
/*      tableOut()              writes msg and log tab files to disk    */
/*                              and frees RAM                           */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*      ExeAplic() gets the name of an aplication and executes it.      */
/* -------------------------------------------------------------------- */
void ExeAplic(void)
{
    char stuff[100];
    char comm[5];

    /* doCR(); */
    doCR();

    if (!roomBuf.rbflags.APLIC) 
    {
      mPrintf("  -- Room has no application.\n\n");
      changedir(cfg.homepath);
      return;
    }
    if (changedir(cfg.aplpath) == ERROR)
    {
      mPrintf("  -- Can't find application directory.\n\n");
      changedir(cfg.homepath);
      return;
    }

    sprintf(comm, "COM%d", cfg.mdata);
    sprintf(stuff,"%s %s %u %d %s",
           roomBuf.rbaplic,
           onConsole ? "LOCAL" : comm,
           onConsole ? 2400    : bauds[speed],
           sysop,
           deansi(logBuf.lbname)); 

    apsystem(stuff);
    changedir(cfg.homepath);
}

/* -------------------------------------------------------------------- */
/*      shellescape()  handles the sysop '!' shell command              */
/* -------------------------------------------------------------------- */
void shellescape(int super)
{
    char command[80];

    changedir(roomBuf.rbflags.MSDOSDIR ? roomBuf.rbdirname : cfg.homepath);

    sprintf(command, "?%s%s", super ? "!" : "", getenv("COMSPEC"));

    apsystem(command);
    
    update25();

    changedir(cfg.homepath);
}

/* -------------------------------------------------------------------- */
/* -------------------------------------------------------------------- */
void writeAplFile(void)
{
    FILE *fd;
    char buff[80];
    int i;

    sprintf(buff, "%s\\OUTPUT.APL", cfg.aplpath);
    unlink(buff);
    sprintf(buff, "%s\\INPUT.APL", cfg.aplpath);
    unlink(buff);
    if (readMessage)
    {
        sprintf(buff, "%s\\MESSAGE.APL", cfg.aplpath);
        unlink(buff);
    }
    sprintf(buff, "%s\\README.APL", cfg.aplpath);
    unlink(buff);

    sprintf(buff, "%s\\OUTPUT.APL", cfg.aplpath);
    if ((fd = fopen(buff , "wb")) == NULL)
    {
        mPrintf("Can't make userdato.apl");
        return;
    }

    for (i = 0; AplTab[i].item != APL_END; i++)
    {
        switch(AplTab[i].type)
        {
        case TYP_STR:
            sprintf(buff, "%c%s\n", AplTab[i].item, AplTab[i].variable);
            break;

        case TYP_BOOL:
        case TYP_CHAR:
            sprintf(buff, "%c%d\n", AplTab[i].item,
                *((char *)AplTab[i].variable));
            break;

        case TYP_INT:
            sprintf(buff, "%c%d\n", AplTab[i].item,
                *((int *)AplTab[i].variable));
            break;

        case TYP_FLOAT:
            sprintf(buff, "%c%f\n", AplTab[i].item,
                    *((float *)AplTab[i].variable));
            break;

        case TYP_LONG:
            sprintf(buff, "%c%ld\n", AplTab[i].item,
                    *((long *)AplTab[i].variable));
            break;           

        case TYP_OTHER:
            switch (AplTab[i].item)
            {
            case APL_MDATA:
                if (onConsole)  
                {
                    sprintf(buff, "%c0 (LOCAL)\n", AplTab[i].item);
                }
                else
                {
                    sprintf(buff, "%c%d\n", AplTab[i].item, cfg.mdata);
                }
                break;

            case APL_HALL:
                sprintf(buff, "%c%s\n", AplTab[i].item, 
                        hallBuf->hall[thisHall].hallname);
                break;

            case APL_ROOM:
                sprintf(buff, "%c%s\n", AplTab[i].item, roomBuf.rbname);
                break;

            case APL_ACCOUNTING:
                if(!logBuf.lbflags.NOACCOUNT && cfg.accounting)
                {
                    sprintf(buff, "%c1\n", AplTab[i].item);
                }
                else
                {
                    sprintf(buff, "%c0\n", AplTab[i].item);
                }
                break;
            
            case APL_PERMANENT:
                sprintf(buff, "%c%d\n", AplTab[i].item, logBuf.lbflags.PERMANENT);
                break;
            
            case APL_VERIFIED:
                sprintf(buff, "%c%d\n", AplTab[i].item,
                        logBuf.VERIFIED ? 0 : 1);
                break;

            case APL_NETUSER:
                sprintf(buff, "%c%d\n", AplTab[i].item, logBuf.lbflags.NETUSER);
                break;

            case APL_NOMAIL:
                sprintf(buff, "%c%d\n", AplTab[i].item, logBuf.lbflags.NOMAIL);
                break;

            case APL_CHAT:
                sprintf(buff, "%c%d\n", AplTab[i].item, cfg.noChat);
                break;

            case APL_BELLS:
                sprintf(buff, "%c%d\n", AplTab[i].item, cfg.noBells);
                break;

            default:
                buff[0] = 0;
                break;
            }
            break;

        default:
            buff[0] = 0;
            break;
        }

        if (strlen(buff) > 1)
        {
            fputs(buff, fd);
        }
    }

/*    for (i=0; i<) */
    
    fprintf(fd, "%c\n", APL_END);
    
    fclose(fd);
}



/************************************************************************/
/*  Extended Download                                                   */ 
/************************************************************************/ 
void wxsnd(char *path, char *file, char trans)
{
    char  stuff[100];
    label tmp1, tmp2;

    if (changedir(path) == -1 )  return;

    sprintf(tmp1, "%d", cfg.mdata);
    sprintf(tmp2, "%u", bauds[speed]);
    sformat(stuff, extrn[trans-1].ex_snd, "fpsa", file, tmp1, tmp2, cfg.aplpath);
    apsystem(stuff);

    if (debug)  cPrintf("(%s)", stuff);
}

void wxrcv(char *path, char *file, char trans)
{
    char stuff[100];
    label tmp1, tmp2;

    if (changedir(path) == -1 )  return;

    sprintf(tmp1, "%d", cfg.mdata);
    sprintf(tmp2, "%u", bauds[speed]);
    sformat(stuff, extrn[trans-1].ex_rcv, "fpsa", file, tmp1, tmp2, cfg.aplpath);
    apsystem(stuff);

    if (debug)  cPrintf("(%s)", stuff);
}

/* -------------------------------------------------------------------- */
/*      readuserin()  reads userdati.apl from disk                      */
/* -------------------------------------------------------------------- */
void readAplFile(void)
{
    FILE *fd;
    int i;
    char buff[200];
    int item;
    int roomno;
    int found;
    int slot;

    if (readMessage)
    {
        clearmsgbuf();
        strcpy(msgBuf->mbauth, cfg.nodeTitle);
        msgBuf->mbroomno = thisRoom;
    }

    sprintf(buff, "%s\\INPUT.APL", cfg.aplpath);
    if ((fd = fopen(buff, "rt")) != NULL)
    {
        do
        {
            item = fgetc(fd);
            if (feof(fd)) 
            {
                break;
            }

            fgets(buff, 198, fd);
            buff[strlen(buff)-1] = 0;
    
            found = FALSE;

            for(i = 0; AplTab[i].item != APL_END; i++)
            {
                if (AplTab[i].item == item && AplTab[i].keep)
                {
                    found = TRUE;

                    switch(AplTab[i].type)
                    {
                    case TYP_STR:
                        strncpy((char *)AplTab[i].variable, buff, AplTab[i].length);
                        ((char *)AplTab[i].variable)[ AplTab[i].length - 1 ] = 0;
                        break;
    
                    case TYP_BOOL:
                    case TYP_CHAR:
                        *((char *)AplTab[i].variable) = (char)atoi(buff);
                        break;
    
                    case TYP_INT:
                        *((int *)AplTab[i].variable) = atoi(buff);
                        break;
    
                    case TYP_FLOAT:
                        *((float *)AplTab[i].variable) = (float)atof(buff);
                        break;
    
                    case TYP_LONG:
                        *((long *)AplTab[i].variable) = atol(buff);
                        break;
    
                    case TYP_OTHER:
                        switch (AplTab[i].item)
                        {
                        case APL_HALL:
                            if (stricmp(buff, hallBuf->hall[thisHall].hallname)
                                != SAMESTRING)
                            {
                                slot = hallexists(buff);
                                if (slot != ERROR)
                                {
                                    mPrintf("Hall change to: %s", buff);
                                    doCR();
                                    thisHall = (unsigned char)slot;
                                }
                                else
                                {
                                    cPrintf("No such hall %s!\n", buff);
                                }
                            }
                            break;
            
                        case APL_ROOM:
                            if ( (roomno = roomExists(buff)) != ERROR)
                            {
                                if (roomno != thisRoom)
                                {
                                    mPrintf("Room change to: %s", buff);
                                    doCR();
                                    logBuf.lbroom[thisRoom].lbgen   
                                            = roomBuf.rbgen; 
                           /* ug_lvisit = logBuf.lbroom[thisRoom].lvisit; */
                           ug_newpointer = logBuf.newpointer[thisRoom]; 
                                    ug_new    = talleyBuf->room[thisRoom].new;

                           /*    logBuf.lbroom[thisRoom].lvisit   = 0;  */
                                 logBuf.newpointer[thisRoom] = cfg.newest;


                                    talleyBuf->room[thisRoom].hasmail = 0;
                                   /*logBuf.lbroom[thisRoom].mail     = 0;*/

                                    /* zero new count in talleybuffer */
                                    talleyBuf->room[thisRoom].new     = 0;

                                    getRoom(roomno);
 
                                    if ((logBuf.lbroom[thisRoom].lbgen ) 
                                        != roomBuf.rbgen)
                                    {
                                        logBuf.lbroom[thisRoom].lbgen
                                                = roomBuf.rbgen;
                    /*   logBuf.lbroom[thisRoom].lvisit  = (MAXVISIT - 1); */

                                 logBuf.newpointer[thisRoom] = cfg.oldest;

                                    }
                                }
                            }
                            else
                            {
                                cPrintf("No such room: %s!\n", buff);
                            }
                            break;
                        
                        case APL_PERMANENT:
                            logBuf.lbflags.PERMANENT = atoi(buff);
                            break;
                        
                        case APL_VERIFIED:
                            logBuf.VERIFIED = !atoi(buff);
                            break;
            
                        case APL_NETUSER:
                            logBuf.lbflags.NETUSER = atoi(buff);
                            break;
            
                        case APL_NOMAIL:
                            logBuf.lbflags.NOMAIL = atoi(buff);
                            break;
            
                        case APL_CHAT:
                            cfg.noChat = (BOOL)atoi(buff);
                            break;
            
                        case APL_BELLS:
                            cfg.noBells = (BOOL)atoi(buff);
                            break;
            
                        default:
                            mPrintf("Bad value %d \"%s\"", item, buff); doCR();
                            break;
                        }
                        break;
    
                    default:
                        break;
                    }
                }
            }

            if (!found && readMessage)
            {
                found = TRUE;

                switch (item)
                {
                case MSG_NAME:
                    strcpy(msgBuf->mbauth, buff);
                    break;
    
                case MSG_TO:
                    strcpy(msgBuf->mbto, buff);
                    break;
    
                case MSG_GROUP:
                    strcpy(msgBuf->mbgroup, buff);
                    break;
    
                case MSG_ROOM:
                    if ( (roomno = roomExists(buff)) == ERROR)
                    {
                        cPrintf(" AP: No room \"%s\"!\n", buff);
                    }
                    else
                    {
                        msgBuf->mbroomno = roomno;
                    }
                    break;

                default:
                    doCR();
                    found = FALSE;
                    break;
                }
            }

            if (!found && AplTab[i].item != APL_END)
            {
                mPrintf("Bad value %d \"%s\"", item, buff); doCR();
            }
        }
        while (item != APL_END && !feof(fd));

        fclose(fd);
    }

    update25();

    if (readMessage)
    {
        sprintf(buff, "%s\\MESSAGE.APL", cfg.aplpath);
        if ((fd = fopen(buff, "rb")) != NULL)
        {
            GetFileMessage(fd, msgBuf->mbtext, MAXTEXT);
            fclose(fd);
          
            putMessage();
            noteMessage();
        }
        unlink(buff);
    }

    sprintf(buff, "%s\\README.APL", cfg.aplpath);
    if (filexists(buff))
    {
        dumpf(buff);
        unlink(buff);
        doCR();
    }
}

/* -------------------------------------------------------------------- */
/*  execDoor()      Execute a door                                      */
/* -------------------------------------------------------------------- */
BOOL execDoor(char c)
{
    int i;  /* , i2; */
    char str[80];
    char tmp1[10];
    char tmp2[10];
    
    if (c == '?')
    {
        return FALSE;
    }
   
    for (i=0; i<(int)numDoors; i++)
    {
        if (tolower(c) == tolower(doors[i].name[0]))
        {
            if (
                    (!doors[i].SYSOP || sysop)
                 && (!doors[i].AIDE  || aide)
                 && (!doors[i].CON   || onConsole)
                 && (!doors[i].DIR   || roomBuf.rbflags.MSDOSDIR)
               )
            {
                if (doors[i].group[0])
                {
                    if (ingroup(groupexists(doors[i].group)))
                        break;

#ifdef GOODBYE
                    for (i2 = 0 ; i2 < MAXGROUPS; ++i2)
                    {
                        if (strcmpi(grpBuf.group[i2].groupname, doors[i].group) 
                            == SAMESTRING)
                            break;
                    }
                    
                    if  (
                            i2 != MAXGROUPS
                        &&  logBuf.groups[i2] == grpBuf.group[i2].groupgen 
                        )
                        break;

#endif
                }
                else
                {
                    break;
                }
            }
        }
    }
    
    if (i == (int)numDoors)
    {
        return FALSE;
    }
        
    mPrintf("%s", doors[i].name);
    doCR();
    doCR();
    
    sprintf(tmp1, "%d", onConsole ? 0 : cfg.mdata);
    sprintf(tmp2, "%u", bauds[speed]);
    sformat(str, doors[i].cmd, "psa", tmp1, tmp2, cfg.aplpath);
    
    changedir(cfg.aplpath);
    apsystem(str);
    changedir(cfg.homepath);
    
    return TRUE;
}

/* -------------------------------------------------------------------- */
/*      apsystem() turns off interupts and makes a system call          */
/* -------------------------------------------------------------------- */
void apsystem(char *stuff)
{
    int clearit = TRUE,
        superit = FALSE,
        batch   = FALSE,
        door    = TRUE;
    char scratch[256];
    char *words[256];
    int  count;
    uchar c, r;
    char old_saver_on;

    while (*stuff == '!' || *stuff == '@' || *stuff == '$' || *stuff == '?')
    {
        if (*stuff == '!')   superit = TRUE;
        if (*stuff == '@')   clearit = FALSE;
        if (*stuff == '$')   batch   = TRUE;
        if (*stuff == '?')   door    = FALSE;
        stuff++;
    }

    if (door) writeAplFile();

    if (dowhat != NETWORKING && superit)
    {
        mPrintf("Swapping, please wait."); doCR();
    }
            
    if (disabled)
      drop_dtr();

    portExit();

    fcloseall();

    if(clearit)
    {
        save_screen();
        cls();
    }

    old_saver_on = saver_on;
    saver_on = FALSE;

    if (loggedIn && clearit)
        cPrintf("[User: %s]\n", deansi(logBuf.lbname));
    
    if (debug) cPrintf("(%s)\n", stuff);

    if (stricmp(stuff, getenv("COMSPEC")) == SAMESTRING)
        cPrintf("Use the EXIT command to return to %s", programName);

    saver_on = old_saver_on;

    if (batch)
        sprintf(scratch, "%s /c %s", getenv("COMSPEC"), stuff);
    else
        strcpy(scratch, stuff);

    count = parse_it(words, scratch);
    words[count] = NULL;

    if (!anyEcho)
    {
        curson();
    }


    if (superit)
        spawnvpeo(cfg.temppath, words[0], words, environ); 
    else
        spawnvp(P_WAIT, words[0], words);

    /* clear out keyboard buffer, I dunno about this */
    while (STATCON()) GETCON();
  
    getScreenSize(&c, &r);
    if (c != conCols || r != conRows)  
                                /* restore the screen if applicaton */
    {                           /* changes modes.. */
        setscreen();
    }

    /* back to blank screen saver mode */
    if (saver_on)
    {
        saver_on = FALSE;
        cls();
        saver_on = TRUE;
    }
    
    if (clearit && anyEcho)
    {
        restore_screen();
    }


    if (!anyEcho)
    {
        anyEcho = TRUE;

        cursoff();
        logo();
        anyEcho = FALSE;
    }


    portInit();
    baud((int)speed);

    if (aideFl != NULL)
    {
        sprintf(scratch, "%s\\%s", cfg.temppath, "aidemsg.tmp");
        if ((aideFl = fopen(scratch, "a")) == NULL)
        {
            crashout("Can not open AIDEMSG.TMP!");
        }
    }
    trapfl = fopen(cfg.trapfile, "a+");
    sprintf(scratch, "%s\\%s", cfg.msgpath, "msg.dat");
    openFile(scratch,    &msgfl );
    sprintf(scratch, "%s\\%s", cfg.homepath, "grp.dat");
    openFile(scratch,  &grpfl );
    sprintf(scratch, "%s\\%s", cfg.homepath, "hall.dat");
    openFile(scratch,  &hallfl );
    sprintf(scratch, "%s\\%s", cfg.homepath, "log.dat");
    openFile(scratch,  &logfl );
    sprintf(scratch, "%s\\%s", cfg.homepath, "room.dat");
    openFile(scratch,  &roomfl );

    if (disabled)
    {
        drop_dtr();
    }

    readAplFile();

    sprintf(scratch, "%s\\OUTPUT.APL", cfg.aplpath);
    unlink(scratch);

    readMessage = TRUE;

    time(&keyboard_timer); /* for screensaver */
}

