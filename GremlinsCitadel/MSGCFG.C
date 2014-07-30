/************************************************************************/
/*  MSGCFG.C                 Dragon Citadel                             */
/************************************************************************/
/*               This is the high level message code.                   */
/************************************************************************/
#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/************************************************************************/
/*  msgInit()       builds message table from msg.dat                   */
/*  zapMsgFile()    initializes msg.dat                                 */
/*  slidemsgTab()   frees slots at beginning of msg table               */
/************************************************************************/

/************************************************************************/
/*      msgInit() sets up lowId, highId, cfg.catSector and cfg.catChar, */
/*      by scanning over message.buf                                    */
/************************************************************************/
void msgInit(void)
{
    ulong first, here;
    ulong makeroom;
    ulong skipcounter = 0;   /* maybe need to skip a few . Dont put them in
                              message index */

#ifdef NEWMSGTAB
    ulong slot;
#else
    int slot;
#endif

    long neatoconstant;

#if 1
    char twirl[] = "/|\\-";
    int loc=0, nowloc;
#endif
    int result;


    /* for fuel bar calculations */
    neatoconstant =  (long)(cfg.messagek * 16L);


    cPrintf("\n\nBuilding message table\n");

    /* start at the beginning */
    fseek(msgfl, 0l, 0);

    getMessage();

    /* read rest of message in and remember where it ends,      */
    /* in case it turns out to be the last message              */
    /* in which case, that's where to start writing next message*/
    getMsgStr(msgBuf->mbtext, MAXTEXT); 



    /* get the ID# */
#if 0
    sscanf(msgBuf->mbId, "%ld", &first);
#else
    first = atol(msgBuf->mbId);
#endif
            
#if 1
    outCon('|');
    for (nowloc = 0; nowloc < 64; nowloc++)   outCon('ù');
    outCon('|'); PUTCON('\b');
    for (nowloc = 0; nowloc < 64; nowloc++)   PUTCON('\b');
#else
    showtypemsg(first);
#endif

    /* put the index in its place */
    /* mark first entry of index as a reference point */

    cfg.mtoldest = first;
    
    indexmessage(first);

    cfg.newest = cfg.oldest = first;

    cfg.catLoc = ftell(msgfl);

    for(;;) /* while (TRUE) */
    {
        /* result is zero if getmessage encounters 10000 blank characters */
        result = getMessage();
        /* getMessage(); */

#if 0
        sscanf(msgBuf->mbId, "%ld", &here);
#else
        here = atol(msgBuf->mbId);
#endif

        /* if (here == first) break; */

        if ((here == first) || !result) break;

#if 1
/* formula */
/*      nowloc = (int)(ftell(msgfl) / 1024L) / (cfg.messagek / 64); */

        nowloc = (int)(ftell(msgfl) / neatoconstant);

        while (loc < nowloc)                              /* where in bar */
        {
             outCon('#');                                 /* fill bar */
             loc++;
        }
        outCon(twirl[(char)(here % 4)]);                          /* twirl a bit */
        PUTCON('\b');
#else
        showtypemsg(here);
#endif

        /* find highest and lowest message IDs: */
        /* >here< is the dip pholks             */
        if (here < cfg.oldest)
        {

#ifdef NEWMSGTAB
            slot = ( indexslot(cfg.newest) + 1 );
#else
            slot = ( (int)indexslot(cfg.newest) + 1 );
#endif

           /*  makeroom = (int)(cfg.mtoldest - here); */

            makeroom = (cfg.mtoldest - here);

            /* check to see if our table can hold  remaining msgs */
            if (cfg.nmessages < (makeroom + slot))
            {
                skipcounter = (makeroom + slot) - cfg.nmessages;

                slidemsgTab(makeroom - skipcounter);

                cfg.mtoldest = (here + (ulong)skipcounter);
 
            }
            /* nmessages can handle it.. Just make room */
            else
            {
                slidemsgTab(makeroom);
                cfg.mtoldest = here;
            }
            cfg.oldest = here;
        }

        /* if (here > cfg.newest) */

        if (here > cfg.newest)
        {
            cfg.newest = here;

            /* read rest of message in and remember where it ends,      */
            /* in case it turns out to be the last message              */
            /* in which case, that's where to start writing next message*/
            getMsgStr(msgBuf->mbtext, MAXTEXT); 

            cfg.catLoc = ftell(msgfl);
        }


        /* check to see if our table is big enough to handle it */

#ifdef NEWMSGTAB
        if ( (here - cfg.mtoldest) >= cfg.nmessages)
#else
        if ( (int)(here - cfg.mtoldest) >= cfg.nmessages)
#endif
        {
            crunchmsgTab(1);
        }

        if (skipcounter) 
        {
            skipcounter--;
        }
        else
        {
            indexmessage(here);
        }
    }
#if 1
    while (loc < 64)
    {
        outCon('#');
        loc++;
    }
#endif
    
    doccr();
    cPrintf("%s messages, #%lu to %lu.",
    ltoac(cfg.newest - cfg.oldest + 1), cfg.oldest, cfg.newest); 

}             

