/* -------------------------------------------------------------------- */
/*  NETMAIL.C                Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*      Networking libs for the Citadel bulletin board system           */
/*                       NetMail related code.                          */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  NfindRoom()     find the room for main (unimplmented, ret: MAILROOM)*/
/*  alias()         return the name of the BBS from the #ALIAS          */
/*  route()         return the routing of a BBS from the #ROUTE         */
/*  alias_route()   returns the route or alias specified                */
/*  save_mail()     save a message bound for another system             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* static BOOL region_route(char *reg, char *str); */

/* -------------------------------------------------------------------- */
/*  NfindRoom()     find the room for main (unimplmented, ret: MAILROOM)*/
/* -------------------------------------------------------------------- */
int NfindRoom(char *str)
{
    int i = MAILROOM; 

    str[0] = str[0]; /* -W3 */

    if (cfg.netmail)
    {
        i = roomExists(str);

        if (i == ERROR)
            i = MAILROOM;  
    }
    return(i);
}

/* -------------------------------------------------------------------- */
/*  alias()         return the name of the BBS from the #ALIAS          */
/* -------------------------------------------------------------------- */
BOOL alias(char *str)
{
    return alias_route(str, "#ALIAS");
}

/* -------------------------------------------------------------------- */
/*  route()         return the routing of a BBS from the #ROUTE         */
/* -------------------------------------------------------------------- */
BOOL route(char *str)
{
    return alias_route(str, "#ROUTE");
}

/* -------------------------------------------------------------------- */
/*  alias_route()   returns the route or alias specified                */
/* -------------------------------------------------------------------- */
BOOL alias_route(char *str, char *srch)
{                          
    FILE *fBuf;
    char line[95];
    char *words[256];
    char path[80];

    sprintf(path, "%s\\route.cit", cfg.homepath);
    
    if ((fBuf = fopen(path, "r")) == NULL)  /* ASCII mode */
    {  
        crashout("Can't find route.cit!");
    }

    while (fgets(line, 90, fBuf) != NULL)
    {
        if (line[0] != '#') continue;
   
        if (strnicmp(line, srch, 5) != SAMESTRING) continue;
     
        parse_it( words, line);

        if (strcmpi(srch, words[0]) == SAMESTRING)
        {
            if (strcmpi(str, words[1]) == SAMESTRING)
            {
                fclose(fBuf);
                strcpy(str, words[2]);
                return TRUE;
            }
        }
    }
    fclose(fBuf);
    return FALSE;
}

/* -------------------------------------------------------------------- */
/*  save_mail()     save a message bound for another system             */
/* -------------------------------------------------------------------- */
BOOL save_mail()
{
    label tosystem;
    char  filename[100];
    FILE *fl;

    /* where are we sending it? */
    strcpy(tosystem, msgBuf->mbzip);
    
    /* send it vila... */
    route(tosystem);

    /* get the node entery */
    if (!getnode(tosystem))
    {
        /* see if there is a gateway or region route.. */
        if (*msgBuf->mbrzip)
        {
            switch(region_route(msgBuf->mbrzip, tosystem))
            {
            default:
            case 0:         /* hu? */
                return FALSE;
            
            case 1:         /* region route */
                if (!getnode(tosystem))
                    return FALSE;
                /* can you see the repeated code in this function? */
                sprintf(filename, "%s\\%s", cfg.transpath, node.ndmailtmp);
                break;
            
            case 2:         /* gateway */
                sprintf(filename, "%s\\%s", cfg.transpath, tosystem);
                break;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        sprintf(filename, "%s\\%s", cfg.transpath, node.ndmailtmp);
    }

    if ((fl = fopen(filename, "ab")) == NULL)
    {
        /* perror("Cannot open mail file"); */
        return FALSE;
    }


#ifdef GOODBYE
    fl = fopen(filename, "ab");
    if (!fl) return FALSE;
#endif

    PutMessage(fl);

    fclose(fl);

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  region_route()   returns the route or alias specified                */
/* -------------------------------------------------------------------- */
BOOL region_route(char *reg, char *str)
{                          
    label s;
    
    strcpy(s, reg);
    
    if (alias_route(s, "#REGION_ROUTE"))
    {
        strcpy(str, s);
        return 1;
    }
    
    if (alias_route(s, "#REGION_GATE"))
    {
        strcpy(str, s);
        return 2;
    }

    return 0;
}


