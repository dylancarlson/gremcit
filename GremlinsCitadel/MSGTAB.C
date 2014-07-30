/* -------------------------------------------------------------------- */
/*  MSGTAB.C                      ACit                         92May05  */
/*          This is the code to deal with the message table             */
/* -------------------------------------------------------------------- */

#ifdef NEWMSGTAB
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/* this swapping code exists so really huge message tables can exist in */
/* DOS's 640K. it will swap unused portions of the message table (in    */
/* 16K chunks) to XMS, EMS, or the hard disk (in that order).           */
/*                                                                      */
/* the concept is quite simple. the execution is just a little more     */
/* difficult.  :)                                                       */
/*                                                                      */
/* we keep a linked list that tells us where each 16K chunk of the      */
/* message table is located. it is called mtList. when we need to find  */
/* a message, we find where it is in this list. if it is in the heap,   */
/* great; we can get right to it and all is fine. if, however, it is in */
/* xms, ems, or on disk, we have to swap a chunk off the heap into      */
/* auxiliary memory, then copy the desired chunk into the heap so we    */
/* can use it. this is the fun part.                                    */
/*                                                                      */
/* the mtList is kept so that whenever a chunk in heap is used, it is   */
/* moved to the start of the list. thus, the oldest chunk is kept at    */
/* the end. when it is time to swap a chunk out of heap, we look for    */
/* the last chunk in heap in the list to toss out.                      */
/*                                                                      */
/* the rest of citadel can only access the message table through these  */
/* routines. this will be a bit slower overall, but the message table   */
/* only gets a good thrashing when incorporating netmessages, when      */
/* there is nobody on line, anyway.  so, the overall performance hit    */
/* can be pretty much ignored without worry.                            */
/*                                                                      */
/* this code, of course, is not at all portable to any other operating  */
/* system. i wrote it with borland c++ 3.0, but it should compile with  */
/* msc.                                                                 */
/*                                                                      */
/* BiB/92Apr21                                                          */
/* -------------------------------------------------------------------- */

struct msgTabList *mtList;          /* base of our linked list          */
struct msgTabList *freeChunk;       /* the swap chunk                   */

uint chunksInHeap;                  /* how much we have alloaceted      */

/* -------------------------------------------------------------------- */
/*                              contents                                */
/*                                                                      */
/* addMtChunk()             adds another chunk to the mt                */
/* addToHeap()              tries to allocate heap for more mt          */
/* createMsgTab()           does what it says                           */
/* destroyMsgTab()          does just that                              */
/* bringToFront()           moves a mtList entry to the front           */
/* lastMtList()             finds last node of our linked list          */
/* swapOutOldest()          puts the oldest chunk in Heap to aux        */
/* getMsgTab()              gets slot in heap and gives addr            */
/* getFlags()               returns flags for msg#          (1)         */
/* getLocation()            returns location for msg#       (2)         */
/* getOffset()              returns offset for msg#         (3)         */
/* getRoomNum()             returns room# for msg#          (4)         */
/* getToHash()              returns hash of to of msg#      (5)         */
/* getAuthHash()            returns hash of auth of msg#    (6)         */
/* getFwdHash()             returns hash of fwdee of msg#   (7)         */
/* getOriginID()            returns origin id for msg#      (8)         */
/* crunchmsgTab()           obliterates slots at the beginning of table */
/* -------------------------------------------------------------------- */

void *addToHeap(void);
int addMtChunk(ulong startat);
struct msgTabList *lastMtList(void);
void bringToFont(struct msgTabList *theone);
void swapOutOldest(void);
void bringToFront(struct msgTabList *theone);

void destroyMsgTab(void)
    {
    struct msgTabList *lml, *lml2;
    lml = mtList;
    while (lml)
        {
        if (lml->whatMem == mtHEAP)
            {
            _ffree(lml->where);
            }
        lml2 = lml;
        lml = lml->next;
        _ffree(lml2);
        }
    disposeAuxMem(cfg.vmemfile);
    chunksInHeap = 0;
    mtList = NULL;
    _ffree(freeChunk);
    freeChunk = NULL;
    }

