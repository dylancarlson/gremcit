/* -------------------------------------------------------------------- */
/*  LOGEDIT.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                          Userlog edit code                           */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  userEdit()      Edit a user via menu                                */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  userEdit()      Edit a user via menu                                */
/* -------------------------------------------------------------------- */
void userEdit(void)
{
    BOOL    prtMess = TRUE;
    BOOL    quit    = FALSE;
    int     c;
    char    string[200];
    char    oldEcho;
    label   who, temp;
    int     logNo, tsys;
    BOOL    editSelf = FALSE;
    label   dHall;
    int     i;
    char    edited_pointers = FALSE;

    
    *dHall = '\0';
    
    getNormStr("who", who, NAMESIZE, ECHO);
    logNo    = findPerson(who, &lBuf);
    personexists(who);

    if ( !strlen(who) || logNo == ERROR)  
    {
        mPrintf("No \'%s\' known. \n ", who);
        return;
    }

    /* make sure we use curent info */
    if (strcmpi(who, logBuf.lbname) == SAMESTRING)
    {
        tsys = logBuf.lbflags.SYSOP;
        setlogconfig(); /* update curent user */
        logBuf.lbflags.SYSOP = tsys;
        lBuf = logBuf;  /* use their online logbuffer */
        editSelf = TRUE;
    }

    doCR();

    do 
    {
        if (prtMess)
        {
            if (lBuf.hallhash)
            {
                for (i = 1; i < MAXHALLS; ++i)
                {
                  if ( (int)hash( hallBuf->hall[i].hallname )  == lBuf.hallhash 
                       && hallBuf->hall[i].h_inuse)
                    {
                        strcpy(dHall, hallBuf->hall[i].hallname);
                    }
                }
            }
            else
            {
                strcpy(dHall, hallBuf->hall[0].hallname);
            }
            
            doCR();
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);

            mPrintf("<3N0> 3N0ame............. %s", lBuf.lbname);  doCR();
            mPrintf("<310> Title............ %s", lBuf.title);   doCR();
            mPrintf("<320> Surname.......... %s", lBuf.surname); doCR();
            mPrintf("<3L0> 3L0ock T & Surname. %s", 
                                            lBuf.SURNAMLOK ? "Yes" : "No");            
                                            doCR();
            mPrintf("<3Y0> S3y0sop............ %s", 
                                            lBuf.lbflags.SYSOP ? "Yes" : "No");
                                            doCR();
            mPrintf("<3D0> Ai3d0e............. %s", 
                                            lBuf.lbflags.AIDE ? "Yes" : "No");
                                            doCR();
            mPrintf("<3O0> N3o0de............. %s", 
                                            lBuf.lbflags.NODE ? "Yes" : "No");
                                            doCR();
            mPrintf("<3P0> 3P0ermanent........ %s", 
                                            lBuf.lbflags.PERMANENT ?"Yes":"No");
                                            doCR();
            mPrintf("<3E0> N3e0tuser.......... %s", 
                                            lBuf.lbflags.NETUSER ? "Yes" :"No");
                                            doCR();
            mPrintf("<3T0> 3T0witted.......... %s", 
                                            lBuf.lbflags.PROBLEM ? "Yes" :"No");  
                                            doCR();
            mPrintf("<3M0> 3M0ail............. %s", 
                                            lBuf.lbflags.NOMAIL ? "Off" : "On");
                                            doCR();
            mPrintf("<3V0> 3V0erified......... %s",
                                            !lBuf.VERIFIED ? "Yes" : "No");
                                            doCR();
            mPrintf("<3B0> 3B0orders.......... %s", 
                                            lBuf.BOARDERS ? "Yes" :"No");  
                                            doCR();
            mPrintf("<3H0> Default 3H0all..... %s %s",
                                            dHall,
                                            lBuf.LOCKHALL ? "3<Locked>0" : ""
                                            );
                                            doCR();
            mPrintf("<3U0> 3U0nlisted......... %s",
                                            lBuf.lbflags.UNLISTED ? "Yes" : "No");
                                            doCR();
            mPrintf("<330> Auto Next Hall... %s",
                                            lBuf.NEXTHALL ? "On" : "Off");
                                            doCR();
            mPrintf("<340> Psycho user...... %s",         
                                            lBuf.PSYCHO ? "Yes" : "No");
                                            doCR();
                                            
            if (cfg.accounting)
            {
                mPrintf("<3I0> T3i0me (minutes)... ");
                
                if (lBuf.lbflags.NOACCOUNT)
                    mPrintf("N/A");
                else
                    mPrintf("%.0f", lBuf.credits);
    
                doCR();
            }

#ifdef GOODBYE            
            mPrintf("<3R0> 3R0eset all new message pointers");  doCR();
#endif

            mPrintf("<3Z0> Pointer edit menu");  doCR();
            
            if (onConsole)
            {
                mPrintf("    Password.......... %s;%s", lBuf.lbin, lBuf.lbpw); 
                                                            doCR();
            }
            
            doCR();
            mPrintf("<3S0> to save, <3A0> to abort."); doCR();
            prtMess = (BOOL)(!expert);
        }

        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);

        doCR();
        mPrintf("2Change:0 ");
        
        oldEcho = echo;
        echo    = NEITHER;
        setio(whichIO, echo, outFlag);

        c       = iChar();
        echo    = oldEcho;
        setio(whichIO, echo, outFlag);

        if (!CARRIER)
            return;

        switch(toupper(c))
        {
        case 'N':
            mPrintf("Name"); doCR();
            strcpy(temp, lBuf.lbname);
            getString("New name", lBuf.lbname, NAMESIZE, FALSE, ECHO, temp);
            normalizeString(lBuf.lbname);
            
            if ( 
                   (
                      personexists(lBuf.lbname) != ERROR 
                   && strcmpi (lBuf.lbname, temp) != SAMESTRING
                   )
                   || (strcmpi(lBuf.lbname, "Sysop") == SAMESTRING)
                   || (strcmpi(lBuf.lbname, "Aide") == SAMESTRING)
                   || !strlen (lBuf.lbname) 
               )
            {
                strcpy(lBuf.lbname, temp);
            }
            break;

        case '1':
            mPrintf("Title"); doCR();
            if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf)
            {
                doCR();
                mPrintf("User has locked thier title and surname!"); doCR();
            }
            else 
            {
                strcpy(temp, lBuf.title);
                getString("new title", lBuf.title, NAMESIZE, FALSE, ECHO, temp);
                if (!strlen(lBuf.title))
                {
                    strcpy(lBuf.title, temp);
                }
                normalizeString(lBuf.title);
            }
            break;
        
        case '2':
            mPrintf("Surname"); doCR();
            if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf)
            {
                doCR();
                mPrintf("User has locked thier title and surname!"); doCR();
            }
            else 
            {
                strcpy(temp, lBuf.surname);
                getString("New surname", lBuf.surname, NAMESIZE, FALSE, ECHO, temp);
                if (!strlen(lBuf.surname))
                {
                    strcpy(lBuf.surname, temp);
                }
                normalizeString(lBuf.surname);
            }
            break;

        case 'L':
            if (lBuf.lbflags.SYSOP && lBuf.SURNAMLOK && !editSelf)
            {
                mPrintf("Lock Title and Surname.");  doCR();
                doCR();
                mPrintf("You can not change that!"); doCR();
            }
            else
            {
                lBuf.SURNAMLOK = (BOOL)(!lBuf.SURNAMLOK);
                mPrintf("Lock Title and Surname: %s", 
                                                lBuf.SURNAMLOK ? "On" : "Off");
                                                doCR();
            }
            break;

        case 'Y':
            lBuf.lbflags.SYSOP = (BOOL)(!lBuf.lbflags.SYSOP);
            mPrintf("Sysop %s", lBuf.lbflags.SYSOP ? "Yes" : "No");  doCR();
            break;
        
        case 'B':
            lBuf.BOARDERS = (BOOL)(!lBuf.BOARDERS);
            mPrintf("Borders %s", lBuf.BOARDERS ? "Yes" : "No");  doCR();
            break;
        
        case 'U':
            lBuf.lbflags.UNLISTED = (BOOL)(!lBuf.lbflags.UNLISTED);
            mPrintf("Unlisted %s", lBuf.lbflags.UNLISTED ? "Yes" : "No");  doCR();
            break;
        
        case '3':
            lBuf.NEXTHALL = (BOOL)(!lBuf.NEXTHALL);
            mPrintf("Auto Next Hall %s", lBuf.NEXTHALL ? "On" : "Off");  
            doCR();
            break;
        
        case '4':
            lBuf.PSYCHO = (BOOL)(!lBuf.PSYCHO);
            mPrintf("Psycho %s", lBuf.PSYCHO ? "On" : "Off");  
            doCR();
            break;

        case 'D':
            lBuf.lbflags.AIDE = (BOOL)(!lBuf.lbflags.AIDE);
            mPrintf("Aide %s", lBuf.lbflags.AIDE ? "Yes" : "No");  doCR();
            break;

        case 'O':
            lBuf.lbflags.NODE = (BOOL)(!lBuf.lbflags.NODE);
            mPrintf("Node %s", lBuf.lbflags.NODE ? "Yes" : "No");  doCR();
            break;

        case 'P':
            lBuf.lbflags.PERMANENT = (BOOL)(!lBuf.lbflags.PERMANENT);
            mPrintf("Permanent %s", lBuf.lbflags.PERMANENT ? "Yes" : "No");  
                doCR();
            break;
 
        case 'E':
            lBuf.lbflags.NETUSER = (BOOL)(!lBuf.lbflags.NETUSER);
            mPrintf("Netuser %s", lBuf.lbflags.NETUSER ? "Yes" : "No");  
                doCR();
            break;

        case 'T':
            lBuf.lbflags.PROBLEM = (BOOL)(!lBuf.lbflags.PROBLEM);
            mPrintf("Twit/Problem user %s", lBuf.lbflags.PROBLEM ? "Yes" : "No");  
                doCR();
            break;

        case 'M':
            lBuf.lbflags.NOMAIL = (BOOL)(!lBuf.lbflags.NOMAIL);
            mPrintf("Mail %s", lBuf.lbflags.NOMAIL ? "Off" : "On");  
                doCR();
            break;

        case 'V':
            lBuf.VERIFIED = (BOOL)(!lBuf.VERIFIED);
            mPrintf("Verified %s", !lBuf.VERIFIED ? "Yes" : "No");  
                doCR();
            break;
        
        case 'I':
            mPrintf("Minutes"); doCR();
            if (cfg.accounting)
            {
                lBuf.lbflags.NOACCOUNT = 
                    getYesNo("Disable user's accounting", 
                        (BOOL)lBuf.lbflags.NOACCOUNT);
    
                if (!lBuf.lbflags.NOACCOUNT)
                {
                    lBuf.credits = (float)
                        getNumber("minutes in account", (long)0,
                        (long)cfg.maxbalance, (long)lBuf.credits);
                }
            }
            else 
            {
                doCR();
                mPrintf("Accounting turned off for system.");
            }
            break;

