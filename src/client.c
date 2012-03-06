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
#include "error.h"

const char sendbuf[MAXLINE] =
        "012345678901234567890123456789012345678901234567890123456789012\n";

client *
client_new (void)
{
    client *c = malloc (sizeof (client));
    c->ntot = 0;
    c->nreq = 0;
    c->quit = false;

    return c;
}

/**
 * client_process_cmd
 *
 * Processes the current client command that triggered an event.
 *
 * @param c The client data containing the file descriptor to write to.
 */
void
client_process_cmd (client *c)
{
    ssize_t n;
    char *recv;

    recv = malloc (MAXLINE * sizeof (char));

    n = readline (c->fd, recv, MAXLINE);
    if (n == 0)
        return;

    /* a request message received from the client triggers a write */
    if (strcmp (recv, "request\n") == 0)
    {
        if ((n = writen (c->fd, sendbuf, strlen (sendbuf))) != strlen (sendbuf))
            CLDD_MESSAGE("Client write error - %d != %d", strlen (sendbuf), n);
    }
    else if (strcmp (recv, "quit\n") == 0)
        c->quit = true;

    c->ntot += n;
    c->nreq++;
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
client_free (gpointer data)
{
    client *c = (client *)data;
    free (c);
}