void createMsgTab(void)
    {
    ulong i;
    struct msgTabList *tmp;

    setUpAuxMem(cfg.vmemfile);

    chkptr(mtList);

    for (i = 0; i < cfg.nmessages; i += MSGtABpERbLK)
        {
        if (!addMtChunk(i))
            crashout("Error allocating msgTab\n");
        }

    /* add one more chunk for swapping */
    tmp = mtList;
    mtList = NULL;
    if (!addMtChunk(0))
        crashout("Error allocating msgTab\n");
    freeChunk = mtList;
    mtList = tmp;
    }

int addMtChunk(ulong startat)
    {
    struct msgTabList *mtLocal;
    ulong where;

    if ((mtLocal = malloc(sizeof(*mtLocal))) == NULL) return(FALSE);

    if (mtList == NULL)
        {
        mtList = mtLocal;
        }
    else
        {
        lastMtList()->next = mtLocal;
        }

    mtLocal->firstMsg = startat;
    mtLocal->next     = NULL;

    if ((where = (ulong) addToHeap()) != 0)
        {
        mtLocal->whatMem = mtHEAP;
        mtLocal->where = (struct messagetable *) where;
        }
    else if ((where = (ulong) addToXMS()) != 0)
        {
        mtLocal->whatMem = mtXMS;
        mtLocal->where = (struct messagetable *) where;
        }
    else if ((where = (ulong) addToEMS()) != 0)
        {
        mtLocal->whatMem = mtEMS;
        mtLocal->where = (struct messagetable *) where;
        }
    else if ((where = (ulong) addToVirt()) != 0)
        {
        mtLocal->whatMem = mtVIRT;
        mtLocal->where = (struct messagetable *) where;
        }
    else
        {
        return (FALSE);
        }
    return (TRUE);
    }

void *addToHeap(void)
    {
    if (++chunksInHeap <= cfg.msgHeapPages)
        {
        return ((void *)_fcalloc(1, 16384));
        }
    else
        {
        chunksInHeap--;
        return (NULL);
        }
    }

struct msgTabList *lastMtList(void)
    {
    struct msgTabList *lml;

    lml = mtList;
    if (lml == NULL) return (NULL);

    while (lml->next != NULL) lml = lml->next;
    return (lml);
    }

#ifdef __BORLANDC__
#pragma warn -rvl   /* borland c++ 'function should return a value' */
#endif
struct messagetable *getMsgTab(ulong slot)
    {
    struct msgTabList *lml, tmp;

    for(lml = mtList; lml != NULL; lml = lml->next)
        {
        if (lml->firstMsg <= slot && slot < (ulong)lml->firstMsg + (ulong)MSGtABpERbLK)
            {
            /* our message table slot is in this block */
            if (lml->whatMem != mtHEAP)
                {
                swapOutOldest();
                switch (lml->whatMem)
                    {
                    case mtXMS:
                        {
                        if (!XMStoHeap(freeChunk->where, FP_OFF(lml->where)))
                            crashout("XMS->heap error\n");
                        break;
                        }
                    case mtEMS:
                        {
                        if (!EMStoHeap(freeChunk->where, FP_OFF(lml->where)))
                            crashout("EMS->heap error\n");
                        break;
                        }
                    case mtVIRT:
                        {
                        if (!VirttoHeap(freeChunk->where, FP_OFF(lml->where)))
                            crashout("Virt->heap error\n");
                        break;
                        }
                    }
                tmp.whatMem = freeChunk->whatMem;
                tmp.where   = freeChunk->where;

                freeChunk->whatMem = lml->whatMem;
                freeChunk->where   = lml->where;


                lml->whatMem = tmp.whatMem;
                lml->where   = tmp.where;
                }
            bringToFront(lml);
            return (&(mtList->where[slot - mtList->firstMsg]));
            }
        }

/*****************************/

        sprintf(msgBuf->mbtext, "bad msgTab request! slot: %lu msg: %lu",
            slot, cfg.mtoldest + slot);

        trap(msgBuf->mbtext, T_ERROR); 

/*****************************/

/*  Don't Exit */
/*  crashout("bad msgTab request!\n");  */


    }
#ifdef __BORLANDC__
#pragma warn +rvl   /* borland c++ 'function should return a value' */
#endif

