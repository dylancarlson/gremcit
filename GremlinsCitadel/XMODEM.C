/*
 * xmodem.c copyright 1986 Maple Lawn Farm, Inc. 
 * modified for MS-DOS and MSC 5.0 by Matthew Pfleger
 * supports Xmodem Checksum, Xmodem CRC and Xmodem CRC 1K
 * (c) 1988 Anticlimactic Teleservices Ltd.
 *
 * Updated June 1990 for use with DragCit
 */

#include "ctdl.h"

#define MAXERR 10

static  char   soh = 0x01,
               stx = 0x02,
               eot = 0x04,
               ack = 0x06,
               nak = 0x15,
               can = 0x18,
               crcinit = 'C';

static unsigned char cksum;

static  int    crc;   /* must be set before calling xget() */
static  int    onek;  /* must be set before calling xget() or xput() */
static  int    bsize;

static  unsigned       crcsum;

static  char far *buf; 

/*
 *  note: Mode = 0: Xmodem Checksum
 *        Mode = 1: Xmodem CRC
 *        Mode = 2: Xmodem CRC 1K
 */

xsend(char *filename, int mode)
{
    FILE    *fp;
    char retval;

    crc  = !!mode;
    onek = (mode == 2);
    bsize = (mode == 2) ? 1024 : 128;

    buf = (char far *)_fcalloc(bsize, 1);
    if (buf == NULL)
        cPrintf("\nwc.c : _fcalloc() Error\n");

    if (!(fp = fopen(filename, "rb"))) 
    {
        cPrintf("\nCannot open %s\n", filename); 
        return(0);
    }
    retval = (char)xput(fp);
    _ffree(buf);
    return(retval);
}
 
xreceive(char *filename, int mode)
{
    FILE    *fp;
    char retval;

    crc  = !!mode;
    onek = (mode == 2);
    bsize = (mode == 2) ? 1024 : 128;

    if (!(fp = fopen(filename, "wb")))
    {
        cPrintf("\nCannot write %s\n", filename);
        return(0);
    }

    buf = (char far *)_fcalloc(bsize, 1);
    if (buf == NULL)
        cPrintf("\nwc.c : _fcalloc() Error\n");

    retval = (char)xget(fp);
    _ffree(buf);
    
    if (!retval)
    {
        if (unlink(filename) == -1)
            cPrintf("\nCannot unlink %s\n", filename);
        return(0);
    }
    return(1);
}

