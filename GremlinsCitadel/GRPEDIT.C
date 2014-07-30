/* -------------------------------------------------------------------- */
/*  GRPEDIT.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                Code for the aide/sop to edit groups..                */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  groupfunc()         aide fn: to add/remove group members            */
/*  globalgroup()       aide fn: to add/remove group members  (global)  */
/*  globaluser()        aide fn: to add/remove user from groups (global)*/
/*  killgroup()         sysop special to kill a group                   */
/*  listgroup()         aide fn: to list groups                         */
/*  newgroup()          sysop special to add a new group                */
/*  renamegroup()       sysop special to rename a group                 */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*   5/14/91    (PAT)   Code moved from SYSOP1.C to shrink that file    */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  groupfunc()         aide fn: to add/remove group members            */
/* -------------------------------------------------------------------- */
void groupfunc(void)
{
    label               who;
    label groupname;
    int groupslot,      logNo;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    groupslot = partialgroup(groupname);

    if (groupslot != ERROR)
    {
        if ( grpBuf.group[groupslot].lockout && !sysop )
            groupslot = ERROR;
    
        if(  grpBuf.group[groupslot].hidden 
          && !ingroup(groupslot) 
          && !onConsole 
          )
            groupslot = ERROR;
    }

    if ( groupslot == ERROR || !strlen(groupname) )
    {
        mPrintf("\n No such group.");
        return;
    }

    getNormStr("who", who, NAMESIZE, ECHO);
    logNo   = findPerson(who, &lBuf);
    if (logNo == ERROR || !strlen(who) )  
    {
        mPrintf("No \'%s\' known. \n ", who);
        return;
    }

    if (lBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen)
    {
        if (getYesNo("Remove from group", 0))
        {
            lBuf.groups[groupslot] =
                (uchar)((grpBuf.group[groupslot].groupgen
              + (MAXGROUPGEN - 1)) % MAXGROUPGEN);

            sprintf(msgBuf->mbtext,
                    "%s kicked out of group %s by %s",
                    lBuf.lbname,
                    grpBuf.group[groupslot].groupname,
                    logBuf.lbname );

            trap(msgBuf->mbtext, T_AIDE);

            aideMessage();
        }
    }
    else

    if (getYesNo("Add to group", 0))
    {
        lBuf.groups[groupslot] = grpBuf.group[groupslot].groupgen;

        sprintf(msgBuf->mbtext,
        "%s added to group %s by %s",
        lBuf.lbname,
        grpBuf.group[groupslot].groupname,
        logBuf.lbname );

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();

    }

    putLog(&lBuf, logNo);

    /* see if it is us: */
    if (loggedIn  &&  strcmpi(logBuf.lbname, who) == SAMESTRING) 
    {
        logBuf.groups[groupslot] = lBuf.groups[groupslot];
    }

}

/* -------------------------------------------------------------------- */
/*  globalgroup()       aide fn: to add/remove group members  (global)  */
/* -------------------------------------------------------------------- */
void globalgroup(void)
{
    label groupname;
    int groupslot, i, yn, add, logNo;

    mPrintf("(A/R/[B]): ");

    switch (toupper( iCharNE() ))
    {
    case 'A':
        mPrintf("Add\n ");
        add = 1;
        break;
    
    case 'R':
        mPrintf("Remove\n ");
        add = 2;
        break;
    
    case '?':
        tutorial("grpglob.mnu");
        return;
        
    case 'B':
    case 10:
    case 13:
    default:
        mPrintf("Both\n ");
        add = 0;
        break;
    }

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    groupslot = partialgroup(groupname);

    if ( grpBuf.group[groupslot].hidden && !ingroup(groupslot) )
        groupslot = ERROR;

    if ( groupslot == ERROR || !strlen(groupname) )
    {
        mPrintf("\n No such group.");
        return;
    }

    if ( grpBuf.group[groupslot].lockout && !sysop )
    {
        mPrintf("\n Group is locked.");
        return;
    }

    for (i = 0; i < cfg.MAXLOGTAB; i++)
    {
        if (logTab[i].ltpwhash != 0 && logTab[i].ltnmhash !=0)
        {
            logNo=logTab[i].ltlogSlot;
            getLog(&lBuf, logNo);
            
            /* Get out.. */
            outFlag = OUTOK;
            setio(whichIO, echo, outFlag);

            if (mAbort(FALSE)) break;
            outFlag = IMPERVIOUS;
            setio(whichIO, echo, outFlag);

            
            if (lBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen)
            {
                if(add == 2 || add == 0)
                {
                    mPrintf(" 3%s0", lBuf.lbname);
                    yn=getYesNo("Remove from group", 0+3);
                    if (yn == 2)
                    {
                        SaveAideMess();
                        return;
                    }
      
                    if (yn)
                    {
                        lBuf.groups[groupslot] =
                            (uchar)((grpBuf.group[groupslot].groupgen
                          + (MAXGROUPGEN - 1)) % MAXGROUPGEN);
          
                        sprintf(msgBuf->mbtext,
                            "%s kicked out of group %s by %s",
                            lBuf.lbname,
                            grpBuf.group[groupslot].groupname,
                            logBuf.lbname );
           
                        trap(msgBuf->mbtext, T_SYSOP);
          
                        amPrintf(" %s\n", msgBuf->mbtext);
                    }
                }
            }
            else
            {
                if (add == 0 || add == 1)
                {
                    mPrintf(" 3%s0", lBuf.lbname);
                    yn=getYesNo("Add to group", 0+3);
                    if (yn == 2)
                    {
                        SaveAideMess();
                        return;
                    }
                    if (yn)
                    {
                        lBuf.groups[groupslot] =
                            grpBuf.group[groupslot].groupgen;
           
                        sprintf(msgBuf->mbtext,
                            "%s added to group %s by %s",
                            lBuf.lbname,
                            grpBuf.group[groupslot].groupname,
                            logBuf.lbname );
         
                        trap(msgBuf->mbtext, T_AIDE);
           
                        amPrintf(" %s\n",msgBuf->mbtext);
                    }
                }
            }
  
            putLog(&lBuf, logNo);
     
            /* see if it is us: */
            if (loggedIn  &&  strcmpi(logBuf.lbname, lBuf.lbname) == SAMESTRING)
            {
                logBuf.groups[groupslot] = lBuf.groups[groupslot];
            }
        }
    }

    SaveAideMess();
}

