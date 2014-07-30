
/****************************************************************************/
/*                              Ctdl.h                                      */
/*               #include file for all Citadel C files.                     */
/*              Now includes only #defines and structs.                     */
/****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>
#include <time.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <bios.h>
#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <fcntl.h>
 
#undef toupper
#undef tolower
#undef strcmpi


#define strcmp(a ,b)  deansi_str_cmp(a, b)
#define stricmp(a ,b) deansi_str_cmpi(a, b)
#define strcmpi(a ,b) deansi_str_cmpi(a, b)

/* macro to translate setmem calls to the equivalent memset call */
/* #define setmem(p, n, c) memset(p, c, n)    */

typedef unsigned long  ulong ;   /* Makes it easier to format            */
typedef unsigned short ushort;   /* Makes it easier to format            */
typedef unsigned int   uint  ;   /* Makes it easier to format            */
typedef unsigned char  uchar ;   /* Makes it easier to format            */
typedef unsigned char  BOOL  ;   /* Makes it easier to format            */

typedef int (*QSORT_CMP_FNP)(const void *, const void *); 

#define NAMESIZE       30       /* length of room names                 */
#define LABELSIZE      30       /* length of room names                 */

typedef char label[LABELSIZE+1];

#define strftime strcitftime

#define CARRIER     (haveCarrier())
#define onConsole   (whichIO == CONSOLE)


#define CTRL_A      (1)
#define CTRL_B      (2)
#define ESC         (27)
#define CR          (13)
#define LF          (10)

/* -------------------------------------------------------------------- */
/*                      Stuff to size system with:                      */
/* -------------------------------------------------------------------- */

#define MAXROOMS        250    /* number of rooms allowed in system    */
#define MAXHALLS        64      /* number of halls allowed in system    */
#define MAXCRON         30      /* number of events in cron.cit         */
#define MAXEXTERN       25      /* number of protocols in external.cit  */
#define MAXDOORS        25      /* number of doors in external.cit      */
                               
#define MAXGROUPS       64      /* number of groups allowed in system   */
#define MAXTEXT         8192    /* cheating  (message buffer size)      */
                               
#define MAXDIRS         6       /* number of directions                 */
                      
#define MAXBORDERS      10      /* Number of boarder lines              */

#define MAXWORD 256     /* maximum length of a word */

/* -------------------------------------------------------------------- */
/* Citadel uses readTables() and writeTables() to write images          */
/* of the external variables in RAM to disk, and later restore          */
/* them.  The images are stored in Etc.dat and all the .TAB files.      */
/* Ctdl.Exe will automatically reconstruct these files the hard way     */
/* when necessary and write them out when finished.                     */
/* Etc.dat and the .TAB files are always destroyed after reading, to    */
/* minimize the possibility of reading out-of-date versions.  In        */
/* general, this technique works well and saves time and head-banging   */
/* upon bootup.  You should, however, note carefully the following      */
/* caution:                                                             */
/*  o  Whenever you change the declarations in Ctdl.h you should:       */
/*   -->  Destroy the current Etc.dat file.                             */
/*   -->  Recompile and reload Ctdl.Exe.                                */
/*   -->  Ctdl.Exe will automatically build new Etc.dat and tables.     */
/*                                                                      */
/* If you ignore this warning, little pixies will prick you in your     */
/* sleep for the rest of your life.                                     */
/* -------------------------------------------------------------------- */

                /* Let's begin by defining the configuration struct.    */
                /* This is part of the contents of Etc.dat              */
struct config 
{
    long    version;
    
    /*
     *  Portion from ETC.DAT
     */
    ulong   callno;             /* how many calls the system has gotten */
    ulong   oldest;             /* 32-bit ID# of first message in system*/
    ulong   newest;             /* 32-bit ID# of last  message in system*/
    ulong   mtoldest;           /* oldest message ID # in message table */
    long    catLoc;             /* where to begin writing next message  */
    
    /*
     *  Portion from CONFIG.DAT
     */

    /* SYSTEM NAME */

    label   nodeTitle;          /* name of the system                    */ 
    label   sysop;              /* name of the system operator           */
    label   nodeRegion;         /* name of the system's reagon           */ 
    label   nodeContry;         /* name of the system's reagon           */ 
    char    nodeSignature[91];  /* Signature line for the node           */
    long    sig_first_pos;      /* Position of first signature (Multi)   */
    long    sig_current_pos;    /* Current position of signature (Multi) */

    label   twitRegion;
    label   twitCountry;
    char    alias[5];           /* alias, for routing, not standardized  */
    label   nodephone;          /* phone #, in "(206) 555-1212" format   */
     
    /* MODEM STUFF */
    int     mdata;              /* 3f8 com 1,  2f8 com 2                */
    char    dumbmodem;          /* See CONFIG.CIT                       */
    char    initbaud;           /* Baud to start modem at               */
    char    modsetup[64];       /* Normal init string                   */
    label   offhooksetup[20];
    char    dialsetup[64];      /* Dial out string                      */
    label   dialpref;           /* ATDT                                 */
    label   dialring;           /* What to dial when RING received      */
    char    updays[7];          /* Which Days board is answering?       */
    char    uphours[24];        /* Which hours board is answering?      */
    
    /* PATHS */
    char homepath[64];          /* which path system files will be in   */
    char msgpath[64];           /* which path message file will be in   */
    char helppath[64];          /* path which help files live in        */
    char temppath[64];          /* temporary files (networking) go here */
    char roompath[64];          /* where to look for room descriptions  */
    char trapfile[64];          /* where trap file goes                 */
    char aplpath[64];           /* which path applications will be in   */
    char dirpath[64];           /* default path for directory rooms     */
    char transpath[64];         /* path transient files will be in      */
    char printer[64];           /* where to put output at alt-p         */
    
    /* DEFAULT TERMINAL */
    uchar   width;              /* Default unlogged screen width        */
    uchar   nulls;              /* Default unlogged # nulls             */
    uchar   linesScreen;
    BOOL    uppercase      ;    /* Default is upper case?               */
    BOOL    linefeeds      ;    /* Default linefeeds given?             */
    BOOL    tabs           ;    /* Default do we have tabs?             */
    
    /* NEW USER */
    label   enter_name;         /* enter "you name or nym"              */
    char    newuserapp[13];     /* application to run for new users     */
    BOOL    readOld;
    int     oldcount;           /* how many message new to new users    */
    char    user[20];           /* #user default status                 */
    BOOL    l_closedsys    ;   /* System is closed to new users        */
    BOOL    l_verified     ;   /* Leave users verified                 */
    BOOL    l_questionare  ;   /* Ask the build in questionare         */
    BOOL    l_application  ;   /* Call an application                  */
    BOOL    l_sysop_msg    ;   /* Message to Sysop                     */
    BOOL    l_create       ;
    
    /* ACCOUNTING */
    char    unlogtimeout;       /* how many idle mins unlogged user gets*/
    char    timeout;            /* how many idle mins logged user gets  */
  /*  char    maxwarn; */           /* # times to warn user before hangup   */
    float   unlogbal;           /* # credits unlogged users get         */
    float   newbal;             /* # credits new users get              */
    float   maxbalance;         /* maximum amount of credits user gets  */
    
