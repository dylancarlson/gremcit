/************************************************************************/
/*                            keywords.h                                */
/*        all keywords used by Citadel configuration program            */
/************************************************************************/

/************************************************************************/
/*                         config.cit keywords                          */
/************************************************************************/
#ifdef MAIN
char *keywords[] = 
{
    "#ACCOUNTING",      
    "#AIDEHALL",
    "#APPLICPATH",
    "#ATTR",            
    "#BATTR",
    "#BIOS",
    "#CATTR",
    "#DATESTAMP",
    "#DIAL_INIT",             
    "#DIAL_PREF",
    "#DUMB_MODEM",      
    "#ENTER_NAME",
    "#F6PASSWORD",
    "#FLOORS",
    "#HELPPATH",        
    "#HOMEPATH",        
    "#IDLE_WAIT",       
    "#INIT_BAUD",
    "#KILL",
    "#LINEFEEDS",       
    "#LINES_SCREEN",
    "#LOGIN",
    "#VDATESTAMP",
    "#MAXBALANCE",      
    "#MAXFILES",
    "#MAXLOGTAB",       
    "#CHECKCTS",
    "#NODEPHONE",
    "#MDATA",           
    "#MESSAGEK",        
    "#MESSAGE_ROOM",
    "#MODERATE",
    "#MODSETUP",        
    "#MSGPATH",         
    "#NET_PREFIX",
    "#NEWBAL",
    "#NEWUSERAPP",
    "#NMESSAGES",       
    "#NODECOUNTRY",
    "#NODENAME",        
    "#NODEREGION",
    "#NOPWECHO",        
    "#NULLS",           
    "#OFFHOOK",
    "#OLDCOUNT",        
    "#PRINTER",         
    "#READLLOG",
    "#READOLD",
    "#ROOMOK",          
    "#ROOMPATH",
    "#ROOMTELL",
    "#TABS",            
    "#TEMPPATH",        
    "#TIMEOUT",         
    "#TRANSPATH",
    "#TRAP",            
    "#TRAP_FILE",       
    "#TWIT_FEATURES",
    "#UNLOGGEDBALANCE", 
    "#UNLOGTIMEOUT",    
    "#UPPERCASE",       
    "#USER",            
    "#UTTR",
    "#PROMPT",
    "#WATTR",
    "#WIDTH",           
    "#SIGNATURE",
    "#ANONAUTHOR",
    "#DIAL_RING",
    "#UP_HOURS",
    "#TWITREGION",
    "#TWITCOUNTRY",
    "#DIRPATH",
    "#UP_DAYS",
    "#NETMAIL",
    "#SUBHUBS",
    "#SYSOP",
    "#ENTEROK",
    "#READOK",
    "#FORCELOGIN",
    "#ALIAS",
    "#SCREENSAVE",
    "#MESSAGE_NYM",
    "#BORDER",
    "#MSGHEAPPAGES",   
    "#VIRTMEM",
    "#DOWNSHIFT",
    "#HANGUPDELAY",
    "#AUTOANSI",
    "#FORWARD",
    "#CONNECTWAIT",
    NULL
};
#else
extern char *keywords[];
#endif /* MAIN */
                       
