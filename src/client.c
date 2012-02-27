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
#include "client.h"

client *
client_new (void)
{
    client *c = malloc (sizeof (client));
    c->msg = malloc (MAXLINE * sizeof (char));
    c->msg_pending = false;

    /* create the mutex for message handling */
    if (pthread_mutex_init (&c->msg_lock, NULL) != 0)
    {
        free (c);
        return NULL;
    }

    /* create condition variables for controlling message handling */
    if (pthread_cond_init (&c->msg_ready, NULL) != 0)
    {
        free (c);
        return NULL;
    }

    return c;
}

bool
client_compare (const void * _a, const void * _b)
{
    const client *a = (const client *) _a;
    const client *b = (const client *) _b;

    if (a->fd == b->fd)
        return true;
    else
        return false;
}

void
client_free (void *c)
{
    client *_c = (client *)c;

    /* destroy the locks */
    pthread_mutex_destroy (&_c->msg_lock);

    /* destroy the condition variable */
    pthread_cond_destroy (&_c->msg_ready);

    free (_c->msg);
    free (_c);
}
