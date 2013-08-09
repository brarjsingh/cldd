
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

#include "common.h"

#include "cldd.h"
#include "error.h"

static void
err_doit (int errnoflag, int level, const char *fmt, va_list ap)
{
    int errno_save, n;
    char buf[MAXLINE + 1];

    errno_save = errno;     /* value caller might want printed */
//#ifdef  HAVE_VSNPRINTF
    vsnprintf (buf, MAXLINE, fmt, ap);   /* safe */
//#else
//    vsprintf(buf, fmt, ap);             /* not safe */
//#endif
    n = strlen (buf);
    if (errnoflag)
        snprintf (buf + n, MAXLINE - n, ": %s", strerror(errno_save));
    strcat (buf, "\n");

    if (options.daemon) {
        syslog (level, buf);
    } else {
        fflush (stdout);     /* in case stdout and stderr are the same */
        fputs (buf, stderr);
        fflush (stderr);
    }
    return;
}

void
err_ret (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit (1, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

void
err_sys (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    err_doit (1, LOG_ERR, fmt, ap);
    va_end (ap);
    exit (1);
}

void
err_dump (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    err_doit (1, LOG_ERR, fmt, ap);
    va_end (ap);
    abort ();        /* dump core and terminate */
    exit (1);        /* shouldn't get here */
}

void
err_msg (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    err_doit (0, LOG_INFO, fmt, ap);
    va_end (ap);
    return;
}

void
err_quit (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    err_doit (0, LOG_ERR, fmt, ap);
    va_end (ap);
    exit (1);
}