    /* APPEARANCE */
    char    datestamp[64];      /* time date format string              */
    char    vdatestamp[64];     /* time date format string (verbose)    */
    label   msg_nym;            /* Name of message..                    */
    label   msgs_nym;           /* Name of messages..                   */
    label   msg_done;           /* What to doto message                 */
    char    border[MAXBORDERS][81];/* Border lines...                   */
    char    netPrefix[11];
    uchar   nopwecho;           /* what to echo initials & pw at login  */
    label   anonauthor;         /* author of anonymous messages         */
    
    /* SYSTEM SIZE */
    uint    messagek;           /* how many K message table is..        */
    int     maxtext;            /* # of characters per message          */
    int     maxfiles;           /* Max number of files per directory    */
    int     MAXLOGTAB;          /* number of log entries supported      */
    int     nmessages;          /* # of messages to be stored in table  */
    
    /* FEATURES */
    BOOL    msgNym         ;   /* Message nyms enabled..               */
    BOOL    borders        ;   /* Borderlines enabled..                */
    BOOL    forcelogin     ;   /* Automatically log someone in?        */
    BOOL    floors         ;   /* Changes the system to use floors, not
                                  halls.                               */
    BOOL    offhook        ;   /* TRUE to go off-hook when 'L' is hit  */
    BOOL    roomtell       ;   /* display room descriptions?           */
    BOOL    loginstats     ;   /* display log-in statistics?           */
    BOOL    accounting     ;   /* is accounting enabled on the system? */
    BOOL    netmail        ;   /* put networked mail in proper rooms?  */
    char    subhubs        ;   /* provision for special g)oto looping  */
    BOOL    colors         ;   /* allow colored room names etc...      */
    
    /* PRIVILEGES */
    BOOL    entersur       ;   /* Let users enter their own surname?   */
    BOOL    aidehall       ;   /* Aides mess with halls.               */
    BOOL    readluser      ;   /* Read Limited Access Userlog?         */
    BOOL    unlogEnterOk   ;   /* TRUE if OK to enter messages anon    */
    BOOL    unlogReadOk    ;   /* TRUE if unlogged folks can read mess */
    BOOL    nonAideRoomOk  ;   /* TRUE general folks can make rooms    */
    BOOL    moderate       ;   /* can aides see moderated messages?    */
    BOOL    kill           ;   /* TRUE if user can kill own messages   */
    
    /* TITLES / SURNAMES */
    BOOL    surnames       ;   /* are the surenames on?                */
    BOOL    netsurname     ;   /* Display networked surnames?          */
    BOOL    titles         ;   /* Titles are on                        */
    BOOL    nettitles      ;   /* Titles enabled                       */
    
    /* CONSOLE */
    BOOL    bios           ;   /* TRUE to use BIOS calls for screen I/O*/
    uchar   attr;               /* color of text displayed on screen    */
    uchar   wattr;              /* color of text displayed on window    */
    uchar   cattr;              /* color of text displayed on window    */
    uchar   battr;              /* color of text screen border lines    */
    uchar   uttr;               /* color of text displayed in underline */
    int     screensave;         /* # minutes before blanking screen     */
    
    /* OTHER STUFF */
    int     idle;               /* how long we wait before we net       */
    
    int     ansiBye;            /* Goodbye blbs.                        */
    int     normBye;            /* Goodbye blbs.                        */
    int     ansiHello;     
    int     normHello;     
    
    label   f6pass;

    char    trapit[17];         /* which events are logged              */

    uchar   MessageRoom;        /* max messages per room per call       */
    
    BOOL    noChat         ;   /* TRUE to suppress chat attempts       */
    BOOL    noBells        ;   /* TRUE to supress all bells            */
} ; 

/* -------------------------------------------------------------------- */
/*                           Misc. #defines                             */
/* -------------------------------------------------------------------- */
/* values for functions to return: */

#define TRUE            1
#define FALSE           0
#define ERROR          -1

#define SAMESTRING      0       /* value for strcmp() & friend          */


/* -------------------------------------------------------------------- */
/*                              Room data                               */
/* -------------------------------------------------------------------- */
#define LOBBY           0       /* Lobby> is >always< room 0.           */
#define MAILROOM        1       /* Mail>  is >always< room 1.           */
#define AIDEROOM        2       /* Aide)  is >always< room 2.           */
#define DUMP            3       /* Dump>  is >always< room 3.           */
#define DUNGEON         4       /* ....>  is >always< room 4.           */

#define MAXGEN        128       /* six bits of generation => 64 of them */

#define OLDNEW          0       /* List both unread and read rooms      */
#define NEWRMS          1       /* Only list unread rooms.              */
#define WINDWS          2       /* List rooms which are windows         */
#define DIRRMS          4       /* Directory rooms..                    */
#define MAILRM          5       /* Mail rooms......                     */
#define APLRMS          6       /* Aplication rooms                     */
#define STATRM          7       /* Status of Room                       */
#define LIMRMS          8       /* Limited Access Rooms                 */
#define OLDRMS          9       /* Old Rooms                            */
#define XCLRMS          10      /* Xcluded Rooms                        */
#define SHRDRM          11      /* Shared rooms                         */
#define NOTSHRDRM       12      /* Not Shared rooms                     */

struct rflags                   /* Room flags                           */
{  
    unsigned INUSE    : 1;      /* Room in use?                         */
    unsigned PUBLIC   : 1;      /* Room public?                         */
    unsigned MSDOSDIR : 1;      /* Room directory?                      */
    unsigned PERMROOM : 1;      /* Room permanent?                      */
    unsigned GROUPONLY: 1;      /* Room Owned by a group?               */
    unsigned READONLY : 1;      /* Room Read only?                      */
    unsigned DOWNONLY : 1;      /* Room download only?                  */
    unsigned SHARED   : 1;      /* Room Shared?                         */ 
    unsigned MODERATED: 1;      /* Room Moderated?                      */ 
    unsigned APLIC    : 1;      /* Room have application?               */ 

/* 3.10 */    
/*  unsigned BIO      : 1; */   /* By invintation only..                */
    unsigned AUTOAPP  : 1;      /* Automatically run application.       */

    unsigned UPONLY   : 1;      /* Upload only                          */
    
/* 3.12 */    
    unsigned PRIVGRP  : 1;      /* Has a privlage group                 */

/* 3.11 */    
    unsigned ANON     : 1;      /* Anonymous messages in this room...   */
    
/* 3.12 */    
    unsigned SUBJECT  : 1;      /* ask user for subject in this room    */
    unsigned GRP_MOD  : 1;      /* Access group is moderators..         */
} ;

struct rTable                   /* The summation of a room              */
{  
    uchar         rtgen;        /* generation # of room                 */
    uchar         grpno;        /* group # which owns it                */
 /* uchar UNUSED  grpgen; */    /* generation # of group which owns it  */
    struct rflags rtflags;      /* public/private flag etc              */
    label         rtname;       /* name of room                         */
    int           netIDhash;    /* hash if NET ID                       */
} ;                             /* And see ROOMA.C for declaration      */

struct aRoom                        /* The appearance of a room:            */
{  
    uchar           rbgen;          /* generation # of room                 */
    struct rflags   rbflags;        /* same bits as flags above             */
    label           rbname;         /* name of room                         */
    char            rbdirname[64];  /* user directory for this room's files */
    char            rbroomtell[13]; /* filename for room description        */
    char            rbaplic[13];    /* filename for room description        */
    uchar UNUSED  /*rbgrpgen*/;     /* group gen# for Limited Access rooms  */
    uchar           rbgrpno;        /* group # which owns it                */

