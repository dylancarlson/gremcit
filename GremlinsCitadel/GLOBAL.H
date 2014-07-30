/****************************************************************************/
/*  GLOBAL.H                                                                */
/*  All the global variables in GremCit.                                    */
/****************************************************************************/
#ifndef EXTERN
#define EXTERN extern
#endif

#ifdef MAIN

EXTERN char version[]       = "3.20A/059";
EXTERN long cfgver          = 114L;
EXTERN char programName[]   = "Gremlin's Citadel";
EXTERN char compilerName[]  = "Borland C++ 3.00"; 
EXTERN char copyright[]     = "Copyright (c) 1992 Anticlimactic Teleservices Ltd."  ;
EXTERN char cmpDate[]       = __DATE__; 
EXTERN char cmpTime[]       = __TIME__; 

EXTERN char   verbose = FALSE;
EXTERN char   reverse = FALSE;

EXTERN BOOL    lst_space = FALSE;
EXTERN BOOL    lst_rtn = FALSE;

EXTERN long keyboard_timer = 0L; /* screensaver */
EXTERN char saver_on = FALSE;

/* FILES */
EXTERN FILE                    *msgfl;             /* msg file descriptor          */   
EXTERN FILE                    *printfile;         /* printer.out                  */
EXTERN FILE                    *trapfl;            /* trap file descriptor         */
EXTERN FILE                    *aideFl = NULL;     /* a temp file for aide messages */
EXTERN FILE                    *hallfl;            /* Hall file descriptor         */     
EXTERN FILE                    *grpfl;             /* Group file descriptor        */     
EXTERN FILE                    *roomfl;            /* Room file descriptor         */
EXTERN FILE                    *logfl;             /* log file descriptor          */

/* STRUCTURES */
EXTERN struct accounting       thisAccount;    /* Groupdata used for current user  */
EXTERN struct accountBuffer    accountBuf;         /* Accounting buffer            */
EXTERN struct mfilter          mf;                 /* Message filter               */
EXTERN struct nodest           node;               /* node buffer                  */
EXTERN label                   editcmd;            /* External editor commands     */
EXTERN struct ext_editor       *edit;              /* External editor commands     */
EXTERN struct fInfo            *fileinfo;          /* pointer for file-info struct */
EXTERN struct groupBuffer      grpBuf;             /* Group data goes here         */
EXTERN struct hallBuffer       *hallBuf;           /* Hall stuff right here        */ 
EXTERN struct logBuffer        logBuf;             /* Log buffer of a person       */
EXTERN struct logBuffer        lBuf;               /* Temp buffer...........       */
EXTERN struct logBuffer        *lBuf2;             /* Temp buffer...........       */
EXTERN struct config           cfg;                /* A buncha variables           */ 
EXTERN struct lTable           *logTab;            /* RAM index of pippuls         */ 

#ifndef NEWMSGTAB
EXTERN struct msgflags *msgTab_mtmsgflags;  /* every message gets flags               */
EXTERN int             *msgTab_mtmsgLocLO;  /* offset where message starts            */
EXTERN int             *msgTab_mtmsgLocHI;  /* offset where message starts            */
EXTERN uchar           *msgTab_mtroomno;    /* room # of message                      */
EXTERN int             *msgTab_mttohash;    /* hash of recipient or group             */
EXTERN int             *msgTab_mtauthhash;  /* hash of author of message              */
EXTERN uint            *msgTab_mtomesg;     /* LO net id, forwardee hash, copy offset */
#endif

EXTERN struct aRoom            roomBuf;            /* Room buffer                  */
EXTERN struct msgB             *msgBuf;            /* The -sole- message buffer    */   
EXTERN struct msgB             *msgBuf2;           /* The Old-message buffer       */   
EXTERN struct talleyBuffer     *talleyBuf;         /* message talley for rooms     */
EXTERN struct fDir             *filedir;           /* pointer for filedir struct   */
EXTERN TERMINAL                term;


