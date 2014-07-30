/************************************************************************/
/*                              Files.c                                 */
/*                  File handling routines for ctdl                     */
/************************************************************************/

#include "ctdl.h"

/************************************************************************/
/*                              contents                                */
/*                                                                      */
/*      ambig()                 returns true if filename is ambiguous   */
/*      ambigUnlink()           unlinks ambiguous filenames             */
/*      attributes()            aide fn to set file attributes          */
/*      blocks()                displays how many blocks file is        */
/*      bytesfree()             returns #bytes free on current drive    */
/*      checkfilename()         returns ERROR on illegal filenames      */
/*      checkup()               returns TRUE if filename can be uploaded*/
/*      dir()                   very high level, displays directory     */
/*      dltime()                computes dl time from size & global rate*/
/*      entertextfile()         menu level .et                          */
/*      enterwc()               menu level .ew file                     */
/*      entrycopy()             readable struct -> directory array      */
/*      entrymake()             dos transfer struct -> readable struct  */
/*      filexists()             returns TRUE if a specified file exists */
/*      filldirectory()         fills our directory structure           */
/*      getattr()               returns a file attribute                */
/*      getfirst()              low level, read first item of directory */
/*      getnext()               low level, read next item of directory  */
/*      hide()                  hides a file. for limited-access u-load */
/*      readdirectory()         menu level .rd .rvd routine             */
/*      readinfofile()          menu level .ri .rvi routine             */
/*      readtextfile()          menu level .rt routine                  */
/*      readwc()                menu level .rw file                     */
/*      renamefile()            aide fn to rename a file                */
/*      setattr()               sets file attributes                    */
/*      strlwr()                makes any string lower case             */
/*      textdown()              does wildcarded unformatted file dumps  */
/*      textup()                handles actual text upload              */
/*      unlinkfile()            handles the .au command                 */
/*      wcdown()                calls xmodem downloading routines       */
/*      wcup()                  calls xmodem uploading routines         */
/************************************************************************/

/* our readable transfer structure */
static struct
{
    char filename[13];
    unsigned char attribute;
    char date[9];
    char time[9];  
    long size;
} directory;


/************************************************************************/
/*      ambig() returns TRUE if string is an ambiguous filename         */
/************************************************************************/
ambig(char *filename)
/* char *filename; */
{
    int i;

    for (i = 0; i < (int)strlen(filename); ++i)
    {
        if ( (filename[i] == '*') || (filename[i] == '?') )
       return(TRUE);
    }
    return(FALSE);
}

/************************************************************************/
/*      ambigUnlink() unlinks ambiguous filenames                       */
/************************************************************************/
int ambigUnlink(char *filename, char change)
/* char *filename; */
/* char change;    */
{
    char file[15];
    int i=0;

    if(change)
        if (changedir(roomBuf.rbdirname) == -1)
            return(0);

    verbose = TRUE;
    filldirectory(filename);

    while(filedir[i].entry[0])
    {
        filedir[i].entry[13] = ' ';
        sscanf(filedir[i].entry, "%s ", file);
        if(file[0])
            unlink(file);
        i++;
    }

    /* free file directory structure */
    if(filedir != NULL)
    {
        _ffree((void *)filedir);
    }

    return(i);
}

/************************************************************************/
/*      attributes() aide fn to set file attributes                     */
/************************************************************************/
void attributes(void)
{
    label filename;
    char hidden = 0, readonly = 0;
    unsigned char attr, getattr();

    doCR();
    getNormStr("filename", filename, NAMESIZE, ECHO);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if ( (checkfilename(filename, 0) == ERROR) 
    || ambig(filename))
    {
        mPrintf("\n Invalid filename.");
        changedir(cfg.homepath);
        return;
    }

    if (!filexists(filename))
    {
        mPrintf(" File not found\n"); 
        changedir(cfg.homepath);
        return;
    }

    attr = getattr(filename);

    readonly = (uchar)(attr & 1);
    hidden   = (uchar)( (attr & 2) == 2);

    /* set readonly and hidden bits to zero */
    attr = (attr ^ readonly);
    attr = (uchar)(attr ^ (hidden * 2));

    readonly = (char)getYesNo("Read only", readonly);
    hidden   = (char)getYesNo("Hidden",    hidden);

    /* set readonly and hidden bits */
    attr = (attr   | readonly);
    attr = (uchar)(attr   | (hidden * 2));

    setattr(filename, attr);

    sprintf(msgBuf->mbtext,
    "Attributes of file %s changed in %s] by %s",
    filename,
    roomBuf.rbname,
    logBuf.lbname );

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    changedir(cfg.homepath);
}