    int             rbmodhash;      /* moderator's hash                     */
    int             rbhosthash;     /* host for BIO room                    */
    uchar UNUSED2 /*rbPgrpgen*/;    /* group generation for privliges grp   */
    uchar           rbPgrpno;       /* group number for privliges grp       */
    uchar           rbjunk[2];      /* junk..                               */
    char            descript[80];   /* for .kv[...]                         */

    char            netID[20];      /* Network ID for room                  */
};

struct NEWaRoom                        /* The appearance of a room:            */
{  
    uchar           rbgen;          /* generation # of room                 */
    struct rflags   rbflags;        /* same bits as flags above             */
    label           rbname;         /* name of room                         */
    char            rbdirname[64];  /* user directory for this room's files */
    char            rbroomtell[13]; /* filename for room description        */
    char            rbaplic[13];    /* filename for room description        */
    uchar UNUSED  /*rbgrpgen*/;     /* group gen# for Limited Access rooms  */
    uchar           rbgrpno;        /* group # which owns it                */

    int             rbmodhash;      /* moderator's hash                     */
    int             rbhosthash;     /* host for BIO room                    */
    uchar UNUSED2 /*rbPgrpgen*/;    /* group generation for privliges grp   */
    uchar           rbPgrpno;       /* group number for privliges grp       */
    uchar           rbjunk[2];      /* junk..                               */
    char            descript[80];   /* for .kv[...]                         */

    char            netID[20];      /* Network ID for room                  */
};

/* -------------------------------------------------------------------- */
/*                      group & accounting stuff                        */
/* -------------------------------------------------------------------- */
#define MAXGROUPGEN 128

struct accounting
{
    char days[7];        /* Which Days this group can log-in        */
    char hours[24];      /* Which hours of the day group can log-in */
    char special[24];    /* Which hours are special                 */
    float dayinc;        /* Maximum time add per day                */
    float sp_dayinc;     /* Special Maximum time add per day        */
    float priority;      /* What is the priority for this group     */
    float maxbal;        /* This groups maxtime on the system       */
    float dlmult;        /* Multiplyer for Downloads                */
    float ulmult;        /* Multiplyer for Uploads                  */

    unsigned have_acc: 1;/* Does group/user have accounting?        */
};

struct accountBuffer
{
    struct
    {
        struct accounting account;
    } group[MAXGROUPS];
};

/* -------------------------------------------------------------------- */
/*                          group stuff                                 */
/* -------------------------------------------------------------------- */
struct groupBuffer
{
    struct group_buffer
    {
        label    groupname  ;      /* Name of the group                 */
        uchar    groupgen   ;      /* Group generation #.               */
        char     desc[80]   ;      /* Description of group              */
        unsigned g_inuse : 1;      /* Is group inuse?                   */
        unsigned lockout : 1;      /* Sysop access needed for changes   */
        unsigned hidden  : 1;      /* hidden to all, unless specified 
                                      by name or in group               */
        unsigned autoAdd : 1;      /* Add to new users.                 */
        unsigned UNUSED  :12;
    } group[MAXGROUPS];
} ;


/* -------------------------------------------------------------------- */
/*                            userlog stuff                             */
/* -------------------------------------------------------------------- */
struct lflags                   /* Flags for person in log              */
{
    unsigned L_INUSE   : 1;     /* Is this slot in use?                 */
    unsigned UCMASK    : 1;     /* Uppercase?                           */
    unsigned LFMASK    : 1;     /* Linefeeds?                           */
    unsigned EXPERT    : 1;     /* Expert?                              */
    unsigned AIDE      : 1;     /* Aide?                                */
    unsigned TABS      : 1;     /* Tabs?                                */
    unsigned OLDTOO    : 1;     /* Print out last oldmessage on  N>ew?  */
    unsigned PROBLEM   : 1;     /* Twit bit                             */
    unsigned UNLISTED  : 1;     /* List in Userlog?                     */
    unsigned PERMANENT : 1;     /* Permanent log entry?                 */
    unsigned SYSOP     : 1;     /* Sysop special access.                */
    unsigned NODE      : 1;     /* Are they a node?                     */
    unsigned NETUSER   : 1;     /* Can they send network E-mail         */
    unsigned NOACCOUNT : 1;     /* Accounting disabled for this person? */
    unsigned NOMAIL    : 1;     /* Can user send exclusive mail?        */
    unsigned ROOMTELL  : 1;     /* Room-descriptions?                   */
} ;

#define MAXVISIT       16       /* #visits we remember old newestLo for */

struct lbroomdata
{
    uchar    lbgen;             /* which rooms are visible?             */
    unsigned xclude : 1;        /* is room excluded?                    */
    unsigned UNUSED2/*mail*/:1; /* private mail in this room?           */
    unsigned lvisit : 5;        /* lastvisit pointer 1 - 16             */
    unsigned bypass : 1;        /* user has bypassed the room..         */
    
            /* 3.12 */
    unsigned msgaide: 1;        /* user is a room moderator             */
    unsigned filaide: 1;        /* user is a room moderator             */
    unsigned created: 1;        /* user created room                    */
    unsigned UNUSED : 5;
};



struct logBuffer                /* The appearance of a user:            */
{
    label         lbname;       /* caller's name                        */
    label         lbin;         /* caller's initials                    */
    label         lbpw;         /* caller's password                    */
    label         forward;      /* forwarding address (user)            */
    label         forward_node; /* forwarding address (node)            */
    label         surname;      /* users surname                        */
    label         title;        /* users title                          */
    int           hallhash;     /* default hallway hash                 */
    uchar         lbnulls;      /* #Nulls, lCase, lFeeds                */
    uchar         lbwidth;      /* terminal width                       */
    long          calltime;     /* Time/date stamp of last call         */
    ulong         callno;       /* Which caller # they were             */
    float         credits;      /* Credits for accounting               */
    uchar     groups[MAXGROUPS];/* Group generation # 16 of them        */
    struct lbroomdata lbroom[MAXROOMS]; /* Data for each room in LogBuf */
    ulong     lbvisit[MAXVISIT];/* newestLo for this and 15 prev.visits */

/*
 * new as of v3.10.00 DragCit 
 */
    uchar   lastRoom;           /* room they were in last call          */
    uchar   lastHall;           /* hall they were in last call          */

    uchar   linesScreen;        /* the number of lines a caller has per screen */
    uchar   attributes[5];      /* normal, bold, inverse, blink, underline */

    char    UNUSED2[4];
 
    char    protocol;           /* Default protocol */
    
    struct lflags lbflags;      /* UCMASK, LFMASK, EXPERT, AIDE, INUSE...*/
    unsigned DUNGEONED  : 1;    /* dungeoned user?                      */
    unsigned MSGAIDE    : 1;    /* message only aide?                   */
    unsigned FORtOnODE  : 1;    /* forward to node?                     */
    unsigned NEXTHALL   : 1;    /* Auto Next Hallway?                   */
    unsigned FILEAIDE   : 1;    /* Aide responsible for file sections   */
    unsigned BOARDERS   : 1;    /* Can they enter boarders              */
    unsigned VERIFIED   : 1;    /* has the user been verified?          */
    unsigned SURNAMLOK  : 1;    /* surname locked?                      */
    unsigned LOCKHALL   : 1;    /* lock default hallway                 */
    unsigned PSYCHO     : 1;    /* make life a pain in the ass for him  */
    unsigned DISPLAYTS  : 1;    /* display titles/surnames?             */
    unsigned SUBJECTS   : 1;    /* display subjects?                    */
    unsigned SIGNATURES : 1;    /* display signatures?                  */
    unsigned IBMGRAPH   : 1;    /* Terminal : IBM graphics characters?  */
    unsigned IBMANSI    : 1;    /* Terminal : IBM ansi enabled?         */
    unsigned IBMCOLOR   : 1;    /* Terminal : ISO-Color?                */
    unsigned TWIRLY     : 1;    /* twirling cursor at prompts?          */
    unsigned VERBOSE    : 1;    /* Auto-verbose flag?                   */
    unsigned MSGPAUSE   : 1;    /* Pause after each message.            */
    unsigned MINIBIN    : 1;    /* MiniBin login stats?                 */

