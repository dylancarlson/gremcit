/************************************************************************/
/*                              Infofile.c                              */
/*                 Infofile handling routines for ctdl                  */
/************************************************************************/
#include "ctdl.h"

/************************************************************************/
/*                              contents                                */
/*                                                                      */
/*      addinfo()               adds an entry to fileinfo.cit           */
/*      entercomment()          high level upload/comment routine       */
/*      fillinfo()              allocates buffer and reads fileinfo.cit */
/*      infoslot()              returns slot of filename in info-buffer */
/*      killinfo()              removes comment from fileinfo.cit       */
/*      readinfofile()          menu level .ri .rvi routine             */
/*      setfileinfo()           menu level .as routine                  */
/*      showinfo()              show info-buffer according to verbose   */
/*      updateinfo()            removes all non-existant entries        */
/*      batchinfo()             prompts for comments on new files when  */
/*                              TRUE, FALSE adds null comments          */
/*      moveFile()              copy info-buffer & move file            */
/************************************************************************/

#define MINUTE 60

/************************************************************************/
/*            External function definitions for FILEINFO.C              */
/************************************************************************/

/************************************************************************/
/*      addinfo()  appends comment fileinfo.cit                         */
/************************************************************************/
void addinfo( char *filename, char *uploader, char *comment )
/* char *filename, *uploader, *comment; */
{  
    struct fInfo info;
    FILE *fd;

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    strcpy(info.fn,       filename);
    strncpy(info.uploader, uploader, 19);
    info.uploader[20] = 0;
    strcpy(info.comment,  comment);

    if ( (fd = fopen("fileinfo.cit", "ab" )) == NULL) return;

    fwrite( &info, sizeof(*fileinfo), 1, fd);

    fclose(fd);

    changedir(cfg.homepath);
}

/************************************************************************/
/*      entercomment()  high level upload/comment routine               */
/************************************************************************/
#ifdef OLD_INFO
void entercomment(filename, uploader, comment)
char *filename, *uploader, *comment;
{
    killinfo(filename);             /* kill old entry if present */

    addinfo(filename, uploader, comment);
}
#endif

/************************************************************************/
/*      fillinfo()  allocates buffer and reads fileinfo.cit in ram      */
/************************************************************************/
void fillinfo()
{
    FILE *fd;

    if (changedir(roomBuf.rbdirname) == -1 )
    {
        infolength = 0;             /* no infofile if can't change to dir */
        changedir(cfg.homepath);
        return;
    }

    if ( (fd = fopen("fileinfo.cit", "rb" )) == NULL)
    {
        infolength = 0;                /* 0 length if file not there */
        changedir(cfg.homepath);
        return;
    }

    infolength = (int)filelength(fileno(fd));

    if(infolength == 0)
    {
        fclose(fd);         /* if found and 0 length don't halloc */
        return;
    }                

    fileinfo = _fcalloc (infolength, 1);

    if (fileinfo == NULL)
    {
         doCR();
         mPrintf(" Error allocating fileinfo ");
         doCR();
    }
    else  fread( fileinfo, infolength, 1, fd );

    fclose(fd);
}

/************************************************************************/
/*      infoslot()  returns slot of specified filename in info-buffer   */
/************************************************************************/
int infoslot(char *filename)
/* char *filename; */
{
    int i;
    int numrecords;

    if (infolength == 0) return(ERROR);     /* don't try if 0 length */

    numrecords = infolength / ( sizeof(*fileinfo) );

    for (i = 0;  i < numrecords; ++i)
    {
        if (strcmpi(filename, fileinfo[i].fn) == SAMESTRING)
            return(i);
    }
    return(ERROR);
}


/************************************************************************/
/*      killinfo()  removes comment from fileinfo.cit                   */
/************************************************************************/
void killinfo(char *filename)
/* char *filename; */
{
    int slot, numslots;
    FILE *fd;

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    fillinfo();

    if(infolength ==0)              /* don't make blank infofile.cit's */
    {                            /* don't hfree if 0 lenght wasn't halloc'ed */
        changedir(cfg.homepath);
        return;
    }

    numslots = (infolength) / sizeof(*fileinfo);

    slot = infoslot(filename);

    if (slot != ERROR)
    {
        if( (fd = fopen( "fileinfo.cit", "wb" )) == NULL)
        {
            changedir(cfg.homepath);
            return;
        }

/*        hmemcpy( &fileinfo[slot], &fileinfo[slot+1],
             (long)(sizeof(*fileinfo) * (( numslots - slot) - 1)) );  */

        _fmemmove( &fileinfo[slot], &fileinfo[slot+1],
             (unsigned)(sizeof(*fileinfo) * (( numslots - slot) - 1)) );


        fwrite( fileinfo, (infolength - sizeof(*fileinfo)), 1, fd);

        fclose(fd);

        if((infolength - sizeof(*fileinfo)) ==0)
             unlink("fileinfo.cit");       /* remove if 0 length file */
    }

    _ffree((void *)fileinfo);

    changedir(cfg.homepath);
}

