/************************************************************************/
/*                             roomedit.c                               */
/*              room code for Citadel bulletin board system             */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      renameRoom()            sysop special to rename rooms           */
/************************************************************************/
static int directory_l(char *str);

/* ------------------------------------------------------------------------ */
/*  directory_l()   returns wether a directory is locked                    */
/* ------------------------------------------------------------------------ */
static int directory_l(char *str)
{                          
    FILE *fBuf;
    char line[90];
    char *words[256];
    char path[80];

    sprintf(path, "%s\\external.cit", cfg.homepath);
    
    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {  
        crashout("Can't find route.cit!");
    }

    while (fgets(line, 90, fBuf) != NULL)
    {
        if (line[0] != '#') continue;
   
        if (strnicmp(line, "#DIRE", 5) != SAMESTRING) continue;
     
        parse_it( words, line);

        if (strcmpi(words[0], "#DIRECTORY") == SAMESTRING)
        {
            if (u_match(str, words[1]))
            {
                fclose(fBuf);
                return TRUE;
            }
        }
    }
    fclose(fBuf);
    return FALSE;
}

/************************************************************************/
/*      renameRoom() is sysop special fn                                */
/*      Returns:        TRUE on success else FALSE                      */
/************************************************************************/
void renameRoom(void)
{ 
    char    pathname[64];
    char    summary[500];
    label   roomname;
    label   oldname;
    label   groupname;
    char    line[80];
    char    waspublic;
    int     groupslot;
    char    description[13];
    int     roomslot;
    BOOL    prtMess = TRUE;
    BOOL    quit    = FALSE;
    int     c;
    char    oldEcho;
   
    strcpy(oldname,roomBuf.rbname);
    if (!roomBuf.rbflags.MSDOSDIR)
    {
        roomBuf.rbdirname[0] = '\0';
    }

    doCR();

    do 
    {
        if (prtMess)
        {
            doCR();
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);
            mPrintf("<3N0> Name.............. %s", roomBuf.rbname);   doCR();
            mPrintf("<3I0> Infoline.......... %s", roomBuf.descript); doCR();
            mPrintf("<3D0> Directory......... %s",
                             roomBuf.rbflags.MSDOSDIR
                             ? roomBuf.rbdirname : "None");             doCR();
            
            mPrintf("<3L0> Application....... %s",
                             roomBuf.rbflags.APLIC
                             ? roomBuf.rbaplic   : "None");             doCR();

            if (roomBuf.rbflags.APLIC)
            {
            mPrintf("<3O0> Auto Application.. %s", 
                             roomBuf.rbflags.AUTOAPP ? "Yes" : "No" );  doCR();
            }

            mPrintf("<3F0> Description File.. %s", 
                             roomBuf.rbroomtell[0]
                             ? roomBuf.rbroomtell : "None");            doCR();
            
            mPrintf("<3G0> Access Group...... %s", 
                             roomBuf.rbflags.GROUPONLY
                             ? grpBuf.group[roomBuf.rbgrpno].groupname
                             : "None");                                 doCR();
            
            mPrintf("<3V0> Privileges Group.. %s", 
                             roomBuf.rbflags.PRIVGRP
                             ? grpBuf.group[roomBuf.rbPgrpno].groupname
                             : "None");                                 doCR();
                             
            if (roomBuf.rbflags.PRIVGRP)
            {
                mPrintf("    Download only..... %s", 
                             roomBuf.rbflags.DOWNONLY ? "Yes" : "No" ); doCR();
                
                mPrintf("    Upload only....... %s", 
                             roomBuf.rbflags.UPONLY   ? "Yes" : "No" ); doCR();
                
                mPrintf("    Read only......... %s", 
                             roomBuf.rbflags.READONLY ? "Yes" : "No" ); doCR();
                
                mPrintf("    Group moderates... %s", 
                             roomBuf.rbflags.GRP_MOD  ? "Yes" : "No" ); doCR();
            }
            
            mPrintf("<3H0> Hidden............ %s", 
                             roomBuf.rbflags.PUBLIC ? "No" : "Yes" );   doCR();
            
            mPrintf("<3Y0> Anonymous......... %s", 
                             roomBuf.rbflags.ANON ? "Yes" : "No" );     doCR();

#ifdef GOODBYE            
            mPrintf("<3O0> BIO............... %s", 
                             roomBuf.rbflags.BIO ? "Yes" : "No" );      doCR();
#endif                                                

            mPrintf("<3M0> Moderated......... %s", 
                             roomBuf.rbflags.MODERATED ? "Yes" : "No" );doCR();
            
            mPrintf("<3E0> Networked/Shared.. %s", 
                             roomBuf.rbflags.SHARED ? "Yes" : "No" );   doCR();

            if (roomBuf.rbflags.SHARED)
            {
               mPrintf("<3W0> Network ID........ %s", roomBuf.netID); doCR();
            }

            
            mPrintf("<3P0> Permanent......... %s", 
                             roomBuf.rbflags.PERMROOM ? "Yes" : "No" ); doCR();
            
            mPrintf("<3U0> Subject........... %s", 
                             roomBuf.rbflags.SUBJECT ? "Yes" : "No" ); doCR();
            
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
        case 'L':
            mPrintf("Application"); doCR();
            
            if (sysop && onConsole)
            {
                if ( getYesNo("Application", (uchar)(roomBuf.rbflags.APLIC) ) )
                {
                    getString("Application filename", description, 13, FALSE,
                            ECHO, (roomBuf.rbaplic[0]) ? roomBuf.rbaplic : "");

                    strcpy(roomBuf.rbaplic, description);

                    roomBuf.rbflags.APLIC = TRUE;
                }
                else
                {
                    roomBuf.rbaplic[0] = '\0';
                    roomBuf.rbflags.APLIC = FALSE;
                }
            }
            else
            {
                mPrintf("Must be Sysop at console to enter application.");
                doCR();
            }
            break;

        case 'O':
           mPrintf("Auto Application room"); doCR();
           roomBuf.rbflags.AUTOAPP =
           getYesNo("Auto Application room", (uchar)(roomBuf.rbflags.AUTOAPP));
           break;
   
        case 'N':
            mPrintf("Name"); doCR();
            
            getString("New room name", roomname, NAMESIZE, FALSE, ECHO, 
                      roomBuf.rbname);
            normalizeString(roomname);
            roomslot = roomExists(roomname);
            if (roomslot >= 0  &&  roomslot != thisRoom)
            {
                mPrintf("A \"%s\" room already exists.\n", roomname);
            }
            else 
            {
                strcpy(roomBuf.rbname, roomname); /* also in room itself */
            }
            break;
    
        case 'I':
            mPrintf("Info-line \n ");
            getNormStr("New room Info-line", roomBuf.descript, 79, ECHO);
            break;

        case 'W':
            mPrintf("Network ID"); doCR();

            getString("New Network ID", roomname, 19, FALSE, ECHO, 
                      roomBuf.netID);

            normalizeString(roomname);

            roomslot = IdExists(roomname);

            if (roomslot != ERROR && (roomslot != thisRoom) && strlen(roomname))
            {
                mPrintf(" A \"%s\" Network ID already exists.\n", roomname);
            }
            else
            {
                strcpy(roomBuf.netID, roomname);
            }

            break;
    
        case 'D':
            mPrintf("Directory"); doCR();

            if (sysop)
            {
                if (getYesNo("Directory room", (uchar)roomBuf.rbflags.MSDOSDIR))
                {
                    roomBuf.rbflags.MSDOSDIR = TRUE;

                    if (!roomBuf.rbdirname[0])
                        mPrintf(" No drive and path");
                    else
                        mPrintf(" Now space %s",roomBuf.rbdirname);

                    doCR();
                    getString("Path", pathname, 63, FALSE, ECHO,
                     (roomBuf.rbdirname[0]) ? roomBuf.rbdirname : cfg.dirpath);
                    pathname[0] = (char)toupper(pathname[0]);

                    doCR();
                    mPrintf("Checking pathname \"%s\"", pathname);
                    doCR();
                    
                    if (directory_l(pathname) && !onConsole)
                    {
                        logBuf.VERIFIED = TRUE;

                        sprintf(msgBuf->mbtext, 
                                "Security violation on dirctory %s by %s\n "
                                "User unverified.", pathname, logBuf.lbname);
                        aideMessage();

                        doCR();
                        mPrintf("Security violation, your account is being "
                                "held for sysop's review"); 
                        doCR();
                        Hangup();

                        getRoom(thisRoom);
                        return;
                    }

                    if (changedir(pathname) != -1)
                    {
                        mPrintf(" Now space %s", pathname);
                        doCR();
                        strcpy(roomBuf.rbdirname, pathname);
                    }
                    else
                    {
                        mPrintf("%s does not exist! ", pathname);
                        if (getYesNo("Create", 0))
                        {
                            if (mkdir(pathname) == -1)
                            {
                                mPrintf("mkdir() ERROR!");
                                strcpy(roomBuf.rbdirname, cfg.temppath);
                            }
                            else
                            {
                                strcpy(roomBuf.rbdirname, pathname);
                                mPrintf(" Now space %s",roomBuf.rbdirname);
                                doCR();
                            }
                        }
                        else
                        {
                            strcpy(roomBuf.rbdirname, cfg.temppath);
                        }
                    }

                    if (roomBuf.rbflags.PRIVGRP && roomBuf.rbflags.MSDOSDIR)
                    {
                        roomBuf.rbflags.DOWNONLY =
                            getYesNo("Download only", 
                                    (uchar)roomBuf.rbflags.DOWNONLY);

                        if (!roomBuf.rbflags.DOWNONLY)
                        {
                            roomBuf.rbflags.UPONLY   =  getYesNo("Upload only", 
                                                 (uchar)roomBuf.rbflags.UPONLY);
                        }
                    }
                }
                else
                {
                    roomBuf.rbflags.MSDOSDIR = FALSE;
                    roomBuf.rbflags.DOWNONLY = FALSE;
                }
                changedir(cfg.homepath);
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make directories.");
                doCR();
            }
            break;
    
        case 'F':
            mPrintf("Description File"); doCR();

            if (cfg.roomtell && sysop)
            {
                if ( getYesNo("Display room description File",
                        (uchar)(roomBuf.rbroomtell[0] != '\0') ) )
                {
                    getString("Description Filename", description, 13, FALSE,
                    ECHO, (roomBuf.rbroomtell[0]) ? roomBuf.rbroomtell : "");
                    strcpy(roomBuf.rbroomtell, description);
                }
                else roomBuf.rbroomtell[0] = '\0';
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop and have Room descriptions configured.");
                doCR();
            }
            break;
    
        case 'G':
            mPrintf("Access Group"); doCR();
            
            if ((thisRoom > 2) || (thisRoom > 0 && sysop))
            {
                if (getYesNo("Change Group", 0))
                {
                    getString("Group for room <CR> for no group",
                                    groupname, NAMESIZE, FALSE, ECHO, "");

                    roomBuf.rbflags.GROUPONLY = TRUE;

                    groupslot = partialgroup(groupname);

                    if (!strlen(groupname) || (groupslot == ERROR) )
                    {
                        roomBuf.rbflags.GROUPONLY = 0;

                        if (groupslot == ERROR && strlen(groupname))
                            mPrintf("No such group.");
                    }

                    if (roomBuf.rbflags.GROUPONLY)
                    {
                        roomBuf.rbgrpno  = (unsigned char)groupslot;
                     /* roomBuf.rbgrpgen = grpBuf.group[groupslot].groupgen;*/
                    }
                }
            }
            else
            {
                if(thisRoom > 0)
                {
                    doCR();
                    mPrintf("Must be Sysop to change group for Mail> or Aide)");
                    doCR();
                }
                else
                {
                    doCR();
                    mPrintf("Lobby> can never be group only");
                    doCR();
                }
            }
            break;
        
        case 'V':
            mPrintf("Privileges Group"); doCR();
            
            if (getYesNo("Change Group", 0))
            {
                getString("Group for room <CR> for no group",
                                groupname, NAMESIZE, FALSE, ECHO, "");

                roomBuf.rbflags.PRIVGRP = TRUE;

                groupslot = partialgroup(groupname);

                if (!strlen(groupname) || (groupslot == ERROR) )
                {
                    roomBuf.rbflags.PRIVGRP   = FALSE;
                    roomBuf.rbflags.READONLY  = FALSE;
                    roomBuf.rbflags.DOWNONLY  = FALSE;
                    roomBuf.rbflags.UPONLY    = FALSE;
                    roomBuf.rbflags.GRP_MOD   = FALSE;

                    if (groupslot == ERROR && strlen(groupname))
                        mPrintf("No such group.");
                }

                if (roomBuf.rbflags.PRIVGRP )
                {
                    roomBuf.rbPgrpno  = (unsigned char)groupslot;
                 /* roomBuf.rbPgrpgen = grpBuf.group[groupslot].groupgen; */
                }
            }
            
            if (roomBuf.rbflags.PRIVGRP)
            {
                roomBuf.rbflags.READONLY =
                    getYesNo("Read only", (uchar)roomBuf.rbflags.READONLY);
                
                roomBuf.rbflags.GRP_MOD  =
                    getYesNo("Group Moderates", (uchar)roomBuf.rbflags.GRP_MOD);
            
                if (roomBuf.rbflags.MSDOSDIR)
                {    
                    roomBuf.rbflags.DOWNONLY =
                        getYesNo("Download only", 
                                (uchar)roomBuf.rbflags.DOWNONLY);
    
                    if (!roomBuf.rbflags.DOWNONLY)
                    {
                        roomBuf.rbflags.UPONLY   =  getYesNo("Upload only", 
                                             (uchar)roomBuf.rbflags.UPONLY);
                    }
                }
            }
            
            break;
            
        case 'H':
            mPrintf("Hidden Room"); doCR();
            
            if ((thisRoom > 2) || (thisRoom>=0 && sysop))
            {
                waspublic = (uchar)roomBuf.rbflags.PUBLIC;

                roomBuf.rbflags.PUBLIC =
                    !getYesNo("Hidden room", (uchar)(!roomBuf.rbflags.PUBLIC));

                if (waspublic && (!roomBuf.rbflags.PUBLIC))
                {
                    roomBuf.rbgen = (uchar)((roomBuf.rbgen +1) % MAXGEN);
                    logBuf.lbroom[thisRoom].lbgen = roomBuf.rbgen;
                }
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make Lobby>, Mail> or Aide) hidden.");
                doCR();
            }
            break;
    
        case 'Y':
            mPrintf("Anonymous Room"); doCR();
            
            if ((thisRoom > 2) || (thisRoom>=0 && sysop))
            {
                roomBuf.rbflags.ANON =
                     getYesNo("Anonymous room", (uchar)(roomBuf.rbflags.ANON));
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make Lobby>, Mail> or Aide) Anonymous.");
                doCR();
            }
            break;