/************************************************************************/
/*      blocks()  displays how many blocks file is upon download        */
/************************************************************************/
void blocks(char *filename, int bsize)
{
    FILE *stream;
    long length;
    int blocks;

    double dltime();

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    stream = fopen(filename, "r");

    length = filelength(fileno(stream));

    fclose(stream);
 
    if (length == -1l) return;

    if (bsize)
      blocks = ((int)(length/(long)bsize) + 1);
    else
      blocks = 0;

    doCR();

    if (!bsize)
      mPrintf("File Size: %ld %s",
        length, (length == 1l) ? "byte" : "bytes" );
    else
      mPrintf("File Size: %d %s, %ld %s",
        blocks, (blocks == 1) ? "block" : "blocks",
        length, (length == 1l)? "byte" : "bytes" );

    doCR();
    mPrintf("Transfer Time: %.0f minutes", dltime(length));
    doCR();
}

/************************************************************************/
/*      bytesfree() returns # bytes free on drive                       */
/************************************************************************/
long bytesfree()
{
    char path[64];
    long bytes;
    union REGS REG;

    getcwd(path, 64);

    REG.h.ah = 0x36;      /* select drive */

    REG.h.dl = (uchar)(path[0] - '@');

    intdos(&REG, &REG);

    bytes = (long)( (long)REG.x.cx * (long)REG.x.ax * (long)REG.x.bx); 

    return(bytes);
}

/************************************************************************/
/*      checkfilename() checks a filename for illegal characters        */
/************************************************************************/
int checkfilename(char *filename, char xtype)
{
    char *s;
    char device[20];
    FILE *fl;
    
    if (strpbrk(filename, 
                extrn[xtype-1].ex_batch ? 
                " '\"/\\[]:|<>+=;," : "'\"/\\[]:|<>+=;,") != NULL)
    {
        return (ERROR);
    }
    
    strncpy(device, filename, 13);
    device[13] = '\0';
    if ((s = strchr(device, '.')) != NULL)
    {
        *s = '\0';
    }

    if ((fl = fopen(filename, "rb")) == NULL)
    {
        return(TRUE);
    }
    
    if (isatty(fileno(fl)))
    {
        return(ERROR);
    }

    fclose(fl);
    
    return(TRUE);
}

/***********************************************************************/
/*      checkup()  returns TRUE if filename can be uploaded            */
/***********************************************************************/
checkup(char *filename)
/* char *filename; */
{
    if (ambig(filename) )  return(ERROR);

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR)
    {
        mPrintf("\n Invalid filename.");
        return(ERROR);
    }

    if (changedir(roomBuf.rbdirname) == -1 )  return(ERROR);

    if (filexists(filename))
    {
        mPrintf("\n File exists."); 
        changedir(cfg.homepath);
        return(ERROR);
    }
    return(TRUE);
}

int revstrcmp(void *a, void *b);

int revstrcmp(void *a, void *b)
{
    int result;

    result = strcmp(a,b);
    if (result < 0) return 1;
    if (result > 0) return -1;
    return 0;
}

#ifdef GOODBYE
/************************************************************************/
/*     filldirectory()  this routine will fill the directory structure  */
/*     according to wildcard                                            */
/************************************************************************/
void filldirectory(char *filename)
{
    int i, ax;
    struct ffblk  file_buf;
    int filetypes;
    int strip;

    /* allocate the first record of the file dir structure */
    filedir = farcalloc((long)(sizeof(*filedir)), cfg.maxfiles);

    /* return on error allocating */
    if(filedir == NULL)
    {
        cPrintf("Failed to allocate FILEDIR\n");
        return;
    }    

    filetypes = /*FA_NORMAL |*/ (aide ? FA_HIDDEN : 0);

    /* keep going till it errors, which is end of directory */
    for ( ax = findfirst("*.*", &file_buf, filetypes) , i = 0;
          ax == 0;
          ax = findnext(&file_buf), ++i )
    {
        /* Only cfg.maxfiles # of files files */
        if (i == cfg.maxfiles) break;

        /* translate dos's structure to something we can read */
        entrymake(&file_buf);

        if (!strpos('.', directory.filename))
        {
            strcat(directory.filename, ".");
            strip = TRUE;
        }else{
            strip = FALSE;
        }

        /* copy "directory" to "filedir" */
        /* NO zero length filenames */

        if ( (!(directory.attribute & 16))
            /* either aide or not a hidden file */
            /*&& (aide || !(directory.attribute & 2) ) */

            /* filename match wildcard? */
            && ( u_match(directory.filename, filename) )

            /* never the volume name either */
            && !(directory.attribute & 8) 

            /* never display fileinfo.cit */              
            && (strcmpi( directory.filename, "fileinfo.cit") != SAMESTRING) )

            /* if passed, put into structure, else loop again */
        {
            if (strip)
                directory.filename[strlen(directory.filename)-1] = '\0';
            entrycopy(i);
        }else{ 
            i--;
        }

    }
    filedir[i].entry[0] = '\0';  /* tie it off with a null */

    /* alphabetical order */
    qsort(
          filedir,
          i, 
          90,
          (QSORT_CMP_FNP)strcmp);
}
#endif



