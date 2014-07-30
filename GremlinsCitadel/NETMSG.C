/* -------------------------------------------------------------------- */
/*  NETMSG.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*      Networking libs for the Citadel bulletin board system           */
/*              Networking message handling rutines                     */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  GetStr()        gets a null-terminated string from a file           */
/*  PutStr()        puts a null-terminated string to a file             */
/*  GetMessage()    Gets a message from a file, returns sucess          */
/*  PutMessage()    Puts a message to a file                            */
/*  NewRoom()       Puts all new messages in a room to a file           */
/*  saveMessage()   saves a message to file if it is netable            */
/*  ReadMsgFile()   Reads a message file into thisRoom                  */
/*  netcanseeroom() Can the node see this room?                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  GetStr()        gets a null-terminated string from a file           */
/* -------------------------------------------------------------------- */
BOOL GetStr(FILE *fl, char *str, int mlen)
{
    int   l = 0;
    uchar ch = 1;

    if (feof(fl))
        return(FALSE);
  
    while(!feof(fl) && ch)
    {
        ch=(uchar)fgetc(fl);
        if ((ch != 0xFF && ch != '\r')/* tfilter[ch]*/ && l < mlen)
        {
            str[l]=ch /* tfilter[ch] */;
            l++;
        }
    }
    str[l]='\0';

    if (!strlen(str))
        return(FALSE);

    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  GetFStr()       gets a null-terminated FILTERED string from a file  */
/*                  or Totalitarian Communist string processing.        */
/*                  fLevel      0 = filters ^B codes only               */
/*                              1 = filters ^A codes & ^B codes.        */
/*                              2 = IBM extended                        */
/* -------------------------------------------------------------------- */
void GetFStr(FILE *fl, char *str, int mlen, int fLevel)
{
    int   l = 0;
    uchar ch = 1;
    BOOL  ctrla = FALSE;
    BOOL  ctrlb = FALSE;
    
    while(!feof(fl) && ch)
    {
        ch=(uchar)fgetc(fl);
        if (   (ch != 0xFF && ch != '\r')       /* no FFs, or CRs (LFs only) */
            && ( tfilter[ch] || fLevel < 2)      /* No IBM extended */
            && l < mlen)                        /* Not past end of str */
        {
            if (ch == CTRL_A /* CTRL-A */)
            {
                ctrla = TRUE;
            }
            else
            if (ch == CTRL_B /* CTRL-A */)
            {
                ctrlb = TRUE;
            }
            else
            if (ctrlb || (ctrla && fLevel > 0))
            {
                ctrla = TRUE;
                ctrlb = FALSE;
            }
            else
            {
                if (ctrla)
                {
                    str[l]=CTRL_A;
                    l++;     
                    ctrla=FALSE;
                }
                
                str[l]=ch;
                l++;
            }
        }
    }
    str[l]='\0';
}

/* -------------------------------------------------------------------- */
/*  PutStr()        puts a null-terminated string to a file             */
/* -------------------------------------------------------------------- */
void PutStr(FILE *fl, char *str)
{
    fwrite(str, sizeof(char), (strlen(str) + 1), fl);
}

/* -------------------------------------------------------------------- */
/*  GetMessage()    Gets a message from a file, returns sucess          */
/* -------------------------------------------------------------------- */
BOOL GetMessage(FILE *fl)
{
    struct unkLst *lul; /* brent, I dunno */


    char c;

    /* clear message buffer out */
    clearmsgbuf();

    /* find start of message */
    do
    {
        c = (uchar)fgetc(fl);
    } while (c != -1 && !feof(fl));

    if (feof(fl))
        return FALSE;

    /* get message's attribute byte */
    msgBuf->mbattr = (uchar)fgetc(fl);

    GetStr(fl, msgBuf->mbId, LABELSIZE);

    do 
    {
        c = (uchar)fgetc(fl);
        switch (c)
        {
        case 'A':     GetFStr(fl, msgBuf->mbauth,  LABELSIZE, 0);    break;
        case 'B':     GetFStr(fl, msgBuf->mbsub,   79       , 0);    break;
        case 'D':     GetStr(fl,  msgBuf->mbtime,  LABELSIZE);       break;
        case 'F':     GetFStr(fl, msgBuf->mbfwd,   LABELSIZE, 0);    break;
        case 'G':     GetFStr(fl, msgBuf->mbgroup, LABELSIZE, 0);    break;
        case 'I':     GetStr(fl,  msgBuf->mbreply, LABELSIZE);       break;
        case 'J':     GetFStr(fl, msgBuf->mbcreg,  LABELSIZE, 0);    break;
        case 'j':     GetFStr(fl, msgBuf->mbccont, LABELSIZE, 0);    break;
        case 'M':     /* will be read off disk later */              break;
        case 'N':     GetFStr(fl, msgBuf->mbtitle, LABELSIZE, 0);    break;
        case 'n':     GetFStr(fl, msgBuf->mbsur,   LABELSIZE, 0);    break;
        case 'O':     GetFStr(fl, msgBuf->mboname, LABELSIZE, 0);    break;
        case 'o':     GetFStr(fl, msgBuf->mboreg,  LABELSIZE, 0);    break;
        case 'P':     GetFStr(fl, msgBuf->mbfpath, 256      , 0);    break;
        case 'p':     GetFStr(fl, msgBuf->mbtpath, 256      , 0);    break;
        case 'Q':     GetFStr(fl, msgBuf->mbocont, LABELSIZE, 0);    break;
        case 'q':     GetFStr(fl, msgBuf->mbczip,  LABELSIZE, 0);    break;
        case 'R':     GetFStr(fl, msgBuf->mbroom,  LABELSIZE, 0);    break;
        case 'S':     GetStr(fl,  msgBuf->mbsrcId, LABELSIZE);       break;
        case 's':     GetFStr(fl, msgBuf->mbsoft,  LABELSIZE, 0);    break;
        case 'T':     GetFStr(fl, msgBuf->mbto,    LABELSIZE, 0);    break;
/*      case 'X':     GetStr(fl,  msgBuf->mbx,     LABELSIZE);       break; */
        case 'Z':     GetFStr(fl, msgBuf->mbzip,   LABELSIZE, 0);    break;
        case 'z':     GetFStr(fl, msgBuf->mbrzip,  LABELSIZE, 0);    break;
        case '.':     GetFStr(fl, msgBuf->mbsig,   90       , 0);    break;
        case '_':     GetFStr(fl, msgBuf->mbusig,  90       , 0);    break;
        /* nodephone */
        case 'H':     GetFStr(fl, msgBuf->mbophone,LABELSIZE, 0);    break;
        case 'h':     GetFStr(fl, msgBuf->mbzphone,LABELSIZE, 0);    break;

        case '\0':  break;

#ifdef GOODBYE        
        default:
            GetStr(fl, msgBuf->mbtext, cfg.maxtext);  /* discard unknown field  */
            msgBuf->mbtext[0]    = '\0';
            break;
#endif


        default: 
            { 
            /* try to store unknown field */ 
            if ((lul = addUnknownList()) != NULL) 
                { 
                lul->whatField = c; 
                GetStr(fl, lul->theValue, MAXUNKLEN); 
                } 
            else 
                { 
                /* cannot save it - discard unknown field  */ 
                GetStr(fl, msgBuf->mbtext, MAXTEXT); 
                msgBuf->mbtext[0]    = '\0'; 
                } 
            } 



        }
    } while (c != 'M' && !feof(fl));

    if (feof(fl))
    {
        return FALSE;
    }

    GetFStr(fl, msgBuf->mbtext, MAXTEXT, 0);  /* get the message field  */
    
    if (!*msgBuf->mboname)
    {
        strcpy(msgBuf->mboname, node.ndname);
    }

    if (!*msgBuf->mboreg)
    {
        strcpy(msgBuf->mboreg, node.ndregion);
    }

    if (!*msgBuf->mbsrcId)
    {
        strcpy(msgBuf->mbsrcId, msgBuf->mbId);
    }

    /*
     * If the other node did not set up a from path, do it.
     */
    if (!*msgBuf->mbfpath)
    {
        if (strcmpi(msgBuf->mboname, node.ndname) == 0)
        {
            strcpy(msgBuf->mbfpath, msgBuf->mboname);
        }
        else
        {
            /* last node did not originate, make due with what we got... */
            strcpy(msgBuf->mbfpath, msgBuf->mboname);
            strcat(msgBuf->mbfpath, "!..!");
            strcat(msgBuf->mbfpath, node.ndname);
        }
    }


    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  PutMessage()    Puts a message to a file                            */
/* -------------------------------------------------------------------- */
void PutMessage(FILE *fl)
{
    struct unkLst *lul; /* brent, I dunno */


    /* write start of message */
    fputc(0xFF, fl);

    /* put message's attribute byte */
    msgBuf->mbattr = (uchar)(msgBuf->mbattr & (ATTR_RECEIVED|ATTR_REPLY));
    fputc(msgBuf->mbattr, fl);

    /* put local ID # out */
    PutStr(fl, msgBuf->mbId);

    if (!msgBuf->mbsrcId[0])
    {
        strcpy(msgBuf->mboname, cfg.nodeTitle);
        strcpy(msgBuf->mboreg,  cfg.nodeRegion);
        strcpy(msgBuf->mbocont, cfg.nodeContry);
        strcpy(msgBuf->mbsrcId, msgBuf->mbId);
        strcpy(msgBuf->mbsoft,  programName);
        strcat(msgBuf->mbsoft,  " ");
        strcat(msgBuf->mbsoft,  version);


        strcpy(msgBuf->mbcreg,  cfg.twitRegion);
        strcpy(msgBuf->mbccont, cfg.twitCountry);

        /* nodephone */
        strcpy(msgBuf->mbophone, cfg.nodephone);

    }
    
    if (*msgBuf->mbfpath)
    {
        strcat(msgBuf->mbfpath, "!");
    }
    strcat(msgBuf->mbfpath, cfg.nodeTitle);

    if (!msgBuf->mbtime[0])
    {
        sprintf(msgBuf->mbtime, "%ld", time(NULL));
    }
    
    fputc('A', fl); PutStr(fl, msgBuf->mbauth);
    fputc('D', fl); PutStr(fl, msgBuf->mbtime);
    fputc('O', fl); PutStr(fl, msgBuf->mboname);
    fputc('o', fl); PutStr(fl, msgBuf->mboreg);
    fputc('S', fl); PutStr(fl, msgBuf->mbsrcId);
    fputc('P', fl); PutStr(fl, msgBuf->mbfpath);
    
    if (msgBuf->mbsub[0])   { fputc('B', fl); PutStr(fl, msgBuf->mbsub);   }
    if (msgBuf->mbfwd[0])   { fputc('F', fl); PutStr(fl, msgBuf->mbfwd);   }
    if (msgBuf->mbgroup[0]) { fputc('G', fl); PutStr(fl, msgBuf->mbgroup); }
    if (msgBuf->mbreply[0]) { fputc('I', fl); PutStr(fl, msgBuf->mbreply); }
    if (msgBuf->mbcreg[0])  { fputc('J', fl); PutStr(fl, msgBuf->mbcreg);  }
    if (msgBuf->mbccont[0]) { fputc('j', fl); PutStr(fl, msgBuf->mbccont); }
    if (msgBuf->mbtitle[0]) { fputc('N', fl); PutStr(fl, msgBuf->mbtitle); }
    if (msgBuf->mbsur[0])   { fputc('n', fl); PutStr(fl, msgBuf->mbsur);   }
    if (msgBuf->mbtpath[0]) { fputc('p', fl); PutStr(fl, msgBuf->mbtpath); }
    if (msgBuf->mbocont[0]) { fputc('Q', fl); PutStr(fl, msgBuf->mbocont); }
    if (msgBuf->mbczip[0])  { fputc('q', fl); PutStr(fl, msgBuf->mbczip);  }
    if (msgBuf->mbroom[0])  { fputc('R', fl); PutStr(fl, msgBuf->mbroom);  }
    if (msgBuf->mbsoft[0])  { fputc('s', fl); PutStr(fl, msgBuf->mbsoft);  }
    if (msgBuf->mbto[0])    { fputc('T', fl); PutStr(fl, msgBuf->mbto);    }
    if (msgBuf->mbzip[0])   { fputc('Z', fl); PutStr(fl, msgBuf->mbzip);   }
    if (msgBuf->mbrzip[0])  { fputc('z', fl); PutStr(fl, msgBuf->mbrzip);  }
    if (msgBuf->mbsig[0])   { fputc('.', fl); PutStr(fl, msgBuf->mbsig);   }
    if (msgBuf->mbusig[0])  { fputc('_', fl); PutStr(fl, msgBuf->mbusig);  }
    /* nodephone */
    if (msgBuf->mbophone[0]){ fputc('H', fl); PutStr(fl, msgBuf->mbophone);}
    if (msgBuf->mbzphone[0]){ fputc('h', fl); PutStr(fl, msgBuf->mbzphone);}


    /* put any unknowns... */ 
    for(lul = firstUnk; lul != NULL; lul = lul->next) 
        { 
        fputc(lul->whatField, fl); 
        PutStr(fl, lul->theValue); 
        } 


    /* put the message field  */
    fputc('M', fl); PutStr(fl, msgBuf->mbtext);
}

/* -------------------------------------------------------------------- */
/*  NewRoom()       Puts all new messages in a room to a file           */
/* -------------------------------------------------------------------- */
void NewRoom(int room, char *filename)
{
#ifdef NEWMSGTAB
    ulong i;
    struct messagetable *lmt;
#else
    int i;
#endif
    ulong size_table;

    int   h;
    char str[100];
    ulong lowLim, highLim, msgNo;
    FILE *file;

    /* lowLim  = logBuf.lbvisit[ logBuf.lbroom[room].lvisit ] + 1; */

    lowLim = logBuf.newpointer[room] + 1;
    highLim = cfg.newest;

    /* logBuf.lbroom[room].lvisit = 0; */
    logBuf.newpointer[room] = cfg.newest;


    /* stuff may have scrolled off system unseen, so: */
    if (cfg.oldest  > lowLim)  lowLim = cfg.oldest;

    sprintf(str, "%s\\%s", cfg.temppath, filename);


#ifdef GOODBYE
    file = fopen(str, "ab");
    if (!file)
    {
        return;
    }
#endif

    if ((file = fopen(str, "ab")) == NULL)
    {
        /* perror("Cannot open 'room.###'"); */
        return;
    }

    h = hash(cfg.nodeTitle);

    size_table = sizetable();

#ifdef NEWMSGTAB    
    for (i = 0; i != size_table; i++)
#else
    for (i = 0; i != (int)size_table; i++)
#endif

    {

        msgNo = (ulong)(cfg.mtoldest + i);
        
        if ( msgNo >= lowLim && highLim >= msgNo )
        {
#ifdef NEWMSGTAB
            lmt = getMsgTab(i);
#endif

            /* skip messages not in this room */

#ifdef NEWMSGTAB
            if (lmt->mtroomno != room) continue;
#else
            if (msgTab_mtroomno[i] != (uchar)room) continue;
#endif
    
            /* no open messages from the system */
#ifdef NEWMSGTAB
            if (lmt->mtauthhash == h) continue;
#else
            if (msgTab_mtauthhash[i] == h) continue;
#endif
    
            /* skip mail */
#ifdef NEWMSGTAB
            if (lmt->mtmsgflags.MAIL) continue;
#else
            if (msgTab_mtmsgflags[i].MAIL) continue;
#endif
    
            /* No problem user shit */

#ifdef NEWMSGTAB
            if (
                (lmt->mtmsgflags.PROBLEM || lmt->mtmsgflags.MODERATED) 
            && !(lmt->mtmsgflags.MADEVIS)
               )
#else
            if (
                (msgTab_mtmsgflags[i].PROBLEM || msgTab_mtmsgflags[i].MODERATED) 
            && !(msgTab_mtmsgflags[i].MADEVIS)
               )
#endif
            { 
                continue;
            }

            copyflag = FALSE;
            saveMessage( msgNo, file );
            mread ++;
        }
    }
    fclose(file);
}

/* -------------------------------------------------------------------- */
/*  saveMessage()   saves a message to file if it is netable            */
/* -------------------------------------------------------------------- */
void saveMessage(ulong id, FILE *fl)
{
    ulong here;
    ulong loc;
    ulong result;
/*  int originroomno; */

#ifdef NEWMSGTAB
    ulong slot;
#else
    int   slot;
#endif
    FILE *fl2;
/*   label str; */


    result = indexslot(id);

#ifdef NEWMSGTAB    
    slot = indexslot(id);
#else
    slot = (int)indexslot(id);
#endif
    
    if (result == ULONG_ERROR) return;


#ifdef NEWMSGTAB
    if (getFlags(slot)->COPY)
#else
    if (msgTab_mtmsgflags[slot].COPY)
#endif

    {
        copyflag     = TRUE;
        originalId   = id;
        originalattr = 0;


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

#ifdef GOODBYE                       
        if (msgTab3[slot].mtoffset <= (ushort)slot)
            saveMessage( (ulong)(id - (ulong)msgTab3[slot].mtoffset), fl);
#endif


#ifdef NEWMSGTAB
/*      if (getOriginID(slot)     <= (ushort)slot)  */
        if (long_JOIN(getToHash(slot), getAuthHash(slot))     <= slot)
#else
   /*   if (msgTab_mtomesg[slot]  <= (ushort)slot)  */

if(long_JOIN(msgTab_mttohash[slot], msgTab_mtauthhash[slot]) <= (ushort)slot) 


#endif

#ifdef NEWMSGTAB
        /*  saveMessage( (ulong)(id - (ulong)getOriginID(slot)    ), fl); */
            saveMessage( (id - long_JOIN(getToHash(slot), getAuthHash(slot))   ), fl);
#else
        /*  saveMessage( (ulong)(id - (ulong)msgTab_mtomesg[slot] ), fl);  */

            saveMessage( (ulong)(id - long_JOIN(msgTab_mttohash[slot], 
            msgTab_mtauthhash[slot])) , fl);

#endif

        return;
    }

    /* in case it returns without clearing buffer */
    msgBuf->mbfwd[  0]  = '\0';
    msgBuf->mbto[   0]  = '\0';

/*  loc = msgTab2[slot].mtmsgLoc; */

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


#ifdef NEWMSGTAB
    if (!mayseeindexmsg(slot) && !getFlags(slot)->NET) return;
#else
    if (!mayseeindexmsg(slot) && !msgTab_mtmsgflags[slot].NET) return;
#endif

    fseek(msgfl, loc, 0);

    getMessage();
    getMsgStr(msgBuf->mbtext, MAXTEXT);

    sscanf(msgBuf->mbId, "%lu", &here);

    /* cludge to return on dummy msg #1 */
    /* if ((int)here == 1) return; */

#ifdef NEWMSGTAB
    if (!mayseemsg() && !getFlags(slot)->NET) return;
#else
    if (!mayseemsg() && !msgTab_mtmsgflags[slot].NET) return;
#endif

    if (here != id )
    {
        cPrintf("Can't find message. Looking for %lu at byte %ld!\n ",
                 id, loc);
        return;
    }

    if (msgBuf->mblink[0])
    {
        if ((fl2 = fopen(msgBuf->mblink, "rt")) == NULL)
        {
            return;
        }
        GetFileMessage(fl2, msgBuf->mbtext, MAXTEXT);
        fclose(fl2);
    }

#ifdef HENGE    
    if (node.network == NET_HENGE)
    {
        HengePutMessage(fl);
    }
    else
    {
#endif         
        PutMessage(fl);
#ifdef HENGE    
    }
#endif         
}

#ifdef GOODBYE

        if (node.network == NET_1_69)
        {

#ifdef NEWMSGTAB    
    slot = indexslot(originalId);
#else
    slot = (int)indexslot(originalId);
#endif
           /* booga */

#ifdef NEWMSGTAB
if (strcmpi(msgBuf->mbroom, (copyflag) ? 
roomTab[getRoomNum(indexslot(originalId))].rtname : 
roomTab[msgBuf->mbroomno].rtname) == SAMESTRING)
#else
if (strcmpi(msgBuf->mbroom, (copyflag) ? 
roomTab[msgTab_mtroomno[(int)indexslot(originalId)]].rtname :
roomTab[msgBuf->mbroomno].rtname) == SAMESTRING)
#endif
            {
                strcpy(str, msgBuf->mbroom);
                NametoId(str);
                strcpy(msgBuf->mbroom, str);
            }


#ifdef GOODBYE
            if (!copyflag)
            {
                strcpy(str, msgBuf->mbroom);
                NametoId(str);
                strcpy(msgBuf->mbroom, str);
            }
#endif

#ifdef GOODBYE
            if (copyflag)
            {
#ifdef NEWMSGTAB
                originroomno = getRoomNum(indexslot(originalId));
#else
                originroomno = msgTab_mtroomno[(int)indexslot(originalId)];
#endif
                strcpy(msgBuf->mbroom, roomTab[originroomno].rtname);
            }
            else
            {
                strcpy(str, msgBuf->mbroom);
                NametoId(str);
                strcpy(msgBuf->mbroom, str);
            }

#ifdef GOODBYE
            if (strcmpi(roomTab[msgBuf->mbroomno].rtname, msgBuf->mbroom) == SAMESTRING)
            {
                strcpy(str, msgBuf->mbroom);
                NametoId(str);
                strcpy(msgBuf->mbroom, str);
            }
#endif

#endif
        }

        PutMessage(fl);
#ifdef HENGE    
    }
#endif         
}
#endif