/************************************************************************/
/*      readinfofile()  menu level .ri .rvi routine                     */
/************************************************************************/
void readinfofile(BOOL ask)
{
    label filename;
    char oldverbose;

    if (changedir(roomBuf.rbdirname) == -1 )  return;
    
    if (ask)
    {
        getNormStr("", filename, NAMESIZE, ECHO);
    }
    else
    {
        *filename = 0;
        doCR();
    }

    /* if there is no info-file, just do a normal disk directory */
    if (!filexists("fileinfo.cit"))
    {
        if (strlen(filename))  dir(filename);
        else                   dir("*.*");
        changedir(cfg.homepath);
        return;
    }

    oldverbose = verbose;
    verbose = TRUE;
    if (strlen(filename))  filldirectory(filename);
    else                   filldirectory("*.*");
    verbose = oldverbose;

    /* check for matches */
    if ( !filedir[0].entry[0]) 
    {
        if (!strlen(filename)) strcpy(filename, "*.*");

        mPrintf("\n No file %s", filename);

        /* free file directory structure */
        if(filedir != NULL)
            _ffree((void *)filedir);

        changedir(cfg.homepath);
        return;
    }

    /* allocate & read in fileinfo buffer */
    fillinfo();

    /* display info-buffer according to verbose */
    showinfo();

    /* free file directory structure */
    if(filedir != NULL)
        _ffree((void *)filedir);

    /* free info-buffer */

    if(infolength != /*NULL*/0)
        _ffree((void *)fileinfo);
}

/************************************************************************/
/*      setfileinfo()  menu level .as routine sets entry to aide's name */
/*                     if none present or leaves original uploader      */
/************************************************************************/
#ifdef OLD_INFO
void setfileinfo(void)
{
    label filename;
    label uploader;
    char comments[64];
    int  slot;

    getNormStr("filename", filename, NAMESIZE, ECHO);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    /* no bad file names */
    if (checkfilename(filename,0) == ERROR)
    {
        mPrintf("\n No file %s", filename);
        changedir(cfg.homepath);
        return;
    }

    /* no file name? */
    if (!filexists(filename))
    {
        mPrintf("\n No file %s", filename);
        changedir(cfg.homepath);
        return;
    }

    fillinfo();
             
    slot = infoslot(filename);

             /* set to old uploader if one was present  */
             /* or set logBuf.lbname if no fileinfo.cit */
    if(slot !=ERROR && fileinfo[slot].uploader[0] !=0)
    {
        strcpy(uploader, fileinfo[slot].uploader);
    }
    else
    {
        strcpy(uploader, logBuf.lbname);
    }

             /* free memory if it was halloc'ed */
    if(infolength != NULL)
       _ffree((void *)fileinfo);
   
    getString("comments", comments, 64, FALSE, TRUE, "");

    entercomment(filename, uploader, comments);

    sprintf(msgBuf.mbtext, "File info changed for file %s by %s",
    filename, logBuf.lbname);

    trap(msgBuf.mbtext, T_AIDE);

    changedir(cfg.homepath);
}
#endif

/************************************************************************/
/*      showinfo()  display info-buffer according to verbose            */
/************************************************************************/
void showinfo(void)
/* char verbose; */
{
    int i, slot;

    char comment[64];
    label uploader;
    char filename[15];
    char size[10];
    char downtime[10];
    char date[20];
    char time[20];

    outFlag = OUTOK;
    setio(whichIO, echo, outFlag);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    /* check for matches */
    if ( !filedir[0].entry[0]) 
    {
        mPrintf("\n No file %s", filename);
        /* go to our home-path */
        changedir(cfg.homepath);
        return;
    }

    if (!verbose)
    {
        doCR();
        mPrintf("3Filename        Size  Comments0");
        doCR();/*--------.--- -------  ------------------... */
    }

    for (i = 0; 
         (filedir[i].entry[0] && (outFlag != OUTSKIP) && !mAbort(FALSE) );
         ++i)
    {    

        /* get rid of asterisks */
        filedir[i].entry[0]  = ' ';
        filedir[i].entry[13] = ' ';

        sscanf( filedir[i].entry, 
                "%s %s %s %s %s",
                filename,
                date,
                size,
                downtime,
                time );

        slot = infoslot(filename);

        if (slot != ERROR)
        {
            strcpy( comment,  fileinfo[slot].comment );
            strcpy( uploader, fileinfo[slot].uploader);
        }
        else
        {
            comment[ 0] = '\0';
            uploader[0] = '\0';
        }

        if(verbose)
        {
            doCR();
            mPrintf(" %-16s %s", "3Filename:0", filename); 

            doCR();
            mPrintf(" %-16s %s (%s minutes to download)", "3Size:0",
            size, downtime);

            doCR();
            mPrintf(" %-16s %s %s", "3Date:0", date, time);

            doCR();
            mPrintf(" %-16s %s", "3Uploaded By:0", uploader);

            doCR();
            mPrintf(" %-16s %s", "3Comments:0", comment); 
            doCR();
        }
        else
        {
            mPrintf("%-12s %7s  %s", filename, size, comment);
            doCR();       
        }
    }

    /* go to our home-path */
    changedir(cfg.homepath);
}