xget(FILE *fp)
{
    unsigned char  b = 1,
                   inch,
                   inch2,
                   crchi,
                   errcount = 0;
    int       blocknum = 1;
    register  i; 

    (crc) ? PUTRS(crcinit) : PUTRS(nak);
    FLUSHRS();
    for (;;)
    {
        if (!errcount)
        {
             status("Reading block", blocknum, errcount);
        }

        if (!CARRSTATRS())
        {
            status("Carrier loss", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }

        if (errcount == MAXERR)
        {
            status("Maximum errors exceeded", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }
        errcount++;

        if (rchar(10, &inch) == -1)
        {
            if (onek)
                status("Timeout during STX", blocknum, errcount);
            else
                status("Timeout during SOH", blocknum, errcount);

            cksend((char)(crc ? crcinit : nak)) ;
            continue;
        }
        if (inch == (unsigned char)eot) 
            break;
        if (inch == (unsigned char)can)
        {
            status("CAN", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }

        if (onek)
        {
            if (inch != (unsigned char)stx)
            {
                status("Bad STX", blocknum, errcount);

                cksend(nak);
                continue;
            }
        }
        else 
        {
            if (inch != (unsigned char)soh)
            {
                status("Bad SOH", blocknum, errcount);

                cksend(nak);
                continue;
            }
        }

        if (rchar(2, &inch) == -1)
        {
            status("Timeout during blocknum", blocknum, errcount);

            cksend(nak);
            continue;
        }

        if (rchar(2, &inch2) == -1)
        {
            status("Timeout during ~blocknum", blocknum, errcount);

            cksend(nak);
            continue;
        }

        if ((~inch &0xff) != (inch2 &0xff))
        {
            status("Blocknum mis-match", blocknum, errcount);

            cksend(nak);
            continue;
        }

        if (inch == (unsigned char)((b - 1) &0xff))
        {
            status("Duplicate record", blocknum, errcount);

            cksend(ack);
            continue;
        }

        if ((unsigned char)(inch & 0xff) != b) 
        {
            status("Unexpected blocknum", blocknum, errcount);

            cksend(nak);
            continue;
        }

        if ((inch2 & 0xff) != (~b & 0xff))
        {
            status("Unexpected ~blocknum", blocknum, errcount);

            cksend(nak);
            continue;
        }
        /* Read in 128 byte block without taking time for checksums or crc. */
        for (i = 0; i < bsize; i++)
        {
            if (rchar(2, (unsigned char *)&buf[i]) == -1) 
                break;
        }

        if (i < bsize)
        {
            status("Timeout data recv", blocknum, errcount);

            cksend(nak);
            continue;
        }
        if (crc)
        {
            if (rchar(2, &crchi) == -1)
            {
                status("Timeout crc hibyte", blocknum, errcount);

                cksend(nak);
                continue;
            }
            crchi &= 0xff;
        }

        if (rchar(2, &inch) == -1)
        {
            if (crc) 
                status("Timeout crc lobyte", blocknum, errcount);
            else
                status("Timeout checksum", blocknum, errcount);


            cksend(nak);
            continue;
        }
        /* Now, when we have the whole packet, do the checksum or crc. */
        for (cksum = 0, crcsum = 0, i = 0; i < bsize; i++) 
            upsum(buf[i]);
        if (crc)
        {
            upsum(0);        /* needed for crcsum */
            upsum(0);
            if ((unsigned int)((inch & 0xff) + (crchi << 8)) != crcsum)
            {
                status("Unexpected crc", blocknum, errcount);

                cksend(nak);
                continue;
            }
        }
        else
        {
            cksum %= 256;
            if (cksum != (unsigned char)(inch & 0xff))
            {
                status("Unexpected checksum", blocknum, errcount);

                cksend(nak);
                continue;
            }
        }
        PUTRS(ack);
        FLUSHRS();

        fwrite(buf, bsize, 1, fp);
        b++;
        b %= 256;
        blocknum++;
        errcount = 0;
    }
    PUTRS(ack);
    FLUSHRS();

    complete(1, blocknum, errcount - 1, fp);
    return(1);
}

xput(FILE *fp)
{
    unsigned char  b = 1,
                   cb,
                   crclo,
                   inch,
                   errcount = 0;
    int       blocknum = 1;
    register  i; 
    int       cread;

    FLUSHRS();

    status("Awaiting startup NAK/'C'", blocknum, errcount);

    rchar(60, &cb); 
    if (cb == (unsigned char)crcinit)
        crc = 1;
    else if (cb == (unsigned char)nak && !onek)
        crc = 0;
    else 
    {
        if (onek)
            status("No startup 'C'", blocknum, errcount);
        else
            status("No startup NAK/'C'", blocknum, errcount);

        sleep(2);

        complete(0, blocknum, errcount, fp);
        return(0);
    }
    cread = fillbuf(fp, buf);
    while (cread)
    {
        if (!errcount)
        {
            status("Transmitting block", blocknum, errcount);
        }

        if (!CARRSTATRS())
        {
            status("Carrier loss", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }

        if (errcount == MAXERR)
        {
            status("Maximum errors exceeded", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }
        errcount++;

        for (i = cread; i < bsize; i++) 
            buf[i] = 0x1a;

        if (onek)
            PUTRS(stx);
        else
            PUTRS(soh);

        PUTRS(b);
        cb = (unsigned char)(~b & 0xff);
        PUTRS(cb);

        for (cksum = 0, crcsum = 0, i = 0; i < bsize; i++) 
        {
            PUTRS(buf[i]);
            upsum(buf[i]);
        }

        if (crc)
        {
            upsum(0);        /* needed for crcsum */
            upsum(0);
            crclo = (unsigned char)crcsum;
            cb = (unsigned char)(crcsum >> 8);
            PUTRS(cb);
            PUTRS(crclo);
        }
        else
        {
            cksum %= 256;
            PUTRS(cksum);
        }

        FLUSHRS();

        if (rchar(15, &inch) == -1)
        {
            status("Timeout after block", blocknum, errcount);

            continue;
        }
        if (inch == (unsigned char)can)
        {
            status("CAN after block", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }
        if (inch != (unsigned char)ack)
        {

            status("Non-ACK after block", blocknum, errcount);

            continue;
        }

        cread = fillbuf(fp, buf);
        b++;
        b %= 256;
        blocknum++;
        errcount = 0;
    }
    for (;;)
    {
        PUTRS(eot);
        FLUSHRS();

        if (rchar(15, &inch) == -1)
        {
            status("Timeout during EOT", blocknum, errcount);

            continue;
        }
        if (inch == (unsigned char)can)
        {
            status("CAN during EOT", blocknum, errcount);
            sleep(2);

            complete(0, blocknum, errcount, fp);
            return(0);
        }
        if (inch != (unsigned char)ack)
        {
            status("Non-ACK during EOT", blocknum, errcount);

            continue;
        }
        break;
    }
    complete(1, blocknum, errcount, fp);
    return(1);
}

fillbuf(FILE *fp, char far *buf)
{
    int i;
    
    i = fread(buf, 1, bsize, fp);
    return(i);
}

void upsum(char c)
{
    register  unsigned  shift;
    register  unsigned  flag;

    if (crc)
        for (shift = 0x80; shift; shift >>= 1)  
        {
            flag = (crcsum & 0x8000);
                crcsum <<= 1;
                crcsum |= ((shift & c) ? 1 : 0);
                if (flag)
                    crcsum ^= 0x1021;
        }
        else
            cksum += c;
}

rchar(unsigned timeout, unsigned char *ch)
{
    long t;

    t = time(NULL);

    while ((unsigned int)(time(NULL) - t) < timeout)
    {
        if (!CARRSTATRS()) 
            return(-1);

        if (STATRS())
        {
            *ch = (char)GETRS();
            return(1);
        }
    }
    return(-1);
}

void status(char *string, int blocknum, int errcount)
{
    cPrintf("\r%-32s (Sector #%d) (Error #%d) ", string, blocknum, errcount);
}

void sleep(unsigned delay)
{
    long t;

    t = time(NULL);
 
    while ((unsigned int)(time(NULL) - t) < delay); 
}

void complete(int success, int blocknum, int errcount, FILE *fp)
{
    char message[80];
    
    fclose(fp);
    sprintf(message, "File transfer %s.", (success) ? "complete" : "cancelled");
    status(message, blocknum, errcount);
    cPrintf("\n");
}

void cksend(char ch)
{
    int j;
    unsigned char cp;

    do
    {
        j = rchar(2, &cp);
    } while (j != -1);
    PUTRS(ch);
    FLUSHRS();
}