#ifdef NEWMSGTAB
EXTERN int                     *roomPos;
#else
EXTERN uchar                   *roomPos;
#endif

EXTERN char                    extrncmd[MAXEXTERN];/* External command letters     */
EXTERN struct ext_prot         *extrn;   /* External protocols buffer    */
EXTERN struct ext_door         *doors;  /* External commands buffer     */
EXTERN struct rTable           *roomTab;  /* RAM index of rooms           */ 

/* Function pointers */
EXTERN void                    (*charattr)(char, uchar);
EXTERN void                    (*stringattr)(uint, char *, uchar);

/* OTHER */
EXTERN char dialout_fkey; /* function keys in dialout mode */

EXTERN time_t                  started; /* for cron */
EXTERN char                    specialTime;    /* Is it a "special" hour           */
EXTERN float                   startbalance;   /* Number of credits at log-in      */
EXTERN long                    lasttime;       /* timestamp of last update         */
EXTERN char                    readMessage = TRUE;
EXTERN char                    ExitToMsdos = FALSE;  /*  bring system down         */
EXTERN char                    sysReq = FALSE;       /*  SysOp wants system        */
EXTERN char                    *confirm = "Confirm";
EXTERN char                    mailFlag;           /* Sending private mail?        */
EXTERN char                    oldFlag;            /* Entering old message?        */
EXTERN char                    limitFlag;          /* Entering Limited-Access      */
EXTERN char                    chatReq = FALSE;    /* Did they want to chat?       */
EXTERN char                    dowhat = DUNO;      /* What is calling iChar        */
EXTERN int                     duplicate;
EXTERN int                     expired; /* count duplicates and expired */
EXTERN char                    netError = FALSE;   /* save net error message?      */
EXTERN unsigned char           thisHall;           /* hall we're in                */
EXTERN int                     infolength;         /* size of info-file            */
EXTERN int                     thisLog;            /* entry currently in logBuf    */
EXTERN int                     thisRoom = LOBBY;   /* Current room                 */
EXTERN char                    etcpath[64];        /* Path etc.dat and ctdl.exe in */     
EXTERN int                     thisSlot;           /* logTab slot logBuf found via */     
EXTERN char                    loggedIn = FALSE;   /* Global have-caller flag      */     
EXTERN char                    prevChar;           /* for EOLN/EOParagraph stuff   */     
EXTERN char                    expert;             /* true to suppress hints etc.  */     
EXTERN char                    termUpper;          /* uppercase only flag          */     
EXTERN char                    termLF;             /* LF-after-CR flag             */     
EXTERN char                    aide;               /* aide privileges flag         */     
EXTERN char                    sysop;              /* sysop privileges flag        */     
EXTERN char                    twit;               /* Problem user?                */     
EXTERN char                    unlisted;           /* List in userlog?             */     
EXTERN char                    termTab;            /* Tabs                         */     
EXTERN char                    oldToo;             /* Send last old on new request?*/     
EXTERN char                    roomtell;           /* Print room descriptions?     */     
EXTERN unsigned char           termWidth;          /* width to format output to    */     
EXTERN unsigned char           termNulls;          /* # nulls to send at EOLN      */     
EXTERN int                     people;             /* this is the old slot         */
EXTERN char                    sysopNew = FALSE;                                         
EXTERN long                    uptimestamp;
EXTERN long                    logtimestamp;
EXTERN long                    conntimestamp;
EXTERN ulong                   transmitted;     /* how many characters transmitted */
EXTERN ulong                   received;        /* how many characters transmitted */
EXTERN char                    backout = FALSE; /* Words out backward?       */
EXTERN unsigned char           modStat = FALSE; /* Had carrier LAST time you checked */   
EXTERN char                    whichIO = CONSOLE; /* CONSOLE or MODEM            */
EXTERN char                    echo;          /* Either NEITHER, CALLER, or BOTH */
EXTERN char                    echoChar; /* What to echo with if echo equals NEITHER */
EXTERN char                    anyEcho = TRUE;
EXTERN char                    warned;
EXTERN char                    sleepkey = FALSE;
EXTERN char                    sysopkey = FALSE;                                                      
EXTERN char                    chatkey  = FALSE;                                                      
EXTERN char                    eventkey = FALSE;                                                      
EXTERN char                    disabled;                 /* is modem disabled?   */
EXTERN char                    speed;
EXTERN char                    tfilter[256];
EXTERN uchar                   outFilter[256];
EXTERN char                    modem = TRUE;
EXTERN char                    printing = FALSE;
EXTERN char                    console = TRUE;        /* output flag for console */
EXTERN char                    ConLock = FALSE;
EXTERN char                    debug = FALSE;
EXTERN char                    callout = FALSE;
EXTERN unsigned char           crtColumn;          /* current position on screen  */   
EXTERN char                    outFlag = OUTOK;    /* will be one of the above    */   
EXTERN char                    dotoMessage = NO_SPECIAL;                               
EXTERN ulong                   markedMId   = 0l;   /* id of message to be marked  */   
EXTERN char                    heldMessage;                                            
EXTERN char                    replyFlag;          /* Sending a reply?            */   
EXTERN char                    linkMess;           /* Sending a file-linked message? */
EXTERN int                     mread;
EXTERN int                     entered;     /* #messages read and entered  */   
                                                                                
