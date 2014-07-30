/* -------------------------------------------------------------------- */
/*  FORMAT.C                 Dragon Citadel                             */
/* -------------------------------------------------------------------- */
/*  Contains string handling stuff                                      */
/* -------------------------------------------------------------------- */

#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              Contents                                */
/* -------------------------------------------------------------------- */
/*  sformat()       Configurable format                                 */
/*  normalizeString() deletes leading & trailing blanks etc.            */
/*  qtext()         Consumes quoted strings and expands escape chars    */
/*  strpos()        find a character in a string                        */
/*  substr()        is string 1 in string 2?                            */
/*  u_match()       Unix wildcarding                                    */
/*  cclass()        Used with u_match()                                 */
/*  deansi()        Removes Ctrl-A codes from strings                   */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  HISTORY:                                                            */
/*                                                                      */
/*  05/26/89    (PAT)   Cleanup, history created                        */
/*                                                                      */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*  External data                                                       */
/* -------------------------------------------------------------------- */

#include "stdio.h"


/* -------------------------------------------------------------------- */
/*  sformat()       Configurable format                                 */
/* -------------------------------------------------------------------- */
/*
 *  sformat, a 10 minute project
 *  by Peter Torkelson
 *
 *    passes a number of arguments this lets you make configurable things..
 *    call it with:
 *      str    a string buffer
 *      fmt    the format string
 *      val    valid % escapes
 *    here is an example:
 *      sformat(str, "Hello %n, I am %w. (%%, %c)", "nw", "you", "me");
 *    gives you:
 *      "Hello you, I am me. (%, (NULL))"
 *
 */ 
void sformat(char *str, char *fmt, char *val, ... )
{
    int i;

    char s[2];
    va_list ap;

    s[1] = 0;
    *str = 0;

    while(*fmt)
    {
        if (*fmt != '%')
        {
            *s = *fmt;
            strcat(str, s);
        }
        else
        {
            fmt++;
            if (*fmt == '\0')     /*  "somthing %", not nice */
                return;  
            if (*fmt == '%')      /*  %% = %                 */
            {
                *s = *fmt;
                strcat(str, s);
            }
            else                 /*  it must be a % something */
            {                   
                i = strpos(*fmt, val) - 1;
                if (i != -1)
                {
                    va_start(ap, val);
                    while(i--)
                        (void)va_arg(ap, char *);

                    strcat(str, va_arg(ap, char *));
                    va_end(ap);
                }
                else
                {
                    strcat(str, "(NULL)");
                }
    
            } /* fmt == '%' */
    
        } /* first fmt == '%' */
    
        fmt++;

    } /* end while */  
}

/* -------------------------------------------------------------------- */
/*  normalizeString() deletes leading & trailing blanks etc.            */
/* -------------------------------------------------------------------- */
void normalizeString(char *s)
{
    char *pc;

    pc = s;

    if (!cfg.colors)
    {
        /* find end of string   */
        while (*pc)  
        {
            if (*pc < ' ')   *pc = ' ';   /* zap tabs etc... */
            pc++;
        }
    }
    else
    {
        /* find end of string   */
        while (*pc) 
        {
            if (*pc < ' ' && (*pc != 1))    /* less then space & not ^A */
                *pc = ' ';      /* zap tabs etc... */
            pc++;
        }
    }

    /* no trailing spaces: */
    while (pc>s  &&  isspace(*(pc-1))) pc--;
    *pc = '\0';

    /* no leading spaces: */
    while (*s == ' ')
    {
        for (pc=s;  *pc;  pc++)    *pc = *(pc+1);
    }

    /* no double blanks */
    for (; *s;)  
    {
        if (*s == ' ' &&  *(s+1) == ' ')
        {
            for (pc=s;  *pc;  pc++)  *pc = *(pc+1);
        }
        else s++;
    }
}

/* -------------------------------------------------------------------- */
/*  parse_it()      routines to parse strings separated by white space  */
/* -------------------------------------------------------------------- */
/*                                                                      */
/* strategy:  called as                                                 */
/*            count = parse_it(workspace,input);                        */
/*                                                                      */
/* where workspace is a two-d char array of the form:                   */
/*                                                                      */
/* char *workspace[MAXWORD];                                            */
/*                                                                      */
/* and input is the input string to be parsed.  it returns              */
/* the actual number of things parsed.                                  */
/*                                                                      */
/* -------------------------------------------------------------------- */
int parse_it(char *words[], char input[])
{
/* states ofmachine... */
#define INWORD          0
#define OUTWORD         1
#define INQUOTES        2

/* characters */
#undef  TAB
#define TAB             9
#define BLANK           ' '
#define QUOTE           '\"'
#define QUOTE2          '\''
#define MXWORD         128

    int  i,state,thisword;

    input[strlen(input)+1] = 0;         /* double-null */

    for (state = OUTWORD, thisword = i = 0; input[i]; i++) 
    {
        switch (state) 
        {
            case INWORD:
                if (isspace(input[i])) 
                {
                    input[i] = 0;
                    state = OUTWORD;
                }
                break;
            case OUTWORD:
                if (input[i] == QUOTE || input[i] == QUOTE2) 
                {
                    state = INQUOTES;
                } else if (!isspace(input[i])) 
                {
                    state = INWORD;
                }

                /* if we are now in a string, setup, otherwise, break */

                if (state != OUTWORD) 
                {
                    if (thisword >= MXWORD)
                    {
                        return thisword;
                    }

                    if (state == INWORD) 
                    {
                        words[thisword++] = (input + i);
                    } else {
                        words[thisword++] = (input + i + 1);
                    }
                }
                break;
            case INQUOTES:
                i += qtext(input + i, input + i,input[i - 1]);
                state = OUTWORD;
                break;
        }
    }
    return thisword;
}

