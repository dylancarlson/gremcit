/* -------------------------------------------------------------------- */
/*  NETNODE.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*      Networking libs for the Citadel bulletin board system           */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  readnode()      read the node.cit to get the nodes info for logbuf  */
/*  getnode()       read the node.cit to get the nodes info             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  readnode()      read the node.cit to get the nodes info for logbuf  */
/* -------------------------------------------------------------------- */
BOOL readnode(void)
{
    BOOL toreturn;

    toreturn = getnode(logBuf.lbname);

    if (debug) 
    {
        doccr();
        cPrintf("Node:     %s", node.ndname);          doccr();
        cPrintf("Region:   %s", node.ndregion);        doccr();
        cPrintf("Phone:    %s", node.ndphone);         doccr();
        cPrintf("Baud:     %d", node.ndbaud);          doccr();
        cPrintf("Login:    %s", node.ndlogin);         doccr();
        cPrintf("Timeout:  %d", node.nddialto);        doccr();
        cPrintf("Protocol: %s", node.ndprotocol);      doccr();
        cPrintf("Expire:   %d", node.ndexpire);        doccr(); 
        cPrintf("Waitout:  %d", node.ndwaitto);        doccr();
        doccr();
    }

    return toreturn;
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  getnode()       read the node.cit to get the nodes info             */
/* -------------------------------------------------------------------- */
BOOL getnode(char *nodename)
{                          
    FILE *fBuf;
    char line[256], ltmp[256];
    char *words[256];
    int  i, j, k, found = FALSE;
    long pos;
    char path[80];

    memset(&node, 0, sizeof(struct nodest));

    node.ndbaud = cfg.initbaud; /* in case no #baud entry */
    
    sprintf(path, "%s\\nodes.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {  
        cPrintf("Can't find nodes.cit!"); doccr();
        return FALSE;
    }

    pos = ftell(fBuf);
    while (fgets(line, 254, fBuf) != NULL)
    {
        if (line[0] != '#')
        {
            pos = ftell(fBuf);
            continue;
        }

        if (!found && strnicmp(line, "#NODE", 5) != SAMESTRING)
        {
            pos = ftell(fBuf);
            continue;
        }
        
        strcpy(ltmp, line);
        parse_it( words, line);

        for (i = 0; nodekeywords[i] != NULL; i++)
        {
            if (strcmpi(words[0], nodekeywords[i]) == SAMESTRING)
            {
                break;
            }
        }

        if (i == NOK_NODE)
        {
            if (found)
            {
                fclose(fBuf);
                return TRUE;
            }

            if (strcmpi(nodename,  words[1]) == SAMESTRING)
            {
                found = TRUE;  
            }
            else
            {
                continue;
            }
        }     

        switch(i)
        {

        case NOK_BAUD:
            j = atoi(words[1]);
            switch(j) /* ycky hack */
            {
            case 300:
                node.ndbaud = 0;
                break;
            case 1200:
                node.ndbaud = 1;
                break;
            case 2400:
                node.ndbaud = 2;
                break;
            case 4800:
                node.ndbaud = 3;
                break;
            case 9600:
                node.ndbaud = 4;
                break;
            default:
                node.ndbaud = 1;
                break;
            }
            break;


        case NOK_PHONE:
            if (strlen(words[1]) < 50)
                strcpy(node.ndphone, words[1]);
            break;
        
        case NOK_ZIP:
            if (strlen(words[1]) < 40)
                strcpy(node.zip,   words[1]);
            if (strlen(words[2]) < 40)
                strcpy(node.unzip, words[2]);

#ifdef GOODBYE
            if (!filexists(node.zip) || !filexists(node.unzip))
            {
                doccr();
                cPrintf("Nodes.cit - %s Error: File not found ", words[0]);
                doccr();
            }    
#endif

            break;

        case NOK_NETWORK:
             if (strcmpi(words[1], "DRAGCIT1_0") == SAMESTRING) ||
                (strcmpi(words[1], "DRAGCIT1_1") == SAMESTRING))
            {
                node.network = NET_DCIT10;
            }
            
            if (strcmpi(words[1], "DRAGCIT1_5") == SAMESTRING)
            {
                node.network = NET_DCIT15;
            }
            
            if (strcmpi(words[1], "DRAGCIT1_6") == SAMESTRING)
            {
                node.network = NET_DCIT16;
            }
            
            if (strcmpi(words[1], "HENGE") == SAMESTRING)
            {
                node.network = NET_HENGE;
            }

            if (strcmpi(words[1], "NET1_69") == SAMESTRING)
            {
                node.network = NET_1_69;
            }
            if (strcmpi(words[1], "NET6_9") == SAMESTRING)
            {
                node.network = NET_6_9;
            }

            break;
        
        case NOK_PROTOCOL:
            if (strlen(words[1]) < 20)
                strcpy(node.ndprotocol, words[1]);
            break;

        case NOK_MAIL_TMP:
            if (strlen(words[1]) < 20)
                strcpy(node.ndmailtmp, words[1]);
            break;

        case NOK_LOGIN:
            strcpy(node.ndlogin, ltmp);
            break;

        case NOK_NODE:
            if (strlen(words[1]) < 20)
                strcpy(node.ndname, words[1]);
            if (strlen(words[2]) < 20)
                strcpy(node.ndregion, words[2]);
            for (j=0; j<MAXGROUPS; j++)
                node.ndgroups[j].here[0] = '\0';
            node.roomoff = 0L;
            break;

#ifdef GOODBYE
        case NOK_REDIAL:
            node.ndredial = atoi(words[1]);
            break;
#endif

        case NOK_DIAL_TIMEOUT:
            node.nddialto = atoi(words[1]);
            break;

        case NOK_WAIT_TIMEOUT:
            node.ndwaitto = atoi(words[1]);
            break;

        case NOK_EXPIRE:
            node.ndexpire = atoi(words[1]);
            break;

        case NOK_AUTOROOM:
            node.autoroom = atoi(words[1]);
            break;

        case NOK_VERBOSE:
            node.verbose = atoi(words[1]);
            break;

        case NOK_ROOM:
            node.roomoff = pos;
            fclose(fBuf);
            return TRUE;

        case NOK_GROUP:
            for (j = 0, k = ERROR; j < MAXGROUPS; j++)
            {
                if (node.ndgroups[j].here[0] == '\0') 
                {
                    k = j;
                    j = MAXGROUPS;
                }
            } 

            if (k == ERROR) 
            {
                cPrintf("Too many groups!!\n ");
               break;
            }
            
            if (strlen(words[1]) < 20)
                strcpy(node.ndgroups[k].here,  words[1]);
            if (strlen(words[2]) < 20)
                strcpy(node.ndgroups[k].there, words[2]);
            if (!strlen(words[2]))
                strcpy(node.ndgroups[k].there, words[1]);
            break;

        default:
            doccr();
            cPrintf("Nodes.cit - Warning: Unknown variable %s", words[0]);
            doccr();
            break;
        }
        pos = ftell(fBuf);
    }
    fclose(fBuf);
    return (BOOL)(found);
}
#endif



