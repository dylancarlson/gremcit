/* -------------------------------------------------------------------- */
/*  CHAT.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*                        Overlayed chat stuff                          */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  chat()          This is the chat mode                               */
/*  ringSysop()     ring the sysop                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  chat()          This is the chat mode                               */
/* -------------------------------------------------------------------- */
void chat(void)
{
    int c, from, lastfrom, wsize = 0, i;
    char word[50];

    chatkey = FALSE;
    chatReq = FALSE;

    if (!gotCarrier())
    {
        dial_out();
        return;
    }

    lastfrom = 2;

    tutorial("chat.blb");
    
    outFlag = IMPERVIOUS;
    setio(whichIO, echo, outFlag);

    do
    {
        c = 0;

        if (KBReady())
        {
            c = ciChar();
            from = 0;
        }

        if (MIReady())
        {
            if (!onConsole)
            {
                c = getMod();
                from = 1;
            } else {
                getMod();
            }
        }

        c = ( c & 0x7F );

        c = tfilter[c];

        if (c && c != 26 /* CNTRLZ */)
        {
            if (from != lastfrom)
            {
                if (from)
                {
                    termCap(TERM_NORMAL);
                    ansiattr = cfg.attr;
                }
                else
                {
                    termCap(TERM_BOLD);
                    ansiattr = cfg.cattr;
                }
                lastfrom = from;
            }
            
            if (c == '\r' || c == '\n' || c == ' ' || c == '\t' || wsize == 50)
            {
                wsize = 0;
            } else {
                if (crtColumn >= (uchar)(termWidth-1))
                {
                    if (wsize)
                    {
                        for (i = 0; i < wsize; i++)
                            doBS();
                        doCR();
                        for (i = 0; i < wsize; i++)
                            echocharacter(word[i]);
                    } else {
                        doCR();
                    }

                    wsize = 0;
                } else {
                    word[wsize] = (char)c;
                    wsize ++;
                }
            }

            echocharacter((char)c);
        }
    } while ( (c != 26 /* CNTRLZ */) && gotCarrier() );

    time(&lasttime);
    termCap(TERM_NORMAL);
    ansiattr = cfg.attr;

    doCR();
}

/* -------------------------------------------------------------------- */
/*  ringSysop()     ring the sysop                                      */
/* -------------------------------------------------------------------- */
void ringSysop(void)
{
    char i;
    char answered = FALSE;
    int  oldBells;
    static char shave[] = { 10, 5, 5, 10, 20, 10, 75 };
    char ringlimit = 30;
    int j = 0;

    /* turn the ringer on */
    oldBells = cfg.noBells;
    cfg.noBells = FALSE;
    
    mPrintf("\n Ringing sysop.");

    answered = FALSE;

    for (i = 0; (i < ringlimit) && !answered && CARRIER; i++)
    {
        oChar(7 /* BELL */); 

        pause(shave[j]);

        j++;
        if (j ==7)
        {
            j = 0;
        }

        if (debug)
        {
            mPrintf("%d", j);
        }


        if (BBSCharReady() || KBReady())
        {
            answered = TRUE;
        }
    }

    cfg.noBells = (uchar)oldBells;

    if (KBReady())  
    {
        chat();
    }
    else if (i >= ringlimit)
    {
        mPrintf("  Sorry, Sysop not around.\n ");
    }
    else iChar();
}


