/* 
 *  Citadel to Door (CIT2DOOR.C)
 *
 *  Program to convert DragCit's OUTPUT.APL to other formats for programs 
 *  designed to run under other BBS software like QBBS and WWIVs. 
 *
 *  Writen by Peter Torkelson and released into the public domain.
 *  Updated by Richard Goldfinder to handle different EOLs.
 *
 *  Compiled with Turbo C++ 1.0, should compile under any ANSI C compiler.
 *
 */

/*
 *  History:
 *
 *  1.2 07/29/93  - Changed readOutputApl() to strip off any '\r'
 *                  or '\n' found at the end of the line rather
 *                  than just blindly removing the last character.
 *                  Needed for any OUTPUT.APL that has both carriage
 *                  return and linefeed instead of just linefeed
 *                  (improved readability).
 *
 *  1.1 11/10/90  - Fixed CHAIN.TXT to reflect actual time. Minutes WERE
 *                  mapped to seconds. It would tell you that you had 27 
 *                  seconds left in the game, not 27 minutes. Opps. :-)
 *
 *  1.0 11/07/90  - Created with DORINFO1.DEF and CHAIN.TXT ability.
 */ 
 
/*
 * Compiler Includes 
 */ 
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

/*
 * DragCit includes
 */ 
#include  "applic.h"

/*
 * Prototypes
 */ 
void main(int argc, char *argv[]);
void readOutputApl(void);
void outputQBBS(void);
void outputWWIV(void);
 
/*
 * Defines
 */ 
#define     QBBS    0
#define     WWIV    1

/*
 * Global varibles
 */ 
typedef char label[31];

void readOutputApl(void);
void outputWWIV(void);
void outputQBBS(void);

char    outputApl[80] = "OUTPUT.APL";
label   uname;
int     sysop;
int     aide;
int     scrn_size = 80;
int     scrn_lines = 24;

label   node;
label   sysopFn = "";
label   sysopLn = "";
int     port;
int     baud;
int     nettype = 0;
label   fname;
label   lname = "";
label   city = "SEATTLE, WA";
int     ansi;
int     level = 0;  /* 0 or 20 for DORINFO1.DEF */
int     timeleft;
int     fossil = 0;

int     bbs = -1;

/*
 * Where it all began
 */
void main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        printf("\n"
               "CIT2DOOR by Peter Torkelson\n"
               "Updated to handle different EOLs by Richard Goldfinder\n"
               "Released to the public domain.\n"
               "\n"
               "Usage: CIT2DOOR BBS_TYPE [OUTPUT.APL]\n"
               "\n"
               "    BBS_TYPE   = QBBS to make a DORINFO1.DEF\n"
               "                 WWIV to make a CHAIN.TXT\n"
               "    OUTPUT.APL = Full path and filename of the OUTPUT.APL\n"
               "                 Defaultys to OUTPUT.APL in curent path\n"
               "\n"
                );
        exit(1);
    }
    
    if (stricmp(argv[1], "WWIV") == 0)
    {
        bbs = WWIV;
    }
    
    if (stricmp(argv[1], "QBBS") == 0)
    {
        bbs = QBBS;
    }
    
    if (argc == 3)
        strcpy(outputApl, argv[2]);
    
    readOutputApl();
    
    switch(bbs)
    {
    case QBBS:
        outputQBBS();
        break;
    
    case WWIV:
        outputWWIV();
        break;
    
    default:
        break;
    }
}

/*
 * Read the OUTPUT.APL into globals, get all we need to know.
 */    
void readOutputApl(void)
{
    FILE *fl;
    char c;
    char *eol;
    char str[80];
    char done = 0;
    static int bauds[] = { 300, 1200, 2400, 4800, 9600, 14400, 19200};
    int account;
    
    if ((fl = fopen(outputApl, "rb")) == NULL)
    {
        printf("\n\n Can not open %s!\n", outputApl);
        exit(1);
    }
    
    while (!feof(fl) && !done)
    {
        c = fgetc(fl);
        fgets(str, 79, fl);
        eol = str + strlen(str) - sizeof(char);
        while (*eol == '\n' || *eol == '\r')
            {
            *eol-- = '\0';
            }
        
        switch (c)
        {
        case APL_NAME:
            strcpy(uname, str);
            break;
        
        case APL_NODE:
            strcpy(node, str);
            break;
        
        case APL_SYSOP:
            sysop = atoi(str);
            break;
        
        case APL_CREDITS:
            timeleft = atoi(str);
            break;
        
        case APL_AIDE:
            aide = atoi(str);
            break;
        
        case APL_ANSION:
            ansi = atoi(str);
            break;
        
        case APL_MDATA:
            port = atoi(str);
            break;
        
        case APL_ACCOUNTING:
            account = atoi(str);
            break;
        
        case APL_COLUMNS:
            scrn_size = atoi(str);
            break;
        
        case APL_LINES:
            scrn_lines = atoi(str);
            break;
        
        case APL_BAUD:
            baud = atoi(str);
            baud = bauds[baud];
            break;
        
        case APL_END:
            done = 1;
            break;
        
        default: 
            break;
        }
    }
    
    if (!account)
    {
        timeleft = 120;
    }
    
    fclose(fl);
}

/*
 * Write DORINFO1.DEF from globals.
 */
void outputQBBS(void)
{
    FILE *fl;
    
    strupr(node);
    strupr(uname);
    if (sysop)
    {
        strcpy(sysopFn, uname);
        level = 20;
    }
    else
    {
        strcpy(sysopFn, "SYSOP");
        level = 0;
    }
    strcpy(fname, uname);

    if ((fl = fopen("DORINFO1.DEF", "wt")) == NULL)
    {
        printf("\n\nCould not open DORINFO1.DEF for write!\n");
        exit(1);
    }

    fprintf(
        fl,
        "%s\n%s\n%s\nCOM%d\n%d BAUD,N,8,1\n%d\n%s\n%s\n%s\n%d\n%d\n%d\n%d\n",
        node,
        sysopFn,
        sysopLn,
        port,
        baud,
        nettype,
        fname,
        lname,
        city,
        ansi,
        level,
        timeleft,
        fossil
    );
    
    fclose(fl);
}

/*
 * Write CHAIN.TXT from globals.
 */
void outputWWIV(void)
{
    FILE *fl;
    static char    lastOn[20] = "11/01/90";
    
    strupr(node);
    strupr(uname);
    if (sysop)
    {
        level = 255;
    }
    
    if (aide)
    {
        level = 100;
    }
    
    strcpy(fname, uname);

    if ((fl = fopen("CHAIN.TXT", "wt")) == NULL)
    {
        printf("\n\nCould not open CHAIN.TXT for write!\n");
        exit(1);
    }

    fprintf(
        fl,
        "1\n%s\n%s\n\n21\nM\n100\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n"
        ".\\\n.\\\nSYSOP.LOG\n%d\n%d\n",
        uname,
        uname,
        lastOn,
        scrn_size,
        scrn_lines,
        level,
        sysop,
        aide,
        ansi,
        port,
        timeleft * 60,
        baud,
        port ? port : 1
    );
    
    fclose(fl);
}