/************************************************************************/
/*     filldirectory()  this routine will fill the directory structure  */
/*     according to wildcard                                            */
/************************************************************************/
void filldirectory(char *filename)
{
    int i, ax;
    struct find_t  file_buf;
    int filetypes;
    int strip;

    /* allocate the first record of the file dir structure */
    filedir = _fcalloc(sizeof(*filedir), cfg.maxfiles);

    /* return on error allocating */
    if(filedir == NULL)
    {
        cPrintf("Failed to allocate FILEDIR\n");
        return;
    }    

    filetypes = /*FA_NORMAL |*/ (aide ? _A_HIDDEN : 0);

    /* keep going till it errors, which is end of directory */
    for ( ax = _dos_findfirst("*.*", filetypes, &file_buf) , i = 0;
          ax == 0;
          ax = _dos_findnext(&file_buf), ++i )
    {
        /* Only cfg.maxfiles # of files files */
        if (i == cfg.maxfiles) break;

        /* translate dos's structure to something we can read */
        entrymake(&file_buf);

        if (!strpos('.', directory.filename))
        {
            strcat(directory.filename, ".");
            strip = TRUE;
        }else{
            strip = FALSE;
        }

        /* copy "directory" to "filedir" */
        /* NO zero length filenames */

        if ( (!(directory.attribute & 16))
            /* either aide or not a hidden file */
            /*&& (aide || !(directory.attribute & 2) ) */

            /* filename match wildcard? */
            && ( u_match(directory.filename, filename) )

            /* never the volume name either */
            && !(directory.attribute & 8) 

            /* never display fileinfo.cit */              
            && (strcmpi( directory.filename, "fileinfo.cit") != SAMESTRING) )

            /* if passed, put into structure, else loop again */
        {
            if (strip)
                directory.filename[strlen(directory.filename)-1] = '\0';
            entrycopy(i);
        }else{ 
            i--;
        }

    }
    filedir[i].entry[0] = '\0';  /* tie it off with a null */

    /* alphabetical order */
    qsort(filedir, i, 90, (reverse) ? (QSORT_CMP_FNP)revstrcmp : (QSORT_CMP_FNP)strcmp);
}

/***********************************************************************/
/*     dir() highest level file directory display function             */
/***********************************************************************/
void dir(char *filename)
{
    int i;
    long bytesfree();
    char oldverbose;

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    /* no bad files */
/*  if (checkfilename(filename, 0) == ERROR)
    {
        doCR();
        mPrintf(" No file %s", filename);
        return;
    } */

    changedir(roomBuf.rbdirname);

    /* load our directory structure according to filename */
    oldverbose = verbose;
    verbose = TRUE;
    filldirectory(filename);
    verbose = oldverbose;

    if (filedir[0].entry[0])
    {
        if (verbose)  mPrintf("\n 3Filename     Date      Size   D/L Time0"); 
        else          doCR();
    }

    /* check for matches */
    if ( !filedir[0].entry[0]) 
    {
        doCR();
        mPrintf(" No file %s", filename);
    }
    for (i = 0;
         ( filedir[i].entry[0] && (outFlag != OUTSKIP) && !mAbort(FALSE) );
         ++i)
    {    
        if(verbose)
        {
            filedir[i].entry[0] = filedir[i].entry[13];
            filedir[i].entry[13] = ' ';
            filedir[i].entry[40] = '\0'; /* cut timestamp off */
            doCR();
            mPrintf(filedir[i].entry);
        }
        /* display filename */
        else
        {
            filedir[i].entry[13] = ' ';
            filedir[i].entry[14] = '\0';
            mPrintf(filedir[i].entry+1);
        }
    }

    if (verbose && outFlag != OUTSKIP )
    {
        doCR();
        mPrintf("        %d %s    %ld bytes free", i, 
            (i == 1) ? "File" : "Files", bytesfree());
    }

    /* free file directory structure */
    if(filedir != NULL)
    {
        _ffree((void *)filedir);
    }

    /* go to our home-path */
    changedir(cfg.homepath);
}


