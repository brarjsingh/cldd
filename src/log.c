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

#include "log.h"
#include "cmdline.h"
#include "error.h"
#include "server.h"

pthread_t       logging_thread;
pthread_cond_t  log_timer_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t log_timer_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_init (server *s, struct options *options)
{
    s->log_filename = malloc (sizeof (options->log_filename));
    strcpy (s->log_filename, options->log_filename);
    /* don't need the options string anymore, free memory */
    free (options->log_filename);
    /* doesn't account for file overwrites, fix later */
    s->logging = true;
    s->logfp = fopen (s->log_filename, "w+");
}

void setup_log_output (server *s)
{
    int ret;

    /* launch logging thread */
    ret = pthread_create (&logging_thread, NULL, logging_func, s);
    if (ret != 0)
        CLDD_ERROR("Unable to create thread for logging");
}

void close_log_files (server *s)
{
    s->logging = false;
    pthread_join (logging_thread, NULL);
    /* add error checking later */
    fclose (s->logfp);
}

void * logging_func (void *data)
{
    int ret;
    struct timespec ts;
    struct timeval tp;
    time_t start, current;
    double dt;
    struct rusage usage;
    struct rusage *rp=&usage;
    server *s = (server *)data;

    /* setup header */
    time (&start);
    fprintf (s->logfp, "time, n_clients, max_clients, user(s), user(us), sys(s), sys(us)\n");

    for (;s->logging;)
    {
        ret = gettimeofday (&tp, NULL);
        /* convert from timeval to timespec and add a 1s delay */
        /* - NOTE - for < 1s (or > 1Hz) need to calculate new nsec to check
         *          for overrun and +1s if it happens, else new < desired */
        ts.tv_sec  = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += 1;                 /* 1Hz */

        /* wait for the set time */
        pthread_mutex_lock (&log_timer_mutex);
        ret = pthread_cond_timedwait (&log_timer_cond, &log_timer_mutex, &ts);
        pthread_mutex_unlock (&log_timer_mutex);

        /* get the new usage data */
        ret = getrusage (RUSAGE_SELF, rp);
//        ret = getrusage (getppid (), rp);

        /* refresh time for logging */
        time (&current);
        dt = difftime (current, start);

        /* write new data to the log file */
        fprintf (s->logfp, "%9.3f, %9d, %9d, %16lf, %16lf, %16lf, %16lf\n",
                 dt, s->n_clients, s->n_max_connected,
                 rp->ru_utime.tv_sec, rp->ru_utime.tv_usec,
                 rp->ru_stime.tv_sec, rp->ru_stime.tv_usec);
    }

    pthread_mutex_unlock (&log_timer_mutex);

    pthread_exit (NULL);
}
