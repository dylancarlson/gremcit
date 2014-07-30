/* -------------------------------------------------------------------- */
/*  MSG.C                    Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*               This is the high level message code.                   */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  clearmsgbuf()   this clears the message buffer out                  */
/*  mAbort()        returns TRUE if the user has aborted typeout        */
/*  indexslot()     give it a message # and it returns a slot#          */
/*  mayseemsg()     returns TRUE if person can see message. 100%        */
/*  mayseeindexmsg() Can see message by slot #. 99%                     */
/*  sizetable()     returns # messages in table                         */
/*  indexmessage()  builds one message index from msgBuf                */
/* -------------------------------------------------------------------- */
 /*  addUnknownList()    adds another unknown message header thingy      */ 
 /* $disposeUnkownList() disposes all unknown message header thingies    */ 
  
 struct unkLst *firstUnk; 
  
 static void disposeUnknownList(void); 
  
 struct unkLst *addUnknownList(void) 
    { 
    struct unkLst *lul, *lul2; 
  
    lul = malloc(sizeof(struct unkLst)); 
    if (lul != NULL) 
        { 
        lul->next = NULL; 
        if (firstUnk == NULL) 
            { 
            firstUnk = lul; 
            } 
        else 
            { 
            for (lul2 = firstUnk; lul2->next != NULL; lul2 = lul2->next); 
            lul2->next = lul; 
            } 
        } 
    return(lul); 
    } 
  
 static void disposeUnknownList(void) 
    { 
    struct unkLst *lul, *lul2; 
  
    for(lul = firstUnk; lul != NULL; lul2 = lul, lul = lul->next, 
_ffree(lul2)); 
    firstUnk = NULL; 
    } 
  

/* -------------------------------------------------------------------- */
/*  clearmsgbuf()   this clears the message buffer out                  */
/* -------------------------------------------------------------------- */
void clearmsgbuf(void)
{
    /* clear msgBuf out */
    msgBuf->mbroomno    =   0 ;
    msgBuf->mbattr      =   0 ;
    msgBuf->mbauth[ 0]  = '\0';
    msgBuf->mbtitle[0]  = '\0';
    msgBuf->mbocont[0]  = '\0';
    msgBuf->mbfpath[0]  = '\0';
    msgBuf->mbtpath[0]  = '\0';
    msgBuf->mbczip[ 0]  = '\0';
    msgBuf->mbcopy[ 0]  = '\0';
    msgBuf->mbfwd[  0]  = '\0';
    msgBuf->mbgroup[0]  = '\0';
    msgBuf->mbtime[ 0]  = '\0';
    msgBuf->mbId[   0]  = '\0';
    msgBuf->mbsrcId[0]  = '\0';
    msgBuf->mboname[0]  = '\0';
    msgBuf->mboreg[ 0]  = '\0';
    msgBuf->mbreply[0]  = '\0';
    msgBuf->mbroom[ 0]  = '\0';
    msgBuf->mbto[   0]  = '\0';
    msgBuf->mbsur[  0]  = '\0';
    msgBuf->mblink[ 0]  = '\0';
    msgBuf->mbx[    0]  = '\0';
    msgBuf->mbzip[  0]  = '\0';
    msgBuf->mbrzip[ 0]  = '\0';
    msgBuf->mbusig[ 0]  = '\0';
    msgBuf->mbsub[  0]  = '\0';
    msgBuf->mbsig[  0]  = '\0';
    msgBuf->mbsoft[ 0]  = '\0';
    msgBuf->mbcreg[ 0]  = '\0';
    msgBuf->mbccont[0]  = '\0';
    /* nodephone */
    msgBuf->mbophone[0] = '\0';
    msgBuf->mbzphone[0] = '\0';

    disposeUnknownList();

}

