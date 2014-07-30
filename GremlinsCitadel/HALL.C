/************************************************************************/
/*                               hall.c                                 */
/*               hall code for Citadel bulletin board system            */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      accesshall()            returns TRUE if person can access hall  */
/*      defaulthall()           handles .ed  (enter Default-hallway)    */
/*      enterhall()             handles .eh                             */
/*      gotodefaulthall()       goes to user's default hallway          */
/*      hallexists()            returns # of named hall,  else ERROR    */
/*      iswindow()              for .kw .kvw is # room a window         */
/*      knownhalls()            handles .kh, .kvh                       */
/*      partialhall()           returns slot of partially named hall    */
/*      readhalls()             handles .rh, .rvh                       */
/*      roominhall()            indicates if room# is in hall           */
/*      stephall()              handles previous, next hall             */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*    accesshall() returns true if hall can be accessed                 */
/*    from current room                                                 */
/************************************************************************/
int accesshall(int slot)
{
    int accessible = 0;

    if  ( 
          (slot == (int)thisHall) 
          || 
          ( 
                hallBuf->hall[slot].h_inuse  
#ifdef GOODBYE
             && (hallBuf->hall[slot].hroomflags[thisRoom].window || cfg.floors)
#endif
             && (bit_test(hallBuf->hall[slot].window, thisRoom) || cfg.floors)

             && groupseeshall(slot)
          )
        )
    {
        accessible = TRUE;
    }
    return accessible;
}

/************************************************************************/
/*      enterhall()  handles .eh                                        */
/************************************************************************/
void enterhall(void)
{
    label hallname;
    int slot, accessible;

    getString("hall", hallname, NAMESIZE, FALSE, ECHO, "");

    slot = partialhall(hallname);

    if (slot != ERROR)  accessible = accesshall(slot);

    if ( (slot == ERROR) || !strlen(hallname) || !accessible )
    {
        mPrintf(" No such hall.");
        return;
    }
    else 
    {
        thisHall = (unsigned char)slot;
    }
}

/************************************************************************/
/*      gotodefaulthall()  goes to user's default hallway               */
/************************************************************************/
void gotodefaulthall(void)
{
    int i;

    if (logBuf.hallhash)
    {
        for (i = 1; i < MAXHALLS; ++i)
        {
            if ( (int)hash( hallBuf->hall[i].hallname )  == logBuf.hallhash 
               && hallBuf->hall[i].h_inuse)
            {
                if (groupseeshall(i))  thisHall = (unsigned char)i;
            }
        }
    }
}

/************************************************************************/
/*      hallexists()  return # of named hall, else ERROR                */
/************************************************************************/
int hallexists(char *hallname)
{
    int i;

    for (i = 0; i < MAXHALLS; i++)
    {
        if (hallBuf->hall[i].h_inuse &&
            strcmpi(hallname, hallBuf->hall[i].hallname) == SAMESTRING )
        return(i);
    }
    return(ERROR);
}

/************************************************************************/
/*      iswindow()  is room a window into accessible halls?             */
/************************************************************************/
int iswindow(int roomslot)
{
    int i, window = 0;

    if (!roomTab[roomslot].rtflags.INUSE)  return(FALSE);

    for (i = 0; i < MAXHALLS && !window ; i++)
    {
        if (hallBuf->hall[i].h_inuse &&

#ifdef GOODBYE
            hallBuf->hall[i].hroomflags[roomslot].window )
#endif
            bit_test(hallBuf->hall[i].window, roomslot)  )


        window = TRUE;
    }
    return(window);
}


/************************************************************************/
/*      knownhalls()  handles .kh .kvh                                  */
/************************************************************************/
void knownhalls(void)
{
    int i;

    doCR();

    mPrintf(" Hallways accessible:");

    doCR();

    prtList(LIST_START);
    
    for (i = 0; i < MAXHALLS; i++)
    {
        if (accesshall(i))
        {
            prtList(hallBuf->hall[i].hallname);
        }
    }
    
    prtList(LIST_END);
}

