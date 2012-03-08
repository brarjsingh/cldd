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

#include "cldd.h"
#include "client.h"

typedef struct _server server;

struct _server {
    int fd;
    int port;
    int n_clients;
    int n_max_connected;
    pid_t pid;
    bool running;
    /* for epoll on client connections */
    int epoll_fd;
    int num_fds;
    struct epoll_event events[EPOLL_QUEUE_LEN];
    struct epoll_event event;
    /* client management */
    GList *client_list;
    /* data for threading */
    pthread_mutex_t data_lock;
    /* performance logging */
    bool logging;
    FILE *statsfp;
    FILE *logfp;
    char *log_filename;
};

server * server_new (void);
void server_init_tcp (server *s);
void server_init_epoll (server *s);
void server_add_client (server *s, client *c);
void server_free (server *s);

END_C_DECLS

#endif
