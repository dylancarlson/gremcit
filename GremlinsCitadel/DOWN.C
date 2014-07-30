/* -------------------------------------------------------------------- */
/*  DOWN.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  Code to bring citadel down                                          */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  crashout()      Fatal system error                                  */
/*  exitcitadel()   Done with cit, time to leave                        */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  08/04/90    (PAT)   File created                                    */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data definitions                                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  crashout()      Fatal system error                                  */
/* -------------------------------------------------------------------- */
void crashout(char *message)
{
    FILE *fd;           /* Record some crash data */

    Hangup();

    fcloseall();

    fd = fopen("crash.cit", "w");
    fprintf(fd, message);
    fclose(fd);

    writeTables();

    cfg.attr = 7;   /* exit with white letters */

    position(0,0);
    cPrintf("Fatal System Crash: %s\n", message);

    drop_dtr();

    portExit();

    freeTables();
    
    exit(199);
}

/* -------------------------------------------------------------------- */
/*  exitcitadel()   Done with cit, time to leave                        */
/* -------------------------------------------------------------------- */
void exitcitadel(void)
{

    if (loggedIn)
    {
        verbose = FALSE;
        terminate( /* hangUp == */ TRUE);
    }
    if (!slv_door)
    {
        drop_dtr();   /* turn DTR off */
    }

    putGroup();       /* save group table */
    putHall();        /* save hall table  */

    writeTables(); 

    trap("Citadel Terminated", T_SYSOP);

    /* close all files */
    fcloseall();

    cfg.attr = 7;   /* exit with white letters */
    cls();

    portExit();

    freeTables();

#ifdef GOODBYE
    if (gmode() != 7)
    {
        outp(0x3d9,0);
    }
#endif

    exit(return_code);
}