/* -------------------------------------------------------------------- */
/*  qtext()         Consumes quoted strings and expands escape chars    */
/* -------------------------------------------------------------------- */
int  qtext(char *buf, char *line, char end)
{
    int index = 0;
    int slash = 0;
    char chr;

    while (line[index] != '\0' && (line[index] != end || slash != 0)) 
    {
        if (slash == 0) 
        {
            if (line[index] == '\\') 
            {
                slash = 1;
            } 
            else 
            if (line[index] == '^') 
            {
                slash = 2;
            }
            else 
            {
                *(buf++) = line[index];
            }
        } 
        else 
        if (slash == 1) 
        {
            switch (line[index]) 
            {
                default:
                    *(buf++) = line[index];
                    break;
                case 'n':                       /* newline */
                    *(buf++) = '\n';
                    break;
                case 't':                       /* tab */
                    *(buf++) = '\t';
                    break;
                case 'r':                       /* carriage return */
                    *(buf++) = '\r';
                    break;
                case 'f':                       /* formfeed */
                    *(buf++) = '\f';
                    break;
                case 'b':                       /* backspace */
                    *(buf++) = '\b';
                    break;
            }
            slash = 0;
        } else /* if (slash == 2 ) */
        {
            if (line[index] == '?') 
            {
                chr = 127;
            } 
            else 
            if (line[index] >= 'A' && line[index] <= 'Z') 
            {
                chr = (char)(line[index] - 'A' + 1);
            }
            else 
            if (line[index] >= 'a' && line[index] <= 'z') 
            {
                chr = (char)(line[index] - 'a' + 1);
            }
            else 
            {
                chr = line[index];
            }

            *(buf++) = chr;
            slash = 0;
        }

        index++;
    }

    *buf = 0;
    return line[index] == end ? index + 1 : index;
}

/* -------------------------------------------------------------------- */
/*  strpos()        find a character in a string                        */
/* -------------------------------------------------------------------- */
strpos(char ch, char *str)
{
    int i;
  
    for(i=0; i<(int)strlen(str); i++)
        if (ch == str[i])
            return (i+1);

    return 0;
}

/* -------------------------------------------------------------------- */
/*  substr()        is string 2 in string 1?                            */
/* -------------------------------------------------------------------- */
int substr(char *str1, char *str2)
{
    label tmp;

    strcpy(tmp, str1);
    strlwr(tmp);
    strlwr(str2);

    if(strstr(tmp, str2) == NULL)
        return FALSE;
    else
        return TRUE;
}

/* -------------------------------------------------------------------- */
/*  old  substr()        is string 1 in string 2?                       */
/* -------------------------------------------------------------------- */
/*
int substr(char *str1, char *str2)
{
    label tmp;
    int i;
    
    if (!strlen(str2) || !strlen(str1) || (strlen(str1) < strlen(str2)))
        return FALSE;

    for (i=0; i <= strlen(str1) - strlen(str2); i++)
    {
        strcpy(tmp, str1 + i);
        tmp[strlen(str2)] = '\0';
        if (strcmpi(tmp, str2) == SAMESTRING)
            return TRUE;
    }
    return FALSE;
}
*/

/* -------------------------------------------------------------------- */
/*  u_match()       Unix wildcarding                                    */
/* -------------------------------------------------------------------- */
/*
 * int u_match(string, pattern)
 * char *string, *pattern;
 *
 * Match a pattern as in sh(1).
 */

#define	CMASK	0377
#undef  QUOTE
#define QUOTE   0200
#define	QMASK	(CMASK&~QUOTE)
#define	NOT	'!'	/* might use ^ */

static  char    *cclass(register char *p, register int sub);