/* -------------------------------------------------------------------- */
/*  ReadMsgFile()   Reads a message file into thisRoom                  */
/* -------------------------------------------------------------------- */
int ReadMsgFl(int room, char *filename, char *here, char *there)
{
    FILE *file, *fl;
    char str[100];
    ulong oid, loc;
    long l;
    int oauth,  bad, /* oname, */ temproom, lp, goodmsg = 0;
    int j;

    ulong size_table;

#ifdef NEWMSGTAB
    ulong i;
#else
    int i;
#endif


    expired = 0;   duplicate = 0;

    sprintf(str, "%s\\%s", cfg.temppath, filename);


#ifdef GOODBYE
    file = fopen(str, "rb");

    if (!file)
        return -1;
#endif

    if ((file = fopen(str, "rb")) == NULL)
    {
        /* perror("Cannot open 'room.###'"); */
        return -1;
    }

    while(GetMessage(file) == TRUE)
    {
        msgBuf->mbroomno = room;

        sscanf(msgBuf->mbsrcId, "%ld", &oid);
     /* oname = hash(msgBuf->mboname); */
        oauth = hash(msgBuf->mbauth);

        memcpy( msgBuf2, msgBuf, sizeof(struct msgB) );

        bad = FALSE;

        if (strcmpi(cfg.nodeTitle, msgBuf->mboname) == SAMESTRING)
        { 
            bad = TRUE; 
            duplicate++; 
        }

        /* Throw away authorless messages */
        if (!msgBuf->mbauth[0])
        {
            bad = TRUE; 
            duplicate++; 
        }

        if (*msgBuf->mbzip) /* is mail */
        {
            /* not for this system */
            if (strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING)
            {
                if (!save_mail())
                {
                    clearmsgbuf();
                    strcpy(msgBuf->mbauth, "Sysop");
                    strcpy(msgBuf->mbto,   msgBuf2->mbauth);
                    strcpy(msgBuf->mbzip,  msgBuf2->mboname);
                    strcpy(msgBuf->mbrzip, msgBuf2->mboreg);
                    strcpy(msgBuf->mbroom, msgBuf2->mbroom);
                    sprintf(msgBuf->mbtext, 
                           " \n Can not find route to '%s'.", msgBuf2->mbzip);
                    amPrintf( 
                        " Can not find route to '%s' in message from '%s'.\n",
                        msgBuf2->mbzip, msgBuf2->mboname);
                    netError = TRUE;
                
                    save_mail();
                    duplicate++;
                }
                else
                {
                    expired++;
                }
                bad = TRUE;
            }
            else 
            {
                /* 
                 * for this system 
                 */
                
                if (strcmpi(msgBuf->mbto, cfg.nodeTitle) == SAMESTRING)
                {
                    /*
                     * Special command.. 
                     */
                    sprintf(str, "%s\\OUTPUT.NET", cfg.transpath);
                    if ((fl = fopen(str, "ab")) != NULL)
                    {
                        PutMessage(fl);
                        fclose(fl);
                    }
                    bad = TRUE;
                    expired++;
                }
                else


#ifdef GOODBYE
                if (*msgBuf->mbto && personexists(msgBuf->mbto) == ERROR
                    && strcmpi(msgBuf->mbto, "Sysop") != SAMESTRING)
#endif

    /* handled forwarded E-mail. Check existence of forwardee if forwarded */
    if (*msgBuf->mbto 
    && 
    personexists(*msgBuf->mbfwd ? msgBuf->mbfwd : msgBuf->mbto)
    == ERROR
    && 
    strcmpi(*msgBuf->mbfwd ? msgBuf->mbfwd : msgBuf->mbto, "Sysop")
    != SAMESTRING)

                {
                    clearmsgbuf();
                    strcpy(msgBuf->mbauth, "Sysop");
                    strcpy(msgBuf->mbto,   msgBuf2->mbauth);
                    strcpy(msgBuf->mbzip,  msgBuf2->mboname);
                    strcpy(msgBuf->mbrzip, msgBuf2->mboreg);
                    strcpy(msgBuf->mbroom, msgBuf2->mbroom);
                    sprintf(msgBuf->mbtext, 
                        " \n No '%s' user found on %s.", msgBuf2->mbto,
                        cfg.nodeTitle);
                    save_mail();
                    bad = TRUE;
                    duplicate++;
                }
            }
        } 
        else 
        {
            /* is public */
            if (!bad)
            {

                size_table = sizetable();
#ifdef NEWMSGTAB
                for (i = size_table; i <= size_table && !bad; i--)
#else
                for (i = (int)size_table; i <= (int)size_table && !bad; i--)
#endif

                {

                /* just check origin id and author hash */
                /*  if (msgTab_mtorigin[i] == oname */


#ifdef NEWMSGTAB
                    if (getAuthHash(i) == oauth
                       && (uint)oid == getOriginID(i) )
#else
                    if (msgTab_mtauthhash[i] == oauth
                       && (uint)oid == msgTab_mtomesg[i])
#endif

                    {
                      /*  loc = msgTab2[i].mtmsgLoc; */


#ifdef NEWMSGTAB
    loc  = getLocation(i);
#else
    loc  = long_JOIN(msgTab_mtmsgLocLO[i], msgTab_mtmsgLocHI[i]); 
#endif


                        fseek(msgfl, loc, 0);
                        getMessage();
                        if ((strcmpi(msgBuf->mbauth, msgBuf2->mbauth)   
                                                                == SAMESTRING
                         || strcmpi(msgBuf->mboname, msgBuf2->mboname)
                                                                == SAMESTRING)
                         && strcmpi(msgBuf->mbtime, msgBuf2->mbtime)  
                                                                == SAMESTRING
                         && strcmpi(msgBuf->mbsrcId, msgBuf2->mbsrcId)  
                                                                == SAMESTRING
                           )
                        {
                            bad = TRUE; 
                            duplicate++; 
                        }
                    }
                }
            }

            memcpy( msgBuf, msgBuf2, sizeof(struct msgB) );
    
            /* fix group only messages, or discard them! */
            if (*msgBuf->mbgroup && !bad)
            {
                bad = TRUE;
                for (j=0; node.ndgroups[j].here[0]; j++)
                {
                    if (strcmpi(node.ndgroups[j].there, msgBuf->mbgroup) == SAMESTRING)
                    {
                        strcpy(msgBuf->mbgroup, node.ndgroups[j].here);
                        bad = FALSE;
                    }
                }
                /* put it in RESERVED_2 */
                if (bad)
                {
                    bad = FALSE;
                    sprintf(str, " \n 3 Old group was %s. 0", msgBuf->mbgroup);
                    strcat(msgBuf->mbtext, str);
                    strcpy(msgBuf->mbgroup, grpBuf.group[1].groupname);
                }
            }
    
            /* Expired? */
            if ( atol(msgBuf2->mbtime) 
                < (time(&l) - ((long)node.ndexpire *60*60*24)) ) 
            {
                bad = TRUE;
                expired++;
            }
        }

        if (!bad)
        { /* its good, save it */
            temproom = room;

            if (node.network == NET_1_69)
            {
                /* put roomname here */
                strcpy(msgBuf->mbroom, here);
            }
            else
            {
                if (strcmpi(msgBuf->mbroom, there) == SAMESTRING)
                    strcpy(msgBuf->mbroom, here);
            }

#ifdef GOODBYE
            if (node.network == NET_1_69)
            {
                strcpy(str, here);
                NametoId(str);

                if (strcmpi(msgBuf->mbroom, str) == SAMESTRING)
                    strcpy(msgBuf->mbroom, here);
            }
            else
            {
                if (strcmpi(msgBuf->mbroom, there) == SAMESTRING)
                    strcpy(msgBuf->mbroom, here);
            }
#endif

            if (*msgBuf->mbto)
                temproom = NfindRoom(msgBuf->mbroom);

            msgBuf->mbroomno = temproom;



           /* putmessage stores the room name from the roomtable */
           /* this cludge temporarily stores the roomname into the */
           /* room table so we can detect moved messages */
           /* we might need to only do this with net 1_69 */

            strcpy(str, roomTab[msgBuf->mbroomno].rtname);
            strcpy(roomTab[msgBuf->mbroomno].rtname, msgBuf->mbroom);



/*********************************************************************/
/* this is some shit added 7-9-92 to forward messages to 'sysop' to  */
/* #sysop if #forward                                                */
/*********************************************************************/


                if ((strcmpi(msgBuf->mbto, "Sysop") == SAMESTRING) &&
                    cfg.forward && (personexists(cfg.sysop) != ERROR) )
                {
                    strcpy(msgBuf->mbto, cfg.sysop);
                }


/*********************************************************************/
/*************** I see stars   ***************************************/
/*********************************************************************/



            putMessage();

           /* retore the room table */

            strcpy(roomTab[msgBuf->mbroomno].rtname, str);

            noteMessage();

    /* I am not sure about this */
    /* Set newpointers here because if I don't then the new messages */
    /* Will be sent back next time there's a network */
    logBuf.newpointer[msgBuf->mbroomno] = cfg.newest;

            goodmsg++;

            if (*msgBuf->mbto)
            {
                lp = thisRoom;
                thisRoom = temproom;
                /* notelogmessage(msgBuf->mbto); */
                thisRoom = lp;
            }
        }
    }
    fclose(file);


#ifdef GOODBYE
    /* I am not sure about this */
    /* Set newpointers here because if I don't then the new messages */
    /* Will be sent back next time there's a network */
    logBuf.newpointer[msgBuf->mbroomno] = cfg.newest;
#endif

    return goodmsg;
}

/* -------------------------------------------------------------------- */
/*  netcanseeroom() Can the node see this room?                         */
/* -------------------------------------------------------------------- */
BOOL netcanseeroom(int roomslot)
{ 
        /* is room in use              */
    if ( roomTab[roomslot].rtflags.INUSE

        /* and it is shared            */
        && roomTab[roomslot].rtflags.SHARED 

        /* and group can see this room */
        && (groupseesroom(roomslot)
        || roomTab[roomslot].rtflags.READONLY
        || roomTab[roomslot].rtflags.DOWNONLY )       

        /* only aides go to aide room  */ 
        &&   ( roomslot != AIDEROOM || aide) )
    {
        return TRUE;
    }

    return FALSE;
}