/***********************************************************************/
/*    dltime()  give this routine the size of your file and it will    */
/*  return the amount of time it will take to download according to    */
/*  speed                                                              */
/***********************************************************************/
double dltime(long size)
{
    double time;
    /* could be more accurate */
    static long fudge_factor[] = 
        { 1800L, 7200L, 14400L, 28800L, 57600L, 115200L, 115200L};
            
    time = (double)size / (double)(fudge_factor[speed]);

    return(time);
}

/************************************************************************/
/*   entertextfile()  menu level .et                                    */
/************************************************************************/
void entertextfile(void)
{
    label filename;
    char comments[64];

    doCR();
    getNormStr("filename", filename, NAMESIZE, ECHO);

    if (checkup(filename) == ERROR)  return;

    getString("comments", comments, 64, FALSE, TRUE, "");
             
    if (strlen(filename))  textup(filename);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if (filexists(filename))
    {
        entercomment(filename, logBuf.lbname, comments);
        if (!comments[0])
          sprintf(msgBuf->mbtext, " %s uploaded by %s", filename, logBuf.lbname);
        else
          sprintf(msgBuf->mbtext, " %s uploaded by %s\n Comments: %s", 
            filename, logBuf.lbname, comments);
        specialMessage();
    }

    changedir(cfg.homepath);
}

/************************************************************************/
/*      enterwc()  menu level .ew  HIGH level routine                   */
/************************************************************************/
void enterwc(void)
{
    label filename;
    char comments[64];

    doCR();
    getNormStr("filename", filename, NAMESIZE, ECHO);

    if (checkup(filename) == ERROR)  return;
             
    getString("comments", comments, 64, FALSE, TRUE, "");

    if (strlen(filename))  wcup(filename);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if (filexists(filename))
    {
        entercomment(filename, logBuf.lbname, comments);
        if (!comments[0])
          sprintf(msgBuf->mbtext, " %s uploaded by %s", filename, logBuf.lbname);
        else
          sprintf(msgBuf->mbtext, " %s uploaded by %s\n Comments: %s", 
            filename, logBuf.lbname, comments);
        specialMessage();
    }

    changedir(cfg.homepath);
}

/************************************************************************/
/*   entrycopy()                                                        */
/*   This routine copies the single readable "directory" array to the   */
/*   to the specified element of the "dir" array according to verbose.  */
/************************************************************************/
void entrycopy(int element)
{
    double dltime();

    if (verbose)
    {
        sprintf( filedir[element].entry, " %-12s %s %7ld %9.2f %s " ,    
        directory.filename,
        directory.date,
        directory.size,
        dltime(directory.size),
        directory.time
    );

        if  ((directory.attribute & 2))
        filedir[element].entry[13] = '*';

    }
    else sprintf( filedir[element].entry, "%-12s ", directory.filename); 
}
#ifdef GOODBYE
/************************************************************************/
/*   entrymake()                                                        */
/*   This routine converts one filename from the entry structure to the */
/*   "directory" structure.                                             */
/************************************************************************/
void entrymake(struct ffblk *file_buf)
{
    char string[10];

    /* copy filename   */
    strcpy( directory.filename, file_buf->ff_name);
    strlower( directory.filename);  /* make it lower case */

    /* copy attribute  */    
    directory.attribute = file_buf->ff_attrib;

    /* copy date       */
    getdstamp(string, file_buf->ff_fdate);
    strcpy(directory.date, string);

    /* copy time       */
    gettstamp(string, file_buf->ff_ftime);
    strcpy(directory.time, string);

    /* copy filesize   */
    directory.size = file_buf->ff_fsize;
}
#endif

/************************************************************************/
/*   entrymake()                                                        */
/*   This routine converts one filename from the entry structure to the */
/*   "directory" structure.                                             */
/************************************************************************/
void entrymake(struct find_t *file_buf)
{
    char string[10];

    /* copy filename   */
    strcpy( directory.filename, file_buf->name);
    strlower( directory.filename);  /* make it lower case */

    /* copy attribute  */    
    directory.attribute = file_buf->attrib;

    /* copy date       */
    getdstamp(string, file_buf->wr_date);
    strcpy(directory.date, string);

    /* copy time       */
    gettstamp(string, file_buf->wr_time);
    strcpy(directory.time, string);

    /* copy filesize   */
    directory.size = file_buf->size;
}

