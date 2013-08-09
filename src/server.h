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

/**
 * Server data used for setup and client communications.
 */
struct _server {
    /*@{*/
    int fd;                 /**< file descriptor to listen on */
    int port;               /**< port number to open for server */
    pid_t pid;              /**< pid of daemon process */
    bool running;           /**< flag used to stop process */
    bool rt;                /**< flag to indicate that real-time was enabled */
    GList *client_list;     /**< client management */
    GMutex data_lock;       /**< data for threading */
    /*@}*/
    /**
     * edge triggering client connections
     */
    /*@{*/
    int epoll_fd;           /**< file descriptor that an event was seen on */
    int num_fds;            /**< number of file descriptors that saw an event */
    struct epoll_event events[EPOLL_QUEUE_LEN]; /**< list of events */
    struct epoll_event event;                   /**< epoll data for setup */
    /*@}*/
    /**
     * data collection and hardware control
     */
    CldBuilder *c_builder;
    CldXmlConfig *c_xml;
    struct daq_t *daq;
    struct log_t *log;
    /*@{*/
    /*@}*/
    /**
     * performance logging
     */
    /*@{*/
    int n_clients;          /**< number of clients currently connected */
    int n_max_connected;    /**< peak connection count seen by server */
    unsigned long b_sent;   /**< total number of bytes on all streams */
    double tx_rate;         /**< average data rate in kbps for server streams */
    bool logging;           /**< flag to control logging */
    FILE *statsfp;          /**< file to output stats to */
    FILE *logfp;            /**< file to output general server info to */
    char *log_filename;     /**< filename to use for general server info file */
    /*@}*/
};

/**
 * Allocates memory for a new server struct.
 *
 * @return Allocated data for a new server struct
 */
server * server_new (void);

/**
 * Allocates memory for a new server struct.
 *
 * @param  options Command line data to use to create the server data with
 * @return Allocated data for a new server struct
 */
server * server_new_from_options (struct options *so);

/**
 * Frees any memory that was allocated for a server struct.
 *
 * @param s The server data to free up
 */
void server_free (server *s);

/**
 * Initializes the TCP socket used for client command communications.
 *
 * @param s Server data containing the file descriptor and related variables
 *          needed to setup TCP communications
 */
void server_init_tcp (server *s);

/**
 * Initializes epoll edge triggering to handle input events received
 * from clients.
 *
 * @param s Server data containing epoll data needed to enable edge triggering
 */
void server_init_epoll (server *s);

/**
 * Adds a newly allocated client to the list in the server data.
 *
 * @param s Server data containing the list of clients to add to
 * @param c Client data to add to the server list
 */
void server_add_client (server *s, client *c);

/**
 * Perform the client connection setup.
 *
 * @param s Server data to connect client to
 * @param c Client to connect
 * @return 0 on success, 1 if there's no connection request, -1 on error
 */
int server_connect_client (server *s, client *c);

/**
 * Iterates through the list of clients and closes them and any streams they
 * have.
 *
 * @param s Server data containing the list of clients to close
 */
void server_close_clients (server *s);

/**
 * Iterate through the list of clients and get the next available port number
 * to use to setup a streaming socket.
 *
 * @param s Server data containing the list of clients to iterate through and
 *          find the next available port number
 * @return The next available port number that the server can use for a stream
 */
int server_next_stream_port (server *s);

END_C_DECLS

#endif
