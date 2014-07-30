/* -------------------------------------------------------------------- */
/*  MODEM.C                  Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  High level modem code, should not need to be changed for porting(?) */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  domcr()         print cr on modem, nulls and lf's if needed         */
/*  offhook()       sysop fn: to take modem off hook                    */
/*  outstring()     push a string directly to the modem                 */
/*  Mflush()        Flush garbage characters from the modem.            */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  digitbaud()     Calculates digit baud rate (0-4) from number (300+) */
/* -------------------------------------------------------------------- */
int digitbaud(uint bigbaud)
{
    int i;

    switch (bigbaud)
    {
        case   300:    i = 0;             break;
        case  1200:    i = 1;             break;
        case  2400:    i = 2;             break;
        case  4800:    i = 3;             break;
        case  9600:    i = 4;             break;
        case 19200:    i = 5;             break;
        case 38400U:   i = 6;             break;
        case 57600U:   i = 7;             break;
        default:       i = cfg.initbaud;  break;
    }
    return i;
}

/* -------------------------------------------------------------------- */
/*  domcr()         print cr on modem, nulls and lf's if needed         */
/* -------------------------------------------------------------------- */
void domcr(void)
{
    int i;

    outMod('\r');
    for (i = termNulls;  i;  i--) outMod(0);
    if (termLF) outMod('\n');
}

/* -------------------------------------------------------------------- */
/*  offhook()       sysop fn: to take modem off hook                    */
/* -------------------------------------------------------------------- */
void offhook(void)
{
    Initport();
    outstring("ATM0H1\r");
    disabled = TRUE;
}

/* -------------------------------------------------------------------- */
/*  outstring()     push a string directly to the modem                 */
/* -------------------------------------------------------------------- */
void outstring(char *string)
{
    int mtmp;

    mtmp = modem;
    modem = TRUE;

    while(*string)
    {
        outMod(*string++);  /* output string */
    }

    modem = (uchar)mtmp;
}

/* -------------------------------------------------------------------- */
/*  Mflush()        Flush garbage characters from the modem.            */
/* -------------------------------------------------------------------- */
void Mflush(void)
{
    while (MIReady())
        getMod();
}

/* -------------------------------------------------------------------- */
/*      Hangup() breaks the modem connection                            */
/* -------------------------------------------------------------------- */
void Hangup(void)
{
    if (!slv_door)
        pHangup();
    else
        modStat = FALSE;    /* hangup simulation ... */
}