EXTERN ulong                   originalId = 0l;    /* this is the ID of origin      */ 
EXTERN unsigned char           originalattr;       /* Attribute byte of origin      */ 
EXTERN char                    copyflag = FALSE;   /* true when viewing a duplicate */ 
EXTERN char                    MessageRoom[MAXROOMS];  /* number of message/room this  
                                                  call */                       
EXTERN char                    seen;                                                   
EXTERN uint                    bauds[]  =  { 300, 1200, 2400, 4800, 9600, 19200, 38400U, 57600U};
EXTERN char                    showdir;
EXTERN char                    showhidden; 
EXTERN char                    showgroup;
EXTERN char                    skiproom = TRUE;
EXTERN int                     oldroom;
EXTERN ulong                   ug_newpointer; 
EXTERN ulong                   ug_new;
EXTERN int                     ug_bypass;
EXTERN int                     ug_hall;
EXTERN uchar                   scrollpos = 23;    /* adjusts for help menu         */
EXTERN uchar                   ansiattr;
EXTERN char                    heldMessage;
EXTERN char                    replyFlag;          /* Sending a reply?            */
EXTERN char                    linkMess;           /* Sending a file-linked message? */
EXTERN int                     mread;
EXTERN int                     entered;     /* #messages read and entered  */
EXTERN unsigned char           originalattr;       /* Attribute byte of origin      */
EXTERN char                    readconfigcit = FALSE; /* Force read of config.cit?   */

EXTERN char       grpZap  = FALSE;
EXTERN char       hallZap = FALSE;
EXTERN char       msgZap  = FALSE;
EXTERN char       logZap  = FALSE;
EXTERN char       roomZap = FALSE;

EXTERN char   roomBuild = FALSE;               /* Rebuild room-file?           */
EXTERN char   hallBuild = FALSE;               /* Rebuild hall-file?           */

EXTERN char   gprompt[80];

EXTERN uchar               numDoors;         /* number of other commands */

EXTERN uchar               conRows = 24;       /* 1 less than the rows */
EXTERN uchar               conCols = 80;
EXTERN int                 conMode = -1;

EXTERN BOOL                slv_door = FALSE; /* used to start citadel as a slave */
EXTERN uint                slv_baud = UINT_ERROR;
EXTERN BOOL                slv_net  = FALSE;
EXTERN label               slv_node;
EXTERN int                 return_code = 0;

EXTERN BOOL                login_user = FALSE; /* login user name from command line */
EXTERN BOOL                login_pw = FALSE;   /* login user using in and pw */


EXTERN char                cmd_login[NAMESIZE+NAMESIZE+2];

