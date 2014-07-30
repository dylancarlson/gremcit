/* -------------------------------------------------------------------- */
/*  TRAP.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  Trap file and aide message code                                     */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  trap()          Record a line to the trap file                      */
/*  SaveAideMess()  Save aide message from AIDEMSG.TMP                  */
/*  amPrintf()      aide message printf                                 */
/*  amZap()         Zap aide message being made                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  trap()          Record a line to the trap file                      */
/* -------------------------------------------------------------------- */
void trap(char *string, int what)
{
    char dtstr[20];
    char temp[128];

    /* check to see if we are supposed to log this event */
    if (!cfg.trapit[what])  return;

    strftime(dtstr, 19, "%y%b%D %X", 0l); 

    if (strlen(string) < 128)
    {
        strcpy(temp, string);
        stripansi(temp);
    }

    fprintf(trapfl, "%s:  %s\n", dtstr,(strlen(string) < 128) ? temp : string);

    fflush(trapfl);
}

/* -------------------------------------------------------------------- */
/*  SaveAideMess()  Save aide message from AIDEMSG.TMP                  */
/* -------------------------------------------------------------------- */
void SaveAideMess(void)
{
    char temp[90];
    FILE *fd;

    /*
     * Close curent aide message (if any)
     */
    if (aideFl == NULL)
    {
        return;
    }
    fclose(aideFl);
    aideFl = NULL;

    clearmsgbuf();

    /*
     * Read the aide message
     */
    sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");
    if ((fd  = fopen(temp, "rb")) == NULL)
    {
        crashout("AIDEMSG.TMP file not found during aide message save!");
    }
    GetFileMessage(fd, msgBuf->mbtext, MAXTEXT);

    fclose(fd);
    unlink(temp);

    if (strlen(msgBuf->mbtext) < 10)
        return;

    strcpy(msgBuf->mbauth, cfg.nodeTitle);  

    msgBuf->mbroomno = AIDEROOM;

    putMessage();
    noteMessage();
}

/* -------------------------------------------------------------------- */
/*  amPrintf()      aide message printf                                 */
/* -------------------------------------------------------------------- */
void amPrintf(char *fmt, ... )
{
    va_list ap;
    char temp[90];

    /* no message in progress? */
    if (aideFl == NULL)
    {
        sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");

        unlink(temp);
 
        if ((aideFl = fopen(temp, "w")) == NULL)
        {
            crashout("Can not open AIDEMSG.TMP!");
        }
    }

    va_start(ap, fmt);
    vfprintf(aideFl, fmt, ap);
    va_end(ap);

    fflush(aideFl);
}

/* -------------------------------------------------------------------- */
/*  amZap()         Zap aide message being made                         */
/* -------------------------------------------------------------------- */
void amZap(void)
{
    char temp[90];

    if (aideFl != NULL)
    {
        fclose(aideFl);
    }

    sprintf(temp, "%s\\%s", cfg.temppath, "aidemsg.tmp");

    unlink(temp);

    aideFl = NULL;
}


