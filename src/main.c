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
#include "stream.h"
#include "utils.h"

/* function prototypes */
void signal_handler (int sig);
void * client_manager (void *data);
void process_events (server *s);

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
        case SIGINT:
            syslog (LOG_WARNING, "Received SIGINT signal.");
            break;
        default:
            syslog (LOG_WARNING, "Unhandled signal (%d) %s", strsignal(sig));
            break;
    }

    /* condition to exit the main thread */
    running = false;
    pthread_join (master_thread, NULL);
//    pthread_cancel (master_thread);
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

    /* set the server up to use epoll */
    server_init_epoll (s);

    for (;running;)
    {
        pthread_mutex_lock (&s->data_lock);
        s->num_fds = epoll_wait (s->epoll_fd, s->events, EPOLL_QUEUE_LEN, -1);
        pthread_mutex_unlock (&s->data_lock);

        if (s->num_fds < 0)
            CLDD_ERROR("Error while epoll_wait()");
        else if (s->num_fds == 0)
            continue;
        else
            /* data is available on one or more sockets */
            process_events (s);
    }

    CLDD_MESSAGE("Exiting the client manager");
    close (s->fd);

    pthread_exit (NULL);
}

/**
 * process_events
 *
 * Process edge triggered events seen by epoll.
 *
 * @param s The server data containing the epoll events to handle
 */
void
process_events (server *s)
{
    int i, ret;
    client *c = NULL;
    GList *it, *next;

    for (i = 0; i < s->num_fds; i++)
    {
        /* error */
        if (s->events[i].events & (EPOLLHUP | EPOLLERR))
        {
            CLDD_MESSAGE("epoll: EPOLLERR");
            close(s->events[i].data.fd);
            continue;
        }
        assert (s->events[i].events & EPOLLIN);

        /* notification on listening socket indicating one or more
         * incoming connections */
        if (s->events[i].data.fd == s->fd)
        {
            CLDD_MESSAGE("Client connection requested");
            while (true)
            {
                /* create new client data */
                c = client_new ();
                c->sa_len = sizeof (c->sa);

                ret = server_connect_client (s, c);
                if (ret == -1)
                    CLDD_ERROR("Client connection failed");
                else if (ret == 1)
                    /* all incoming connections have been processed */
                    break;
                else
                    CLDD_MESSAGE("Received connection on descriptor %d (%s:%s)",
                                 c->fd_mgmt, c->hbuf, c->sbuf);

                /* add a streaming output socket to the client */
                stream_open (c->stream);

                /* add the new client to the server */
                ret = pthread_mutex_lock (&s->data_lock);
                server_add_client (s, c);
                pthread_mutex_unlock (&s->data_lock);
            }

            continue;
        }
        else
        {
            /* go through the available connections */
            it = s->client_list;
            while (it != NULL)
            {
                c = (client *)it->data;
                next = g_list_next (it);
                if (c->fd_mgmt == s->events[i].data.fd)
                    break;
                it = next;
            }

            /* process any request that caused an event */
            client_process_cmd (c);

            /* check if the client is pending quit */
            if (c->quit)
            {
                pthread_mutex_trylock (&s->data_lock);
                s->n_clients--;
                s->client_list = g_list_delete_link (s->client_list, it);
                CLDD_MESSAGE("Removed client from list, new size: %d",
                             g_list_length (s->client_list));
                /* log the client stats before closing it */
                fprintf (s->statsfp, "%s, %d, %d, %d\n",
                         c->hbuf, c->fd_mgmt, c->nreq, c->ntot);
                pthread_mutex_unlock (&s->data_lock);

                stream_close (c->stream);

                close (c->fd_mgmt);
                client_free (c);
                c = NULL;
            }
        }
    }
}
