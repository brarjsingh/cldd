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
void read_fds (server *s);

pthread_t master_thread;
pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;

struct options options;
bool running = true;

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

    /* condition to exit the main thread */
    running = false;
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
    client *c = NULL;
    GList *it, *next;
    server *s = (server *)data;

    /* set up as a tcp server */
    server_init_tcp (s);

    for (;running;)
    {
        pthread_mutex_lock (&s->data_lock);

        FD_ZERO(&s->fds);
        FD_SET(s->fd, &s->fds);

        /* loop through all possible socket connections and add
         * them to fd_set */
        it = s->client_list;
        while (it != NULL)
        {
            c = (client *)it->data;
            next = g_list_next (it);
            FD_SET(c->fd, &s->fds);
            if (c->fd > s->maxfd)
                s->maxfd = c->fd;
            it = next;
        }

        /* check for socket requests */
        nready = select (s->maxfd + 1, &s->fds, NULL, NULL, NULL);

        pthread_mutex_unlock (&s->data_lock);

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
    int ret;
    client *c = NULL;
    GList *it, *next;

    /* check if a client is trying to connect */
    ret = pthread_mutex_trylock (&s->data_lock);
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

        s->n_clients++;
        s->n_max_connected = (s->n_clients > s->n_max_connected) ?
                              s->n_clients : s->n_max_connected;

        /* add the client data to the linked list */
        s->client_list = g_list_append (s->client_list, (gpointer)c);
        CLDD_MESSAGE("Added client to list, new size: %d",
                     g_list_length (s->client_list));
        c = NULL;
    }
    pthread_mutex_unlock (&s->data_lock);

    /* go through the available connections */
    it = s->client_list;
    while (it != NULL)
    {
        c = (client *)it->data;
        next = g_list_next (it);

        /* process any request that caused an event */
        pthread_mutex_trylock (&s->data_lock);
        if (FD_ISSET(c->fd, &s->fds))
            client_process_cmd (c);
        pthread_mutex_unlock (&s->data_lock);

        /* check if the client is pending quit */
        if (c->quit)
        {
            pthread_mutex_trylock (&s->data_lock);
            s->n_clients--;
            s->client_list = g_list_delete_link (s->client_list, it);
            CLDD_MESSAGE("[%5d] Removed client from list, new size: %d",
                         c->fd, g_list_length (s->client_list));
            pthread_mutex_unlock (&s->data_lock);

            close (c->fd);
            client_free (c);
            c = NULL;
        }

        it = next;
    }
}