/* -------------------------------------------------------------------- */
/*  getnode()       read the nodes.cit to get the nodes info            */
/* -------------------------------------------------------------------- */
BOOL getnode(char *nodename)
{
    FILE *fBuf;
    char line[256], ltmp[256];
    char *words[256];
    int  i, j, k, found = FALSE;
    long pos;
    char path[80];
    char *toomany = "\n%s over %d characters (%s)";

    memset(&node, 0, sizeof(struct nodest));
    node.ndbaud = cfg.initbaud; /* in case no #baud entry */
    
    sprintf(path, "%s\\nodes.cit", cfg.homepath);

    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {  
        cPrintf("\nCan't find nodes.cit!\n");
        return FALSE;
    }

    pos = ftell(fBuf);
    while (fgets(line, 254, fBuf) != NULL)
    {
        if (line[0] != '#')
        {
            pos = ftell(fBuf);
            continue;
        }

        if (!found && strnicmp(line, "#NODE", 5) != SAMESTRING)
        {
            pos = ftell(fBuf);
            continue;
        }
        
        strcpy(ltmp, line);
        parse_it( words, line);

        for (i = 0; nodekeywords[i] != NULL; i++)
        {
            if (strcmpi(words[0], nodekeywords[i]) == SAMESTRING)
            {        /* ^ add one for '#' */
                break;
            }
        }

        if (i == NOK_NODE)
        {
            if (found)
            {
                fclose(fBuf);
                return TRUE;
            }

            if (strcmpi(nodename,  words[1]) == SAMESTRING)
            {
                found = TRUE;  
            }
            else
            {
                continue;
            }
        }     

        switch(i)
        {
        case NOK_BAUD:
            node.ndbaud = digitbaud((uint)atol(words[1]));
            break;

        case NOK_PHONE:
            if (strlen(words[1]) <= 49)
                strcpy(node.ndphone, words[1]);
            else
                cPrintf(toomany, nodekeywords[i], 49, words[1]);
            break;

        case NOK_ZIP:
            if (strlen(words[1]) <= 39)
                strcpy(node.zip,   words[1]);
            else
                cPrintf(toomany, nodekeywords[i], 39, words[1]);

            if (strlen(words[2]) <= 39)
                strcpy(node.unzip, words[2]);
            else
                cPrintf(toomany, nodekeywords[i], 39, words[1]);
            break;

        case NOK_NETWORK:

#ifdef GOODBYE
            if (strcmpi(words[1], "DRAGCIT1_0") == SAMESTRING)
#endif

             if ( 
                   (strcmpi(words[1], "DRAGCIT1_0") == SAMESTRING) ||
                   (strcmpi(words[1], "DRAGCIT1_1") == SAMESTRING)
                )
            {
           
                node.network = NET_DCIT10;
            }
            else
            if (strcmpi(words[1], "DRAGCIT1_5") == SAMESTRING)
            {
                node.network = NET_DCIT15;
            }
            else
            if (strcmpi(words[1], "DRAGCIT1_6") == SAMESTRING)
            {
                node.network = NET_DCIT16;
            }
            else
            if (strcmpi(words[1], "NET1_69") == SAMESTRING)
            {
                node.network = NET_1_69;
            }
            else
            if (strcmpi(words[1], "NET6_9") == SAMESTRING)
            {
                node.network = NET_6_9;
            }
            else
            if (strcmpi(words[1], "HENGE") == SAMESTRING)
            {
                node.network = NET_HENGE;
            }
            else
                cPrintf("\nInvalid network type (%s)", words[1]);
            break;
        
        case NOK_PROTOCOL:
            if (strlen(words[1]) <= LABELSIZE)
                strcpy(node.ndprotocol, words[1]);
            else
                cPrintf(toomany, nodekeywords[i], LABELSIZE, words[1]);
            break;

        case NOK_MAIL_TMP:
            if (strlen(words[1]) <= LABELSIZE)
                strcpy(node.ndmailtmp, words[1]);
            else
                cPrintf(toomany, nodekeywords[i], LABELSIZE, words[1]);
            break;

        case NOK_LOGIN:
            strcpy(node.ndlogin, ltmp);
            break;

        case NOK_NODE:
            if (strlen(words[1]) <= LABELSIZE)
                strcpy(node.ndname, words[1]);
            else
                cPrintf(toomany, nodekeywords[i], LABELSIZE, words[1]);
            if (strlen(words[2]) <= LABELSIZE)
                strcpy(node.ndregion, words[2]);
            else
                cPrintf(toomany, nodekeywords[i], LABELSIZE, words[2]);
            for (j=0; j<MAXGROUPS; j++)
                node.ndgroups[j].here[0] = '\0';
            node.roomoff = 0L;
            break;

#ifdef GOODBYE
        case NOK_REDIAL:
            node.ndredial = atoi(words[1]);
            break;
#endif

        case NOK_DIAL_TIMEOUT:
            node.nddialto = atoi(words[1]);
            break;

        case NOK_WAIT_TIMEOUT:
            node.ndwaitto = atoi(words[1]);
            break;

        case NOK_EXPIRE:
            node.ndexpire = atoi(words[1]);
            break;

        case NOK_AUTOROOM:
            node.autoroom = atoi(words[1]);
            break;

        case NOK_VERBOSE:
            node.verbose = atoi(words[1]);
            break;

        case NOK_ROOM:
            node.roomoff = pos;
            fclose(fBuf);
            return TRUE;

        case NOK_GROUP:
            for (j = 0, k = ERROR; j < MAXGROUPS; j++)
            {
                if (node.ndgroups[j].here[0] == '\0') 
                {
                    k = j;
                    j = MAXGROUPS;
                }
            } 

            if (k == ERROR) 
            {
                cPrintf("\nToo many groups!!");
               break;
            }
            
            if (strlen(words[1]) <= LABELSIZE)
                strcpy(node.ndgroups[k].here,  words[1]);
            else
                cPrintf(toomany, "local group name", LABELSIZE, words[1]);

            if (strlen(words[2]) <= LABELSIZE)
                strcpy(node.ndgroups[k].there, words[2]);
            else
                cPrintf(toomany, "remote group name", LABELSIZE, words[2]);
            if (!strlen(words[2]))
                strcpy(node.ndgroups[k].there, words[1]);
            break;

        default:
            cPrintf("\nNodes.cit - Warning: Unknown variable %s", words[0]);
            break;
        }
        pos = ftell(fBuf);
    }
    fclose(fBuf);
    return (BOOL)(found);
}