#ifdef GOODBYE        
        case 'O':
            mPrintf("BIO Room"); doCR();
            
            if ((thisRoom > 2) || (thisRoom>=0 && sysop))
            {
                roomBuf.rbflags.BIO =
                    getYesNo("BIO room", (uchar)(roomBuf.rbflags.BIO));
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make Lobby>, Mail> or Aide) BIO.");
                doCR();
            }
            break;
#endif
            
        case 'M':
            mPrintf("Moderated"); doCR();
            
            if (sysop)
            {
                if (getYesNo("Moderated", (uchar)(roomBuf.rbflags.MODERATED) ))
                    roomBuf.rbflags.MODERATED = TRUE;
                else
                    roomBuf.rbflags.MODERATED = FALSE;
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make Moderated rooms.");
                doCR();
            }
            break;
    
        case 'E':
            mPrintf("Networked/Shared"); doCR();
            
            if (sysop)
            {
                roomBuf.rbflags.SHARED = getYesNo("Networked/Shared room",
                                         (uchar)roomBuf.rbflags.SHARED);
            }
            else
            {
                doCR();
                mPrintf("Must be Sysop to make Shared rooms.");
                doCR();
            }
            break;
    
        case 'P':
            mPrintf("Permanent");
            doCR();
            if (thisRoom > DUMP)
            {
                if (!roomBuf.rbflags.MSDOSDIR)
                {
                    roomBuf.rbflags.PERMROOM =
                        getYesNo("Permanent", (uchar)roomBuf.rbflags.PERMROOM);
                }
                else
                {
                    roomBuf.rbflags.PERMROOM = 1;
                    doCR();
                    mPrintf("Directory rooms are automaticly Permanent.");
                    doCR();
                }
            }
            else
            {
                doCR();
                mPrintf("Lobby> Mail> Aide) or Dump> always Permanent.");
                doCR();
            }
            break;
   
        case 'U':
            mPrintf("Subject"); doCR();
            
            roomBuf.rbflags.SUBJECT = getYesNo("Ask for subject in room",
                                     (uchar)roomBuf.rbflags.SUBJECT);
            break;
        
        case 'S':
            mPrintf("Save");  doCR();
            if (getYesNo("Save changes", FALSE))
            {
                noteRoom();
                putRoom(thisRoom);

                /* trap file line */
                sprintf(line, "Room \'%s\' changed to \'%s\' by %s",
                                oldname, roomBuf.rbname, logBuf.lbname);
                trap(line, T_AIDE);

                /* Aide room */
                formatSummary(summary);
                sprintf(msgBuf->mbtext, "%s \n%s", line, summary);
                aideMessage();

                return;
            }
            break;
        
        case 'A':
            mPrintf("Abort");  doCR();
            if (getYesNo("Abort", TRUE))
            {
                getRoom(thisRoom);
                return;
            }
            break;
    
        case '\r':
        case '\n':
        case '?':
            mPrintf("Menu"); doCR();
            prtMess = TRUE;
            break;
    
        default:
            mPrintf("%c ? for help", c); doCR();
            break;
        
        }
    } while (!quit);
}