EXTERN char *roomreqin   = "ROOMREQ.IN"     ;
EXTERN char *roomreqout  = "ROOMREQ.OUT"    ;
EXTERN char *roomdatain  = "ROOMDATA.IN"    ;
EXTERN char *roomdataout = "ROOMDATA.OUT"   ;

EXTERN ulong                *msglist;           /* message queue pointer         */
EXTERN int                  msglistsize = 0;    /* current size of queue         */
EXTERN char                 auto_mark   = 0;    /* automatically mark message?   */
EXTERN char                 auto_kill   = 0;    /* automatically kill message?   */

EXTERN char modem_result[15]; /* the result from dialing out */

EXTERN char reconfig  = FALSE;  /* Ctdl -C hit for reconfigure */
EXTERN char batchmode = FALSE;  /* Don't need  -C hit for reconfigure */

EXTERN int numLines = 0;  

EXTERN char justdidpause = FALSE;

#else
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/
/*********************************************************************/


EXTERN char version[]       ;
EXTERN long cfgver          ;
EXTERN char programName[]   ;
EXTERN char compilerName[]  ;
EXTERN char copyright[]     ;
EXTERN char cmpDate[]       ;
EXTERN char cmpTime[]       ;

EXTERN char   verbose;
EXTERN char   reverse;

EXTERN BOOL    lst_space ;
EXTERN BOOL    lst_rtn ;

EXTERN long keyboard_timer ;
EXTERN char saver_on ;

/* FILES */
EXTERN FILE                    *msgfl;             /* msg file descriptor          */   
EXTERN FILE                    *printfile;         /* printer.out                  */
EXTERN FILE                    *trapfl;            /* trap file descriptor         */
EXTERN FILE                    *aideFl ;
EXTERN FILE                    *hallfl;            /* Hall file descriptor         */     
EXTERN FILE                    *grpfl;             /* Group file descriptor        */     
EXTERN FILE                    *roomfl;            /* Room file descriptor         */
EXTERN FILE                    *logfl;             /* log file descriptor          */

/* STRUCTURES */
EXTERN struct accounting       thisAccount;    /* Groupdata used for current user  */
EXTERN struct accountBuffer    accountBuf;         /* Accounting buffer            */
EXTERN struct mfilter          mf;                 /* Message filter               */
EXTERN struct nodest           node;               /* node buffer                  */
EXTERN label                   editcmd;            /* External editor commands     */
EXTERN struct ext_editor       *edit;              /* External editor commands     */
EXTERN struct fInfo            *fileinfo;          /* pointer for file-info struct */
EXTERN struct groupBuffer      grpBuf;             /* Group data goes here         */
EXTERN struct hallBuffer       *hallBuf;           /* Hall stuff right here        */ 
EXTERN struct logBuffer        logBuf;             /* Log buffer of a person       */
EXTERN struct logBuffer        lBuf;               /* Temp buffer...........       */
EXTERN struct logBuffer        *lBuf2;             /* Temp buffer...........       */
EXTERN struct config           cfg;                /* A buncha variables           */ 
EXTERN struct lTable           *logTab;            /* RAM index of pippuls         */ 
        
#ifndef NEWMSGTAB
EXTERN struct msgflags *msgTab_mtmsgflags;  /* every message gets flags               */
EXTERN int             *msgTab_mtmsgLocLO;  /* offset where message starts            */
EXTERN int             *msgTab_mtmsgLocHI;  /* offset where message starts            */
EXTERN uchar           *msgTab_mtroomno;    /* room # of message                      */
EXTERN int             *msgTab_mttohash;    /* hash of recipient or group             */
EXTERN int             *msgTab_mtauthhash;  /* hash of author of message              */
EXTERN uint            *msgTab_mtomesg;     /* LO net id, forwardee hash, copy offset */
#endif