/************************************************************************/
/*      getattr() returns file attribute                                */
/************************************************************************/
unsigned char getattr(char far *filename)
/* char far *filename; */
{
    union REGS inregs;
    union REGS outregs;

    inregs.x.dx = FP_OFF(filename);
    inregs.h.ah = 0x43;      /* CHMOD */
    inregs.h.al = 0;         /* GETATTR */

    intdos(&inregs, &outregs);

    return((uchar)outregs.x.cx);
}

/************************************************************************/
/*      hide()  hides a file. for limited-access u-load                 */
/************************************************************************/
void hide(char *filename)
{
    unsigned char attr, getattr();

    attr = getattr(filename);

    /* set hidden bit on */
    attr = (uchar)(attr | 2);

    setattr(filename, attr);
}

/************************************************************************/
/*      readdirectory()  menu level .rd .rvd routine  HIGH level routine*/
/************************************************************************/
void readdirectory(void)
{
    label filename;

    getNormStr("", filename, NAMESIZE, ECHO);
             
    if (strlen(filename))  dir(filename);
    else                   dir("*.*");
}

/************************************************************************/
/*      readtextfile()  menu level .rt  HIGH level routine              */
/************************************************************************/
void readtextfile(void)
{
    label filename;

    doCR();
    getNormStr("filename", filename, NAMESIZE, ECHO);
             
    if (strlen(filename))  textdown(filename);
}

/************************************************************************/
/*      readwc()  menu level .rw  HIGH level routine                    */
/************************************************************************/
void readwc(void)
{
    label filename;

    doCR();
    getNormStr("filename", filename, NAMESIZE, ECHO);
             
    if (strlen(filename))  wcdown(filename);
}

/************************************************************************/
/*      renamefile()  aide fn to rename a file                          */
/************************************************************************/
void renamefile(void)
{      
    char source[20], destination[20];

    doCR();
    getNormStr("source filename",      source, NAMESIZE, ECHO);
    if (!strlen(source)) return;

    getNormStr("destination filename", destination, NAMESIZE, ECHO);
    if (!strlen(destination)) return;

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if ( (checkfilename(source, 0) == ERROR)   
    || (checkfilename(destination, 0) == ERROR)
    || ambig(source) || ambig(destination) )
    {
        mPrintf("\n Invalid filename.");
        changedir(cfg.homepath);
        return;
    }

    if (!filexists(source))
    {
        mPrintf(" No file %s", source); 
        changedir(cfg.homepath);
        return;
    }

    if (filexists(destination))
    {
        mPrintf("\n File exists."); 
        changedir(cfg.homepath);
        return;
    }

    /* if successful */
    if ( rename(source, destination) == 0)
    {
        sprintf(msgBuf->mbtext,
        "File %s renamed to file %s in %s] by %s",
        source, destination, 
        roomBuf.rbname,
        logBuf.lbname );

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();
    }
    else mPrintf("\n Cannot rename %s\n", source);

    changedir(cfg.homepath);
}

/************************************************************************/
/*      setattr() sets file attributes                                  */
/************************************************************************/
void setattr(char far *filename, unsigned char attr)
{
    union REGS inregs;
    union REGS outregs;

    inregs.x.dx = FP_OFF(filename);
    inregs.h.ah = 0x43;      /* CHMOD */
    inregs.h.al = 1;         /* SET ATTR */

    inregs.x.cx = attr;      /* attribute */

    intdos(&inregs, &outregs);
}

/************************************************************************/
/*   strlwr()   makes a string lower case                               */
/************************************************************************/
void strlower(char *string)
{
    char *s;
    for ( s = string ; *s ; ++s)  *s = (char)tolower(*s);
}