#ifdef GOODBYE
        case 'R':
            mPrintf("Reset all new message pointers"); doCR();
            for (c=0; c < MAXVISIT; c++)
            {
                lBuf.lbvisit[c] = cfg.oldest;
            }
            break;
#endif

#ifdef GOODBYE
        case 'R':
            mPrintf("Reset all new message pointers"); doCR();
            for (c=0; c < MAXROOMS; c++)
            {
                lBuf.newpointer[c] = cfg.oldest;
            }
            break;
#endif

        case 'Z':
            userPointers();
            edited_pointers = TRUE;
            break;
            
        case 'S':
            mPrintf("Save"); doCR();
            if (getYesNo("Save changes", 0))
            {
                quit = TRUE;
            }
            break;

        case 'A':
            mPrintf("Abort"); doCR();
            if (getYesNo("Abort changes", 1))
            {
                return;
            }
            break;

        case '\r':
        case '\n':
        case '?':
            mPrintf("Menu"); doCR();
            prtMess = TRUE;
            break;

        case 'H':
            mPrintf("Default Hallway"); doCR();
        lBuf.LOCKHALL = getYesNo("Lock default hallway", (uchar)lBuf.LOCKHALL);
            break;
            
        default:
            mPrintf("%c ? for help", c); doCR();
            break;
        }

    } while (!quit);

    /* trap it */
    sprintf(string, "%s has:", who);
    if (lBuf.lbflags.SYSOP)     strcat(string, " Sysop:");
    if (lBuf.lbflags.AIDE)      strcat(string, " Aide:");
    if (lBuf.lbflags.NODE)      strcat(string, " Node:");
    if (cfg.accounting)
    {
        if (lBuf.lbflags.NOACCOUNT)
        {
            strcat(string, " No Accounting:");
        }
        else
        {
            sprintf(temp, " %.0f minutes:", lBuf.credits);
            strcat(string, temp);
        }
    }

    if (lBuf.lbflags.PERMANENT) strcat(string, " Permanent:");
    if (lBuf.lbflags.NETUSER)   strcat(string, " Net User:");
    if (lBuf.lbflags.PROBLEM)   strcat(string, " Problem User:");
    if (lBuf.lbflags.NOMAIL)    strcat(string, " No Mail:");
    if (lBuf.VERIFIED)          strcat(string, " Un-Verified:");
    
    trap(string, T_SYSOP);

    /* see if it is us: */
    if (loggedIn  &&  editSelf)
    {
        /* move it back */
        logBuf = lBuf;

        if (edited_pointers)
            roomtalley();

        /* make our environment match */
        setsysconfig();
    }
            
    putLog(&lBuf, logNo);
}