/* -------------------------------------------------------------------- */
/*  mAbort()        returns TRUE if the user has aborted typeout        */
/* -------------------------------------------------------------------- */
BOOL mAbort(BOOL pause)
{
    char c;
    char toReturn = FALSE;
    char oldEcho;
    int  i;
    BOOL more = FALSE;
    
    /*
     * Can not abort IMPERVIOUS
     */
    if (outFlag == IMPERVIOUS)
    {
        /* for screenblanker */
        kb_hit();
        while(STATCON())  GETCON();

        return FALSE;
    }
    
    /*
     * Carrier loss and not on Console
     */
    if (!CARRIER)
    {
        outFlag = OUTSKIP;
        setio(whichIO, echo, outFlag);

        return TRUE;
    }

    /*
     *  Check for keypress..
     */
    if (BBSCharReady() || pause)
    {
        oldEcho  = echo;
    
        if (pause)
        {
            c = 'P';
            more = TRUE;
        }
        else
        {
            echo = NEITHER;
            setio(whichIO, echo, outFlag);

            c = (char)toupper(iChar());
        }

        if (c == 'P' || c == 19 /* XOFF */)       /* Pause! */
        {
            echo = oldEcho;
            setio(whichIO, echo, outFlag);
            
            if (more)
                putWord((uchar *)"<More>");
           
            echo = NEITHER;
            setio(whichIO, echo, outFlag);
            
            c = (char)toupper(iChar());             /* wait to resume */

            echo = oldEcho;
            setio(whichIO, echo, outFlag);
            
            if (more)
                for(i=0; i<6; i++)
                    doBS();
        }
        
        echo = oldEcho;
        setio(whichIO, echo, outFlag);
        
        if (outFlag == NOSTOP)  
            return FALSE;
        
        switch (c)
        {
        case 'C':
            dotoMessage = COPY_IT;
            toReturn    = FALSE;
            break;
            
        case 'J':                            /* jump paragraph:*/
            doCR();
            termCap(TERM_NORMAL);
            putWord((uchar *)"<");
            termCap(TERM_BOLD);
            putWord((uchar *)"Jump");
            termCap(TERM_NORMAL);
            putWord((uchar *)">");
            doCR();
            outFlag     = OUTPARAGRAPH;
            setio(whichIO, echo, outFlag);

            toReturn    = FALSE;
            break;
            
        case 'K':                            /* kill:          */
            if ((dowhat == READMESSAGE) &&  loggedIn &&( CAN_MODERATE()
                 || (cfg.kill && (strcmpi(logBuf.lbname, 
                                          msgBuf->mbauth) == SAMESTRING)))
               )
            {   
                dotoMessage = PULL_IT;
                
                doCR();


                termCap(TERM_NORMAL);
                putWord((uchar *)"<");
                termCap(TERM_BOLD);
                putWord((uchar *)"Kill");
                termCap(TERM_NORMAL);
                putWord((uchar *)">");

                outFlag     = OUTNEXT;
                setio(whichIO, echo, outFlag);

                toReturn    = TRUE;
            }
            else
            {
                toReturn               = FALSE;
            }
            break;
            
        case 'M':                            /* mark:          */
            if ((dowhat == READMESSAGE) && aide)
            {
                dotoMessage = MARK_IT;
                
                doCR();


                termCap(TERM_NORMAL);
                putWord((uchar *)"<");
                termCap(TERM_BOLD);
                putWord((uchar *)"Mark");
                termCap(TERM_NORMAL);
                putWord((uchar *)">");



                outFlag     = OUTNEXT;
                setio(whichIO, echo, outFlag);

                toReturn    = TRUE;
            }
            else
            {
                toReturn               = FALSE;
            }
            break;
            
        case 'N':                            /* next:          */
            doCR();


            termCap(TERM_NORMAL);
            putWord((uchar *)"<");
            termCap(TERM_BOLD);
            putWord((uchar *)"Next");
            termCap(TERM_NORMAL);
            putWord((uchar *)">");


            doCR();
            outFlag     = OUTNEXT;
            setio(whichIO, echo, outFlag);

            toReturn    = TRUE;
            break;
            
        case 'S':                            /* skip:          */
            doCR();

            termCap(TERM_NORMAL);
            putWord((uchar *)"<");
            termCap(TERM_BOLD);
            putWord((uchar *)"Stop");
            termCap(TERM_NORMAL);
            putWord((uchar *)">");

            doCR();
            outFlag     = OUTSKIP;
            setio(whichIO, echo, outFlag);

            toReturn    = TRUE;
            break;
        
        case 'R':
            dotoMessage = REVERSE_READ;
            outFlag     = OUTNEXT;
            setio(whichIO, echo, outFlag);

            toReturn    = FALSE;
            break;

        case 'V':
            verbose = !verbose;

            doCR();

            termCap(TERM_NORMAL);
            putWord((uchar *)"<");
            termCap(TERM_BOLD);

            if (verbose)
            {
                putWord((uchar *)"Verbose +");
            }
            else
            {
                putWord((uchar *)"Verbose -");
            }

            termCap(TERM_NORMAL);
            putWord((uchar *)">");

            doCR();

            echo = oldEcho;
            setio(whichIO, echo, outFlag);

            toReturn    = FALSE;
            break;
        
        default:
            toReturn    = FALSE;
            break;
        }
    }
    
    return toReturn;
}


/* -------------------------------------------------------------------- */
/*  indexslot()     give it a message # and it returns a slot#          */
/* -------------------------------------------------------------------- */
ulong indexslot(ulong msgno)
{ 
    if (msgno < cfg.mtoldest)
    {
        if (debug)
        {
#ifdef GOODBYE
            doCR();
            mPrintf("Can't find attribute");
            doCR();
#endif
            doccr();
            cPrintf("Can't find attribute");
            doccr();
        }
        return(ULONG_ERROR);
    }

    return(msgno - cfg.mtoldest);
}