EXTERN struct aRoom            roomBuf;            /* Room buffer                  */
EXTERN struct msgB             *msgBuf;            /* The -sole- message buffer    */   
EXTERN struct msgB             *msgBuf2;           /* The Old-message buffer       */   
EXTERN struct talleyBuffer     *talleyBuf;         /* message talley for rooms     */
EXTERN struct fDir             *filedir;           /* pointer for filedir struct   */
EXTERN TERMINAL                term;

#ifdef NEWMSGTAB
EXTERN int                     *roomPos;
#else
EXTERN uchar                   *roomPos;
#endif

EXTERN char                    extrncmd[MAXEXTERN];/* External command letters     */
EXTERN struct ext_prot         *extrn;   /* External protocols buffer    */
EXTERN struct ext_door         *doors;  /* External commands buffer     */
EXTERN struct rTable           *roomTab;  /* RAM index of rooms           */ 

/* Function pointers */
EXTERN void                    (*charattr)(char, uchar);
EXTERN void                    (*stringattr)(uint, char *, uchar);

/* OTHER */
EXTERN char dialout_fkey; /* function keys in dialout mode */

EXTERN time_t                  started; /* for cron */
EXTERN char                    specialTime;    /* Is it a "special" hour           */
EXTERN float                   startbalance;   /* Number of credits at log-in      */
EXTERN long                    lasttime;       /* timestamp of last update         */
EXTERN char                    readMessage ;
EXTERN char                    ExitToMsdos ;
EXTERN char                    sysReq ;
EXTERN char                    *confirm ;
EXTERN char                    mailFlag;           /* Sending private mail?        */
EXTERN char                    oldFlag;            /* Entering old message?        */
EXTERN char                    limitFlag;          /* Entering Limited-Access      */
EXTERN char                    chatReq ;
EXTERN char                    dowhat ;
EXTERN int                     duplicate;
EXTERN int                     expired; /* count duplicates and expired */
EXTERN char                    netError ;
EXTERN unsigned char           thisHall;           /* hall we're in                */
EXTERN int                     infolength;         /* size of info-file            */
EXTERN int                     thisLog;            /* entry currently in logBuf    */
EXTERN int                     thisRoom ;
EXTERN char                    etcpath[64];        /* Path etc.dat and ctdl.exe in */     
EXTERN int                     thisSlot;           /* logTab slot logBuf found via */     
EXTERN char                    loggedIn ;
EXTERN char                    prevChar;           /* for EOLN/EOParagraph stuff   */     
EXTERN char                    expert;             /* true to suppress hints etc.  */     
EXTERN char                    termUpper;          /* uppercase only flag          */     
EXTERN char                    termLF;             /* LF-after-CR flag             */     
EXTERN char                    aide;               /* aide privileges flag         */     
EXTERN char                    sysop;              /* sysop privileges flag        */     
EXTERN char                    twit;               /* Problem user?                */     
EXTERN char                    unlisted;           /* List in userlog?             */     
EXTERN char                    termTab;            /* Tabs                         */     
EXTERN char                    oldToo;             /* Send last old on new request?*/     
EXTERN char                    roomtell;           /* Print room descriptions?     */     
EXTERN unsigned char           termWidth;          /* width to format output to    */     
EXTERN unsigned char           termNulls;          /* # nulls to send at EOLN      */     
EXTERN int                     people;             /* this is the old slot         */
EXTERN char                    sysopNew ;
EXTERN long                    uptimestamp;
EXTERN long                    logtimestamp;
EXTERN long                    conntimestamp;
EXTERN ulong                   transmitted;     /* how many characters transmitted */
EXTERN ulong                   received;        /* how many characters transmitted */
EXTERN char                    backout ;
EXTERN unsigned char           modStat ;
EXTERN char                    whichIO ;
EXTERN char                    echo;          /* Either NEITHER, CALLER, or BOTH */
EXTERN char                    echoChar; /* What to echo with if echo equals NEITHER */
EXTERN char                    anyEcho ;
EXTERN char                    warned;
EXTERN char                    sleepkey;
EXTERN char                    sysopkey ;
EXTERN char                    chatkey  ;
EXTERN char                    eventkey ;
EXTERN char                    disabled;                 /* is modem disabled?   */
EXTERN char                    speed;
EXTERN char                    tfilter[256];
EXTERN uchar                   outFilter[256];
EXTERN char                    modem ;
EXTERN char                    printing ;
EXTERN char                    console ;
EXTERN char                    ConLock ;
EXTERN char                    debug ;
EXTERN char                    callout ;
EXTERN unsigned char           crtColumn;          /* current position on screen  */   
EXTERN char                    outFlag ;
EXTERN char                    dotoMessage ;
EXTERN ulong                   markedMId   ;
EXTERN char                    heldMessage;                                            
EXTERN char                    replyFlag;          /* Sending a reply?            */   
EXTERN char                    linkMess;           /* Sending a file-linked message? */
EXTERN int                     mread;
EXTERN int                     entered;     /* #messages read and entered  */   
                                                                                