#ifdef GOODBYE

/* -------------------------------------------------------------------- */
/*  POINTER.C                 Frog Citadel                              */
/* -------------------------------------------------------------------- */
/*                   Overlayed userlog edit sub-menu                    */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Includes                                                            */
/* -------------------------------------------------------------------- */
#include <string.h>
#include <time.h>
#include "ctdl.h"
#include "proto.h"
#include "global.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/* >userPointers()  Manually reset userlog pointers. submenu from (.SU) */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  12/29/90  {zm}    new file, userPointers() moved here from LOG3.C   */
/*  12/31/90  {zm}    full-blown sub-menu.                              */
/*  02/01/91  {zm}    add menu item for scrolling pointers forward.     */
/*  02/07/91  {zm}    fix the loop for the above; clarify text.         */
/*  05/31/91  {zm}    fix display of pointers to use unsigned.          */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */
#endif

/* -------------------------------------------------------------------- */
/*  userPointers()  Manually reset userlog pointers.                    */
/* -------------------------------------------------------------------- */
void userPointers(void)
{
    BOOL menu = TRUE;        /* display menu if non-expert or on demand */
    int i, c;                /* i for looping, c for input of command   */
    ulong scratch[MAXROOMS]; /* store pointers in case of restoration   */

    label roomname;
    int roomslot;

    ulong msgnumber;
    char string[100];

    mPrintf("New Pointer edit menu.");
    doCR();

    for (i = 0; i < MAXROOMS; i++)
    {
        scratch[i] = lBuf.newpointer[i];
    }

  /*  while (TRUE) */
    for(;;)
    {
        outFlag = OUTOK;
        setio(whichIO, echo, outFlag);

        if (menu)
        {
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);

            doCR();

            mPrintf("<3S0> Set all pointers."); doCR();
            mPrintf("<3N0> Set all pointers to New."); doCR();
            mPrintf("<3O0> Set all pointers to Old."); doCR();


#ifdef GOODBYE
            mPrintf("<3B0> Roll pointers 3b0ack one call."); doCR();
            mPrintf("<3F0> Roll pointers 3f0orward one call."); doCR();

            mPrintf("<3R0> Restore pointers to original value."); doCR();
#endif

            mPrintf("<3R0> Set pointer for a room."); doCR();

            mPrintf("<3D0> Display all pointers."); doCR();

            mPrintf("<3A0> Abort to previous menu."); doCR();
            menu = (BOOL)!expert;
        }

        /* display current pointers */
        doCR();

        mPrintf("%s online, #%lu to %lu",
        ltoac(cfg.newest - cfg.oldest + 1), cfg.oldest, cfg.newest); 

#ifdef GOODBYE
        mPrintf("current oldest (in message base) = %lu, newest = %lu",
                cfg.oldest, cfg.newest);
#endif

        doCR();  /* doCR(); */

#ifdef GOODBYE
        for (i = 0; i < MAXROOMS; i++)
        {
            mPrintf(" [%4d] %7ld  ", i, lBuf.newpointer[i]);
            if (!((i+1) % 4))
                doCR();
        }
#endif

        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);
        doCR();
        mPrintf("2Pointer edit cmd:0 ");

        i = echo;
        echo = NEITHER;
        setio(whichIO, echo, outFlag);

        c = iChar();
        echo = (char)i;
        setio(whichIO, echo, outFlag);

        if (!(onConsole || gotCarrier()))
            return;

        switch (toupper(c))
        {


        case 'R':
            mPrintf("Set pointer for particular room"); doCR();

            getString("room name", roomname, NAMESIZE, FALSE, ECHO, "");

            roomslot = roomExists(roomname);

            if (roomslot == ERROR) roomslot = partialExist_ignorehalls(roomname);

            if ((roomslot) == ERROR || !strlen(roomname) )
            {
                mPrintf("\n No %s room", roomname);
                break;
            }

            strcpy(string, "New pointer for '");
            strcat(string, roomTab[roomslot].rtname);
            strcat(string, "'");

            msgnumber = (ulong)getNumber(string,
                              (long)cfg.oldest, (long)cfg.newest, lBuf.newpointer[roomslot]);

            lBuf.newpointer[roomslot] = msgnumber;

            doCR();
            break;

        case 'D':
            mPrintf("Display all pointers."); doCR();

            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);
            doCR();

            for (i = 0; i < MAXROOMS; i++)
            {
                if (roomTab[i].rtflags.INUSE)
                {
                    mPrintf("%31s -> %ld", deansi(roomTab[i].rtname), lBuf.newpointer[i]);
                    doCR();
                }
            }
            break;

        case 'N':
            mPrintf("All messages are New"); doCR();
            for (i = 0; i < MAXROOMS; i++)
            {
                lBuf.newpointer[i] = cfg.oldest;
            }
            break;

        case 'O':
            mPrintf("All messages are Old"); doCR();
            for (i = 0; i < MAXROOMS; i++)
            {
                lBuf.newpointer[i] = cfg.newest;
            }
            break;