    unsigned UNUSED1   : 12;

/*
 * new as of v3.20a/57 GremCit
 */
    char prompt[64];               /* prompt format string    */
    char dstamp[64];               /* time/date format string */
    char vdstamp[64];              /* time/date format string */
    char msg_header[256];          /* message header format string */
    char vmsg_header[256];         /* verbose message header format string */

    char space[64];                /* blank space */
};



struct NEWlogBuffer                /* The appearance of a user:            */
{
    label         lbname;       /* caller's name                        */
    label         lbin;         /* caller's initials                    */
    label         lbpw;         /* caller's password                    */
    label         forward;      /* forwarding address (user)            */
    label         forward_node; /* forwarding address (node)            */
    label         surname;      /* users surname                        */
    label         title;        /* users title                          */
    int           hallhash;     /* default hallway hash                 */
    uchar         lbnulls;      /* #Nulls, lCase, lFeeds                */
    uchar         lbwidth;      /* terminal width                       */
    long          calltime;     /* Time/date stamp of last call         */
    ulong         callno;       /* Which caller # they were             */
    float         credits;      /* Credits for accounting               */
    uchar     groups[MAXGROUPS];/* Group generation # 16 of them        */
    struct lbroomdata lbroom[MAXROOMS]; /* Data for each room in LogBuf */
   /* ulong     lbvisit[MAXVISIT]; */ /* newestLo for this and 15 prev.visits */

    ulong     lastpointer;       /* newest message on system last call */
    ulong     newpointer[MAXROOMS];  /* new pointers for all the rooms     */
/*
 * new as of v3.10.00 DragCit 
 */
    uchar   lastRoom;           /* room they were in last call          */
    uchar   lastHall;           /* hall they were in last call          */

    uchar   linesScreen;        /* the number of lines a caller has per screen */
    uchar   attributes[5];      /* normal, bold, inverse, blink, underline */

    char    UNUSED2[4];
 
    char    protocol;           /* Default protocol */
    
    struct lflags lbflags;      /* UCMASK, LFMASK, EXPERT, AIDE, INUSE...*/
    unsigned DUNGEONED  : 1;    /* dungeoned user?                      */
    unsigned MSGAIDE    : 1;    /* message only aide?                   */
    unsigned FORtOnODE  : 1;    /* forward to node?                     */
    unsigned NEXTHALL   : 1;    /* Auto Next Hallway?                   */
    unsigned FILEAIDE   : 1;    /* Aide responsible for file sections   */
    unsigned BOARDERS   : 1;    /* Can they enter boarders              */
    unsigned VERIFIED   : 1;    /* has the user been verified?          */
    unsigned SURNAMLOK  : 1;    /* surname locked?                      */
    unsigned LOCKHALL   : 1;    /* lock default hallway                 */
    unsigned PSYCHO     : 1;    /* make life a pain in the ass for him  */
    unsigned DISPLAYTS  : 1;    /* display titles/surnames?             */
    unsigned SUBJECTS   : 1;    /* display subjects?                    */
    unsigned SIGNATURES : 1;    /* display signatures?                  */
    unsigned IBMGRAPH   : 1;    /* Terminal : IBM graphics characters?  */
    unsigned IBMANSI    : 1;    /* Terminal : IBM ansi enabled?         */
    unsigned IBMCOLOR   : 1;    /* Terminal : ISO-Color?                */
    unsigned TWIRLY     : 1;    /* twirling cursor at prompts?          */
    unsigned VERBOSE    : 1;    /* Auto-verbose flag?                   */
    unsigned MSGPAUSE   : 1;    /* Pause after each message.            */
    unsigned MINIBIN    : 1;    /* MiniBin login stats?                 */

    unsigned UNUSED1   : 12;

/*
 * new as of v3.20a/57 GremCit
 */
    char prompt[64];               /* prompt format string    */
    char dstamp[64];               /* time/date format string */
    char vdstamp[64];              /* time/date format string */
    char msg_header[256];          /* message header format string */
    char vmsg_header[256];         /* verbose message header format string */

    char space[64];                /* blank space */
};




struct NEWhallBuffer
{
    struct NEW_hall_buffer
    {
        label    hallname;          /* Name of our hall                 */
        uchar    grpno;             /* owning group's #                 */
        unsigned h_inuse    : 1;    /* Is hall inuse?                   */
        unsigned owned      : 1;    /* Is this hall owned?              */
        unsigned described  : 1;    /* described?                       */
        unsigned dmaint     : 1;    /* default mainenacne               */
        unsigned enterRoom  : 1;    /* Can users enter rooms here?      */
        unsigned UNUSED     :11;

        char     inhall[1024/8]; /* Bitbag.. Is room in this hall?  */
        char     window[1024/8]; /* Bitbag.. Is room a window?      */

        uchar    mdefault;           /* hall default mainencance hall   */
        char     htell[13];          /* Hall description                */
    } hall[MAXHALLS];
} ;


struct  hallBuffer
{
    struct hall_buffer
    {
        label    hallname;          /* Name of our hall                 */
        uchar    grpno;             /* owning group's #                 */
        unsigned h_inuse    : 1;    /* Is hall inuse?                   */
        unsigned owned      : 1;    /* Is this hall owned?              */
        unsigned described  : 1;    /* described?                       */
        unsigned dmaint     : 1;    /* default mainenacne               */
        unsigned enterRoom  : 1;    /* Can users enter rooms here?      */
        unsigned UNUSED     :11;

        char     inhall[250/8]; /* Bitbag.. Is room in this hall?  */
        char     window[250/8]; /* Bitbag.. Is room a window?      */

        uchar    mdefault;           /* hall default mainencance hall   */
        char     htell[13];          /* Hall description                */
    } hall[MAXHALLS];
} ;


struct lTable                   /* Summation of a person:               */
{
    int             ltpwhash;   /* hash of password                     */
    int             ltinhash;   /* hash of initials                     */
    int             ltnmhash;   /* hash of name                         */
    int             ltlogSlot;  /* location in userlog.buf              */
    ulong           ltcallno;   /* Which caller # they were             */
    struct lflags   ltflags;    /* UCMASK, LFMASK, EXPERT, AIDE, INUSE...*/
};
 
/* -------------------------------------------------------------------- */
/*                      message stuff                                   */
/* -------------------------------------------------------------------- */
#define HELD           3

#define ATTR_RECEIVED  1
#define ATTR_REPLY     2
#define ATTR_MADEVIS   4


/* This is really cruddy.  But I guess it works.  The cfg.moderate fixes a */
/* bug where aides can moderate messages even when the system config.cit   */
/* says they can't.                                                        */