#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  indexslot()     give it a message # and it returns a slot#          */
/* -------------------------------------------------------------------- */
int indexslot(ulong msgno)
{ 
    if (msgno < cfg.mtoldest)
    {
        return(ERROR);
    }

    return((int)(msgno - cfg.mtoldest));
}
#endif

/* -------------------------------------------------------------------- */
/*  mayseemsg()     returns TRUE if person can see message. 100%        */
/* -------------------------------------------------------------------- */
BOOL mayseemsg(void)
{
    int i;
    uchar attr;

    if (!copyflag) attr = msgBuf->mbattr;
    else           attr = originalattr;


    /* mfUser From Acit */
    if ( mf.mfUser[0] )
    {
        stripansi(mf.mfUser);

        if (   !u_match(deansi(msgBuf->mbto),   mf.mfUser)
            && !u_match(deansi(msgBuf->mbauth), mf.mfUser)
            && !u_match(deansi(msgBuf->mbfwd),  mf.mfUser)
            && !u_match(deansi(msgBuf->mboname),mf.mfUser)
            && !u_match(deansi(msgBuf->mboreg), mf.mfUser)
            && !u_match(deansi(msgBuf->mbocont),mf.mfUser)
            && !u_match(deansi(msgBuf->mbzip),  mf.mfUser)
            && !u_match(deansi(msgBuf->mbrzip), mf.mfUser)
            && !u_match(deansi(msgBuf->mbczip), mf.mfUser)
               ) return (FALSE);
    }


#ifdef GOODBYE
    /* mfUser */
    if ( mf.mfUser[0] )
    {
        if (!u_match(msgBuf->mbto, mf.mfUser)
            && !u_match(msgBuf->mbauth, mf.mfUser) )
            return (FALSE);
    }
#endif

    /* check for PUBLIC non problem user messages first */
    if ( !msgBuf->mbto[0] && !msgBuf->mbx[0] && !msgBuf->mbgroup[0])
        return(TRUE);

    if (!loggedIn && dowhat != NETWORKING) return(FALSE);

    if (msgBuf->mbx[0])
    {
        if ( strcmpi(msgBuf->mbauth,  logBuf.lbname) == SAMESTRING )
        {
            /* problem users cant see copies of their own messages */
            if (copyflag) return(FALSE);
        }
        else
        {
            /* but everyone else cant see the orignal if it has been released */
            if (   !copyflag 
                && ((attr & ATTR_MADEVIS) == ATTR_MADEVIS)
               ) 
                return(FALSE);
            
            /* problem user message... */    
            if (   msgBuf->mbx[0] == 'Y'
                && !/* CAN_MODERATE() */ (sysop || aide)
                && !((attr & ATTR_MADEVIS) == ATTR_MADEVIS)
               )
            {   
                return(FALSE);
            }
            
            /* moderated message... */    
            if (   msgBuf->mbx[0] == 'M' 
                && !((attr & ATTR_MADEVIS) == ATTR_MADEVIS)
               )
            {   
                if ( !(   sysop 
                       || (aide && !cfg.moderate) 
                       /* || (roomTab[thisRoom].rtflags.GRP_MOD&&pgroupseesroom()) */
                      ) 
                   )
                {   
                    return(FALSE);
                }
            }
        }
    }


    if ( msgBuf->mbto[0] )
    {
        /* author can see his own private messages */
        /* but ONLY on local system. */
        if (strcmpi(msgBuf->mbauth,  logBuf.lbname) == SAMESTRING
            &&
            (strcmpi(msgBuf->mbsrcId,  msgBuf->mbId) == SAMESTRING
             || msgBuf->mbsrcId[0]  == '\0' /* cant see if it has been netted */
            )
          )
            return(TRUE);
            
        /* recipient can see private messages      */
        if (
            strcmpi(msgBuf->mbto, logBuf.lbname) == SAMESTRING
            &&
            (
            (!msgBuf->mbfwd[0])
            ||
            /* but only when no forwarding is set and only on local
               system when forwarding is set */
              (
                 (strcmpi(msgBuf->mbsrcId,  msgBuf->mbId) == SAMESTRING
                 || msgBuf->mbsrcId[0]  == '\0' /* cant see if it has been netted */
                 )
              )
             )
             )

        return(TRUE);          

        /* forwardee can see private messages      */
        if (strcmpi(msgBuf->mbfwd, logBuf.lbname) == SAMESTRING)
        return(TRUE);
            
        /* sysops see messages to 'Sysop'           */
        if (( sysop && ( strcmpi(msgBuf->mbto, "Sysop") == SAMESTRING) )
        &&
        !(strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING && *msgBuf->mbzip)
        )
        return(TRUE);

        /* aides see messages to 'Aide'           */
        if (( aide && ( strcmpi(msgBuf->mbto, "Aide") == SAMESTRING) )
        &&
        !(strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING && *msgBuf->mbzip)
        )
        return(TRUE);

#if 1
        /* sysops see messages from 'Sysop'           */
        if ( sysop && ( strcmpi(msgBuf->mbauth, "Sysop") == SAMESTRING)) 
        
        return(TRUE);

        /* aides see messages from 'Aide'           */
        if ( aide && ( strcmpi(msgBuf->mbauth, "Aide") == SAMESTRING) )
        
        return(TRUE);

#endif


        /* none of those so cannot see message     */
        return(FALSE);
    }

    if (msgBuf->mbgroup[0])
    {
        i = groupexists(msgBuf->mbgroup);

        if (i != ERROR)
        {
            return(ingroup(i));
        }
        else
        {     
            if (sysop)
            {
                mPrintf("    Unknown group %s!", msgBuf->mbgroup); doCR();
            }
            return(sysop);
        }
    }

#ifdef GOODBYE
        /* sysop is god and can see all group only messages */
        if (strcmpi(cfg.sysop, logBuf.lbname) == SAMESTRING)
            return(TRUE);

        for (i = 0 ; i < MAXGROUPS; ++i)
        {
            /* check to see which group message is to */
            if (strcmpi(grpBuf.group[i].groupname, msgBuf->mbgroup) == SAMESTRING)
            {
                /* if in that group */
                if (logBuf.groups[i] == grpBuf.group[i].groupgen )
                {
                    return(TRUE);
                }
                else
                {
                    return(FALSE);
                }
            }
        } /* group can't see message, return false */
        
        if (sysop)
            mPrintf("    Unknown group %s!", msgBuf->mbgroup); doCR();
        return(sysop);
    }