/* -------------------------------------------------------------------- */
/*  globaluser()        aide fn: to add/remove user from groups (global)*/
/* -------------------------------------------------------------------- */
void globaluser(void)
{
    label               who;
    int groupslot, yn, logNo;

    getNormStr("who", who, NAMESIZE, ECHO);
    logNo   = findPerson(who, &lBuf);
    if (logNo == ERROR || !strlen(who) )  
    {
        mPrintf("No \'%s\' known. \n ", who);
        return;
    }

    for(groupslot=0; groupslot < MAXGROUPS; groupslot++)
    {
      if (grpBuf.group[groupslot].g_inuse 
          && ( !grpBuf.group[groupslot].lockout || sysop )
          && ( !grpBuf.group[groupslot].hidden  || ingroup(groupslot) )
         )
      {
        mPrintf(" %s", grpBuf.group[groupslot].groupname);
        if (lBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen)
        {
            if ((yn = getYesNo("Remove from group", 3)) != 0 /*NULL*/)
            {
                if (yn == 2)
                {
                    SaveAideMess();
                    return;
                }

                lBuf.groups[groupslot] =
                  (uchar)((grpBuf.group[groupslot].groupgen
                  + (MAXGROUPGEN - 1)) % MAXGROUPGEN);
                sprintf(msgBuf->mbtext,
                  "%s kicked out of group %s by %s",
                  lBuf.lbname,
                grpBuf.group[groupslot].groupname,
                  logBuf.lbname );

                trap(msgBuf->mbtext, T_AIDE);

              amPrintf(" %s\n", msgBuf->mbtext);
            }
        }
        else
        {
          if ((yn = getYesNo("Add to group", 3)) != /* NULL */ 0)
          {
            if (yn == 2)
                {
                    SaveAideMess();
                    return;
                }

            lBuf.groups[groupslot] = grpBuf.group[groupslot].groupgen;

            sprintf(msgBuf->mbtext,
              "%s added to group %s by %s",
              lBuf.lbname,
              grpBuf.group[groupslot].groupname,
              logBuf.lbname );
            trap(msgBuf->mbtext, T_SYSOP);

              amPrintf(" %s\n",msgBuf->mbtext);
          }
        }
        putLog(&lBuf, logNo);

        /* see if it is us: */
        if (loggedIn  &&  strcmpi(logBuf.lbname, who) == SAMESTRING) 
        {
            logBuf.groups[groupslot] = lBuf.groups[groupslot];
        }
      }
    }

    SaveAideMess();
}

