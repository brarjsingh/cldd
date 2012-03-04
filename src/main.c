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
#include "cmdline.h"
#include "conf.h"
#include "daemon.h"
#include "error.h"
#include "log.h"
#include "server.h"
#include "utils.h"

/* function prototypes */
void signal_handler (int sig);
void * client_manager (void *data);
void * client_func (void *data);
void read_fds (server *s);

pthread_t master_thread;
pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;

struct options options;

static void
glue_daemonize_init (const struct options *options)
{
    daemonize_init ("root", "root", PID_FILE);
    if (options->kill)
        daemonize_kill ();
}

int
main (int argc, char **argv)
{
    int ret;
    bool success;
    server *s;

    if (argc == 1)
        usage (argv);
    success = parse_cmdline (argc, argv, &options);
    if (!success)
        CLDD_ERROR("Error while parsing command line arguments\n");

    /* setup signal handling first */
    signal (SIGHUP,  signal_handler);
    signal (SIGINT,  signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGQUIT, signal_handler);

    /* allocate memory for server data */
    s = server_new ();
    s->port = options.port;

    daemonize_close_stdin ();

    glue_daemonize_init (&options);
    log_init (s, &options);

    daemonize_set_user ();

    /* passing true starts daemon in detached mode */
    daemonize (options.daemon);
    setup_log_output (s);

    /* start the master thread for client management */
    ret = pthread_create (&master_thread, NULL, client_manager, s);
    if (ret != 0)
        CLDD_ERROR("Unable to create client management thread");
    pthread_join (master_thread, NULL);

    daemonize_finish ();
    close_log_files (s);

    /* clean up */
    server_free (s);

    return EXIT_SUCCESS;
}

/**
 * signal_handler
 *
 * Use syslog for now, when logging facilities are available should use those
 * instead.
 *
 * @param sig The signal received
 */
void
signal_handler (int sig)
{
    /* do something useful here later */
    switch (sig)
    {
        case SIGHUP:
            syslog (LOG_WARNING, "Received SIGHUP signal.");
            break;
        case SIGTERM:
            syslog (LOG_WARNING, "Received SIGTERM signal.");
            break;
        default:
            syslog (LOG_WARNING, "Unhandled signal (%d) %s", strsignal(sig));
            break;
    }

    /* stop the thread responsible for client management */
    pthread_cancel (master_thread);
}

/**
 * client_manager
 *
 * Thread function to manage client connections.
 *
 * @param data Thread data for the function
 */
void *
client_manager (void *data)
{
    int ret, nready;
    const int on = 1;
    struct sockaddr_in servaddr;
    client *c = NULL;
    struct client_data_t cd;
    struct client_data_t *pcd;

    server *s = (server *)data;

    ///
    node *n = NULL;
    ///

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

    /* right now the listening socket is the max */
    s->maxfd = s->fd;

    for (;;)
    {
        ///build_select_list
        pthread_mutex_lock (&s->server_data_lock);

        FD_ZERO(&s->fds);
        FD_SET(s->fd, &s->fds);

        /* loop through all possible socket connections and add
         * them to fd_set */
        for (n = s->client_list->link; n != NULL; n = n->next)
        {
            c = (client *)n->data;
            FD_SET(c->fd, &s->fds);
            if (c->fd > s->maxfd)
                s->maxfd = c->fd;
        }
        ///

        /* check for socket requests */
        nready = select (s->maxfd + 1, &s->fds, NULL, NULL, NULL);

        pthread_mutex_unlock (&s->server_data_lock);

        if (nready < 0)
        {
            /* possible error */
            if (errno == EINTR)
                continue;
            else
                CLDD_ERROR("select() error");
        }
        else if (nready == 0)
        {
            /* nothing to read, keep going */
            continue;
        }
        else
        {
            /* data is available on one or more sockets */
            read_fds (s);
        }

        c = NULL;
    }

    CLDD_MESSAGE("Exiting the client manager");

    pthread_exit (NULL);
}

/**
 *
 */