/************************************************************************/
/*      updateinfo()  removes all non-existant entries from fileinfo cit*/
/************************************************************************/
void updateinfo()
{
    int i, k, numrecords, nuke;
    char flname[15];
    FILE *fd;

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    fillinfo();

    if(infolength ==0)   /* don't make blank infofile.cit's */
    {
        changedir(cfg.homepath);
        return;
    }

    numrecords = infolength / ( sizeof(*fileinfo) );

    verbose = TRUE;
    filldirectory("*.*");

    for (i = 0;  i < numrecords; ++i )
    {
                           /* if (!filexists(fileinfo[i].fn))  */
        nuke = TRUE;
        for(k=0; filedir[k].entry[0]; k++)
        {
            filedir[k].entry[13] = ' ';
            sscanf(filedir[k].entry, "%s", flname);
            if(strcmpi(fileinfo[i].fn, flname) == SAMESTRING)
            {
                nuke = FALSE;
                break;
            }
        }
        if(nuke)
        {
            _fmemmove( &fileinfo[i], &fileinfo[i+1],
               (unsigned)(sizeof(*fileinfo) * (( numrecords - i) - 1)));

     /*     hmemcpy( &fileinfo[i], &fileinfo[i+1],
               (long)(sizeof(*fileinfo) * (( numrecords - i) - 1)));  */
            i--;
            numrecords--;
        }
    }

    if( (fd = fopen( "fileinfo.cit", "wb" )) == NULL)
    {
        changedir(cfg.homepath);

        /* free file directory structure */
        if(filedir != NULL)
            _ffree((void *)filedir);

        _ffree((void *)fileinfo);
        return;
    }

    fwrite( fileinfo, (numrecords * sizeof(*fileinfo)), 1, fd);

    fclose(fd);

    if((numrecords * sizeof(*fileinfo))==0)
        unlink("fileinfo.cit");

    /* free file directory structure */
    if(filedir != NULL)
        _ffree((void *)filedir);

    _ffree((void *)fileinfo);

    changedir(cfg.homepath);
}

/************************************************************************/
/*  batchinfo() askes for comments on all files not in fileinfo.cit     */
/*              when TRUE, adds null fields when FALSE                  */
/************************************************************************/
int batchinfo(int askuser)
/* int askuser; */
{
    int i, slot, total = 0;
    char comments[64];
    label uploader;
    char filename[15];
    char size[10];
    char date[20];
    char tmp[90];

    sprintf(msgBuf->mbtext, "Batch upload by %s\n", logBuf.lbname);

    if (changedir(roomBuf.rbdirname) == -1 )  return(0);

    verbose = TRUE;
    filldirectory("*.*");

    fillinfo();

    if(askuser)
        strcpy(uploader, logBuf.lbname);
    else
        uploader[0] = '\0';

    for(i=0; filedir[i].entry[0];i++)
    {
        filedir[i].entry[13] = ' ';
        sscanf(filedir[i].entry, "%s %s %s", filename, date, size);
        slot = infoslot(filename);
        if(slot == ERROR)
        {
            if(askuser)
            {
                doCR();
                mPrintf("%-12s %7s %s", filename, size, date);
                doCR();
                getString("comments", comments, 64, FALSE, TRUE, "");
                addinfo(filename, uploader, comments);
                total++;
                if (!comments[0])
                   sprintf(tmp, " %s\n", filename);
                else
                   sprintf(tmp, " %s: %s\n", filename, comments);
                strcat(msgBuf->mbtext, tmp);

                sprintf(tmp, "Batch upload of %s in room %s]",
                              filename, roomBuf.rbname);
                trap(tmp, T_UPLOAD);
            }
            else
            {
                comments[0] = '\0';
                addinfo(filename, uploader, comments);
            }
        }
    }
    if(infolength !=0)
        _ffree((void *)fileinfo);

    changedir(cfg.homepath);

    /* free file directory structure */
    if(filedir != NULL)
        _ffree((void *)filedir);

    return total;
}

