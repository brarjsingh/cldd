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
void * client_spawn_handler (void *data);
void * client_queue_handler (void *data);

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
        case SIGINT:
            syslog (LOG_WARNING, "Received SIGINT signal.");
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
    int ret;
    const int on = 1;
    struct sockaddr_in servaddr;

    server *s = (server *)data;

    /* create TCP socket to listen for client connections */
    CLDD_MESSAGE("Creating TCP socket");
    s->fd = socket (AF_INET, SOCK_STREAM, 0);
    setsockopt (s->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (int));

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
    servaddr.sin_port        = htons (s->port);

    if (bind (s->fd, (struct sockaddr *) &servaddr, sizeof (servaddr)) < 0)
        CLDD_ERROR("Failed to bind socket %ld", s->fd);
    if (listen (s->fd, BACKLOG) < 0)
        CLDD_ERROR("Unable to listen on socket %ld", s->fd);

    /* create the thread that monitors client requests */
    ret = pthread_create (&s->client_queue_tid, NULL, client_queue_handler, s);
    if (ret != 0)
        CLDD_ERROR("Failed to create thread to manage client queue requests");

    /* create the thread that spawns client connections */
    ret = pthread_create (&s->client_spawn_tid, NULL, client_spawn_handler, s);
    if (ret != 0)
        CLDD_ERROR("Failed to create thread that spawns client connections");

    /* wait for the control threads to finish */
    pthread_join (s->client_queue_tid, NULL);
    pthread_join (s->client_spawn_tid, NULL);

    CLDD_MESSAGE("Exiting the client manager");

    pthread_exit (NULL);
}

/**
 * client_func
 *
 * Function to test client connections.
 *
 * @param sockfd The socket of the client
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

    CLDD_MESSAGE("Entering client loop - sock fd = %d", c->fd);
    for (;;)
    {
        /* I hate to do this but without a delay the single thread
         * runs uninterrupted eliminating any traffic concurrency */
        usleep (1000);

        pthread_mutex_lock (&master_lock);
        /* blocking call to wait for client request */
        n = readline (c->fd, recv, MAXLINE);
        if (n == 0)
            break;

        /* a request message received from the client triggers a write */
        if (strcmp (recv, "request\n") == 0)
        {
            if ((n = writen (c->fd, send, strlen (send))) != strlen (send))
                CLDD_MESSAGE("Client write error - %d != %d", strlen (send), n);
            pthread_mutex_unlock (&master_lock);
            c->nreq++;
            c->ntot += n;
        }
        else if (strcmp (recv, "quit\n") == 0)
        {
            pthread_mutex_unlock (&master_lock);
            c->nreq++;
            break;
        }
    }
    CLDD_MESSAGE("Leaving client loop");

    pthread_mutex_lock (&s->server_data_lock);
    s->n_clients--;
    llist_remove (s->client_list, (void *)c, client_compare);
    pthread_mutex_unlock (&s->server_data_lock);
    /* log the client stats before closing it */
    fprintf (s->statsfp, "%s, %d, %d, %d\n",
             inet_ntoa (c->sa.sin_addr), c->fd, c->nreq, c->ntot);
    close (c->fd);
    client_free (c);
    c = NULL;

    free (recv);
    pthread_exit (NULL);
}

/**
 * monitor the client request queue and spawn client threads as necessary
 */
void *
client_spawn_handler (void *data)
{
    server *s = (server *)data;
    client *c = NULL;
    struct client_data_t cd;
    struct client_data_t *pcd;

    for (;;)
    {
        usleep (1000);

        pthread_mutex_lock (&s->spawn_queue_lock);

        CLDD_MESSAGE("Spawning clients");
        /* if any client requests were made spawn a new thread */
        while (queue_is_empty (s->spawn_queue))
            pthread_cond_wait (&s->spawn_queue_ready, &s->spawn_queue_lock);

        /* pop the next node in the queue */
        s->spawn_queue = queue_dequeue (s->spawn_queue, (void **)&c);
        pthread_mutex_unlock (&s->spawn_queue_lock);

        /* launch the thread for the client */
        pthread_mutex_lock (&s->server_data_lock);
        s->n_clients++;
        s->n_max_connected = (s->n_clients > s->n_max_connected)
                             ? s->n_clients : s->n_max_connected;
        pthread_mutex_unlock (&s->server_data_lock);
        cd.s = s;
        cd.c = c;
        pcd = &cd;
        pthread_create (&c->tid, NULL, client_func, pcd);
        CLDD_MESSAGE("Create client thread %ld", c->tid);

        pthread_mutex_lock (&s->server_data_lock);
        s->client_list = llist_append (s->client_list, (void *)c);
        CLDD_MESSAGE("Added client to list, new size: %d",
                     llist_length (s->client_list));
        pthread_mutex_unlock (&s->server_data_lock);
    }

    pthread_exit (NULL);
}

/**
 * wait for new client connection requests
 */
void *
client_queue_handler (void *data)
{
    server *s = (server *)data;
    client *c = NULL;

    for (;;)
    {
        usleep (1000);

        /* get the client data ready */
        c = client_new ();
        c->sa_len = sizeof (c->sa);

        /* blocking call waiting for connections */
        CLDD_MESSAGE("Waiting for a new client connection");
        c->fd = accept (s->fd, (struct sockaddr *) &c->sa, &c->sa_len);
        CLDD_MESSAGE("Received connection from (%s, %d)",
                     inet_ntoa (c->sa.sin_addr), ntohs (c->sa.sin_port));

        /* queue up the new connection */
        pthread_mutex_lock (&s->spawn_queue_lock);
        s->spawn_queue = queue_enqueue (s->spawn_queue, (void *)c);
        pthread_mutex_unlock (&s->spawn_queue_lock);
        /* don't need to signal if a queue exists */
        if (queue_size (s->spawn_queue) == 1)
            pthread_cond_signal (&s->spawn_queue_ready);

        c = NULL;
    }

    pthread_exit (NULL);
}