#define CAN_MODERATE() ((cfg.moderate && aide) || sysop ||\
         (roomTab[thisRoom].rtflags.GRP_MOD && pgroupseesroom()))

/*
 *   #define CAN_MODERATE() (aide || sysop ||\
 *                      (roomTab[thisRoom].rtflags.GRP_MOD && pgroupseesroom()))
 */

struct msgflags                 /* Message attribute flags              */
{  
    unsigned MAIL     : 1;
    unsigned RECEIVED : 1;      /* Was it received?                     */
    unsigned REPLY    : 1;      /* Reply sent?                          */
    unsigned PROBLEM  : 1;      /* Problem User message?                */
    unsigned MADEVIS  : 1;      /* Made Visible?                        */
    unsigned LIMITED  : 1;      /* Limited-access message?              */
    unsigned MODERATED: 1;      /* Moderated message?                   */
    unsigned RELEASED : 1;      /* Released by moderator?               */
    unsigned COPY     : 1;      /* Is message a copy?                   */
    unsigned NET      : 1;      /* Networked mail en-route?             */
    unsigned FORWARDED: 1;      /* Forwarded E-Mail?                    */
    unsigned NETWORKED: 1;      /* Networked Message?                   */
    unsigned UNUSED   : 4;
};

struct msgB                     /* This is what a msg looks like        */
{
    char mbtext[MAXTEXT];       /* buffer text is edited in             */

    
    long  mbheadLoc;            /* Start of message                     */
    uchar mbroomno;             /* room # of message                    */
    uchar mbattr;               /* 3 attribute bits                     */

    label mbauth ;              /* name of author                       */
    label mbsur  ;              /* surname                              */
    label mbtitle;              /* Title...                             */
    label mbfwd  ;              /* forwarding address                   */
    label mboname;              /* name of origin system                */
    label mboreg ;              /*  "        "    region                */
    label mbocont;              /*  "        "    country               */
    label mbcreg ;              /* twit region                          */
    label mbccont;              /* twit country                         */
    label mbophone; /* (NOT USED) */ /* origin phone, like "(206) 542-7825"  */

    label mbroom ;              /* creation room                        */
    char  mbsig[91];            /* signature field                      */
    char  mbusig[91];           /* user signature field                 */
    char  mbsub[81];            /* subject                              */
    label mbsoft;               /* origin software..                    */
    label mbsrcId;              /* message ID on system of origin       */
    char  mbfpath[257];         /* where did the message come from?     */
    
    label mbto   ;              /* private message to                   */
    label mbzip  ;              /* name of destination system           */
    label mbrzip ;              /*   "          "      region           */
    label mbczip ;              /*   "          "      country          */
    label mbzphone; /* (NOT USED) */ /* dest. phone #, "206/746-2694" fmt.   */

    char  mbtpath[257];         /* forced routing vila path             */
    
    label mbId   ;              /* local number of message              */
    label mbcopy ;              /* message ID of copy this msg is of    */
    label mbgroup;              /* group name for limited access msg    */
    label mbtime ;              /* creation time                        */
    label mbreply;              /* message ID of message replied to     */
    label mbx    ;              /* twit/moderated message header        */
    char  mblink[64];           /* file linked message                  */
};  

/* The following is the message filter table..
 */
struct mfilter
{  
    unsigned mfMai   : 1;
    unsigned mfLim   : 1;
    unsigned mfPub   : 1;
    label    mfUser;
    label    mfGroup;
    label    mfSearch;
} ;

/* values for showMess routine */
#define NEWoNLY         0
#define OLDaNDnEW       1
#define OLDoNLY         2

/* -------------------------------------------------------------------- */
/*                      message table stuff                             */
/* -------------------------------------------------------------------- */

#ifdef GOODGOODBYE

struct messagetable1 {
    struct msgflags mtmsgflags; /* every message gets flags             */
};

#ifdef GOODBYE
struct messagetable2 {
    long     mtmsgLoc;          /* offset where message starts          */
};
#endif

struct messagetable2LO {
    int     mtmsgLocLO;          /* offset where message starts          */
};

struct messagetable2HI {
    int     mtmsgLocHI;          /* offset where message starts          */
};


#ifdef GOODBYE                  /* Use mtomesg instead */
struct messagetable3 {
    ushort   mtoffset;          /* msg# - mtoffset == actual msg no     */
                                /* to determine if copy has scrolled    */
};
#endif

struct messagetable4 {
    uchar    mtroomno;          /* room # of message                    */
};
struct messagetable5 {
    int      mttohash;          /* hash of recipient or group           */
};
struct messagetable6 {
    int      mtauthhash;        /* hash of author of message            */
};

#ifdef GOODBYE                  /* Use mtomesg instead */
struct messagetable7 {
    int      mtfwdhash;         /* hash of forwardee                    */
};
#endif

struct messagetable8 {          /* only really need low half of id here */
/* long */ uint mtomesg;        /* this and the next are to check for   */
};

#ifdef GOODBYE
struct messagetable9 {          /* quick check author hash instead */
                                /* duplicate messages                   */
    int      mtorigin;          /* hash of the origin system's name     */
};
#endif

#ifdef GOODBYE
struct messagetable
{
    struct msgflags mtmsgflags; /* every message gets flags             */
    long     mtmsgLoc;          /* offset where message starts          */
    ushort   mtoffset;          /* msg# - mtoffset == actual msg no     */
                                /* to determine if copy has scrolled    */
    uchar    mtroomno;          /* room # of message                    */
    int      mttohash;          /* hash of recipient or group           */
    int      mtauthhash;        /* hash of author of message            */
    int      mtfwdhash;         /* hash of forwardee                    */
    long     mtomesg;           /* this and the next are to check for   */
                                /* duplicate messages                   */
    int      mtorigin;          /* hash of the origin system's name     */
};
#endif
#endif
/* -------------------------------------------------------------------- */
/*          TalleyBuffer Stuff (message counts for each room)           */
/* -------------------------------------------------------------------- */
struct talleyBuffer
{
    struct
    {
        ushort total;
        ushort messages;
        ushort new;
        char   hasmail;
    } room[MAXROOMS];
};

/* -------------------------------------------------------------------- */
/*                       node stuff                                     */
/* -------------------------------------------------------------------- */
struct nodest
{
    int   ndbaud;       /* Auto baud detect */
    int   nddialto;
 /* int   ndredial; */  /* Redundant with Cron #RETRY */
    int   ndwaitto;
    int   ndexpire;

    label ndname;
    label ndregion;
    label ndprotocol;
    char  ndphone[50];
    label ndmailtmp;

    struct {
        label here;
        label there;
    }ndgroups[MAXGROUPS];

    ulong roomoff;

    char  ndlogin[256];
    
    uchar network;
    
    char  zip[41];
    char  unzip[41];
    
    char  cleanup[41];
    int   autoroom;
    int   verbose;
};


/* -------------------------------------------------------------------- */
/*                      modem stuff                                     */
/* -------------------------------------------------------------------- */
/* fiddle factor to timeout on no input: */
#define MODEM           0       /* current user of system is            */
#define CONSOLE         1       /* one of these                         */

/*  output XON/XOFF etc flag... */
#define OUTOK           0       /* normal output                        */
#define OUTNEXT         2       /* quit this message, get the next      */
#define OUTSKIP         3       /* stop current process                 */
#define OUTPARAGRAPH    4       /* skip to next paragraph               */
#define IMPERVIOUS      5       /* make current output unstoppable      */
#define NOSTOP          6       /* can pause, but not stop              */

