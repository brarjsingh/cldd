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

#ifndef _CLDD_CLIENT_H
#define _CLDD_CLIENT_H

#include "common.h"

BEGIN_C_DECLS

#include "cldd.h"
#include "stream.h"

extern const gchar sendbuf[MAXLINE];

typedef struct _client client;

struct _client {
    bool quit;
    int fd_mgmt;        /* socket to use for commands and management */
    int fd_chan;        /* socket to stream data to client with */
    struct sockaddr sa;
    socklen_t sa_len;
    gchar hbuf[NI_MAXHOST];
    gchar sbuf[NI_MAXSERV];
    /* for stats logging */
    int nreq;
    int ntot;
    /* stream for communicating with client */
    struct stream_t *stream;
};

/**
 * client_new
 *
 * Allocate memory for a client struct.
 *
 * @return New client data
 */
client * client_new (void);

/**
 * client_free
 *
 * Frees up the memory that was allocated for the client data.
 *
 * @param data Client to be freed from memory
 */
void client_free (gpointer data);

/**
 * client_process_cmd
 *
 * Processes the current client command that triggered an event.
 *
 * @param c The client data containing the file descriptor to write to.
 */
void client_process_cmd (client *c);

/**
 * client_compare
 *
 * Compare function for use with the ADTs for cleanup etc.
 *
 * @param _a First value for comparison
 * @param _b Second value for comparison
 * @return A true/false value depending on whether or not they are the same
 */
bool client_compare (const void * _a, const void * _b);

END_C_DECLS

#endif
