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
#include "error.h"
#include "server.h"

server *
server_new (void)
{
    server *s = malloc (sizeof (server));
    if (s == NULL)
        CLDD_MESSAGE("LINE: %d, malloc() failed\n", __LINE__);

    s->n_clients = 0;
    s->n_max_connected = 0;

    /* create the collections for client management */
    s->spawn_queue = queue_new ();
    s->client_list = llist_new ();

    /* create the mutexes for controlling access to thread data */
    if ((pthread_mutex_init (&s->spawn_queue_lock, NULL) != 0) ||
        (pthread_mutex_init (&s->server_data_lock, NULL) != 0))
    {
        free (s);
        return NULL;
    }

    /* create condition variables for controlling thread synchronization */
    if (pthread_cond_init (&s->spawn_queue_ready, NULL) != 0)
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
    queue_free (s->spawn_queue);
    llist_free (s->client_list);

    /* destroy the locks */
    pthread_mutex_destroy (&s->spawn_queue_lock);
    pthread_mutex_destroy (&s->server_data_lock);

    /* destroy the condition variable */
    pthread_cond_destroy (&s->spawn_queue_ready);

    /* string should be guaranteed to contain a value */
    free (s->log_filename);

    /* finally clean up the server allocation */
    free (s);
}
