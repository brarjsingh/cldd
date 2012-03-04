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

#ifndef _CLDD_SERVER_H
#define _CLDD_SERVER_H

#include "common.h"

BEGIN_C_DECLS

#include "adt.h"

typedef struct _server server;

struct _server {
    int fd;
    int port;
    int n_clients;
    int n_max_connected;
    pid_t pid;
    bool running;
    /* for select on client connections */
    fd_set fds;
    int maxfd;
    /* client management */
    llist *client_list;
    /* data for threading */
    pthread_mutex_t data_lock;
    /* performance logging */
    bool logging;
    FILE *logfp;
    char *log_filename;
};

server * server_new (void);
void server_init_tcp (server *s);
void server_free (server *s);

END_C_DECLS

#endif
