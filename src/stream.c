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

    s->guest = g_malloc (MAXLINE * sizeof (gchar));
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

static void
stream_init_tcp (struct stream_t *s)
{
    int ret, set = 1;
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

        /* prevent SIGPIPE on write to closed socket
         * TODO: check return type on these for -1 and post error */
//        setsockopt (s->fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof (int));
//        setsockopt (s->fd, SOL_SOCKET, SO_REUSEADDR, (void *)&set, sizeof (int));

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
    stream_init_tcp (s);

    /* the port is open, start listening for data */
    s->open = true;
    pthread_create (&s->task, NULL, stream_thread, (void *)s);
}

void
stream_close (struct stream_t *s)
{
    s->open = false;
    pthread_join (s->task, NULL);
    close (s->fd);
}

void *
stream_thread (void *data)
{
    int ret, n, fd;
    struct timespec ts;
    gchar buf[MAXLINE];
    struct sockaddr_storage stream_addr;
    socklen_t sin_size;
    struct stream_t *s = (struct stream_t *)data;

    sin_size = sizeof stream_addr;
    fd = accept (s->fd, (struct sockaddr *)&stream_addr, &sin_size);
    if (fd == -1)
    {
        CLDD_MESSAGE("Stream socket accept failed: %s", strerror (errno));
        s->open = false;
    }

//    set_nonblocking (fd);

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
        if ((n = writen (fd, buf, strlen (buf))) != strlen (buf))
            CLDD_MESSAGE("Client write error - %d != %d", strlen (buf), n);
    }

    pthread_mutex_unlock (&s->lock);

    pthread_exit (NULL);
}