EXTERN ulong                   originalId ;
EXTERN unsigned char           originalattr;       /* Attribute byte of origin      */ 
EXTERN char                    copyflag ;
EXTERN char                    MessageRoom[MAXROOMS];  /* number of message/room this  
                                                  call */                       
EXTERN char                    seen;                                                   
EXTERN uint                    bauds[];
EXTERN char                    showdir;
EXTERN char                    showhidden; 
EXTERN char                    showgroup;
EXTERN char                    skiproom ;
EXTERN int                     oldroom;
EXTERN ulong                   ug_newpointer; 
EXTERN ulong                   ug_new;
EXTERN int                     ug_bypass;
EXTERN int                     ug_hall;
EXTERN uchar                   scrollpos ;
EXTERN uchar                   ansiattr;
EXTERN char                    heldMessage;
EXTERN char                    replyFlag;          /* Sending a reply?            */
EXTERN char                    linkMess;           /* Sending a file-linked message? */
EXTERN int                     mread;
EXTERN int                     entered;     /* #messages read and entered  */
EXTERN unsigned char           originalattr;       /* Attribute byte of origin      */
EXTERN char                    readconfigcit ;

EXTERN char       grpZap  ;
EXTERN char       hallZap ;
EXTERN char       msgZap  ;
EXTERN char       logZap  ;
EXTERN char       roomZap ;

EXTERN char   roomBuild ;
EXTERN char   hallBuild ;

EXTERN char   gprompt[80];

EXTERN uchar               numDoors;         /* number of other commands */

EXTERN uchar               conRows ;
EXTERN uchar               conCols ;
EXTERN int                 conMode ;

EXTERN BOOL                slv_door ;
EXTERN uint                slv_baud ;
EXTERN BOOL                slv_net  ;
EXTERN label               slv_node;
EXTERN int                 return_code ;

EXTERN BOOL                login_user ;
EXTERN BOOL                login_pw ;


EXTERN char                cmd_login[NAMESIZE+NAMESIZE+2];

EXTERN char *roomreqin   ;
EXTERN char *roomreqout  ;
EXTERN char *roomdatain  ;
EXTERN char *roomdataout ;

EXTERN ulong                *msglist;           /* message queue pointer         */
EXTERN int                  msglistsize ;
EXTERN char                 auto_mark   ;
EXTERN char                 auto_kill   ;

EXTERN char modem_result[15]; /* the result from dialing out */

EXTERN char reconfig;
EXTERN char batchmode;

#ifdef NEWMSGTAB
EXTERN struct msgTabList *mtList;          /* base of our linked list    */
EXTERN struct msgTabList *freeChunk;     /* the swap chunk      */

EXTERN ulong xmsaddr;
EXTERN char usexms;
EXTERN uint xmshand;
EXTERN uint xmssize;
EXTERN uint emsframe;
EXTERN uint emshand;
EXTERN uint emssize;
EXTERN uint vrthand;
EXTERN uint vrtpags;
#endif

extern struct unkLst *firstUnk; 


EXTERN int numLines;  

EXTERN char justdidpause;

#endif

