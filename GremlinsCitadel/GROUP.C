/************************************************************************/
/*                              group.c                                 */
/*             group code for Citadel bulletin board system             */
/************************************************************************/
#include "ctdl.h"

/************************************************************************/
/*                              Contents                                */
/*                                                                      */
/*      cleargroupgen()         removes logBuf from all groups          */
/*      groupexists()           returns # of named group, else ERROR    */
/*      groupseesroom()         indicates if group can see room #       */
/*      groupseeshall()         indicates if group can see hall #       */
/*      ingroup()               returns TRUE if log is in named group   */
/*      partialgroup()          returns slot of partially named group   */
/*      setgroupgen()           sets unmatching group generation #'s    */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*    cleargroupgen()  removes logBuf from all groups                   */
/************************************************************************/
void cleargroupgen(void)
{
    int groupslot;

    for (groupslot = 0; groupslot < MAXGROUPS; groupslot++)
    {
        logBuf.groups[groupslot] =
            (uchar)((grpBuf.group[groupslot].groupgen
          + (MAXGROUPGEN - 1)) % MAXGROUPGEN);
    }
}

/************************************************************************/
/*      groupseeshall()  returns true if group can see hallway          */
/************************************************************************/
int groupseeshall(int hallslot)
{
    if ( (!hallBuf->hall[hallslot].owned) ||

    /* generation in logBuf for this hall's owning group */
    ( logBuf.groups[ hallBuf->hall[hallslot].grpno ]  ==

    /* generation in groupbuffer for this hall's owning group */
    grpBuf.group[ hallBuf->hall[hallslot].grpno ].groupgen  ))
    
    return(TRUE);

    return(FALSE);
}

/************************************************************************/
/*      groupseesroom()  returns true if group can see room             */
/************************************************************************/
int groupseesroom(int groupslot)
{
    if ( 
            (!roomTab[groupslot].rtflags.GROUPONLY) ||

    /* generation in logBuf for this room's owning group == */
    /* generation in groupbuffer for this room's owning group */
            ( logBuf.groups[ roomTab[groupslot].grpno ]  ==
              grpBuf.group[  roomTab[groupslot].grpno ].groupgen  )
       )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

/************************************************************************/
/*      pgroupseesroom()  returns true if group can see room             */
/************************************************************************/
int pgroupseesroom(void)
{
    if ( 
            (!roomTab[thisRoom].rtflags.PRIVGRP) ||

    /* generation in logBuf for this room's owning group == */
    /* generation in groupbuffer for this room's owning group */
            ( logBuf.groups[ roomBuf.rbPgrpno ]  ==
              grpBuf.group[  roomBuf.rbPgrpno ].groupgen  )
       )
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

/************************************************************************/
/*      groupexists()  return # of named group, else ERROR              */
/************************************************************************/
int groupexists(char *groupname)
{
    int i;

    for (i = 0; i < MAXGROUPS; i++)
    {
        if (grpBuf.group[i].g_inuse &&
            strcmpi(groupname, grpBuf.group[i].groupname) == SAMESTRING )
        return(i);
    }
    return(ERROR);
}

/************************************************************************/
/*      ingroup()  returns TRUE if person is in named group             */
/************************************************************************/
int ingroup(int groupslot)
{
    if (groupslot == ERROR) return(FALSE);

    if ( ((logBuf.groups[groupslot] == grpBuf.group[groupslot].groupgen)
          ||
          strcmpi(logBuf.lbname, cfg.sysop) == SAMESTRING  )
        &&  grpBuf.group[groupslot].g_inuse)
        return(TRUE);
    return(FALSE);
}

/************************************************************************/
/*      getgroup()  aide fn: to list groups                            */
/************************************************************************/
void getgroup(void)
{
    label groupname;
    int groupslot;

    mf.mfGroup[0] = '\0' /*NULL*/;

    getString("group", groupname, NAMESIZE, FALSE, ECHO, "");

    if (!(*groupname))
    {
        strcpy(groupname, grpBuf.group[0].groupname);
    }

    groupslot = partialgroup(groupname);

    if (groupslot == ERROR)
    {
        mPrintf("\n No such group!");
        doCR();
        mf.mfLim = FALSE;
        return;
    }

    if ( !(ingroup(groupslot) || sysop || aide))
    {
        mf.mfLim = FALSE;
        return;
    }

    if ( grpBuf.group[groupslot].lockout && (!logBuf.lbflags.SYSOP) )
    {
        mf.mfLim = FALSE;
        return;
    }

    mPrintf("\n Reading group %s only.\n ", grpBuf.group[groupslot].groupname);

    strcpy(mf.mfGroup, grpBuf.group[groupslot].groupname);
}

/************************************************************************/
/*      partialgroup()  returns slot # of partial group name, else error*/
/*      used for .EL Message, .EL Room and .AG .AL                      */
/************************************************************************/
int partialgroup(char *groupname)
{
    int i, length;

    length = strlen(groupname);
    
    if (!length) return ERROR;
    
    /*
     * Exact match
     */
    i = groupexists(groupname);
    if (i != ERROR)
        return i;
    
    /*
     * Start of string match
     */
    for (i = 0; i < MAXGROUPS; i++)
    {
        if (grpBuf.group[i].g_inuse)
        {
            if (
              (strnicmp(grpBuf.group[i].groupname, groupname, length) == SAMESTRING)
              && (ingroup(i) || aide || sysop)
               )
            
            return(i);
        }
    }         
    
    /*
     * Partial match
     */
    for (i = 0; i < MAXGROUPS; i++)
    {
        if (grpBuf.group[i].g_inuse)
        {
            if (
              (substr(grpBuf.group[i].groupname, groupname) != '\0' /*NULL*/)
              && (ingroup(i) || aide || sysop)
               )
            
            return(i);
        }
    }         
   
    return(ERROR);
}