#ifdef GOODBYE
        case 'B':
            mPrintf("Roll back pointers"); doCR();
            for (i = 0; i < MAXVISIT-1; i++)
            {
                lBuf.lbvisit[i] = lBuf.lbvisit[i+1];
            }
            break;

        case 'F':
            mPrintf("Roll pointers forward"); doCR();
            for (i = MAXVISIT-1; i > 0; i--)
            {
                lBuf.lbvisit[i] = lBuf.lbvisit[i-1];
            }
            break;
#endif

        case 'S':
            mPrintf("Set all pointers"); doCR();
        
            msgnumber = getNumber("Message number",
                (long)0, (long)10000000L, -1l);

            if (!msgnumber) break;

            if ( (msgnumber < cfg.mtoldest) ||
                 (msgnumber > cfg.newest  ))
            {
                mPrintf("Value out of range.");
                doCR();
                break;
            }

#ifdef GOODBYE
            msgnumber = (ulong)getNumber("Message number",
                              (long)cfg.oldest, (long)cfg.newest, -1L);

#endif
            for (i = 0; i < MAXROOMS; i++)
            {
                lBuf.newpointer[i] = msgnumber;
            }

            doCR();
            break;

#ifdef GOODBYE
        case 'R':
            mPrintf("Restore original pointers"); doCR();
            for (i = 0; i < MAXROOMS; i++)
            {
                lBuf.newpointer[i] = scratch[i];
            }
            break;
#endif

        case 'A':
        case 'Q':
            mPrintf("Abort to previous menu.");
            doCR();
            return;

        default:
            mPrintf("Menu"); doCR();
            menu = TRUE;
            break;
        }
    }   /* infinite loop, */
}

/* EOF */

