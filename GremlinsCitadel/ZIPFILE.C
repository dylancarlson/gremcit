/* -------------------------------------------------------------------- */
/*                            Zipfile.c                                 */
/*                 ZIP File handling routines for ctdl                  */
/* -------------------------------------------------------------------- */
#include "ctdl.h"

/* -------------------------------------------------------------------- */
/*                              contents                                */
/*        readzip()     menu level .rz  HIGH level routine              */
/*        zipview()     this routine handles viewing ZIP-files          */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*              External variable definitions for ZIPFILE.C             */
/* -------------------------------------------------------------------- */

/* -------------------------------------------------------------------- */
/*              Defines for ZIPFILE.C                                   */
/* -------------------------------------------------------------------- */

#define F_CENTRAL  0x02014b50l

static char *how[] = { "Stored ", "Shrunk ", "Reduced", "Reduced",
                       "Reduced", "Reduced", "Implode", "Unknown", 0};


/* -------------------------------------------------------------------- */
/*   ZIP-files structures                                               */
/* -------------------------------------------------------------------- */

static struct {
    ulong        marker              ;
    ushort       version_made        ;
    ushort       version_needed      ;
    ushort       general_bit         ;
    ushort       compression         ;
    ushort       last_time           ;
    ushort       last_date           ;
    ulong        crc_32              ;
    ulong        compressed          ;
    ulong        uncompressed        ;
    ushort       filename_length     ;
    ushort       extra_length        ;
    ushort       comment_length      ;
    ushort       disk_number_start   ;
    ushort       internal_attributes ;
    ulong        external_attributes ;
    ulong        offset              ;
} central_file;

static struct {
    ushort       number_this_disk      ;
    ushort       disk_start_central    ;
    ushort       disk_central_entries  ;
    ushort       total_entries_central ;
    ulong        size_central_dir      ;
    ulong        offset_central        ;
    ushort       zipfile_comment_length;
} central_end;

/* -------------------------------------------------------------------- */
/*      readzip()  menu level .rz  HIGH level routine                   */
/* -------------------------------------------------------------------- */
void readzip(void)
{
    int i;
    label filename;
    char oldverbose;

    if (changedir(roomBuf.rbdirname) == ERROR) return;
    
    getNormStr("", filename, NAMESIZE, ECHO);
             
    if (!filename[0])
        strcpy(filename, "*.zip");

    if(!strchr(filename, '.'))
            strcat(filename, ".zip");

    if (ambig(filename))
    {
        /* fill our directory array according to filename */
        oldverbose = verbose;
        verbose = FALSE;
        filldirectory(filename);
        verbose = oldverbose;

        /* show contents of ambigous files */
        for (i = 0; filedir[i].entry[0] && !mAbort(FALSE) && (outFlag != OUTSKIP) &&
                (zipview(filedir[i].entry) != ERROR); i++) ;

        if (!i) mPrintf("\n No file %s", filename);

        /* free file directory structure */
        if(filedir != NULL)
            _ffree((void far *)filedir);
    }
    else
        zipview(filename);

    sprintf(msgBuf->mbtext, "Zip-view of file %s in room %s]",
            filename, roomBuf.rbname);

    trap(msgBuf->mbtext, T_DOWNLOAD);

    changedir(cfg.homepath);

}

