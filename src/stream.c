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
#include "error.h"
#include "stream.h"

struct stream_t *
stream_new (void)
{
    struct stream_t *s = g_malloc (sizeof (struct stream_t));

    pthread_mutex_init (&s->lock, NULL);
    pthread_cond_init (&s->cond, NULL);

    return s;
}

void
stream_free (struct stream_t *s)
{
    pthread_mutex_destroy (&s->lock);
    pthread_cond_destroy (&s->cond);
    free (s->guest);
    free (s);
}

void
stream_open (struct stream_t *s)
{
    int n, rv;
    gchar *strport;
    struct addrinfo hints, *servinfo, *p;

    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    /* need the port number as a string for getaddrinfo */
    strport = g_strdup_printf ("%d", s->port);

    if ((rv = getaddrinfo (s->guest, strport, &hints, &servinfo)) != 0)
    {
        g_fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
        return;
    }

    /* loop through all the results and bind to the first we can */
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((s->sd = socket (p->ai_family,
                             p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror ("output stream: socket");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        g_fprintf (stderr, "output stream: failed to bind socket\n");
        return;
    }

    freeaddrinfo (servinfo);

    /* the port is open, start listening for data */
    s->open = true;
    pthread_create (&s->task, NULL, stream_thread, (void *)s);
}

void
stream_close (struct stream_t *s)
{
    s->open = false;
    pthread_join (s->task, NULL);
    close (s->sd);
}

void *
stream_thread (void *data)
{
    int ret, n;
    struct timespec ts;
    gchar buf[MAXLINE];
    struct stream_t *s = (struct stream_t *)data;

    for (;s->open;)
    {
        pthread_mutex_lock (&s->lock);

        /* setup timer for 10Hz */
        clock_gettime (CLOCK_REALTIME, &ts);
        ts.tv_sec += 0;
        ts.tv_nsec += 100000000;
        if (ts.tv_nsec >= 1000000000)
        {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        /* wait for the set time */
        ret = pthread_cond_timedwait (&s->cond, &s->lock, &ts);
        if ((ret != 0) && (ret != ETIMEDOUT))
        {
            CLDD_MESSAGE("pthread_cond_timedwait() returned %d\n", ret);
            pthread_mutex_unlock (&s->lock);
            break;
        }
        pthread_mutex_unlock (&s->lock);

        /* transmit current data */
        g_snprintf (buf, MAXLINE, "$12:00:00.000&0|0.000,1|0.000,2|0.000\n");
        if ((n = writen (s->sd, buf, strlen (buf))) != strlen (buf))
            CLDD_MESSAGE("Client write error - %d != %d", strlen (buf), n);
    }

    pthread_mutex_unlock (&s->lock);

    pthread_exit (NULL);
}
