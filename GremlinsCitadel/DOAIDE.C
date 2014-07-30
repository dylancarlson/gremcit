/* -------------------------------------------------------------------- */
/*  DOAIDE.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*        Code for doAide() and some function implemetations.           */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  doAide()        handles the aide-only menu                          */
/*  msgNym()        Aide message nym setting function                   */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data                                                         */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  doAide()        handles the aide-only menu                          */
/* -------------------------------------------------------------------- */
void doAide(char moreYet, char first)
/* char moreYet; */
/* char first; */    /* first parameter if TRUE              */
{
    int  roomExists();
    char oldchat;
    char ich;

    reverse = FALSE;

    if (moreYet)   first = '\0';

    mPrintf("Aide special fn: ");

    switch (toupper( first ? (char)first : (char)(ich=(char)iCharNE()) ))
    {
    case 'A':
        mPrintf("Attributes ");
        if (roomBuf.rbflags.MSDOSDIR != 1)
        {
            if (expert) mPrintf("? ");
            else        mPrintf("\n Not a directory room.");
        }
        else attributes();
        break;

    case 'C':
        chatReq = TRUE;
        oldchat = (char)cfg.noChat;
        cfg.noChat = FALSE;
        mPrintf("Chat\n ");
        if (whichIO == MODEM)   ringSysop();
        else                    chat() ;
        cfg.noChat = oldchat;
        break;

    case 'E':
        mPrintf("Edit room\n  \n");
        renameRoom();
        break;

    case 'F':
        mPrintf("File set\n  \n");
        batchinfo(TRUE);
        break;
    
    case 'G':
        mPrintf("Group membership\n  \n");
        groupfunc();
        break;

    case 'H':
        mPrintf("Hallway changes\n  \n");
        if (!cfg.aidehall && !sysop)
        {
            mPrintf(" Must be a Sysop!\n");
        }
        else
        {
            hallfunc();
        }
        break;
    case 'I':
        mPrintf("Insert %s\n ", cfg.msg_nym);
        insert();
        break;

    case 'K':
        mPrintf("Kill room\n ");
        killroom();
        break;

    case 'L':
        mPrintf("List group ");
        listgroup();
        break;

    case 'M':
        mPrintf("Move file ");
        moveFile();
        break;

    case 'N':
        mPrintf("Name Messages");
        msgNym();
        break;

    case 'Q':
        mPrintf("Queue fn: ");
    
        switch (toupper((char)(ich=(char)iCharNE()) ))
        {
        case 'A':
            mPrintf("Auto Mark/Kill");
            automark();
            break;
        case 'C':
            mPrintf("Clear");
            clearmsglist();
            break;
        case 'I':
            mPrintf("Insert");
            insertmsglist();
            break;
        case 'K':
            mPrintf("Kill");
            killmsglist();
            break;
        case 'L':
            mPrintf("List");
            printmsglist();
            break;
        case 'M':
            mPrintf("Move");  /* Combine Kill and Insert */
            killmsglist();
            insertmsglist();
            break;
        case 'R':
            mPrintf("Room Mark");  /* Mark whole room */
            markroom();
            break;
        case 'S':
            mPrintf("Sort");

            doCR();

            /* sort forward */
            if (getYesNo("Forward", 0 ))
            {
                sortmsglist(0);
                break;
            }

            /* sort reverse */
            if (getYesNo("Reverse", 0 ))
            {
                sortmsglist(1);
                break;
            }

            break;
        case '?':
            oChar('?');
            doCR();
            doCR();
            mPrintf(" 3A0>uto Mark/Kill\n 3C0>lear\n 3I0>nsert\n 3K0>ill\n ");
            mPrintf("3L0>ist\n 3M0>ove\n 3R0>oom Mark\n 3S0>ort\n 3?0> -- this menu\n ");
            break;
        default:
            oChar(ich);
            if (!expert)  mPrintf("\n '?' for menu.\n "  );
            else          mPrintf(" ?\n "                );
            break;
        } 
        break;
    case 'R':
        mPrintf("Rename file ");
        if (roomBuf.rbflags.MSDOSDIR != 1)
        {
            if (expert) mPrintf("? ");
            else        mPrintf("\n Not a directory room.");
        }
        else
        {
            renamefile();
        }
        break;

    case 'S':
        mPrintf("Set file info\n ");
        if (roomBuf.rbflags.MSDOSDIR != 1)
        {
            if (expert) mPrintf("? ");
            else        mPrintf("\n Not a directory room.");
        }
        else
        {
            setfileinfo();
        }
        break;

    case 'U':
        mPrintf("Unlink file\n ");
        if (roomBuf.rbflags.MSDOSDIR != 1)
        {
            if (expert) mPrintf("? ");
            else        mPrintf("\n Not a directory room.");
        }
        else
        {
            unlinkfile();
        }
        break;

    case 'V':
        mPrintf("View Help Text File\n ");
        tutorial("aide.hlp");
        break;

    case 'W':
        mPrintf("Window into hallway\n ");
        
        if (cfg.floors)
        {
            doCR();
            mPrintf("-- System in floor mode, no efect."); doCR();
            return;
        }
        
        if (!cfg.aidehall && !sysop)
        {
            mPrintf(" Must be a Sysop!\n");
        }
        else
        {
            windowfunc();
        }
        break;

    case '-':
        moveRoom(-1);
        break;
        
    case '+':
        moveRoom(1);
        break;
        
    case '?':
        oChar('?');
        tutorial("aide.mnu");
        break;

    default:
        oChar(ich);
        if (!expert)   mPrintf("\n '?' for menu.\n " );
        else           mPrintf(" ?\n "               );
        break;
    }
}


/* -------------------------------------------------------------------- */
/*  msgNym()        Aide message nym setting function                   */
/* -------------------------------------------------------------------- */
void msgNym(void)
{
    label stuff;

    doCR();
    if (!cfg.msgNym)
    {
        doCR();
        printf(" Message nyms not enabled!");
        doCR();
        return;
    }

getString("name (SINGLE)", stuff,  LABELSIZE, FALSE, ECHO, cfg.msg_nym);


                if (*stuff)
                {
                    strcpy(cfg.msg_nym, stuff);
                    normalizeString(cfg.msg_nym);  
                }

getString("name (PLURAL)", stuff, LABELSIZE, FALSE, ECHO,cfg.msgs_nym);

                if (*stuff)
                {
                    strcpy(cfg.msgs_nym, stuff);
                    normalizeString(cfg.msgs_nym);  
                }

getString("what to do to message", stuff, LABELSIZE, FALSE, ECHO, cfg.msg_done);

                if (*stuff)
                {
                    strcpy(cfg.msg_done, stuff);
                   normalizeString(cfg.msg_done);  
                }


    sprintf(msgBuf->mbtext, "\n Message nym changed by %s to\n "
                            "Single:   %s\n "
                            "Plural:   %s\n "
                            "Verb  :   %s\n ",
                            logBuf.lbname, 
                            cfg.msg_nym, cfg.msgs_nym, cfg.msg_done );
    aideMessage();
    /* doCR(); */
}