void bringToFront(struct msgTabList *theone)
    {
    struct msgTabList *lml;

    if (theone == mtList) return;

    lml = mtList;
    while (lml != NULL && lml->next != theone) lml = lml->next;
    if (!lml) crashout("lost msgTab chunk!\n");

    lml->next = theone->next;
    theone->next = mtList;
    mtList = theone;
    }

void swapOutOldest(void)
    {
    struct msgTabList *lml, tmp;
    char localBuf[80];
    int i;

    if (!mtList) crashout("No msgTab!\n");
    if (freeChunk->whatMem == mtHEAP) crashout("msgTab corruption\n");

    lml = mtList;
    while (lml->next->whatMem == mtHEAP) lml = lml->next;

    /* lml is last heap block - get rid of it. */
    switch (freeChunk->whatMem)
        {
        case mtXMS:
            {
            if (!HeapToXMS(FP_OFF(freeChunk->where), lml->where))
                crashout("Heap->XMS error\n");
            break;
            }
        case mtEMS:
            {
            if ((i = HeapToEMS(FP_OFF(freeChunk->where), lml->where)) != 0)
                {
                sprintf(localBuf, "Heap->EMS error #%d\n", i);
                crashout(localBuf);
                }
            break;
            }
        case mtVIRT:
            {
            if (!HeapToVirt(FP_OFF(freeChunk->where), lml->where))
                crashout("Heap->Virtual error\n");
            break;
            }
        }
    tmp.whatMem = freeChunk->whatMem;
    tmp.where   = freeChunk->where;

    freeChunk->whatMem = lml->whatMem;  /* free chunk now where lml was */
    freeChunk->where   = lml->where;

    lml->whatMem     = tmp.whatMem;     /* lml now what was free */
    lml->where   = tmp.where;
    }

struct msgflags *getFlags(ulong slot)
    {
    return(&(getMsgTab(slot)->mtmsgflags));
    }

ulong getLocation(ulong slot)
    {
    return(getMsgTab(slot)->mtmsgLoc);
    }

#ifdef GOODBYE
ulong getOffset(ulong slot)
    {
    return(getMsgTab(slot)->mtoffset);
    }
#endif

int getRoomNum(ulong slot)
    {
    return(getMsgTab(slot)->mtroomno);
    }

int getToHash(ulong slot)
    {
    return(getMsgTab(slot)->mttohash);
    }

int getAuthHash(ulong slot)
    {
    return(getMsgTab(slot)->mtauthhash);
    }

#ifdef GOODBYE
int getFwdHash(ulong slot)
    {
    return(getMsgTab(slot)->mtfwdhash);
    }
#endif

uint getOriginID(ulong slot)
    {
    return(getMsgTab(slot)->mtomesg);
    }

/* -------------------------------------------------------------------- */
/*  crunchmsgTab()  obliterates slots at the beginning of table         */
/* -------------------------------------------------------------------- */
void crunchmsgTab(ulong howmany)
{
    ulong i;    /* for msgtab index */
    uint room;
    ulong total = cfg.nmessages - howmany;
    ulong total2 = sizetable() - howmany;

    while (howmany > MSGtABpERbLK)
        {
        crunchmsgTab(MSGtABpERbLK);
        howmany -= MSGtABpERbLK;
        }

    for (i = 0; i < howmany; ++i)
    {
        room = getRoomNum(i);

        talleyBuf->room[room].total--;

        if (mayseeindexmsg(i))
        {
            talleyBuf->room[room].messages--;

            if  ((cfg.mtoldest + i) >
                /* logBuf.lbvisit[logBuf.lbroom[room].lvisit]) */
                logBuf.newpointer[room]) 
                talleyBuf->room[room].new--;
        }
    }

    for(i = 0; i < total2; i += MSGtABpERbLK)
        {
        memmove(getMsgTab(i), getMsgTab(i + howmany), sizeof(struct messagetable) * (MSGtABpERbLK - howmany));
        if (i + MSGtABpERbLK < total)
            {
            memcpy(getMsgTab(i + MSGtABpERbLK - howmany), getMsgTab(i + MSGtABpERbLK), sizeof(struct messagetable) * howmany);
            }
        }

    cfg.mtoldest += howmany;
}

#endif