#define     K_ACCOUNTING        0 
#define     K_AIDEHALL          1 
#define     K_APPLICATIONS      2 
#define     K_ATTR              3 
#define     K_BATTR             4 
#define     K_BIOS              5 
#define     K_CATTR             6 
#define     K_DATESTAMP         7 
#define     K_DIAL_INIT         8 
#define     K_DIAL_PREF         9
#define     K_DUMB_MODEM        10
#define     K_ENTER_NAME        11
#define     K_F6PASSWORD        12
#define     K_FLOORS            13
#define     K_HELPPATH          14
#define     K_HOMEPATH          15
#define     K_IDLE_WAIT         16
#define     K_INIT_BAUD         17
#define     K_KILL              18
#define     K_LINEFEEDS         19
#define     K_LINES_SCREEN      20
#define     K_LOGIN             21
#define     K_VDATESTAMP        22
#define     K_MAXBALANCE        23
#define     K_MAXFILES          24
#define     K_MAXLOGTAB         25
#define     K_CHECKCTS          26
#define     K_NODEPHONE         27
#define     K_MDATA             28
#define     K_MESSAGEK          29
#define     K_MESSAGE_ROOM      30
#define     K_MODERATE          31
#define     K_MODSETUP          32
#define     K_MSGPATH           33
#define     K_NET_PREFIX        34
#define     K_NEWBAL            35
#define     K_NEWUSERAPP        36
#define     K_NMESSAGES         37
#define     K_NODECONTRY        38
#define     K_NODENAME          39
#define     K_NODEREGION        40
#define     K_NOPWECHO          41
#define     K_NULLS             42
#define     K_OFFHOOK           43
#define     K_OLDCOUNT          44
#define     K_PRINTER           45
#define     K_READLLOG          46
#define     K_READOLD           47
#define     K_ROOMOK            48
#define     K_ROOMPATH          49
#define     K_ROOMTELL          50
#define     K_TABS              51
#define     K_TEMPPATH          52
#define     K_TIMEOUT           53
#define     K_TRANSPATH         54
#define     K_TRAP              55
#define     K_TRAP_FILE         56
#define     K_TWIT_FEATURES     57
#define     K_UNLOGGEDBALANCE   58
#define     K_UNLOGTIMEOUT      59
#define     K_UPPERCASE         60
#define     K_USER              61
#define     K_UTTR              62
#define     K_PROMPT            63
#define     K_WATTR             64
#define     K_WIDTH             65
#define     K_NODESIG           66
#define     K_ANONAUTHOR        67
#define     K_DIAL_RING         68
#define     K_UP_HOURS          69
#define     K_TWITREGION        70
#define     K_TWITCOUNTRY       71
#define     K_DIRPATH           72
#define     K_UP_DAYS           73
#define     K_NETMAIL           74
#define     K_SUBHUBS           75
#define     K_SYSOP             76
#define     K_ENTEROK           77
#define     K_READOK            78
#define     K_FORCELOGIN        79
#define     K_ALIAS             80
#define     K_SCREENSAVE        81
#define     K_MESSAGE_NYM       82
#define     K_BORDER            83
#define     K_MSGHEAPPAGES      84
#define     K_VIRTMEM           85
#define     K_DOWNSHIFT         86
#define     K_HANGUPDELAY       87
#define     K_AUTOANSI          88
#define     K_FORWARD           89
#define     K_CONNECTWAIT       90
                                 
#define     K_NWORDS            90   /* must be changed to add new keywords */


#ifdef MAIN
char *trapkeywords[] =
{
    "ALL",
    "CARRIER",
    "LOGIN",
    "NEWROOM",
    "PROBLEM_USER",
    "CHAT",
    "PASSWORD",
    "AIDE",
    "SYSOP",
    "UPLOAD",
    "DOWNLOAD",
    "ERROR",
    "NETWORK",
    "ACCOUNT",
    "APPLIC",
    "CRON",
    NULL
};

char *userkeywords[] = 
{   
    "PROBLEM_USER",
    "PERMANENT",
    "NO_ACCOUNTING",
    "NETWORK",
    "NO_MAIL",
    "AIDE",
    "SYSOP",
    "BORDERS",
    NULL
};

char *twitfeatures[] = 
{   
    "MSG_NYMS",
    "BORDER_LINES",
    "TITLES",
    "NET_TITLES",
    "SURNAMES",
    "NET_SURNAMES",
    "ENTER_TITLES",
    "COLORS",
    NULL
};
#else
extern char *trapkeywords[];
extern char *userkeywords[];
extern char *twitfeatures[];
#endif /* MAIN */

#ifdef MAIN
char *newuserkeywords[] = 
{
    "CLOSED_SYSTEM",
    "VERIFIED",
    "QUESTIONS",
    "APPLICATION",
    "SYSOP_MESSAGE",
    "NEW_ACCOUNTS",
    NULL
};
#else
extern char *newuserkeywords[];
#endif /* MAIN */