void
read_fds (server *s)
{
    client *c = NULL;
    node *n = NULL;
    struct client_data_t cd;
    struct client_data_t *pcd;

    /* check if a client is trying to connect */
    if (FD_ISSET(s->fd, &s->fds))
    {
        /* get the client data ready */
        c = client_new ();
        c->sa_len = sizeof (c->sa);

        /* blocking call waiting for connections */
        c->fd = accept (s->fd, (struct sockaddr *) &c->sa, &c->sa_len);
        set_nonblocking (c->fd);
        CLDD_MESSAGE("Received connection from (%s, %d)",
                     inet_ntoa (c->sa.sin_addr),
                     ntohs (c->sa.sin_port));

        pthread_mutex_lock (&s->server_data_lock);
        s->n_clients++;
        s->n_max_connected = (s->n_clients > s->n_max_connected) ?
                              s->n_clients : s->n_max_connected;
        pthread_mutex_unlock (&s->server_data_lock);

        /* data for the client thread */
        cd.s = s;
        cd.c = c;
        pcd = &cd;

        /* launch the thread for the client */
        pthread_mutex_lock (&c->lock);
        pthread_create (&c->tid, NULL, client_func, pcd);
        CLDD_MESSAGE("Created client thread %ld", c->tid);

        /* add the client data to the linked list */
        pthread_mutex_lock (&s->server_data_lock);
        s->client_list = llist_append (s->client_list, (void *)c);
        CLDD_MESSAGE("Added client to list, new size: %d",
                     llist_length (s->client_list));
        pthread_mutex_unlock (&s->server_data_lock);

        c = NULL;
    }

    /* go through the available connections */
    for (n = s->client_list->link; n != NULL; n = n->next)
    {
        pthread_mutex_lock (&s->server_data_lock);
        c = (client *)n->data;
        if (FD_ISSET(c->fd, &s->fds))
        {
            /* unlock client thread condition variable */
            CLDD_MESSAGE("received cmd on %d", c->fd);
            pthread_mutex_lock (&c->lock);
            c->data_ready = true;
            pthread_cond_signal (&c->ready);
            pthread_mutex_unlock (&c->lock);
        }
        pthread_mutex_unlock (&s->server_data_lock);
    }
}

/**
 * client_func
 *
 * Function to test client connections.
 *
 * @param data The data containing the client and server references
 **/
void *
client_func (void *data)
{
    ssize_t n;
    char *recv;
    /* this is hokey and only for the test, but a 64kB block will be sent
     * to the client 16 times making 1kB sent in total */
    char send[MAXLINE] =
        "012345678901234567890123456789012345678901234567890123456789012\n";
    client *c = (client *)((struct client_data_t *)data)->c;
    server *s = (server *)((struct client_data_t *)data)->s;

    recv = malloc (MAXLINE * sizeof (char));

    /* send ready command to client */
    if ((n = writen (c->fd, "ready\n", 6)) != 6)
        CLDD_MESSAGE("Client write error - %d != %d", n, 6);

    CLDD_MESSAGE("[%5d] Entering client loop", c->fd);
    for (;;)
    {
        /* wait for server event to trigger the client */
        CLDD_MESSAGE("[%5d] Wait on client", c->fd);
        pthread_mutex_lock (&c->lock);
        while (!c->data_ready)
            pthread_cond_wait (&c->ready, &c->lock);
        pthread_mutex_unlock (&c->lock);
        CLDD_MESSAGE("[%5d] Continue on client", c->fd);

        n = readline (c->fd, recv, MAXLINE);
        if (n == 0)
            break;

        /* a request message received from the client triggers a write */
        if (strcmp (recv, "request\n") == 0)
        {
            if ((n = writen (c->fd, send, strlen (send))) != strlen (send))
                CLDD_MESSAGE("Client write error - %d != %d", strlen (send), n);
        }
        else if (strcmp (recv, "quit\n") == 0)
        {
            break;
        }
    }
    CLDD_MESSAGE("[%5d] Leaving client loop", c->fd);

    pthread_mutex_lock (&s->server_data_lock);
    s->n_clients--;
    s->client_list = llist_remove (s->client_list, (void *)c, client_compare);
    CLDD_MESSAGE("[%5d] Removed client from list, new size: %d",
                 c->fd, llist_length (s->client_list));
    pthread_mutex_unlock (&s->server_data_lock);

    close (c->fd);
    client_free (c);
    c = NULL;

    free (recv);
    pthread_exit (NULL);
}
