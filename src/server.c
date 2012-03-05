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
server_init_tcp (server *s)
{
    const int on = 1;
    struct sockaddr_in servaddr;

    /* create TCP socket to listen for client connections */
    CLDD_MESSAGE("Creating TCP socket");
    s->fd = socket (AF_INET, SOCK_STREAM, 0);
    if (s->fd < 0)
        CLDD_ERROR("Socket creation");

    /* set the socket to allow re-bind without wait issues */
    setsockopt (s->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));

    /* make the server socket non-blocking */
    set_nonblocking (s->fd);

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port        = htons (s->port);

    if (bind (s->fd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
        CLDD_ERROR("Failed to bind socket %ld", s->fd);

    /* setup the socket for incoming connections */
    if (listen (s->fd, BACKLOG) < 0)
        CLDD_ERROR("Unable to listen on socket %ld", s->fd);
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
