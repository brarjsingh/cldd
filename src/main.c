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
#include "conf.h"
#include "daemon.h"
#include "utils.h"
#include "error.h"

/* replace later with value taken from configuration file */
#define PID_FILE    "/var/run/cldd.pid"

/* function prototypes */
void usage (void);
void signal_handler (int sig);
void *client_manager (void *data);
void *str_echo (void *arg);
ssize_t writen (int fd, const void *vptr, size_t n);
ssize_t readline (int fd, void *vptr, size_t maxlen);

pthread_t master_thread;
bool daemonized = false;

int main (int argc, char *argv[])
{
    int ret;

    /* setup signal handling first */
    signal (SIGHUP,  signal_handler);
    signal (SIGINT,  signal_handler);
    signal (SIGTERM, signal_handler);
    signal (SIGQUIT, signal_handler);

    daemonize_close_stdin ();

    daemonize_init ("root", "root", PID_FILE);
    //glue_daemonize_init (&options);
    //log_init (options.verbose, options.log_stderr);

    daemonize_set_user ();

    /* true starts daemon in detached mode */
    daemonize (true);
    //setup_log_output (options.log_stderr);

    /* start the master thread for client management */
    ret = pthread_create (&master_thread, NULL, client_manager, NULL);
    if (ret != 0)
        CLDD_ERROR("Unable to create client management thread");
    pthread_join (master_thread, NULL);

    daemonize_finish ();
    //close_log_files ();

    return EXIT_SUCCESS;
}

/**
 * usage
 *
 * Print the correct usage for launching the daemon, or the help.
 */
void usage (void)
{
}

/**
 * signal_handler
 *
 * Use syslog for now, when logging facilities are available should use those
 * instead.
 *
 * @param sig The signal received
 */
void signal_handler (int sig)
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
void *client_manager (void *data)
{
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

//    server *s = (server)data;

    /* setup the socket to listen for client connections */
    listenfd = socket (AF_INET, SOCK_STREAM, 0);

    bzero (&servaddr, sizeof (servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
//    servaddr.sin_port        = htons (s->port);
    servaddr.sin_port        = htons (8000);

    bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr));
    listen (listenfd, 5);

    for (;;)
    {
        pthread_t client;

        clilen = sizeof (cliaddr);
        /* blocking call waiting for connections */
        connfd = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);
        CLDD_MESSAGE("Received connection from (%s , %d)\n",
                     inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

        /* launch the thread for the client */
        pthread_create (&client, NULL, str_echo, &connfd);
    }

    pthread_exit (NULL);
}

/**
 * str_echo
 *
 * Function to test client connections.
 *
 * @param sockfd The socket of the client
 **/
void *str_echo (void *arg)
{
    long arg1, arg2;
    ssize_t n;
    char line[MAXLINE];

    int sockfd = (int)arg;

    for (;;)
    {
        if ((n = readline (sockfd, line, MAXLINE)) == 0)
            return;

        if (sscanf (line, "%ld%ld", &arg1, &arg2) == 2)
            snprintf (line, sizeof (line), "%ld\n", arg1 + arg2);
        else
            snprintf (line, sizeof (line), "input error\n");

        n = strlen (line);
        if (writen (sockfd, line, n) != n)
            CLDD_ERROR("Write error");
    }
}

/**
 * writen
 *
 * Write n bytes to a descriptor. Taken from Stevens UNP.
 */
ssize_t writen (int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write (fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;

            nleft -= nwritten;
            ptr   += nwritten;
        }
    }

    return n;
}

static int  read_cnt;
static char *read_ptr;
static char read_buf[MAXLINE];

static ssize_t
my_read (int fd, char *ptr)
{

    if (read_cnt <= 0) {
again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return(-1);
        } else if (read_cnt == 0)
            return(0);
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return(1);
}

ssize_t
readline (int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char    c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;  /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return(n - 1);  /* EOF, n - 1 bytes were read */
        } else
            return(-1);     /* error, errno set by read() */
    }

    *ptr = 0;   /* null terminate like fgets() */
    return(n);
}