/************************************************************************/
/*      textdown() dumps a host file with no formatting                 */
/*      this routine handles wildcarding of text downloads              */
/************************************************************************/
void textdown(char *filename)
{
    int retval=0;
    int i;
    char oldverbose;

    outFlag     = OUTOK;
    setio(whichIO, echo, outFlag);

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR)
    {
        doCR();
        mPrintf(" No file %s", filename);
        return;
    }

    if (changedir(roomBuf.rbdirname) == -1 ) return;


    if (ambig(filename))
    {
        /* fill our directory array according to filename */
        oldverbose = verbose;
        verbose = FALSE;
        filldirectory(filename);
        verbose = oldverbose;

        /* print out all the files */
        for (i = 0; filedir[i].entry[0] && ( retval != ERROR); i++)
        {
            if (verbose)
                retval = dumpf(filedir[i].entry);
            else
                retval = dump(filedir[i].entry);
        }

#ifdef GOODBYE
        /* print out all the files */
        for (i = 0; filedir[i].entry[0] && 
        ( dump(filedir[i].entry) != ERROR); i++);
#endif

        if ( !i) { doCR(); mPrintf(" No file %s", filename); }

        /* free file directory structure */
        if(filedir != NULL)
        _ffree((void *)filedir);
    }
    else
    {
        if (verbose) 
            dumpf(filename);
        else 
            dump(filename);
    }

    sprintf(msgBuf->mbtext, "Text download of file %s in room %s]",
        filename, roomBuf.rbname);

    trap(msgBuf->mbtext, T_DOWNLOAD);

    doCR();

    /* go to our home-path */
    changedir(cfg.homepath);
}

/************************************************************************/
/*      textup()  handles textfile uploads                              */
/************************************************************************/
void textup(char *filename)
{
    int i;
    FILE *upfd;

    if (!expert)  tutorial("textup.blb");

    changedir(roomBuf.rbdirname);

    doCR();

    if ((upfd = fopen( filename, "wt")) == NULL)
    {
        mPrintf("\n Can't create %s!\n", filename);
    }
    else
    {
        while(  ((i = iChar()) != 26 /* CNTRLZ */ )
                && outFlag != OUTSKIP 
                && CARRIER )
        {
            fputc(i, upfd);
        }
        fclose(upfd);

        sprintf(msgBuf->mbtext, "Text upload of file %s in room %s]",
        filename, roomBuf.rbname);

        if (limitFlag && filexists(filename))  
            hide(filename);

        trap(msgBuf->mbtext, T_UPLOAD);
    }

    changedir(cfg.homepath);
}

/************************************************************************/
/*      unlinkfile()  handles .au  aide unlink                          */
/************************************************************************/
void unlinkfile(void)
{
    label filename;

    getNormStr("filename", filename, NAMESIZE, ECHO);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if (checkfilename(filename, 0) == ERROR) 
    {
        mPrintf(" No file %s", filename); 
        changedir(cfg.homepath);
        return;
    }

    if (!filexists(filename))
    {
        mPrintf(" No file %s", filename); 
        changedir(cfg.homepath);
        return;
    }

    /* if successful */
    if ( unlink(filename) == 0)
    {
        sprintf(msgBuf->mbtext,
        "File %s unlinked in %s] by %s",
        filename,
        roomBuf.rbname,
        logBuf.lbname );

        trap(msgBuf->mbtext, T_AIDE);

        aideMessage();

        killinfo(filename);

    }
    else mPrintf("\n Cannot unlink %s\n", filename);

    changedir(cfg.homepath);
}

/************************************************************************/
/*      wcdown() calls xmodem downloading routines                      */
/*      0=wc, 1=wx                                                      */
/************************************************************************/
void wcdown(char *filename)
{
    long transTime1, transTime2;
    if (changedir(roomBuf.rbdirname) == -1 )  return;

    /* no bad files */
    if (checkfilename(filename, 0) == ERROR)
    {
        doCR();
        mPrintf(" No file %s", filename);
        changedir(cfg.homepath);
        return;
    }

    /* no ambiguous xmodem downloads */
    if (ambig(filename))
    {
        changedir(cfg.homepath);
        return;
    }

    if (!filexists(filename))
    {
        doCR();
        mPrintf(" No file %s", filename);
    }
    else
    {
        if (!expert) tutorial("wcdown.blb");
        changedir(roomBuf.rbdirname);

        /* display # blocks & download time */
        blocks(filename, 128);

        if (getYesNo("Ready for file transfer", 0))
        {
           /* later to be replaced by my own xmodem routines */
           time(&transTime1);
           doCR();
           xsend(filename,0);
           
           time(&transTime2);

           if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime)
           {
               calc_trans(transTime1, transTime2, 0);
           }
           sprintf(msgBuf->mbtext, "WC download of file %s in room %s]",
                           filename, roomBuf.rbname);

           trap(msgBuf->mbtext, T_DOWNLOAD);
        }
    }
    /* go back to home */
    changedir(cfg.homepath);
}