#define CANOUTPUT()     (outFlag == OUTOK || outFlag == IMPERVIOUS \
                         || outFlag == NOSTOP)

#define NEITHER         0       /* don't echo input at all              */
#define CALLER          1       /* echo to caller only --passwords etc  */
#define BOTH            2       /* echo to caller and console both      */

#define NO_ECHO         0       /* Echo input as .'s                    */
#define ECHO            1       /* Echo input   (getString())           */


/* -------------------------------------------------------------------- */
/*                           Trap File Stuff                            */
/* -------------------------------------------------------------------- */
#define T_ALL        0             /* All events are logged             */
#define T_CARRIER    1             /* carrier detect & loss are logged  */
#define T_LOGIN      2             /* login, logout, new users          */
#define T_NEWROOM    3             /* new room creations are logged     */
#define T_PROBLEM    4             /* problem user messages are logged  */
#define T_CHAT       5             /* chat requests are logged          */
#define T_PASSWORD   6             /* password changes are logged       */
#define T_AIDE       7             /* aide functions are logged         */
#define T_SYSOP      8             /* sysop functions are logged        */
#define T_UPLOAD     9             /* uploads are logged                */
#define T_DOWNLOAD   10            /* all downloads are logged          */
#define T_ERROR      11            /* internal errors are logged        */
#define T_NETWORK    12            /* network events are logged         */
#define T_ACCOUNT    13            /* accounting information is logged  */
#define T_APPLIC     14            /* application executions are logged */
#define T_CRON       15            /* cron events are logged            */

/* -------------------------------------------------------------------- */
/*                        Default User Stuff                            */
/* -------------------------------------------------------------------- */
#define D_PROBLEM     0            /* Everyone defaults to problem user */
#define D_PERMANENT   1            /* Everyone gets perm log entry      */
#define D_NOACCOUNT   2            /* Everyone has accounting disabled  */
#define D_NETWORK     3            /* Everyone's a network user         */
#define D_NOMAIL      4            /* Everyone sends no mail            */
#define D_AIDE        5            /*                                   */
#define D_SYSOP       6            /*                                   */
#define D_BOARDER     7

/* -------------------------------------------------------------------- */
/*  External stuff                                                      */
/* -------------------------------------------------------------------- */
/*  External protical structure  */
struct ext_prot
{
    label ex_name;
    char  ex_rcv[40];
    char  ex_snd[40];
    char  ex_batch;
    int   ex_block;
};

/*  External editor structure  */
struct ext_editor
{
    label ed_name;
    char  ed_cmd[30];
    char  ed_local;
};

/*  External other command structure  */
struct ext_door
{
    label    name;
    char     cmd[41];
    label    group;
    unsigned DIR    :1;
    unsigned SYSOP  :1;
    unsigned AIDE   :1;
    unsigned CON    :1;
    unsigned UNUSED :12;
};

/* -------------------------------------------------------------------- */
/*                       cron stuff                                     */
/* -------------------------------------------------------------------- */
struct event 
{
    char   e_type;       /* Event type (network, shell, ext)              */
    char   e_str[31];    /* nodename, shell command, ext                  */
    char   e_hours[24];  /* valid hours for event                         */
    char   e_days[7];    /* valid days for event                          */
    int    e_redo;       /* minutes before it will redo this event        */
    int    e_retry;      /* minutes before it will retry                  */
  
    long l_success;       /* last time it has success                     */
    long l_try;          /* last time it tryed                            */
};

#define CRON_TIMEOUT   0   /* why do_cron() is called                   */
#define CRON_LOGOUT    1
#define CRON_PROMPT    2

/* -------------------------------------------------------------------- */
/* Infofile nonsence                                                    */
/* -------------------------------------------------------------------- */
struct fInfo                                /*  fileinfo data structure */ 
{
    char fn[13];
    char uploader[31];
    char comment[65];
};

/* -------------------------------------------------------------------- */
/*  File directory structure.                                           */
/* -------------------------------------------------------------------- */
struct fDir
{
    char entry[90];
};

/* -------------------------------------------------------------------- */
/*  What we are doing..  dowhat                                         */
/* -------------------------------------------------------------------- */
#define DUNO        0
#define MAINMENU    1
#define SYSOPMENU   2
#define PROMPT      3
#define MESSAGETEXT 4
#define DIALOUT     5
#define NETWORKING  6
#define WAITCALL    7
#define READMESSAGE 8

/* -------------------------------------------------------------------- */
/*  TERMCAP definitions                                                 */
/* -------------------------------------------------------------------- */
#define ATTR_NORMAL     0
#define ATTR_BLINK      1
#define ATTR_REVERSE    2
#define ATTR_BOLD       3
#define ATTR_UNDERLINE  4


#define TERM_NORMAL     '0'
#define TERM_BLINK      '1'
#define TERM_REVERSE    '2'
#define TERM_BOLD       '3'
#define TERM_UNDERLINE  '4'
#define TERM_BS         'B'
#define TERM_IMPERV     'I'
#define TERM_PAUSE      'P'
#define TERM_HANGUP     'H'

typedef struct
{
    label   normal;
    label   bold;
    label   inverse;
    label   blink;
    label   under;
    BOOL    ibmAnsi;
    BOOL    ibmColor;
}
TERMINAL;

/* -------------------------------------------------------------------- */
/*  These flags are for special action during message retrvial          */
/* -------------------------------------------------------------------- */
#define              NO_SPECIAL     0   /* dont do anything with message */
#define              PULL_IT        1   /* kill the message */
#define              MARK_IT        2   /* mark the message to be moved */
#define              REVERSE_READ   3   /* change read direction */
#define              COPY_IT        4   /* copy message to file */

/* -------------------------------------------------------------------- */
/* list stuff                                                           */
/* -------------------------------------------------------------------- */
#define     LIST_START  NULL
#define     LIST_END    ((void *)1)

/* -------------------------------------------------------------------- */
/* Citadel event stuff                                                  */
/* -------------------------------------------------------------------- */
#define     E_SHUTDOWN  0
#define     E_IDLE      1
#define     E_CARRIER   2
#define     E_LOCAL     4
#define     E_SYSOP     5
#define     E_SHELL     6
#define     E_SWAPSHELL 7

/* -------------------------------------------------------------------- */
/* Console #defines                                                     */
/* -------------------------------------------------------------------- */
#define ALT_A   30  /* */
#define ALT_B   48  /* */
#define ALT_C   46  /* */
#define ALT_D   32  /* */
#define ALT_E   18  /* */
#define ALT_F   33
#define ALT_G   34
#define ALT_H   35
#define ALT_I   23
#define ALT_J   36
#define ALT_K   37
#define ALT_L   38  /* */
#define ALT_M   50
#define ALT_N   49
#define ALT_O   24
#define ALT_P   25  /* */
#define ALT_Q   16
#define ALT_R   19
#define ALT_S   31
#define ALT_T   20  /* */
#define ALT_U   22
#define ALT_V   47  /* */
#define ALT_W   17
#define ALT_X   45  /* */
#define ALT_Y   21
#define ALT_Z   44

#define F1      59  /* */
#define F2      60  /* */
#define F3      61  /* */
#define F4      62  /* */
#define F5      63  /* */
#define F6      64  /* */
#define F7      65  /* */
#define F8      66  /* */
#define F9      67  /* */
#define F10     68  /* */