/* -------------------------------------------------------------------- */
/*  zapMsgFl()  initializes message.buf                                 */
/* -------------------------------------------------------------------- */
zapMsgFile()
{
    int i;
    long sect = 0;
    char buff[1024];

    /* Clear it out just in case */
    for (i = 0;  i < 1024;  i++) buff[i] = 0;

    /* put smiley message in first sector... */

    buff[0]    = 0xFF; /*                                   */
    buff[1]    = DUMP; /*  \  To the dump                   */
    buff[2]    = '\0'; /*  /  Attribute                     */
    buff[3]    =  '1'; /*  >                                */
    buff[4]    = '\0'; /*  \  Message ID "1" MS-DOS style   */
    buff[5]    = 'A';
    buff[6]    = 'C';
    buff[7]    = 'i';
    buff[8]    = 't';
    buff[9]    = 'a';
    buff[10]   = 'd';
    buff[11]   = 'e';
    buff[12]   = 'l';
    buff[13]   = '\0'; 
    buff[14]   = 'D';
    buff[15]   = '0';
    buff[16]   = '\0'; 
    buff[17]   = 'R';
    buff[18]   = 'D';
    buff[19]   = 'u';
    buff[20]   = 'm';
    buff[21]   = 'p';
    buff[22]   = '\0'; 
    buff[23]   =  'M'; 
    buff[24]   =  ':';
    buff[25]   =  '-';
    buff[26]   =  ')';
    buff[27]   = '\0'; 
                                                  
    cfg.newest = cfg.oldest = 1l;

    /* cfg.catLoc = 7l; */

    if (fwrite(buff, 1024, 1, msgfl) != 1)
    {
        cPrintf("zapMsgFil: write failed"); doccr();
    }

    for (i = 0;  i < 100;  i++) buff[i] = 0;

    doccr();  doccr();
    cPrintf("MESSAGEK=%lu", cfg.messagek);  doccr();
    cPrintf("Clearing block %lu\r", sect);

    for (sect = 1;  sect < cfg.messagek;  sect++)
    {
        if (!(sect % 10))
            cPrintf("Clearing block %u\r", sect);
        if (fwrite(buff, 1024, 1, msgfl) != 1)
        {
            cPrintf("zapMsgFil: write failed");  doccr();
        }
    }
    return TRUE;
}


#ifdef NEWMSGTAB
/************************************************************************/
/*      slidemsgTab() frees >howmany< slots at the beginning of the     */
/*      message table.                                                  */
/* -------------------------------------------------------------------- */
/* this is way boo boo slow, but as it is only called once, and that is */
/* when building the message table, we can ignore the slowness in favor */
/* of program size; even if the function was just a return, building    */
/* the message table would still take forever...                        */
/************************************************************************/
void slidemsgTab(ulong howmany)
{
    ulong i;

    for (i = cfg.nmessages - 1; i >= howmany; i--)
    {
        memcpy (getMsgTab(i), getMsgTab(i - howmany), sizeof(struct messagetable));
    }
}
#else
/************************************************************************/
/*      slidemsgTab() frees >howmany< slots at the beginning of the     */
/*      message table.                                                  */
/************************************************************************/
void slidemsgTab(ulong how_many)
{
    uint numnuked;

    int howmany = (int)how_many;
   
    numnuked = cfg.nmessages - howmany;
   
    _fmemmove(&msgTab_mtmsgflags[howmany], msgTab_mtmsgflags,
            (uint)(numnuked * (sizeof(*msgTab_mtmsgflags)) ));

    _fmemmove(&msgTab_mtmsgLocLO[howmany], msgTab_mtmsgLocLO,
            (uint)(numnuked * (sizeof(*msgTab_mtmsgLocLO)) ));

    _fmemmove(&msgTab_mtmsgLocHI[howmany], msgTab_mtmsgLocHI,
            (uint)(numnuked * (sizeof(*msgTab_mtmsgLocHI)) ));

    _fmemmove(&msgTab_mtroomno[howmany], msgTab_mtroomno,
            (uint)(numnuked * (sizeof(*msgTab_mtroomno)) ));

    _fmemmove(&msgTab_mttohash[howmany], msgTab_mttohash,
            (uint)(numnuked * (sizeof(*msgTab_mttohash)) ));

    _fmemmove(&msgTab_mtauthhash[howmany], msgTab_mtauthhash,
            (uint)(numnuked * (sizeof(*msgTab_mtauthhash)) ));

    _fmemmove(&msgTab_mtomesg[howmany], msgTab_mtomesg,
            (uint)(numnuked * (sizeof(*msgTab_mtomesg)) ));

}
#endif