/* -------------------------------------------------------------------- */
/*  killgroup()         sysop special to kill a group                   */
/* -------------------------------------------------------------------- */
void killgroup(void)
{
    label groupname;
    int groupslot, i;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    groupslot = groupexists(groupname);

    if ( groupslot == 0 || groupslot == 1)
    {
        mPrintf("\n Cannot delete Null or Reserved_2 groups.");
        return;
    }

    if ( groupslot == ERROR || !strlen(groupname) )
    {
        mPrintf("\n No such group.");
        return;
    }

    for (i = 0; i < MAXROOMS; i++)
    {
         if  (   roomTab[i].rtflags.INUSE
             &&  roomTab[i].rtflags.GROUPONLY
             && (roomTab[i].grpno  == (unsigned char)groupslot)
          /* && (roomTab[i].grpgen == grpBuf.group[groupslot].groupgen)*/ )
         {
             mPrintf("\n Group still has rooms.");
             return;
         }
    }

    for (i = 0; i < MAXHALLS; i++)
    {
        if ( hallBuf->hall[i].h_inuse
        &&   hallBuf->hall[i].owned 
        &&   hallBuf->hall[i].grpno == (unsigned char)groupslot)
        {
            mPrintf("\n Group still has hallways.");
            return;
        }
    }

    if (!getYesNo(confirm, 0)) return;

    grpBuf.group[groupslot].g_inuse = 0;

    putGroup();

    sprintf(msgBuf->mbtext,
    "Group %s deleted", groupname );

    trap(msgBuf->mbtext, T_SYSOP);
}
    

/* -------------------------------------------------------------------- */
/*  listgroup()         aide fn: to list groups                         */
/* -------------------------------------------------------------------- */
void listgroup(void)
{
    label groupname;
    int i, groupslot;

    getString("", groupname, NAMESIZE, FALSE, ECHO, "");

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    if ( !strlen(groupname))
    {
        for (i = 0; i < MAXGROUPS; i++)
        {
            /* can they see the group */
            if (  grpBuf.group[i].g_inuse
              && (!grpBuf.group[i].lockout || sysop)
              && (!grpBuf.group[i].hidden  || onConsole || ingroup(i))
              )
            {
                mPrintf("%c%-30s %c%c%c ", 

                        (ingroup(i)) ? '*' : ' ',
                        grpBuf.group[i].groupname,
                        grpBuf.group[i].lockout ? 'L' : ' ',
                        grpBuf.group[i].hidden  ? 'H' : ' ',
                        (grpBuf.group[i].autoAdd  || !i)
                                                ? 'A' : ' '
                        );

                mPrintf("%s", grpBuf.group[i].desc);
                doCR();
            }
        }  
        return;
    }

    groupslot = partialgroup(groupname);

    if ( grpBuf.group[groupslot].hidden && !ingroup(groupslot) )
        groupslot = ERROR;

    if (groupslot == ERROR)
    {
        mPrintf("\n No such group.");
        return;
    }

    if ( grpBuf.group[groupslot].lockout
    && (!(logBuf.lbflags.SYSOP || onConsole)) )
    {
        mPrintf("\n Group is locked.");
        return;
    }

    doCR();

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    mPrintf(" Users:"); doCR();
    
    prtList(LIST_START);

    for (i = 0; ( ( i < cfg.MAXLOGTAB) && (outFlag != OUTSKIP)
        && (outFlag != OUTNEXT) ); i++)
    {
        if (logTab[i].ltpwhash != 0 && logTab[i].ltnmhash != 0)
        {
            getLog(&lBuf, logTab[i].ltlogSlot);

            if (lBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen

          ||  (strcmpi(lBuf.lbname, cfg.sysop) == SAMESTRING  )
)
            {
                prtList(lBuf.lbname);
                
                /* doCR();
                mPrintf(" %s", lBuf.lbname); */
            }
            else
            {
                mAbort(FALSE);
            }
        }
        
        if (outFlag == OUTSKIP)
        {
            doCR();
            return;
        }  
    }
    
    prtList(LIST_END);
    
    if (outFlag == OUTSKIP)
    {
        doCR();
        return;
    }  

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR(); 

    mPrintf(" Rooms:"); doCR();
    
    prtList(LIST_START);

    for (i = 0; ( ( i < MAXROOMS) && (outFlag != OUTSKIP)
        && (outFlag != OUTNEXT) ); i++)
    {
         if  (   roomTab[i].rtflags.INUSE 
             &&  roomTab[i].rtflags.GROUPONLY
             && (roomTab[i].grpno  == (unsigned char)groupslot)
         /*  && (roomTab[i].grpgen == grpBuf.group[groupslot].groupgen) */ )
         {
            prtList(roomTab[i].rtname);
         }
    }

    prtList(LIST_END);
    
    if (outFlag == OUTSKIP)
    {
        doCR();
        return;
    }  

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    doCR(); 

    mPrintf(" Hallways:"); 

    for (i = 0; ( ( i < MAXHALLS) && (outFlag != OUTSKIP)
        && (outFlag != OUTNEXT) ); i++)
    {
        if ( hallBuf->hall[i].h_inuse &&
             hallBuf->hall[i].owned   &&
            (hallBuf->hall[i].grpno == (unsigned char)groupslot) )
        {
            doCR();
            mPrintf(" %s", hallBuf->hall[i].hallname);
        }
    }
    doCR();
}

