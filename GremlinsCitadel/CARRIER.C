/* -------------------------------------------------------------------- */
/*  CARRIER.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  Code to detect and report on Carrier.                               */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  haveCarrier()   do we have a connection, either console or modem?   */
/*  getModStr()     get a string from the modem, waiting for upto 3 secs*/
/*  carrier()       checks carrier                                      */
/*  carrdetect()    sets global flags for carrier detect                */
/*  carrloss()      sets global flags for carrier loss                  */
/*  checkCR()       Checks for CRs from the data port for half a second.*/
/*  checkring()     Checks for 'RING' or '2' from data port.            */
/*  findbaud()      Finds the baud from sysop and user supplied data.   */
/*  ringdetectbaud()    sets baud rate according to ring detect         */
/*  smartbaud()     sets baud rate according to result codes            */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*      checkCR()               Checks for CRs from the data port for half a second */
/* -------------------------------------------------------------------- */
 BOOL checkCR(void)
        {
        pause(50);
        if (MIReady() && (getMod() == '\r'))
                {
                return(TRUE);
                }
        return(FALSE);
        }

/* -------------------------------------------------------------------- */
/*  findbaud()      Finds the baud from sysop and user supplied data.   */
/* -------------------------------------------------------------------- */
 int findbaud(void)
        {
        BOOL good = FALSE;
        int time = 0;
        int baudRunner = 0;

        while (MIReady())
                {
                getMod();
                }

        while (gotCarrier() && !good && time < 120)
                {
                time++;
        baud(baudRunner);
                good = checkCR();
                if (!good)
                        {
                        baudRunner = (baudRunner + 1) % 6;
                        }
                }
        return(good);
        }

/* -------------------------------------------------------------------- */
/*  haveCarrier()   do we have a connection, either console or modem?   */
/* -------------------------------------------------------------------- */
BOOL haveCarrier(void)
{
    return (BOOL)((onConsole || (gotCarrier() && modStat)) && !ExitToMsdos);
}

/* -------------------------------------------------------------------- */
/*  carrdetect()    sets global flags for carrier detect                */
/* -------------------------------------------------------------------- */
void carrdetect(void)
{
    char temp[30];
    
    warned          = FALSE;
    modStat         = TRUE;

    time(&conntimestamp);

    /* screensaver junk only if fucking_stupid */
    if (cfg.fucking_stupid)
    {
        time(&keyboard_timer);
        if (cfg.screensave && saver_on)
        {
            saver_on = FALSE;
            restore_screen();
            curson();
        }
    }

    connectcls();
    update25();

    sprintf(temp, "Carrier-Detect (%u)", bauds[speed]);
    trap(temp,  T_CARRIER);

    logBuf.credits = cfg.unlogbal;
}

