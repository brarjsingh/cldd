/**
 * Copyright (C) 2010 Geoff Johnson <geoff.jay@gmail.com>
 *
 * This file is part of cldd.
 *
 * cldd is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <common.h>

#include "cldd.h"
#include "utils.h"

int set_nonblocking (int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl (fd, F_SETFL, flags | O_NONBLOCK);
}

/**
 * writen
 *
 * Write n bytes to a descriptor. Taken from Stevens UNP.
 */
ssize_t
writen (int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
//    CLDD_MESSAGE("Want to print: %s", ptr);
//    CLDD_MESSAGE("At write start n = %d, nleft = %d", n, nleft);
    while (nleft > 0)
    {
        if ((nwritten = write (fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }

//        CLDD_MESSAGE("Write loop: nwritten = %d", nwritten);
        nleft -= nwritten;
        ptr   += nwritten;
    }
//    CLDD_MESSAGE("At write finish n = %d, nleft = %d", n, nleft);

    return n;
}

static int  read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];

static ssize_t
my_read (int fd, char *ptr)
{

    if (read_cnt <= 0) {
again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return(-1);
        } else if (read_cnt == 0)
            return(0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}

ssize_t
readline (int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;      /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return(n - 1);  /* EOF, n - 1 bytes were read */
        } else
            return(-1);     /* error, errno set by read() */
    }

    *ptr = 0;   /* null terminate like fgets() */
    return(n);
}