#define ALT_F1  104
#define ALT_F2  105
#define ALT_F3  106
#define ALT_F4  107
#define ALT_F5  108
#define ALT_F6  109 /* */
#define ALT_F7  110
#define ALT_F8  111
#define ALT_F9  112
#define ALT_F10 113

#define SFT_F1  84
#define SFT_F2  85
#define SFT_F3  86
#define SFT_F4  87
#define SFT_F5  88
#define SFT_F6  89  /* */
#define SFT_F7  90
#define SFT_F8  91
#define SFT_F9  92
#define SFT_F10 93

#define PGUP    73  /* */
#define PGDN    81  /* */

#include "global.h"
#include "keywords.h"
#include "proto.h"
#include "spawno.h"

void convertlog(void);
void convertroom(void);

struct logBuffer        OLDlogBuf;             /* Old Log buffer of a person       */
struct NEWlogBuffer     NEWlogBuf;             /* New Log buffer of a person       */

struct aRoom             OLDroomBuf;           /* Old room buffer */ 
struct NEWaRoom          NEWroomBuf;           /* New room buffer */ 

struct hallBuffer         *OLDhallBuf;
struct NEWhallBuffer      *NEWhallBuf;


/* These nice bitbag Routines were STOLEN from Stonenehenge source code */
/* copyrighted  by Megalithic Microproducts Ltd.                        */
/* If you would like to sue me for copyright infringment go ahead:      */
/* my info:                                                             */
/*          Matthew S. Pfleger                                          */
/*          11201 Algonquin Rd                                          */
/*          Edmonds, WA 98020                                           */
/*          Ph: 206-628-3810                                            */

/************************************************************************/
/*      bit_clear()                                                     */
/************************************************************************/
int     bit_clear(char *bitbag, int bit_no)
{
        unsigned byte,offset;

        byte    = bit_no >> 3;
        offset  = bit_no &  7;

        return (bitbag[byte] &= ~(1 << offset));
}

/************************************************************************/
/*      bit_test()                                                      */
/************************************************************************/
int     bit_test(char *bitbag, int bit_no)
{
        unsigned byte,offset;

        byte    = bit_no >> 3;
        offset  = bit_no &  7;
        return (bitbag[byte] & (1 << offset));
}

/************************************************************************/
/*      bit_set()                                                       */
/************************************************************************/
int     bit_set(char *bitbag, int bit_no)
{
        unsigned    byte,offset;
        byte    = bit_no >> 3;
        offset  = bit_no &  7;

        return (bitbag[byte] |= (1 << offset));
}

void converthall(void);


/* uchar                   *roomPos; */


void main(int argc, char *argv[])
{
     printf("Grem-Convert v.58.\n");
     printf("Copyright (c) 1992 Anticlimactic Teleservices Ltd.\n\n");
     printf("This convert program adds space in logbuffer for \n");
     printf("real message pointers.\n");


     convertlog(); 

#ifdef GOODBYE
    converthall();
    convertroom();
    convertlog();
#endif
}

void convertlog(void)
{
    FILE *OLDlogfl, *NEWlogfl;
    int i;

    int logno = 0;

    /* open log file */

    if ((OLDlogfl = fopen("LOG.DAT", "r+b")) == NULL)
    {
        printf("Could not open log.dat\n");
    }
    if ((NEWlogfl = fopen("NEWLOG.DAT", "w+b")) == NULL)
    {
        printf("Could not open newlog.dat\n");
    }

    for(;;)
    {
        printf("Converting log #%d\r", logno);
        logno++;

        if (fread(&OLDlogBuf, sizeof OLDlogBuf, 1, OLDlogfl) != 1)
        {
            printf("\nlog.dat EOF.\n");
            break;
        }

        /* do conversion here */

        strcpy(NEWlogBuf.lbname,    OLDlogBuf.lbname);
        strcpy(NEWlogBuf.lbin,      OLDlogBuf.lbin  );
        strcpy(NEWlogBuf.lbpw,      OLDlogBuf.lbpw  );
        strcpy(NEWlogBuf.forward,   OLDlogBuf.forward );
        strcpy(NEWlogBuf.surname,   OLDlogBuf.surname );
        strcpy(NEWlogBuf.title,     OLDlogBuf.title   );

        NEWlogBuf.hallhash = OLDlogBuf.hallhash;
        NEWlogBuf.lbnulls  = OLDlogBuf.lbnulls;
        NEWlogBuf.lbwidth  = OLDlogBuf.lbwidth;
        NEWlogBuf.calltime = OLDlogBuf.calltime;
        NEWlogBuf.callno   = OLDlogBuf.callno;
        NEWlogBuf.credits  = OLDlogBuf.credits;


        for (i = 0; i < MAXGROUPS; ++i)
        {
            NEWlogBuf.groups[i]  = OLDlogBuf.groups[i];
        }

        for (i = 0; i < MAXROOMS; ++i)
        {
            NEWlogBuf.lbroom[i]  = OLDlogBuf.lbroom[i];
        }

#ifdef GOODBYE
        for (i = 0; i < MAXVISIT; ++i)
        {
            NEWlogBuf.lbvisit[i]  = OLDlogBuf.lbvisit[i];
        }
#endif


/* this is the new shit */
      for (i = 0; i < MAXROOMS; ++i)
      {
      NEWlogBuf.newpointer[i] = OLDlogBuf.lbvisit[OLDlogBuf.lbroom[i].lvisit];
      }

      NEWlogBuf.lastpointer  = OLDlogBuf.lbvisit[0];

/* wow did you see that new shit?? */

        NEWlogBuf.lastRoom  = OLDlogBuf.lastRoom;
        NEWlogBuf.lastHall  = OLDlogBuf.lastHall;
        NEWlogBuf.linesScreen  = OLDlogBuf.linesScreen;

        for (i = 0; i < 5; ++i)
        {
             NEWlogBuf.attributes[i]  = OLDlogBuf.attributes[i];
        }

        NEWlogBuf.protocol  = OLDlogBuf.protocol;
        NEWlogBuf.lbflags   = OLDlogBuf.lbflags;

        NEWlogBuf.DUNGEONED  =  OLDlogBuf.DUNGEONED   ;
        NEWlogBuf.MSGAIDE    =  OLDlogBuf.MSGAIDE     ;
        NEWlogBuf.FORtOnODE  =  OLDlogBuf.FORtOnODE   ;
        NEWlogBuf.NEXTHALL   =  OLDlogBuf.NEXTHALL    ;
        NEWlogBuf.FILEAIDE   =  OLDlogBuf.FILEAIDE    ;
        NEWlogBuf.BOARDERS   =  OLDlogBuf.BOARDERS    ;
        NEWlogBuf.VERIFIED   =  OLDlogBuf.VERIFIED    ;
        NEWlogBuf.SURNAMLOK  =  OLDlogBuf.SURNAMLOK   ;
        NEWlogBuf.LOCKHALL   =  OLDlogBuf.LOCKHALL    ;
        NEWlogBuf.PSYCHO     =  OLDlogBuf.PSYCHO      ;
        NEWlogBuf.DISPLAYTS  =  OLDlogBuf.DISPLAYTS   ;
        NEWlogBuf.SUBJECTS   =  OLDlogBuf.SUBJECTS    ;
        NEWlogBuf.SIGNATURES =  OLDlogBuf.SIGNATURES  ;
        NEWlogBuf.IBMGRAPH   =  OLDlogBuf.IBMGRAPH    ;
        NEWlogBuf.IBMANSI    =  OLDlogBuf.IBMANSI     ;
        NEWlogBuf.IBMCOLOR   =  OLDlogBuf.IBMCOLOR    ;
        NEWlogBuf.TWIRLY     =  OLDlogBuf.TWIRLY      ;
        NEWlogBuf.VERBOSE    =  OLDlogBuf.VERBOSE     ;
        NEWlogBuf.MSGPAUSE   =  OLDlogBuf.MSGPAUSE    ;

        NEWlogBuf.MINIBIN = TRUE;

        strcpy(NEWlogBuf.prompt, "%r%e");
        strcpy(NEWlogBuf.dstamp, "%x %X");
        strcpy(NEWlogBuf.vdstamp, "%x %X %Z");

        /* ^^ do conversion here ^^ */

        if (fwrite(&NEWlogBuf, sizeof NEWlogBuf, 1, NEWlogfl) != 1)
        {
            printf("newlog.dat write fucked.\n");
        }
    }         

    fclose(OLDlogfl);
    fclose(NEWlogfl);

     printf("Done.  Rename NEWLOG.DAT to LOG.DAT\n");
     printf("       Please keep old LOG.DAT just in case\n") ;

}

    FILE *OLDhallfl, *NEWhallfl;

