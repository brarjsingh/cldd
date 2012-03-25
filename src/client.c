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
#include "stream.h"

/* this was just for testing */
const gchar sendbuf[MAXLINE] =
        "012345678901234567890123456789012345678901234567890123456789012\n";

client *
client_new (void)
{
    client *c = g_malloc (sizeof (client));

    c->ntot = 0;
    c->nreq = 0;
    c->quit = false;
    c->stream = stream_new ();

    return c;
}

void
client_free (gpointer data)
{
    client *c = (client *)data;
    stream_free (c->stream);
    g_free (c);
}

void
client_process_cmd (client *c)
{
    ssize_t n;
    gchar *recv, *buf;

    recv = g_malloc (MAXLINE * sizeof (gchar));

    n = readline (c->fd_mgmt, recv, MAXLINE);
    if (n == 0)
        return;

    /* remove trailing whitespace or newline characters */
    recv = g_strchomp (recv);

    /* test for the different command options */

    /* not sure if this one's useful yet */
    if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_ACK])))
        CLDD_MESSAGE("Client ACK received");
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_SCH])))
        CLDD_MESSAGE("Client SCH received");
    /* client disconnected */
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_DIS])))
    {
        CLDD_MESSAGE("Client DIS received");
        c->quit = true;
    }
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_PNG])))
    {
        CLDD_MESSAGE("Client PNG received");
        /* ping received, send pong */
        buf = g_strdup_printf ("%s\n", cmds[CMD_PNG].str);
        n = strlen (buf);
        if (writen (c->fd_mgmt, buf, n) != n)
            CLDD_MESSAGE("Client write error: PNG/ACK");
        g_free (buf);
    }
    /* client requested stream setup information */
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_SSU])))
    {
        CLDD_MESSAGE("Client SSU received");
        buf = g_strdup_printf ("port:%d\n", c->stream->port);
        n = strlen (buf);
        if (writen (c->fd_mgmt, buf, n) != n)
            CLDD_MESSAGE("Client write error: SSU");
        g_free (buf);
    }
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_AO])))
        CLDD_MESSAGE("Client AO received");
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_AI])))
        CLDD_MESSAGE("Client AI received");
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_DO])))
        CLDD_MESSAGE("Client DO received");
    else if (g_str_has_prefix (recv, cldd_cmd_prefix (&cmds[CMD_DI])))
        CLDD_MESSAGE("Client DI received");

    c->ntot += n;
    c->nreq++;

    g_free (recv);
}

bool
client_compare (const void * _a, const void * _b)
{
    const client *a = (const client *) _a;
    const client *b = (const client *) _b;

    /* there should never be two clients with the same socket descriptor */
    if (a->fd_mgmt == b->fd_mgmt)
        return true;
    else
        return false;
}