#endif


    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  mayseeindexmsg() Can see message by slot #. 99%                     */
/* -------------------------------------------------------------------- */
BOOL mayseeindexmsg(ulong slot)
{
    int i;

#ifdef NEWMSGTAB
    ulong snot = slot;
#else
    int snot = (int)slot;
#endif



#ifdef NEWMSGTAB
    ulong oslot;
    struct messagetable *lmt;

#else
    int oslot;
#endif

    char copy = 0;


#ifdef NEWMSGTAB
    oslot = snot;
#else
    oslot = (int)snot;
#endif


#ifdef NEWMSGTAB
   lmt = getMsgTab(snot);
#endif



/*  if (msgTab3[snot].mtoffset > (unsigned short)snot)  return(FALSE); */

    /* seek out original message table entry */
#ifdef NEWMSGTAB
    while (lmt->mtmsgflags.COPY)
#else
    while (msgTab_mtmsgflags[snot].COPY)
#endif
    {
       copy = TRUE;
       /* copy has scrolled */

#ifdef NEWMSGTAB
/*    if (  lmt->mtomesg  lmt->mtmsgLoc  > snot)  */

      if (long_JOIN(lmt->mttohash, lmt->mtauthhash)  >  snot)

#else
    /*  if (msgTab_mtomesg[snot]  > (unsigned int)snot) */

     if
((int)long_JOIN(msgTab_mttohash[(int)snot], msgTab_mtauthhash[(int)snot]) 
     >  (unsigned int)snot)

#endif

           return(FALSE);

       else  /* look at original message index */

#ifdef NEWMSGTAB
/*         snot = snot - (int)lmt->mtomesg;  */
/*         snot = snot - lmt->mtmsgLoc; */
           snot = snot - long_JOIN(lmt->mttohash, lmt->mtauthhash);

           lmt = getMsgTab(snot);
#else
/*         snot = snot - (int)msgTab_mtomesg[snot]; */
           snot = snot - (int)(long_JOIN(msgTab_mttohash[(int)snot], 
          msgTab_mtauthhash[(int)snot]));
#endif
    }

    /* check for PUBLIC non problem user messages first */

#ifdef NEWMSGTAB
    if (    !lmt->mttohash 
         && !lmt->mtmsgflags.PROBLEM 

         /* new addition */
         && !lmt->mtmsgflags.MODERATED


         && !lmt->mtmsgflags.LIMITED)
#else
    if (    !msgTab_mttohash[snot] 
         && !msgTab_mtmsgflags[snot].PROBLEM 

         /* new addition */
         && !msgTab_mtmsgflags[snot].MODERATED

         && !msgTab_mtmsgflags[snot].LIMITED)
#endif
    {
        return(TRUE);
    }

    if (!loggedIn && dowhat != NETWORKING) return(FALSE);

#ifdef NEWMSGTAB
    if (lmt->mtmsgflags.PROBLEM 

    /* new addition */
    || lmt->mtmsgflags.MODERATED
)
#else
    if (msgTab_mtmsgflags[snot].PROBLEM

    /* new addition */
   || msgTab_mtmsgflags[snot].MODERATED

)
#endif
    
    {
        if (copy)
        {
            /* problem users can not see copies of their messages */

#ifdef NEWMSGTAB
            if (lmt->mtauthhash == (int)hash(logBuf.lbname))
#else
            if (msgTab_mtauthhash[snot] == (int)hash(logBuf.lbname))
#endif

                return FALSE;
        }
        else
        {
            if (
                  !(
                        /* if you are a aide/sop and it is not MADEVIS */
                        (aide || sysop) 
#ifdef NEWMSGTAB
                     || getFlags(oslot)->MADEVIS
#else
                     || msgTab_mtmsgflags[oslot].MADEVIS
#endif
                   )

#ifdef NEWMSGTAB
                && lmt->mtauthhash         != (int)hash(logBuf.lbname)
#else
                && msgTab_mtauthhash[snot] != (int)hash(logBuf.lbname)
#endif

               ) return FALSE;
        }
    }   


#ifdef NEWMSGTAB
    if (lmt->mtmsgflags.MAIL)
#else
    if (msgTab_mtmsgflags[snot].MAIL)

#endif

    {


/*****************************/
#ifdef GOODBYE

        /* author can see his own private messages */
        if (msgTab_mtauthhash[snot] == (int)hash(logBuf.lbname)
            && (msgTab9[snot].mtorigin == 0 ||
            msgTab9[snot].mtorigin == (int)hash(cfg.nodeTitle))
             )
        {
              return(TRUE);
        }

#endif

        /* author can see his own private messages */
        /* author can see his own net private message ONLY on local system */

#ifdef NEWMSGTAB
        if (lmt->mtauthhash == (int)hash(logBuf.lbname)
#else
        if (msgTab_mtauthhash[snot] == (int)hash(logBuf.lbname)
#endif

            /* it the source id is equal to the local id then message */
            /* is thought to be not netted */
            /* It would be possible however by vast cooincidence that a */
            /* message netted out would have the same local and src id */
            /* fucking unlikely */
            /* it still won't get through mayseemsg tho. */
            &&

#ifdef GOODBYE
            ( 
            (msgTab_mtomesg[snot] == 0)  
            ||
            (msgTab_mtomesg[snot] == (unsigned int)(cfg.mtoldest + snot)) 
            )           
            )
#endif

#ifdef NEWMSGTAB
          !lmt->mtmsgflags.NETWORKED   /* is local outbound mail */
#else
          !msgTab_mtmsgflags[snot].NETWORKED   /* is local outbound mail */
#endif

        )
        {
              return(TRUE);
        }

/*****************************/


        /* recipient can see private messages      */



#ifdef NEWMSGTAB
        if (lmt->mttohash == (int)hash(logBuf.lbname)
            /* recipient can see net e-mail if its forwarded */
   && ((!lmt->mtmsgflags.NET) || lmt->mtmsgflags.FORWARDED 
           /* msgTab7.mtfwdhash  */

)
            &&
            (
          ( /* !msgTab7.mtfwdhash */ !lmt->mtmsgflags.FORWARDED)
            ||
               /* but only when no forwarding is set and only on local
                  system when forwarding is set */

                !lmt->mtmsgflags.NETWORKED
            )
            ) 
         return(TRUE);
#else
        if (msgTab_mttohash[snot] == (int)hash(logBuf.lbname)
            /* recipient can see net e-mail if its forwarded */
   && ((!msgTab_mtmsgflags[snot].NET) || msgTab_mtmsgflags[snot].FORWARDED 
           /* msgTab7[snot].mtfwdhash  */

)
            &&
            (
          ( /* !msgTab7[snot].mtfwdhash */ !msgTab_mtmsgflags[snot].FORWARDED)
            ||
               /* but only when no forwarding is set and only on local
                  system when forwarding is set */


                !msgTab_mtmsgflags[snot].NETWORKED
            )
            ) 
         return(TRUE);

#endif

#ifdef NEWMSGTAB
        /* forwardee can see private messages      */   
        if(lmt->mtmsgflags.FORWARDED)
        {
        /*  if (msgTab7.mtfwdhash == (int)hash(logBuf.lbname)  */
        /*  if ((int)lmt->mtomesg == (int)hash(logBuf.lbname)  
                && !lmt->mtmsgflags.NET  )   return(TRUE);  */

            /* if networked message, the FORWARDEE hash is stored in the
                RECIPIENT hash and RECIPIENT does not see message. */
            if (  

                   (  (!lmt->mtmsgflags.NETWORKED)
                   ? (int)lmt->mtomesg 
                   : (int)lmt->mttohash)


                    == (int)hash(logBuf.lbname)  
                && !lmt->mtmsgflags.NET
              )   return(TRUE);

#else
        /* forwardee can see private messages      */   
        if(msgTab_mtmsgflags[snot].FORWARDED)
        {
        /*  if (msgTab7[snot].mtfwdhash == (int)hash(logBuf.lbname)  */
        /*  if ((int)msgTab_mtomesg[snot] == (int)hash(logBuf.lbname)  
                && !msgTab_mtmsgflags[snot].NET  )   return(TRUE);  */

            /* if networked message, the FORWARDEE hash is stored in the
                RECIPIENT hash and RECIPIENT does not see message. */
            if (  

                   (  (!msgTab_mtmsgflags[snot].NETWORKED)
                   ? (int)msgTab_mtomesg[snot] 
                   : (int)msgTab_mttohash[snot])


                    == (int)hash(logBuf.lbname)  
                && !msgTab_mtmsgflags[snot].NET
              )   return(TRUE);

#endif

        }    
        /* sysops see messages to 'Sysop'           */

#ifdef NEWMSGTAB
        if ( sysop && (lmt->mttohash == (int)hash("Sysop"))
#else
        if ( sysop && (msgTab_mttohash[snot] == (int)hash("Sysop"))
#endif


        /* to keep messages to sysop @ blah from being seen on local */

#ifdef NEWMSGTAB
        && !lmt->mtmsgflags.NET
#else
        && !msgTab_mtmsgflags[snot].NET
#endif

 )
        return(TRUE);

        /* aides see messages to 'Aide'           */

#ifdef NEWMSGTAB
        if ( aide && (lmt->mttohash == (int)hash("Aide"))
#else
        if ( aide && (msgTab_mttohash[snot] == (int)hash("Aide"))
#endif


        /* to keep messages to sysop @ blah from being seen on local */

#ifdef NEWMSGTAB
        && !lmt->mtmsgflags.NET
#else
        && !msgTab_mtmsgflags[snot].NET
#endif

 )
        return(TRUE);

#if 1
        /* sysops see messages from 'Sysop'           */

#ifdef NEWMSGTAB
        if ( sysop && (lmt->mtauthhash == (int)hash("Sysop")) )
#else
        if ( sysop && (msgTab_mtauthhash[snot] == (int)hash("Sysop")) )
#endif

        return(TRUE);

        /* aides see messages to 'Aide'           */

#ifdef NEWMSGTAB
        if ( aide && (lmt->mtauthhash == (int)hash("Aide")) )
#else
        if ( aide && (msgTab_mtauthhash[snot] == (int)hash("Aide")) )
#endif

        return(TRUE);
#endif


        /* none of those so cannot see message     */
        return(FALSE);
    }


#ifdef NEWMSGTAB
    if (lmt->mtmsgflags.LIMITED)
#else
    if (msgTab_mtmsgflags[snot].LIMITED)
#endif

    {
#ifdef GOODBYE
        /* sysop is god and can see all group only messages */
        if (strcmpi(cfg.sysop, logBuf.lbname) == SAMESTRING)
            return(TRUE);
#endif

        for (i = 0 ; i < MAXGROUPS; ++i)
        {
            /* check to see which group message is to */


#ifdef NEWMSGTAB
            if (((int)hash(grpBuf.group[i].groupname) == lmt->mttohash)
#else
            if (((int)hash(grpBuf.group[i].groupname) == msgTab_mttohash[snot])
#endif

                && (grpBuf.group[i].g_inuse))
            {
                return(ingroup(i));

#ifdef GOODBYE
                /* if in that group */
                if (logBuf.groups[i] == grpBuf.group[i].groupgen )
                {
                    return(TRUE);
                }
                else
                {
                    return(FALSE);
                }
#endif

            }
        } /* group can't see message, return false */
        return(sysop);
    }
    return(TRUE);
}

/* -------------------------------------------------------------------- */
/*  sizetable()     returns # messages in table                         */
/* -------------------------------------------------------------------- */
ulong sizetable(void)
{
    return (cfg.newest - cfg.mtoldest + 1);
}


/* -------------------------------------------------------------------- */
/*  indexmessage()  builds one message index from msgBuf                */
/* -------------------------------------------------------------------- */
void indexmessage(ulong here)
{
#ifndef NEWMSGTAB
    ushort slot;
#endif

    ulong copy;
    ulong oid;

#ifdef NEWMSGTAB
    struct messagetable *lmt;
    struct msgflags *lmf;
    
    lmt = getMsgTab(indexslot(here));
    lmf = &(lmt->mtmsgflags);
#endif


#ifndef NEWMSGTAB
    slot = indexslot(here);
#endif

    

#ifdef NEWMSGTAB
/*  msgTab2.mtmsgLoc            = (long)0; */

    lmt->mtmsgLoc            = 0;
/*   lmt->mtmsgLocHI            = 0;  */

    lmf->MAIL     = 0;
    lmf->RECEIVED = 0;
    lmf->REPLY    = 0;
    lmf->PROBLEM  = 0;
    lmf->MADEVIS  = 0;
    lmf->LIMITED  = 0;
    lmf->MODERATED= 0;
    lmf->RELEASED = 0;
    lmf->COPY     = 0;
    lmf->NET      = 0;
    lmf->FORWARDED= 0;
    lmf->NETWORKED= 0;

    lmt->mtauthhash  = 0;
    lmt->mttohash    = 0;
/*  msgTab7.mtfwdhash   = 0; */
/*  msgTab3.mtoffset    = 0; */
/*  msgTab9.mtorigin    = 0; */
    lmt->mtomesg     = /* (long) */ 0;

    lmt->mtroomno    = DUMP;

    /* --- */
    
/*  msgTab2[slot].mtmsgLoc    = msgBuf->mbheadLoc; */

    lmt->mtmsgLoc = msgBuf->mbheadLoc;

/*    lmt->mtmsgLocLO[slot]    = long_LO(msgBuf->mbheadLoc);  */
/*    lmt->mtmsgLocHI[slot]    = long_HI(msgBuf->mbheadLoc);  */


#else


/*  msgTab2[slot].mtmsgLoc            = (long)0; */

    msgTab_mtmsgLocLO[slot]            = 0;
    msgTab_mtmsgLocHI[slot]            = 0;

    msgTab_mtmsgflags[slot].MAIL     = 0;
    msgTab_mtmsgflags[slot].RECEIVED = 0;
    msgTab_mtmsgflags[slot].REPLY    = 0;
    msgTab_mtmsgflags[slot].PROBLEM  = 0;
    msgTab_mtmsgflags[slot].MADEVIS  = 0;
    msgTab_mtmsgflags[slot].LIMITED  = 0;
    msgTab_mtmsgflags[slot].MODERATED= 0;
    msgTab_mtmsgflags[slot].RELEASED = 0;
    msgTab_mtmsgflags[slot].COPY     = 0;
    msgTab_mtmsgflags[slot].NET      = 0;
    msgTab_mtmsgflags[slot].FORWARDED= 0;
    msgTab_mtmsgflags[slot].NETWORKED= 0;

    msgTab_mtauthhash[slot]  = 0;
    msgTab_mttohash[slot]    = 0;
/*  msgTab7[slot].mtfwdhash   = 0; */
/*  msgTab3[slot].mtoffset    = 0; */
/*  msgTab9[slot].mtorigin    = 0; */
    msgTab_mtomesg[slot]     = /* (long) */ 0;

    msgTab_mtroomno[slot]    = DUMP;

    /* --- */
    
/*  msgTab2[slot].mtmsgLoc    = msgBuf->mbheadLoc; */
    msgTab_mtmsgLocLO[slot]    = long_LO(msgBuf->mbheadLoc); 
    msgTab_mtmsgLocHI[slot]    = long_HI(msgBuf->mbheadLoc); 



#endif




/*  if (*msgBuf->mbsrcId)  */

     if (!(strcmpi(msgBuf->mbsrcId,  msgBuf->mbId) == SAMESTRING
             || msgBuf->mbsrcId[0]  == '\0' 
            ))

    {
        sscanf(msgBuf->mbsrcId, "%ld", &oid);

#ifdef NEWMSGTAB
        lmt->mtomesg = (unsigned int)oid;
        lmf->NETWORKED = 1;
#else
        msgTab_mtomesg[slot] = (unsigned int)oid;
        msgTab_mtmsgflags[slot].NETWORKED = 1;
#endif

    }


#ifdef NEWMSGTAB
    if (*msgBuf->mbauth)  lmt->mtauthhash =  hash(msgBuf->mbauth);
#else
    if (*msgBuf->mbauth)  msgTab_mtauthhash[slot] =  hash(msgBuf->mbauth);
#endif


    if (*msgBuf->mbto)
    {

#ifdef NEWMSGTAB
        lmt->mttohash   =  hash(msgBuf->mbto);    
#else
        msgTab_mttohash[slot]   =  hash(msgBuf->mbto);    
#endif


#ifdef NEWMSGTAB
        lmf->MAIL = 1;
#else
        msgTab_mtmsgflags[slot].MAIL = 1;
#endif


        if (*msgBuf->mbfwd)  
        {

#ifdef NEWMSGTAB
             lmf->FORWARDED = 1;
#else
             msgTab_mtmsgflags[slot].FORWARDED = 1;
#endif

        /*   msgTab7[slot].mtfwdhash = hash(msgBuf->mbfwd);  */

        /*     if (!*msgBuf->mbsrcId) */ /* if local message */

     if (strcmpi(msgBuf->mbsrcId,  msgBuf->mbId) == SAMESTRING
             || msgBuf->mbsrcId[0]  == '\0' 
            )

             {   /* use the mtomesg field */

#ifdef NEWMSGTAB
                 lmt->mtomesg = hash(msgBuf->mbfwd);
#else
                 msgTab_mtomesg[slot] = hash(msgBuf->mbfwd);
#endif

             }
             else
             {  /* use the mttohash and lose the recipient */

#ifdef NEWMSGTAB
                lmt->mttohash = hash(msgBuf->mbfwd);
#else
                msgTab_mttohash[slot] = hash(msgBuf->mbfwd);
#endif

             }
        }
    }

    if (*msgBuf->mbgroup)
    {

#ifdef NEWMSGTAB
        lmt->mttohash =  hash(msgBuf->mbgroup);
        lmf->LIMITED  = 1;
#else
        msgTab_mttohash[slot]   =  hash(msgBuf->mbgroup);
        msgTab_mtmsgflags[slot].LIMITED = 1;
#endif

    }

/*  if (*msgBuf->mboname)
      msgTab9[slot].mtorigin = hash(msgBuf->mboname); */

    if (strcmpi(msgBuf->mbzip, cfg.nodeTitle) != SAMESTRING && *msgBuf->mbzip)
    {

#ifdef NEWMSGTAB
        lmf->NET = 1;
#else
        msgTab_mtmsgflags[slot].NET = 1;
#endif


      /* what's the following line for? It screws up reading of forwarded */
      /* e-mail on the local system */
      /*  msgTab_mttohash[slot]       = hash(msgBuf->mbzip); */
    }



#ifdef NEWMSGTAB
    if (*msgBuf->mbx == 'Y')  lmf->PROBLEM = 1;
    if (*msgBuf->mbx == 'M')  lmf->MODERATED = 1;
#else
    if (*msgBuf->mbx == 'Y')  msgTab_mtmsgflags[slot].PROBLEM = 1;
    if (*msgBuf->mbx == 'M')  msgTab_mtmsgflags[slot].MODERATED = 1;
#endif

#ifdef NEWMSGTAB
    lmf->RECEIVED = 
#else
    msgTab_mtmsgflags[slot].RECEIVED = 
#endif
        ((msgBuf->mbattr & ATTR_RECEIVED) == ATTR_RECEIVED);


#ifdef NEWMSGTAB
    lmf->REPLY    = 
#else
    msgTab_mtmsgflags[slot].REPLY    = 
#endif
        ((msgBuf->mbattr & ATTR_REPLY   ) == ATTR_REPLY   );




#ifdef NEWMSGTAB
    lmf->MADEVIS  = 
#else
    msgTab_mtmsgflags[slot].MADEVIS  = 
#endif

        ((msgBuf->mbattr & ATTR_MADEVIS ) == ATTR_MADEVIS );

#ifdef NEWMSGTAB
    lmt->mtroomno = msgBuf->mbroomno;
#else
    msgTab_mtroomno[slot] = msgBuf->mbroomno;
#endif

    /* This is special. */
    if  (*msgBuf->mbcopy)
    {
#ifdef NEWMSGTAB
        lmf->COPY = 1;
#else
        msgTab_mtmsgflags[slot].COPY = 1;
#endif

        /* get the ID# */
        sscanf(msgBuf->mbcopy, "%ld", &copy);

        /* msgTab3[slot].mtoffset = (ushort)(here - copy); */
#ifdef NEWMSGTAB
/*        lmt->mtomesg = (uint)(here - copy);  */
/*        lmt->mtmsgLoc = (here - copy); */ 


    lmt->mttohash      = long_LO(here - copy); 
    lmt->mtauthhash    = long_HI(here - copy); 


#else
/*        msgTab_mtomesg[slot] = (uint)(here - copy);  */


/*    msgTab_mtmsgLocLO[slot]    = long_LO((uint)(here - copy));  */
/*    msgTab_mtmsgLocHI[slot]    = long_HI((uint)(here - copy));  */

    msgTab_mttohash[slot]      = long_LO((uint)(here - copy)); 
    msgTab_mtauthhash[slot]    = long_HI((uint)(here - copy)); 

#endif
    }

    if (roomBuild) buildroom();
}

