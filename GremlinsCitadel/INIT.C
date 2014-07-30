/* -------------------------------------------------------------------- */
/*  INIT.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  INIT Citadel code                                                   */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  initCitadel()   Load up data files and open everything.             */
/*  openFile()      Special to open a .cit file                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  initCitadel()   Load up data files and open everything.             */
/* -------------------------------------------------------------------- */
void initCitadel(void)
{
    static char prompt[92];
    char *envprompt;
    char read_it;

    char *grpFile, *hallFile, *logFile, *msgFile, *roomFile;
    char scratch[80];

    setscreen();
    logo();
    
    if (time(NULL) < 607415813L)
    {
        cPrintf("\n\nPlease set your time and date!\n");
        exit(200);
    }

    update25();

    whichIO = CONSOLE;
    outFlag = OUTOK;
    echo    = BOTH;
    setio(whichIO, echo, outFlag);

    
    asciitable();

    envprompt = getenv("PROMPT");
    sprintf(prompt, "PROMPT=%s[Cit] ", envprompt);
    putenv(prompt);

    cfg.version = cfgver;       /* initialize it for comparison later */
    
    if (!(read_it = readTables()) || (cfg.version != cfgver))
    {
        if ((reconfig || (cfg.version != cfgver)) || batchmode)  
        {
            pause(200);
            cls();

            if (read_it && (cfg.version != cfgver))
            {
                freeTables();
            }

            cCPrintf("Reconfiguring Citadel."); doccr();
            configcit();
            readcron();  /* read it when starting new */
        }
        else
        {
            cPrintf("\n The ETC.DAT file was not found, probably due to "
                    "the computer crashing\n"
                    "or another copy of Citadel being run in a multitasking "
                    "environment.\n\n"
                    " If you're sure that you aren't running another copy "
                    "of Citadel, rerun it\n"
                    "with the -C switch on the Citadel command line "
                    "to reconfigure.\n");
            exit(1);
        }
    }
    else
    if (readconfigcit)
        readconfig(1);   /* forced to read in config.cit */

    if (cfg.f6pass[0])
    {
        ConLock = TRUE;
    }

    portInit();

    setscreen();

    logo();

    update25();
    
    if (cfg.msgpath[ (strlen(cfg.msgpath) - 1) ]  == '\\')
        cfg.msgpath[ (strlen(cfg.msgpath) - 1) ]  =  '\0';

    sprintf(scratch, "%s\\%s", cfg.msgpath, "msg.dat");

    /* open message files: */
    grpFile     = "grp.dat" ;
    hallFile    = "hall.dat";
    logFile     = "log.dat" ;
    msgFile     =  scratch  ;
    roomFile    = "room.dat";
    
    /* move to home-path */
    changedir(cfg.homepath);

    openFile(grpFile,  &grpfl );
    openFile(hallFile, &hallfl);
    openFile(logFile,  &logfl );
    openFile(msgFile,  &msgfl );
    openFile(roomFile, &roomfl);

    /* open Trap file */
    trapfl = fopen(cfg.trapfile, "a+");

    trap("Citadel Started", T_SYSOP);

    getGroup();
    getHall();

    if (cfg.accounting)
    {
        readaccount();    /* read in accounting data */
    }
    readprotocols();

    getRoom(LOBBY);     /* load Lobby>  */
    checkdir();

    if (!slv_door)
    {
        Initport();
        Initport(); 
    }
    whichIO = MODEM;
    setio(whichIO, echo, outFlag);

    /* record when we put system up */
    time(&uptimestamp);

    setdefaultconfig();
    setalloldrooms();
    roomtalley();
}

/* -------------------------------------------------------------------- */
/*  openFile()      Special to open a .cit file                         */
/* -------------------------------------------------------------------- */
void openFile(char *filename, FILE **fd)
{
    char str[80];
    
    /* open message file */
    if ((*fd = fopen(filename, "r+b")) == NULL)
    {
        sprintf(str, "%s file missing! (%d / %s)", filename, errno);
        crashout(str);
    }
}