/* -------------------------------------------------------------------- */
/*  newgroup()          sysop special to add a new group                */
/* -------------------------------------------------------------------- */
void newgroup(void)
{
    label groupname;
    int slot, i;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    if ( (groupexists(groupname) != ERROR) || !strlen(groupname) )
    {
        mPrintf("\n We already have a \'%s\' group.", groupname);
        return;
    }

    /* search for a free group slot */

    for (i = 0, slot = 0; i < MAXGROUPS && !slot ; i++)
    {
        if (!grpBuf.group[i].g_inuse) slot = i;
    }

    if (!slot)
    {
        mPrintf("\n Group table full.");
        return;
    }
    
    getString("group description", grpBuf.group[slot].desc, 79, FALSE, ECHO,"");
    
    grpBuf.group[slot].lockout = (getYesNo("Lock group from aides", 0 ));

    grpBuf.group[slot].hidden  = (getYesNo("Hide group", 0 ));
    
    grpBuf.group[slot].autoAdd = (getYesNo("Auto-Add group", 0 ));

    strcpy(grpBuf.group[slot].groupname, groupname);
    grpBuf.group[slot].g_inuse = 1;

    /* increment group generation # */
    grpBuf.group[slot].groupgen =
   (uchar)((grpBuf.group[slot].groupgen + 1) % MAXGROUPGEN);

    if (getYesNo(confirm, 0))
    {
        putGroup();

        sprintf(msgBuf->mbtext,
        "Group %s created", grpBuf.group[slot].groupname );

        trap(msgBuf->mbtext, T_SYSOP);
        
        logBuf.groups[slot] = grpBuf.group[slot].groupgen;
        storeLog();
    }
    else
    {
        getGroup();
    }
}

/* -------------------------------------------------------------------- */
/*  renamegroup()       sysop special to rename a group                 */
/* -------------------------------------------------------------------- */
void renamegroup(void)
{
    label groupname, newname;
    char desc[80];
    int groupslot, locked, hidden, autoAdd;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    groupslot = partialgroup(groupname);

    if ( grpBuf.group[groupslot].hidden && !ingroup(groupslot) && !onConsole)
        groupslot = ERROR;

    if ( groupslot == ERROR || !strlen(groupname) )
    {
        mPrintf("\n No such group.");
        return;
    }

    doCR();
    mPrintf(" Group: %s ", grpBuf.group[groupslot].groupname);
    if (grpBuf.group[groupslot].lockout)
    {
        mPrintf("(Locked) ");
    }
    if (grpBuf.group[groupslot].hidden)
    {
        mPrintf("(Hidden) ");
    }
    if (grpBuf.group[groupslot].autoAdd)
    {
        mPrintf("(Auto-Add)");
    }
    doCR();
    mPrintf(" Descibed: %s ", grpBuf.group[groupslot].desc);
    doCR();
    
    getString("new group name", newname, NAMESIZE, FALSE, ECHO, 
              grpBuf.group[groupslot].groupname);
    
    getString("new group description", desc, 79, FALSE, ECHO, "");
    
    if (groupexists(newname) != ERROR 
     && stricmp(newname, grpBuf.group[groupslot].groupname) != SAMESTRING)
    {
        mPrintf("\n A \'%s\' group already exists.", newname);
        return;
    }

    /*  locked group? */
    locked =
        (getYesNo("Lock group from aides", 
                  (char)grpBuf.group[groupslot].lockout ));

    hidden =
        (getYesNo("Make group hidden", (char)grpBuf.group[groupslot].hidden ));
    
    if (groupslot)
    {
        autoAdd =
        (getYesNo("Make group auto-add", 
                 (char)grpBuf.group[groupslot].autoAdd));
    }

    if (getYesNo(confirm, 0))
    {
        grpBuf.group[groupslot].lockout = locked;
        grpBuf.group[groupslot].hidden  = hidden;
        grpBuf.group[groupslot].autoAdd = autoAdd;
        
        strcpy(grpBuf.group[groupslot].groupname, newname);
        
        if (*desc)
        {
            strcpy(grpBuf.group[groupslot].desc, desc);
        }
        
        sprintf(msgBuf->mbtext,
        "Group %s renamed %s", groupname, newname);
    
        trap(msgBuf->mbtext, T_SYSOP);
    
        putGroup();
    }
}