/************************************************************************/
/*      wcup() calls xmodem uploading routines                          */
/************************************************************************/
void wcup(char *filename)
{
    long transTime1, transTime2;
    if (!expert)
    {
        tutorial("wcup.blb");
        doCR();
    }
    changedir(roomBuf.rbdirname);

    if (getYesNo("Ready for file transfer", 0))
    {
        /* later to be replaced by my own xmodem routines */

        time(&transTime1);             /* when did they start the Uload    */
        doCR();
        xreceive(filename,0);
        
        time(&transTime2);

        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime)
        {
            calc_trans(transTime1, transTime2, 1);
        }

        sprintf(msgBuf->mbtext, "WC upload of file %s in room %s]",
                                 filename, roomBuf.rbname);

        if (limitFlag && filexists(filename))  
            hide(filename);

        trap(msgBuf->mbtext, T_UPLOAD);

    }
    /* go back to home */
    changedir(cfg.homepath);
}


/************************************************************************/
/*      download()  menu level download routine                         */
/************************************************************************/
void download(char c)
{
    long    transTime1, transTime2;    /* to give no cost uploads       */
    char    filename[80];
    char    ch, xtype, i;
    time_t  t;
    char    ich;
                
    if (!c) 
      ch=(char)tolower(ich=(char)iCharNE());
    else
      ch = c;

     if (ch == '\n' || ch == '\r')
         ch = (!logBuf.protocol) ? '\0' : logBuf.protocol;

    xtype = (char)strpos(ch, extrncmd);
    
    if (!xtype)
    {
        if (ch == '?')
        {
          oChar('?');
          upDownMnu('D');
        }
        else{
          oChar(ich);
          mPrintf(" ?");
          if (!expert)
            upDownMnu('D');
        }
        return;
    }else{
        mPrintf("%s", extrn[xtype-1].ex_name);    
    }

    doCR();

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    getNormStr("filename", filename,
        (extrn[xtype-1].ex_batch) ? 80 : NAMESIZE, ECHO);
             
    if (extrn[xtype-1].ex_batch)
    {
      char *words[256];
      int count, i;
      char temp[80];

      strcpy(temp, filename);

      count = parse_it( words, temp);

      if (count == 0)
        return;

      for (i=0; i < count; i++)
      {
        if (checkfilename(words[i], 0) == ERROR)
        {
          doCR();
          mPrintf(" No file %s", words[i]);
          changedir(cfg.homepath);
          return;
        }

        if (!filexists(words[i]) && !ambig(words[i]))
        {
          doCR();
          mPrintf(" No file %s", words[i]);
          return;
        }

        if (!ambig(words[i]))
        {
          doCR();
          mPrintf("%s: ", words[i]);
          blocks(words[i], extrn[xtype-1].ex_block);
        }  
      }
    }else{
      if (checkfilename(filename, 0) == ERROR)
      {
        doCR();
        mPrintf(" No file %s", filename);
        changedir(cfg.homepath);
        return;
      }
      if (ambig(filename))
      {
          mPrintf("\n Not a batch protocol");
          changedir(cfg.homepath);
          return;
      }
      if (!filexists(filename))
      {
          doCR();
          mPrintf(" No file %s", filename);
          return;
      }
      blocks(filename, extrn[xtype-1].ex_block);
    }
  
    if (!strlen(filename))  return;

    if (!expert) tutorial("wcdown.blb");

    doCR();
    mPrintf("<3D0>ownload, <3H0>angup after download or <3A0>bort: ");
    i = (char)toupper(iCharNE());
    
    if (   i == 'D' || i == 'H' 
        || i == CR  || i == LF
        || i == 'Y' || i == 'N' /* old prompt compatible */)
    {
        if (i == 'H')
            mPrintf("Hangup after ");       
        
        mPrintf("Download"); doCR();
       
        time(&transTime1);
        wxsnd(roomBuf.rbdirname, filename, xtype);
        time(&transTime2);

        if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime)
        {
            calc_trans(transTime1, transTime2, 0);
        }

        sprintf(msgBuf->mbtext, "%s download of file %s in room %s]",
                      extrn[xtype-1].ex_name, filename, roomBuf.rbname);

        trap(msgBuf->mbtext, T_DOWNLOAD);
    }
    else
    {
        mPrintf("Abort"); doCR();
    }
    
    if (i == 'H')
    {
        time(&t);
        i = 0;
        ch = -1;
        xtype = TRUE;
        
        outFlag = IMPERVIOUS;
        setio(whichIO, echo, outFlag);
        
        doCR();
        doCR();
        mPrintf(" H to Hangup or any other key to abort."); doCR();
        
        do 
        {
            i = (char)(20L - (time(NULL) - t));
            
            if (i != ch)
            {
                mPrintf("%d ", i);
                ch = i;
            }
            
            if (BBSCharReady())
            {
                ch = (char)toupper(iCharNE());
                xtype = (char)(ch == 'H');
                break;
            }
        }
        while (i);
        
        doCR();
       
        if (xtype)
        {
            verbose = FALSE;
            terminate(TRUE);
        }
    }
    
    Mflush();
    
    /* go back to home */
    changedir(cfg.homepath);
}

