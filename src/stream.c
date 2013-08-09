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
#include "stream.h"

struct stream_t *
stream_new (void)
{
    struct stream_t *s = g_malloc (sizeof (struct stream_t));
    s->guest = g_malloc (MAXLINE * sizeof (gchar));
    s->b_sent = 0;
    return s;
}

void
stream_free (struct stream_t *s)
{
    free (s->guest);
    free (s);
}

static void
stream_init_tcp (struct stream_t *s)
{
    int ret, yes = 1;
    char port[6];
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* all interfaces */

    /* FIXME: change to snprintf */
    sprintf (port, "%d", s->port);
    ret = getaddrinfo (NULL, port, &hints, &result);
    if (ret != 0)
        CLDD_ERROR("getaddrinfo: %s", gai_strerror (ret));

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        s->fd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s->fd == -1)
            continue;

        if (setsockopt (s->fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                        sizeof(int)) == -1)
            CLDD_MESSAGE("Failed to set socket option SO_REUSEADDR");

        ret = bind (s->fd, rp->ai_addr, rp->ai_addrlen);
        if (ret == 0)
            /* managed to bind successfully */
            break;

        close (s->fd);
    }

    if (rp == NULL)
        CLDD_ERROR("Unable to bind socket");

    /* make the streaming socket non-blocking */
//    set_nonblocking (s->fd);

    freeaddrinfo (result);

    /* setup the socket for incoming connections */
    if (listen (s->fd, BACKLOG) < 0)
        CLDD_ERROR("Unable to listen on socket %ld", s->fd);
}

void
stream_open (struct stream_t *s)
{
    GError *error;

    stream_init_tcp (s);

    /* the port is open, start listening for data */
    s->open = true;
    s->task = g_thread_create ((GThreadFunc)stream_thread,
                               (gpointer)s, true, &error);

}

void
stream_close (struct stream_t *s)
{
    s->open = false;
    g_thread_join (s->task);
    close (s->fd);
}

void *
stream_thread (gpointer data)
{
    struct stream_t *s = (struct stream_t *)data;

    int ret, n, fd, delay;
    gchar buf[MAXLINE];
    struct sockaddr_storage stream_addr;
    socklen_t sin_size;

    static GStaticMutex lock = G_STATIC_MUTEX_INIT;
    GCond *cond = g_cond_new ();
    GTimeVal next_time;

    sin_size = sizeof stream_addr;
    fd = accept (s->fd, (struct sockaddr *)&stream_addr, &sin_size);
    if (fd == -1)
    {
        CLDD_MESSAGE("Stream socket accept failed: %s", strerror (errno));
        s->open = false;
    }

//    set_nonblocking (fd);

    /* 10Hz timer setup (in usec) */
    delay = 1e6/1000;
    g_get_current_time (&next_time);

    while (s->open)
    {
        /* transmit current data - typical stream sample of 32 channels used */
        g_snprintf (buf, MAXLINE, "$12:00:00.000&"
                "0|0.000,1|0.000,2|0.000,3|0.000,4|0.000,5|0.000,6|0.000,7|0.000,"
                "8|0.000,9|0.000,10|0.000,11|0.000,12|0.000,13|0.000,14|0.000,15|0.000,"
                "16|0.000,17|0.000,18|0.000,19|0.000,20|0.000,21|0.000,21|0.000,23|0.000,"
                "24|0.000,25|0.000,26|0.000,27|0.000,28|0.000,29|0.000,30|0.000,31|0.000\n");

        if ((n = writen (fd, buf, strlen (buf))) != strlen (buf))
            CLDD_MESSAGE("Client write error: %s", strerror (errno));

        if (errno == ECONNRESET)
            s->open = false;

        /* add to the stream total byte count */
        s->b_sent += n;

        /* add another time delay */
        g_time_val_add (&next_time, delay);
        g_static_mutex_lock (&lock);
        while (g_cond_timed_wait (cond,
                                  g_static_mutex_get_mutex (&lock),
                                  &next_time))
            ;   /* do nothing */
        g_static_mutex_unlock (&lock);
    }

    g_cond_free (cond);
    g_thread_exit (NULL);
}