#define     L_CLOSED        0
#define     L_VERIFIED      1
#define     L_QUESTIONS     2
#define     L_APPLICATION   3
#define     L_SYSOP_MESSAGE 4
#define     L_NEW_ACCOUNTS  5

/************************************************************************/
/*                        grpdata.cit keywords                          */
/************************************************************************/

#ifdef MAIN
char *grpkeywords[] =
{
    "#DAYS",
    "#GROUP",
    "#HOURS",
    "#DAY_INC",
    "#SPECIAL",
    "#PRIORITY",
    "#MAX_BAL",
    "#DL_MULT",
    "#UL_MULT",
    NULL
};
#else
extern char *grpkeywords[];
#endif /* MAIN */

#define GRK_DAYS              0
#define GRK_GROUP             1
#define GRK_HOURS             2
#define GRK_DAYINC            3
#define GRK_SPECIAL           4
#define GRK_PRIORITY          5
#define GRK_MAXBAL            6
#define GRK_DLMULT            7
#define GRK_ULMULT            8

#ifdef MAIN
char *daykeywords[] = 
{
   "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Any", NULL 
};
#else
extern char *daykeywords[];
#endif


/************************************************************************/
/*                         nodes.cit keywords                           */
/************************************************************************/

#ifdef MAIN
char *nodekeywords[] = 
{
    "#BAUD",
    "#DISCARD",
    "#DISCARD_USER",
    "#EXPIRE",
    "#GROUP",
    "#DIAL_TIMEOUT",
    "#LOGIN",
    "#WAIT_TIMEOUT",
    "#PROTOCOL",
    "#NODE",
    "#PHONE",
    "#MAIL_TMP",
    "#ROOM",
    "#ROUTE",
    "#ALIAS",
    "#REGION",
    "#NETWORK",
    "#ZIP",
    "#CLEANUP",
    "#AUTOROOM",
    "#VERBOSE",
    NULL
};
#else
extern char *nodekeywords[];
#endif /* MAIN */

#define NOK_BAUD          0
#define NOK_DISCARD       1
#define NOK_DISCARD_USER  2
#define NOK_EXPIRE        3
#define NOK_GROUP         4
#define NOK_DIAL_TIMEOUT  5
#define NOK_LOGIN         6
#define NOK_WAIT_TIMEOUT  7
#define NOK_PROTOCOL      8
#define NOK_NODE          9
#define NOK_PHONE         10
#define NOK_MAIL_TMP      11
#define NOK_ROOM          12
#define NOK_ROUTE         13
#define NOK_ALIAS         14
#define NOK_REGION        15
#define NOK_NETWORK       16
#define NOK_ZIP           17
#define NOK_CLEANUP       18
#define NOK_AUTOROOM      19
#define NOK_VERBOSE       20

#define NET_DCIT10        0
#define NET_DCIT15        1
#define NET_HENGE         2
#define NET_DCIT16        3
#define NET_1_69          4
#define NET_6_9           5

/************************************************************************/
/*                         cron.cit keywords                            */
/************************************************************************/
#ifdef MAIN
char *cronkeywords[] = 
{
  "#DO",
  "#HOURS",
  "#DAYS",
  "#REDO_TIME",
  "#RETRY_TIME",
  "#PRIORITY",
  NULL  
};
#else
extern char *cronkeywords[];
#endif /* MAIN */

#define CR_DO            0
#define CR_HOURS         1
#define CR_DAYS          2
#define CR_REDO          3
#define CR_RETRY         4
#define CR_PRIORITY      5

#ifdef MAIN
char *crontypes[] = 
{
  "NETWORK",
  "SHELL_1",
  "SHELL_2",
  "SHUTDOWN",
  "CHAT_ON",
  "CHAT_OFF",
  NULL
};
#else
extern char *crontypes[];
#endif

#define CR_NET      0
#define CR_SHELL_1  1
#define CR_SHELL_2  2
#define CR_SHUTDOWN 3
#define CR_CHAT_ON  4
#define CR_CHAT_OFF 5