void converthall(void)
{
    int i, j;


    OLDhallBuf = _fcalloc(1,  sizeof(struct hallBuffer));
    NEWhallBuf = _fcalloc(1,  sizeof(struct NEWhallBuffer));


       /* roomPos = _fcalloc(MAXROOMS,   1      ); */

    if ((OLDhallfl = fopen("HALL.DAT", "r+b")) == NULL)
    {
        printf("Could not open hall.dat\n");
    }
    if ((NEWhallfl = fopen("NEWHALL.DAT", "w+b")) == NULL)
    {
        printf("Could not open newhall.dat\n");
    }

    getHall();


/* do conversion here */
    for (i = 0; i < MAXHALLS; i++)
    {
        strcpy(NEWhallBuf->hall[i].hallname, OLDhallBuf->hall[i].hallname);
        strcpy(NEWhallBuf->hall[i].htell, OLDhallBuf->hall[i].htell);

    NEWhallBuf->hall[i].grpno = OLDhallBuf->hall[i].grpno;
    NEWhallBuf->hall[i].mdefault = OLDhallBuf->hall[i].mdefault;

    NEWhallBuf->hall[i].h_inuse    = OLDhallBuf->hall[i].h_inuse;
    NEWhallBuf->hall[i].owned    = OLDhallBuf->hall[i].owned;
    NEWhallBuf->hall[i].described  = OLDhallBuf->hall[i].described;
    NEWhallBuf->hall[i].dmaint     = OLDhallBuf->hall[i].dmaint;
    NEWhallBuf->hall[i].enterRoom  = OLDhallBuf->hall[i].enterRoom;

    for (j = 0; j < MAXROOMS; j++)
    {
      /* if (OLDhallBuf->hall[i].hroomflags[j].inhall) */

        if (bit_test(OLDhallBuf->hall[i].inhall, j))
           bit_set(NEWhallBuf->hall[i].inhall, j);
        else
           bit_clear(NEWhallBuf->hall[i].inhall, j);

     /*  if (OLDhallBuf->hall[i].hroomflags[j].window) */

        if (bit_test(OLDhallBuf->hall[i].window, j))
           bit_set(NEWhallBuf->hall[i].window, j);
        else
           bit_clear(NEWhallBuf->hall[i].window, j);


    }
}

/* ****************** */

   putHall();

}




void convertroom(void)
{
    FILE *OLDroomfl, *NEWroomfl;

    int roomno = 0;

    /* open room file */

    if ((OLDroomfl = fopen("ROOM.DAT", "r+b")) == NULL)
    {
        printf("Could not open room.dat\n");
    }
    if ((NEWroomfl = fopen("NEWROOM.DAT", "w+b")) == NULL)
    {
        printf("Could not open newroom.dat\n");
    }


    
    do
    {
        printf("Converting room #%d\r", roomno);
        roomno++;


        if (roomno < 250)
        {

        if (fread(&OLDroomBuf, sizeof OLDroomBuf, 1, OLDroomfl) != 1)
        {
            printf("\nroom.dat EOF.\n");
            break;
        }

        /* do conversion here */
        NEWroomBuf.rbgen = OLDroomBuf.rbgen;
        NEWroomBuf.rbflags = OLDroomBuf.rbflags;
        strcpy(NEWroomBuf.rbname, OLDroomBuf.rbname);
        strcpy(NEWroomBuf.rbdirname, OLDroomBuf.rbdirname);
        strcpy(NEWroomBuf.rbroomtell, OLDroomBuf.rbroomtell);
        strcpy(NEWroomBuf.rbaplic, OLDroomBuf.rbaplic);
        NEWroomBuf.rbgrpno = OLDroomBuf.rbgrpno;
        NEWroomBuf.rbmodhash = OLDroomBuf.rbmodhash;
        NEWroomBuf.rbhosthash = OLDroomBuf.rbhosthash;
        NEWroomBuf.rbPgrpno = OLDroomBuf.rbPgrpno;

        NEWroomBuf.rbjunk[0] = OLDroomBuf.rbjunk[0];
        NEWroomBuf.rbjunk[1] = OLDroomBuf.rbjunk[1];

        strcpy(NEWroomBuf.descript, OLDroomBuf.descript);
        strcpy(NEWroomBuf.netID, OLDroomBuf.netID);

        /* ^^ do conversion here ^^ */

        }
        else
        {
            NEWroomBuf.rbname[0] = 0;
            NEWroomBuf.rbflags.INUSE = 0;
        }



        if (fwrite(&NEWroomBuf, sizeof NEWroomBuf, 1, NEWroomfl) != 1)
        {
            printf("newroom.dat write fucked.\n");
        }
    }         
    while (roomno < 1024);

    fclose(OLDroomfl);
    fclose(NEWroomfl);

     printf("Done.  Rename NEWROOM.DAT to ROOM.DAT\n");
     printf("       Please keep old ROOM.DAT just in case\n") ;

}








/************************************************************************/
/*      putHall() stores group data into hall.cit                       */
/************************************************************************/
void putHall(void)
{
    int i;

    fseek(NEWhallfl, 0L, 0);



    if ( fwrite(NEWhallBuf, sizeof (struct NEWhallBuffer), 1, NEWhallfl) != 1)
    {
        printf("putHall-write fail!");
    }
    
#ifdef GOODBYE
    for (i=0; i<MAXROOMS; i++)
         roomPos[i] = (char)i;

    if ( fwrite(roomPos, MAXROOMS, 1, NEWhallfl) != 1)
    {
        printf("putHall-write fail!");
    }
#endif    
    fflush(NEWhallfl);
}


/************************************************************************/
/*      getHall() loads hall data into RAM buffer                       */
/************************************************************************/
void getHall(void)
{
    
    fseek(OLDhallfl, 0L, 0);

    if (fread(OLDhallBuf, sizeof (struct hallBuffer), 1, OLDhallfl) != 1)
    {
        printf("getHall-EOF detected!");
    }
}

