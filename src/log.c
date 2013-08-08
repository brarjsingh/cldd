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

//#include <glibtop.h>
//#include <glibtop/cpu.h>
//#include <glibtop/mem.h>
//#include <glibtop/proclist.h>

#include "log.h"
#include "cmdline.h"
#include "error.h"
#include "server.h"

GThread *log_task;

struct log_t *
log_new (void)
{
    struct log_t *log = malloc (sizeof (struct log_t));

    log->active = false;
    log->reload = false;

    return log;
}

void
log_free (struct log_t *log)
{
    log->active = false;
    /* TODO: join the log thread here */

    free (log);
    log = NULL;
}

void *
log_thread (gpointer data)
{
    g_thread_exit (NULL);
}

int
log_start (struct log_t *log)
{
    return 0;
}

int
log_pause (struct log_t *log)
{
    return 0;
}

int
log_stop (struct log_t *log)
{
    return 0;
}

void
stat_log_new (server *s, struct options *options)
{
    char stats_filename[80];

    /* should create a log struct and do this in a new func */
//    glibtop_init ();

    s->log_filename = malloc (sizeof (options->log_filename));
    strcpy (s->log_filename, options->log_filename);

    /* don't need the options string anymore, free memory */
    free (options->log_filename);

    /* these don't account for file overwrites, fix later */
    s->logging = true;

    /* this should pull from the cldd configuration file which doesn't exist */
    s->logfp = fopen (s->log_filename, "w");
}

void
stat_log_free (struct stat_log_t *log)
{
    free (log);
    log = NULL;
}

void
stat_log_setup_output (server *s)
{
    int ret;
    GError *error;

    /* launch logging thread */
    log_task = g_thread_create ((GThreadFunc)log_thread,
                               (gpointer)s, true, &error);
}

void
stat_log_file_close (server *s)
{
    s->logging = false;
    g_thread_join (log_task);
    fclose (s->logfp);
}

void *
stat_log_thread (gpointer data)
{
    int i, ret, delay;
    unsigned long n_bytes, n_bytes_old;
    struct timespec ts;
    time_t start, current;
    double dt;
    char buf[PATH_MAX];
    client *c = NULL;
    GList *it, *next;
    server *s = (server *)data;

    static GStaticMutex lock = G_STATIC_MUTEX_INIT;
    GCond *cond = g_cond_new ();
    GTimeVal next_time;

    /* new stuff for glibtop */
//    glibtop_cpu cpu;
//    glibtop_mem memory;
//    glibtop_proclist proclist;

    /* setup header */
    time (&start);
    fprintf (s->logfp, "time, n_clients, max_clients, cpu_tot, cpu_user, "
                       "cpu_nice, cpu_sys, cpu_idle, cpu_freq, mem_tot (MB), "
                       "mem_used (MB), mem_free (MB), mem_buffered (MB), "
                       "mem_cached (MB), mem_user (MB), mem_locked (MB), "
                       "tot_sent (Bytes), tx_rate_avg (kBps)\n");

    /* for baudrate calc */
    i = 1;
    n_bytes = n_bytes_old = 0;

    /* 10Hz timer setup (in usec) */
    delay = 1e6/10;
    g_get_current_time (&next_time);

    for (;s->logging;)
    {
        /* refresh time for logging */
        time (&current);
        dt = difftime (current, start);

//        glibtop_get_cpu (&cpu);
//        glibtop_get_mem (&memory);

        /* calculate the new transmission rate - 10 times through the loop
         * is a fairly accurate calculation for 1 second, not perfect though */
        if (i == 10)
        {
            i = 1;
            n_bytes_old = n_bytes;
            g_mutex_trylock (&s->data_lock);
            it = s->client_list;
            /* go through the list of clients and add up stream byte count */
            while (it != NULL)
            {
                c = (client *)it->data;
                next = g_list_next (it);
                s->b_sent += c->stream->b_sent;
                it = next;
            }
            n_bytes = s->b_sent;
            /* no time component in the calc since this is done over 1 sec */
            s->tx_rate = (double)(n_bytes - n_bytes_old) / 1024;
            g_mutex_unlock (&s->data_lock);
        }

        /* write the next line */
        /*
        fprintf (s->logfp, "%.3f, %d, %d, %ld, %ld, %ld, %ld, %ld, %ld, %ld, "
                           "%ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %.3f\n",
                 dt, s->n_clients, s->n_max_connected,
                 (unsigned long)cpu.total,
                 (unsigned long)cpu.user,
                 (unsigned long)cpu.nice,
                 (unsigned long)cpu.sys,
                 (unsigned long)cpu.idle,
                 (unsigned long)cpu.frequency,
                 (unsigned long)memory.total/(1024*1024),
                 (unsigned long)memory.used/(1024*1024),
                 (unsigned long)memory.free/(1024*1024),
                 (unsigned long)memory.shared/(1024*1024),
                 (unsigned long)memory.buffer/(1024*1024),
                 (unsigned long)memory.cached/(1024*1024),
                 (unsigned long)memory.user/(1024*1024),
                 (unsigned long)memory.locked/(1024*1024),
                 s->b_sent, s->tx_rate);
        */

        /* add another time delay */
        g_time_val_add (&next_time, delay);
        g_static_mutex_lock (&lock);
        while (g_cond_timed_wait (cond,
                                  g_static_mutex_get_mutex (&lock),
                                  &next_time))
            ;   /* do nothing */
        g_static_mutex_unlock (&lock);

        i++;
    }

    g_cond_free (cond);
    g_thread_exit (NULL);
}