/************************************************************************/
/*      moveFile()  copy info-buffer & move file                        */
/************************************************************************/
void moveFile(void)
{
    struct fInfo info;
    FILE *fd;

    char    source[20], destination[64];
    char    temp[84];
    label   destRoom;
    
    int i, slot, roomNo, oldRoom;

    char size[10];
    char downtime[10];
    char date[20];
    char time[20];

    doCR();
    /* 
     * Source file checking... 
     */
    getNormStr("source filename", source, NAMESIZE, ECHO);
    if (!strlen(source)) return;

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if ( (checkfilename(source, 0) == ERROR) || ambig(source))
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

    /*
     * destination room check.
     */
    getNormStr("destination room", destRoom, NAMESIZE, ECHO);
    if (!strlen(destRoom)) 
        return;
        
    roomNo = roomExists(destRoom);
    if (roomNo == ERROR) 
        roomNo = partialExist(destRoom);
        
    if (roomNo == ERROR)    
    {
        mPrintf("No room '%s'.", destRoom); doCR();
        return;
    }
    
    oldRoom = thisRoom;
    getRoom(roomNo);
    if (roomBuf.rbflags.MSDOSDIR)
    {
        strcpy(destination, roomBuf.rbdirname);
        strcpy(destRoom, roomBuf.rbname);
    }
    else
    {
        *destination = '\0';
    }
    getRoom(oldRoom);
    
    if (*destination == '\0')
    {
        mPrintf("Room '%s' not a directory room.", destRoom); doCR();
        return;
    }
    
    if (changedir(destination) == -1 )
    {
        mPrintf("\n Invalid pathname.");
        changedir(cfg.homepath);
        return;
    }

    /*
     * Source file at destination 
     */
    sprintf(temp, "%s\\%s", destination, source);
    if (filexists(temp))
    {
        mPrintf("\n File exists."); 
        changedir(cfg.homepath);
        return;
    }
    
    if (changedir(roomBuf.rbdirname) == -1 )  return;

    verbose = TRUE;
    filldirectory(source);

    /* check for matches */
    if ( !filedir[0].entry[0])
    {
        mPrintf("\n No file %s", source);

        /* free file directory structure */
        if(filedir != NULL)
            _ffree((void *)filedir);

        /* go to our home-path */
        changedir(cfg.homepath);
        return;
    }

    fillinfo();

    for (i = 0; ( filedir[i].entry[0]);  ++i)
    {    

        /* get rid of asterisks */
        filedir[i].entry[0]  = ' ';
        filedir[i].entry[13] = ' ';

        sscanf(filedir[i].entry, "%s %s %s %s %s",
        source,
        date,
        size,
        downtime,
        time );

        slot = infoslot(source);

        if (slot != ERROR)
        {
            strcpy(info.fn,       source);
            strcpy(info.uploader, fileinfo[slot].uploader);
            strcpy(info.comment,  fileinfo[slot].comment );
        }
        else
        {
            strcpy(info.fn, source);
            info.comment[ 0] = '\0';
            info.uploader[0] = '\0';
        }

    }

    /* free file directory structure */
    if(filedir != NULL)
        _ffree((void *)filedir);

    /* free info-buffer */
    if(infolength != /*NULL*/ 0)
    {
        _ffree((void *)fileinfo);
    }

    changedir(destination);

    if ( (fd = fopen("fileinfo.cit", "ab" )) == NULL) 
    {
        if ( (fd = fopen("fileinfo.cit", "wb" )) == NULL) 
        {
            mPrintf("\n Unable to write FILEINFO.CIT!\n");
            return;
        }
    }

    fwrite( &info, sizeof(*fileinfo), 1, fd);

    fclose(fd);

    if (changedir(roomBuf.rbdirname) == -1 )  return;

    if ( rename(source, temp) != 0)
    {
        doCR();
        mPrintf("Copying file..");
        doCR();
        if (copyfile(source, temp))
        {
            unlink(source);
        }
        else
        {
            doCR();
            mPrintf("Cannot move %s", source);
            doCR();
            changedir(cfg.homepath);
            return;
        }
    }
    
    sprintf(msgBuf->mbtext,
    "File %s moved to %s] from %s] by %s",
    source, 
    destRoom, 
    roomBuf.rbname,
    logBuf.lbname );

    trap(msgBuf->mbtext, T_AIDE);

    aideMessage();

    killinfo(source);           /* kill old entry */

    /* go to our home-path */
    changedir(cfg.homepath);
}