/************************************************************************/
/*      partialhall()  returns slot # of partial hall name, else error  */
/*      used for .Enter Hallway and .Enter Default-hallway  only!       */
/************************************************************************/
int partialhall(char *hallname)
{
    int i, length;

    i = hallexists(hallname);
    if (i != ERROR)
        return i;
        
    length = strlen(hallname);

    for (i = 0; i < MAXHALLS; i++)
    {
        if (hallBuf->hall[i].h_inuse)
        {
            if ((strnicmp(hallBuf->hall[i].hallname, hallname, length) == SAMESTRING)
                && groupseeshall(i))
            return(i);
        }
    }         
    
    for (i = 0; i < MAXHALLS; i++)
    {
        if (hallBuf->hall[i].h_inuse)
        {
            if ((substr(hallBuf->hall[i].hallname, hallname) != 0 /*NULL*/)
                && groupseeshall(i))
            return(i);
        }
    }         
    
    return(ERROR);
}

/************************************************************************/
/*      readhalls()  handles .rh .rvh                                   */
/************************************************************************/
void readhalls(void)
{
    int i;
    char str[LABELSIZE + LABELSIZE];

    doCR();
    doCR();

    mPrintf("Room %s is contained in:", makeRoomName(thisRoom, str));
    doCR();

    prtList(LIST_START);

    for (i = 0; i < MAXHALLS; i++)
    {
        if  ( hallBuf->hall[i].h_inuse
#ifdef GOODBYE
        && hallBuf->hall[i].hroomflags[thisRoom].inhall
#endif
        && bit_test(hallBuf->hall[i].inhall, thisRoom) 


        && groupseeshall(i))

        prtList(hallBuf->hall[i].hallname);
    }

    prtList(LIST_END);
}

/************************************************************************/
/*    roominhall()  returns TRUE if room# is in current hall            */
/************************************************************************/
int roominhall(int roomslot)
{
#ifdef GOODBYE
    if (   hallBuf->hall[thisHall].hroomflags[roomslot].inhall 
#endif

    if (   bit_test(hallBuf->hall[thisHall].inhall, roomslot) 


       ||  (roomslot == LOBBY && cfg.floors)
       )
        return(TRUE);

    return(FALSE);
}

/************************************************************************/
/*    stephall()  handles previous, next hall                           */
/************************************************************************/
void stephall(int direction)
{
    int i;
    char done = FALSE;

    i = thisHall;

    do
    {
        /* step */
        if (direction == 1)
        {
            ++i;
            if (i == MAXHALLS ) 
                i = 0; 
        } else {
            --i;
            if ( i == -1 )
                i = MAXHALLS - 1;
        }

        /* keep from looping endlessly */
        if (i == (int)thisHall)
        {
            mPrintf("%s ", hallBuf->hall[i].hallname);
            return;
        }

        if (hallBuf->hall[i].h_inuse)
        {
            /* is this room a window in hall we're checking */
#ifdef GOODBYE
            if  (hallBuf->hall[i].hroomflags[thisRoom].window || cfg.floors)
#endif

            if (bit_test(hallBuf->hall[i].window, thisRoom) || cfg.floors)

            {       
                /* can group see this hall */
                if (groupseeshall(i))
                {
                    mPrintf("%s ", hallBuf->hall[i].hallname);
                    thisHall = (unsigned char)i;
                    done = TRUE;
                }
            }
        }

    } while ( !done );

    if (hallBuf->hall[thisHall].described && logBuf.HALLTELL /* roomtell */)
    {
        if (changedir(cfg.roompath) == -1 ) return;

        if (checkfilename(hallBuf->hall[thisHall].htell, 0) == ERROR)
        {
            changedir(cfg.homepath);
            return;
        }

        doCR();
    
        if (!filexists(hallBuf->hall[thisHall].htell))
        {
            doCR();
            mPrintf("No hallway description %s", hallBuf->hall[thisHall].htell);
            changedir(cfg.homepath);
            doCR();
            return;
        }
    
        if (!expert)
        {
            doCR();
            mPrintf("<3J0>ump <3P0>ause <3S0>top");
            doCR();
        }
    
        /* print it out */
        dumpf(hallBuf->hall[thisHall].htell);
    
        /* go to our home-path */
        changedir(cfg.homepath);
    
        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);

    }
}