/************************************************************************/
/*      upload()  menu level routine                                    */
/************************************************************************/
void upload(char c)
{
    long    transTime1, transTime2;
    label filename;
    char comments[64];
    char ch, xtype;
    char ich;
                
    if (!c) 
      ch=(char)tolower(ich=(char)iCharNE());
    else
      ch = c;

     if (ch == '\n' || ch == '\r')
         ch = (!logBuf.protocol) ? '\0' : logBuf.protocol;

    xtype = (char)strpos(ch, extrncmd);
    
    if (!xtype)
    {
        if (ch == '?')
        {
          oChar('?');
          upDownMnu('U');
        }
        else{
          oChar(ich);
          mPrintf(" ?");
          if (!expert)
            upDownMnu('U');
        }
        return;
    }else{
        mPrintf("%s", extrn[xtype-1].ex_name);    
    }

    doCR();

    if (!extrn[xtype-1].ex_batch)
    {
      getNormStr("filename", filename, NAMESIZE, ECHO);

      if (checkup(filename) == ERROR)  return;
             
      if (strlen(filename))
        getString("comments", comments, 64, FALSE, TRUE, "");
      else
        return;
    }
    else
        batchinfo(FALSE);

    if (!expert)
    {
        tutorial("wcup.blb");
        doCR();
    }

    if (getYesNo("Ready for file transfer", 0))
    {
       time(&transTime1);          /* when did they start the Uload    */
       wxrcv(roomBuf.rbdirname, extrn[xtype-1].ex_batch ? "" : filename,
         xtype);
       time(&transTime2);          /* when did they get done           */

       if (cfg.accounting && !logBuf.lbflags.NOACCOUNT && !specialTime)
       {
           calc_trans(transTime1, transTime2, 1);
       }

       Mflush();
       
       if (!extrn[xtype-1].ex_batch)
       {
          if (limitFlag && filexists(filename)) hide(filename);

          if (filexists(filename))
          {
             entercomment(filename, logBuf.lbname, comments);

             sprintf(msgBuf->mbtext, "%s upload of file %s in room %s]",
                       extrn[xtype-1].ex_name, filename, roomBuf.rbname);

             trap(msgBuf->mbtext, T_UPLOAD);
             
             if (comments[0])
                 sprintf(msgBuf->mbtext, "%s uploaded by %s\n Comments: %s",
                       filename, logBuf.lbname, comments);
             else
                 sprintf(msgBuf->mbtext, "%s uploaded by %s", filename,
                       logBuf.lbname);

             specialMessage();
          }
       }
       else
       {
           sprintf(msgBuf->mbtext, "%s file upload in room %s]",
               extrn[xtype-1].ex_name, roomBuf.rbname);

           trap(msgBuf->mbtext, T_UPLOAD);

           if (batchinfo(TRUE))
               specialMessage();
       }
    }
    changedir(cfg.homepath);
}

/*
 * Up/Down menu
 */
void upDownMnu(char cmd)
{
    int i;
  
    doCR();
    doCR();
    for (i=0; i<(int)strlen(extrncmd); i++)
        mPrintf(" 3.%c%c0>%s\n", cmd, *(extrn[i].ex_name), (extrn[i].ex_name + 1));
    mPrintf(" 3.%c?0> -- this list\n", cmd);
    doCR();
}


#define MAXBUF  MAXTEXT    /* 8K */

/*
 * Copy file function. (and test code)
 */
BOOL    copyfile(char *source, char *dest)
{
    FILE    *sfl, *dfl;
    int     bytes;
    
    if ((sfl = fopen(source, "rb")) == NULL)
    {
        return FALSE;
    }
    if ((dfl = fopen(dest, "wb")) == NULL)
    {
        fclose(sfl);
        return FALSE;
    }
    
    do
    {
        if ((bytes = fread(msgBuf->mbtext, 1, MAXBUF, sfl)) != 0)
        {
            if (fwrite(msgBuf->mbtext, 1, bytes, dfl) != (unsigned)bytes)
            {
                fclose(sfl);
                fclose(dfl);
                unlink(dest);
                return FALSE;
            }
        }
    }
    while (bytes == MAXBUF);
    
    fclose(sfl);
    fclose(dfl);

    return TRUE;
}