/* -------------------------------------------------------------------- */
/*   zipview()  this routine handles viewing ZIP-files                  */
/* -------------------------------------------------------------------- */
char zipview(char *filename)
{
    FILE *fptr;

    char ttime[9];
    char tdate[9];
    char percent[5];
    char found=FALSE;
    uchar buffer[1024];

    ulong tsize;
    ulong tlength;

    int a, i=0, j, ch;

    outFlag   = OUTOK;
    setio(whichIO, echo, outFlag);


    if( (fptr = fopen(filename, "rb")) ==NULL)
    {
        mPrintf("\n No file %s", filename);
        return ERROR;
    }

    if(fseek(fptr, -17L, SEEK_END) !=0)
    {
        fclose(fptr);
        return FALSE;
    }

    tsize = ftell(fptr);

    do
    {
        if (tsize < 1024L)
            tlength = tsize;
        else
            tlength = 1024L;

        fseek(fptr, -(tlength), SEEK_CUR);
        fread((void *)buffer, (int)tlength, 1, fptr);
        for(i=0; i<(int)(tlength -3l); i++)
        {
            if(buffer[i]   == 0x50 &&
               buffer[i+1] == 0x4b &&
               buffer[i+2] == 0x05 &&
               buffer[i+3] == 0x06)
            {
                fseek(fptr, -( tlength - (long)(i+4) ), SEEK_CUR);
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
            fseek(fptr, -(tlength), SEEK_CUR);
            tsize = ftell(fptr);
        }

    } while (!found && tsize > 0L);

    doCR();
    mPrintf(" Contents of: %s", filename);

    if(!found)
    {
        doCR();
        mPrintf(" Can't find End of Central Directory in ZIP-file.");
        doCR();
        fclose(fptr);
        return FALSE;
    }

    tlength = 0l;
    tsize = 0l;

    fread(&central_end, sizeof(central_end), 1, fptr);

    if(central_end.zipfile_comment_length)
    {
        mPrintf(" - ");
        for(i=0; i<(int)central_end.zipfile_comment_length; i++)
        {
            ch = fgetc(fptr);
            if(ch == '\n')
                doCR();
            else
                oChar((char)ch);
        }
    }

    doCR();
    doCR();

    if((fseek(fptr, central_end.offset_central, SEEK_SET)) != 0)
    {
        mPrintf(" Can't seek to Central Directory area of ZIP-file.");
        doCR();
        fclose(fptr);
        return FALSE;
    }

    if(verbose)
    {
        mPrintf(" Length  Method  Size    Ratio  Date    Time    Name");
        doCR();
        mPrintf(" ------- ------- ------- ----- ------- -------- --------");
               /* ####### XXXXXXX ####### ##.#% XXXXXXX XXXXXXXX XXXXXXXX*/
    }
    else
    {
        mPrintf(" Length   Date    Time    Name");
        doCR();
        mPrintf(" ------- ------- -------- --------");
    }

    doCR();

    for(i=0; (i < (int)central_end.disk_central_entries) &&
                             (outFlag != OUTNEXT)   &&
                             (outFlag != OUTSKIP)     ; i++)
    {
        fread(&central_file, sizeof(central_file), 1, fptr);
        if(central_file.marker != F_CENTRAL)
        {
            mPrintf(" Error in Zip-file");
            doCR();
            fclose(fptr);
            return FALSE;
        }

        if(central_file.uncompressed != 0l)
            a = (int)(((central_file.uncompressed-central_file.compressed)*1000l)
                                             /central_file.uncompressed);
        else
            a = 0;

        sprintf(percent, "%2d.%1.1d", a/10, a % 10);

        gettstamp(ttime, central_file.last_time);
        getdstamp(tdate, central_file.last_date);

        if(central_file.compression > 6)  central_file.compression = 7;

        mPrintf(" %7ld", central_file.uncompressed);

        if(verbose)
        mPrintf(" %s %7ld %4s%%",
                      how[central_file.compression],
                      central_file.compressed,
                      percent);

        mPrintf(" %s %s ", tdate, ttime);

        for (j=0; j<(int)central_file.filename_length; j++)
        {
            ch = fgetc(fptr);
            oChar((char)ch);
        }

        doCR();

        tsize   += central_file.compressed;
        tlength += central_file.uncompressed;

        if((fseek(fptr, (long)(central_file.extra_length +
                               central_file.comment_length),
                                                  SEEK_CUR)) != 0)
        {
            mPrintf(" Can't seek to next Central Directory entry of ZIP-file.");
            doCR();
            fclose(fptr);
            return FALSE;
        }
    }

    if((outFlag == OUTNEXT) || (outFlag == OUTSKIP))
    {
        fclose(fptr);
        return FALSE;
    }

    if(tlength != 0l)
        a = (int)(((tlength-tsize)*1000l)/tlength);
    else
        a = 0;

    sprintf(percent, "%2d.%1.1d", a/10, a % 10);

    if(verbose)
    {
        mPrintf(" ------- ------- ------- ----- ------- -------- --------");
    }
    else
    {
        mPrintf(" ------- ------- -------- --------");
    }
    doCR();
            
    mPrintf(" %7ld", tlength);

    if(verbose)
        mPrintf("         %7ld %4s%%", tsize, percent);

    mPrintf("                       %3d", central_end.disk_central_entries);

    doCR();
    fclose(fptr);
    return TRUE;
}


