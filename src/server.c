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
#include "adt.h"
#include "client.h"
#include "error.h"
#include "server.h"
#include "stream.h"

server *
server_new (void)
{
    server *s = malloc (sizeof (server));
    if (s == NULL)
        CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);

    s->pid = getpid ();
    s->n_clients = 0;
    s->n_max_connected = 0;

    /* create the mutexes for controlling access to thread data */
    if (pthread_mutex_init (&s->data_lock, NULL) != 0)
    {
        free (s);
        return NULL;
    }

    return s;
}

void
server_free (server *s)
{
    /* clear the data for the collections */
    g_list_free (s->client_list);

    /* destroy the locks */
    pthread_mutex_destroy (&s->data_lock);

    /* string should be guaranteed to contain a value */
    free (s->log_filename);

    /* finally clean up the server allocation */
    free (s);
}

void
server_init_tcp (server *s)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char port[6];
    int ret;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* all interfaces */

    sprintf (port, "%d", s->port);
    ret = getaddrinfo (NULL, port, &hints, &result);
    if (ret != 0)
        CLDD_ERROR("getaddrinfo: %s", gai_strerror (ret));

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        s->fd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (s->fd == -1)
            continue;

        ret = bind (s->fd, rp->ai_addr, rp->ai_addrlen);
        if (ret == 0)
            /* managed to bind successfully */
            break;

        close (s->fd);
    }

    if (rp == NULL)
        CLDD_ERROR("Unable to bind socket");

    /* make the server socket non-blocking */
    set_nonblocking (s->fd);

    freeaddrinfo (result);

    /* setup the socket for incoming connections */
    if (listen (s->fd, BACKLOG) < 0)
        CLDD_ERROR("Unable to listen on socket %ld", s->fd);
}

void
server_init_epoll (server *s)
{
    /* create the epoll file descriptor */
    s->epoll_fd = epoll_create (EPOLL_QUEUE_LEN);
    if (s->epoll_fd == -1)
        CLDD_ERROR("epoll_create() error");

    /* Add the server socket to the epoll event loop */
    s->event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    s->event.data.fd = s->fd;
    if (epoll_ctl (s->epoll_fd, EPOLL_CTL_ADD, s->fd, &s->event) == -1)
        CLDD_ERROR("epoll_ctl() error");
}

void
server_add_client (server *s, client *c)
{
    /* update counters */
    s->n_clients++;
    s->n_max_connected = (s->n_clients > s->n_max_connected)
                        ? s->n_clients : s->n_max_connected;

    /* add the new socket descriptor to the epoll loop */
    s->event.data.fd = c->fd_mgmt;
    if (epoll_ctl (s->epoll_fd, EPOLL_CTL_ADD, c->fd_mgmt, &s->event) == -1)
        CLDD_ERROR("epoll_ctl() error");

    /* add the client data to the linked list */
    s->client_list = g_list_append (s->client_list, (gpointer)c);
    CLDD_MESSAGE("Added client to list, new size: %d",
                 g_list_length (s->client_list));
}

int
server_connect_client (server *s, client *c)
{
    int ret;

    c->fd_mgmt = accept (s->fd, (struct sockaddr *)&c->sa, &c->sa_len);
    if (c->fd_mgmt == -1)
    {
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            return 1;
        else
            return -1;
    }

    ret = getnameinfo (&c->sa, c->sa_len,
                        c->hbuf, sizeof (c->hbuf),
                        c->sbuf, sizeof (c->sbuf),
                        NI_NUMERICHOST | NI_NUMERICSERV);

    /* make the new fd non-blocking */
    set_nonblocking (c->fd_mgmt);

    /* setup stream properties */
    c->stream->port = server_next_stream_port (s);
    c->stream->guest = g_strdup_printf ("%s", c->hbuf);

    return 0;
}

int
server_next_stream_port (server *s)
{
    int i;
    bool found = false;
    client *c = NULL;
    GList *it, *next;

    /* 10000 is ridiculous and not what this server is intended to handle */
    for (i = STREAM_PORT_BASE; i < STREAM_PORT_BASE+10000; i++)
    {
        CLDD_MESSAGE("Checking for port %d availability", i);

        /* go through the available connections */
        it = s->client_list;
        while (it != NULL)
        {
            c = (client *)it->data;
            next = g_list_next (it);
            if (c->stream->port == i)
            {
                found = true;
                break;
            }
            it = next;
        }

        if (!found)
        {
            CLDD_MESSAGE("Using port %d for new client stream", i);
            return i;
        }
    }

    return -1;
}