int u_match(register char *s, register char *p)
{
	register int sc, pc;

	if (s == NULL || p == NULL)
		return(0);
	while ((pc = *p++ & CMASK) != '\0') {
		sc = *s++ & QMASK;
		switch (pc) {
		case '[':
			if ((p = cclass(p, sc)) == NULL)
				return(0);
			break;

		case '?':
			if (sc == 0)
				return(0);
			break;

		case '*':
			s--;
			do {
                if (*p == '\0' || u_match(s, p))
					return(1);
			} while (*s++ != '\0');
			return(0);

		default:
            if (tolower(sc) != (tolower(pc&~QUOTE)))
				return(0);
		}
	}
	return(*s == 0);
}

/* -------------------------------------------------------------------- */
/*  cclass()        Used with u_match()                                 */
/* -------------------------------------------------------------------- */
static char *cclass(register char *p, register int sub)
{
	register int c, d, not, found;

    sub = tolower(sub);

    if ((not = *p == NOT) != 0)
		p++;
	found = not;
	do {
		if (*p == '\0')
			return(NULL);
        c = tolower(*p & CMASK);
		if (p[1] == '-' && p[2] != ']') {
            d = tolower(p[2] & CMASK);
			p++;
		} else
			d = c;
		if (c == sub || c <= sub && sub <= d)
			found = !not;
	} while (*++p != ']');
	return(found? p+1: NULL);
}

/* -------------------------------------------------------------------- */
/*  parseNetAddress()   Parse a net address from user input..           */
/* -------------------------------------------------------------------- */
/*
 Varible guide: (I am lazy..)
 
 str    input string
 u      user name
 n      node name
 r      region name
 c      country name
*/

void parseNetAddress(char *str, char *u, char *n, char *r, char *c)
{
    char *s1, *s2;
    
    *u = 0;
    *n = 0;
    *r = 0;
    *c = 0;      /* null out all strings.. */
    
    s2 = str;
    
    /*
     * User name..
     *
     * Find the first '@', 
     * NULL it to tie off the string, 
     * normalize that part of the string, 
     * and copy <= LABELSIZE of characters into the user.. 
     *
     * This is done for the others in the same basic way.
     */
    s1 = strchr(s2, '@');
    
    if (s1)
    {
        *s1 = 0;
        s1++;
    }
    
    normalizeString(s2);
    strncpy(u, s2, LABELSIZE);
    n[LABELSIZE] = 0;
    
    s2 = s1;
    
    if (!s2)
        return;
    
    /* 
     * node..
     */
    s1 = strchr(s2, ',');
    
    if (s1)
    {
        *s1 = 0;
        s1++;
    }
    
    normalizeString(s2);
    strncpy(n, s2, LABELSIZE);
    n[LABELSIZE] = 0;

    s2 = s1;
    
    if (!s2)
        return;
        
    /* 
     * region..
     */
    s1 = strchr(s2, ',');
    
    if (s1)
    {
        *s1 = 0;
        s1++;
    }
    
    normalizeString(s2);
    strncpy(r, s2, LABELSIZE);
    n[LABELSIZE] = 0;

    s2 = s1;
    
    if (!s2)
        return;
        
    /* 
     * country..
     */
    
    normalizeString(s2);
    strncpy(c, s2, LABELSIZE);
    n[LABELSIZE] = 0;

    s2 = s1;
}


#undef strcmp
#undef stricmp
#undef strcmpi

int deansi_str_cmp(char *string1, char *string2)
{
    char buf1[129];
    char buf2[129];

    if ((strlen(string1) > 128) || (strlen(string2) > 128) )
        cPrintf("CMP_ERROR\n");

    strcpy(buf1, string1);
    strcpy(buf2, string2);

    stripansi(buf1);
    stripansi(buf2);

    return(strcmp(buf1, buf2));

}

int deansi_str_cmpi(char *string1, char *string2)
{
    char buf1[129];
    char buf2[129];

    if ((strlen(string1) > 128) || (strlen(string2) > 128) )
        cPrintf("CMPI_ERROR\n");

    strcpy(buf1, string1);
    strcpy(buf2, string2);

    stripansi(buf1);
    stripansi(buf2);

    return(stricmp(buf1, buf2));
}

char *deansi(char *str)
{

    static char buf[31];
    int i=0;
    char *pc;

    if (strlen(str) > NAMESIZE)
        cPrintf("DEANSI_ERR\n");

    pc = str;

    while (*pc)  
    {
        if (*pc == 1)   pc += 2;   /* Zap Ctrl-A plus following Char */
        else
        {
            buf[i] = *pc;
            i++;
            pc++;
        }
    }
    buf[i] = 0;


    return(buf);
}    

/* changes string being sent, does not return a pointer like deansi */
void stripansi(char *str)
{

    int i=0;


    char *pc;
    pc = str;

    while (*pc)  
    {
        if (*pc == 1)   pc += 2;   /* Zap Ctrl-A plus following Char */
        else
        {
            str[i] = *pc;
            i++;
            pc++;
        }
    }
    str[i] = 0;
}    


