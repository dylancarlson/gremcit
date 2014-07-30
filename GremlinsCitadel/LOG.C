/* -------------------------------------------------------------------- */
/*  LOG.C                    Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                           Local log code                             */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  findPerson()    loads log record for named person.                  */
/*                  RETURNS: ERROR if not found, else log record #      */
/*  personexists()  returns slot# of named person else ERROR            */
/*  setlogconfig()  this sets the configuration in current logBuf equal */
/*                  to the global configuration variables               */
/*  setsysconfig()  this sets the global configuration variables equal  */
/*                  to the the ones in logBuf                           */
/*  storeLog()      stores the current log record.                      */
/*  displaypw()     displays callers name, initials & pw                */
/*  normalizepw()   This breaks down inits;pw into separate strings     */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  06/14/89    (PAT)   Created from LOG.C to move some of the system   */
/*                      out of memory. Also cleaned up moved code to    */
/*                      -W3, ext.                                       */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  findPerson()    loads log record for named person.                  */
/*                  RETURNS: ERROR if not found, else log record #      */
/* -------------------------------------------------------------------- */
int findPerson(char *name, struct logBuffer *lbuf)
{
    int slot, logno;

    slot = personexists(name);

    if (slot == ERROR) return(ERROR);

    getLog(lbuf, logno = logTab[slot].ltlogSlot);

    return(logno);
}

/* -------------------------------------------------------------------- */
/*  personexists()  returns slot# of named person else ERROR            */
/* -------------------------------------------------------------------- */
int personexists(char *name)
{
    int i, namehash;
    struct logBuffer logRead;

    namehash = hash(name);

    /* check to see if name is in log table */

    for ( i = 0;  i < cfg.MAXLOGTAB;  i++)
    {
        if (namehash == logTab[i].ltnmhash)
        {
            getLog(&logRead, logTab[i].ltlogSlot);

            if (strcmpi(name, logRead.lbname) == SAMESTRING)
                return(i);
        }
    }

    return(ERROR);
}

/* -------------------------------------------------------------------- */
/*  setlogconfig()  this sets the configuration in current logBuf equal */
/*                  to the global configuration variables               */
/* -------------------------------------------------------------------- */
void setlogconfig(void)
{
    logBuf.lbwidth           = termWidth;
    logBuf.lbnulls           = termNulls;
    logBuf.lbflags.EXPERT    = expert;
    logBuf.lbflags.UCMASK    = termUpper;
    logBuf.lbflags.LFMASK    = termLF;
    logBuf.lbflags.AIDE      = aide;
    logBuf.lbflags.SYSOP     = sysop;
    logBuf.lbflags.TABS      = termTab;
    logBuf.lbflags.PROBLEM   = twit;
    logBuf.lbflags.UNLISTED  = unlisted;
    logBuf.lbflags.OLDTOO    = oldToo;
    logBuf.lbflags.ROOMTELL  = roomtell;
    /* strcpy(logBuf.tty, term.name); */
}

/* -------------------------------------------------------------------- */
/*  setsysconfig()  this sets the global configuration variables equal  */
/*                  to the the ones in logBuf                           */
/* -------------------------------------------------------------------- */
void setsysconfig(void)
{
    termWidth   = logBuf.lbwidth;
    termNulls   = logBuf.lbnulls;
    termLF      = (BOOL)logBuf.lbflags.LFMASK ;
    termUpper   = (BOOL)logBuf.lbflags.UCMASK ;
    expert      = (BOOL)logBuf.lbflags.EXPERT ;
    aide        = (BOOL)logBuf.lbflags.AIDE   ;
    sysop       = (BOOL)logBuf.lbflags.SYSOP  ;
    termTab     = (BOOL)logBuf.lbflags.TABS   ;
    oldToo      = (BOOL)logBuf.lbflags.OLDTOO ;
    twit        = (BOOL)logBuf.lbflags.PROBLEM;
    unlisted    = (BOOL)logBuf.lbflags.UNLISTED;
    roomtell    = (BOOL)logBuf.lbflags.ROOMTELL;
    backout     = (BOOL)logBuf.PSYCHO;
    setlogTerm();
}

/* -------------------------------------------------------------------- */
/*  storeLog()      stores the current log record.                      */
/* -------------------------------------------------------------------- */
void storeLog(void)
{
    /* make log configuration equal to our environment */
    setlogconfig();

    if (loggedIn)
    putLog(&logBuf, thisLog);
}

/* -------------------------------------------------------------------- */
/*  displaypw()     displays callers name, initials & pw                */
/* -------------------------------------------------------------------- */
void displaypw(char *name, char *in, char *pw)
{
    mPrintf("\n nm: %s",name);
    mPrintf("\n in: ");
    echo = CALLER;
    setio(whichIO, echo, outFlag);
    mPrintf("%s", in);
    echo = BOTH;
    setio(whichIO, echo, outFlag);
    mPrintf("\n pw: ");
    echo = CALLER;
    setio(whichIO, echo, outFlag);
    mPrintf("%s",pw);
    echo = BOTH;
    setio(whichIO, echo, outFlag);

    doCR();
}

/* -------------------------------------------------------------------- */
/*  normalizepw()   This breaks down inits;pw into separate strings     */
/* -------------------------------------------------------------------- */
void normalizepw(char *InitPw, char *Initials, char *passWord)
{
    char *pwptr;
    char *inptr;
    char *inpwptr;

    inpwptr = InitPw;
    pwptr   = passWord;
    inptr   = Initials;

    while (*inpwptr != ';')
    {
        *inptr++ = *inpwptr;
        inpwptr++;
    }
    *inptr++ = '\0';  /* tie off with a null */

    inpwptr++;   /* step over semicolon */

    while (*inpwptr != '\0')
    {
        *pwptr++ = *inpwptr;
        inpwptr++;
    }
    *pwptr++ = '\0';  /* tie off with a null */

    normalizeString(Initials);
    normalizeString(passWord);

    /* dont allow anything over 19 characters */
    Initials[19] = '\0';
    passWord[19] = '\0';
}

/* -------------------------------------------------------------------- */
/*  log2tab()       logbuf to logtable converter                        */
/* -------------------------------------------------------------------- */
void log2tab(struct lTable *lt, struct logBuffer *lb)
{
    lt->ltflags = lb->lbflags;
    
    if (lb->lbflags.L_INUSE)
    {
        lt->ltinhash = hash(lb->lbin);
        lt->ltpwhash = hash(lb->lbpw);
        lt->ltnmhash = hash(lb->lbname);
        
        lt->ltcallno = lb->callno;
    }
    else
    {
        lt->ltinhash = 0;
        lt->ltpwhash = 0; 
        lt->ltnmhash = 0; 
                      
        lt->ltcallno = 0; 
    }
    
    /*
     * slot not taken care of.. 
     */
}