/* -------------------------------------------------------------------- */
/*  carrloss()      sets global flags for carrier loss                  */
/* -------------------------------------------------------------------- */
void carrloss(void)
{
    outFlag         = OUTSKIP;
    setio(whichIO, echo, outFlag);

    modStat         = FALSE;
    pause(100);
    /* Initport(); */  /* why was this commented out */
    disabled = TRUE;
    trap("Carrier-Loss", T_CARRIER);

#ifdef GOODBYE
    /* screensaver junk, wait before clearing the screen */
    time(&keyboard_timer);
#endif
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  checkCR()       Checks for CRs from the data port for half a second.*/
/* -------------------------------------------------------------------- */
BOOL checkCR(void)
{
    int i;

    for (i = 0; i < 50; i++)
    {
        pause(1);
        if (MIReady()) if (getMod() == '\r') return FALSE;
    }
    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  findbaud()      Finds the baud from sysop and user supplied data.   */
/* -------------------------------------------------------------------- */
int findbaud(void)
{
    char noGood = TRUE;
    int  Time = 0;
    int  baudRunner;                    /* Only try for 60 seconds      */

    while (MIReady())   getMod();               /* Clear garbage        */
    baudRunner = 0;
    while (gotCarrier() && noGood && Time < 120)
    {
        Time++;
        baud(baudRunner);
        noGood = checkCR();
        if (noGood) baudRunner = (baudRunner + 1) % (3 /* 2400 too */);
    }
    return !noGood;
}
#endif

/* -------------------------------------------------------------------- */
/*  ringdetectbaud()    sets baud rate according to ring detect         */
/* -------------------------------------------------------------------- */
void ringdetectbaud(void)
{
    baud(ringdetect());
}

static char *smart_response[] = {                 
        "CONNECT"     ,  "1",       /* 0,   1     */       
        "CONNECT 1200",  "5", "15", /* 2,   3,  4 */
        "CONNECT 2400", "10", "16", /* 5,   6,  7 */
        "CONNECT 9600", "13", "17", /* 8,   9, 10 */
        "NO CARRIER",    "3",       /* 11, 12 */
        "ERROR",         "4",       /* 13, 14 */
        "NO DIALTONE",   "6",       /* 15, 16 */
        "BUSY",          "7",       /* 17, 18 */
        "NO ANSWER",     "8",       /* 19, 20 */
        "CONNECT 12000",            /* 21 */
        "CONNECT 14400",            /* 22 */
        "CONNECT 19200",            /* 23 */
        "CONNECT 38400",            /* 24 */
        "CONNECT 57600",            /* 25 */
        NULL
};


/* -------------------------------------------------------------------- */
/*  smartbaud()   sets baud rate according to result codes              */
/*                returns ERROR if result code is an error code         */
/* -------------------------------------------------------------------- */
int smartbaud(void)
{
    char str[42]; /* for baud detect */
    int i   ;
    
    while (getModStr(str)  /* && gotCarrier() */)
    {
        for (i = 0; smart_response[i] != NULL; i++)
        {
            if (strcmpi(str, smart_response[i]) == SAMESTRING)
            {
                switch (i)
                {
                case 0: /* Connect */
                case 1:
                    baud(0);
                    strcpy(modem_result, "Connect");
                    return(TRUE);
                case 2: /* Connect 1200 */
                case 3:
                case 4:
                    baud(1);
                    strcpy(modem_result, "Connect 1200");
                    return(TRUE);
                case 5: /* Connect 2400 */
                case 6:
                case 7:
                    baud(2);
                    strcpy(modem_result, "Connect 2400");
                    return(TRUE);
                case 8: /* Connect 9600 */
                case 9:
                case 10:
                    baud(4);
                    strcpy(modem_result, "Connect 9600");
                    return(TRUE);

                /* Bunch of errors, for networking purposes */
                case 11:  /* No Carrier */
                case 12:
                    strcpy(modem_result, "No Carrier");
                    return(ERROR);
                case 13:  /* Error */
                case 14:
                    strcpy(modem_result, "Error");
                    return(ERROR);
                case 15:  /* No Dialtone */
                case 16:
                    strcpy(modem_result, "No Dialtone");
                    return(ERROR);
                case 17:  /* Busy */
                case 18:
                    strcpy(modem_result, "Busy");
                    return(ERROR);
                case 19:  /* No Answer */
                case 20:
                    strcpy(modem_result, "No Answer");
                    return(ERROR);
                case 21:
                    strcpy(modem_result, "Connect 12000");
                    return(TRUE);
                case 22:
                    strcpy(modem_result, "Connect 14400");
                    return(TRUE);
                case 23:
                    strcpy(modem_result, "Connect 19200");
                    return(TRUE);
                case 24:
                    strcpy(modem_result, "Connect 38400");
                    return(TRUE);
                case 25:
                    strcpy(modem_result, "Connect 57600");
                    return(TRUE);
                }
            }
        }
    }
    return (FALSE);
}

/* -------------------------------------------------------------------- */
/*  getModStr()     get a string from the modem, waiting for upto 3 secs*/
/*                  for it. Returns TRUE if it gets one.                */
/* -------------------------------------------------------------------- */
int getModStr(char *str)
{
    long tm;
    int  l = 0, c;

    time(&tm);

    if (debug) cPrintf("[");

    while (
             (time(NULL) - tm) < 31 
          && !KBReady() 
          && l < 40 
          )
    {
        if (MIReady())
        {
            c = getMod();

            if (c == 13 || c == 10) /* CR || LF */
            {
                str[l] = '\0' /* NULL */;
                if (debug) cPrintf("]\n");
                return TRUE;
            }
            else
            {
                if (debug) cPrintf("%c", c);
                str[l] = (char)c;
                l++;
            }
        }
    }

    if (debug) cPrintf(":F]\n");

    str[0] = '\0' /* NULL */;

    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  checkring()       checks for 'RING' or '2'                          */
/*                    and sends answer string to modem                  */
/* -------------------------------------------------------------------- */
void checkring(void)
{
    char str[42]; /* For Ring detect */
    char send_ata = 0;

    if (MIReady())
    {
        /* don't be interrupted by cron events */
        started = time(NULL);

        /* make this pause as small as possible */
        pause(25);

        /* to deal with the time from "CONNECT" to gotcarrier() being true */
        if (gotCarrier())
            return;

        getModStr(str);

        if ( (strcmp(str, "RING") == SAMESTRING) ||
             (strcmp(str, "2"   ) == SAMESTRING)     )
            send_ata = 1;
    }

    if (send_ata)
    {
        pause(50);
        outstring(cfg.dialring);
        outstring("\r");
    }
}

/* -------------------------------------------------------------------- */
/*  carrier()       checks carrier                                      */
/* -------------------------------------------------------------------- */
int carrier(void)
{
    unsigned char c;
                       /* ignore carrier according to uphours & updays */
    if (disabled || (!(cfg.uphours[hour()] && cfg.updays[dayofweek()])
                     && !debug)

       )  
    {
        Mflush();
        return FALSE;
    }

    /* detect  'RING' or '2' and send cfg.dialstring */
    if (/* cfg.dumbmodem == 3 */ cfg.dialring[0])
        checkring();

    if (gotCarrier())
    {
        switch(cfg.dumbmodem)
        {
        case 0:    /* smartbaud! */
        case 3:    /* smartbaud! dial_ring */
            c = (unsigned char)smartbaud();
            break;

        case 1:     /* returns */
            c = (unsigned char)findbaud();
            break;

        case 2:     /* HS on RI */
            ringdetectbaud();
            c = TRUE;
            break;

        default:
        case 4:     /* forced */
            baud(cfg.initbaud);
            c = TRUE;
            break;

        case 5:     /* forced */
            c = (unsigned char)smartbaud();
            if (c)
            {
                if (speed > 2)
                    baud(cfg.initbaud);
            }

            c = TRUE;
            break;

        case 6:     /* forced */
            c = (unsigned char)smartbaud();
            if (c)
            {
                if (speed > 2)
                    baud(cfg.initbaud);
                else
                {
                    /* 300, 1200, 2400 */
                    dumbmodemHangup();
                    outstring(cfg.downshift);
                    outstring("\r");
                }
            }

            c = TRUE;
            break;
        }

        if (c == TRUE)
        {
            update25();
            carrdetect();
            update25();
            return(TRUE);
        }
        else
        {
            update25();
            Initport();
            return(FALSE);
        }
    } 
    else
    {
        return FALSE;
    }
}
 

