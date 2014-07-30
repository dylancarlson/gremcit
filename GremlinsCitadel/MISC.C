/* -------------------------------------------------------------------- */
/*  MISC.C                   Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  Citadel garbage dump, if it aint elsewhere, its here.               */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  crashout()      Fatal system error                                  */
/*  exitcitadel()   Done with cit, time to leave                        */
/*  filexists()     Does the file exist?                                */
/*  hash()          Make an int out of their name                       */
/*  initCitadel()   Load up data files and open everything.             */
/*  openFile()      Special to open a .cit file                         */
/*  hmemcpy()       Terible hack to copy over 64K, beacuse MSC cant.    */
/*  h2memcpy()      Terible hack to copy over 64K, beacuse MSC cant. PT2*/
/*  changedir()     changes curent drive and directory                  */
/*  changedisk()    change to another drive                             */
/*  ltoac()         change a long into a number with ','s in it         */
/*  doBorder()      print a boarder line.                               */
/*  editBorder()    edit a boarder line.                                */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  05/26/89    (PAT)   Many of the functions move to other modules     */
/*  02/08/89    (PAT)   History Re-Started                              */
/*                      InitAideMess and SaveAideMess added             */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  Static Data definitions                                             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  filexists()     Does the file exist?                                */
/* -------------------------------------------------------------------- */
BOOL filexists(char *filename)
{
    return (BOOL)((access(filename, 4) == 0) ? TRUE : FALSE);
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  hash()          Make an int out of their name                       */
/* -------------------------------------------------------------------- */
uint hash(char *str)
{
    int  h, shift;

    for (h=shift=0;  *str;  shift=(shift+1)&7, str++)
    {
        h ^= (toupper(*str)) << shift;
    }
    return h;
}
#endif

/* -------------------------------------------------------------------- */
/*  hash()          Make an int out of their name                       */
/* -------------------------------------------------------------------- */
uint hash(char *string)
{
    char *str;
    int  h, shift;

    str = deansi(string);

    for (h=shift=0;  *str;  shift=(shift+1)&7, str++)
    {
        h ^= (toupper(*str)) << shift;
    }
    return h;
}

#ifdef GOODBYE
/* -------------------------------------------------------------------- */
/*  hmemcpy()       Terible hack to copy over 64K, beacuse MSC cant.    */
/* -------------------------------------------------------------------- */
#define K32  (32840L)
void hmemcpy(void huge *xto, void huge *xfrom, long size)
{
    char huge *from;
    char huge *to;

    to = xto; from = xfrom;

    if (to > from)
    {
        h2memcpy(to, from, size);
        return;
    }

    while (size > K32)
    {
        memcpy((char far *)to, (char far *)from, (unsigned int)K32);
        size -= K32;
        to   += K32;
        from += K32;
    }

    if (size)
        memcpy((char far *)to, (char far *)from, (uint)size);
}

/* -------------------------------------------------------------------- */
/*  h2memcpy()      Terible hack to copy over 64K, beacuse MSC cant. PT2*/
/* -------------------------------------------------------------------- */
void h2memcpy(char huge *to, char huge *from, long size)
{
    to += size;
    from += size;

    size++;

    while(size--)
        *to-- = *from--;
}
#endif

/* -------------------------------------------------------------------- */
/*  changedir()     changes curent drive and directory                  */
/* -------------------------------------------------------------------- */
int changedir(char *path)
{
    /* uppercase   */ 
    path[0] = (char)toupper(path[0]);

    /* change disk */
    changedisk(path[0]);

    /* change path */
    if (chdir(path)  == -1) return -1;

    return TRUE;
}

/* -------------------------------------------------------------------- */
/*  changedisk()    change to another drive                             */
/* -------------------------------------------------------------------- */
void changedisk(char disk)
{
    union REGS REG;

    REG.h.ah = 0x0E;      /* select drive */

    REG.h.dl = (uchar)(disk - 'A');

    intdos(&REG, &REG);
}

/* -------------------------------------------------------------------- */
/*  ltoac()         change a long into a number with ','s in it         */
/* -------------------------------------------------------------------- */
char *ltoac(long num)
{
    char s1[30];
    static char s2[40];
    int i, i2, i3, l;

    sprintf(s1, "%lu", num);

    l = strlen(s1);

    for (i = l, i2 = 0, i3 = 0; s1[i2]; i2++, i--)
    {
        if (!(i % 3) && i != l)
        {
            s2[i3++] = ',';
        }
        s2[i3++] = s1[i2];
    }

    s2[i3] = '\0' /*NULL*/;

    return s2;
}

/* -------------------------------------------------------------------- */
/*  editBorder()    edit a boarder line.                                */
/* -------------------------------------------------------------------- */
void editBorder(void)
{
    int i;
    char stuff[100];

    doCR();
    doCR();
        
    if (!cfg.borders)
    {
        mPrintf(" Border lines not enabled!");
        doCR();
        return;
    }

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);
    
    for (i = 0; i < MAXBORDERS; i++)
    {
        mPrintf("2Border %d:0 ", i+1);
        if (*cfg.border[i])
        {
            /* doCR(); */
            mPrintf("%s", cfg.border[i]);
        }
        else
        {
            mPrintf("Empty!"); 
        }
        doCR();
        doCR();
    }

    i = (int)getNumber("border line to change", 0L, (long)MAXBORDERS, 0L);

    if (i)
    {
        doCR();

        mPrintf("2Border %d:0 ", i);

        if (*cfg.border[i-1])
        {
            /* doCR(); */
            mPrintf("%s", cfg.border[i-1]);
        }
        else
        {
            mPrintf("Empty!"); 
        }


        doCR();  
        getString("border line", stuff /* cfg.border[i-1] */, 80, FALSE, ECHO, "");

        if (*stuff)
        {
            strcpy(cfg.border[i-1], stuff);
            normalizeString(cfg.border[i-1]);
        }
    }
}

/* -------------------------------------------------------------------- */
/*  doBorder()      print a boarder line.                               */
/* -------------------------------------------------------------------- */
void doBorder(void)
{
    static count = 0;
    static line  = 0;
    int    tries;

    if (count++ == 20)
    {
        count = 0;

        for (line == MAXBORDERS-1 ? line = 0 : line++, tries = 0; 
             tries < MAXBORDERS + 1;
             line == MAXBORDERS-1 ? line = 0 : line++, tries++)
        {
            if (*cfg.border[line])
            {
                doCR();
                mPrintf("%s", cfg.border[line]);
                break;
            }
        }
    }
}

#ifdef GOODBYE

#define ADDITION            (1)
#define CHANGE              (3)
#define DELETION            (5)
#define COMP_LEN            (30)

#define ARR_SIZE            (COMP_LEN+1)
#define SMALLEST_OF(x,y,z)  ( (x<y) ? min(x,z) : min(y,z) )
#define ZERO_IF_EQUAL(x,y)  (tolower(requested[x-1]) == tolower(found[y-1]) ? 0 : CHANGE)

int     l_distance(char *requested, char *found)
{
    register int i,j;
    int r_len, f_len;
    int distance[ARR_SIZE][ARR_SIZE];
    
    /*
     * First character does not match 
     */
    if (tolower(requested[0]) != tolower(found[0]))
        return 1000;
    
    r_len = min(strlen(requested),COMP_LEN);
    f_len = min(strlen(found),    COMP_LEN);

    /*
     * The lengths are too diffrent
     */
    if ( abs(r_len - f_len) > (r_len/3))
        return 1001;
    
    distance[0][0] = 0;
    
    for (j = 1; j <= ARR_SIZE; j++)
        distance[0][j] = distance[0][j-1] + ADDITION;
    for (j = 1; j <= ARR_SIZE; j++)
        distance[j][0] = distance[j-1][0] + DELETION;
        
    for (i=1; i<=r_len; i++)    
        for (j=1; j<=f_len; j++)    
            distance[i][j] = SMALLEST_OF
                             (
                                (distance[i-1][j-1] + ZERO_IF_EQUAL(i,j)),
                                (distance[i  ][j-1] + ADDITION),
                                (distance[i-1][j  ] + DELETION)
                             );
    
    return (distance[r_len][f_len]);
}

#endif


#ifdef NEWMSGTAB

char int_LO(int big_number)
{
    union
    {
        struct 
        {
            char char_LO;
            char char_HI;
        } breakdown;

        int char_HI_LO;
    } int_number;
        
    int_number.char_HI_LO = big_number;

    return int_number.breakdown.char_LO;
}

char int_HI(int big_number)
{
    union
    {
        struct 
        {
            char char_LO;
            char char_HI;
        } breakdown;

        int char_HI_LO;
    } int_number;
        
    int_number.char_HI_LO = big_number;

    return int_number.breakdown.char_HI;
}


int int_JOIN(char lo_number, char hi_number)
{
    union
    {
        struct 
        {
            char char_LO;
            char char_HI;
        } breakdown;

        int char_HI_LO;
    } int_number;
        
    int_number.breakdown.char_LO = lo_number;
    int_number.breakdown.char_HI = hi_number;

    return int_number.char_HI_LO;
}
#endif


int long_LO(long big_number)
{
    union
    {
        struct 
        {
            int int_LO;
            int int_HI;
        } breakdown;

        long int_HI_LO;
    } long_number;
        
    long_number.int_HI_LO = big_number;

    return long_number.breakdown.int_LO;
}

int long_HI(long big_number)
{
    union
    {
        struct 
        {
            int int_LO;
            int int_HI;
        } breakdown;

        long int_HI_LO;
    } long_number;
        
    long_number.int_HI_LO = big_number;

    return long_number.breakdown.int_HI;
}

long long_JOIN(int lo_number, int hi_number)
{
    union
    {
        struct 
        {
            int int_LO;
            int int_HI;
        } breakdown;

        long int_HI_LO;
    } long_number;
        
    long_number.breakdown.int_LO = lo_number;
    long_number.breakdown.int_HI = hi_number;

    return long_number.int_HI_LO;
}




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

